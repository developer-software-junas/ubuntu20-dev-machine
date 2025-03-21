//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRcc::assembleReceiptDetail (Json::Value& jdetail,
                                          Json::Value& jhdrtrx,
                                          salesReceiptType type)
{
    (void) type;
    //////////////////////////////////////
    //  Retrieve the item details
    string trxcode = jhdrtrx["transaction_cd"].asString();
    double quantity = stodsafe(jdetail["quantity"].asString());
    if (quantity < 0.00)  quantity = quantity * -1;
    double priceOriginal = stodsafe(jdetail["retail_price"].asString());

    string itemSuffix = "";
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
    //////////////////////////////////////
    //  Is the item discounted?
    double discountAmount = 0.00;


        discountAmount = _util->stodsafe(jdetail["item_discount_amount"].asString());
        discountAmount = (discountAmount*1000)/1000;
        if (jhdrtrx["is_sc"].asString() == "1" ||
            jhdrtrx["is_pwd"].asString() == "1" ||
            jhdrtrx["is_zero_rated"].asString() == "1")  {
            itemSuffix = "NV";
        }  else  {
            itemSuffix = "V";
        }
    //////////////////////////////////////
    //  Item description
    string itemDescLine;
    itemDescLine  = jdetail["detail_product_header"]["category_cd"].asString();
    itemDescLine += "-";
    itemDescLine += jdetail["detail_product_header"]["item_code"].asString();
    itemDescLine += " ";
    itemDescLine += _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    itemDescLine += "\n";
    _txtdet += itemDescLine;
    //////////////////////////////////////
    //  Item quantity / price
    string retailPrice="0.00";
    char szRight [40], szQuantity [32], szItemPriceLine[64];

    //if (type == SRT_POSTVOID)
        //sprintf (szQuantity, "-%0.2f", quantity);
    //else
        sprintf (szQuantity, "%0.2f", quantity);
    string baseTypeCode = jdetail["detail_product_header"]["base_type_code"].asString();

    if (trxcode == "44")  {
        double adjustedPrice = 0.00;

        if (quantity < 1.00)
            adjustedPrice = (priceOriginal*quantity)-(discountAmount*quantity);
        else
            adjustedPrice = priceOriginal - discountAmount;

        retailPrice = FMTNumberComma(adjustedPrice);
        string strTotalPrice = FMTNumberComma(adjustedPrice*quantity);

        sprintf (szItemPriceLine, "  %s PC @ %s %s", szQuantity,
                 retailPrice.c_str(), baseTypeCode.c_str());
        sprintf (szRight, "%s%s", strTotalPrice.c_str(),itemSuffix.c_str());

         _txtdet += _util->cyLRText(28, szItemPriceLine, 13, szRight);

    }  else  {

        retailPrice = FMTNumberComma(priceOriginal);
        sprintf (szItemPriceLine, "  %s PC @ %s %s", szQuantity,
                 retailPrice.c_str(), baseTypeCode.c_str());
        string strTotalPrice = FMTNumberComma(priceOriginal*quantity);
        //if (type == SRT_POSTVOID)  {
            //sprintf (szRight, "-%s%s", strTotalPrice.c_str(),itemSuffix.c_str());
        //}  else  {
            sprintf (szRight, "%s%s", strTotalPrice.c_str(),itemSuffix.c_str());
        //}

        _txtdet += _util->cyLRText(28, szItemPriceLine, 13, szRight);
        //////////////////////////////////
        //  Display the VAT
        if (jhdrtrx["is_zero_rated"].asString() == "1")  {
            string taxLine = "  Less VAT";
            double vatAmount = _util->stodsafe(jdetail["amtZero"].asString());
            sprintf (szRight, "%.02f", vatAmount);
            /***
            if (_isZeroRated)
                _txttot += _util->cyLRText(28,"LESS VAT 12%",13,szRight);
            else if (_isPwd || _isSenior)
                _txttot += _util->cyLRText(28,"LESS VAT",13,szRight);
             ***/
            _txtdet += _util->cyLRText(28, taxLine, 13, szRight);
        }
        //////////////////////////////////
        //  Display the discount
        if (discountAmount != 0.00)  {

            string discountAmt = "", discountLine = "  Less (";
            discountLine += jdetail["discount_code"].asString();
            discountLine += ")";

            if (quantity < 1.00)
                sprintf (szRight, "%.02f", discountAmount);
            else
                sprintf (szRight, "%.02f", discountAmount*quantity);
            _txtdet += _util->cyLRText(28, discountLine, 13, FMTNumberComma(_util->stodsafe(szRight)));

            Json::Value jdi;
            if (jdetail.isMember("det_discount"))  {
                jdi = jdetail["det_discount"];
                if (jdi.isMember("name"))  {
                    string name = jdi["name"].asString ();
                    if (name.length() > 0)  {
                        discountLine  = "  ";
                        discountLine += name;
                        discountLine += ", ";
                        discountLine += jdi["acct_type_code"].asString ();
                        discountLine += ", ";
                        discountLine += jdi["account_number"].asString ();
                        discountLine += "\n";
                        _txtdet += discountLine;
                    }
                }
            }

        }
    }

    _receipt += _txtdet;
    return true;
}

