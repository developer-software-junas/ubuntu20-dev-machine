//*******************************************************************
//        FILE:     detail_calculate_regular.cpp
// DESCRIPTION:     Sales detail record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "cyw_ini.h"
using std::string;
//*******************************************************************
//    FUNCTION:         detailCalculate
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateSenior(Json::Value& jdetail)
{
    //////////////////////////////////////
    //  VAT exempt item???
    bool vatExempt = false;
    double vatRate = 0.00;
    bool vatPercentage = false;
    double localTrxDiscount = 0;
    double localItemDiscount = 0;
    //////////////////////////////////////
    //  Check the tax rate
    Json::Value jtax = jdetail["det_tax"];
    string tmp = jtax["tax_value"].asString();
    vatPercentage = jtax["is_percentage"].asString() == "1";
    if (_util->valid_decimal("Tax value", tmp.c_str (), 1, 99, 0, 999999))  {
        vatRate = _util->stodsafe(tmp);
        if (vatRate == 0.00)
            vatExempt = true;
    }  else  {
        return seterrormsg(_util->errormsg());
    }
    //////////////////////////////////////
    //  Check the VAT exempt flag
    Json::Value jbranchitem = jdetail["det_product"];
    if (jbranchitem["is_vat"].asString() == "1")
        vatExempt = true;
    jdetail["vat_exempt"]=vatExempt;
    //////////////////////////////////////
    //  Item discount flag
    Json::Value jxitem = jdetail["settings_xitem"];
    //////////////////////////////////////
    //  Get the pos branch settings
    string sql;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = _ini->errormsg();
        return false;
    }
    dbsel->sql_reset ();
    Json::Value jheader;
    string systransnum = jdetail["systransnum"].asString();
    sql  = "select * from tg_pos_mobile_header ";
    sql += " where systransnum = ";
    sql += dbsel->sql_bind (1, systransnum);

    salesQueryType ret = json_select_single(dbsel, jheader, sql);
    if (ret == SQT_ERROR)  {
        _error = "Unable to retrieve the header record...";
        return false;
    }
    string location_code = jheader["branch_code"].asString();

    dbsel->sql_reset ();
    Json::Value jsettingspos;
    sql  = "select * from pos_settings ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind (1, location_code);

    ret = json_select_single(dbsel, jsettingspos, sql);
    if (ret == SQT_ERROR)  {
        _error = "Unable to retrieve the special senior / pwd discount rules";
        return false;
    }
    string ps_senior = jsettingspos["senior"].asString();
    string ps_senior_code = jsettingspos["senior_code"].asString();
    string ps_nodiscountvatexempt = jsettingspos["nodiscountvatexempt"].asString();
    //////////////////////////////////////
    //  JOEL - no discount means not VAT exempt
    std::stringstream xx;
    xx << jdetail["settings_detail_discount"];
    _log->logdebug("XXXXX",xx.str().c_str());
    if (ps_nodiscountvatexempt == "1" &&
        jdetail["settings_detail_discount"] == Json::nullValue)  {
        return detailCalculateRegular(jdetail);
    }
    /*
    if (cgiform("ini")=="joel" &&
        jdetail["settings_detail_discount"] == Json::nullValue)  {
        return detailCalculateRegular(jdetail);
    }*/
    //////////////////////////////////////
    //  Quantity / price
    double dLow = -999999999.99;
    tmp = jdetail["quantity"].asString();
    if (!_util->valid_decimal("Quantity", tmp.c_str (), 1, 99, dLow, 999999999))
        return seterrormsg(_util->errormsg());
    double quantity = _util->stodsafe(tmp);
    if (quantity < 1.00)
        return detailCalculateSeniorWeighted(jdetail);

    tmp = jdetail["retail_price"].asString();
    if (!_util->valid_decimal("Retail price", tmp.c_str (), 1, 99, 0, 999999999))
        return seterrormsg(_util->errormsg());
    double price = _util->stodsafe(tmp);
    double adjustedPrice = price;
    //////////////////////////////////////
    //  Add on amount(s)
    double addon = 0.00;
    /*
    string addonamount = jdetail["amount_trx"].asString();
    if (_util->valid_decimal("Add on amount", addonamount.c_str (), 1, 99, 0, 999999999))  {
        addon = _util->stodsafe(addonamount);
    }*/
    price += addon;
    adjustedPrice += addon;
    _totalAddonAmount += addon;
    //  Update totals
    _totalItems += quantity;
    _totalGrossAmount += price * quantity;
    ////////////////////////////////////
    //  Subtract VAT
    double vatAmount = 0.00;
    if (vatPercentage)  {
        vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    }  else  {
        vatAmount = adjustedPrice - vatRate;
    }
    //////////////////////////////////////
    //  VAT total(s)
    vatAmount = (vatAmount * 100) / 100;
    adjustedPrice = (adjustedPrice * 100) / 100;
    double amtExempt = adjustedPrice - vatAmount;
    double netVat = 0.00;
    double amtVat = 0.00;
    double netZero = 0.00;
    double amtZero = 0.00;
    adjustedPrice -= amtExempt;
    //////////////////////////////////////
    //  Calculate the item discount???
    tmp = jdetail["xitem"].asString();
    tmp = jxitem["xvalue"].asString();
    //  20241124
    //if (jdetail["xitem"].asString() != jxitem["xvalue"].asString()) {
    if (jdetail["override_trx_discount"].asString() == "1") {
        if (jdetail["settings_detail_discount"] != Json::nullValue)  {
            double discountValue = 0.00;
            Json::Value jdiscount = jdetail["settings_detail_discount"];
            bool discountPercentage = jdiscount["disctype"].asString() == "1";
            //  Validate the discount value
            tmp = jdiscount["discvalue"].asString();
            if (atoi(tmp.c_str())==0)  {
                tmp = jdetail["discount_amount"].asString();
            }
            if (_util->valid_decimal("Discount value", tmp.c_str (), 1, 99, 0, 999999999))  {
                discountValue = _util->stodsafe(tmp);
            }  else  {
                return seterrormsg(_util->errormsg());
            }
            //  Calculate the discount
            double discount = 0.00;
            if (discountPercentage)  {
                discountValue = (discountValue / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*10000))/10000;
            if (discountPercentage)
                adjustedPrice -= discount;
            else
                adjustedPrice -= (discount/quantity);
            localItemDiscount += discount;
            jdetail["item_discount"]=true;
            jdetail["item_discount_amount"]=discount;
            //  Update totals
            if (quantity < 1.00 || false == discountPercentage)
                _totalItemDiscount += discount;
            else
                _totalItemDiscount += discount * quantity;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_ITEM;
            strDiscount->amount = discount * quantity;
            strDiscount->discount_desc = jdiscount["description"].asString();
            strDiscount->discount_code = jdiscount["discount_cd"].asString();
            strDiscount->discount_type_code = jdiscount["discount_type_cd"].asString();

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code &&
                    disc->discount_type_code == strDiscount->discount_type_code)  {
                    found = true;
                    disc->amount += (discount * quantity);
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    }
    ////////////////////////////////////
    //  Transaction discount only if
    //  the item has a discount.
    //  NOTE:  The old POS allows a
    //  transaction discount even if
    //  the item has been marked X
    tmp = jdetail["xitem"].asString();
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString()) {
        if (_hdrDiscountCode.length() > 0)  {
            //  Calculate the discount
            double discount = 0.00, discountValue = 0.00;
            if (_hdrDiscountPct)  {
                discountValue = (_hdrDiscountRate / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*10000))/10000;
            /***
            if (_isSenior)  {
                if (_trxDiscountMaxAmount < discount)  {
                    //  important!!!
                    discount = _trxDiscountMaxAmount;
                    //////////////////////////////////////
                    //  Database connection
                    CYDbSql* db = _ini->dbconn();
                    if (nullptr == db)
                        return seterrormsg(_ini->errormsg());
                    string sql = "update tg_pos_mobile_discount_detail set discount_type = 0, discount_amount = ";
                    sql += db->sql_field_value ("discount_value",);
                }
            }***/
            adjustedPrice -= discount;
            localTrxDiscount += discount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=discount;
            //  Update totals
            if (quantity < 1.00 || false == _hdrDiscountPct)
                _totalTrxDiscount += discount;
            else
                _totalTrxDiscount += discount * quantity;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_GLOBAL;
            if (quantity < 1.00 || false == _hdrDiscountPct)
                strDiscount->amount = discount;
            else
                strDiscount->amount = discount * quantity;
            strDiscount->discount_desc = _hdrDiscountDesc;
            strDiscount->discount_code = _hdrDiscountCode;

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code)  {
                    found = true;
                    disc->amount += discount * quantity;
                }
            }
            //adjustedPrice -= (strDiscount->amount * quantity);
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    }

    jdetail["retail_price_adjusted"]=adjustedPrice;
    if (quantity < 1.00)  {
        //_totalAmtSales += amtVat;
        _totalNetSales += adjustedPrice;
    }  else  {
        //_totalAmtSales += amtVat * quantity;
        _totalNetSales += adjustedPrice * quantity;
    }
    if (quantity < 1.00)  {
        _totalNetVat += netVat;
        _totalNetZero += netZero;
        _totalNetExempt += vatAmount - (localTrxDiscount+localItemDiscount);

        _totalAmtVat += amtVat;
        _totalAmtZero += amtZero;
        _totalAmtExempt += amtExempt;

    }  else  {
        _totalNetVat += netVat * quantity;
        _totalNetZero += netZero * quantity;
        _totalNetExempt += (vatAmount * quantity) -
                (localTrxDiscount*quantity)+(localItemDiscount*quantity);

        _totalAmtVat += amtVat * quantity;
        _totalAmtZero += amtZero * quantity;
        _totalAmtExempt += amtExempt * quantity;
    }

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=vatAmount;
    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;

    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=amtExempt;
    jdetail["amount_tax"]=vatAmount;
    if (localTrxDiscount > 0)
        jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;
    return mark_detail_trx("senior",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
//*******************************************************************
//    FUNCTION:         detailCalculate
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateSeniorWeighted(Json::Value& jdetail)
{
    //////////////////////////////////////
    //  VAT exempt item???
    bool vatExempt = false;
    double vatRate = 0.00;
    bool vatPercentage = false;
    double localTrxDiscount = 0;
    double localItemDiscount = 0;
    //////////////////////////////////////
    //  Check the tax rate
    Json::Value jtax = jdetail["det_tax"];
    string tmp = jtax["tax_value"].asString();
    vatPercentage = jtax["is_percentage"].asString() == "1";
    if (_util->valid_decimal("Tax value", tmp.c_str (), 1, 99, 0, 999999))  {
        vatRate = _util->stodsafe(tmp);
        if (vatRate == 0.00)
            vatExempt = true;
    }  else  {
        return seterrormsg(_util->errormsg());
    }
    //////////////////////////////////////
    //  Check the VAT exempt flag
    Json::Value jbranchitem = jdetail["det_product"];
    if (jbranchitem["is_vat"].asString() == "1")
        vatExempt = true;
    jdetail["vat_exempt"]=vatExempt;
    //////////////////////////////////////
    //  Item discount flag
    Json::Value jxitem = jdetail["settings_xitem"];
    //////////////////////////////////////
    //  Get the pos branch settings
    string sql;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = _ini->errormsg();
        return false;
    }
    dbsel->sql_reset ();
    Json::Value jheader;
    string systransnum = jdetail["systransnum"].asString();
    sql  = "select * from tg_pos_mobile_header ";
    sql += " where systransnum = ";
    sql += dbsel->sql_bind (1, systransnum);

    salesQueryType ret = json_select_single(dbsel, jheader, sql);
    if (ret == SQT_ERROR)  {
        _error = "Unable to retrieve the header record...";
        return false;
    }
    string location_code = jheader["branch_code"].asString();

    dbsel->sql_reset ();
    Json::Value jsettingspos;
    sql  = "select * from pos_settings ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind (1, location_code);

    ret = json_select_single(dbsel, jsettingspos, sql);
    if (ret == SQT_ERROR)  {
        _error = "Unable to retrieve the special senior / pwd discount rules";
        return false;
    }
    string ps_senior = jsettingspos["senior"].asString();
    string ps_senior_code = jsettingspos["senior_code"].asString();
    string ps_nodiscountvatexempt = jsettingspos["nodiscountvatexempt"].asString();
    //////////////////////////////////////
    //  JOEL - no discount means not VAT exempt
    if (ps_nodiscountvatexempt == "1" &&
        jdetail["settings_detail_discount"] == Json::nullValue)  {
        return detailCalculateRegular(jdetail);
    }
    //////////////////////////////////////
    //  Quantity / price
    double dLow = -999999999.99;
    tmp = jdetail["quantity"].asString();
    if (!_util->valid_decimal("Quantity", tmp.c_str (), 1, 99, dLow, 999999999))
        return seterrormsg(_util->errormsg());
    double quantity = _util->stodsafe(tmp);

    tmp = jdetail["retail_price"].asString();
    if (!_util->valid_decimal("Retail price", tmp.c_str (), 1, 99, 0, 999999999))
        return seterrormsg(_util->errormsg());
    double price = _util->stodsafe(tmp);
    double adjustedPrice = price;
    //////////////////////////////////////
    //  Add on amount(s)
    double addon = 0.00;
    /*
    string addonamount = jdetail["amount_trx"].asString();
    if (_util->valid_decimal("Add on amount", addonamount.c_str (), 1, 99, 0, 999999999))  {
        addon = _util->stodsafe(addonamount);
    }*/
    price += addon;
    adjustedPrice += addon;
    _totalAddonAmount += addon;
    //  Update totals
    _totalItems += quantity;
    _totalGrossAmount += price * quantity;
    ////////////////////////////////////
    //  Subtract VAT
    double vatAmount = 0.00;
    if (vatPercentage)  {
        vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    }  else  {
        vatAmount = adjustedPrice - vatRate;
    }
    //////////////////////////////////////
    //  VAT total(s)
    vatAmount = (vatAmount * 100) / 100;
    vatAmount = vatAmount * quantity;
    adjustedPrice = (adjustedPrice * 100) / 100;
    adjustedPrice = adjustedPrice * quantity;
    double amtExempt=0.00,netZero=0.00,amtZero=0.00;
    if (vatRate == 0)  {
        netZero = 0;//vatAmount;
        amtZero = adjustedPrice - vatAmount;
    }  else  {
        amtExempt = adjustedPrice - vatAmount;
    }
    double netVat = 0.00;
    double amtVat = 0.00;
    adjustedPrice -= amtExempt;
    //////////////////////////////////////
    //  Calculate the item discount???
    bool alreadyDiscounted = false;
    //string xi = jdetail["xitem"].asString();
    //string xv = jxitem["xvalue"].asString();
    //if (xi != xv) {
        if (jdetail["settings_detail_discount"] != Json::nullValue)  {
            alreadyDiscounted = true;
            double discountValue = 0.00;
            Json::Value jdiscount = jdetail["settings_detail_discount"];
            bool discountPercentage = jdiscount["disctype"].asString() == "1";
            //  Validate the discount value
            tmp = jdiscount["discvalue"].asString();
            if (atoi(tmp.c_str())==0)  {
                tmp = jdetail["discount_amount"].asString();
            }
            if (_util->valid_decimal("Discount value", tmp.c_str (), 1, 99, 0, 999999999))  {
                discountValue = _util->stodsafe(tmp);
            }  else  {
                return seterrormsg(_util->errormsg());
            }
            //  Calculate the discount
            double discount = 0.00;
            if (discountPercentage)  {
                discountValue = (discountValue / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*10000))/10000;




            adjustedPrice -= discount;
            localItemDiscount += discount;
            jdetail["item_discount"]=true;
            jdetail["item_discount_amount"]=discount;
            //  Update totals
                _totalItemDiscount += discount;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_ITEM;
            strDiscount->amount = discount;
            strDiscount->discount_desc = jdiscount["description"].asString();
            strDiscount->discount_code = jdiscount["discount_cd"].asString();
            strDiscount->discount_type_code = jdiscount["discount_type_cd"].asString();

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code &&
                    disc->discount_type_code == strDiscount->discount_type_code)  {
                    found = true;
                    disc->amount += (discount * quantity);
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    //}
    ////////////////////////////////////
    //  Transaction discount only if
    //  the item has a discount.
    //  NOTE:  The old POS allows a
    //  transaction discount even if
    //  the item has been marked X
    tmp = jdetail["xitem"].asString();
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString()) {
        if (_hdrDiscountCode.length() > 0 && false == alreadyDiscounted)  {
            //  Calculate the discount
            double discount = 0.00, discountValue = 0.00;
            if (_hdrDiscountPct)  {
                discountValue = (_hdrDiscountRate / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*10000))/10000;
            adjustedPrice -= discount;
            localTrxDiscount += discount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=discount;
            //  Update totals
            if (quantity < 1.00 || false == _hdrDiscountPct)
                _totalTrxDiscount += discount;
            else
                _totalTrxDiscount += discount * quantity;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_GLOBAL;
            if (quantity < 1.00 || false == _hdrDiscountPct)
                strDiscount->amount = discount;
            else
                strDiscount->amount = discount * quantity;
            strDiscount->discount_desc = _hdrDiscountDesc;
            strDiscount->discount_code = _hdrDiscountCode;

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code)  {
                    found = true;
                    disc->amount += discount * quantity;
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    }

    jdetail["retail_price_adjusted"]=adjustedPrice;
    _totalNetSales += adjustedPrice;

        _totalNetVat += netVat;
        _totalNetZero += netZero;
        _totalNetExempt += adjustedPrice;

        _totalAmtVat += amtVat;
        _totalAmtZero += amtZero;
        _totalAmtExempt += amtExempt;

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=vatAmount;
    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;

    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=amtExempt;
    jdetail["amount_tax"]=vatAmount;
    jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;
    return mark_detail_trx("senior",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
