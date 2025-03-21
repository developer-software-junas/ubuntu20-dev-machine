//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRcc::assembleReceiptPayment (Json::Value& jheader,
                                           Json::Value& jpayment,
                                           salesReceiptType receiptType)
{
    string tmp;
    char sztmp[32];
    bool isGc = false;
    bool isEgc = false;
    bool isCheck = false;
    bool isCharge = false;
    bool isCreditCard = false;


    _isCash = false;
    _isEpurse = false;
    _isDrawer = false;
    _isResRma = false;
    _resRmaNumber = "";


    _txtpay = "";
    Json::Value jtmp, jpay;
    double totalPaymentAmount = 0.00;
    size_t j = jpayment.size();

    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];

        tmp = jpay["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, -99999999, 999999999))
            return seterrormsg(_util->errormsg());
        double paymentAmount = stodsafe(jpay["payment_amount"].asString());
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
        totalPaymentAmount += paymentAmount;
        //////////////////////////////////////
        //  Special payment information
        //isGc = false;
        //isCheck = false;
        //isDebit = false;
        //isCharge = false;
        //isCreditCard = false;

        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isGc = jtmp["is_gc"].asString() == "1";
        isEgc = jtmp["is_egc"].asString() == "1";
        isCheck = jtmp["is_check"].asString() == "1";
        isCharge = jtmp["is_charge"].asString() == "1";
        _isResRma = jtmp["is_res"].asString() == "1" ||
                    jtmp["is_rma"].asString() == "1";
        isCreditCard = jtmp["is_credit_card"].asString() == "1";

        _isCash = jtmp["is_cash"].asString() == "1";
        if (!_isEpurse)
            _isEpurse = jtmp["is_epurse"].asString() == "1";
        _isDrawer = jtmp["is_drawer"].asString() == "1";
        ////////////////////////////////////
        //  GC special processing
        if (isGc) {
            if (jpay["gc"] == Json::nullValue)
                return seterrormsg("Unable to process the GC payment information");

            _isSigLine = true;
            Json::Value gc, jgc = jpay["gc"];
            ////////////////////////////
            //  GC payment(s)
            if (isGc) {
                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());
                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                    gc.clear ();
                    gc = jgc[i];
                    _txtpay += gc["description"].asString();
                    //_txtpay += "\n";
                    _txtpay += _util->cyLRText(23, "  GC SERIES", 18, gc["gcnum"].asString());

                    double dlimit = stodsafe(gc["original_balance"].asString());
                    tmp = FMTNumberComma(dlimit);
                    _txtpay += _util->cyLRText(23, "  OLD BALANCE", 18, tmp.c_str());

                    double dpaid = stodsafe(gc["amount_paid"].asString());
                    double dbalance = dlimit - dpaid;
                    if (dbalance < 0.00) dbalance *= -1;
                    tmp = FMTNumberComma(dbalance);
                    _txtpay += _util->cyLRText(23, "  NEW BALANCE", 18, tmp.c_str());

                    if (dbalance > 0.00)  {
                        Json::Value jgcrefund = tmp;
                    }
                }
            }
        }  else if (isCharge)  {
            if (jpay["pay_code"].asString() == "YCC")  {
                _isSigLine = true;
                _isSigText = true;
                double yccpay, yccbalance;
                yccpay = _util->stodsafe(jpay["payment_amount"].asString());
                yccbalance = _util->stodsafe(jpay["credit_balance"].asString());

                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());


                tmp  = "       YCC# : ";
                tmp += jpay["account_number"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "       NAME : ";
                tmp += jpay["first_name"].asString();
                tmp += " ";
                tmp += jpay["middle_name"].asString();
                tmp += " ";
                tmp += jpay["last_name"].asString();
                tmp += "\n";
                _txtpay += tmp;


                tmp  = "OLD BALANCE : ";
                sprintf (sztmp, "%.02f\n", yccbalance);
                tmp += sztmp;
                _txtpay += tmp;

                tmp  = "NEW BALANCE : ";
                sprintf (sztmp, "%.02f\n", yccbalance - yccpay);
                tmp += sztmp;
                _txtpay += tmp;
            }  else if (jpay["pay_code"].asString() == "HC_R")  {
                _isSigLine = true;
                _isSigText = true;
                string slipNum = jpay["approval_merchant"].asString();
                //double yccpay, yccbalance;
                //yccpay = _util->stodsafe(jpay["payment_amount"].asString());
                //yccbalance = _util->stodsafe(jpay["credit_balance"].asString());
                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());

                tmp  = "      CARD# : ";
                tmp += jpay["account_number"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "       NAME : ";
                tmp += jpay["first_name"].asString();
                tmp += " ";
                tmp += jpay["middle_name"].asString();
                tmp += " ";
                tmp += jpay["last_name"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "      SLIP# : ";
                tmp += slipNum;
                tmp += "\n";
                _txtpay += tmp;
/*
                tmp  = "OLD BALANCE : ";
                sprintf (sztmp, "%.02f\n", yccbalance);
                tmp += sztmp;
                _txtpay += tmp;

                tmp  = "NEW BALANCE : ";
                sprintf (sztmp, "%.02f\n", yccbalance - yccpay);
                tmp += sztmp;
                _txtpay += tmp;
*/
            }  else if (jpay["pay_code"].asString() == "GAR")  {
                _isSigLine = true;
                _isSigText = true;
                string slipNum = jpay["approval_merchant"].asString();
                double yccpay, yccbalance;
                yccpay = _util->stodsafe(jpay["payment_amount"].asString());
                yccbalance = _util->stodsafe(jpay["credit_balance"].asString());

                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());

                tmp  = "      CARD# : ";
                tmp += jpay["account_number"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "       NAME : ";
                tmp += jpay["first_name"].asString();
                tmp += " ";
                tmp += jpay["middle_name"].asString();
                tmp += " ";
                tmp += jpay["last_name"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "       GRC# : ";
                tmp += jpay["approval_code"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "    BALANCE : ";
                sprintf (sztmp, "%.02f\n", yccbalance - yccpay);
                tmp += sztmp;
                _txtpay += tmp;

                tmp  = "      SLIP# : ";
                tmp += slipNum;
                tmp += "\n";
                _txtpay += tmp;

            }
        }  else if (isCheck)  {
            if (jpay["pay_code"].asString() == "CAAC")  {
                _isSigLine = true;
                _isSigText = true;
                string dateCheck = jpay["approval_terms"].asString();
                string bankCode = jpay["approval_merchant"].asString();
                string approval_code = jpay["approval_code"].asString();
                string checkNumber = jpay["change_currency_code"].asString();

                if (dateCheck.length() < 1)
                    dateCheck = _util->date_eight();

                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());

                tmp  = "   CAAC #     : ";
                tmp += jpay["account_number"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "     NAME     : ";
                tmp += jpay["first_name"].asString();
                tmp += " ";
                tmp += jpay["middle_name"].asString();
                tmp += " ";
                tmp += jpay["last_name"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "    CHECK     : ";
                tmp += checkNumber;
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "BANK CODE     : ";
                tmp += bankCode;
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "    DATED     : ";
                tmp += dateCheck;
                tmp += "\n";
                _txtpay += tmp;
                tmp += "\n\n\n";

                tmp  = "APPROVAL CODE : ";
                tmp += approval_code;
                tmp += "\n";
                _txtpay += tmp;
            }
        }  else if (isEgc)  {
            _isSigLine = true;
            tmp = FMTNumberComma(paymentAmount);

            string left = "";
            left += jpay["description"].asString();

            _txtpay += _util->cyLRText(23, left, 18, tmp.c_str());

            tmp  = "       CARD : ";
            tmp += jpay["account_number"].asString();
            tmp += "\n";
            _txtpay += tmp;

        }  else if (_isResRma)  {
/***
            if (jpay["pay_code"].asString() == "RMA")  {
                _isSigText = true;
                _isSigLine = true;
            }  else if (jpay["pay_code"].asString() == "RES")  {
                _isExtra = true;
            }
***/
            if (receiptType != SRT_POSTVOID)  {
                if (totalPaymentAmount < 0.00) totalPaymentAmount = totalPaymentAmount * -1;
            }
            tmp = FMTNumberComma(totalPaymentAmount);
            _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());

            tmp  = "     NAME : ";
            tmp += jpay["first_name"].asString();
            tmp += " ";
            tmp += jpay["middle_name"].asString();
            tmp += " ";
            tmp += jpay["last_name"].asString();
            tmp += "\n";
            _txtpay += tmp;

            //////////////////////////////
            //  Hide the reference number
            //  for RCC
            tmp  = "  REF NO. : ";
            tmp += jpay["approval_code"].asString();
            tmp += "\n";
            //_txtpay += tmp;

            _resRmaNumber = jpay["approval_code"].asString();
        }  else if (jpay["pay_code"].asString() == "HC_R")  {
                _isSigLine = true;
                _isSigText = true;
                string slipNum = jpay["approval_merchant"].asString();
                //double yccpay, yccbalance;
                //yccpay = _util->stodsafe(jpay["payment_amount"].asString());
                //yccbalance = _util->stodsafe(jpay["credit_balance"].asString());
                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());

                tmp  = "      CARD# : ";
                tmp += jpay["account_number"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "       NAME : ";
                tmp += jpay["first_name"].asString();
                tmp += " ";
                tmp += jpay["middle_name"].asString();
                tmp += " ";
                tmp += jpay["last_name"].asString();
                tmp += "\n";
                _txtpay += tmp;

                tmp  = "      SLIP# : ";
                tmp += slipNum;
                tmp += "\n";
                _txtpay += tmp;
        }  else  {

            tmp = FMTNumberComma(paymentAmount);

            string left = "";
            left += jpay["description"].asString();

            _txtpay += _util->cyLRText(23, left, 18, tmp.c_str());

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

    //////////////////////////////////////
    //  Net sales amount
    double netSalesAmount = _totalGrossAmount -
            (_totalItemDiscount+_totalTrxDiscount+_totalAmtExempt+_totalAmtZero);
    //////////////////////////////////////
    //  Calculate the change
    double absNet = netSalesAmount;
    if (absNet < 0)
        absNet = netSalesAmount * -1;
    double totalChangeAmount = totalPaymentAmount - absNet;
    //  GC has no change
    if (isGc && totalChangeAmount > 0) totalChangeAmount = 0.00;
    sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
    totalChangeAmount = atof(sztmp);
    jheader["cytotal_change"]=totalChangeAmount;
    if (receiptType != salesReceiptType::SRT_REFUND)  {
        if (totalChangeAmount > 0) {
            if (receiptType == SRT_POSTVOID)
                sprintf (sztmp, "-%.02f", (totalChangeAmount*1000)/1000);
            _txtpay += _util->cyLRText(23, "Change", 18, sztmp);
        }  else  {
            if (totalChangeAmount != 0)  {
                double dtmp = totalChangeAmount * -1;
                sprintf (sztmp, "%.02f", dtmp);
                //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
            }
            if (_isCash)
                _txtpay += _util->cyLRText(23, "Change", 18, "0.00");
        }
    }

    //CONSTRUCT TERMS FOR CREDIT CARD PAYMENT
    if (isCreditCard) {
        jpay = jpayment[0];
        string strterms = jpay["approval_terms"].asString();

        switch (atoi(strterms.c_str())) {
        case 1:
            tmp = "3 Months";
            break;
        case 2:
            tmp = "6 Months";
            break;
        case 3:
            tmp = "9 Months";
            break;
        case 4:
            tmp = "12 Months";
            break;
        case 5:
            tmp = "18 Months";
            break;
        case 6:
            tmp = "24 Months";
            break;
        case 7:
            tmp = "36 Months";
            break;
        default:
            tmp = "Straight";
            break;
        }
        _txtpay += "    TERMS : ";
        _txtpay += tmp.c_str();
        _txtpay += _util->cyCenterText(40, " ");
    }

    int w = g_widthReceipt;
    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%0.2f-item(s)",_totalItems);
    _txtpay += _util->cyCenterText(w, sztmp);
    _txtpay += _util->cyCenterText(w, " ");


    _receipt += _txtpay;
    return true;
}
