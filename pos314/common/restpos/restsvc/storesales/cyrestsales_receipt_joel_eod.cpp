//*******************************************************************
//        FILE:     cyrestsales_receipt_rcc_eod.cpp
// DESCRIPTION:     Sales receipt generation X AND Z RECEIPT
//*******************************************************************
#include "cyrestreceipt.h"
#include "cyrestreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoelEod::assembleEodReceipt(Json::Value jheader,
                                              string type)
{

   string rcptAssemble = "rcptaseembly";
    if (!assembleEodHeader(jheader,type))
        return false;
    _log->logmsg(rcptAssemble.c_str(),"header done");

    //FOR TEST ONLY-OK
    //std::stringstream ss;
    //ss <<jheader;
    //string msg = ss.str();
    //_log->logmsg("DEBUG JOEL EOD",msg.c_str());
    _txtpay = "";
    Json::Value jline,jpayment;
    jpayment = jheader["paytype"];
    int j = jpayment.size ();
    for (int i = 0; i < j; i++)  {
        jline = jpayment[i];
        if (!assembleEodPayment(jheader, jline))
            return false;
        //jpayment[i]=jline;
    }
    //////////////////////////////////////
    //  Add a payment total type for refunds
    double damt = stodsafe(jheader["refund_amount"].asString());
    if (damt != 0)  {
        string left, right;
        left  = "REFUND ";
        left += "(Count)";
        right = "Amount";
        _txtpay = _util->cyLRText(20,left,20,right);

        left  = " REFUND (";
        left += jheader["refund_count"].asString();
        left += ")";

        right = FMTNumberComma(damt);
        _txtpay += _util->cyLRText(20,left,20,right);

        left = "TOTAL REFUND (";
        left += jheader["refund_count"].asString();
        left += ")";

        right = FMTNumberComma(damt);
        _txtpay += _util->cyLRText(20,left,20,right);
        _txtpay += "\n";
        _receipt += _txtpay;
    }
    //////////////////////////////////////
    _log->logmsg(rcptAssemble.c_str(),"payment done");
    if (!assembleEodTotals(jheader))
        return false;
    _log->logmsg(rcptAssemble.c_str(),"total done");

    if (!assembleEodVat(jheader))
        return false;
    _log->logmsg(rcptAssemble.c_str(),"vat breakdown done");

    //BANK AQUIRER
    if (!assembleEodBank(jheader))
        return false;
    _log->logmsg(rcptAssemble.c_str(),"bank done");

    //CASHIERS AUDIT
    if (!assembleEodCashier(jheader,type))
        return false;
    _log->logmsg(rcptAssemble.c_str(),"cashier audit done");


    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoelEod::assembleEodHeader (Json::Value jheader,
                                              std::string type)
{
    _txthdr = "";
    _receipt = "";
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_widthReceipt;
    string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
    Json::Value jtemp2, jtemp = jheader["hdr_branch"];
    Json::Value jhdr_branch = jheader["hdr_branch"];
    Json::Value jhdr_settings = jheader["hdr_settings"];
    Json::Value jsettings_register = jheader["settings_register"];
    Json::Value jsettings_cashier = jheader["settings_cashier"];

    _txthdr += _util->cyCenterText(w, " ");
    _txthdr += _util->cyCenterText(w, jhdr_settings["trade_name"].asString());
    _txthdr += _util->cyCenterText(w, jhdr_branch["description"].asString());

    _txthdr += _util->cyCenterText(w, jhdr_branch["addr1"].asString());
    _txthdr += _util->cyCenterText(w, jhdr_branch["addr2"].asString());

    tmp  = "VAT-REG-TIN-";
    tmp += jhdr_branch["tin"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    tmp = jsettings_register["serialno"].asString();
    if (tmp.length() > w + 1) {
        tmp2 = tmp.substr(0, (w - 1));
    } else {
        tmp2 = tmp;
    }
    _txthdr += _util->cyCenterText(w, tmp2);

    tmp  = "Permit No. ";
    tmp += jsettings_register["permit"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    tmp  = "ACCR.# ";
    tmp += jhdr_branch["bir_num"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    _txthdr += _util->cyCenterText(w, " ");
    if (type == "zread"){
        _txthdr += _util->cyCenterText(w, "Z READING");
    } else
        _txthdr += _util->cyCenterText(w, "X READING");

    tmp  = "POS - ";
    tmp += jsettings_register["register_num"].asString();
    _txthdr += _util->cyCenterText(w,tmp);

    if (type == "xread")  {
        tmp  = "Shift ";
        tmp += jsettings_cashier["shift"].asString();
        tmp += " of ";
        tmp += jsettings_cashier["last_name"].asString();
        tmp += ", ";
        tmp += jsettings_cashier["first_name"].asString();
        tmp += " - ";
        tmp += jsettings_cashier["login"].asString();
        _txthdr += _util->cyCenterText(w,tmp);

        string start_time = jsettings_cashier["start_time"].asString();
        string end_time = jsettings_cashier["end_time"].asString();
        tmp  = "From: ";
        tmp += date_rcc(jsettings_cashier["start_date"].asString());
        tmp += "-";
        tmp += _util->time_colon(start_time.c_str());
        _txthdr += _util->cyCenterText(w,tmp);

        tmp  = "To: ";
        tmp += date_rcc(jsettings_cashier["end_date"].asString());
        tmp += "-";
        tmp += _util->time_colon(end_time.c_str());
        _txthdr += _util->cyCenterText(w,tmp);
    }  else  {
        string start_time = jheader["start_time"].asString();
        string end_time = jheader["end_time"].asString();
        tmp  = "From: ";
        tmp += date_rcc(jheader["start_date"].asString());
        tmp += "-";
        tmp += _util->time_colon(start_time.c_str());
        _txthdr += _util->cyCenterText(w,tmp);

        tmp  = "To: ";
        tmp += date_rcc(jheader["end_date"].asString());
        tmp += "-";
        tmp += _util->time_colon(end_time.c_str());
        _txthdr += _util->cyCenterText(w,tmp);
    }

    _txthdr += _util->cyAcrossText(40, "-");
    _txthdr += _util->cyCenterText(40,"Payment type(s)");
    _txthdr += _util->cyAcrossText(40, "-");
    _receipt += _txthdr;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoelEod::assembleEodPayment (Json::Value jheader, Json::Value jpayment)
{
    bool adjusted = false;
    string tmp, tmp2;
    string adjust_amount = "0.00";
    Json::Value jpay = jpayment["payment"], jpay2;

    double dpayAmt,dTotalAmt;
    //PER TENDER TYPE
    int j = jpay.size();
    for (int i = 0; i < j; i++){
        jpay2 = jpay[i];
        if (jpay2["pay_code"].asString () == "CP" && !adjusted)  {
            adjusted = true;
            adjust_amount = jheader["void_amount"].asString();
        }

        tmp = jpay2["pay_desc"].asString();
        tmp += "(Count)";
        tmp2 = "Amount";
        _txtpay = _util->cyLRText(24,tmp,16,tmp2);

        dpayAmt = stodsafe(jpay2["payamt"].asString());
        tmp  = " ";
        tmp += jpay2["pay_desc"].asString();
        tmp += " (";
        tmp += jpay2["pay_count"].asString();
        tmp += ")";

        tmp2 = FMTNumberComma(dpayAmt);

        _txtpay += _util->cyLRText(24,tmp,16,tmp2);

    }

    dTotalAmt =  stodsafe(jpayment["payamt"].asString());
    double dAdjustAmt = stodsafe(adjust_amount);
    dTotalAmt -= dAdjustAmt;

    tmp = "TOTAL ";
    tmp += jpayment["pay_type_desc"].asString();
    tmp += " (";
    tmp += jpayment["pay_count"].asString();
    tmp += ")";

    tmp2 = FMTNumberComma(dTotalAmt);
    _txtpay += _util->cyLRText(24,tmp,16,tmp2);;

    _txtpay += "\n";

    _receipt += _txtpay;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoelEod::assembleEodCashier (Json::Value jheader,
                                                              std::string type)
{
    _txtsig = "";
    char szTmp[255];
    string tmp,tmp2;
    _txtsig += _util->cyAcrossText(40,"-");
    _txtsig += _util->cyCenterText(40,"Cashier's Audit");
    _txtsig += _util->cyAcrossText(40,"-");

    double dTmp;

    tmp  = "No. Items Sold";
    dTmp = stodsafe(jheader["tax"]["item_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Sales Txn #";
    tmp2 =   jheader["sales_count"].asString();
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Non Sales Txn #";
    dTmp = stodsafe(jheader["tax"]["nonsalestxn"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Total Txn #";
    dTmp = stodsafe(jheader["sales_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Cancel Txn #";
    dTmp = stodsafe(jheader["cancel_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "No Items Suspended";
    dTmp = stodsafe(jheader["suspend_item_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "No Items Suspended (PAC)";
    dTmp = stodsafe(jheader["suspend_item_count_pac"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Suspended Txn #";
    dTmp = stodsafe(jheader["suspend_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Suspended Txn(PAC) #";
    dTmp = stodsafe(jheader["suspend_count_pac"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Total Void txn #";
    dTmp = stodsafe(jheader["void_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Tran Void Amt";
    dTmp = stodsafe(jheader["void_amount"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;


    tmp  = "Discount Amt";
    tmp2 =   FMTNumberComma(_totalTrxDiscount);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Total Refund Txn #";
    dTmp = stodsafe(jheader["refund_count"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Refund amt";
    dTmp = stodsafe(jheader["refund_amount"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "Ave Check";
    dTmp = stodsafe(jheader["tax"]["nonsalestxn"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

    //ADJUSTMENTS
    int startAdjustTrx = stodsafe(jheader["start_trxadjust"].asString());
    int endAdjustTrx = stodsafe(jheader["end_trxadjust"].asString());
    double startAdjAmt = stodsafe(jheader["start_adjust_balance"].asString());
    double endAdjAmt = stodsafe(jheader["end_adjust_balance"].asString());
    _txtsig += _util->cyAcrossText(40,"-");
    _txtsig += _util->cyCenterText(40,"Adjustment transaction nos.");
    sprintf(szTmp,"%d",startAdjustTrx);
    tmp = szTmp;
    tmp += " to ";
    sprintf(szTmp,"%d",endAdjustTrx);
    tmp += szTmp;
    _txtsig += _util->cyCenterText(40,tmp);
    _txtsig += "\n";
    tmp = "Old Adjusted Amount";
    tmp2 = FMTNumberComma(startAdjAmt);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;
    tmp = "New Adjusted Amount";
    tmp2 = FMTNumberComma(endAdjAmt);
    _txtsig += _util->cyLRText(20,tmp,20,tmp2);;
    _txtsig += _util->cyAcrossText(40,"-");
    ////////////////////////////////////////////
    //ADD CONDITION IF Z READ
    //IF ZEAD
    if (type=="zread"){
        _txtsig += _util->cyAcrossText(40,"=");
        tmp  = "OLD ACCUM SALES";
        if (jheader["start_balance"].asString().length() < 1)
            jheader["start_balance"]="0.00";
        dTmp = stodsafe(jheader["start_balance"].asString());
        tmp2 =   FMTNumberComma(dTmp);
        _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

        tmp  = "NEW ACCUM SALES";
        if (jheader["end_balance"].asString().length() < 1)
            jheader["end_balance"]="0.00";
        dTmp = stodsafe(jheader["end_balance"].asString());
        tmp2 =   FMTNumberComma(dTmp);
        _txtsig += _util->cyLRText(20,tmp,20,tmp2);;

        tmp = "ZREAD COUNT: ";
        tmp += jheader["zread_count"].asString();
        _txtsig += tmp;
        _txtsig += "\n";

        tmp = "RESET COUNT: 0";
        _txtsig += tmp;
        _txtsig += "\n";
    }

    _txtsig += _util->cyAcrossText(40,"=");
    ///////////////////////////////////////////
    _txtsig += _util->cyCenterText(40,"S.I. Nos.");

    tmp = jheader["start_trans"].asString();
    _txtsig += _util->cyCenterText(40,tmp);
    tmp = "to";
    _txtsig += _util->cyCenterText(40,tmp);
    tmp = jheader["end_trans"].asString();
    _txtsig += _util->cyCenterText(40,tmp);

    _txtsig += _util->cyCenterText(40,"GENERATED ON");
    string today,now;
    _util->int_date(today,0);
    now = _util->int_time();

    tmp = date_rcc(today);
    tmp += "-";
    tmp += _util->time_colon(now.c_str());
    _txtsig += _util->cyCenterText(40,tmp.c_str());
/*
    string manager = jheader["authorized_by"].asString();
    _txtsig += _util->cyCenterText(40,"Authorized by");
    _txtsig += _util->cyCenterText(40,manager.c_str());
*/
    _receipt += _txtsig;
    return true;
}


//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptJoelEod::assembleEodVat (Json::Value jheader)
{
    _txtvat = "";
    string tmp,tmp2;
    int w = g_widthReceipt;
    Json::Value jtax = jheader["tax"];
    double dTmp;

    _txtvat += _util->cyAcrossText(w, "-");
    _txtvat += _util->cyCenterText(w,"VAT");
    _txtvat += _util->cyAcrossText(w, "-");

    double netVat=0, netExempt=0, netZero=0, amtVat=0;

    tmp  = "VATable";
    dTmp = stodsafe(jtax["netvat"].asString());
    if (dTmp < 0.01) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);
    netVat = dTmp;

    tmp  = "VAT-Exempt";
    dTmp = stodsafe(jtax["netexempt"].asString());
    if (dTmp < 0.01) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);
    netExempt = dTmp;

    tmp  = "VAT-Zero";
    dTmp = stodsafe(jtax["netzero"].asString());
    if (dTmp < 0.01) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);;
    netZero = dTmp;

    tmp  = "NET SALES";
    dTmp = netVat + netZero + netExempt;
    if (dTmp < 0.01) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);

    tmp  = "VAT";
    dTmp = stodsafe(jtax["amtvat"].asString());
    if (dTmp < 0.015) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);
    amtVat = dTmp;

    tmp  = "GROSS SALES";
    dTmp = netVat + netZero + netExempt + amtVat;
    if (dTmp < 0.01) dTmp = 0;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);

    tmp  = "SERVICE CHARGE";
    dTmp = stodsafe(jtax["servicecharge"].asString());
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);;

    tmp  = "GRAND TOTAL";
    dTmp = netVat + netZero + netExempt + amtVat;
    tmp2 =   FMTNumberComma(dTmp);
    _txtvat += _util->cyLRText(20,tmp,20,tmp2);;
    _receipt += _txtvat;
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoelEod::assembleEodBank (Json::Value jheader)
{
    _txtacct = "";
    string tmp,tmp2;
    Json::Value jbank = jheader["bank"];
    Json::Value jline;
    double dTmp=0.00,dTotal=0.00,count=0;

    _txtacct += _util->cyAcrossText(40,"-");
    _txtacct += _util->cyCenterText(40,"Acquiring Bank");
    _txtacct += _util->cyAcrossText(40,"-");
    _txtacct += _util->cyLRText(20,"Bank (Count)",20,"Amount");


    int j = jbank.size();
    for (int i=0;i<j;i++) {
        jline = jbank[i];
        count += stodsafe(jline["pay_count"].asString());

        tmp = jline["bankdesc"].asString();
        tmp += " (";
        tmp += jline["pay_count"].asString();
        tmp += ")";

        dTmp = stodsafe(jline["payamt"].asString());
        dTotal += dTmp;
        tmp2 = FMTNumberComma(dTmp);
        _txtacct += _util->cyLRText(20,tmp,20,tmp2);;
    }

    char szTmp[255];
    sprintf(szTmp,"TOTAL (%d)",(int)count);
    tmp = szTmp;
    sprintf(szTmp,"%s",FMTNumberComma(dTotal).c_str());
    tmp2 = szTmp;
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);;
    _log->logmsg("bank side","bank done");




    /////////////////////////////////////
    //CASH FUND / PULLOUT
    _txtacct += _util->cyAcrossText(40,"-");
    _txtacct += _util->cyCenterText(40,"Cash fund / pullout");
    _txtacct += _util->cyAcrossText(40,"-");
    //cash fund
    tmp = "Change Fund";
    tmp2 = "Amount";
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    double cash_fund = stodsafe(jheader["fund"].asString());
    tmp = "Total Cash Fund";
    tmp2 = FMTNumberComma(cash_fund);
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    _txtacct += "\n";

    tmp  = "Cash pullout";
    tmp2 = "Amount";
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    Json::Value jcashTOut = jheader["takeout"];
    int x = jcashTOut.size();
    for (int i=0;i<x;i++) {
        jline = jcashTOut[i];
        double cash_takeout = stodsafe(jline["takeoutamt"].asString());
        tmp = "Total ";
        tmp += jline["tender_desc"].asString();
        tmp += " pullout";
        tmp2 = FMTNumberComma(cash_takeout);
        _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    }

    _txtacct += "\n";
    tmp = "TOTAL IN DRAWER";
    tmp2 = FMTNumberComma(0.00);
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    _log->logmsg("bank side","fund and takeout done");


    /////////////////////////////////////
    //TOTAL DECLARATION
    _txtacct += _util->cyAcrossText(40,"-");
    _txtacct += _util->cyCenterText(40,"Cash Declaration");
    _txtacct += _util->cyAcrossText(40,"-");
    tmp = "Tender Type";
    tmp2 = "Amount";
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    //double cashDec = stodsafe(jheader["cashdec"].asString());
    //////////////////////////////////////
    //  FIX!!!! JUNAS!!!!
    double cashDec = 0.00;
    Json::Value jtmp;
    //CRASHED HERE
    //cashDec = stodsafe (jtmp[1]["declared_amount"].asString ());

    if (!jheader["cashdec"].isNull())  {
        Json::Value jfixCashDec;
        jfixCashDec = jheader["cashdec"];
        j = jfixCashDec.size ();
        for (int i = 0; i < j; i++)  {
            jline = jfixCashDec[i];
            if (jline["is_cash"].asString() == "1")
                cashDec += stodsafe(jline["declared_amount"].asString());
        }
    }
    tmp = "CASH";
    tmp2 = FMTNumberComma(cashDec);
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    _log->logmsg("bank side","declaration done");


    _txtacct += _util->cyAcrossText(40,"-");
    _txtacct += _util->cyCenterText(40,"Over / Short");
    _txtacct += _util->cyAcrossText(40,"-");

    //////////////////////////////////////
    //  FIX!!!! JUNAS!!!!
    double dTcash=0, overShort = 0;
    //ALSO CRASHED HERE
    //dTcash = stodsafe (jtmp[1]["payamt"].asString ());
    if (!jheader["cash"].isNull())  {
        Json::Value jfixCash;
        jfixCash = jheader["cash"];
        if (jfixCash.isArray())  {
            j = jfixCash.size ();
            for (int i = 0; i < j; i++)  {
                jline = jfixCash[i];
                //if (jline["is_cash"].asString() == "1")
                    dTcash += stodsafe(jline["payamt"].asString());
            }
        }  else  {
            dTcash += stodsafe(jfixCash["payamt"].asString());
        }
    }

    overShort = cashDec - dTcash;
    if (overShort == 0)
        tmp  = "Over / Short";
    else if (overShort < 0)
        tmp  = "Short";
    else if (overShort > 0)
        tmp  = "Over";

    tmp2 = FMTNumberComma(overShort < 0 ? overShort *= -1 : overShort);
    _txtacct += _util->cyLRText(20,tmp,20,tmp2);
    _log->logmsg("bank side","over short done");
    _txtacct += _util->cyAcrossText(40,"-");

    //////////////////////////////////////
    ////PER BANK
    //Json::Value jpayment;
    //double  dpayAmt = 0;
    //jpayment = jheader["paytype"];
    //int jj = jpayment.size ();
    //for (int i = 0; i < jj; i++)  {
    //    jline = jpayment[i];
    //    Json::Value jpay = jline["payment"], jpay2;
    //    int x = jpay.size();
    //    for (int i = 0; i < x; i++){
    //        jpay2 = jpay[i];
    //        dpayAmt = stodsafe(jpay2["payamt"].asString());
    //        tmp  = jpay2["pay_desc"].asString();
    //        tmp += " (";
    //        tmp += jpay2["pay_count"].asString();
    //        tmp += ")";
    //        tmp2 = FMTNumberComma(dpayAmt);
    //        _txtacct += _util->cyLRText(20,tmp,20,tmp2);;
    //    }
    //}

    _receipt += _txtacct;
    return true;
}


//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptJoelEod::assembleEodTotals (Json::Value jheader)
{
    _txttot = "";
    char sztmp[255];
    string tmp = "",tmp2;
    Json::Value jtotal;
    Json::Value jtrans =  jheader["sysdate"];
    double dTotalNcash = 0.00;
    double dTotalCash = 0.00;
    if (!jtotal.isNull())  {
        tmp = jtotal["payamt"].asString();
        if (tmp.length() < 1)  tmp = "0";
        dTotalNcash = stodsafe(tmp);
    }
    //CASH
    jtotal =  jheader["cash"];
    if (!jtotal.isNull())  {
        tmp = jtotal["payamt"].asString();
        if (tmp.length() < 1)  tmp = "0";
        dTotalCash = stodsafe(tmp);
    }
    _txttot += _util->cyAcrossText(40, "-");
    int cashCount = 0;
    if (jtotal["pay_count"].asString() != ""){
        cashCount = atoi(jtotal["pay_count"].asString().c_str());
    }
    sprintf(sztmp,"%d",cashCount);
    tmp = "TOTAL CASH";
    tmp += " (";
    tmp += sztmp;
    tmp += ") ";
    tmp2 = FMTNumberComma(dTotalCash);
    _txttot += _util->cyLRText(20,tmp,20,tmp2);

    //NON CASH
    jtotal =  jheader["notcash"];
    int nCashCount = 0;
    if (jtotal["pay_count"].asString() != ""){
        nCashCount = atoi(jtotal["pay_count"].asString().c_str());
    }
    dTotalNcash = stodsafe(jtotal["payamt"].asString());

    tmp = FMTNumberComma(dTotalNcash);
    sprintf(sztmp,"%d",nCashCount);
    tmp = "TOTAL NON CASH";
    tmp += " (";
    tmp += sztmp;
    tmp += ") ";

    tmp2 = FMTNumberComma(dTotalNcash);
    _txttot += _util->cyLRText(20,tmp,20,tmp2);;

    //TOTAL SALES
    tmp = "TOTAL SALES";
    tmp += " (";
    tmp += jheader["sales_count"].asString();
    tmp += ") ";
    tmp2 = FMTNumberComma(dTotalNcash+dTotalCash);
    _txttot += _util->cyLRText(20,tmp,20,tmp2);
    //////////////////////////////////////
    //DISCOUNT PART
    Json::Value jbank = jheader["discount"];
    Json::Value jline;
    double dTmp=0.00,dTotal=0.00,count=0;

    _txttot += _util->cyAcrossText(40, "-");
    _txttot += _util->cyCenterText(40,"Discount");
    _txttot += _util->cyAcrossText(40, "-");
    _txttot += _util->cyLRText(20,"Discount (Count)",20,"Amount");

    int j = jbank.size();
    for (int i=0;i<j;i++) {
        jline = jbank[i];
        count += stodsafe(jline["discount_count"].asString());

        tmp = jline["discount_cd"].asString();
        tmp += " (";
        tmp += jline["discount_count"].asString();
        tmp += ")";

        dTmp = stodsafe(jline["discount_amount"].asString());
        dTotal += dTmp;
        tmp2 = FMTNumberComma(dTmp);
        _txttot += _util->cyLRText(20,tmp,20,tmp2);;
    }

    char szTmp[255];
    sprintf(szTmp,"TOTAL Discount(%d)",(int)count);
    tmp = szTmp;
    sprintf(szTmp,"%s",FMTNumberComma(dTotal).c_str());
    tmp2 = szTmp;
    _txttot += _util->cyLRText(20,tmp,20,tmp2);;
    _totalTrxDiscount =  dTotal;
    _receipt += _txttot;

    return true;
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptJoelEod::CYRestReceiptJoelEod (CYDbEnv* env, cycgi* cgi) : CYRestCommon (env, cgi)

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
CYRestReceiptJoelEod::CYRestReceiptJoelEod (CYDbEnv* env, cylog* log, cyini* ini, cycgi* cgi)
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
CYRestReceiptJoelEod::~CYRestReceiptJoelEod ()
{
    resetEodTotals();
    if (nullptr != _env)
        delete (_env);
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
bool CYRestReceiptJoelEod::saveReceipt()
{
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create JSON for xread
//*******************************************************************
bool CYRestReceiptJoelEod::retrieveXread(Json::Value& jheader,
                                         std::string logical_date,
                                         std::string location_code,
                                         std::string register_num,
                                         std::string shift,
                                         std::string cashier,
                                         std::string manager,
                                         std::string mgrpass)
{
    (void)mgrpass;
    char szFunc [] = "xRead";
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbsel = dbconn ();
    if (!dbsel->ok()){
        _error = dbsel->errordb();
        return false;
    }

    CYDbSql* dbsel2 = dbconn ();
    if (!dbsel2->ok()){
        _error = dbsel2->errordb();
        return false;
    }

    if (logical_date.length() < 1)  {
        _error =  "Invalid business date";
        return false;
    }
    dbsel->sql_reset();
    string sql, hold_register="", register_alias = "0";
    sql  = " select register_alias from pos_register where register_num = ";
    sql += dbsel->sql_bind (1, register_num);
    sql += " and    location_code = ";
    sql += dbsel->sql_bind (2, location_code);
    if (!dbsel->sql_result (sql,true)){
        _error =  dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ()) {
        register_alias = dbsel->sql_field_value("register_alias");
        if (register_alias.length() > 0 && register_alias != "0")  {
            hold_register = register_num;
            register_num = register_alias;
        }
    }
    ///////////////////////////////////////////////////////
    //  Xread json - settings
    //////////////////////////////////////
    //  REGISTER / BRANCH
    Json::Value jsettings_register,jsettings_branch,
                jsettings, jsettings_cashier;
    //////////////////////////////////////
    //  register settings
    dbsel->sql_reset ();
    sql  = "select   * ";
    sql += "from     pos_register ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, register_num);

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)
            jsettings_register[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    }
    ////////////////////////////////////////
    //  branch settings
    dbsel->sql_reset ();
    sql  = "select   * ";
    sql += "from     cy_location ";
    sql += "where    location_code = ";
    sql += dbsel->sql_bind(1, location_code);

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)
            jsettings_branch[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    }
    ////////////////////////////////////////
    //POS SETTINGS
    sql  = "select   * ";
    sql += "from     pos_settings ";

    if (!dbsel->sql_result (sql,false))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)
            jsettings[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    }
    //////////////////////////////////////
    //  cashier settings
    sql  = "select   a.first_name,a.last_name,a.mi,a.login ";
    sql += "from     cy_user a ";
    sql += "where    a.login = ";
    sql += dbsel->sql_bind(1, cashier);

    if (!dbsel->sql_result (sql,true)) {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)
            jsettings_cashier[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    }
    //////////////////////////////////////
    //  Start / end date
    dbsel->sql_reset();
    sql  = "select transaction_date, transaction_time from tg_pos_daily_header ";
    sql += "where branch_code = ";
    sql += dbsel->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbsel->sql_bind(2,logical_date);
    sql += " and register_number = ";
    sql += dbsel->sql_bind(3,register_num);
    sql += " and cashier = ";
    sql += dbsel->sql_bind(4,cashier);
    sql += " and cashier_shift = ";
    sql += dbsel->sql_bind(5,shift);
    sql += " order by transaction_date, transaction_time limit 1";
    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jheader["start_date"]=dbsel->sql_field_value("transaction_date");
        jheader["start_time"]=dbsel->sql_field_value("transaction_time");
    }  else  {
        jheader["start_date"]=logical_date;
        jheader["start_time"]="0001";
    }
    dbsel->sql_reset();
    sql  = "select transaction_date, transaction_time from tg_pos_daily_header ";
    sql += "where branch_code = ";
    sql += dbsel->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbsel->sql_bind(2,logical_date);
    sql += " and register_number = ";
    sql += dbsel->sql_bind(3,register_num);
    sql += " and cashier = ";
    sql += dbsel->sql_bind(4,cashier);
    sql += " and cashier_shift = ";
    sql += dbsel->sql_bind(5,shift);
    sql += " order by transaction_date desc, transaction_time desc limit 1";
    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jheader["end_date"]=dbsel->sql_field_value("transaction_date");
        jheader["end_time"]=dbsel->sql_field_value("transaction_time");
    }  else  {
        jheader["end_date"]=logical_date;
        jheader["end_time"]="0001";
    }
    jsettings_cashier["end_date"]=jheader["end_date"].asString();
    jsettings_cashier["end_time"]=jheader["end_time"].asString();
    jsettings_cashier["start_date"]=jheader["start_date"].asString();
    jsettings_cashier["start_time"]=jheader["start_time"].asString();
    ///////////////////////////////////////////////////////
    CYDbSql* dbeod = dbconn ();
    if (!dbeod->ok()){
        _error = _util->jsonerror(szFunc,_env->errordbenv());
        return false;
    }

    dbeod->sql_reset ();
    sql = "select * from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and cashier = ";
    sql += dbeod->sql_bind(3,cashier);
    sql += " and shift = ";
    sql += dbeod->sql_bind(4,shift);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(5,register_num);
    if (!dbeod->sql_result(sql,true)){
        _error =  dbeod->errordb();
        return false;
    }
    if (dbeod->eof ()){
        //////////////////////////////////
        //  Can Zread
        dbsel->sql_reset ();
        sql = "update tg_pos_xread_cashier set xread_done = 1 ";
        sql += "where location_code = ";
        sql += dbsel->sql_bind(1,location_code);
        sql += " and logical_date = ";
        sql += dbsel->sql_bind(2,logical_date);
        sql += " and cashier = ";
        sql += dbsel->sql_bind(3,cashier);
        sql += " and shift = ";
        sql += dbsel->sql_bind(4,shift);
        sql += " and register_num = ";
        sql += dbsel->sql_bind(5,register_num);
        if (!dbsel->sql_only(sql,true)){
            _error =  dbsel->errordb();
            return false;
        }
        if (!dbsel->sql_commit()){
            _error =  dbsel->errordb();
            return false;
        }
        //////////////////////////////////
        //  POS register
        dbsel->sql_reset ();
        sql = "select * from pos_register ";
        sql += "where location_code = ";
        sql += dbsel->sql_bind(1,location_code);
        sql += " and register_num = ";
        sql += dbsel->sql_bind(2,register_num);
        if (!dbsel->sql_result(sql,true)){
            _error =  dbsel->errordb();
            return false;
        }
        if (!dbsel->eof ()){
            //////////////////////////////////////
            //  Cashier from / to



            jheader["end_num"]=dbsel->sql_field_value("transaction_count");
            jheader["start_num"]=dbsel->sql_field_value("transaction_count");
            jheader["start_trans"]=dbsel->sql_field_value("transaction_count");

            jheader["end_trans"]=dbsel->sql_field_value("transaction_count");

            jheader["end_balance"]=dbsel->sql_field_value("transaction_amount");
            jheader["start_balance"]=dbsel->sql_field_value("transaction_amount");

            jheader["end_trxadjust"]=dbsel->sql_field_value("adjust_amount");
            jheader["start_trxadjust"]=dbsel->sql_field_value("adjust_amount");

            jheader["end_adjust_balance"]=dbsel->sql_field_value("last_adjust_amount");
            jheader["start_adjust_balance"]=dbsel->sql_field_value("last_adjust_amount");
        }
    }  else  {
        //////////////////////////////////////
        //  Cashier from / to

        jheader["end_num"]=dbeod->sql_field_value("end_trxnum");
        jheader["start_num"]=dbeod->sql_field_value("start_trxnum");
        jheader["start_trans"]=dbeod->sql_field_value("start_trxnum");

        jheader["end_trans"]=dbeod->sql_field_value("end_trxnum");

        jheader["end_balance"]=dbeod->sql_field_value("end_balance");
        jheader["start_balance"]=dbeod->sql_field_value("start_balance");

        jheader["end_trxadjust"]=dbeod->sql_field_value("end_trxadjust");
        jheader["start_trxadjust"]=dbeod->sql_field_value("start_trxadjust");

        jheader["end_adjust_balance"]=dbeod->sql_field_value("end_adjust_balance");
        jheader["start_adjust_balance"]=dbeod->sql_field_value("start_adjust_balance");
    }
    //////////////////////////////////////
    //  Can Zread
    int idx = 0;
    dbsel->sql_reset ();
    sql  = "select count(*) as xread_cashier from tg_pos_xread_cashier ";
    sql += "where  register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " and   xread_done = 0 ";
    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string xread_cashier = dbsel->sql_field_value("xread_cashier");
    if (xread_cashier.length () < 1)  xread_cashier = "0";
    //////////////////////////
    //  Get the manager name
    dbsel->sql_reset();
    if (manager.length() < 1)  cgiform("manager");
    if (manager.length() < 1)  manager = cashier;
    sql  = "select * from cy_user ";
    sql += "where  login = ";
    sql += dbsel->sql_bind(1, manager);

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }

    string authorized_by  = dbsel->sql_field_value("first_name");
    authorized_by += " ";
    authorized_by += dbsel->sql_field_value("last_name");
    authorized_by += " (";
    authorized_by += dbsel->sql_field_value("login");
    authorized_by += ")";
    //////////////////////////////////////
    //  cash declaration
    idx = 0;
    Json::Value jline, jcashdec;

    dbsel->sql_reset ();
    sql  = "select   a.declared_amount, b.description, c.is_cash  ";
    sql += "from     cy_cash_dec a, pos_tender b, pos_tender_type c ";
    sql += "where    a.tender_code = b.tender_cd ";
    sql += " and     b.tender_type_cd = c.tender_type_cd ";
    sql += " and     a.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     a.register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     a.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     a.transaction_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     a.location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " order by b.description ";

    if (!dbsel->sql_result (sql,true)){
        _error = sql.c_str();
        return false;
    }
    if (dbsel->eof ())
        jcashdec = Json::nullValue;
    else  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jcashdec[idx]=jline;
            idx++;
            dbsel->sql_next ();
        }  while(!dbsel->eof ());
    }

    //////////////////////////////////////
    //  Payment summary
    idx = 0;
    int idx2 = 0;
    Json::Value jline2;
    Json::Value jpaytype;
    Json::Value jpay;

    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, ";
    sql += "         tender_type_code as pay_type_code,tender_type_desc as pay_type_desc ";
    sql += "from     tg_pos_xread_payment a ";
    sql += "inner join tg_pos_daily_header b on ";
    sql += "         a.systransnum = b.systransnum ";
    sql += "where    a.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     a.register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     a.shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     a.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     a.location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " group by a.tender_type_code,a.tender_type_desc  ";
    sql += "order by a.tender_type_code,a.tender_type_desc ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            string ptc = dbsel->sql_field_value("pay_type_code");

            jpay.clear ();
            dbsel2->sql_reset ();
            sql  = "select   count(*) as pay_count, sum(amount) as payamt, ";
            sql += "         a.tender_type_code as pay_type_code, a.tender_code as pay_code, ";
            sql += "         a.tender_type_desc as pay_type_desc, a.tender_desc as pay_desc ";
            sql += "from     tg_pos_xread_payment a ";
            sql += "inner join tg_pos_daily_header b on ";
            sql += "         a.systransnum = b.systransnum ";
            sql += "where    a.cashier = ";
            sql += dbsel2->sql_bind(1, cashier);
            sql += " and     a.register_num = ";
            sql += dbsel2->sql_bind(2, register_num);
            sql += " and     a.shift = ";
            sql += dbsel2->sql_bind(3, shift);
            sql += " and     a.logical_date = ";
            sql += dbsel2->sql_bind(4, logical_date);
            sql += " and     a.location_code = ";
            sql += dbsel2->sql_bind(5, location_code);
            sql += " and     a.tender_type_code = ";
            sql += dbsel2->sql_bind(6, ptc);
            sql += " group by a.tender_type_code, a.tender_type_desc, a.tender_code, a.tender_desc  ";
            sql += "order by tender_desc ";

            if (!dbsel2->sql_result (sql,true)){
                _error = dbsel2->errordb();
                return false;
            }
            if (!dbsel2->eof ())  {
                idx2 = 0;
                jpay.clear ();
                do  {
                    jline2.clear ();
                    for (int x = 0; x < dbsel2->numcols(); x++)
                        jline2[dbsel2->sql_field_name(x)]=dbsel2->sql_field_value (dbsel2->sql_field_name(x));
                    jpay[idx2]=jline2;
                    idx2++;
                    dbsel2->sql_next ();
                } while(!dbsel2->eof());
                jline["payment"]=jpay;
            }
            jpaytype[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  Bank summary
    idx = 0;
    Json::Value jbank;
    idx = 0;
    dbsel->sql_reset ();

    sql  = "select   count(*) as pay_count, sum(amount) as payamt, bank_desc as bankdesc ";
    sql += "from     tg_pos_xread_bank ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " group by bank_desc order by bank_desc ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jbank[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  cash summary
    Json::Value jcash;
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, cash as is_cash ";
    sql += "from     tg_pos_xread_payment ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " and cash = 1 group by cash  ";
    sql += "order by cash ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jcash[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  non cash summary
    Json::Value jnoncash;
    idx = 0;
    dbsel->sql_reset ();
    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, cash as is_cash ";
    sql += "from     tg_pos_xread_payment ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " and cash <> 1 group by cash  ";
    sql += "order by cash ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jnoncash[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  Tax summary
    Json::Value jtax;
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   SUM(net_vat) as netvat, SUM(net_zero) as netzero, ";
    sql += "         SUM(net_exempt) as netexempt, SUM(amt_vat) as amtvat, ";
    sql += "         SUM(amt_zero) as amtzero, SUM(amt_exempt) as amtexempt, ";
    sql += "         SUM(net_amount) as salesnet, SUM(gross_amount) as salesgross, ";
    sql += "         SUM(service_charge) as servicecharge ";
    sql += "from     tg_pos_xread_tax ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " group by cashier, register_num ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jtax[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  Total item(s)
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   SUM(quantity) as item_count from tg_pos_mobile_detail ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_daily_header z ";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,false)){
        _error = dbsel->errordb();
        return false;
    }
    string item_count = dbsel->sql_field_value("item_count");
    jtax["item_count"]=item_count;
    //////////////////////////////////////
    //  discount summary
    idx = 0;
    Json::Value jdiscount;
    dbsel->sql_reset ();
    sql  = "select   count(*) as discount_count, sum(amount) as discount_amount, ";
    sql += "         discount_desc as description,discount_code as discount_cd ";
    sql += "from     tg_pos_xread_discount ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and     shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    /*
    sql += " and     systransnum in (select z.systransnum from tg_pos_daily_header z where ";
    sql += "         z.cashier = ";
    sql += dbsel->sql_bind(6, cashier);
    sql += " and     z.register_number = ";
    sql += dbsel->sql_bind(7, register_num);
    sql += " and     z.cashier_shift = ";
    sql += dbsel->sql_bind(8, shift);
    sql += " and     z.logical_date = ";
    sql += dbsel->sql_bind(9, logical_date);
    sql += " and     z.branch_code = ";
    sql += dbsel->sql_bind(10, location_code);*/
    sql += " group by discount_desc,discount_code order by discount_desc ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    idx = 0;
    while (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jdiscount[idx]=jline;
        idx++;
        dbsel->sql_next ();
    }
    //////////////////////////////////////
    //  Cash fund summary
    idx = 0;
    Json::Value jfund;

    dbsel->sql_reset ();

    sql  = "select   count(*) as fund_count, sum(cash_fund_amt) as fundamt, ";
    sql += "         tender_code, tender_desc ";
    sql += "from     pos_cash_fund ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " group by tender_code, tender_desc ";
    sql += "order by tender_code, tender_desc ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jfund[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  Cash takeout summary
    idx = 0;
    Json::Value jtakeout;

    dbsel->sql_reset ();

    sql  = "select   count(*) as takeout_count, sum(denomination * qty) as takeoutamt, ";
    sql += "         tender_code, tender_desc ";
    sql += "from     pos_cash_takeout ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_num = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += " group by tender_code, tender_desc ";
    sql += "order by tender_code, tender_desc ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jtakeout[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }  else  {
        dbsel->sql_reset();
        sql  = "select   '1' as takeout_count, '0' as takeoutamt, ";
        sql += "         'CP' as tender_code, 'CASH PESO' as tender_desc ";

        if (!dbsel->sql_result (sql,true))
        {
            _error = dbsel->errordb();
            return false;
        }

        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jtakeout[0]=jline;
    }
    //////////////////////////////////////
    //  Voided transaction(s)
    string void_count = dbeod->sql_field_value("audit_trx_void");
    string void_amount = dbeod->sql_field_value("audit_trx_void_amount");
    //////////////////////////////////////
    //  Suspended item count
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   SUM(quantity) as suspend_item_count from tg_pos_mobile_detail ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_suspend_header z ";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string suspend_item_count = dbsel->sql_field_value("suspend_item_count");
    //////////////////////////////////////
    //  Suspend count
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_suspend_header z ";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string suspend_count = dbsel->sql_field_value("recs");
    //////////////////////////////////////
    //  Cancel count
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_cancel_header z ";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string cancel_count = dbsel->sql_field_value("recs");

    //////////////////////////////////////
    //  Total transaction(s)
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_daily_header z ";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string sales_count = dbsel->sql_field_value("recs");
    //////////////////////////////////////
    //  Refund count
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   count(*) as refund_count from tg_pos_refund_header ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(5, location_code);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string refund_count = dbsel->sql_field_value("refund_count");
    //////////////////////////////////////
    //  Refund amount
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select   sum(payment_amount) as refund_amount from tg_pos_mobile_payment ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_refund_header z";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string refund_amount = dbsel->sql_field_value("refund_amount");
    //////////////////////////////////////
    //  VOID count
    dbsel->sql_reset ();
    sql  = "select   count(*) as void_count from tg_pos_void_header ";
    sql += "where    cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and     register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(5, location_code);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    void_count = dbsel->sql_field_value("void_count");
    //////////////////////////////////////
    //  void amount
    dbsel->sql_reset ();
    sql  = "select   sum(payment_amount-change_amount) as void_amount from tg_pos_mobile_payment ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_void_header z";
    sql += "                         where z.cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += "                           and z.register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += "                           and z.cashier_shift = ";
    sql += dbsel->sql_bind(3, shift);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(5, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    void_amount = dbsel->sql_field_value("void_amount");
    //////////////////////////////////////
    //  Z read item count
    idx = 0;
    dbsel->sql_reset ();
    sql  = "select * from pos_register ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string xread_count = dbsel->sql_field_value("xreadcount");
    string zread_count = dbsel->sql_field_value("zreadcount");
    string reset_count = dbsel->sql_field_value("resetcount");
    if (xread_count.length () < 1)  xread_count = "0";
    if (zread_count.length () < 1)  zread_count = "0";
    if (reset_count.length () < 1)  reset_count = "0";
    //////////////////////////////////////
    //  Get cashier's with ordering
    //  privileges only
    //////////////////////////////////////
    //  Sysdate
    idx = 0;
    Json::Value jsysdate;
    dbsel->sql_reset ();
    sql  = "select   * ";
#ifdef _CYSTORESERVER
    sql += "from     pos_sysdate ";
#else
    sql += "from     sales.pos_sysdate ";
#endif
    sql += "where transaction_date = ";
    sql += dbsel->sql_bind(1,logical_date);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jsysdate[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    jsysdate["location_code"]="";
    //////////////////////////////////////
    //  Update the shift total(s)
    //  Total sales
    string totcashpay = jcash["payamt"].asString();
    if (totcashpay.length() < 1)
        totcashpay = "0";
    string totnoncash = jnoncash["payamt"].asString();
    if (totnoncash.length() < 1)
        totnoncash = "0";
    double d1 = stodsafe(totcashpay);
    double d2 = stodsafe(totnoncash);

    char sztmp[64];
    snprintf (sztmp, 30, "%.02f", (d1 + d2));
    string transaction_amount = sztmp;
    //  VAT amount
    string netvat = jtax["netvat"].asString();
    if (netvat.length() < 1)
        netvat = "0";
    string netamt = jtax["netamt"].asString();
    if (netamt.length() < 1)
        netamt = "0";
    //  NONVAT amount
    string netzero = jtax["netzero"].asString();
    if (netzero.length() < 1)
        netzero = "0";
    string netexempt = jtax["netexempt"].asString();
    if (netexempt.length() < 1)
        netexempt = "0";

    d1 = stodsafe(netamt);
    d2 = stodsafe(netvat);
    snprintf (sztmp, 30, "%.02f", (d2-d1));
    transaction_amount = sztmp;

    //////////////////////////////////////
    //  NONE sales transaction(s)
    jtax["nonsalestxn"]=dbeod->sql_field_value("audit_trx_nonsales");
    //////////////////////////////////////
    //  Assemble all JSON object(s)
    jheader["status"]="ok";
    jheader["tax"]=jtax;
    jheader["bank"]=jbank;
    jheader["cash"]=jcash;
    jheader["fund"]=jfund;
    jheader["sysdate"]=jsysdate;
    jheader["cashdec"]=jcashdec;
    jheader["takeout"]=jtakeout;
    jheader["notcash"]=jnoncash;
    jheader["paytype"]=jpaytype;
    jheader["discount"]=jdiscount;
    jheader["settings_register"]=jsettings_register;
    jheader["hdr_branch"]=jsettings_branch;
    jheader["hdr_settings"]=jsettings;
    jheader["settings_cashier"]=jsettings_cashier;

    jheader["void_count"]=void_count;
    jheader["zread_count"]=zread_count;
    jheader["xread_count"]=xread_count;
    jheader["reset_count"]=reset_count;
    jheader["void_amount"]=void_amount;
    jheader["sales_count"]=sales_count;
    jheader["cancel_count"]=cancel_count;
    jheader["suspend_count"]=suspend_count;
    jheader["suspend_item_count"]=suspend_item_count;

    jheader["refund_count"]=refund_count;
    jheader["refund_amount"]=refund_amount;

    jheader["suspend_count_pac"]="0";
    jheader["suspend_item_count_pac"]="0";

    jheader["authorized_by"]=authorized_by;
    jheader["can_zread"]=(atoi(xread_cashier.c_str()) == 0);
    _log->trace("xread cashier count",xread_cashier.c_str ());

    return true;
}
