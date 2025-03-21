//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::assembleReceiptClaimStub(Json::Value& jheader,
                                                           Json::Value& jdetail)
{
    if (_isGiftWrap) {
        string tmp,tmp2,strCShdr,strCSftr;
        char sztmp[32];
        Json::Value jtmp;
        Json::Value jcashier;
        jcashier = jheader["settings_cashier"];

        int w = g_widthReceipt;
        strCShdr  = "\n\n";
        strCShdr += _util->cyAcrossText(w, "=");
        strCShdr += _util->cyCenterText(w, "CLAIM STUB");
        strCShdr += _util->cyAcrossText(w, "=");

        tmp += jcashier["login"].asString();

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
                label  = jacct["last_name"].asString();
                label +=  ", ";
                label +=  jacct["first_name"].asString();
                break;
            }
        }

        strCShdr += _util->cyLRText(20,tmp.c_str(),20,label.c_str());

        jtmp = jheader["hdr_branch"];
        string trx_num = jheader["transaction_number"].asString();


        tmp  = jheader["register_number"].asString();
        tmp += "-";
        tmp += trx_num;

        //CONSTRUCT DATE IN RCC FORMAT
        string trans_date,dd,mm,yyyy;
        trans_date = jheader["transaction_date"].asString();
        _util->date_split(trans_date.c_str(),yyyy,mm,dd);
        tmp2  = mm;
        tmp2 += "/";
        tmp2 += dd;
        tmp2 += "/";
        tmp2 += yyyy;

        strCShdr += _util->cyLRText(20, tmp.c_str(),20,tmp2.c_str());

        tmp   = "Store#: ";
        tmp  += jtmp["location_code"].asString();
        tmp2  = "SI#: ";
        tmp2 += legacyTransnum(jheader);

        strCShdr += _util->cyLRText(15,tmp.c_str(),25,tmp2.c_str());

        tmp   = "Terminal No. :                ";
        tmp   += jheader["register_number"].asString();
        strCShdr += tmp.c_str();
        strCShdr += "\n";


        //RETRIVE SKU WITH G-WRAP
        Json::Value jline;
        int j = jdetail.size ();
        string gw_counter;
        int tQty = 0;
        for (int i = 0; i < j; i++)  {
            jline = jdetail[i];
            if (jline["gift_wrap_quantity"] != Json::nullValue)  {
                tmp = jline["gift_wrap_quantity"].asString ();
                if (tmp.length() > 0)  {
                    if (_util->valid_decimal("Gift wrap quantity", tmp.c_str (), 1, 8, 1.00, 9999.00))  {
                        double dqty = stodsafe(tmp);
                        tQty += dqty;
                        sprintf(sztmp,"%.02f",dqty);

                        Json::Value jproduct;
                        jproduct = jline["detail_product_header"];
                        string strGdet = "";

                        if (i == 0)
                            strCShdr += _util->cyPostSpace(25, "SKU :");
                        else
                            strCShdr += _util->cyPostSpace(25, " ");

                        strCShdr += _util->cyPreSpace (12, jline["item_code"].asString());
                        strCShdr += "\n";

                        if (jline["picture_url"] != Json::nullValue)
                            gw_counter = jline["picture_url"].asString();

                    }
                }
            }
        }

        sprintf(sztmp,"Total Package/Gifts :         %d",tQty);
        strCShdr += sztmp;
        strCShdr += "\n";

        strCShdr += "Total Amount:                 ";
        strCShdr += FMTNumberComma(_totalNetSales);
        strCShdr += "\n";

        strCShdr += "GW Counter No: ";
        strCShdr += gw_counter.c_str();
        strCShdr += "\n";


        //FOOTER CLAIM STUB
        strCSftr  = "Upon endorsement of items for wrapping\n";
        strCSftr += "customer acknowledges he/she has read\n";
        strCSftr += "and agrees to the following conditions;\n";
        strCSftr += "\n";
        strCSftr += "1. Customer shall be responsible for\n";
        strCSftr += "   safekeeping the Gift Claim Stub.\n";
        strCSftr += "   Packages/ Items shall be released\n";
        strCSftr += "   to anyone who presents the Gift\n";
        strCSftr += "   Claim Stub.\n";
        strCSftr += "\n";
        strCSftr += "2. Management shall not be responsible\n";
        strCSftr += "   to replace items due to the loss of\n";
        strCSftr += "   the Gift Claim Stub.\n";
        strCSftr += "\n";
        strCSftr += "3. Proof of ownership shall be\n";
        strCSftr += "   required for Customers who lost\n";
        strCSftr += "   their Gift Claim Stub/s before any\n";
        strCSftr += "   item may be released.\n";
        strCSftr += "\n";
        strCSftr += "4. Fifty Pesos (PhP. 50.00) shall be\n";
        strCSftr += "   charged for each lost Gift Claim\n";
        strCSftr += "   Stub.\n";
        strCSftr += "\n";
        strCSftr += "5. Gifts may only be claimed within\n";
        strCSftr += "   store hours. Management has the\n";
        strCSftr += "   discretion to charge for storage\n";
        strCSftr += "   and/ or forfeit unclaimed items.\n";
        strCSftr += "   Items shall be forfeited if\n";
        strCSftr += "   unclaimed after 30 days.\n";

        _txtgift += strCShdr.c_str();
        _txtgift+= "\n\n";
        _txtgift += strCSftr.c_str();
        _txtgift += RCPT_CUT;


    }
    _receipt += _txtgift;
    return true;
}
