//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrico.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRico::assembleReceiptDetail (Json::Value& jdetail,
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
    string iniarg = cgiform("ini");
    //////////////////////////////////////
    //  Void multiplier?
    int multiplier = 1;
    if (type == salesReceiptType::SRT_POSTVOID)
        multiplier = -1;
    //////////////////////////////////////
    //  Quantity and price
    double quantity = stodsafe(jdetail["quantity"].asString())*multiplier;
    double priceOriginal = stodsafe(jdetail["retail_price"].asString())*multiplier;

    int wleft = 23, wright = 18;
    if (skinny == "true")   {
        wleft = 19;
        wright = 12;
    }
    //////////////////////////////////////
    //  Item suffix
    string itemSuffix="";
    /*
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
    }*/
    //////////////////////////////////////
    //  Gift wrap flag
    _isGiftWrap = (stodsafe(jdetail["gift_wrap_quantity"].asString()) > 0);
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
    //  Transaction discount
    double dlinedisc = 0.00;
    double dsubtot = priceOriginal*quantity;
    //////////////////////////////////////
    //  Transaction discount
    strTotalPrice = FMTNumberComma(priceOriginal*quantity);

    if (jhdrtrx["is_sc"].asString() == "1" ||
        jhdrtrx["is_pwd"].asString() == "1" ||
        jhdrtrx["is_zero_rated"].asString() == "1") {
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
    char sztmp[512];
    string tmp;
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
    //////////////////////////////////
    //  GC sale?
    if (jhdrtrx["is_internal"].asString() == "1") {
      _txtdet += "  GIFT CARD# ";
      _txtdet += jdetail["gc_number"].asString();
      _txtdet += "\n";
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
    /*
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less (";
        Json::Value jdisc = jdetail["settings_detail_discount"];
        desc += jdisc["discount_cd"].asString();
        desc += ")";
        double dtmp = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
        sprintf (szright, "%.02f", dtmp);
        if (iniarg == "east" && dtmp != 0)  {
            _txtdet += _util->cyLRText(left, desc, right, szright);
        }
        if (quantity < 1.00)
            sprintf (szright, "%.02f", dtmp);
        else
            sprintf (szright, "%.02f", dtmp*quantity);
        if (dtmp != 0)  {
            _txtdet += _util->cyLRText(left, desc, right, szright);
        }
    }*/
    //////////////////////////////////////
    //  Split adjustment
    if (_totalSplitAdjust > 0)  {
        string desc  = "  Less split check ";
        sprintf (szright, "%.02f", (_totalSplitAdjust*1000)/1000);
        _txtdet += _util->cyLRText(left, desc, 12, szright);
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
