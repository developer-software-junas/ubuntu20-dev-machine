//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using namespace std;
#include <cmath>
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptElena::assembleReceiptDetail (Json::Value& jdetail,
                                                           Json::Value& jhdrtrx,
                                                           salesReceiptType type)
{
    //////////////////////////////////////
    //  Receipt width / INI type
    /*
    int left = 23, right = 18;
    string skinny = cgiform("receipt_skinny");
    if (skinny == "true")  {
        left = 18; right = 13;
    }*/
    //_txtdet = "";
    int multiplier = 1;
    if (type == salesReceiptType::SRT_POSTVOID)
        multiplier = -1;
    char sztmp[32];
    string tmp2 = "";
    string seq = jdetail["item_seq"].asString();
    double quantity = stodsafe(jdetail["quantity"].asString())*multiplier;
    double priceOriginal = stodsafe(jdetail["retail_price"].asString())*multiplier;    

    int wleft = 23, wright = 18;
    if (_isSkinny)   {
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
    //  Display line detail(s)
    string tmp;
    double consolidatedQuantity=0;
    bool proceed = false;
    Json::Value jsplitt = jdetail["det_split"];
    Json::Value jproduct = jdetail["detail_product_header"];
    string specialSplit = jdetail["is_split"].asString();

    bool wholeNumber = false;
    double dseq = _util->stodsafe(seq);
    if (std::floor(dseq) == dseq)
        wholeNumber = true;

    proceed = false;
    consolidatedQuantity = -999.99;
    if (specialSplit == "1")  {
        if (jsplitt != Json::nullValue)  {
            if (wholeNumber)  {
                if (jsplitt.isMember("quantity"))  {
                    tmp = jsplitt["quantity"].asString();
                    consolidatedQuantity = stodsafe(tmp)*multiplier;
                    proceed = true;
                }
            }
        }
    }
    ///////////////////////////////////////////
    //  Display the line item
    if (consolidatedQuantity == -999.99)  {
        if (wholeNumber)  {
            proceed = true;
        }
        sprintf (sztmp, "%0.2f", quantity);
    }  else  {
        sprintf (sztmp, "%0.2f", consolidatedQuantity);
    }
    tmp  = sztmp;
    tmp += "  ";
    tmp += _util->string_cleanse(jdetail["description"].asString().c_str (),true);

    if (consolidatedQuantity == -999.99)
        sprintf (sztmp, "%0.2f", priceOriginal*quantity);
    else
        sprintf (sztmp, "%0.2f", priceOriginal*consolidatedQuantity);
    string fmtright = FMTNumberComma(atof(sztmp));
    if (proceed)  {
        _txtdet += _util->cyLRText(wleft+3,tmp.c_str(),wright-3, fmtright);
    }
    //////////////////////////////////////
    //  Item quantity / price
    if (jdetail["description"].asString() == "SPLIT CHECK ADJUSTMENT")  {
        sprintf (sztmp, "%.02f", priceOriginal * quantity);
        tmp = _util->cyLRText(wleft, jdetail["description"].asString(), wright, sztmp);
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
                jhdrtrx["is_pwd"].asString() == "1")  {
                itemSuffix = "NV";
            }  else if (jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "V";
            }  else  {
                itemSuffix = "T";
            }
        }
    }    
    //////////////////////////////////////
    //  Split adjustment
    if (_totalSplitAdjust > 0)  {
        //sprintf (sztmp, "%.02f", _totalSplitAdjust);
        //_txtdet += _util->cyLRText(23, "Split check adjustment", 18, sztmp);
        string desc  = "  Less split check ";
        sprintf (sztmp, "%.02f", (_totalSplitAdjust*1000)/1000);
        string fmtright = FMTNumberComma(atof(sztmp));
        tmp = _util->cyLRText(wleft+3, desc, wright-3, fmtright);
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

    _receipt += _txtdet;
    return true;
}
