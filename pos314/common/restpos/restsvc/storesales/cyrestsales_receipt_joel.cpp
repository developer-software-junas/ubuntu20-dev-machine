//*******************************************************************
//        FILE:     cyrestsales_receipt_rcc.cpp
// DESCRIPTION:     Sales receipt generation daily receipt
//*******************************************************************
#include "cyrestreceipt.h"
#include "cyrestreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoel::assembleFspUrl(Json::Value& jheader,
                                      Json::Value& jdetail,
                                      Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
//*******************************************************************
//    FUNCTION:     xreadSaveTax
// DESCRIPTION:     Save the tax totals
//*******************************************************************
bool CYRestReceiptJoel::xreadSaveTax (string systransnum)
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconn ();
    if (!db->ok())
        return seterrormsg(_env->errordbenv());
    //////////////////////////////////////
    //  See if a record already exists
    string sql = "select count(*) as recs from tg_pos_xread_tax ";
    sql += " where systransnum = ";
    sql += db->sql_bind(1,systransnum);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    string recs = db->sql_field_value("recs");

    db->sql_reset ();
    if (atoi(recs.c_str ()) > 0)  {
        sql = "delete from tg_pos_xread_tax ";
        sql += " where systransnum = ";
        sql += db->sql_bind(1,systransnum);
        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());
    }
    char sztmp[64];
    db->sql_bind_reset ();
    sql  = "insert into tg_pos_xread_tax(systransnum,net_vat,net_zero,net_exempt,";
    sql += "amt_vat,amt_zero,amt_exempt,net_amount,vat_amount,gross_amount,service_charge)";
    sql += " values (";
    sql += db->sql_bind(1,systransnum); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_vat * 100) / 100);
    sql += db->sql_bind(2,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_zero * 100) / 100);
    sql += db->sql_bind(3,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_exempt * 100) / 100);
    sql += db->sql_bind(4,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_vat * 100) / 100);
    sql += db->sql_bind(5,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_zero * 100) / 100);
    sql += db->sql_bind(6,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_exempt * 100) / 100);
    sql += db->sql_bind(7,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_sales * 100) / 100);
    sql += db->sql_bind(8,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.vat_amount * 100) / 100);
    sql += db->sql_bind(9,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.gross_sales * 100) / 100);
    sql += db->sql_bind(10,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.service_charge * 100) / 100);
    sql += db->sql_bind(11,sztmp);
    sql += ")";
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    if (!db->sql_commit())
        return seterrormsg(db->errordb());

    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoel::assembleReceipt(Json::Value& jheader,
                                       Json::Value& jdetail,
                                       Json::Value& jpayment,
                                       salesReceiptType receiptType)
{
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;
    //FOR TEST ONLY-OK
    std::stringstream ss;
    ss <<jheader;
    string msg = ss.str();
    //_log->logmsg("DEBUG JOEL RECEIPT REGULAR JSON ",msg.c_str());
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

    //_log->logmsg("x1x1x1x1",_receipt.c_str ());
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptGiftWrap
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoel::assembleReceiptClaimStub(Json::Value& jheader,
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

        //NEDD TO ADD API TNX NO
        tmp  = jheader["register_number"].asString();
        tmp += "-xxxx";

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

        strCShdr += "GW Counter No:                ";
        strCShdr += "xx";
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


    }
    _receipt += _txtgift;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptGiftWrap
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoel::assembleReceiptGiftWrap (Json::Value& jheader,
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

        tmp = "SI Number    ";
        tmp2 = jheader["transaction_number"].asString();
        long ltrxnum = atol (tmp2.c_str ());
        sprintf (sztmp, "%012ld", ltrxnum);
        tmp += sztmp;

        //NEED TO ADD IN API
        tmp2 = "Txn: ";
        tmp2 += "xxxx";

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
        strGhdr += _util->cyAcrossText(w, "-");


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
bool                CYRestReceiptJoel::assembleReceiptFooter (Json::Value& jheader,
                                                             salesReceiptType receiptType)
{
    _txtfoot = "";
    Json::Value jtemp;
    string tmp = "",label = "";
    if (jheader["hdr_settings"]==Json::nullValue)
        return seterrormsg("Unable to retrieve the POS settings information");

    Json::Value jpos,jcashier,jaccount,jacct;
    int w = g_widthReceipt;
    jpos = jheader["hdr_settings"];

    if (receiptType == salesReceiptType::SRT_REPRINT ||
        receiptType == salesReceiptType::SRT_POSTVOID)  {
        _txtfoot += _util->cyCenterText(w, "THIS DOES NOT SERVE AS AN INVOICE");
        _txtfoot += _util->cyCenterText(w, "THIS DOCUMENT IS NOT VALID");
        _txtfoot += _util->cyCenterText(w, "FOR CLAIM OF INPUT TAX");
        _txtfoot += "\n";
    }

    if (receiptType != salesReceiptType::SRT_NORMAL &&
            receiptType != salesReceiptType::SRT_REPRINT &&
            receiptType != salesReceiptType::SRT_DAILY)  {
    }  else  {
        _txtfoot += _util->cyCenterText(w, "CYWARE INCORPORATED");
        _txtfoot += _util->cyCenterText(w, "6057 Palma Street");
        _txtfoot += _util->cyCenterText(w, "Poblacion NCR Fourth District Makati");

        jtemp = jheader["hdr_branch"];
        tmp  = "VAT-REG-TIN: ";
        tmp += jtemp["tin"].asString();
        _txtfoot += _util->cyCenterText(w, tmp);

        tmp  = "ACCR.# ";
        tmp += jtemp["bir_num"].asString();
        _txtfoot += _util->cyCenterText(w,tmp);

        string permitStart,PermitEnd,mm,dd,yyyy;
        permitStart = jtemp["bir_issue"].asString();
        _util->date_split(permitStart.c_str(),yyyy,mm,dd);

        tmp  = "EFFECTIVITY DATE: ";
        tmp += mm.c_str();
        tmp += "/";
        tmp += dd.c_str();
        tmp += "/";
        tmp += yyyy.c_str();
        _txtfoot += _util->cyCenterText(w,tmp);

        permitStart = jtemp["bir_expire"].asString();
        _util->date_split(permitStart.c_str(),yyyy,mm,dd);

        tmp  = "VALID UNTIL: ";
        tmp += mm.c_str();
        tmp += "/";
        tmp += dd.c_str();
        tmp += "/";
        tmp += yyyy.c_str();
        _txtfoot += _util->cyCenterText(w,tmp);

        tmp  = "Permit No: ";
        tmp  += jtemp["permitno"].asString();
        _txtfoot += _util->cyCenterText(w,tmp);

        //format date mm/dd/yyyy
        jtemp = jheader["settings_register"];
        string permitDate;
        permitDate = jtemp["permit_start"].asString();
        _util->date_split(permitDate.c_str(),yyyy,mm,dd);

        tmp  = "Date Issued: ";
        tmp += mm.c_str();
        tmp += "/";
        tmp += dd.c_str();
        tmp += "/";
        tmp += yyyy.c_str();
        _txtfoot += _util->cyCenterText(w,tmp);
    }

    _txtfoot += _util->cyCenterText(w, " ");
    _receipt += _txtfoot;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoel::assembleReceiptSignature (Json::Value& jheader,
                                                                salesReceiptType receiptType)
{
    _txtsig = "";
    (void)receiptType;
    if (jheader["hdr_tax_receipt"]==Json::nullValue)  {
        _txtsig += "\n      OFFICIAL RECEIPT DETAIL(S)\n";
        _txtsig += "Customer name:   _______________________\n";
        _txtsig += "Address:         _______________________\n";
        _txtsig += "TIN:             _______________________\n";
        _txtsig += "Business style:  _______________________\n";
        _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n\n";
        _txtsig += "Signature: _____________________________\n";
        _txtsig += "\n";
        return true;
    }

    Json::Value jacct;
    jacct = jheader["hdr_tax_receipt"];
    //////////////////////////////////////
    //  OR customer
    if (jacct["customer_name"].asString().length() > 0)
        _txtsig += "\n      OFFICIAL RECEIPT DETAIL(S)\n";
    if (jacct["company_name"].asString().length() > 0)  {
        _txtsig += "           Name: ";
        _txtsig += jacct["company_name"].asString();
        _txtsig += "\n";
    }
    if (jacct["address1"].asString().length() > 0)  {
        _txtsig += "       Address1: ";
        _txtsig += jacct["address1"].asString();
        _txtsig += "\n";
    }
    if (jacct["address2"].asString().length() > 0)  {
        _txtsig += "       Address2: ";
        _txtsig += jacct["address2"].asString();
        _txtsig += "\n";
    }
    if (jacct["tin"].asString().length() > 0)  {
        _txtsig += "            TIN: ";
        _txtsig += jacct["tin"].asString();
        _txtsig += "\n";
    }
    if (jacct["customer_name"].asString().length() > 0)  {
        _txtsig += " Business Style: ";
        _txtsig += jacct["customer_name"].asString();
        _txtsig += "\n";
    }
    _txtsig += "\n";
    _receipt += _txtsig;
    return true;
}


//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoel::assembleVatBreakdown (salesReceiptType receiptType)
{
    _txtvat = "";
    int w = g_widthReceipt;

    if (receiptType == SRT_SUSPEND)
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

    if (receiptType == SRT_REFUND)  {
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
    double totalAmtVat = _totalAmtVat;// + _totalAmtZero + _totalAmtExempt;
    char szTotalAmtVat[32];

    if (receiptType == SRT_REFUND)  {
        if (totalNetVat == 0)
            strcpy (szTotalNetVat, "0.00");
        else
            sprintf (szTotalNetVat, "-%s", FMTNumberComma((totalNetVat * 1000) / 1000).c_str());
        if (totalAmtVat == 0)
            strcpy (szTotalAmtVat, "0.00");
        else
            sprintf (szTotalAmtVat, "-%s", FMTNumberComma((totalNetVat * 1000) / 1000).c_str());
    }  else  {
        if (totalNetVat != 0)
            sprintf (szTotalNetVat, "%s", FMTNumberComma((totalNetVat * 1000) / 1000).c_str());
        else
            strcpy (szTotalNetVat, "0.00");

        if (totalAmtVat != 0)
            sprintf (szTotalAmtVat, "%s", FMTNumberComma((totalAmtVat * 1000) / 1000).c_str());
        else
            strcpy (szTotalAmtVat, "0.00");
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
bool CYRestReceiptJoel::assembleReceiptAccount (Json::Value& jheader, salesReceiptType receiptType)
{
    _txtacct = "";
    string info, label,tmp;
    string today;
    _util->int_date(today,0);
    bool isPshopper = false;
    Json::Value jacct, jaccount,jcashier;
    int w = g_widthReceipt;
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

    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_shopper"].asString() == "1") {
            label = "PShopper ID";
            isPshopper = true;
        } else if (jacct["is_zero_rated"].asString() == "1") {
            label = "VAT-Exempt";
        } else if (jacct["is_agent"].asString() == "1") {
            label = "CRM account";
        } else {
            if (jacct["is_clerk"].asString() != "1")
                label = jacct["account_type_desc"].asString();
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

            if (isPshopper) {
                label =  "RIDC #: ";
                label += jheader["ridc_number"].asString();

                _txtacct += label.c_str();
                _txtacct += "\n";

                isPshopper=false;

            }
        }

        label = "";

    }

    if (receiptType == salesReceiptType::SRT_POSTVOID || receiptType == salesReceiptType::SRT_SUSPEND){
        //NEED DATA FROM API
        info  = "Txn No. : ";
        info += jheader["transaction_number"].asString();
        _txtacct += info.c_str();
        _txtacct += "\n";
    }


    info  = "Business Date:  ";
    info += today.c_str();
    _txtacct += info.c_str();
    _txtacct += "\n";

    if (receiptType != SRT_SUSPEND)
        _txtacct += _util->cyAcrossText(w,"-");


    _receipt += _txtacct;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoel::assembleReceiptPayment (Json::Value& jheader,
                                               Json::Value& jpayment,
                                               salesReceiptType receiptType)
{
    string tmp;
    char sztmp[32];
    bool isGc = false;
    //bool isCheck = false;
    //bool isDebit = false;
    bool isCharge = false;
    bool isCreditCard = false;
    bool isMobilePay = false;

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
        isGc = false;
        //isCheck = false;
        //isDebit = false;
        isCharge = false;
        isCreditCard = false;

        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isGc = jtmp["is_gc"].asString() == "1";
        //isCheck = jtmp["is_check"].asString() == "1";
        isCharge = jtmp["is_charge"].asString() == "1";
        //isDebit = jtmp["is_debit_card"].asString() == "1";
        isCreditCard = jtmp["is_credit_card"].asString() == "1";
        isMobilePay = jtmp["is_social_distancing"].asString() == "1";
        ////////////////////////////////////
        //  GC special processing
        if (isGc) {
            if (jpay["gc"] == Json::nullValue)
                return seterrormsg("Unable to process the GC payment information");

            Json::Value gc, jgc = jpay["gc"];
            ////////////////////////////
            //  GC payment(s)
            if (isGc) {
                sprintf (sztmp, "%.02f", paymentAmount);
                _txtpay += _util->cyLRText(23, jpay["description"].asString(), 18, sztmp);
                int j = jgc.size();
                for (int i = 0; i < j; i++) {
                  gc.clear ();
                  gc = jgc[i];
                  _txtpay += gc["description"].asString();
                  //_txtpay += "\n";
                  _txtpay += _util->cyLRText(23, "  GC SERIES", 18, gc["gcnum"].asString());

                  double dtmp = stodsafe(gc["amount_paid"].asString());
                  sprintf (sztmp, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(23, "  OLD BALANCE", 18, sztmp);
                  dtmp = stodsafe(gc["remaining_balance"].asString());
                  sprintf (sztmp, "%.02f", dtmp);
                  _txtpay += _util->cyLRText(23, "  NEW BALANCE", 18, sztmp);

                  if (dtmp > 0.00)  {
                      Json::Value jgcrefund = jpay["gcrefund"];

                  }
                }
            }
        }  else  {
            if (receiptType == SRT_REFUND || receiptType == SRT_POSTVOID)
                sprintf (sztmp, "-%.02f", paymentAmount);
            else
                sprintf (sztmp, "%.02f", paymentAmount);
            string right = FMTNumberComma(atof(sztmp));

            string left = "";
            left += jpay["description"].asString();
            left += "(";
            left += jpay["pay_code"].asString();
            left += ")";
            _txtpay += _util->cyLRText(23, left, 18, right);

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
        }
        ////////////////////////////////////
        //  Credit card
        bool bextra = false;
        string headerText, lineDesc = "";
        if (isCreditCard) {
            bextra = true;
            int idxTerms = 0;
            lineDesc = "  TERMS ";
            tmp = jpay["approval_terms"].asString();
            if (!_util->valid_number("Payment terms", tmp, 1, 2))
                jpay["approval_terms"]="0";
            idxTerms = atoi(tmp.c_str ());
            switch (idxTerms) {
              case 0:
                headerText += "Straight";
                break;
              case 1:
                headerText += "3 months";
                break;
              case 2:
                headerText += "6 months";
                break;
              case 3:
                headerText += "9 months";
                break;
              case 4:
                headerText += "12 months";
                break;
              case 5:
                headerText += "18 months";
                break;
              case 6:
                headerText += "24 months";
                break;
              case 7:
                headerText += "36 months";
                break;
            }
            headerText += " ";
            if (isCreditCard) {
              _txtpay += _util->cyLRText(24, lineDesc, 18, headerText);
            }
            //lineDesc  = "  ACQUIRER";
            //receipt += cyLRText(23, lineDesc, 19, "${pay.getApprovalMerchant()} ");
            lineDesc = "  APPROVAL CODE ";
            _txtpay += _util->cyLRText(23, lineDesc, 18, jpay["approval_code"].asString());
        }
        ////////////////////////////////////
        //  Charge card
        lineDesc = "";
        if (isCharge) {
            bextra = true;            bextra = true;
            lineDesc = "  ACCOUNT   ";
            lineDesc += jpay["account_number"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;

            lineDesc = "  CUSTOMER  ";
            lineDesc += jpay["first_name"].asString();
            lineDesc += " ";
            lineDesc += jpay["last_name"].asString();
            lineDesc += "\n";
            _txtpay += lineDesc;
        }
        if (isMobilePay) {
            int idxTerms = 0;
            bextra = true;
            lineDesc = "  TERMS ";
            tmp = jpay["approval_terms"].asString();
            if (!_util->valid_number("Payment terms", tmp, 1, 2))
                return seterrormsg(_util->errormsg());
            idxTerms = atoi(tmp.c_str ());
            switch (idxTerms) {
              case 0:
                headerText += "Straight";
                break;
              case 1:
                headerText += "3 months";
                break;
              case 2:
                headerText += "6 months";
                break;
              case 3:
                headerText += "9 months";
                break;
              case 4:
                headerText += "12 months";
                break;
              case 5:
                headerText += "18 months";
                break;
              case 6:
                headerText += "24 months";
                break;
              case 7:
                headerText += "36 months";
                break;
            }
            _txtpay += _util->cyLRText(17, lineDesc, 24, headerText);

            lineDesc  = "  ACQUIRER";
            _txtpay += _util->cyLRText(17, lineDesc, 24, "eGHL");

            lineDesc  = "  CARD HOLDER";
            _txtpay += _util->cyLRText(17, lineDesc, 24, jpay["first_name"].asString());

            lineDesc = "  APPROVAL CODE ";
            _txtpay += _util->cyLRText(17, lineDesc, 24, jpay["approval_code"].asString());
        }
        if (!bextra)  {
            lineDesc  = "    REFERENCE: ";
            lineDesc += jpay["approval_code"].asString();
            if (jpay["approval_code"].asString().length() > 0 &&
                receiptType != SRT_REFUND)
                _txtpay += lineDesc;
        }
    }
    jheader["cytotal_payment"]=totalPaymentAmount;
    //////////////////////////////////////
    //  Net sales amount
    double netSalesAmount = _totalGrossAmount -
            (_totalItemDiscount+_totalTrxDiscount+_totalAmtExempt+_totalAmtZero);
    //////////////////////////////////////
    //  Calculate the change
    double absNet = netSalesAmount;
    if (absNet < 0)
        absNet = netSalesAmount * -1;
    double totalChangeAmount = totalPaymentAmount - absNet;
    //  GC has no change
    if (isGc && totalChangeAmount > 0) totalChangeAmount = 0.00;
    sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
    totalChangeAmount = atof(sztmp);
    jheader["cytotal_change"]=totalChangeAmount;
    if (receiptType != salesReceiptType::SRT_REFUND)  {
        if (totalChangeAmount > 0) {
            if (receiptType == SRT_POSTVOID)
                sprintf (sztmp, "-%.02f", (totalChangeAmount*1000)/1000);
            _txtpay += _util->cyLRText(23, "Change", 18, sztmp);
        }  else  {
            if (totalChangeAmount != 0)  {
                double dtmp = totalChangeAmount * -1;
                sprintf (sztmp, "%.02f", dtmp);
                //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
            }
        }
    }

    int w = g_widthReceipt;
    _txtpay += _util->cyCenterText(w, " ");
    sprintf (sztmp, "%.03f-item(s)", _totalItems);
    _txtpay += _util->cyCenterText(w, sztmp);
    _txtpay += _util->cyCenterText(w, " ");



    Json::Value jcashier;
    if (jheader["settings_cashier"] == Json::nullValue)
        return seterrormsg("Unable to get the cashier details");
    jcashier = jheader["settings_cashier"];

    size_t len = jcashier["first_name"].asString().length();
    len += jcashier["last_name"].asString().length();
    len += 14;
    len += jcashier["login"].asString().length();

    if (len > 40)  {
        _txtpay += "Cashier      : ";
        _txtpay += jcashier["first_name"].asString();
        _txtpay += " ";
        _txtpay += jcashier["last_name"].asString();

        _txtpay += "\n           [";
        _txtpay += jcashier["login"].asString();
        _txtpay += "]\n";
    }  else  {
        _txtpay += "Cashier      : ";
        _txtpay += jcashier["first_name"].asString();
        _txtpay += " ";
        _txtpay += jcashier["last_name"].asString();
        _txtpay += "[";
        _txtpay += jcashier["login"].asString();
        _txtpay += "]\n";
    }
    if (receiptType == SRT_REFUND)  {
        _txtpay += "Txn No.      : ";
        _txtpay += jheader["systransnum"].asString();
        _txtpay += "\n";

        _txtpay += "Business Date: ";
        _txtpay += jheader["logical_date"].asString();
        _txtpay += "\n";

        _txtpay += "Orig Pos No  : ";
        _txtpay += jheader["orig_register_number"].asString();
        _txtpay += "\n";

        _txtpay += "Orig Bus Date: ";
        _txtpay += jheader["orig_transaction_date"].asString();
        _txtpay += "\n";

        _txtpay += "Original SI# : ";
        _txtpay += jheader["orig_systransnum"].asString();
        _txtpay += "\n";
    }  else  {
        _txtpay += " Date-Time   : ";
        _txtpay += _util->date_pos();
        _txtpay += "\n";

        if (jheader["ridc_number"].asString().length () > 0)  {
          _txtpay += " RIDC#       : ";
          _txtpay += jheader["ridc_number"].asString();
          _txtpay += "\n";
        }
        _txtpay += " SYSDATE     : ";
        _txtpay += _util->date_nice(jheader["logical_date"].asString().c_str ());
        _txtpay += "\n";
    }

    _receipt += _txtpay;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoel::assembleReceiptTotals (Json::Value& jheader,
                                              salesReceiptType receiptType)
{
    _txttot = "";
    string tmp2 = "", tmp = "";
    int w = g_widthReceipt;
    //////////////////////////////////////
    //  Prep work
    string trxDiscountDesc = "";
    if (_totalTrxDiscount > 0)  {
        Json::Value jtrx = jheader["settings_transaction_discount"];
        trxDiscountDesc = jtrx["description"].asString ();
    }
    //////////////////////////////////////
    //  Detail total(s)
    _txttot += _util->cyAcrossText(w, "=");

    //TOTAL SALES AMOUNT
    if (receiptType != SRT_POSTVOID){
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
        else
            tmp = FMTNumberComma(_totalGrossAmount);
    } else {
        if (_totalItemDiscount > 0)
            tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
        else
            tmp = FMTNumberComma(_totalGrossAmount);
    }

    if (receiptType == SRT_REFUND)  {
        tmp2 = "-";
        tmp2 += tmp;
    }  else
        tmp2 = tmp;
    _txttot += _util->cyLRText(23, "Total Sales Amount", 18, tmp2.c_str());
    //////////////////////////////////////
    //  Sequence change based on type
    char prefix[2];

    if (receiptType == SRT_REFUND)  {
        strcpy (prefix,"-");
    }  else  {
        strcpy(prefix,"");
    }


    if (_isPwd || _isSenior)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp);
        }

        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtExempt != 0.00)  {
            tmp = FMTNumberComma(_totalAmtExempt);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else if (_isZeroRated)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtZero != 0.00)  {
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else  {
        //  Transaction discount
        if (_totalTrxDiscount > 0)  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtVat);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());

        //  VAT exempt
        if (_totalAmtZero > 0)  {
            //  VAT exempt
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }  else  {
            tmp = FMTNumberComma(_totalAmtVat);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }
    _receipt += _txttot;

    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptDetail
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoel::assembleReceiptDetail (Json::Value& jdetail,
                                              Json::Value& jhdrtrx,
                                              salesReceiptType type)
{
    //_txtdet = "";
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
    tmp  = jdetail["detail_product_header"]["item_code"].asString();
    tmp += " ";


    tmp += _util->string_cleanse(jdetail["description"].asString().c_str (),true);
    //tmp += jdetail["description"].asString().c_str ();
    if (tmp.length() > 39)
        tmp = tmp.substr(0,39);
    _txtdet += tmp;
    _txtdet += "\n";

    //FORMAT QTY @ ORIG PRICE LEFT SIDE
    string strOrigPrice = FMTNumberComma(priceOriginal);
    string itmBaseType =  jdetail["detail_product_header"]["base_type_code"].asString();
    sprintf (sztmp, "  %0.03f %s @ %s %s",quantity, "PC",strOrigPrice.c_str(),itmBaseType.c_str());

    //FORMAT TOTAL PRICE * QTY RIGHT SIDE
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

    //////////////////////////////////////////////////
    //check if item has modifier
    /*Json::Value jmain = jdetail["info_modifier"];
    Json::Value jsubmain,jmodifier,jmodifierdet;
    int x = jmain.size();

    if (jmain!= Json::nullValue) {
        for ( int i = 0; i < x;i++){
            jsubmain = jmain[i];
            int y = jsubmain.size();
            for (int j= 0; j < y;j++){
                jmodifier = jsubmain[j];
                string modifier_name = jmodifier["modifier_name"].asString();
                tmp = " ";
                tmp += modifier_name;
                _txtdet += tmp;

                int z = jmodifier.size();
                for (int k = 0; k < z;k++){
                    jmodifierdet = jmodifier[k];
                    int nModQty = stodsafe(jmodifierdet["quantity"].asString());
                    string modDesc = jmodifierdet["description"].asString();
                    sprintf(sztmp,"%d",nModQty);
                    tmp  = "  ";
                    tmp += sztmp;
                    tmp += " ";
                    tmp += modDesc;
                    _txtdet += tmp;
                }
            }
        }
    }*/
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
bool CYRestReceiptJoel::assembleReceiptHeader (Json::Value& jheader,
                                              salesReceiptType receiptType)
{
    _txthdr = "";
    _receipt = "";
    _receiptType = receiptType;
    //code for debugging only
    /*std::stringstream out;
    out << jheader;
    _txthdr = out.str();
    return true;*/
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
    _txthdr += _util->cyCenterText(w," ");
    if (receiptType == salesReceiptType::SRT_DAILY ||
        receiptType == salesReceiptType::SRT_NORMAL){
        _txthdr += _util->cyCenterText(w,"SALES INVOICE");
        _txthdr += _util->cyCenterText(w," ");
    }

    //////////////////////////////////////
    //  Account
    Json::Value jacct, jaccount, jaccttype;
    if (jheader["hdr_trxaccount"] == Json::nullValue)  {
        jacct = Json::nullValue;
        jaccount = Json::nullValue;
    }  else  {
        jacct = jheader["hdr_trxaccount"];
        string account_type  = "settings_account_";
        account_type += jacct["account_type_code"].asString();
        jaccttype = jheader[account_type];

        if (jaccttype["is_pwd"].asString() == "1" ||
            jaccttype["is_senior"].asString() == "1" ||
            jaccttype["is_zero_rated"].asString() == "1")  {

            _txthdr += "Customer name: ";
            tmp  = jacct["first_name"].asString();
            tmp += " ";
            tmp += jacct["last_name"].asString();
            tmp += "\n";
            _txthdr += tmp;

            _txthdr += "Account type: ";
            tmp  = jacct["account_type_desc"].asString();
            tmp += "\n";
            _txthdr += tmp;

            _txthdr += "OSCA/PWD/PNSTM/SPIC ID No: ";
            tmp  = jacct["account_number"].asString();
            tmp += "\n";
            _txthdr += tmp;


            _txthdr += "Signature: _____________________________\n";
        }  else  {
        //int j = jaccount.size();
        //for (int i = 0; i < j; i++)  {
            //jacct = jaccount[i];
            tmp = " Account Type: ";
            tmp += jacct["account_type_desc"].asString();
            tmp += "\n";
            _txthdr += tmp;

            tmp = "     Account#: ";
            tmp += jacct["account_number"].asString();
            tmp += "\n";
            _txthdr += tmp;

            tmp = "         Name: ";
            tmp += jacct["first_name"].asString();
            tmp += " ";
            tmp += jacct["last_name"].asString();
            tmp += "\n";
            _txthdr += tmp;
        //}
        }
        _receipt += _txthdr;
    }

    //////////////////////////////////////
    //  Special receipt header(s)

    if (receiptType == salesReceiptType::SRT_REPRINT) {
        _txttype = "reprint";
        _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
    }
    else if (receiptType == salesReceiptType::SRT_POSTVOID) {
        _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_SUSPEND) {
        _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
    }
    else if (receiptType == salesReceiptType::SRT_CANCEL) {
        _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_REFUND) {
        _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
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

        _txthdr += _util->cyAcrossText(w," ");

        if (receiptType == salesReceiptType::SRT_REFUND) {
            tmp2 = "Trx No       : ";
            long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
            sprintf (sztmp, "%012ld", ltrx);
            tmp2 += sztmp;

            _txthdr += tmp2.c_str();
            _txthdr += newLine.c_str();

        }  else  {
            if (receiptType != salesReceiptType::SRT_SUSPEND &&
                receiptType == salesReceiptType::SRT_RESUME) {

                tmp = jheader["transaction_number"].asString();
                long ltrxnum = atol (tmp.c_str ());
                sprintf (sztmp, "%012ld", ltrxnum);

                tmp2 = "SI Number    : ";
                tmp3 = sztmp;
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {
                tmp2 = "Trx No       : ";
                long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
                sprintf (sztmp, "%012ld", ltrx);
                tmp2 += sztmp;

                _txthdr += tmp2.c_str();
                _txthdr += newLine.c_str();
                //ORIG SI NUMBER
                tmp2 = "ORIG-SI No   : ";
                ltrx = atol(jheader["picture_url"].asString ().c_str ());
                sprintf (sztmp, "%012ld", ltrx);
                tmp2 += sztmp;

                _txthdr += tmp2.c_str();
                _txthdr += newLine.c_str();
            }  else if (receiptType == salesReceiptType::SRT_RESUME) {
                tmp2 = "  Suspend # : ";
                tmp3 = jheader["transaction_number"].asString ();
                _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
            }  else if (receiptType == salesReceiptType::SRT_CANCEL) {
                tmp = jheader["transaction_number"].asString();
                long ltrxnum = atol (tmp.c_str ());
                sprintf (sztmp, "%012ld", ltrxnum);

                tmp2 = "Trx Number   : ";
                tmp2 += sztmp;
                tmp2 += "\n";
                _txthdr += tmp2;
            }  else if (receiptType != salesReceiptType::SRT_SUSPEND) {
                tmp2 = "SI Number    : ";
                long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
                sprintf (sztmp, "%012ld", ltrx);
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

    Json::Value jcashier;
    if (jheader["settings_cashier"] == Json::nullValue)
        return seterrormsg("Unable to get the cashier details");
    jcashier = jheader["settings_cashier"];

    size_t len = jcashier["first_name"].asString().length();
    len += jcashier["last_name"].asString().length();
    len += 14;
    len += jcashier["login"].asString().length();

    if (len > 40)  {
        _txthdr += "Cashier      : ";
        _txthdr += jcashier["first_name"].asString();
        _txthdr += " ";
        _txthdr += jcashier["last_name"].asString();

        _txthdr += "\n           [";
        _txthdr += jcashier["login"].asString();
        _txthdr += "]\n";
    }  else  {
        _txthdr += "Cashier      : ";
        _txthdr += jcashier["first_name"].asString();
        _txthdr += " ";
        _txthdr += jcashier["last_name"].asString();
        _txthdr += "[";
        _txthdr += jcashier["login"].asString();
        _txthdr += "]\n";
    }

    if (receiptType == SRT_REFUND)  {

        _txthdr += "Orig Pos No  : ";
        _txthdr += jheader["orig_register_number"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Orig Bus Date: ";
        _txthdr += jheader["orig_transaction_date"].asString();
        _txthdr += newLine.c_str();

        _txthdr += "Original SI# : ";

        char sztmp[32];
        long ltrx = atol(jheader["orig_transaction_number"].asString ().c_str ());
        sprintf (sztmp, "%012ld", ltrx);
        _txthdr += sztmp;
        _txthdr += newLine.c_str();
    }

    //////////////////////////////////////
    //  Transaction type
    _txthdr += _util->cyCenterText(w, " ");
    jtemp = jheader["settings_transtype"];
    _txthdr += _util->cyCenterText(w, jtemp["description"].asString());

    if (jheader["ba_redemption_reference"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, " ");
        _txthdr += "Reference Code : ";
        _txthdr += jheader["ba_redemption_reference"].asString();
        _txthdr += newLine.c_str();
    }

    _txthdr += _util->cyAcrossText(w, "=");
    _receipt += _txthdr;
    return true;
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptJoel::CYRestReceiptJoel (CYDbEnv* env, cycgi* cgi) : CYRestCommon (env, cgi)
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
CYRestReceiptJoel::CYRestReceiptJoel (CYDbEnv* env, cylog* log, cyini* ini, cycgi* cgi)
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
CYRestReceiptJoel::~CYRestReceiptJoel ()
{
    resetEodTotals();
    if (nullptr != _env)
        delete (_env);
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
bool CYRestReceiptJoel::saveReceipt()
{
    return true;
}

