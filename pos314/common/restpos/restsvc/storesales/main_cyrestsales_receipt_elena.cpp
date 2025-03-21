//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleFspUrl(Json::Value& jheader,
                                         Json::Value& jdetail,
                                         Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceipt(Json::Value& jheader,
                                    Json::Value& jdetail,
                                    Json::Value& jpayment,
                                    salesReceiptType receiptType)
{
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;

    Json::Value jline;
    _txtdet = "";
    int j = jdetail.size ();
    for (int i = 0; i < j; i++)  {
        jline = jdetail[i];
        Json::Value jhdrtrx = jheader["settings_transtype"];
        if (!assembleReceiptDetail(jline,jhdrtrx,receiptType))
            return false;
        jdetail[i]=jline;
    }

    if (!assembleReceiptTotals(jheader))
        return false;

    if (!assembleReceiptPayment(jheader,jpayment,receiptType))
        return false;

    if (!assembleReceiptAccount(jheader))
        return false;

    if (!assembleVatBreakdown(receiptType))
        return false;

    if (!assembleReceiptSignature(jheader))
        return false;

    if (!assembleReceiptFooter(jheader,receiptType))
        return false;

    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptElena::assembleReceiptFooter (Json::Value& jheader,
                                                          salesReceiptType receiptType)
{
    _txtfoot = "";
    if (jheader["hdr_settings"]==Json::nullValue)
        return seterrormsg("Unable to retrieve the POS settings information");

    Json::Value jpos;
    int w = g_widthReceipt;
    jpos = jheader["hdr_settings"];
    if (receiptType != salesReceiptType::SRT_NORMAL)  {
        _txtfoot += _util->cyCenterText(w, "THIS DOCUMENT IS NOT VALID");
        _txtfoot += _util->cyCenterText(w, "FOR CLAIM OF INPUT TAX");

    }  else  {
        string tmp;
        Json::Value jtemp;

        jtemp = jheader["hdr_company"];
        _txtfoot += _util->cyCenterText(w, jtemp["description"].asString());
        _txtfoot += _util->cyCenterText(w, "c/o CYWARE INCORPORATED");


        jtemp = jheader["hdr_branch"];
        tmp  = jtemp["addr1"].asString();
        tmp += ", ";
        tmp += jtemp["addr2"].asString();
        _txtfoot += _util->cyCenterText(w, tmp);


        jtemp = jheader["hdr_branch"];
        tmp  = "VAT-REG-TIN: ";
        tmp += jtemp["tin"].asString();
        _txtfoot += _util->cyCenterText(w, tmp);
        _txtfoot += _util->cyCenterText(w," ");

        tmp  = "ACCR.# ";
        tmp += jtemp["bir_num"].asString();
        _txtfoot += _util->cyCenterText(w, tmp);

        jtemp = jheader["settings_register"];
        tmp  = "Date issued: ";
        tmp += _util->date_nice(jtemp["permit_start"].asString().c_str ());
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = "Valid until: ";
        tmp += _util->date_nice(jtemp["permit_expire"].asString().c_str ());
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = jtemp["permit"].asString();
        _txtfoot += _util->cyCenterText(w, tmp);
        _txtfoot += _util->cyCenterText(w," ");

        tmp  = "This serves as your Sales Invoice.";
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = "THIS INVOICE/RECEIPT SHALL BE VALID";
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = "FOR FIVE (5) YEARS FROM THE DATE OF";
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = "THE PERMIT TO USE";
        _txtfoot += _util->cyCenterText(w, tmp);
        //_txtfoot += _util->cyCenterText(w," ");
    }

    if (receiptType == SRT_REFUND)  {
        _txtfoot += "Customer Name :________________________\n";
        _txtfoot += "Address :______________________________\n";
        _txtfoot += "Phone No. :____________________________\n\n";
        _txtfoot += "Approving Manager :____________________\n\n\n\n";

        _txtfoot += "     Return and Exchange Slip (RES)\n";
        _txtfoot += "----------------------------------------\n";
        _txtfoot += "Please present RES upon exchange of item\n";
        _txtfoot += "         For one-time use only\n";
        _txtfoot += "       Not replaceable when lost\n";
        _txtfoot += "Can be used in Rustan`s Department Store\n";
        _txtfoot += "                  only\n";
    }
    //////////////////////////////////////
    //  Footer
    _txtfoot += _util->cyCenterText(w, " ");
    if (jpos["szmsg01"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg01"].asString());
    }
    if (jpos["szmsg02"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg02"].asString());
    }
    if (jpos["szmsg03"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg03"].asString());
    }
    if (jpos["szmsg04"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg04"].asString());
    }
    if (jpos["szmsg05"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg05"].asString());
    }
    _txtfoot += _util->cyCenterText(w, " ");

    _receipt += _txtfoot;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptElena::assembleReceiptSignature (Json::Value& jheader)
{
    _txtsig = "";
    if (jheader["hdr_tax_receipt"]==Json::nullValue)  {
        _txtsig += "\n";
        _txtsig += "Customer name:   _______________________\n";
        _txtsig += "Address:         _______________________\n";
        _txtsig += "TIN:             _______________________\n";
        _txtsig += "Business style:  _______________________\n";
        _txtsig += "\n";
        return true;
    }

    Json::Value jacct;
    jacct = jheader["hdr_tax_receipt"];
    //////////////////////////////////////
    //  OR customer
    _txtsig += "\n";
    if (jacct["company_name"].asString().length() > 0)  {
        _txtsig += "           Name: ";
        _txtsig += jacct["company_name"].asString();
        _txtsig += "\n";
    }
    if (jacct["address1"].asString().length() > 0)  {
        _txtsig += "       Address1: ";
        _txtsig += jacct["address1"].asString();
        _txtsig += "\n";
    }
    if (jacct["address2"].asString().length() > 0)  {
        _txtsig += "       Address2: ";
        _txtsig += jacct["address2"].asString();
        _txtsig += "\n";
    }
    if (jacct["tin"].asString().length() > 0)  {
        _txtsig += "            TIN: ";
        _txtsig += jacct["tin"].asString();
        _txtsig += "\n";
    }
    if (jacct["customer_name"].asString().length() > 0)  {
        _txtsig += " Business Style: ";
        _txtsig += jacct["customer_name"].asString();
        _txtsig += "\n";
    }
    _txtsig += "\n";
    _receipt += _txtsig;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptElena::assembleVatBreakdown (salesReceiptType receiptType)
{
    _txtvat = "";
    int w = g_widthReceipt;

    _txtvat += "\n";
    _txtvat += _util->cyPostSpace(13, "VAT BREAKDOWN");
    _txtvat += _util->cyPreSpace(15, "NET SALE");
    _txtvat += _util->cyPreSpace(12, "VAT AMT");
    _txtvat += "\n";
    _txtvat += _util->cyAcrossText(w, "-");
    //////////////////////////////////////
    //  VAT amounts
    char szNetVat[32]; char szNetZero[32]; char szNetExempt[32];
    char szAmtVat[32]; char szAmtZero[32]; char szAmtExempt[32];

    if (receiptType == SRT_REFUND || receiptType == SRT_POSTVOID)  {
        if (_totalNetVat != 0)
            sprintf (szNetVat, "-%.02f", (_totalNetVat * 1000) / 1000);
        else
            sprintf (szNetVat, "%.02f", (_totalNetVat * 1000) / 1000);
        if (_totalNetZero != 0)
            sprintf (szNetZero, "-%.02f", (_totalNetZero * 1000) / 1000);
        else
            sprintf (szNetZero, "%.02f", (_totalNetZero * 1000) / 1000);
        if (_totalNetExempt != 0)
            sprintf (szNetExempt, "-%.02f", (_totalNetExempt * 1000) / 1000);
        else
            sprintf (szNetExempt, "%.02f", (_totalNetExempt * 1000) / 1000);

        if (_totalAmtVat != 0)
            sprintf (szAmtVat, "-%.02f", (_totalAmtVat * 1000) / 1000);
        else
            sprintf (szAmtVat, "%.02f", (_totalAmtVat * 1000) / 1000);
        if (_totalAmtZero != 0)
            sprintf (szAmtZero, "-%.02f", (_totalAmtZero * 1000) / 1000);
        else
            sprintf (szAmtZero, "%.02f", (_totalAmtZero * 1000) / 1000);
        if (_totalAmtExempt != 0)
            sprintf (szAmtExempt, "-%.02f", (_totalAmtExempt * 1000) / 1000);
        else
            sprintf (szAmtExempt, "%.02f", (_totalAmtExempt * 1000) / 1000);
    }  else  {
        sprintf (szNetVat, "%.02f", (_totalNetVat * 1000) / 1000);
        sprintf (szNetZero, "%.02f", (_totalNetZero * 1000) / 1000);
        sprintf (szNetExempt, "%.02f", (_totalNetExempt * 1000) / 1000);
        sprintf (szAmtVat, "%.02f", (_totalAmtVat * 1000) / 1000);
        sprintf (szAmtZero, "%.02f", (_totalAmtZero * 1000) / 1000);
        sprintf (szAmtExempt, "%.02f", (_totalAmtExempt * 1000) / 1000);
    }

    _txtvat += _util->cyPostSpace(12, "VATable");
    _txtvat += _util->cyPreSpace(16, szNetVat);
    _txtvat += _util->cyPreSpace(12, szAmtVat);
    _txtvat += "\n";

    _txtvat += _util->cyPostSpace(12, "VAT exempt");
    _txtvat += _util->cyPreSpace(16, szNetExempt);
    _txtvat += _util->cyPreSpace(12, "0.00");//szAmtExempt);
    _txtvat += "\n";

    _txtvat += _util->cyPostSpace(12, "VAT zero");
    _txtvat += _util->cyPreSpace(16, szNetZero);
    _txtvat += _util->cyPreSpace(12, "0.00");//szAmtZero);
    _txtvat += "\n";

    _txtvat += _util->cyAcrossText(40, "-");

    double totalNetVat = _totalNetVat + _totalNetZero + _totalNetExempt;
    char szTotalNetVat[32];

    if (totalNetVat != 0 && receiptType == SRT_REFUND)
        sprintf (szTotalNetVat, "-%.02f", (totalNetVat * 1000) / 1000);
    else
        sprintf (szTotalNetVat, "%.02f", (totalNetVat * 1000) / 1000);
    //sprintf (szTotalNetVat, "%.02f", (totalNetVat * 1000) / 1000);


    double totalAmtVat = _totalAmtVat;// + _totalAmtZero + _totalAmtExempt;
    char szTotalAmtVat[32];
    if (totalAmtVat != 0 && receiptType == SRT_REFUND)
        sprintf (szTotalAmtVat, "-%.02f", (totalAmtVat * 1000) / 1000);
    else
        sprintf (szTotalAmtVat, "%.02f", (totalAmtVat * 1000) / 1000);
    //sprintf (szTotalAmtVat, "%.02f", (totalAmtVat * 1000) / 1000);

    _txtvat += _util->cyPostSpace(13, "TOTAL");
    _txtvat += _util->cyPreSpace (15, szTotalNetVat);
    _txtvat += _util->cyPreSpace (12, szTotalAmtVat);
    _txtvat += "\n";

    _receipt += _txtvat;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptAccount (Json::Value& jheader)
{
    _txtacct = "";
    string info, label;
    bool isFsp = false;
    bool isBeauty = false;
    Json::Value jacct, jaccount;
    int w = g_widthReceipt;

    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    int j = jaccount.size();
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_loyalty"].asString() == "1") {
          isFsp = true;
          label = "FSP Details";
        } else if (jacct["is_shopper"].asString() == "1") {
          label = "Personal shopper";
        } else if (jacct["is_clerk"].asString() == "1") {
          label = "Sales clerk";
        } else if (jacct["is_zero_rated"].asString() == "1") {
          label = "VAT exempt";
        } else if (jacct["is_beauty_addict"].asString() == "1") {
          isBeauty = true;
          label = "Beauty Card Details";
        } else if (jacct["is_agent"].asString() == "1") {
          label = "CRM account";
        } else {
          label = jacct["account_type_desc"].asString();
        }

        if (isFsp || isBeauty) {
            _txtacct += _util->cyCenterText(w, " ");
            _txtacct += _util->cyCenterText(w, label);

            info  = "Account Number: ";
            info += jacct["account_number"].asString();
            _txtacct += _util->cyCenterText(w, info);

            info = "Name: ";
            info += jacct["first_name"].asString();
            info += " ";
            info += jacct["last_name"].asString();
            _txtacct += _util->cyCenterText(w, info);
            /***TODO  ADD POINTS HERE ***/
        }  else  {
          _txtacct += _util->cyCenterText(w, " ");
          _txtacct += _util->cyCenterText(w, label);

          info  = "Account Number: ";
          info += jacct["account_number"].asString();
          _txtacct += _util->cyCenterText(w, info);

          info = "Name: ";
          info += jacct["first_name"].asString();
          info += " ";
          info += jacct["last_name"].asString();
          _txtacct += _util->cyCenterText(w, info);
        }
    }
    _receipt += _txtacct;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptPayment (Json::Value& jheader,
                                            Json::Value& jpayment,
                                            salesReceiptType receiptType)
{
    string tmp;
    char sztmp[32];
    bool isGc = false;
    //bool isCheck = false;
    //bool isDebit = false;
    bool isCharge = false;
    bool isCreditCard = false;


    _txtpay = "";
    Json::Value jtmp, jpay;
    double totalPaymentAmount = 0.00;
    size_t j = jpayment.size();
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];

        tmp = jpay["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, 0, 999999999))
            return seterrormsg(_util->errormsg());
        double paymentAmount = stod(jpay["payment_amount"].asString());
        //////////////////////////////////
        //  Discounted payment???
        if (jpay["payment_discount"]!=Json::nullValue)  {
            Json::Value jdiscount = jpay["payment_discount"];
            //////////////////////////////////////
            //  Calculate the item discount
            double discount = 0.00;
            if (jdiscount["discount_code"].asString().length() > 0) {
                tmp = jdiscount["discount_value"].asString ();
                if (!_util->valid_decimal("Payment discount value", tmp.c_str (), 1, 24, 0, 999999999))
                    return seterrormsg(_util->errormsg());
                double discountValue = stod(jdiscount["discount_value"].asString ());
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
        totalPaymentAmount += paymentAmount;
        //////////////////////////////////////
        //  Special payment information
        isGc = false;
        //isCheck = false;
        //isDebit = false;
        isCharge = false;
        isCreditCard = false;

        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isGc = jtmp["is_gc"].asString() == "1";
        //isCheck = jtmp["is_check"].asString() == "1";
        isCharge = jtmp["is_charge"].asString() == "1";
        //isDebit = jtmp["is_debit_card"].asString() == "1";
        isCreditCard = jtmp["is_credit_card"].asString() == "1";
        ////////////////////////////////////
        //  GC special processing
        if (isGc) {
            if (jpay["gc"] == Json::nullValue)
                return seterrormsg("Unable to process the GC payment information");

            Json::Value gc, jgc = jpay["gc"];
            ////////////////////////////
            //  GC payment(s)
            if (isGc) {
                sprintf (sztmp, "%.02f", paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, sztmp);
                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                  gc.clear ();
                  gc = jgc[i];
                  _txtpay += gc["description"].asString();
                  //_txtpay += "\n";
                  _txtpay += _util->cyLRText(23, "  GC SERIES", 18, gc["gcnum"].asString());

                  double dtmp = stod(gc["amount_paid"].asString());
                  sprintf (sztmp, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(23, "  OLD BALANCE", 18, sztmp);
                  dtmp = stod(gc["remaining_balance"].asString());
                  sprintf (sztmp, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(23, "  NEW BALANCE", 18, sztmp);

                  if (dtmp > 0.00)  {
                      Json::Value jgcrefund = jpay["gcrefund"];

                  }
                }
            }
        }  else  {
            if (receiptType == SRT_REFUND || receiptType == SRT_POSTVOID)
                sprintf (sztmp, "-%.02f", paymentAmount);
            else
                sprintf (sztmp, "%.02f", paymentAmount);

            string left = "";
            left += jpay["description"].asString();
            left += "(";
            left += jpay["pay_code"].asString();
            left += ")";
            _txtpay += _util->cyLRText(23, left, 18, sztmp);

            if (receiptType == SRT_REFUND)  {
                left  = "    RES NO.  : ";
                left += jpay["approval_code"].asString ();
                left += "\n";
                _txtpay += left;

                left  = "    NAME     : ";
                left += jheader["first_name"].asString ();
                left += " ";
                left += jheader["last_name"].asString ();
                left += "\n";
                _txtpay += left;
            }
        }
        ////////////////////////////////////
        //  Credit card
        string headerText, lineDesc = "";
        if (isCreditCard) {
            int idxTerms = 0;
            lineDesc = "  TERMS ";
            tmp = jpay["approval_terms"].asString();
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
            headerText += " ";
            if (isCreditCard) {
              _txtpay += _util->cyLRText(24, lineDesc, 18, headerText);
            }
            //lineDesc  = "  ACQUIRER";
            //receipt += cyLRText(23, lineDesc, 19, "${pay.getApprovalMerchant()} ");
            lineDesc = "  APPROVAL CODE ";
            _txtpay += _util->cyLRText(23, lineDesc, 18, jpay["approval_code"].asString());
        }
        ////////////////////////////////////
        //  Charge card
        lineDesc = "";
        if (isCharge) {
            lineDesc = "  ACCOUNT   ";
            lineDesc += jpay["account_number"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;

            lineDesc = "  CUSTOMER  ";
            lineDesc += jpay["first_name"].asString();
            lineDesc += " ";
            lineDesc += jpay["last_name"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;
        }
    }
    jheader["cytotal_payment"]=totalPaymentAmount;
    double totalChangeAmount = totalPaymentAmount - _totalNetSales;
    if (isGc && totalChangeAmount > 0)
        totalChangeAmount = 0.00;
    sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
    totalChangeAmount = atof(sztmp);
    jheader["cytotal_change"]=totalChangeAmount;
    if (receiptType != salesReceiptType::SRT_REFUND)  {
        if (totalChangeAmount > 0) {
            _txtpay += _util->cyLRText(23, "Change", 18, sztmp);
        }  else  {
            if (totalChangeAmount != 0)  {
                double dtmp = totalChangeAmount * -1;
                sprintf (sztmp, "%.02f", dtmp);
                //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
            }
        }
    }

    int w = g_widthReceipt;
    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%ld-item(s)", (long)_totalItems);
    _txtpay += _util->cyCenterText(w, sztmp);
    _txtpay += _util->cyCenterText(w, " ");



    Json::Value jcashier;
    if (jheader["settings_cashier"] == Json::nullValue)
        return seterrormsg("Unable to get the cashier details");
    jcashier = jheader["settings_cashier"];

    size_t len = jcashier["first_name"].asString().length();
    len += jcashier["last_name"].asString().length();
    len += 14;
    len += jcashier["login"].asString().length();

    if (len > 40)  {
        _txtpay += "Cashier      : ";
        _txtpay += jcashier["first_name"].asString();
        _txtpay += " ";
        _txtpay += jcashier["last_name"].asString();

        _txtpay += "\n           [";
        _txtpay += jcashier["login"].asString();
        _txtpay += "]\n";
    }  else  {
        _txtpay += " Cashier     : ";
        _txtpay += jcashier["first_name"].asString();
        _txtpay += " ";
        _txtpay += jcashier["last_name"].asString();
        _txtpay += "[";
        _txtpay += jcashier["login"].asString();
        _txtpay += "]\n";
    }
    if (receiptType == SRT_REFUND)  {
        _txtpay += "Txn No.      : ";
        _txtpay += jheader["systransnum"].asString();
        _txtpay += "\n";

        _txtpay += "Business Date: ";
        _txtpay += jheader["systransnum"].asString();
        _txtpay += "\n";

        _txtpay += "Orig Pos No  : ";
        _txtpay += jheader["orig_register_number"].asString();
        _txtpay += "\n";

        _txtpay += "Orig Bus Date: ";
        _txtpay += jheader["orig_transaction_date"].asString();
        _txtpay += "\n";

        _txtpay += "Original SI# : ";
        _txtpay += jheader["orig_systransnum"].asString();
        _txtpay += "\n";
    }  else  {
        _txtpay += " Date-Time   : ";
        _txtpay += _util->date_pos();
        _txtpay += "\n";

        if (jheader["ridc_number"].asString().length () > 0)  {
          _txtpay += " RIDC#       : ";
          _txtpay += jheader["ridc_number"].asString();
          _txtpay += "\n";
        }
        _txtpay += " SYSDATE     : ";
        _txtpay += _util->date_nice(jheader["logical_date"].asString().c_str ());
        _txtpay += "\n";
    }

    _receipt += _txtpay;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptTotals (Json::Value& jheader)
{
    char sztmp [32];
    _txttot = "";
    //////////////////////////////////////
    //  Prep work
    string trxDiscountDesc = "";
    if (_totalTrxDiscount > 0)  {
        Json::Value jtrx = jheader["settings_transaction_discount"];
        trxDiscountDesc = jtrx["description"].asString ();
    }
    //////////////////////////////////////
    //  Detail total(s)
    _txttot += _util->cyAcrossText(40, "-");
    sprintf (sztmp, "%.02f", _totalGrossAmount);
    _txttot += _util->cyLRText(23, "Total sales amount", 18, sztmp);
    //////////////////////////////////////
    //  Sequence change based on type
    if (_isPwd || _isSenior)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,sztmp);
        }
        //  Item discount
        if (_totalItemDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalItemDiscount);
            _txttot += _util->cyLRText(23,"Item discount",18,sztmp);
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount);
        sprintf (sztmp, "%.02f", netSalesAmount);
        _txttot += _util->cyLRText(23,"Net sales amount",18,sztmp);
        //  VAT exempt
        if (_totalAmtExempt != 0.00)  {
            sprintf (sztmp, "-%.02f", _totalAmtExempt);
            _txttot += _util->cyLRText(23,"VAT 12%",18,sztmp);
        }
        //  Amount due
        sprintf (sztmp, "%.02f", _totalNetSales);
        _txttot += _util->cyLRText(23,"Total amount due",18,sztmp);
    }  else if (_isZeroRated)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,sztmp);
        }
        //  Item discount
        if (_totalItemDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalItemDiscount);
            _txttot += _util->cyLRText(23,"Item discount",18,sztmp);
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount);
        sprintf (sztmp, "%.02f", netSalesAmount);
        _txttot += _util->cyLRText(23,"Net sales amount",18,sztmp);
        //  VAT exempt
        if (_totalAmtZero != 0.00)  {
            sprintf (sztmp, "-%.02f", _totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,sztmp);
        }
        //  Amount due
        sprintf (sztmp, "%.02f", _totalNetSales);
        _txttot += _util->cyLRText(23,"Total amount due",18,sztmp);
    }  else  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,sztmp);
        }
        //  Item discount
        if (_totalItemDiscount > 0)  {
            sprintf (sztmp, "-%.02f", _totalItemDiscount);
            _txttot += _util->cyLRText(23,"Item discount",18,sztmp);
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount);
        sprintf (sztmp, "%.02f", netSalesAmount);
        _txttot += _util->cyLRText(23,"Net sales amount",18,sztmp);

        //  VAT exempt
        if (_totalAmtZero > 0)  {
            //  VAT exempt
            sprintf (sztmp, "-%.02f", _totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,sztmp);
        }  else  {
            sprintf (sztmp, "%.02f", _totalAmtVat);
            _txttot += _util->cyLRText(23,"VAT 12%",18,sztmp);
        }
        //  Amount due
        sprintf (sztmp, "%.02f", _totalNetSales);
        _txttot += _util->cyLRText(23,"Total amount due",18,sztmp);
    }
    _receipt += _txttot;
/*
    receipt += cyLRText(23,"VAT 12%",19,"${cyFormatNumber(birVatAmt.toString(), 2)} ");
    receipt += cyLRText(23,"Total amount due",19,"${cyFormatNumber(getTotalSalesAmountDue().toString(), 2)} ");

    receiptPayment();

    receipt += cyCenterText(w, " ");
    receipt += cyCenterText(w, "${cyFormatNumber(sales.ttSumQuantity().toString(), 2)}-item(s)");
    receipt += cyCenterText(w, " ");
    receipt += "Cashier   : ${cashier.getFirstName()} "
               "${cashier.getLastName()}[${cashier.getLogin()}]\n";
    receipt += "DATE-TIME : $formatter\n";
    if (sales.getRidcNumber().isNotEmpty && sales.getRidcNumber() != "0")  {
      receipt += "    RIDC# : ${sales.getRidcNumber()}\n";
    }
    receipt += "  SYSDATE : ${cyNiceDate(sales.getLogicalDate())}\n";*/
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptDetail (Json::Value& jdetail,
                                           Json::Value& jhdrtrx,
                                           salesReceiptType type)
{
    //_txtdet = "";

    string tmp = jdetail["quantity"].asString();
    double quantity = stod(jdetail["quantity"].asString());
    double priceOriginal = stod(jdetail["retail_price"].asString());
    if (type == SRT_POSTVOID)  {
        quantity = quantity * -1;
    }
    //////////////////////////////////////
    //  Display consignment detail(s)
    if (jdetail["consign_reference"].asString().length() > 0)  {
        tmp  = "Consignment-";
        tmp += jdetail["consign_reference"].asString();
        tmp += "-";
        tmp += jdetail["descripttion"].asString();
        tmp += "\n";
        _txtdet += tmp;
    }  else if (jdetail["description"].asString() != "SPLIT CHECK ADJUSTMENT") {
        Json::Value jproduct = jdetail["detail_product_header"];
        tmp  = jproduct["category_cd"].asString();
        tmp += "-";
        tmp += jdetail["item_code"].asString();
        tmp += "-";
        tmp += _util->string_cleanse(jdetail["description"].asString().c_str (),true);

        if (tmp.length() > 40) {
            _txtdet += tmp.substr(0, 39);
        } else {
            _txtdet += tmp;
        }
        _txtdet += "\n";
    }
    //////////////////////////////////////
    //  Item quantity / price
    char sztmp[32], sztmp2[32];
    if (jdetail["description"].asString() == "SPLIT CHECK ADJUSTMENT")  {
        sprintf (sztmp, "%.02f", priceOriginal * quantity);
        tmp = _util->cyLRText(23, jdetail["description"].asString(), 18, sztmp);
        _txtdet += tmp;
    }  else {
        string uom;
        if (jdetail["uom_code"].asString().length() < 1) {
            uom = "";
        } else {
            uom = jdetail["uom_code"].asString();
        }
        string itemSuffix;
        if (jdetail["vat_exempt"].asString () == "true")
            itemSuffix = "NV";
        else  {
            if (jhdrtrx["is_sc"].asString() == "1" ||
                jhdrtrx["is_pwd"].asString() == "1" ||
                jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "NV";
            }  else  {
                itemSuffix = "V";
            }
        }

        sprintf (sztmp, "  %.02f %s @ %.02f", quantity, uom.c_str (), priceOriginal);
        sprintf (sztmp2, "%.02f%s", priceOriginal * quantity, itemSuffix.c_str ());
        tmp = _util->cyLRText(22, sztmp, 19, sztmp2);
        _txtdet += tmp;
    }
    //////////////////////////////////
    //  Are there addons?
    /***addon
    double daddtotal = 0.00;
    Json::Value jaddon, jaddarray;
    if (jdetail["det_modifier"]!=Json::nullValue)  {
        jaddarray = jdetail["det_modifier"];
        int j = jaddarray.size ();
        for (int i = 0; i < j; i++)  {
            jaddon = jaddarray[i];
            string addonamt = jaddon["addon_amount"].asString();
            if (!_util->valid_decimal("Add on amount", addonamt.c_str(), 1, 12, 0.00, 999999999))
                return seterrormsg(_util->errormsg());
            double damtadd = stod(addonamt);
            if (damtadd > 0)  {
                tmp  = "  ";
                tmp += jaddon["modifier_desc"].asString();
                tmp += " ";
                tmp += jaddon["modifier_subdesc"].asString();
                tmp += "\n";
                _txtdet += tmp;

                string addonqty = jaddon["addon_quantity"].asString();
                if (!_util->valid_decimal("Add on quantity", addonqty.c_str(), 1, 12, 0.01, 999999999))
                    return seterrormsg(_util->errormsg());
                double damtqty = stod(addonqty);
                double damttot = damtadd * damtqty;
                daddtotal += damttot;

                char szmod1 [64];
                sprintf (szmod1, "  %.02f @ %.02f", damtqty, damtadd);
                char szmod2 [64];
                sprintf (szmod2, "%.02f", (daddtotal + (priceOriginal * quantity)));
                tmp = _util->cyLRText(22, szmod1, 18, szmod2);
                _txtdet += tmp;
            }
        }
    }***/
    //////////////////////////////////
    //  Is the item discounted?
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less ";
        Json::Value jdisc = jdetail["settings_detail_discount"];
        desc += jdisc["description"].asString();
        sprintf (sztmp, "%.02f", (jdetail["item_discount_amount"].asDouble()*1000)/1000);
        tmp = _util->cyLRText(28, desc, 12, sztmp);
        _txtdet += tmp;
    }
    //////////////////////////////////////
    //  Split adjustment
    /*
    if (_totalSplitAdjust > 0)  {
        //sprintf (sztmp, "%.02f", _totalSplitAdjust);
        //_txtdet += _util->cyLRText(23, "Split check adjustment", 18, sztmp);
        string desc  = "  Less split check ";
        sprintf (sztmp, "%.02f", (_totalSplitAdjust*1000)/1000);
        tmp = _util->cyLRText(28, desc, 12, sztmp);
        _txtdet += tmp;
    }*/

    _receipt += _txtdet;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptHeader (Json::Value& jheader,
                                         salesReceiptType receiptType)
{
    _txthdr = "";
    _receipt = "";
    _receiptType = receiptType;
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_widthReceipt;
    string tmp = "", tmp2 = "";
    Json::Value jtemp2, jtemp = jheader["hdr_branch"];
    if (jtemp["hdrmsg1"].asString().length() > 0)  {
      _txthdr += _util->cyCenterText(w, jtemp["hdrmsg1"].asString());
    }
    if (jtemp["hdrmsg2"].asString().length() > 0)  {
      _txthdr += _util->cyCenterText(w, jtemp["hdrmsg2"].asString());
    }
    if (jtemp["hdrmsg3"].asString().length() > 0)  {
      _txthdr += _util->cyCenterText(w, jtemp["hdrmsg3"].asString());
    }
    _txthdr += _util->cyCenterText(w, " ");
    jtemp = jheader["hdr_settings"];
    _txthdr += _util->cyCenterText(w, jtemp["trade_name"].asString());

    jtemp = jheader["hdr_company"];
    _txthdr += _util->cyCenterText(w, jtemp["description"].asString());
    jtemp = jheader["hdr_branch"];
    _txthdr += _util->cyCenterText(w, jtemp["description"].asString());
    _txthdr += _util->cyCenterText(w, jtemp["addr1"].asString());
    _txthdr += _util->cyCenterText(w, jtemp["addr2"].asString());
    jtemp = jheader["hdr_branch"];
    tmp  = "VAT-REG-TIN-";
    tmp += jtemp["tin"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    jtemp = jheader["settings_register"];
    tmp = jtemp["serialno"].asString();
    if (tmp.length() > w + 1) {
      tmp2 = tmp.substr(0, (w - 1));
    } else {
      tmp2 = tmp;
    }
    _txthdr += _util->cyCenterText(w, tmp2);

    jtemp = jheader["hdr_branch"];
    tmp  = "Permit No. ";
    tmp += jtemp["permitno"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    tmp  = "ACCR.# ";
    tmp += jtemp["bir_num"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    jtemp = jheader["hdr_branch"];
    tmp = "STORE # ";
    tmp += jtemp["location_code"].asString();
    jtemp = jheader["settings_register"];
    tmp += "   POS # ";
    tmp += jtemp["register_num"].asString();
    _txthdr += _util->cyCenterText(w, tmp);
    _txthdr += "\n";
    //////////////////////////////////////
    //  Display account(s)
    jtemp = jheader["hdr_trxaccount"];
    if (jtemp["customer_id"].asString().length () >0)  {
        jtemp2 = jheader["settings_transtype"];
        if (jtemp2["is_sc"].asString() == "1" ||
            jtemp2["is_pwd"].asString() == "1")  {
            if (jtemp["account_type_desc"].asString().length() > 0)  {
                _txthdr += "   Account     : ";
                _txthdr += jtemp["account_type_desc"].asString ();
                _txthdr += "\n";
            }
            if (jtemp["account_number"].asString().length() > 0)  {
                _txthdr += "   Card number : ";
                _txthdr += jtemp["account_number"].asString ();
                _txthdr += "\n";
            }
            if (jtemp["last_name"].asString().length() > 0)  {
                _txthdr += "   Name        : ";
                _txthdr += jtemp["first_name"].asString ();
                _txthdr += " ";
                _txthdr += jtemp["last_name"].asString ();
                _txthdr += "\n";
            }
            _txthdr += "   Signature   : ______________________";
            _txthdr += "\n";
        }
    }
    //////////////////////////////////////
    //  Special receipt header(s)
    if (receiptType == salesReceiptType::SRT_REPRINT) {
        _txttype = "reprint";
      _txthdr += _util->cyCenterText(w, " ");
      _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
    }
    if (receiptType == salesReceiptType::SRT_POSTVOID) {
      _txthdr += _util->cyCenterText(w, " ");
      _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
    }
    if (receiptType == salesReceiptType::SRT_SUSPEND) {
      _txthdr += _util->cyCenterText(w, " ");
      _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
    }
    if (receiptType == salesReceiptType::SRT_CANCEL) {
      _txthdr += _util->cyCenterText(w, " ");
      _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
    }
    if (receiptType == salesReceiptType::SRT_REFUND) {
      _txthdr += _util->cyCenterText(w, " ");
      _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
    }  else  {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += _util->cyCenterText(w, "***  SALES INVOICE ***");
    }
    _txthdr += _util->cyCenterText(w, " ");
    //////////////////////////////////////
    //  PAC transaction???
    if (jheader["pac_number"].asString().length() > 0 &&
        jheader["ba_redemption_reference"].asString().length() < 1)  {
        _txthdr += "   PAC number  : ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += "\n";
    }
    //////////////////////////////////////
    //  Beauty addict / PAC???
    if (jheader["pac_number"].asString().length() > 0 &&
        jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += "  Customer Id : ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += "\n";
        _txthdr += "    Date-Time : ";
        _txthdr += _util->date_pos();
        _txthdr += "\n";
    }  else  {
        char sztmp[24];
        jtemp = jheader["settings_register"];
        long lRegister = atol (jtemp["register_num"].asString().c_str ());
        sprintf (sztmp, "%03ld", lRegister);

        if (receiptType == salesReceiptType::SRT_REFUND) {
            _txthdr += "   SI number   : ";
            _txthdr += refundTransnum(jheader);
            _txthdr += "\n";
        }  else  {
            if (receiptType != salesReceiptType::SRT_SUSPEND) {
                _txthdr += "   SI number   : ";
                _txthdr += legacyTransnum(jheader);
                _txthdr += "\n";
            }  else if (receiptType == salesReceiptType::SRT_RESUME) {
                _txthdr += "   Suspend #   : ";
                _txthdr += sztmp;
                _txthdr += "-";
                _txthdr += jheader["transaction_number"].asString ();
                _txthdr += "/n";
            }  else  {
                _txthdr += "   Suspend #   : ";
                _txthdr += sztmp;
                _txthdr += "-";
                _txthdr += _suspendNum;
                _txthdr += "/n";
            }
        }
        _txthdr += "   Date-Time   : ";
        _txthdr += _util->date_pos();
        _txthdr += "\n";
    }
    //////////////////////////////////////
    //  Transaction type
    _txthdr += _util->cyCenterText(w, " ");
    jtemp = jheader["settings_transtype"];
    _txthdr += _util->cyCenterText(w, jtemp["description"].asString());

    if (jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += "Reference Code : ";
        _txthdr += jheader["ba_redemption_reference"].asString();
        _txthdr += "\n";
    }

    _txthdr += _util->cyAcrossText(40, "-");
    _receipt += _txthdr;
    return true;
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptElena::CYRestReceiptElena (CYDbEnv* env, cycgi* cgi) : CYRestCommon (env, cgi)
{
    _env = nullptr;
    _env = new SqliteEnv;
    string rcptdir = _ini->get_value("PATH","RECEIPT");
    rcptdir += "/receipt.qtdb";
    env->connect ("", "", "", rcptdir, 0);
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptElena::CYRestReceiptElena (CYDbEnv* env, cylog* log, cyini* ini, cycgi* cgi)
  : CYRestCommon (env, log, ini, cgi)
{
    _env = nullptr;
    _env = new SqliteEnv;
    string rcptdir = _ini->get_value("PATH","RECEIPT");
    rcptdir += "/receipt.qtdb";
    env->connect ("", "", "", rcptdir, 0);
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYRestReceiptElena::~CYRestReceiptElena ()
{
    resetEodTotals();
    if (nullptr != _env)
        delete (_env);
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
bool CYRestReceiptElena::saveReceipt()
{
    return true;
}
