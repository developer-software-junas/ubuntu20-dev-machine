//*******************************************************************
//        FILE:     detail_calculate_regular.cpp
// DESCRIPTION:     Sales detail record retrieval
//*******************************************************************
#include "cyrestcommon.h"
using std::string;
//*******************************************************************
//    FUNCTION:         detailCalculate
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateZeroRated(Json::Value& jdetail)
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
    //  Quantity / price
    double dLow = -999999999.99;
    tmp = jdetail["quantity"].asString();
    if (!_util->valid_decimal("Quantity", tmp.c_str (), 1, 99, dLow, 999999999))
        return seterrormsg(_util->errormsg());
    double quantity = _util->stodsafe(tmp);
    if (quantity < 1.00)
        return detailCalculateZeroRatedWeighted(jdetail);

    tmp = jdetail["retail_price"].asString();
    if (!_util->valid_decimal("Retail price", tmp.c_str (), 1, 99, 0, 999999999))
        return seterrormsg(_util->errormsg());
    double price = _util->stodsafe(tmp);
    double adjustedPrice = price;
    //////////////////////////////////////
    //  Add on amount / FNB modifier(s)
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
    ////////////////////////////////////
    //  Subtract VAT
    double vatAmount = 0.00;
    if (vatPercentage)  {
        vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    }  else  {
        vatAmount = adjustedPrice = vatRate;
    }
    //vatAmount = vatAmount * quantity;
    //////////////////////////////////////
    //  VAT total(s)
    /*
    double netExempt=0.00,amtExempt=0.00,netZero=0.00,amtZero=0.00;
    if (vatRate == 0)  {
        netZero = 0;//vatAmount;
        amtZero = (adjustedPrice * quantity) - vatAmount;
        adjustedPrice -= amtZero;
    }  else  {
        netExempt = vatAmount;
        amtExempt = (adjustedPrice * quantity) - vatAmount;
        adjustedPrice -= amtExempt;
    }
    double netVat = 0.00;
    double amtVat = 0.00;*/

    double netZero = vatAmount;
    double amtZero = adjustedPrice - vatAmount;
    //////////////////////////////////////
    //  VAT total(s)
    double netVat = 0.00;
    double amtVat = 0.00;
    //double netZero = vatAmount;
    //double amtZero = adjustedPrice - vatAmount;
    double netExempt = 0.00;
    double amtExempt = 0.00;

    if (vatRate > 0)  {
        netZero = 0;
        amtZero = 0;
        netExempt = vatAmount;// * quantity;
        amtExempt = (adjustedPrice - vatAmount);// * quantity;
    }
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
            localItemDiscount = discount;
            adjustedPrice -= localItemDiscount;
            jdetail["item_discount"]=true;
            jdetail["item_discount_amount"]=localItemDiscount;
            //  Update totals
            _totalItemDiscount += (localItemDiscount*quantity);
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
                    disc->amount += discount * quantity;
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
    /*
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
            discount = (double)((int)(discount*100))/100;
            localTrxDiscount = (discount*quantity);
            adjustedPrice -= localTrxDiscount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=localTrxDiscount;
            //  Update totals
            _totalTrxDiscount += (localTrxDiscount*quantity);
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
                    disc->amount += discount * quantity;
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    }*/
    double tmpnet = (price * quantity)-((localItemDiscount*quantity)+localTrxDiscount);
    _totalNetSales += tmpnet;
    if (vatRate == 0)  {
        netExempt = 0;
        netZero = (price * quantity)-((localItemDiscount*quantity)+localTrxDiscount);
    }  else  {
        netZero = 0;
        netExempt = (price * quantity)-((localItemDiscount*quantity)+localTrxDiscount);
        netExempt -= (amtExempt * quantity);
    }
    adjustedPrice -= amtExempt;
    _totalNetSales -= (amtExempt*quantity);
    amtExempt *= quantity;

    //netZero *= quantity;
    //netExempt *= quantity;
    /*
    netVat *= quantity;
    netZero *= quantity;
    netExempt *= quantity;

    amtVat *= quantity;
    amtZero *= quantity;
    amtExempt *= quantity;*/

    _totalNetVat += netVat;
    _totalNetZero += netZero;
    _totalNetExempt += netExempt;

    _totalAmtVat += amtVat;
    _totalAmtZero += amtZero;
    _totalAmtExempt += amtExempt;

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=netExempt;
    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;
    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=netVat;
    jdetail["amount_tax"]=netZero;
    if (netVat == 0 && netZero == 0)  {
        jdetail["less_tax"]=netExempt;
        jdetail["amount_tax"]=amtExempt;
    }
    if (localTrxDiscount > 0)
        jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;

    return mark_detail_trx("zero",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
//*******************************************************************
//    FUNCTION:         detailCalculate
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculateZeroRatedWeighted(Json::Value& jdetail)
{
    //////////////////////////////////////
    //  VAT exempt item???
    bool vatExempt = false;
    double vatRate = 0.00;
    double localTrxDiscount = 0;
    double localItemDiscount = 0;
    //////////////////////////////////////
    //  Check the tax rate
    Json::Value jtax = jdetail["det_tax"];
    string tmp = jtax["tax_value"].asString();
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
    //  Add on amount / FNB modifier(s)
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
    ////////////////////////////////////
    //  Subtract VAT
    double vatAmount = 0.00;
    vatAmount = ((adjustedPrice * 100) / ((100 + vatRate) * 100) * 100);
    //vatAmount = vatAmount * quantity;
    //////////////////////////////////////
    //  VAT total(s)
    double netExempt=0.00,amtExempt=0.00,netZero=0.00,amtZero=0.00;
    if (vatRate == 0)  {
        netZero = 0;//vatAmount;
        amtZero = adjustedPrice - vatAmount;
        adjustedPrice -= amtZero;
    }  else  {
        netExempt = vatAmount;
        amtExempt = adjustedPrice - vatAmount;
        adjustedPrice -= amtExempt;
    }
    double netVat = 0.00;
    double amtVat = 0.00;
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
            localItemDiscount = discount;
            adjustedPrice -= localItemDiscount;
            jdetail["item_discount"]=true;
            jdetail["item_discount_amount"]=localItemDiscount;
            //  Update totals
            _totalItemDiscount += localItemDiscount;
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
                    disc->amount += discount * quantity;
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
    /*
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
            discount = (double)((int)(discount*100))/100;
            localTrxDiscount = (discount*quantity);
            adjustedPrice -= localTrxDiscount;
            jdetail["trx_discount"]=true;
            jdetail["trx_discount_amount"]=localTrxDiscount;
            //  Update totals
            _totalTrxDiscount += (localTrxDiscount*quantity);
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
                    disc->amount += discount * quantity;
                }
            }
            if (!found)
                _xread_discount.push_back(strDiscount);
            else
                delete (strDiscount);
        }
    }*/

    _totalNetSales += (price * quantity)-(localItemDiscount+localTrxDiscount);
    if (false == vatExempt)  {
        netZero = 0;
        netExempt = (price * quantity)-(localItemDiscount+localTrxDiscount);
        netExempt -= amtExempt;
    }  else  {
        netExempt = 0;
        netZero = (price * quantity)-(localItemDiscount+localTrxDiscount);
    }
    //adjustedPrice -= amtExempt;
    _totalNetSales -= amtExempt;


    _totalNetVat += netVat;
    _totalNetZero += netZero;
    _totalNetExempt += netExempt;

    _totalAmtVat += amtVat;
    _totalAmtZero += amtZero;
    _totalAmtExempt += amtExempt;

    jdetail["netVat"]=netVat;
    jdetail["netZero"]=netZero;
    jdetail["netExempt"]=netExempt;
    jdetail["amtVat"]=amtVat;
    jdetail["amtZero"]=amtZero;
    jdetail["amtExempt"]=amtExempt;
    //////////////////////////////////////
    //  Save all calculated value(s)
    jdetail["less_tax"]=netVat;
    jdetail["amount_tax"]=netZero;
    jdetail["amount_trx"]=localTrxDiscount;
    jdetail["amount_discount"]=localItemDiscount;
    jdetail["retail_price_adjusted"]=adjustedPrice;

    return mark_detail_trx("zero",
            jdetail["systransnum"].asString(),
            jdetail["item_seq"].asString());
}
