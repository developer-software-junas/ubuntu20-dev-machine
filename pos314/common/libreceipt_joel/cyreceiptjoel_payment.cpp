//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptJoel::assembleReceiptPayment (Json::Value& jheader,
                                            Json::Value& jpayment,
                                            salesReceiptType receiptType)
{
    string txtChange = "";
    //////////////////////////////////////
    //  Receipt width
    string skinny = cgiform("receipt_skinny");
    int left, right;
    receipt_offset(left, right);
    string iniarg = cgiform("ini");
    //////////////////////////////////////
    //  Payment flag(s)
    bool bextra = false;
    bool isGc = false; bool isRes = false;
    bool isCash = false; bool isCharge = false;
    bool isCreditCard = false; bool isMobilePay = false;
    //////////////////////////////////////
    //  Calculate the totals
    string tmp;
    Json::Value  jpayline;
    size_t j = jpayment.size ();
    double calcPayAmount = 0.00;
    double calcChangeAmount = 0.00;
    for (size_t i = 0; i < j; i++) {
        jpayline.clear ();
        jpayline = jpayment[(int)i];
        std::stringstream pp;
        pp << jpayline;
        _log->logmsg("PPPPP",pp.str().c_str());
        _log->logdebug("PPPPP",pp.str().c_str());
        //  Make sure a tender type exists
        if (jpayline["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        //  Validate the payment amount
        tmp = jpayline["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, -999999999, 999999999))
            return seterrormsg(_util->errormsg());

        double changeAmount = stodsafe(jpayline["change_amount"].asString());
        double paymentAmount = stodsafe(jpayline["payment_amount"].asString());
        if (paymentAmount < 0) paymentAmount *= -1;
        paymentAmount -= changeAmount;
        calcChangeAmount += changeAmount;

        //////////////////////////////////
        //  Discounted payment???
        if (jpayline["payment_discount"]!=Json::nullValue)  {
            Json::Value jdiscount = jpayline["payment_discount"];
            //////////////////////////////////////
            //  Calculate the item discount
            double discount = 0.00;
            if (jdiscount["discount_code"].asString().length() > 0) {
                tmp = jdiscount["discount_value"].asString ();
                if (!_util->valid_decimal("Payment discount value", tmp.c_str (), 1, 24, 0, 999999999))
                    return seterrormsg(_util->errormsg());
                double discountValue = stodsafe(jdiscount["discount_value"].asString ());
                if (jdiscount["discount_type"] != Json::nullValue)  {
                    tmp = jdiscount["discount_type"].asString();
                    if (tmp == "1") {
                        discountValue = (discountValue / 100);
                        discount = paymentAmount * discountValue;
                    } else {
                        discount = discountValue;
                    }
                }
                paymentAmount = paymentAmount - discount;
            }
        }
        calcPayAmount += paymentAmount;
        //////////////////////////////////
        //  Set the payment flag(s)
        char szright[64];
        Json::Value jtendertype;
        jtendertype = jpayline["tendertype"];
        isGc = jtendertype["is_gc"].asString() == "1";
        isRes = jtendertype["is_gc"].asString() == "1";
        isCash = jtendertype["is_cash"].asString() == "1";
        isCharge = jtendertype["is_charge"].asString() == "1";
        isCreditCard = jtendertype["is_credit_card"].asString() == "1";
        isMobilePay = jtendertype["is_social_distancing"].asString() == "1";
        //////////////////////////////////
        //  GC special processing
        if (isGc) {
            if (jpayline["gc"] == Json::nullValue)
                return seterrormsg("Unable to process the GC payment information");

            Json::Value gc, jgc = jpayline["gc"];
            ////////////////////////////
            //  GC payment(s)
            if (isGc) {
                sprintf (szright, "%.02f", paymentAmount);
                _txtpay += _util->cyLRText(left, jpayline["description"].asString(), right, szright);

                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                  gc.clear ();
                  gc = jgc[i];
                  _txtpay += gc["description"].asString();
                  //_txtpay += "\n";
                  _txtpay += _util->cyLRText(left, "  GC SERIES", right, gc["gcnum"].asString());

                  double dtmp = stodsafe(gc["original_balance"].asString());
                  sprintf (szright, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(left, "  OLD BALANCE", right, szright);

                  dtmp = stodsafe(gc["remaining_balance"].asString());
                  sprintf (szright, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(left, "  NEW BALANCE", right, szright);

                  bextra = true;
                  if (dtmp > 0.00)  {
                      Json::Value jgcrefund = jpayline["gcrefund"];
                  }
                }
            }
        }  else  {
            string sright = "0.00";
            if (receiptType == SRT_REFUND)  {
                if (paymentAmount != (_totalAmtDue + _totalServiceCharge))  {
                    double diff = paymentAmount - (_totalAmtDue + _totalServiceCharge);
                    if (diff < 0)  diff = diff * -1;
                    if (diff > .01)
                        sprintf (szright, "-%.02f", paymentAmount);
                    else
                        sprintf (szright, "-%.02f", _totalAmtDue + _totalServiceCharge);
                }  else
                    sprintf (szright, "-%.02f", paymentAmount);
            }  else if (receiptType == SRT_POSTVOID)  {
                sprintf (szright, "-%.02f", paymentAmount);
            } else  {
                if (isCash)
                    sprintf (szright, "%.02f", (paymentAmount+changeAmount));
                else
                    sprintf (szright, "%.02f", paymentAmount);
            }
            sright = FMTNumberComma(atof(szright));

            string sleft = "";
            sleft += jpayline["description"].asString();
            sleft += "(";
            sleft += jpayline["pay_code"].asString();
            sleft += ")";
            _txtpay += _util->cyLRText(left, sleft, right, sright);

            if (receiptType == SRT_REFUND)  {
                sleft  = "    RES NO.  : ";
                sleft += jpayline["approval_code"].asString ();
                sleft += "\n";
                _txtpay += sleft;

                sleft  = "    NAME     : ";
                sleft += jheader["first_name"].asString ();
                sleft += " ";
                sleft += jheader["last_name"].asString ();
                sleft += "\n";
                _txtpay += sleft;
            }
        }
        ////////////////////////////////////
        //  Credit card
        string headerText, lineDesc = "";
        if (isCreditCard) {
            bextra = true;
            int idxTerms = 0;
            lineDesc = "  TERMS ";
            tmp = jpayline["approval_terms"].asString();
            if (!_util->valid_number("Payment terms", tmp, 1, 2))
                jpayline["approval_terms"]="0";
            idxTerms = atoi(tmp.c_str ());
            switch (idxTerms) {
              case 0:
                headerText += "Straight";
                break;
              case 1:
                headerText += "3 months";
                break;
              case 2:
                headerText += "6 months";
                break;
              case 3:
                headerText += "9 months";
                break;
              case 4:
                headerText += "12 months";
                break;
              case 5:
                headerText += "18 months";
                break;
              case 6:
                headerText += "24 months";
                break;
              case 7:
                headerText += "36 months";
                break;
            }
            headerText += " ";
            if (isCreditCard) {
              _txtpay += _util->cyLRText(left, lineDesc, right, headerText);
            }
            //lineDesc  = "  ACQUIRER";
            //receipt += cyLRText(23, lineDesc, 19, "${pay.getApprovalMerchant()} ");
            lineDesc = "  APPROVAL CODE ";
            _txtpay += _util->cyLRText(left, lineDesc, right, jpayline["approval_code"].asString());
        }
        ////////////////////////////////////
        //  Charge card
        lineDesc = "";
        if (isCharge) {
            bextra = true;
            lineDesc = "  ACCOUNT   ";
            lineDesc += jpayline["account_number"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;

            lineDesc = "  CUSTOMER  ";
            lineDesc += jpayline["first_name"].asString();
            lineDesc += " ";
            lineDesc += jpayline["last_name"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;
        }
        if (isMobilePay) {
            int idxTerms = 0;
            bextra = true;
            lineDesc = "  TERMS ";
            tmp = jpayline["approval_terms"].asString();
            if (!_util->valid_number("Payment terms", tmp, 1, 2))
                return seterrormsg(_util->errormsg());
            idxTerms = atoi(tmp.c_str ());
            switch (idxTerms) {
              case 0:
                headerText += "Straight";
                break;
              case 1:
                headerText += "3 months";
                break;
              case 2:
                headerText += "6 months";
                break;
              case 3:
                headerText += "9 months";
                break;
              case 4:
                headerText += "12 months";
                break;
              case 5:
                headerText += "18 months";
                break;
              case 6:
                headerText += "24 months";
                break;
              case 7:
                headerText += "36 months";
                break;
            }
            _txtpay += _util->cyLRText(left, lineDesc, right, headerText);

            lineDesc  = "  ACQUIRER";
            _txtpay += _util->cyLRText(left, lineDesc, right, "eCredit");

            lineDesc  = "  CARD HOLDER";
            _txtpay += _util->cyLRText(left, lineDesc, right, jpayline["first_name"].asString());

            lineDesc = "  APPROVAL CODE ";
            _txtpay += _util->cyLRText(left, lineDesc, right, jpayline["approval_code"].asString());
        }
        if (!bextra)  {
            lineDesc  = "    REFERENCE: ";
            lineDesc += jpayline["approval_code"].asString();
            if (jpayline["approval_code"].asString().length() > 0 && receiptType != SRT_REFUND)  {
                _txtpay += lineDesc;
                _txtpay += "\n";
            }
            if (isRes)  {
                lineDesc  = "    REFERENCE: ";
                lineDesc += jpayline["credit_balance"].asString();
                if (jpayline["credit_balance"].asString().length() > 0 && receiptType != SRT_REFUND)  {
                    _txtpay += lineDesc;
                    _txtpay += "\n";
                }
            }
        }
        //////////////////////////////////////
        //  GC has no change
        char sztmp[64];
        if (isGc && calcChangeAmount > 0)
            calcChangeAmount = 0.00;
        sprintf (sztmp, "%.02f", (calcChangeAmount*1000)/1000);
        calcChangeAmount = atof(sztmp);
        jheader["cytotal_change"]=calcChangeAmount;
        //////////////////////////////////////
        //  Display the change
        if (receiptType != salesReceiptType::SRT_REFUND &&
            receiptType != salesReceiptType::SRT_POSTVOID)  {
            if (calcChangeAmount > 0) {
                txtChange = _util->cyLRText(left, "Change", right, sztmp);
            }  else  {
                if (calcChangeAmount != 0)  {
                    double dtmp = calcChangeAmount * -1;
                    sprintf (sztmp, "%.02f", dtmp);
                    //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
                }
            }
        }
    }
    _txtpay += txtChange;
    //////////////////////////////////////
    //  Save the total payment
    jheader["cytotal_payment"]=calcPayAmount;
     //////////////////////////////////////
     //  Net sales amount
    //////////////////////////////////////
    //  Net sales amount
    string netSales = "0";
    receipt_vat_breakdown(_receiptType,true,netSales);
    double netSalesAmount = _util->stodsafe(netSales);
     //double netSalesAmount = _totalGrossAmount -
             //(_totalItemDiscount+_totalTrxDiscount+_totalAmtExempt+_totalAmtZero);
     //////////////////////////////////////
     //  Calculate the change
     double absNet = netSalesAmount;
     if (absNet < 0)
         absNet = netSalesAmount * -1;

     /////////////////////////////////////
     //  Total item(s)
     char sztmp[128];
     _txtpay += _util->cyCenterText(_rcptWidth, " ");
     sprintf (sztmp, "%.03f-item(s)", _totalItems);
     _txtpay += _util->cyCenterText(_rcptWidth, sztmp);
     _txtpay += _util->cyCenterText(_rcptWidth, " ");
     if (receiptType == salesReceiptType::SRT_SUSPEND)  {
         _receipt += _txtpay;
         return true;
     }
     /////////////////////////////////////
     //  Payment footer
     Json::Value jcashier;
     if (jheader["settings_cashier"] == Json::nullValue)
         return seterrormsg("Unable to get the cashier details");
     jcashier = jheader["settings_cashier"];

     size_t len = jcashier["first_name"].asString().length();
     len += jcashier["last_name"].asString().length();
     len += 14;
     len += jcashier["login"].asString().length();

     if (len > (size_t)_rcptWidth)  {
         if (skinny == "true")  {
             _txtpay += "Cashier   : ";
         }  else  {
             _txtpay += "Cashier      : ";
         }
         _txtpay += jcashier["first_name"].asString();
         _txtpay += " ";
         _txtpay += jcashier["last_name"].asString();

         _txtpay += "\n            [";
         _txtpay += jcashier["login"].asString();
         _txtpay += "]\n";
     }  else  {
         if (skinny == "true")  {
             _txtpay += "Cashier   : ";
         }  else  {
             _txtpay += "Cashier      : ";
         }
         _txtpay += jcashier["first_name"].asString();
         _txtpay += " ";
         _txtpay += jcashier["last_name"].asString();
         _txtpay += "[";
         _txtpay += jcashier["login"].asString();
         _txtpay += "]\n";
     }
     if (receiptType == SRT_REFUND)  {
         if (skinny == "true")  {
             _txtpay += "Txn No.  : ";
         }  else  {
             _txtpay += "Txn No.      : ";
         }
         _txtpay += jheader["systransnum"].asString();
         _txtpay += "\n";

         if (skinny == "true")  {
             _txtpay += "    Date : ";
         }  else  {
             _txtpay += "Business Date: ";
         }
         _txtpay += jheader["logical_date"].asString();
         _txtpay += "\n";

         if (skinny == "true")  {
             _txtpay += "Orig Pos#: ";
         }  else  {
             _txtpay += "Orig Pos No  : ";
         }
         _txtpay += jheader["orig_register_number"].asString();
         _txtpay += "\n";

         if (skinny == "true")  {
             _txtpay += "Orig Date: ";
         }  else  {
             _txtpay += "Orig Bus Date: ";
         }
         _txtpay += jheader["orig_transaction_date"].asString();
         _txtpay += "\n";

         if (skinny == "true")  {
             _txtpay += "Orig INV : ";
         }  else  {
             _txtpay += "Orig Invoice : ";
         }
         _txtpay += jheader["orig_systransnum"].asString();
         _txtpay += "\n";
     }  else  {

         string tmp2, trans_time,trans_date,dd,mm,yyyy;
         trans_date = jheader["transaction_date"].asString();
         trans_time = jheader["transaction_time"].asString();
         _util->date_split(trans_date.c_str(),yyyy,mm,dd);

         if (skinny == "true")  {
             tmp2  = "Date-time : ";
             tmp2 += mm;
             tmp2 += "/";
             tmp2 += dd;
             tmp2 += "/";
             tmp2 += yyyy;
             tmp2 += "-";
             tmp2 += _util->time_colon(trans_time.c_str());

             _txtpay += tmp2.c_str();
             _txtpay += "\n";

             _txtpay += "SYSDATE   : ";
             _txtpay += _util->date_nice(jheader["logical_date"].asString().c_str ());
             _txtpay += "\n\n";
         }  else  {
             tmp2  = "Date-time    : ";
             tmp2 += mm;
             tmp2 += "/";
             tmp2 += dd;
             tmp2 += "/";
             tmp2 += yyyy;
             tmp2 += "-";
             tmp2 += _util->time_colon(trans_time.c_str());

             _txtpay += tmp2.c_str();
             _txtpay += "\n";

             _txtpay += "SYSDATE      : ";
             _txtpay += _util->date_nice(jheader["logical_date"].asString().c_str ());
             _txtpay += "\n\n";
         }
     }
     _receipt += _txtpay;
     return true;
}
