//*******************************************************************
//        FILE:     cyrestsales_receipt_rcc.cpp
// DESCRIPTION:     Sales receipt generation daily receipt
//*******************************************************************
#include "cyrestreceipt.h"
#include "cyrestreceiptrcc.h"
using std::string;
#define RCPT_CUT "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+"
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleFspUrl(Json::Value& jheader,
                                      Json::Value& jdetail,
                                      Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceipt(Json::Value& jheader,
                                       Json::Value& jdetail,
                                       Json::Value& jpayment,
                                       salesReceiptType receiptType)
{
    _isEpurse = false;
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;
    //FOR TEST ONLY-OK
    //std::stringstream ss;
    //ss <<jheader;
    //string msg = ss.str();
    //_log->logmsg("DEBUG RCC RECEIPT JSON ",msg.c_str());
    Json::Value jline;
    _txtdet = "";
    int j = jdetail.size ();
    for (int i = 0; i < j; i++)  {
        jline = jdetail[i];
        Json::Value jhdrtrx = jheader["settings_transtype"];
        if (!assembleReceiptDetail(jline,jhdrtrx,receiptType))
            return false;
        jdetail[i]=jline;
    }
    
    if (!assembleReceiptTotals(jheader,receiptType))
        return false;
    
    if (!assembleReceiptPayment(jheader,jpayment,receiptType))
        return false;
    
    if (!assembleReceiptAccount(jheader,receiptType))
        return false;
    
    if (!assembleVatBreakdown(receiptType))
        return false;
    
    if (!assembleReceiptSignature(jheader,receiptType))
        return false;
    
    if (!assembleReceiptFooter(jheader,receiptType))
        return false;
    
    if (!assembleReceiptGiftWrap(jheader,jdetail))
        return false;
    
    if (!assembleReceiptClaimStub(jheader,jdetail))
        return false;

    if (!assembleReceiptPromoItem(jheader,jdetail))
        return false;

    _log->logmsg("DEBUG RCC RECEIPT RECEIPT ",_receipt.c_str());

    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptGiftWrap
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleReceiptClaimStub(Json::Value& jheader,
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

//*******************************************************************
//    FUNCTION:     assembleReceiptGiftWrap
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleReceiptGiftWrap (Json::Value& jheader,
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


//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleReceiptFooter (Json::Value& jheader,
                                                             salesReceiptType receiptType)
{
    _txtfoot = "";
    string tmp = "",label = "";
    if (jheader["hdr_settings"]==Json::nullValue)
        return seterrormsg("Unable to retrieve the POS settings information");
    
    Json::Value jpos,jcashier,jaccount,jacct;
    int w = g_widthReceipt;
    jpos = jheader["hdr_settings"];
    
    if (receiptType != salesReceiptType::SRT_NORMAL &&
            receiptType != salesReceiptType::SRT_REPRINT &&
            receiptType != salesReceiptType::SRT_DAILY)  {
        
        _txtfoot += _util->cyCenterText(w, "THIS DOES NOT SERVE AS AN INVOICE");
        _txtfoot += "\n\n";
        
        _txtfoot += _util->cyCenterText(w, "THIS DOCUMENT IS NOT VALID");
        _txtfoot += _util->cyCenterText(w, "FOR CLAIM OF INPUT TAX");
        
    }  else  {
        //////////////////////////////////////
        //  Database connection
        CYDbSql* db = dbconn ();
        if (!db->ok())
            return seterrormsg(db->errordb());

        //////////////////////////////////////
        //  pos gift item footer ***///
        db->sql_reset ();
        string value,align;
        size_t f;

        string sql  = "select * from pos_giftitem_promo_rcpt where gift_code = 888; ";
        _log->logmsg("footer start select", sql.c_str ());

        if (!db->sql_result (sql, true))  {
            _log->logmsg("footer error", sql.c_str ());
            return seterrormsg(db->errordb ().c_str ());
        }
        if (!db->eof ())  {
            string permit_no = jheader["settings_register"]["permit"].asString();
            do {
                value = db->sql_field_value("rcpt_line");
                align = db->sql_field_value("is_center");
                if (value.length () < 1) value = "";

                f = value.find("<PERMIT-NO>");

                if (f != std::string::npos)
                    value.replace(f,11,permit_no.c_str());

                if (strcmp(align.c_str(),"1") == 0)
                    _txtfoot += _util->cyCenterText(w,value.c_str());
                else{
                    _txtfoot += value.c_str();
                    _txtfoot += "\n";
                }

                db->MoveNext();
            } while (!db->eof());
        }
    }
    
    if (receiptType == SRT_REFUND)  {
        _txtfoot += "Customer Name :________________________\n";
        _txtfoot += "Address :______________________________\n";
        _txtfoot += "Phone No. :____________________________\n\n";
        _txtfoot += "Approving Manager :____________________\n\n\n\n";
        
        _txtfoot += "     Return and Exchange Slip (RES)\n";
        _txtfoot += "----------------------------------------\n";
        _txtfoot += "Please present RES upon exchange of item\n";
        _txtfoot += "         For one-time use only\n";
        _txtfoot += "       Not replaceable when lost\n";
        _txtfoot += "Can be used in Rustan`s Department Store\n";
        _txtfoot += "                  only\n";
    }
    
    
    
    if (receiptType != SRT_POSTVOID) {
        if (jpos["szmsg01"].asString().length() > 0)  {
            _txtfoot +=  _util->cyCenterText(w, jpos["szmsg01"].asString());
        }
        if (jpos["szmsg02"].asString().length() > 0)  {
            _txtfoot += _util->cyCenterText(w, jpos["szmsg02"].asString());
        }
        if (jpos["szmsg03"].asString().length() > 0)  {
            _txtfoot += _util->cyCenterText(w, jpos["szmsg03"].asString());
        }
        if (jpos["szmsg04"].asString().length() > 0)  {
            _txtfoot += _util->cyCenterText(w, jpos["szmsg04"].asString());
        }
        
        if (jpos["szmsg05"].asString().length() > 0)  {
            _txtfoot += _util->cyCenterText(w, " ");
            _txtfoot += _util->cyCenterText(w, jpos["szmsg05"].asString());
        }
    }

    _txtfoot += _util->cyCenterText(40,"--+++--");
    _txtfoot += RCPT_CUT;
    
    if (receiptType != SRT_SUSPEND &&
        receiptType != SRT_POSTVOID &&
        receiptType != SRT_CANCEL){
        if (_isFspMember || _isBaMember) {

            bool isFsp = false,isBeauty = false;
            jaccount = jheader["hdr_account"];

            int j = jaccount.size();

            //RETRIVE BA
            for (int i = 0; i < j; i++)  {
                jacct = jaccount[i];

                if (jacct["is_beauty_addict"].asString() == "1") {
                    isBeauty = true;
                }

                if (isBeauty){

                    _txtfoot += "\n\n\n";
                    label = "========= BEAUTY CARD DETAILS =========";
                    _txtfoot += _util->cyCenterText(40,label.c_str());

                    label  = "Account Number: ";
                    label += jacct["account_number"].asString();
                    _txtfoot += label.c_str();
                    _txtfoot += "\n";

                    label  = "Member Name: ";
                    label += jacct["first_name"].asString();
                    label += " ";
                    label += jacct["middle_name"].asString();
                    label += " ";
                    label += jacct["last_name"].asString();
                    _txtfoot += label.c_str();
                    _txtfoot += "\n";
                    _txtfoot += _util->cyAcrossText(40,"=");

                    break;
                }

            }

            //RETRIVE FSP DETAILS
            for (int i = 0; i < j; i++)  {
                jacct = jaccount[i];
                if (jacct["is_loyalty"].asString() == "1") {
                    isFsp = true;
                }

                if (isFsp){
                    string totalPoints = jacct["points_balance"].asString();
                    string totalEPoints = jacct["epurse_balance"].asString();

                    _txtfoot += "\n\n\n";
                    label = "============= FSP DETAILS =============";
                    _txtfoot += _util->cyCenterText(40,label.c_str());

                    label  = "Account Number: ";
                    label += jacct["account_number"].asString();
                    _txtfoot += label.c_str();
                    _txtfoot += "\n";

                    label  = "Member Name: ";
                    label += jacct["first_name"].asString();
                    label += " ";
                    label += jacct["middle_name"].asString();
                    label += " ";
                    label += jacct["last_name"].asString();
                    _txtfoot += label.c_str();
                    _txtfoot += "\n\n";

                    label  = "TOTAL PURCHASE: ";
                    tmp = FMTNumberComma(_totalNetSales);
                    _txtfoot += _util->cyLRText(20, label.c_str(),20,tmp.c_str());

                    //CONSTRUCT DATE IN RCC FORMAT
                    string trans_time,trans_date,dd,mm,yyyy;
                    trans_date = jheader["transaction_date"].asString();
                    trans_time = jheader["transaction_time"].asString();
                    _util->date_split(trans_date.c_str(),yyyy,mm,dd);
                    label  = mm;
                    label += "/";
                    label += dd;
                    label += "/";
                    label += yyyy;
                    label += "-";
                    label += _util->time_colon(trans_time.c_str());

                    tmp = "TERM NO. ";
                    tmp += jheader["register_number"].asString();
                    _txtfoot += _util->cyLRText(20, label.c_str(),20,tmp.c_str());

                    //NEED DATA IN API
                    string trx_num = jheader["transaction_number"].asString();
                    label  = "TRANS NO. ";
                    label += trx_num;


                    jcashier = jheader["settings_cashier"];

                    tmp  = jcashier["last_name"].asString();
                    tmp +=  ", ";
                    tmp +=  jcashier["first_name"].asString();
                    _txtfoot += _util->cyLRText(20, label.c_str(),20,tmp.c_str());
                    _txtfoot += _util->cyAcrossText(40,"=");

                    break;
                }
            }

        }
    }
    
    _txtfoot += _util->cyCenterText(w, " ");
    _receipt += _txtfoot;


    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleReceiptSignature (Json::Value& jheader,
                                                                salesReceiptType receiptType)
{

   if (receiptType == salesReceiptType::SRT_SUSPEND )
        return 0;

    char szTmp[255];
    _txtsig = "";
    _isBaMember = false;
    _isFspMember = false;
    Json::Value jacct,jtemp,jtemp2;
    string tmp3="",tmp2="",newLine = "\n";
    //////////////////////////////////////
    //CHECK IF FSP OR BA
    string info, label,tmp,txtacct2 = "";
    bool isFsp = false;
    bool isBeauty = false;
    bool isLine = false;
    Json::Value jaccount,jcashier;
    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    
    int j = jaccount.size();
    
    //RETRIVE FSP DETAILS
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_loyalty"].asString() == "1") {
            isFsp = true;

        }
        
        if (isFsp){
            string totalPoints = jacct["points_balance"].asString();
            string totalEPoints = jacct["epurse_balance"].asString();
            
            label = "============= FSP DETAILS =============";
            _txtsig += _util->cyCenterText(40,label.c_str());
            
            label  = "Account Number: ";
            label += jacct["account_number"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            label  = "Member Name: ";
            label += jacct["first_name"].asString();
            label += " ";
            label += jacct["middle_name"].asString();
            label += " ";
            label += jacct["last_name"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            //sprintf(szTmp,"%0d",atoi(totalPoints.c_str()));
            label  = "Current Available Points: ";
            label += _util->fmt_number_comma(totalPoints.c_str());
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            sprintf(szTmp,"%d",atoi(totalEPoints.c_str()));
            label  = "Current Available E-Purse: ";
            label += _util->fmt_number_comma(szTmp);
            label += ".00";
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            isFsp = false;
            isLine = true;
            _isFspMember = true;
        }
    }
    //RETRIVE BA
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        
        if (jacct["is_beauty_addict"].asString() == "1") {
            isBeauty = true;
        }
        
        if (isBeauty){
            string totalPoints = jacct["points_balance"].asString();
            string pointsEarned = jacct["points_earned"].asString();
            
            _txtsig += "\n";
            label = "========= BEAUTY CARD DETAILS =========";
            _txtsig += _util->cyCenterText(40,label.c_str());
            
            label  = "Account Number: ";
            label += jacct["account_number"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            label  = "Member Name: ";
            label += jacct["first_name"].asString();
            label += " ";
            label += jacct["middle_name"].asString();
            label += " ";
            label += jacct["last_name"].asString();
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            label  = "Current Earned Points: ";
            label += _util->fmt_number_comma(pointsEarned.c_str());
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            label  = "Total Earned Points: ";
            label += _util->fmt_number_comma(totalPoints.c_str());
            _txtsig += label.c_str();
            _txtsig += "\n";
            
            isBeauty = false;
            isLine = true;
            _isBaMember = true;
            
        }
        
    }
    
    if (isLine)
        _txtsig += _util->cyAcrossText(40,"=");
    
    //END OF FSP AND BA
    
    if (receiptType != salesReceiptType::SRT_POSTVOID) {
        //IF CUSTOMER IS TAG
        string fspTagMobile = jheader["crm_mobile"].asString();
        if(strlen(fspTagMobile.c_str()) > 0){
            _txtsig += newLine.c_str();
            _txtsig += "I have given full consent to receive\n";
            _txtsig += "communication for the purpose of being\n";
            _txtsig += "invited to join the Rustan's loyalty\n";
            _txtsig += "program and receiving information about\n";
            _txtsig += "marketing programs from Rustan's.\n";
            _txtsig += newLine.c_str();
        }
        
        jacct = jheader["hdr_tax_receipt"];
        jtemp = jheader["hdr_trxaccount"];
        
        jtemp2 = jheader["hdr_trxaccount"];
        tmp3 = jtemp2["first_name"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["last_name"].asString();
        
        if (jtemp2["first_name"].asString().length() < 1 && jtemp2["last_name"].asString().length() < 1){
            _txtsig += "Customer Name:   _______________________\n";
            _txtsig += "Address:         _______________________\n";
            _txtsig += "TIN:             _______________________\n";
            _txtsig += "Business Style:  _______________________\n";
            _txtsig += newLine.c_str();
        } else {
            _txtsig += newLine.c_str();
            
            if (tmp3.length()+17 <= 40){
                tmp2 = "Customer name:   ";
                tmp2 += tmp3.c_str();
                tmp2 += newLine.c_str();
                
                _txtsig += tmp2.c_str();
            } else {
                tmp2 = "Customer name:   ";
                tmp2 += jtemp2["first_name"].asString();
                tmp2 += newLine.c_str();
                tmp2 += jtemp2["last_name"].asString();
                tmp2 += newLine.c_str();
                
                _txtsig += tmp2.c_str();
            }
            
            jtemp2 = jheader["hdr_tax_receipt"];
            tmp3 = jtemp2["address1"].asString();
            tmp3 += " ";
            tmp3 += jtemp2["address2"].asString();
            
            if (jtemp2["address1"].asString().length() < 1 && jtemp2["address2"].asString().length() < 1){
                _txtsig += "Address: _______________________________";
                _txtsig += newLine.c_str();
            } else if (tmp3.length() + 9 <=40){
                _txtsig += "Address: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Address: ";
                _txtsig += jtemp2["address1"].asString();
                _txtsig += newLine.c_str();
                _txtsig += jtemp2["address2"].asString();
                _txtsig += newLine.c_str();
            }
            
            tmp3 = jtemp2["tin"].asString();
            if (tmp3.length() < 1) {
                _txtsig += "TIN : __________________________________";
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "TIN: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            }
            
            tmp3 = jtemp2["business_style"].asString();
            
            if (tmp3.length() < 1) {
                _txtsig += "Business style:_________________________";
                _txtsig += newLine.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Business style: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
                _txtsig += newLine.c_str();
            }
        }
        
        
        jtemp2 = jheader["header_transtype_discount"];
        
        if (jtemp["is_senior"].asString() == "1" || jtemp2["discount_code"].asString() == "SENIOR") {
            
            jtemp2 = jheader["hdr_trxaccount"];
            tmp3 = jtemp2["first_name"].asString();
            tmp3 += " ";
            tmp3 += jtemp2["last_name"].asString();
            
            _txtsig += _util->cyCenterText(40,"DISCOUNT CLAIM");
            _txtsig += newLine.c_str();
            _txtsig += "Signature: _____________________________\n";
            _txtsig += "Discount: ";
            
            jtemp2 = jheader["header_transtype_discount"];
            _txtsig += jtemp2["description"].asString();
            _txtsig += newLine.c_str();
            
            if (tmp3.length()+6 <=40){
                _txtsig += "Name: ";
                _txtsig += tmp3.c_str();
                _txtsig += newLine.c_str();
            } else {
                _txtsig += "Name: ";
                _txtsig += jtemp2["first_name"].asString();
                _txtsig += newLine.c_str();
                _txtsig += jtemp2["last_name"].asString();
                _txtsig += newLine.c_str();
            }
            _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n";
        }
    }
    
    _receipt += _txtsig;
    return true;
}


//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleVatBreakdown (salesReceiptType receiptType)
{
    _txtvat = "";
    int w = g_widthReceipt;
    
    if (receiptType == salesReceiptType::SRT_SUSPEND )
        return 0;
    
    _txtvat += "VAT Breakdown:";
    _txtvat += "\n";
    _txtvat += _util->cyPostSpace(13, "VAT Breakdown");
    _txtvat += _util->cyPreSpace(15, "Net Sale");
    _txtvat += _util->cyPreSpace(12, "Amount");
    _txtvat += "\n";
    
    //////////////////////////////////////
    //  VAT amounts
    char szNetVat[32]; char szNetZero[32]; char szNetExempt[32];
    char szAmtVat[32]; char szAmtZero[32]; char szAmtExempt[32];
    
    if (receiptType == SRT_REFUND || receiptType == SRT_POSTVOID)  {
        if (_totalNetVat != 0)
            sprintf (szNetVat, "%s", FMTNumberComma((_totalNetVat * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetVat, "%s", FMTNumberComma((_totalNetVat * 1000) / 1000).c_str());
        if (_totalNetZero != 0)
            sprintf (szNetZero, "%s", FMTNumberComma((_totalNetZero * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetZero, "%s", FMTNumberComma((_totalNetZero * 1000) / 1000).c_str());
        if (_totalNetExempt != 0)
            sprintf (szNetExempt, "%s", FMTNumberComma((_totalNetExempt * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetExempt, "%s", FMTNumberComma((_totalNetExempt * 1000) / 1000).c_str());
        
        if (_totalAmtVat != 0)
            sprintf (szAmtVat, "%s", FMTNumberComma((_totalAmtVat * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtVat,  "%s", FMTNumberComma((_totalAmtVat * 1000) / 1000).c_str());
        if (_totalAmtZero != 0)
            sprintf (szAmtZero, "%s", FMTNumberComma((_totalAmtZero * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtZero, "%s", FMTNumberComma((_totalAmtZero * 1000) / 1000).c_str());
        if (_totalAmtExempt != 0)
            sprintf (szAmtExempt, "%s", FMTNumberComma((_totalAmtExempt * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtExempt, "%s", FMTNumberComma((_totalAmtExempt * 1000) / 1000).c_str());
        
        _txtvat += _util->cyPostSpace(12, "VATable");
        _txtvat += _util->cyPreSpace(16, szNetVat);
        _txtvat += _util->cyPreSpace(12, szAmtVat);
        _txtvat += "\n";
        
        _txtvat += _util->cyPostSpace(12, "VAT-Exempt");
        _txtvat += _util->cyPreSpace(16, szNetExempt);
        _txtvat += _util->cyPreSpace(12, "0.00");//szAmtExempt);
        _txtvat += "\n";
        
        _txtvat += _util->cyPostSpace(15, "Zero-Rated VAT");
        _txtvat += _util->cyPreSpace(13, szNetZero);
        _txtvat += _util->cyPreSpace(12, "0.00");//szAmtZero);
        _txtvat += "\n";
        
    }  else  {
        
        _txtvat += _util->cyPostSpace(12, "VATable");
        _txtvat += _util->cyPreSpace(16, FMTNumberComma((_totalNetVat * 1000) / 1000));
        _txtvat += _util->cyPreSpace(12, FMTNumberComma((_totalAmtVat * 1000) / 1000));
        _txtvat += "\n";
        
        _txtvat += _util->cyPostSpace(12, "VAT-Exempt");
        _txtvat += _util->cyPreSpace(16,  FMTNumberComma((_totalNetExempt * 1000) / 1000));
        _txtvat += _util->cyPreSpace(12, "0.00");//szAmtExempt);
        _txtvat += "\n";
        
        _txtvat += _util->cyPostSpace(15, "Zero-Rated VAT");
        _txtvat += _util->cyPreSpace(13, FMTNumberComma((_totalNetZero * 1000) / 1000));
        _txtvat += _util->cyPreSpace(12, "0.00");//szAmtZero);
        _txtvat += "\n";
        
    }
    
    
    
    double totalNetVat = _totalNetVat + _totalNetZero + _totalNetExempt;
    char szTotalNetVat[32];
    
    if (totalNetVat != 0 && receiptType != SRT_POSTVOID)
        sprintf (szTotalNetVat, "%s", FMTNumberComma((totalNetVat * 1000) / 1000).c_str());
    else {
        if (totalNetVat == 0)
            strcpy (szTotalNetVat, "0.00");
        else
            sprintf (szTotalNetVat, "%s", FMTNumberComma((totalNetVat * 1000) / 1000 * -1).c_str());
    }
    
    
    double totalAmtVat = _totalAmtVat;// + _totalAmtZero + _totalAmtExempt;
    char szTotalAmtVat[32];
    if (totalAmtVat != 0 && receiptType != SRT_POSTVOID)
        sprintf (szTotalAmtVat, "%s", FMTNumberComma((totalAmtVat * 1000) / 1000).c_str());
    else{
        if (totalAmtVat == 0)
            strcpy (szTotalAmtVat, "0.00");
        else
            sprintf (szTotalAmtVat, "%s", FMTNumberComma((totalNetVat * 1000) / 1000 * -1).c_str());
    }
    
    _txtvat += _util->cyPostSpace(13, "Total");
    _txtvat += _util->cyPreSpace (15, szTotalNetVat);
    _txtvat += _util->cyPreSpace (12, szTotalAmtVat);
    _txtvat += "\n";
    _txtvat += _util->cyAcrossText(w,"-");
    _receipt += _txtvat;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceiptAccount (Json::Value& jheader, salesReceiptType receiptType)
{
    _txtacct = "";
    string info, label,label2,tmp;
    string today;
    _util->int_date(today,0);

    Json::Value jacct, jaccount,jcashier;
    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    jcashier = jheader["settings_cashier"];
    
    tmp = "Cashier:  ";
    tmp += jcashier["last_name"].asString();
    tmp +=  ", ";
    tmp +=  jcashier["first_name"].asString();
    tmp +=  "[";
    tmp +=  jcashier["login"].asString();
    tmp +=  "]";
    
    _txtacct += tmp.c_str();
    _txtacct += "\n";
    
    
    int j = jaccount.size();
    
    //RETRIEVE SALES CLERK ONLY
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_clerk"].asString() == "1") {
            label = "Sales Clerk";
        }
        
        if (strlen(label.c_str()) > 0){
            label += ":  ";
            label += jacct["last_name"].asString();
            label +=  ", ";
            label +=  jacct["first_name"].asString();
            label +=  "[";
            label +=  jacct["account_number"].asString();
            label +=  "]";
            
            _txtacct += label.c_str();
            _txtacct += "\n";
        }
        
        label = "";
    }
    
    //IF PSHOPPER ONLY
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
         if (jacct["is_shopper"].asString() == "1") {
            label = "PShopper ID";
        }
        
        
        if (strlen(label.c_str()) > 0){
            label2  = label.c_str();
            label2 += ":  ";
            label2 += jacct["last_name"].asString();
            label2 +=  ", ";
            label2 +=  jacct["first_name"].asString();
            label2 +=  "[";
            label2 +=  jacct["account_number"].asString();
            label2 +=  "]";

            if (strlen(label2.c_str()) > 40) {

                tmp  = label.c_str();
                tmp += ":  ";
                tmp += jacct["last_name"].asString();
                tmp +=  ", ";
                tmp +=  jacct["first_name"].asString();

                _txtacct += tmp.c_str();
                _txtacct += "\n";

                tmp  =  "[";
                tmp +=  jacct["account_number"].asString();
                tmp +=  "]";

                _txtacct += tmp.c_str();
                _txtacct += "\n";
            } else {
                _txtacct += label2.c_str();
                _txtacct += "\n";
            }
            
            string ridc_number = jheader["ridc_number"].asString();
            if (strlen(ridc_number.c_str()) > 0) {
                label =  "RIDC #: ";
                label += jheader["ridc_number"].asString();
                
                _txtacct += label.c_str();
                _txtacct += "\n";
            }
        }
        
        label = "";
        
    }
    
    if (receiptType == salesReceiptType::SRT_POSTVOID || receiptType == salesReceiptType::SRT_SUSPEND){

        info  = "Txn No. : ";
        info += jheader["transaction_number"].asString();
        _txtacct += info.c_str();
        _txtacct += "\n";
    }
    
    string business_date = jheader["settings_cashier_shift"]["transaction_date"].asString();
    info  = "Business Date:  ";
    info += business_date.c_str();
    _txtacct += info.c_str();
    _txtacct += "\n";


    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconn ();
    if (!db->ok())
        return seterrormsg(db->errordb());

    //////////////////////////////////////
    //GET IPAD DEAILS
    string device_name,sql;
    db->sql_reset ();
    if (receiptType != salesReceiptType::SRT_POSTVOID){
        sql  = "select register_num from tg_pos_mobile_alias where alias_systransnum = ";
        sql += db->sql_bind(1,jheader["systransnum"].asString());
    } else {
        sql  = "select register_num from tg_pos_mobile_alias where systransnum = ";
        sql += db->sql_bind(1,jheader["systransnum"].asString());
    }

    if (!db->sql_result (sql, true))
        return seterrormsg(db->errordb());
    if (!db->eof ()){
        device_name = db->sql_field_value("register_num");
        if (device_name.size() < 1)
            device_name = "000";
    }


    info  = "Device ID:  IPAD-";
    info += device_name.c_str();
    _txtacct += info.c_str();
    _txtacct += "\n";
    
    if (receiptType == salesReceiptType::SRT_POSTVOID){
        Json::Value jHdrSetReg = jheader["settings_register"];
        Json::Value jHdrSetCshrShift = jheader["settings_cashier_shift"];
        
        char sztmp[24];
        long lRegister = atol (jHdrSetReg["register_num"].asString().c_str ());
        long lStoreLocation = atol (jHdrSetCshrShift["location_code"].asString().c_str ());
        sprintf (sztmp, "%02ld-%03ld", lStoreLocation,lRegister);
        
        tmp = "Original SI #: ";
        tmp += sztmp;
        tmp += "-";
        long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
        sprintf (sztmp, "%09ld", ltrx);
        tmp += sztmp;
        
        _txtacct += tmp.c_str();
        _txtacct += "\n";
    }
    
    if (receiptType != SRT_SUSPEND)
        _txtacct += _util->cyAcrossText(40,"-");
    
    
    _receipt += _txtacct;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceiptPayment (Json::Value& jheader,
                                               Json::Value& jpayment,
                                               salesReceiptType receiptType)
{
    string tmp;
    char sztmp[32];
    bool isGc = false;
    //bool isCheck = false;
    //bool isDebit = false;
    //bool isCharge = false;
    bool isCreditCard = false;
    _isEpurse = false;
    
    
    _txtpay = "";
    Json::Value jtmp, jpay;
    double totalPaymentAmount = 0.00;
    size_t j = jpayment.size();
    
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];
        
        tmp = jpay["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, 0, 999999999))
            return seterrormsg(_util->errormsg());
        double paymentAmount = stodsafe(jpay["payment_amount"].asString());
        //////////////////////////////////
        //  Discounted payment???
        if (jpay["payment_discount"]!=Json::nullValue)  {
            Json::Value jdiscount = jpay["payment_discount"];
            //////////////////////////////////////
            //  Calculate the item discount
            double discount = 0.00;
            if (jdiscount["discount_code"].asString().length() > 0) {
                tmp = jdiscount["discount_value"].asString ();
                if (!_util->valid_decimal("Payment discount value", tmp.c_str (), 1, 24, 0, 999999999))
                    return seterrormsg(_util->errormsg());
                double discountValue = stodsafe(jdiscount["discount_value"].asString ());
                if (jdiscount["discount_type"] != Json::nullValue)  {
                    tmp = jdiscount["discount_type"].asString();
                    if (tmp == "1") {
                        discountValue = (discountValue / 100);
                        discount = paymentAmount * discountValue;
                    } else {
                        discount = discountValue;
                    }
                }
                paymentAmount = paymentAmount - discount;
            }
        }
        totalPaymentAmount += paymentAmount;
        //////////////////////////////////////
        //  Special payment information
        //isGc = false;
        //isCheck = false;
        //isDebit = false;
        //isCharge = false;
        //isCreditCard = false;
        
        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isGc = jtmp["is_gc"].asString() == "1";
        //isCheck = jtmp["is_check"].asString() == "1";
        //isCharge = jtmp["is_charge"].asString() == "1";
        //isDebit = jtmp["is_debit_card"].asString() == "1";
        isCreditCard = jtmp["is_credit_card"].asString() == "1";
        
        _isEpurse = jtmp["is_epurse"].asString() == "1";
        ////////////////////////////////////
        //  GC special processing
        if (isGc) {
            if (jpay["gc"] == Json::nullValue)
                return seterrormsg("Unable to process the GC payment information");
            
            
            Json::Value gc, jgc = jpay["gc"];
            ////////////////////////////
            //  GC payment(s)
            if (isGc) {
                tmp = FMTNumberComma(paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, tmp.c_str());
                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                    gc.clear ();
                    gc = jgc[i];
                    _txtpay += gc["description"].asString();
                    //_txtpay += "\n";
                    _txtpay += _util->cyLRText(23, "  GC SERIES", 18, gc["gcnum"].asString());
                    
                    double dtmp = stodsafe(gc["amount_paid"].asString());
                    tmp = FMTNumberComma(dtmp);
                    _txtpay += _util->cyLRText(23, "  OLD BALANCE", 18, tmp.c_str());
                    dtmp = stodsafe(gc["remaining_balance"].asString());
                    tmp = FMTNumberComma(dtmp);
                    _txtpay += _util->cyLRText(23, "  NEW BALANCE", 18, tmp.c_str());
                    
                    if (dtmp > 0.00)  {
                        Json::Value jgcrefund = jpay["gcrefund"];
                        
                    }
                }
            }
        }  else  {
            
            tmp = FMTNumberComma(paymentAmount);
            
            string left = "";
            left += jpay["description"].asString();
            
            _txtpay += _util->cyLRText(23, left, 18, tmp.c_str());
            
            if (receiptType == SRT_REFUND)  {
                left  = "    RES NO.  : ";
                left += jpay["approval_code"].asString ();
                left += "\n";
                _txtpay += left;
                
                left  = "    NAME     : ";
                left += jheader["first_name"].asString ();
                left += " ";
                left += jheader["last_name"].asString ();
                left += "\n";
                _txtpay += left;
            }
            if (jpay["remarks"].asString().length() > 0)  {
                left  = "    REMARKS  : ";
                left += jpay["remarks"].asString ();
                left += "\n";
                _txtpay += left;
            }
        }
    }
    
    jheader["cytotal_payment"]=totalPaymentAmount;
    double totalChangeAmount = totalPaymentAmount - _totalNetSales;
    if (isGc && totalChangeAmount > 0)
        totalChangeAmount = 0.00;
    sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
    totalChangeAmount = atof(sztmp);
    jheader["cytotal_change"]=totalChangeAmount;
    if (receiptType != salesReceiptType::SRT_REFUND)  {
        if (!isCreditCard){
            if (totalChangeAmount > 0) {
                _txtpay += _util->cyLRText(23, "Change", 18, sztmp);
            }  else  {
                if (receiptType != salesReceiptType::SRT_SUSPEND){
                    if (totalChangeAmount != 0)  {
                        double dtmp = totalChangeAmount * -1;
                        sprintf (sztmp, "%.02f", dtmp);
                        //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
                    }
                }
            }
        }
    }
    
    //CONSTRUCT TERMS FOR CREDIT CARD PAYMENT
    if (isCreditCard) {
        jpay = jpayment[0];
        string strterms = jpay["approval_terms"].asString();
        
        switch (atoi(strterms.c_str())) {
        case 1:
            tmp = "3 Months";
            break;
        case 2:
            tmp = "6 Months";
            break;
        case 3:
            tmp = "9 Months";
            break;
        case 4:
            tmp = "12 Months";
            break;
        case 5:
            tmp = "18 Months";
            break;
        case 6:
            tmp = "24 Months";
            break;
        case 7:
            tmp = "36 Months";
            break;
        default:
            tmp = "Straight";
            break;
        }
        _txtpay += "    TERMS : ";
        _txtpay += tmp.c_str();
        _txtpay += _util->cyCenterText(40, " ");
    }
    
    int w = g_widthReceipt;
    int jj = 0;
    receiptType != SRT_POSTVOID ? jj=1 : jj=-1;
    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%0.2f-item(s)",_totalItems * jj);
    _txtpay += _util->cyCenterText(w, sztmp);
    _txtpay += _util->cyCenterText(w, " ");
    
    
    _receipt += _txtpay;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceiptTotals (Json::Value& jheader,
                                              salesReceiptType receiptType)
{
    _txttot = "";
    string tmp = "";
    //////////////////////////////////////
    //  Prep work
    string trxDiscountDesc = "";
    if (_totalTrxDiscount > 0)  {
        Json::Value jtrx = jheader["settings_transaction_discount"];
        trxDiscountDesc = jtrx["description"].asString ();
    }
    //////////////////////////////////////
    //  Detail total(s)
    _txttot += _util->cyAcrossText(40, "=");
    
    //TOTAL SALES AMOUNT
    if (receiptType != SRT_POSTVOID){
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
        else
            tmp = FMTNumberComma(_totalGrossAmount);
    } else {
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount * -1);
        else
            tmp = FMTNumberComma(_totalGrossAmount * -1);
    }
    
    
    _txttot += _util->cyLRText(23, "Total Sales Amount", 18, tmp.c_str());
    //////////////////////////////////////
    //  Sequence change based on type
    int j = 0;
    
    receiptType != SRT_POSTVOID ? j=1 : j=-1;
    
    if (_isPwd || _isSenior)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount * -1 * j);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp);
        }
        
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount - (_totalAmtExempt);
        tmp = FMTNumberComma(netSalesAmount * j);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtExempt != 0.00)  {
            tmp = FMTNumberComma(_totalAmtExempt * -1 * j);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue * j);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else if (_isZeroRated)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount * -1 * j);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalAmtZero);
        tmp = FMTNumberComma(netSalesAmount * j);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtZero != 0.00)  {
            tmp = FMTNumberComma(_totalAmtZero * -1 * j);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue * j);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount * -1 * j);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtVat);
        tmp = FMTNumberComma(netSalesAmount * j);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        
        //  VAT exempt
        if (_totalAmtZero > 0)  {
            //  VAT exempt
            tmp = FMTNumberComma(_totalAmtZero * -1 * j);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }  else  {
            tmp = FMTNumberComma(_totalAmtVat * j);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue * j);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }
    _receipt += _txttot;
    
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceiptDetail (Json::Value& jdetail,
                                              Json::Value& jhdrtrx,
                                              salesReceiptType type)
{
    //_txtdet = "";
    char sztmp[32],sztmp2[72];
    string tmp = jdetail["quantity"].asString(),tmp2 = "";
    double quantity = stodsafe(jdetail["quantity"].asString());
    double priceOriginal = stodsafe(jdetail["retail_price"].asString());
    if (type == SRT_POSTVOID)  {
        quantity = quantity * -1;
    }
    //////////////////////////////////////
    //  Display item detail(s)
    sprintf (sztmp, "%0.2f", quantity);
    
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
    
    _txtdet += jdetail["detail_product_header"]["category_cd"].asString();
    _txtdet += "-";
    _txtdet += jdetail["detail_product_header"]["item_code"].asString();
    _txtdet += " ";
    _txtdet += _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    _txtdet += "\n";
    
    //FORMAT QTY @ ORIG PRICE
    string strOrigPrice = FMTNumberComma(priceOriginal);
    string itmBaseType =  jdetail["detail_product_header"]["base_type_code"].asString();
    sprintf (sztmp, "  %0.0f %s @ %s %s",quantity, "PC",strOrigPrice.c_str(),itmBaseType.c_str());
    
    //FORMAT TOTAL PRICE * QTY
    string strTotalPrice = FMTNumberComma(priceOriginal*quantity);
    sprintf (sztmp2, "%s%s", strTotalPrice.c_str(),itemSuffix.c_str());
    
    tmp = _util->cyLRText(23, sztmp, 18, sztmp2);
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
    
    //////////////////////////////////
    //  Is the item discounted?
    if (jdetail["item_discount"]!=Json::nullValue)  {
        string desc  = "  Less (";
        Json::Value jdisc = jdetail["settings_detail_discount"];
        desc += jdisc["discount_cd"].asString();
        desc += ")";
        double dtmp = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
        sprintf (sztmp, "%.02f", dtmp);
        if (dtmp != 0)  {
            tmp = _util->cyLRText(28, desc, 12, sztmp);
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
        tmp = _util->cyLRText(28, desc, 12, sztmp);
        _txtdet += tmp;
    }
    
    _receipt += _txtdet;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptRCC::assembleReceiptHeader (Json::Value& jheader,
                                              salesReceiptType receiptType)
{
    _txthdr = "";
    _receipt = "";
    _receiptType = receiptType;
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_widthReceipt;
    string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
    Json::Value jtemp2, jtemp = jheader["hdr_branch"];
    Json::Value jHdrBranch = jheader["hdr_branch"];
    Json::Value jHdrSetting = jheader["hdr_settings"];
    Json::Value jHdrCompany = jheader["hdr_company"];
    Json::Value jHdrSetReg = jheader["settings_register"];
    Json::Value jHdrSetCshrShift = jheader["settings_cashier_shift"];
    
    if (jtemp["hdrmsg1"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg1"].asString());
    }
    if (jtemp["hdrmsg2"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg2"].asString());
    }
    if (jtemp["hdrmsg3"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg3"].asString());
    }
    _txthdr += _util->cyCenterText(w, " ");
    _txthdr += _util->cyCenterText(w, jHdrSetting["trade_name"].asString());
    //_txthdr += _util->cyCenterText(w, jHdrCompany["description"].asString());
    _txthdr += _util->cyCenterText(w, jHdrBranch["description"].asString());
    
    _txthdr += _util->cyCenterText(w, jHdrBranch["addr1"].asString());
    _txthdr += _util->cyCenterText(w, jHdrBranch["addr2"].asString());
    
    
    //HARDCODED VALUE NEED ADD TO API
    _txthdr += _util->cyCenterText(w, "TEL NO. +632 865-7700");
    
    tmp  = "VAT-REG-TIN-";
    tmp += jtemp["tin"].asString();
    _txthdr += _util->cyCenterText(w, tmp);
    
    tmp = jHdrSetReg["serialno"].asString();
    if (tmp.length() > w + 1) {
        tmp2 = tmp.substr(0, (w - 1));
    } else {
        tmp2 = tmp;
    }
    _txthdr += _util->cyCenterText(w, tmp2);
    
    tmp  = "STORE # ";
    tmp += jHdrSetCshrShift["location_code"].asString();
    tmp += " ";
    tmp += "POS #";
    tmp += jHdrSetCshrShift["register_num"].asString();
    
    _txthdr += _util->cyCenterText(w,tmp);
    
    //DEFAULT TEMPORARY
    _txthdr += newLine.c_str();
    _txthdr += "CUSTOMER ID: 999999\n";
    _txthdr += newLine.c_str();
    
    //////////////////////////////////////
    //  Special receipt header(s)
    
    if (receiptType == salesReceiptType::SRT_REPRINT) {
        _txttype = "reprint";
        _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","REPRINT");
    }
    else if (receiptType == salesReceiptType::SRT_POSTVOID) {
        _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","VOID");
    }
    else if (receiptType == salesReceiptType::SRT_SUSPEND) {
        _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","SUSPEND");
    }
    else if (receiptType == salesReceiptType::SRT_CANCEL) {
        _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","CANCEL");
    }
    else if (receiptType == salesReceiptType::SRT_REFUND) {
        _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","RETURN");
    } else {
        _log->logmsg("DEBUG RCC-RECEIPT ","REGULAR");
    }
    
    
    //////////////////////////////////////
    //  PAC transaction???
    if (jheader["pac_number"].asString().length() > 0 &&
            jheader["ba_redemption_reference"].asString().length() < 1)  {
        _txthdr += "PAC number: ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += newLine.c_str();
    }
    //////////////////////////////////////
    //  Beauty addict / PAC???
    if (jheader["pac_number"].asString().length() > 0 &&
            jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += "  Customer Id : ";
        _txthdr += jheader["pac_number"].asString();
        _txthdr += newLine.c_str();
        _txthdr += "    Date-Time : ";
        _txthdr += _util->date_pos();
        _txthdr += newLine.c_str();
    }  else  {
        char sztmp[24];
        long lRegister = atol (jHdrSetReg["register_num"].asString().c_str ());
        long lStoreLocation = atol (jHdrSetCshrShift["location_code"].asString().c_str ());
        sprintf (sztmp, "%02ld-%03ld", lStoreLocation,lRegister);
        
        _txthdr += _util->cyAcrossText(40," ");
        
        if (receiptType == salesReceiptType::SRT_REFUND) {
            tmp2 = "SI number    : ";
            tmp3 = refundTransnum(jheader).c_str();
            _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            
        }  else  {
            if (receiptType != salesReceiptType::SRT_SUSPEND &&
                    receiptType == salesReceiptType::SRT_RESUME) {
                tmp2 = "  SI Number : ";
                tmp3 = legacyTransnum(jheader).c_str();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType == salesReceiptType::SRT_RESUME) {
                tmp2 = "  Suspend # : ";
                tmp2 += sztmp;
                tmp2 += "-";
                tmp3 = jheader["transaction_number"].asString ();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType != salesReceiptType::SRT_SUSPEND) {
                tmp2 = "SI Number    : ";
                tmp2 += sztmp;
                tmp2 += "-";
                long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
                sprintf (sztmp, "%09ld", ltrx);
                tmp2 += sztmp;
                
                _txthdr += tmp2.c_str();
                _txthdr += newLine.c_str();
            }
        }
        
        //CONSTRUCT DATE IN RCC FORMAT
        string trans_time,trans_date,dd,mm,yyyy;
        trans_date = jheader["transaction_date"].asString();
        trans_time = jheader["transaction_time"].asString();
        _util->date_split(trans_date.c_str(),yyyy,mm,dd);
        tmp2  = "Date-time    : ";
        tmp2 += mm;
        tmp2 += "/";
        tmp2 += dd;
        tmp2 += "/";
        tmp2 += yyyy;
        tmp2 += "-";
        tmp2 += _util->time_colon(trans_time.c_str());
        
        _txthdr += tmp2.c_str();
        _txthdr += newLine.c_str();
    }
    
    if (receiptType == SRT_REFUND)  {
        _txthdr += "Txn No.      : ";
        _txthdr += jheader["systransnum"].asString();
        _txthdr += newLine.c_str();
        
        _txthdr += "Business Date: ";
        _txthdr += jheader["systransnum"].asString();
        _txthdr += newLine.c_str();
        
        _txthdr += "Orig Pos No  : ";
        _txthdr += jheader["orig_register_number"].asString();
        _txthdr += newLine.c_str();
        
        _txthdr += "Orig Bus Date: ";
        _txthdr += jheader["orig_transaction_date"].asString();
        _txthdr += newLine.c_str();
        
        _txthdr += "Original SI# : ";
        _txthdr += jheader["orig_systransnum"].asString();
        _txthdr += newLine.c_str();
    }
    
    //////////////////////////////////////
    //  Transaction type
    _txthdr += _util->cyCenterText(w, " ");
    
    if (jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += "Reference Code : ";
        _txthdr += jheader["ba_redemption_reference"].asString();
        _txthdr += newLine.c_str();
    }
    
    
    if (receiptType != SRT_POSTVOID)
        _txthdr += _util->cyCenterText(w, "REGULAR");
    
    
    _txthdr += _util->cyAcrossText(40, "=");
    _receipt += _txthdr;




    return true;
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptRCC::CYRestReceiptRCC (CYDbEnv* env, cycgi* cgi) : CYRestCommon (env, cgi)
{
    _env = nullptr;
    _env = new SqliteEnv;
    string rcptdir = _ini->get_value("PATH","RECEIPT");
    rcptdir += "/receipt.qtdb";
    env->connect ("", "", "", rcptdir, 0);
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptRCC::CYRestReceiptRCC (CYDbEnv* env, cylog* log, cyini* ini, cycgi* cgi)
    : CYRestCommon (env, log, ini, cgi)
{
    _env = nullptr;
    _env = new SqliteEnv;
    string rcptdir = _ini->get_value("PATH","RECEIPT");
    rcptdir += "/receipt.qtdb";
    env->connect ("", "", "", rcptdir, 0);
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYRestReceiptRCC::~CYRestReceiptRCC ()
{
    resetEodTotals();
    if (nullptr != _env)
        delete (_env);
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
bool CYRestReceiptRCC::saveReceipt()
{
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptPromoItem
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptRCC::assembleReceiptPromoItem (Json::Value& jheader,
                                                                Json::Value& jdetail)
{
    _txtpromoitem = "";
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* db = dbconn ();
    if (!db->ok())
        return seterrormsg(db->errordb());

    CYDbSql* dbsel = dbconn ();
    if (!dbsel->ok())
        return seterrormsg(dbsel->errordb());
    //////////////////////////////////////
    //  Retrieve all active promotions
    dbsel->sql_reset();
    salesQueryType sqt;
    Json::Value jhdrtrx, jgiftpromo;
    jhdrtrx = jheader["hdr_trx"];

    sql  = "select   * ";
    sql += "from     pos_giftitem_promo a, pos_giftitem_promo_trans b ";
    sql += "where    a.gift_code = b.gift_code ";
    sql += " and     a.start_date <= ";
    sql += dbsel->sql_bind (1, _util->date_eight());
    sql += " and     a.end_date >= ";
    sql += dbsel->sql_bind (2, _util->date_eight());
    sql += " and     b.transaction_cd = ";
    sql += dbsel->sql_bind (3, jhdrtrx["transaction_code"].asString());

    sqt = json_select_multiple(dbsel, jgiftpromo, sql);
    if (sqt == salesQueryType::SQT_ERROR)
        return false;
    else if (sqt == salesQueryType::SQT_NOROWS)
        return true;

    //////////////////////////////////////
    //  For each qualified promotion
    string tmp;
    double promoTotal = 0.00;
    Json::Value jpromo, jitem,
                jcategory;
    int j = jgiftpromo.size();
    for (int i = 0; i < j; i++)  {
        jpromo = jgiftpromo[i];
        double quantity, retailPrice;
        //////////////////////////////////
        //  Check if item(s) qualify
        int y = jdetail.size();
        for (int x = 0; x < y; x++)  {
            jitem = jdetail[x];
            tmp = jitem["quantity"].asString();
            if (tmp .length() < 1) tmp = "0.00";
            quantity = stodsafe(tmp);

            tmp = jitem["retail_price"].asString();
            if (tmp .length() < 1) tmp = "0.00";
            retailPrice = stodsafe(tmp);

            dbsel->sql_reset();
            sql  = "select count(*) as recs ";
            sql += "from   pos_giftitem_promo_sku ";
            sql += "where  gift_code = ";
            sql +=  dbsel->sql_bind (1, jpromo["gift_code"].asString());
            sql += " and   item_code = ";
            sql += dbsel->sql_bind (2, jitem["item_code"].asString());

            if (!dbsel->sql_result(sql,true))
                return seterrormsg(dbsel->errordb());

            if (atoi(dbsel->sql_field_value("recs").c_str()))  {
                promoTotal += quantity * retailPrice;
            }  else  {
                //////////////////////////
                //  qualify by hierarchy
                dbsel->sql_reset();
                sql  = "select * ";
                sql += " from  pos_giftitem_promo_dept ";
                sql += " where gift_code = ";
                sql += dbsel->sql_bind(1, jpromo["gift_code"].asString());

                if (!dbsel->sql_result(sql,true))
                    return seterrormsg(dbsel->errordb());

                bool bexit = false;
                string itemCategory = "";
                string promoCategory = "";
                if (!dbsel->eof ())  {
                    do  {
                        jcategory.clear ();
                        jcategory = jitem["detail_product_header"];

                        itemCategory  = jcategory["category_cd"].asString();
                        if (dbsel->sql_field_value ("subcat_cd").length() > 0)
                            itemCategory += jcategory["subcat_cd"].asString();
                        if (dbsel->sql_field_value ("class_cd").length() > 0)
                            itemCategory += jcategory["class_cd"].asString();
                        if (dbsel->sql_field_value ("subclass_cd").length() > 0)
                            itemCategory += jcategory["subclass_cd"].asString();

                        promoCategory  = dbsel->sql_field_value ("category_cd");
                        promoCategory += dbsel->sql_field_value ("subcat_cd");
                        promoCategory += dbsel->sql_field_value ("class_cd");
                        promoCategory += dbsel->sql_field_value ("subclass_cd");

                        dbsel->sql_next ();
                        bexit = dbsel->eof ();
                        if (promoCategory == itemCategory)  {
                            promoTotal += quantity * retailPrice;
                            bexit = true;
                        }
                    } while (!bexit);
                }
            }
        }
        //////////////////////////////////
        //  Minimum promo amount
        tmp = jpromo["min_amount"].asString();
        if (tmp .length() < 1) tmp = "0.00";
        double promoMin = stodsafe(tmp);
        /////////////////////////////////
        Json::Value jbranch, jsetting, jcompany, jtrxtype;
        if (promoTotal >= promoMin)  {
           ///////////////////////////////
           // Promo Satisfied
           _txtpromoitem += "\n\n\n";
           ///////////////////////////////
           //  No fractional portion
           long freeItem = (long) promoTotal / promoMin;
           string barCode = _util->fmt_number(3, jheader["register_number"].asString().c_str());
           barCode += _util->date_sequence().c_str();
           ///////////////////////////////
           //  Extended receipt
           size_t  w = g_widthReceipt;
           jbranch = jheader["hdr_branch"];
           jsetting = jheader["hdr_settings"];
           jcompany = jheader["hdr_company"];
           jtrxtype = jhdrtrx["trx_type"];

           _txtpromoitem += _util->cyCenterText(w, jcompany["description"].asString());
           _txtpromoitem += _util->cyCenterText(w, jsetting["trade_name"].asString());
           _txtpromoitem += _util->cyCenterText(w, jbranch["description"].asString());
           _txtpromoitem += "\n\n";

           if(jtrxtype["is_postvoid"].asString() == "1" )
               _txtpromoitem += _util->cyCenterText(w, "*** V O I D ***");

           dbsel->sql_reset();
           sql  = "select * from  pos_giftitem_promo_rcpt ";
           sql += "where  gift_code = ";
           sql += dbsel->sql_bind(1, jpromo["gift_code"].asString());
           sql += " order by seq_num ";

           if (!dbsel->sql_result(sql,true))
               return seterrormsg(dbsel->errordb());

           while (!dbsel->eof ())  {
               string line   = dbsel->sql_field_value("rcpt_line");
               string center = dbsel->sql_field_value("is_center");

               size_t found = line.find("<QTY>");
               if (found != std::string::npos)
                    line.replace (found, 5, _util->longtostring(freeItem));

               if (center == "1") {
                   _txtpromoitem += _util->cyCenterText(w, line);
               }  else  {
                   _txtpromoitem += line;
               }
               dbsel->sql_next();
           }
           _txtpromoitem += "\n\n";
           _txtpromoitem += _util->cyLRText(w/2, "TOTAL PURCHASE. ",
                                            w/2, _util->fmt_decimal(2, _util->doubletostring(promoTotal).c_str()).c_str());
           string left, right = "TERM NO. ";
           right += jheader["register_number"].asString();

           string trans_time,trans_date,dd,mm,yyyy;
           trans_date = jheader["transaction_date"].asString();
           trans_time = jheader["transaction_time"].asString();
           _util->date_split(trans_date.c_str(),yyyy,mm,dd);

           left  = mm; left += "/"; left += dd;
           left += "/"; left += yyyy; left += "-";
           left += _util->time_colon(trans_time.c_str());
           _txtpromoitem += _util->cyLRText(w/2, left.c_str(), w/2, right.c_str());


           left  = "TRANS NO. ";
           left += jheader["transaction_number"].asString();

           Json::Value jcashier;
           jcashier = jheader["settings_cashier"];
           right  = jcashier["last_name"].asString();
           right += ",";
           right += jcashier["first_name"].asString();

           _txtpromoitem += _util->cyLRText(w/2, left.c_str() ,w/2, right.c_str());
           _txtpromoitem += "\n";


           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += _util->cyCenterText(w, barCode.c_str());
           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += _util->cyAcrossText(w, "_");
           _txtpromoitem += _util->cyCenterText(w, "Customer Signature over Printed Name");
           _txtpromoitem += "\n\n\n";

           db->sql_reset ();
           sql  = "insert into pos_giftitem_promo_redeem (gift_code, ";
           sql += "item_code, bar_code, total_purchase, is_redeem ) values('";
           sql += jpromo["gift_code"].asString();
           sql += "', '', '";
           sql += barCode;
           sql += "',";
           sql += _util->doubletostring(promoTotal);
           sql += ",0)";

            if(!db->sql_only(sql,false)){
                return seterrormsg(db->errordb());
            }

            if(!db->sql_commit()){
                return seterrormsg(db->errordb());
            }
        }
        promoTotal = 0.00;
    }
    _receipt += _txtpromoitem;
    return true;
}

