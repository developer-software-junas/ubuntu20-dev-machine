//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptElena::assembleReceiptPayment (Json::Value& jheader,
                                                 Json::Value& jpayment,
                                                 salesReceiptType receiptType)
{
    string tmp;
    char sztmp[32];
    bool isGc = false;

    size_t  w = g_widthReceipt;
    int wleft = 23, wright = 18;
    if (_isSkinny)   {
        w = 30;
        wleft = 19;
        wright = 12;
    }

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
        double paymentAmount = _util->stodsafe(jpay["payment_amount"].asString());
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
                double discountValue = _util->stodsafe(jdiscount["discount_value"].asString ());
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
        //isCharge = false;
        //isCreditCard = false;

        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isGc = jtmp["is_gc"].asString() == "1";
        //isCheck = jtmp["is_check"].asString() == "1";
        //isCharge = jtmp["is_charge"].asString() == "1";
        //isDebit = jtmp["is_debit_card"].asString() == "1";
        //isCreditCard = jtmp["is_credit_card"].asString() == "1";
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
                string fmtright = FMTNumberComma(atof(sztmp));
                _txtpay += _util->cyLRText(wleft, jpay["description"].asString(), wright, fmtright);
                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                    gc.clear ();
                    gc = jgc[i];
                    _txtpay += gc["description"].asString();
                    //_txtpay += "\n";
                    _txtpay += _util->cyLRText(wleft, "  GC SERIES", wright, gc["gcnum"].asString());

                    double dtmp = _util->stodsafe(gc["amount_paid"].asString());
                    sprintf (sztmp, "%.02f", dtmp);
                    string fmtright = FMTNumberComma(atof(sztmp));
                    _txtpay += _util->cyLRText(wleft, "  OLD BALANCE", wright, fmtright);
                    dtmp = _util->stodsafe(gc["remaining_balance"].asString());
                    sprintf (sztmp, "%.02f", dtmp);
                    fmtright = FMTNumberComma(atof(sztmp));
                    _txtpay += _util->cyLRText(wleft, "  NEW BALANCE", wright, fmtright);

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
            //left += "(";
            //left += jpay["pay_code"].asString();
            //left += ")";
            string fmtright = FMTNumberComma(atof(sztmp));
            _txtpay += _util->cyLRText(wleft, left, wright, fmtright);

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
            if (jpay["remarks"].asString().length() > 0)  {
                left  = "    REMARKS  : ";
                left += jpay["remarks"].asString ();
                left += "\n";
                _txtpay += left;
            }
        }
    }

    jheader["cytotal_payment"]=totalPaymentAmount;
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
    double totalChangeAmount = totalPaymentAmount - (absNet+_totalAmtVat);
    //  GC has no change
    if (isGc && totalChangeAmount > 0)
        totalChangeAmount = 0.00;
    if (totalChangeAmount < 0.05)
        totalChangeAmount = 0.00;

    jheader["cytotal_change"]=totalChangeAmount;
    if (receiptType != salesReceiptType::SRT_REFUND)  {
        if (totalChangeAmount > 0) {
            if (receiptType == SRT_POSTVOID)
                sprintf (sztmp, "-%.02f", (totalChangeAmount*1000)/1000);
            else
                sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
            string fmtright = FMTNumberComma(atof(sztmp));
            _txtpay += _util->cyLRText(wleft, "Change", wright, fmtright);
        }  else  {
            if (totalChangeAmount != 0)  {
                double dtmp = totalChangeAmount * -1;
                sprintf (sztmp, "%.02f", dtmp);
                //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
            }
        }
    }
    //////////////////////////////////////
    //  Food and beverage total
    _txtpay += "\n";
    if (_fnbFood < 0)
        _fnbFood = _fnbFood * -1;
    sprintf (sztmp, "%.02f", _fnbFood);
    string fmtright = FMTNumberComma(atof(sztmp));
    _txtpay += _util->cyLRText(wleft, "Total Food", wright, fmtright);
    if (_fnbBeverage < 0)
        _fnbBeverage = _fnbBeverage * -1;
    sprintf (sztmp, "%.02f", _fnbBeverage);
    fmtright = FMTNumberComma(atof(sztmp));
    _txtpay += _util->cyLRText(wleft, "Total Beverage", wright, fmtright);

    Json::Value jfnb = jheader["hdr_fnb"];
    if (jfnb != Json::nullValue)  {
        _txtpay += "\n";
        string tmp = jfnb["senior_count"].asString();
        if (tmp.empty()) tmp = "0";
        int senior_count = atoi(tmp.c_str ());

        tmp = jfnb["customer_count"].asString();
        if (tmp.empty()) tmp = "0";
        int customer_count = atoi(tmp.c_str ());

        if (_fnbFood < 0)
            _fnbFood = _fnbFood * -1;
        if (senior_count > 0)  {
            sprintf (sztmp, "%d", senior_count);
            _txtpay += _util->cyLRText(wleft, "Senior Customer(s)", wright, sztmp);
            sprintf (sztmp, "%d", customer_count);
            _txtpay += _util->cyLRText(wleft, "Total Customer(s)", wright, sztmp);
        }
    }


    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%.02f-item(s)", _totalItems);
    _txtpay += _util->cyCenterText(w, sztmp);
    //_txtpay += _util->cyCenterText(w, " ");



    /*Json::Value jcashier;
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
    }*/

    _receipt += _txtpay;
    return true;
}
