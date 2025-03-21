//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunnies.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptSunnies::assembleReceiptDetail (Json::Value& jdetail,
                                                          Json::Value& jhdrtrx,
                                                          salesReceiptType type)
{
    //////////////////////////////////////
    //  Receipt width / INI type
    int left = 23, right = 18;
    string skinny = cgiform("receipt_skinny");
    if (skinny == "true")  {
        left = 18; right = 13;
    }
    //_txtdet = "";
    int multiplier = 1;
    if (type == salesReceiptType::SRT_POSTVOID)
        multiplier = -1;
    char sztmp[32];
    string tmp2 = "";
    double quantity = stodsafe(jdetail["quantity"].asString())*multiplier;
    double priceOriginal = stodsafe(jdetail["retail_price"].asString())*multiplier;

    int wleft = 23, wright = 18;
    if (skinny == "true")   {
        wleft = 19;
        wright = 12;
    }
    //////////////////////////////////////
    //  Display item detail(s)
    //FORMAT SUFFIX
    string itemSuffix;
    if (jdetail["vat_exempt"].asString () == "true")
        itemSuffix = "NV";
    else  {
        if (jhdrtrx["is_sc"].asString() == "1" ||
            jhdrtrx["is_pwd"].asString() == "1")  {
            itemSuffix = "NV";
        }  else if (jhdrtrx["is_zero_rated"].asString() == "1")  {
            itemSuffix = "V";
        }  else  {
            itemSuffix = "T";
        }
    }

    if (stodsafe(jdetail["gift_wrap_quantity"].asString()) > 0)
        _isGiftWrap = true;
    //////////////////////////////////////
    //  Detail line 1 - sku / description
    string line1  = jdetail["detail_product_header"]["item_code"].asString();
           line1 += " ";

    if (skinny == "true")
        line1  = _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    else
        line1 += _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    //////////////////////////////////////
    //  Truncate if longer than receipt width
    if (line1.length() > (size_t)(_rcptWidth-1))
        line1 = line1.substr(0,(_rcptWidth-1));
    _txtdet += line1;
    _txtdet += "\n";
    //////////////////////////////////////
    //  Detail line 2 - quantity / price
    string line2 = "";
    char szleft[64], szright[64];

    string uom, desc;
    if (jdetail["uom_code"].asString().length() < 1) {
        uom = "PC";
    } else {
        uom = jdetail["uom_code"].asString();
    }

    string strOrigPrice = FMTNumberComma(priceOriginal);
    string itmBaseType =  jdetail["detail_product_header"]["base_type_code"].asString();
    if (skinny == "true")
        sprintf (szleft, " %0.03f @ %s",quantity, strOrigPrice.c_str());
    else
        sprintf (szleft, " %0.03f %s @ %s",quantity, uom.c_str(), strOrigPrice.c_str());

    string strTotalPrice = FMTNumberComma(priceOriginal*quantity);
    sprintf (szright, "%s%s", strTotalPrice.c_str(),itemSuffix.c_str());

    line2 = _util->cyLRText(left, szleft, right, szright);
    _txtdet += line2;
    //////////////////////////////////////
    //  Display line detail(s)
    string tmp;
    Json::Value jproduct = jdetail["detail_product_header"];
    sprintf (sztmp, "%0.2f", quantity);

    tmp  = sztmp;
    tmp += "  ";
    tmp += _util->string_cleanse(jdetail["description"].asString().c_str (),true);

    sprintf (sztmp, "%0.2f", priceOriginal*quantity);
    //_txtdet += _util->cyLRText(wleft+3,tmp.c_str(),wright-3, sztmp);
    //////////////////////////////////////
    //  Item quantity / price
    if (jdetail["description"].asString() == "SPLIT CHECK ADJUSTMENT")  {
        sprintf (sztmp, "%.02f", priceOriginal * quantity);
        tmp = _util->cyLRText(wleft, jdetail["description"].asString(), wright, sztmp);
        //_txtdet += tmp;
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
                jhdrtrx["is_pwd"].asString() == "1")  {
                itemSuffix = "NV";
            }  else if (jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "V";
            }  else  {
                itemSuffix = "T";
            }
        }
    }
    double dlinedisc = 0.00;
    double dsubtot = priceOriginal*quantity;
    //////////////////////////////////////
    //  Transaction discount
    strTotalPrice = FMTNumberComma(priceOriginal*quantity);
    if (jhdrtrx["is_sc"].asString() == "1" || jhdrtrx["is_pwd"].asString() == "1") {
        if (jdetail["override_trx_discount"].asString() == "0")  {
            double dtot = _util->stodsafe(strTotalPrice);
            double dqty = _util->stodsafe(jdetail["quantity"].asString());
            double dtmp = _util->stodsafe(jdetail["less_tax"].asString());
            //  VAT discount
            double dcalc = 0;
            if (dqty < 1)  {
                dcalc = dtmp;
            }  else  {
                dcalc = dtmp * dqty;
            }
            dtot -= (dcalc);
            sprintf (szright, "%.02f",dcalc);
            if ((dcalc) != 0)  {
                if (quantity < 1)  {
                     dsubtot += dtmp;
                     dlinedisc += dsubtot;
                }  else  {
                     dsubtot += (dcalc);
                     dlinedisc += (dcalc);
                }
                _txtdet += _util->cyLRText(left, "  LESS VAT", right, szright);
            }

            string desc  = "  LESS DISCOUNT";
            //desc += jdetail["discount_code"].asString ();
            //desc += ")";
            dtmp = _util->stodsafe(jdetail["amount_trx"].asString());
            if (dqty < 1)  {
                dcalc = dtmp;
            }  else  {
                dcalc = dtmp * dqty;
            }

            dtot -= (dcalc);
            sprintf (szright, "%.02f",dcalc);
            if ((dtmp*dqty) != 0)  {
                _txtdet += _util->cyLRText(left, desc, right, szright);
            }
        }
    }
    //////////////////////////////////
    //  Is the item discounted?
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less ";
        Json::Value jdisc = jdetail["settings_detail_discount"];

        std::stringstream vv;
        vv << jdisc;
        _log->logmsg("CVVV",vv.str().c_str());

        desc += jdisc["description"].asString();
        string type = jdisc["discount_rule"].asString();
        string tmpdisc = jdetail["amount_discount"].asString();
        if (tmpdisc.length() < 1) tmpdisc = "0";
        double disctemp = _util->stodsafe(tmpdisc);
        if (disctemp > 0.00)  {
            if (quantity < 1 || type == "1")  {
                 dsubtot -= disctemp;
                 dlinedisc += disctemp;
            }  else  {
                 dsubtot -= (disctemp*quantity);
                 dlinedisc += (disctemp*quantity);
                 disctemp = disctemp * quantity;
            }

            sprintf (sztmp, "%.02f", (disctemp*1000)/1000);
            tmp = _util->cyLRText(wleft, desc, wright, sztmp);
            _txtdet += tmp;
        }
    }
    /*
    if (subtot)  {
        sprintf (sztmp, "%.02f", ((priceOriginal*quantity) - dlinedisc));
        tmp = _util->cyLRText(wleft, "  DISCOUNTED PRICE", wright, sztmp);
        _txtdet += tmp;
    }*/
    //////////////////////////////////////
    //  Split adjustment
    if (_totalSplitAdjust > 0)  {
        //sprintf (sztmp, "%.02f", _totalSplitAdjust);
        //_txtdet += _util->cyLRText(23, "Split check adjustment", 18, sztmp);
        string desc  = "  Less split check ";
        sprintf (sztmp, "%.02f", (_totalSplitAdjust*1000)/1000);
        tmp = _util->cyLRText(wleft+3, desc, wright-3, sztmp);
        _txtdet += tmp;
    }
    //////////////////////////////////////
    //  Tally FNB / BEVERAGE
    double dFnbTally = 0.00;
    double dprice = _util->stodsafe(jdetail["retail_price_adjusted"].asString());
    double dqty = _util->stodsafe(jdetail["quantity"].asString());
    if (dqty < 1.00)
        dFnbTally = dprice;
    else
        dFnbTally = dqty * dprice;
    if (jdetail["printer_code"].asString() == "KB")  {
        _fnbBeverage += dFnbTally;
    }  else  {
        _fnbFood += dFnbTally;
    }
    //////////////////////////////////
    //  Calculate the service charge
    if (jdetail["total_service_charge"]!=Json::nullValue)  {
        string tsc = jdetail["total_service_charge"].asString();
        if (jdetail["is_auto_servicecharge"].asString() == "1")  {
            _totalServiceCharge += _util->stodsafe(tsc);
        }
    }
    _receipt += _txtdet;
    return true;
}
