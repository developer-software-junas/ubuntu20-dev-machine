//*******************************************************************
//        FILE:     detail_calculate_regular.cpp
// DESCRIPTION:     Sales detail record retrieval
//*******************************************************************
#include "cyrestcommon.h"
using std::string;
//*******************************************************************
//    FUNCTION:         detailCalculateRegular
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateRegular(Json::Value& jdetail)
{
    //////////////////////////////////////
    //  VAT exempt item???
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
    }  else  {
        return seterrormsg(_util->errormsg());
    }
    //////////////////////////////////////
    //  Item discount flag
    Json::Value jxitem = jdetail["settings_xitem"];
    //////////////////////////////////////
    //  Quantity / price
    double dLow = -999999999.99;
    tmp = jdetail["quantity"].asString();
    if (!_util->valid_decimal("Quantity", tmp.c_str (), 1, 99, dLow, 999999999))
        return seterrormsg(_util->errormsg());
    double quantity = _util->stodsafe(tmp);
    if (quantity < 1.00)
        return detailCalculateRegularWeighted(jdetail);

    tmp = jdetail["retail_price"].asString();
    if (!_util->valid_decimal("Retail price", tmp.c_str (), 1, 99, 0, 999999999))
        return seterrormsg(_util->errormsg());
    double price = _util->stodsafe(tmp);
    double adjustedPrice = price;
    //////////////////////////////////////
    //  Add on amount(s)
    double addon = 0.00;
    /***
    string addonamount = jdetail["amount_trx"].asString();
    if (_util->valid_decimal("Add on amount", addonamount.c_str (), 1, 99, 0, 999999999))  {
        addon = _util->stodsafe(addonamount);
    }
    price += addon;
    adjustedPrice += addon;
    ***/
    _totalAddonAmount += addon;
    //////////////////////////////////////
    //  Update totals
    _totalItems += quantity;
    _totalGrossAmount += price * quantity;
    //////////////////////////////////////
    //  Calculate the item discount???
    jdetail["item_discount"]=false;
    jdetail["item_discount_amount"]=0.00;
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString()) {
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
                discount = price * discountValue;
            }  else  {
                double ddiscount = price - discountValue;
                if (ddiscount == 0)
                    discount = 0;
                else
                    discount = discountValue;
            }
            discount = (double)((int)(discount*100))/100;
            adjustedPrice -= discount;
            localItemDiscount += discount;
            jdetail["item_discount"]=true;
            jdetail["item_discount_amount"]=discount;
            //  Update totals

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
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString() &&
        jdetail["override_trx_discount"].asString() == "0") {
        if (_hdrDiscountCode.length() > 0)  {
            //  Calculate the discount
            double discount = 0.00, discountValue = 0.00;
            if (_hdrDiscountPct)  {
                discountValue = (_hdrDiscountRate / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*100))/100;
            adjustedPrice -= discount;
            localTrxDiscount += discount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=discount;
            //  Update totals
            _totalTrxDiscount += discount * quantity;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_GLOBAL;
            strDiscount->amount = discount * quantity;
            strDiscount->discount_desc = _hdrDiscountDesc;
            strDiscount->discount_code = _hdrDiscountCode;

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code)  {
                    found = true;
                    disc->amount += (discount * quantity);
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }  //else if (jdetail[""])
    }
    ////////////////////////////////////
    //  Subtract VAT
    double vatAmount = 0.00, lineVat=0.00;
    if (vatPercentage)  {
        vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    }  else  {
        vatAmount = adjustedPrice = vatRate;
    }
    lineVat = vatAmount;
    vatAmount = vatAmount * quantity;
    //////////////////////////////////////
    //  VAT total(s)
    double netVat=0.00,amtVat=0.00,netZero=0.00,amtZero=0.00;
    if (vatRate == 0)  {
        lineVat = 0;
        netZero = vatAmount;
        amtZero = 0.00;//(adjustedPrice * quantity) - vatAmount;
    }  else  {
        netVat = vatAmount;
        amtVat = (adjustedPrice * quantity) - vatAmount;
    }
    double netExempt = 0.00;
    double amtExempt = 0.00;


    _totalNetVat += netVat;
    _totalNetZero += netZero;
    _totalNetExempt += netExempt;

    _totalAmtVat += amtVat;
    _totalAmtZero += amtZero;
    _totalAmtExempt += amtExempt;

    if (_isSen5 && quantity <= 1)  {
        adjustedPrice -= adjustedPrice - vatAmount;
    }  else if (_isSen5 && quantity > 1.00 && vatRate == 0.00)  {
        adjustedPrice = adjustedPrice;
    }  else if (_isSen5 && quantity > 1.00 && vatRate > 0.00)  {
        adjustedPrice = lineVat;
    }  else  {
        adjustedPrice = (adjustedPrice - lineVat);
    }

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=netExempt;

    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;
    //  Update totals
    _totalVatAmount += amtVat;
    _totalNetSales += adjustedPrice * quantity;
    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=amtVat;
    jdetail["amount_tax"]=netVat;
    jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;

    return mark_detail_trx("regular",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
//*******************************************************************
//    FUNCTION:         detailCalculateRegular
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateRegularWeighted(Json::Value& jdetail)
{
    //////////////////////////////////////
    //  VAT exempt item???
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
    }  else  {
        return seterrormsg(_util->errormsg());
    }
    //////////////////////////////////////
    //  Item discount flag
    Json::Value jxitem = jdetail["settings_xitem"];
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
    double adjustedPrice = price * quantity;
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
    //////////////////////////////////////
    //  Update totals
    _totalItems += quantity;
    _totalGrossAmount += price * quantity;
    //////////////////////////////////////
    //  Calculate the item discount???
    jdetail["item_discount"]=false;
    jdetail["item_discount_amount"]=0.00;
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString()) {
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
            discount = (double)((int)(discount*100))/100;
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
    }
    ////////////////////////////////////
    //  Transaction discount only if
    //  the item has a discount.
    //  NOTE:  The old POS allows a
    //  transaction discount even if
    //  the item has been marked X
    if (jdetail["xitem"].asString() != jxitem["xvalue"].asString() &&
        jdetail["override_trx_discount"].asString() != "1") {
        if (_hdrDiscountCode.length() > 0)  {
            //  Calculate the discount
            double discount = 0.00, discountValue = 0.00;
            if (_hdrDiscountPct)  {
                discountValue = (_hdrDiscountRate / 100);
                discount = adjustedPrice * discountValue;
            }  else  {
                discount = discountValue;
            }
            discount = (double)((int)(discount*100))/100;
            adjustedPrice -= discount;
            localTrxDiscount += discount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=discount;
            //  Update totals
            _totalTrxDiscount += discount * quantity;
            //////////////////////////////
            //  EOD details
            struct_discount* strDiscount = new struct_discount;

            strDiscount->type = DISC_GLOBAL;
            strDiscount->amount = discount * quantity;
            strDiscount->discount_desc = _hdrDiscountDesc;
            strDiscount->discount_code = _hdrDiscountCode;

            bool found = false;
            size_t j = _xread_discount.size();
            for (size_t i = 0; i < j; i++)  {
                struct_discount* disc = _xread_discount.at(i);
                if (disc->discount_code == strDiscount->discount_code)  {
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
    //  Subtract VAT
    double vatAmount = 0.00;
    if (vatPercentage)  {
        vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    }  else  {
        vatAmount = adjustedPrice = vatRate;
    }
    //////////////////////////////////////
    //  VAT total(s)
    double netVat=0.00,amtVat=0.00,netZero=0.00,amtZero=0.00;
    if (_isSen5 && vatRate == 0)  {
        adjustedPrice -= adjustedPrice - vatAmount;
        amtVat = 0;
        netZero = 0;//vatAmount;
        amtZero = adjustedPrice - vatAmount;
        if (amtZero == 0)
            netZero = adjustedPrice;
    }  else if (_isSen5 && quantity > 1.00 && vatRate > 0.00)  {
        netVat = vatAmount;
        amtVat = (adjustedPrice - vatAmount);
    }  else  {
        if (vatRate == 0)  {
            amtVat = 0;
            netZero = 0;//vatAmount;
            amtZero = adjustedPrice - vatAmount;
            if (amtZero == 0)
                netZero = adjustedPrice;
        }  else  {
            netVat = vatAmount;
            amtVat = (adjustedPrice - vatAmount);
        }
    }



    double netExempt = 0.00;
    double amtExempt = 0.00;

    _totalNetVat += netVat;
    _totalNetZero += netZero;
    _totalNetExempt += netExempt;

    _totalAmtVat += amtVat;
    _totalAmtZero += amtZero;
    _totalAmtExempt += amtExempt;

    adjustedPrice -= amtVat;

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=netExempt;

    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;
    //  Update totals
    _totalVatAmount += amtVat;
    _totalNetSales += adjustedPrice * quantity;
    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=amtVat;
    jdetail["amount_tax"]=netVat;
    jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;

    return mark_detail_trx("regular",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
