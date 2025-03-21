//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyrestreceipt.h"
#include "cyrestreceiptelena.h"
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

    if (!assembleReceiptTotals(jheader, receiptType))
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
    string tmp = "";
    if (jheader["hdr_settings"]==Json::nullValue)
        return seterrormsg("Unable to retrieve the POS settings information");

    Json::Value jpos;
    int w = g_widthReceipt;
    jpos = jheader["hdr_settings"];

        Json::Value jtemp;

        jtemp = jheader["hdr_company"];

        //HARDCODED VALUE - NO DETAILS IN API
        _txtfoot += _util->cyCenterText(w, " ");
        _txtfoot += _util->cyCenterText(w, "CYWARE INCORPORATED");
        _txtfoot += _util->cyCenterText(w, "6057 R. Palma Street Poblacion");
        _txtfoot += _util->cyCenterText(w, "Makati, 1210 Metro Manila");
        _txtfoot += _util->cyCenterText(w, "VAT REGTIN:201-140-102-00000");
        _txtfoot += _util->cyCenterText(w, "ACCR.# 049-201140102-0000462151");
        _txtfoot += _util->cyCenterText(w, "EFFECTIVITY DATE 08/01/2020");
        _txtfoot += _util->cyCenterText(w, "VALID UNTIL 07/31/2025");
        _txtfoot += _util->cyCenterText(w, "Date Issued: 05/11/2005");


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
    //FOR SAMPLE ONLY INSERT QR CODE HERE
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    // END OF QR CODE AREA

    if (jpos["szmsg01"].asString().length() > 0)  {
        _txtfoot +=  _util->cyCenterText(w, jpos["szmsg01"].asString());
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
        _txtfoot += _util->cyCenterText(w, " ");
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
    Json::Value jacct,jtemp,jtemp2;
    string tmp3="",tmp2="",newLine = "\n";
    jacct = jheader["hdr_tax_receipt"];
    jtemp = jheader["hdr_trxaccount"];

    jtemp2 = jheader["hdr_trxaccount"];
    tmp3 = jtemp2["first_name"].asString();
    tmp3 += " ";
    tmp3 += jtemp2["last_name"].asString();

    if (jtemp2["first_name"].asString().length() < 1 && jtemp2["last_name"].asString().length() < 1){
        _txtsig += newLine.c_str();
        _txtsig += "Customer name:   _______________________\n";
        _txtsig += "Address:         _______________________\n";
        _txtsig += "TIN:             _______________________\n";
        _txtsig += "Business style:  _______________________\n";
        _txtsig += newLine.c_str();
    } else {
        _txtsig += newLine.c_str();

        if (tmp3.length()+17 <= 40){
            tmp2 = "Customer name:   ";
            tmp2 += tmp3.c_str();
            tmp2 += newLine.c_str();

            _txtsig += tmp2.c_str();
        } else {
            tmp2 = "Customer name:   ";
            tmp2 += jtemp2["first_name"].asString();
            tmp2 += newLine.c_str();
            tmp2 += jtemp2["last_name"].asString();
            tmp2 += newLine.c_str();

            _txtsig += tmp2.c_str();
        }

        jtemp2 = jheader["hdr_tax_receipt"];
        tmp3 = jtemp2["address1"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["address2"].asString();

        if (jtemp2["address1"].asString().length() < 1 && jtemp2["address2"].asString().length() < 1){
            _txtsig += "Address: _______________________________";
            _txtsig += newLine.c_str();
        } else if (tmp3.length() + 9 <=40){
            _txtsig += "Address: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Address: ";
            _txtsig += jtemp2["address1"].asString();
            _txtsig += newLine.c_str();
            _txtsig += jtemp2["address2"].asString();
            _txtsig += newLine.c_str();
        }

        tmp3 = jtemp2["tin"].asString();
        if (tmp3.length() < 1) {
            _txtsig += "TIN : __________________________________";
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "TIN: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        }

        tmp3 = jtemp2["business_style"].asString();

        if (tmp3.length() < 1) {
            _txtsig += "Business style:_________________________";
            _txtsig += newLine.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Business style: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
            _txtsig += newLine.c_str();
        }
    }

    _txtsig += _util->cyAcrossText(40, "-");

    jtemp2 = jheader["header_transtype_discount"];

    if (jtemp["is_senior"].asString() == "1" || jtemp2["discount_code"].asString() == "SENIOR") {

        jtemp2 = jheader["hdr_trxaccount"];
        tmp3 = jtemp2["first_name"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["last_name"].asString();

        _txtsig += _util->cyCenterText(40,"DISCOUNT CLAIM");
        _txtsig += newLine.c_str();
        _txtsig += "Signature: _____________________________\n";
        _txtsig += "Discount: ";

        jtemp2 = jheader["header_transtype_discount"];
        _txtsig += jtemp2["description"].asString();
        _txtsig += newLine.c_str();

        if (tmp3.length()+6 <=40){
            _txtsig += "Name: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Name: ";
            _txtsig += jtemp2["first_name"].asString();
            _txtsig += newLine.c_str();
            _txtsig += jtemp2["last_name"].asString();
            _txtsig += newLine.c_str();
        }
        _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n";
    }

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
    //bool isCharge = false;
    //bool isCreditCard = false;


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
            //left += "(";
            //left += jpay["pay_code"].asString();
            //left += ")";
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
            if (jpay["remarks"].asString().length() > 0)  {
                left  = "    REMARKS  : ";
                left += jpay["remarks"].asString ();
                left += "\n";
                _txtpay += left;
            }
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
        }/*  else  {
            if (totalChangeAmount != 0)  {
                double dtmp = totalChangeAmount * -1;
                sprintf (sztmp, "%.02f", dtmp);
                _txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
            }
        }*/
    }

    _txtpay += "\n";
    sprintf (sztmp, "%.02f", _fnbFood);
    _txtpay += _util->cyLRText(23, "Total Food", 18, sztmp);
    sprintf (sztmp, "%.02f", _fnbBeverage);
    _txtpay += _util->cyLRText(23, "Total Beverage", 18, sztmp);

    int w = g_widthReceipt;
    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%ld-item(s)", (long)_totalItems);
    _txtpay += _util->cyCenterText(w, sztmp);
    _txtpay += _util->cyCenterText(w, " ");



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
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptElena::assembleReceiptTotals (Json::Value& jheader,
                                                salesReceiptType receiptType)
{
    _txttot = "";
    string tmp2 = "", tmp = "";
    int w = g_widthReceipt;
    //////////////////////////////////////
    //  Prep work
    string trxDiscountDesc = "";
    if (_totalTrxDiscount > 0)  {
        Json::Value jtrx = jheader["settings_transaction_discount"];
        trxDiscountDesc = jtrx["description"].asString ();
    }
    //////////////////////////////////////
    //  Detail total(s)
    _txttot += _util->cyAcrossText(w, "=");

    //TOTAL SALES AMOUNT
    if (receiptType != SRT_POSTVOID){
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
        else
            tmp = FMTNumberComma(_totalGrossAmount);
    } else {
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
        else
            tmp = FMTNumberComma(_totalGrossAmount);
    }

    if (receiptType == SRT_REFUND)  {
        tmp2 = "-";
        tmp2 += tmp;
    }  else
        tmp2 = tmp;
    _txttot += _util->cyLRText(23, "Total Sales Amount", 18, tmp2.c_str());
    //////////////////////////////////////
    //  Sequence change based on type
    char prefix[2];

    if (receiptType == SRT_REFUND)  {
        strcpy (prefix,"-");
    }  else  {
        strcpy(prefix,"");
    }


    if (_isPwd || _isSenior)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp);
        }

        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtExempt != 0.00)  {
            tmp = FMTNumberComma(_totalAmtExempt);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else if (_isZeroRated)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtZero != 0.00)  {
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtVat);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());

        //  VAT exempt
        if (_totalAmtZero > 0)  {
            //  VAT exempt
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }  else  {
            tmp = FMTNumberComma(_totalAmtVat);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }
    _receipt += _txttot;

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
    int multiplier = 1;
    if (type == salesReceiptType::SRT_POSTVOID)
        multiplier = -1;
    char sztmp[32];
    string tmp2 = "";
    double quantity = stodsafe(jdetail["quantity"].asString())*multiplier;
    double priceOriginal = stodsafe(jdetail["retail_price"].asString())*multiplier;


    //////////////////////////////////////
    //  Display item detail(s)
    //FORMAT SUFFIX
    string itemSuffix;
    if (jdetail["vat_exempt"].asString () == "true")
        itemSuffix = "NV";
    else  {
        if (jhdrtrx["is_sc"].asString() == "1" ||
                jhdrtrx["is_pwd"].asString() == "1" ||
                jhdrtrx["is_zero_rated"].asString() == "1")  {
            itemSuffix = "NV";
        }  else  {
            itemSuffix = "T";
        }
    }

    if (stodsafe(jdetail["gift_wrap_quantity"].asString()) > 0)
        _isGiftWrap = true;

    //////////////////////////////////////
    //  Display line detail(s)
    string tmp;
    Json::Value jproduct = jdetail["detail_product_header"];
    sprintf (sztmp, "%0.2f", quantity);

    tmp  = sztmp;
    tmp += "  ";
    tmp += _util->string_cleanse(jdetail["description"].asString().c_str (),true);

    sprintf (sztmp, "%0.2f", priceOriginal*quantity);
    _txtdet += _util->cyLRText(29,tmp.c_str(),12 , sztmp);
    //////////////////////////////////////
    //  Item quantity / price
    if (jdetail["description"].asString() == "SPLIT CHECK ADJUSTMENT")  {
        sprintf (sztmp, "%.02f", priceOriginal * quantity);
        tmp = _util->cyLRText(23, jdetail["description"].asString(), 18, sztmp);
        _txtdet += tmp;
    }

    //////////////////////////////////////
    //  Item quantity / price
    if (jhdrtrx["is_sc"].asString() == "1") {
        string uom;
        if (jdetail["uom_code"].asString().length() < 1) {
            uom = "";
        } else {
            uom = jdetail["uom_code"].asString();
        }
        string itemSuffix;
        if (jdetail["vat_exempt"].asString () == "true")
            itemSuffix = "NV";
        else {
            if (jhdrtrx["is_sc"].asString() == "1" ||
                    jhdrtrx["is_pwd"].asString() == "1" ||
                    jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "NV";
            }  else  {
                itemSuffix = "V";
            }
        }
    }
    //////////////////////////////////
    //  Is the item discounted?
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less ";
        Json::Value jdisc = jdetail["settings_detail_discount"];
        desc += jdisc["description"].asString();
        string tmpdisc = jdetail["item_discount_amount"].asString();
        if (tmpdisc.length() < 1) tmpdisc = "0";
        double disctemp = stod(tmpdisc);
        if (disctemp > 0.00)  {
            sprintf (sztmp, "%.02f", (disctemp*1000)/1000);
            tmp = _util->cyLRText(29, desc, 12, sztmp);
            _txtdet += tmp;
        }
    }
    //////////////////////////////////////
    //  Split adjustment
    if (_totalSplitAdjust > 0)  {
        //sprintf (sztmp, "%.02f", _totalSplitAdjust);
        //_txtdet += _util->cyLRText(23, "Split check adjustment", 18, sztmp);
        string desc  = "  Less split check ";
        sprintf (sztmp, "%.02f", (_totalSplitAdjust*1000)/1000);
        tmp = _util->cyLRText(28, desc, 12, sztmp);
        _txtdet += tmp;
    }
    //////////////////////////////////////
    //  Tally FNB / BEVERAGE
    double dFnbTally = 0.00;
    double dprice = stod(jdetail["retail_price_adjusted"].asString());
    double dqty = stod(jdetail["quantity"].asString());
    if (dqty < 1.00)
        dFnbTally = dprice;
    else
        dFnbTally = dqty * dprice;
    if (jdetail["printer_code"].asString() == "KB")  {
        _fnbBeverage += dFnbTally;
    }  else  {
        _fnbFood += dFnbTally;
    }

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
    string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
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

    // full address
    fullAddress  = jtemp["addr1"].asString();
    if (!jtemp["addr2"].isNull())  {
        fullAddress += ", ";
        fullAddress += jtemp["addr2"].asString();
    }
    fullAddress = fullAddress.substr(0, 39);

    _txthdr += _util->cyCenterText(w, fullAddress.c_str());
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

    //CHECK IF CHARGE PAYMENT
    Json::Value jtmp, jpay,jpayment;
    jpayment = jheader["pay_sales"];
    size_t j = jpayment.size();
    bool isCharge = false;
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];

        if (jpay["tendertype"] == Json::nullValue)
            isCharge=false;
        jtmp = jpay["tendertype"];
        isCharge = jtmp["is_charge"].asString() == "1";
        if (isCharge)
            break;
    }

    //////////////////////////////////////
    //  Special receipt header(s)
    if (!isCharge) {
        if (receiptType == salesReceiptType::SRT_REPRINT) {
            _txttype = "reprint";
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
        }
        else if (receiptType == salesReceiptType::SRT_POSTVOID) {
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
        }
        else if (receiptType == salesReceiptType::SRT_SUSPEND) {
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
        }
        else if (receiptType == salesReceiptType::SRT_CANCEL) {
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
        }
        else if (receiptType == salesReceiptType::SRT_REFUND) {
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
        }  else  {
            _txthdr += _util->cyCenterText(w, " ");
            _txthdr += _util->cyCenterText(w, "***  OFFICIAL RECEIPT ***");
        }
    } else {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += _util->cyCenterText(w, "This serves as your Provisional Receipt");
        _txthdr += _util->cyCenterText(w, "for the account of");
    }

    _txthdr += _util->cyCenterText(w, " ");

    //////////////////////////////////////
    //  MEMBER INFORMATION DISPLAYS ONLY WHEN CHARGE
    if (isCharge) {
        jtemp2 = jheader["hdr_trxaccount"];
        tmp2 = "   Member Name : ";
        tmp3 = jtemp2["first_name"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["last_name"].asString();
        _txthdr += tmp2.c_str();
        _txthdr += _util->cyCenterText(w-tmp2.length(), tmp3.c_str());

        tmp2 = "   Member ID No. : ";
        tmp3 = jtemp2["account_number"].asString();
        _txthdr += tmp2.c_str();
        _txthdr += _util->cyCenterText(w-tmp2.length(), tmp3.c_str());
        _txthdr += newLine.c_str();
    }
    //////////////////////////////////////
    //  PAC transaction???
    if (jheader["pac_number"].asString().length() > 0 &&
            jheader["ba_redemption_reference"].asString().length() < 1)  {
        _txthdr += "PAC number: ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += newLine.c_str();
    }
    //////////////////////////////////////
    //  Beauty addict / PAC???
    if (jheader["pac_number"].asString().length() > 0 &&
            jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += "  Customer Id : ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += newLine.c_str();
        _txthdr += "    Date-Time : ";
        _txthdr += _util->date_pos();
        _txthdr += newLine.c_str();
    }  else  {
        char sztmp[24];
        jtemp = jheader["settings_register"];
        long lRegister = atol (jtemp["register_num"].asString().c_str ());
        sprintf (sztmp, "%03ld", lRegister);

        if (receiptType == salesReceiptType::SRT_REFUND) {
            tmp2 = "SI number : ";
            tmp3 = refundTransnum(jheader).c_str();
            _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());

        }  else  {
            if (receiptType != salesReceiptType::SRT_SUSPEND &&
                receiptType == salesReceiptType::SRT_RESUME) {
                tmp2 = "SI number : ";
                tmp3 = legacyTransnum(jheader).c_str();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType == salesReceiptType::SRT_RESUME) {

                tmp2 = "Suspend # : ";
                tmp2 += sztmp;
                tmp2 += "-";
                tmp3 = jheader["transaction_number"].asString ();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {

                tmp2 = "Reference#: ";
                tmp2 += sztmp;
                tmp2 += "-";
                tmp3 = jheader["transaction_number"].asString ();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else  {
                tmp2 = "Suspend # : ";
                tmp2 += sztmp;
                tmp2 += "-";
                //tmp3 = _suspendNum;
                long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
                sprintf (sztmp, "%05ld", ltrx);
                tmp2 += sztmp;
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }
        }
        tmp2 = "Date-Time : ";
        tmp3 = _util->date_pos_elena();
        _txthdr += _util->cyLRText(15,tmp2.c_str(),26,tmp3.c_str());
    }

    //////////////////////////////////////
    //CASHIER/TABLE/SERVER INFORMATION
    Json::Value jcashier;
    if (jheader["settings_cashier"] == Json::nullValue)
        return seterrormsg("Unable to get the cashier details");
    jcashier = jheader["settings_cashier"];
    jtemp = jheader["hdr_fnb"];

    size_t len = jcashier["first_name"].asString().length();
    len += jcashier["last_name"].asString().length();
    len += 11;

    len += jtemp["tabledesc"].asString().length();
    len += 8;

    if (len > 40)  {
        tmp2 = "Cashier : ";
        tmp3  = jcashier["first_name"].asString();
        tmp3 += " ";
        tmp3 += jcashier["last_name"].asString();
        _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());

        tmp2 = "Table   : ";
        if (jtemp["tabledesc"].asString().length() > 5)
            tmp3 = jtemp["tabledesc"].asString();
        else
            tmp3 = "";
        _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());

    }  else  {
        tmp2 = "Cashier : ";
        tmp2 += jcashier["first_name"].asString();
        tmp2 += " ";
        tmp2 += jcashier["last_name"].asString();

        tmp3  = "Table : ";
        len = jtemp["tabledesc"].asString().length();
        if (len > 5)
            tmp3 += jtemp["tabledesc"].asString().substr(6,len-6);
        else
            tmp3 += "";
        _txthdr += _util->cyLRText(29,tmp2.c_str(),12,tmp3.c_str());

    }

    tmp2   = "Server  : ";
    tmp3   = jtemp["first_name"].asString();
    tmp3  +=  " ";
    tmp3  += jtemp["last_name"].asString();
    _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
    _txthdr += newLine.c_str();

    if (receiptType == SRT_REFUND)  {
        _txthdr += "Txn No.      : ";
        _txthdr += jheader["systransnum"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Business Date: ";
        _txthdr += jheader["systransnum"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Orig Pos No  : ";
        _txthdr += jheader["orig_register_number"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Orig Bus Date: ";
        _txthdr += jheader["orig_transaction_date"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Original SI# : ";
        _txthdr += jheader["orig_systransnum"].asString();
        _txthdr += newLine.c_str();
    }

    //////////////////////////////////////
    //  Transaction type
    _txthdr += _util->cyCenterText(w, " ");

    if (jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += "Reference Code : ";
        _txthdr += jheader["ba_redemption_reference"].asString();
        _txthdr += newLine.c_str();
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
