//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptSunmi::assembleReceiptDetail (Json::Value& jdetail,
                                                          Json::Value& jhdrtrx,
                                                          salesReceiptType type)
{
    int w = 40;
    int multiplier = 1;
    if (type == salesReceiptType::SRT_POSTVOID)
        multiplier = -1;
    char sztmp[32],sztmp2[72];
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

    //FORMAT QTY ITEM CODE ITEM DESCRIPTION
    //string tmp = jdetail["quantity"].asString();
    string tmp = jdetail["quantity"].asString();
    //tmp  = jdetail["detail_product_header"]["item_code"].asString();
    //tmp += " ";


    tmp = _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    //tmp += jdetail["description"].asString().c_str ();
    if ((int)tmp.length() > (w-1))
        tmp = tmp.substr(0,(w-1));
    _txtdet += tmp;
    _txtdet += "\n";

    //FORMAT QTY @ ORIG PRICE LEFT SIDE
    string strOrigPrice = FMTNumberComma(priceOriginal);
    string itmBaseType =  jdetail["detail_product_header"]["base_type_code"].asString();
    sprintf (sztmp, "  %0.03f %s @ %s %s",quantity, "PC",strOrigPrice.c_str(),itmBaseType.c_str());

    //FORMAT TOTAL PRICE * QTY RIGHT SIDE
    string strTotalPrice = FMTNumberComma(priceOriginal*quantity);
    sprintf (sztmp2, "%s%s", strTotalPrice.c_str(),itemSuffix.c_str());

    tmp = _util->cyLRText(18, sztmp, 12, sztmp2);
    _txtdet += tmp;

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
        /*
        sprintf (sztmp, "%.02f%s", priceOriginal * quantity, itemSuffix.c_str ());
        _txtdet += sztmp;

        sprintf (sztmp, "  %.02f %s @.. %.02f", quantity, uom.c_str (), priceOriginal);
        _txtdet += sztmp;*/


    }

    //////////////////////////////////////////////////
    // modifier check
    if (jdetail.isMember("det_modifier"))  {
        if (jdetail["det_modifier"] != Json::nullValue)  {
            Json::Value jmain = jdetail["det_modifier"];
            Json::Value jsubmain,jmodifier,jmodifierdet;

            if (jmain != Json::nullValue) {
                if (jmain.isArray())  {
                    int x = jmain.size();
                    for ( int i = 0; i < x;i++){
                        jsubmain = jmain[i];
                        if (jsubmain.isMember("quantity"))  {
                            double dqty = _util->stodsafe(jsubmain["quantity"].asString());
                            if (dqty > 0.00)  {
                                std::string strqty = jsubmain["quantity"].asString();
                                std::string strdesc = jsubmain["modifier_subdesc"].asString();
                                _txtdet += "    ";
                                _txtdet += _util->cyPostSpace(6, strqty);
                                _txtdet += strdesc;
                                _txtdet += "\n";
                                //_txtdet += strdesc;
                                //_txtdet += " - ";
                                //_txtdet += strqty;
                                //_txtdet += "\n";
                            }
                        }
                    }
                }
            }
        }
    }
    //////////////////////////////////
    //  Is the item discounted?
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less (";
        Json::Value jdisc = jdetail["settings_detail_discount"];
        desc += jdisc["discount_cd"].asString();
        desc += ")";
        double dtmp = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
        if (jhdrtrx["is_sc"].asString() == "1" ||
            jhdrtrx["is_pwd"].asString() == "1" ||
            jhdrtrx["is_zero_rated"].asString() == "1")  {
            itemSuffix = "NV";
        }  else  {
            itemSuffix = "V";
        }
        if (quantity < 1.00)
            sprintf (sztmp, "%.02f", dtmp);
        else
            sprintf (sztmp, "%.02f", dtmp*quantity);
        if (dtmp != 0)  {
            tmp = _util->cyLRText(18, desc, 12, sztmp);
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
        tmp = _util->cyLRText(18, desc, 12, sztmp);
        _txtdet += tmp;
    }
    //////////////////////////////////
    //  Is there a service charge
    if (jdetail["total_service_charge"]!=Json::nullValue)  {
        string tsc = jdetail["total_service_charge"].asString();
        if (jdetail["is_auto_servicecharge"].asString() == "1")  {

            _totalServiceCharge += _util->stodsafe(tsc);
/*
            string desc = "  Add (";
            desc += jdetail["service_charge"].asString();
            desc += "% SC)";

            string szamt = "";
            string tsc = jdetail["total_service_charge"].asString();
            double dtmp = _util->stodsafe(tsc);
            sprintf (sztmp, "%.02f", dtmp);
            if (dtmp != 0)  {
                tmp = _util->cyLRText(28, desc, 12, sztmp);
                _txtdet += tmp;
            }*/
            /***
            double dtmp = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
            if (jhdrtrx["is_sc"].asString() == "1" ||
                jhdrtrx["is_pwd"].asString() == "1" ||
                jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "NV";
            }  else  {
                itemSuffix = "V";
            }
            if (quantity < 1.00)
                sprintf (sztmp, "%.02f", dtmp);
            else
                sprintf (sztmp, "%.02f", dtmp*quantity);
            if (dtmp != 0)  {
                tmp = _util->cyLRText(28, desc, 12, sztmp);
                _txtdet += tmp;
            }*/
        }
    }

    _receipt += _txtdet;
    return true;
}
