//*******************************************************************
//        FILE:     cyrestsales_fspurl.cpp
// DESCRIPTION:     FSP url generation
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleFsp
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspUrl (Json::Value& jheader,
                                    Json::Value& jdetail,
                                    Json::Value& jpayment)
{
    if (!assembleFspHeader(jheader))
        return false;

    Json::Value jline;
    int j = jdetail.size ();

    _fspurl.item_type = "";
    _fspurl.item_code = "";
    _fspurl.item_amount = "";
    _fspurl.item_quantity = "";
    _fspurl.category_cd = "";
    _fspurl.is_beauty = "";

    for (int i = 0; i < j; i++)  {
        jline = jdetail[i];
        if (!assembleFspDetail(jline))
            return false;
        jdetail[i]=jline;
        if ((i+1) < j)  {
            _fspurl.is_beauty += "|";
            _fspurl.item_type += "|";
            _fspurl.item_code += "|";
            _fspurl.item_amount += "|";
            _fspurl.item_quantity += "|";
            _fspurl.category_cd += "|";
        }
    }

    if (!assembleFspTotals(jheader))
        return false;

    if (!assembleFspPayment(jpayment))
        return false;

    if (!assembleFspAccount(jheader))
        return false;


    _fspurl.args  = "state=state_postrans&card_number=";
    _fspurl.args += _fspurl.card_number;
    _fspurl.args += "&amount=";
    _fspurl.args += _fspurl.amount;
    _fspurl.args += "&trans_no=";
    _fspurl.args += _fspurl.trans_no;
    _fspurl.args += "&company_cd=";
    _fspurl.args += _fspurl.company_cd;
    _fspurl.args += "&branch_cd=";
    _fspurl.args += _fspurl.branch_cd;
    _fspurl.args += "&pos_no=";
    _fspurl.args += _fspurl.pos_no;
    _fspurl.args += "&trans_cd=";
    _fspurl.args += _fspurl.trans_cd;
    _fspurl.args += "&tender_cd=";
    _fspurl.args += _fspurl.tender_cd;
    _fspurl.args += "&tender_amount=";
    _fspurl.args += _fspurl.tender_amount;
    _fspurl.args += "&item_code=";
    _fspurl.args += _fspurl.item_code;
    _fspurl.args += "&category_cd=";
    _fspurl.args += _fspurl.category_cd;
    _fspurl.args += "&item_quantity=";
    _fspurl.args += _fspurl.item_quantity;
    _fspurl.args += "&item_amount=";
    _fspurl.args += _fspurl.item_amount;
    _fspurl.args += "&item_type=";
    _fspurl.args += _fspurl.item_type;
    _fspurl.args += "&beauty_card=";
    _fspurl.args += _fspurl.beauty_card;
    _fspurl.args += "&is_beauty=";
    _fspurl.args += _fspurl.is_beauty;

    return true;
}
//*******************************************************************
//    FUNCTION:     assembleFspAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspAccount (Json::Value& jheader)
{
    string tmp;
    bool isFsp = false;
    bool isBeauty = false;
    Json::Value jacct, jaccount;

    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    int j = jaccount.size();
    for (int i = 0; i < j; i++)  {
        isFsp = false;
        isBeauty = false;
        jacct = jaccount[i];
        if (jacct["is_loyalty"].asString() == "1") {
          isFsp = true;
        } else if (jacct["is_beauty_addict"].asString() == "1") {
          isBeauty = true;
        }
        if (isFsp)  {
            tmp = jacct["epurse_balance"].asString();
            _fspurl.epurse = tmp;
            tmp = jacct["account_number"].asString();
            _fspurl.card_number = tmp;

            Json::Value jhttp;
            if (jheader["settings_account_FSP"] != Json::nullValue)  {
                jhttp = jheader["settings_account_FSP"];
                _fspurl.port  = jhttp["rest_port"].asString();
                _fspurl.path  = jhttp["rest_path"].asString();
                _fspurl.host  = jhttp["rest_host"].asString();
                _fspurl.host += ".";
                _fspurl.host += jhttp["rest_domain"].asString();
            }
        }
        if (isBeauty)  {
            tmp = jacct["account_number"].asString();
            _fspurl.beauty_card = tmp;

            Json::Value jhttp;
            if (jheader["settings_account_BA"] != Json::nullValue)  {
                jhttp = jheader["settings_account_FSP"];
                _fspurl.port  = jhttp["rest_port"].asString();
                _fspurl.path  = jhttp["rest_path"].asString();
                _fspurl.host  = jhttp["rest_host"].asString();
                _fspurl.host += ".";
                _fspurl.host += jhttp["rest_domain"].asString();
            }

        }
    }
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleFspPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspPayment (Json::Value& jpayment)
{
    string tmp;
    char sztmp[64];

    _fspurl.tender_cd = "";
    _fspurl.tender_amount = "";

    Json::Value jpay;
    double totalPaymentAmount = 0.00;
    size_t j = jpayment.size();
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];
        tmp = jpay["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, -999999999, 999999999))
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
        //////////////////////////////////
        //  Check for change amount
        double dchange;
        tmp = jpay["change_amount"].asString ();
        if (_util->stodsafe(tmp) > 0.00)  {
            dchange = 0.00;
            Json::Value jtender;
            jtender = jpay["tendertype"];
            if (jtender!=Json::nullValue)  {
                if (jtender["allow_change"].asString() == "1")  {
                    dchange = _util->stodsafe(tmp);
                }
            }
        }  else
            dchange = 0.00;


        totalPaymentAmount += (paymentAmount - dchange);
        sprintf(sztmp, "%.02f", (paymentAmount - dchange));
        _fspurl.tender_cd += jpay["pay_code"].asString();
        _fspurl.tender_amount += sztmp;

        if ((i+1) < j)  {
            _fspurl.tender_cd += "|";
            _fspurl.tender_amount += "|";
        }
    }
    sprintf(sztmp, "%.02f", totalPaymentAmount);
    _fspurl.amount = sztmp;

    return true;
}
//*******************************************************************
//    FUNCTION:     assembleFspDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspTotals (Json::Value& jheader)
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

    receipt += "  SYSDATE : ${cyNiceDate(sales.getLogicalDate())}\n";*/
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleFspDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspDetail (Json::Value& jdetail)
{    
    _fspurl.item_code += jdetail["item_code"].asString();
    _fspurl.is_beauty += jdetail["is_beauty"].asString();
    _fspurl.item_quantity += jdetail["quantity"].asString();
    _fspurl.item_amount += jdetail["retail_price_adjusted"].asString();

    Json::Value jdetprod;
    if (jdetail["detail_product_header"]!=Json::nullValue)  {
        jdetprod = jdetail["detail_product_header"];
        _fspurl.item_type += jdetprod["base_type_code"].asString();
        _fspurl.category_cd += jdetprod["category_cd"].asString();
    }

    return true;
}
//*******************************************************************
//    FUNCTION:     assembleFspHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceipt::assembleFspHeader (Json::Value& jheader)
{
    //////////////////////////////////////
    //  Get header information
    _fspurl.branch_cd = jheader["branch_code"].asString();
    _fspurl.pos_no = jheader["register_number"].asString();
    _fspurl.company_cd = jheader["company_code"].asString();
    _fspurl.trans_cd = jheader["transaction_code"].asString();
    _fspurl.trans_no = jheader["transaction_number"].asString();
    return true;
}
