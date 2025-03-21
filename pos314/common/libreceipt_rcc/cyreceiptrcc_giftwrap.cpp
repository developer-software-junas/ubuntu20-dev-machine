//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptGiftWrap
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::assembleReceiptGiftWrap (Json::Value& jheader,
                                                           Json::Value& jdetail)
{
    _txtgift = "";
    if (_isGiftWrap) {
        string tmp,tmp2,strGhdr,strGftr;
        char sztmp[32];
        Json::Value jtmp;
        int w = g_widthReceipt;
        strGhdr  = "\n\n";
        strGhdr += _util->cyAcrossText(w, "=");
        strGhdr += _util->cyCenterText(w, "GIFT RECEIPT");
        strGhdr += _util->cyAcrossText(w, "=");

        jtmp = jheader["hdr_branch"];
        tmp = "STORE # ";
        tmp += jtmp["location_code"].asString();
        jtmp = jheader["settings_register"];
        tmp += "  POS # ";
        tmp += jtmp["register_num"].asString();



        //CONSTRUCT DATE IN RCC FORMAT
        string trans_date,dd,mm,yyyy;
        trans_date = jheader["transaction_date"].asString();
        _util->date_split(trans_date.c_str(),yyyy,mm,dd);
        tmp2  = "Date: ";
        tmp2 += mm;
        tmp2 += "/";
        tmp2 += dd;
        tmp2 += "/";
        tmp2 += yyyy;

        strGhdr += _util->cyLRText(20, tmp.c_str(),20,tmp2.c_str());

        tmp = "SI Number ";
        tmp += legacyTransnum(jheader);

        //NEED TO ADD IN API
        string trx_num = jheader["transaction_number"].asString();
        tmp2 = "Txn: ";
        tmp2 += trx_num;

        strGhdr += _util->cyLRText(30,tmp.c_str(),10,tmp2.c_str());

        Json::Value jcashier;
        if (jheader["settings_cashier"] == Json::nullValue)
            return seterrormsg("Unable to get the cashier details");
        jcashier = jheader["settings_cashier"];

        size_t len = jcashier["first_name"].asString().length();
        len += jcashier["last_name"].asString().length();
        len += 14;
        len += jcashier["login"].asString().length();

        strGhdr += "Cashier: ";
        strGhdr += jcashier["first_name"].asString();
        strGhdr += " ";
        strGhdr += jcashier["last_name"].asString();
        if (len > 40)  {
            strGhdr += "\n           [";
            strGhdr += jcashier["login"].asString();
            strGhdr += "]\n";
        }  else  {
            strGhdr += "[";
            strGhdr += jcashier["login"].asString();
            strGhdr += "]\n";
        }

        ///////////////////////////
        //RETRIEVE SALES CLERK ONLY
        string label = "";
        Json::Value jacct, jaccount;
        jaccount = jheader["hdr_account"];
        int jj = jaccount.size();
        for (int i = 0; i < jj; i++)  {
            jacct = jaccount[i];
            if (jacct["is_clerk"].asString() == "1") {
                label = "Sales Clerk";
            }

            if (strlen(label.c_str()) > 0){
                label += ": ";
                label += jacct["last_name"].asString();
                label +=  ", ";
                label +=  jacct["first_name"].asString();
                label +=  "[";
                label +=  jacct["account_number"].asString();
                label +=  "]";
                break;
            }
        }

        strGhdr += label.c_str();
        strGhdr += "\n";

        strGhdr += _util->cyPostSpace(14, "Dept.");
        strGhdr += _util->cyPreSpace (12, "SKU");
        strGhdr += _util->cyPreSpace (14, "Qty");
        strGhdr += "\n";
        strGhdr += _util->cyAcrossText(40, "-");


        strGftr  = "\n\n\n";
        strGftr += _util->cyCenterText(w, "Important!!!");
        strGftr += _util->cyCenterText(w, "Present Gift receipt to exchange");
        strGftr += _util->cyCenterText(w, "within 15 days from purchase.  Goods");
        strGftr += _util->cyCenterText(w, "are not refundable for cash and");
        strGftr += _util->cyCenterText(w, "Rustans will not accept used,");
        strGftr += _util->cyCenterText(w, "damaged or altered goods for");
        strGftr += _util->cyCenterText(w, "exchange.");
        strGftr += "\n\n\n\n\n\n";


        Json::Value jline;
        int j = jdetail.size ();
        for (int i = 0; i < j; i++)  {
            jline = jdetail[i];
            if (jline["gift_wrap_quantity"] != Json::nullValue)  {
                tmp = jline["gift_wrap_quantity"].asString ();
                if (tmp.length() > 0)  {
                    if (_util->valid_decimal("Gift wrap quantity", tmp.c_str (), 1, 8, 1.00, 9999.00))  {
                        double dqty = stodsafe(tmp);
                        sprintf(sztmp,"%.02f",dqty);

                        Json::Value jproduct;
                        jproduct = jline["detail_product_header"];
                        string strGdet = "";

                        for (int ii=0; ii<dqty; ii++) {
                            strGdet  = _util->cyPostSpace(14, jproduct["category_cd"].asString());
                            strGdet += _util->cyPreSpace (12, jline["item_code"].asString());
                            strGdet += _util->cyPreSpace (14, "1");

                            _txtgift += strGhdr.c_str();
                            _txtgift += strGdet.c_str();
                            _txtgift += strGftr.c_str();
                            _txtgift += RCPT_CUT;
                        }
                    }
                }
            }
        }
    }
    _receipt += _txtgift;
    return true;
}
