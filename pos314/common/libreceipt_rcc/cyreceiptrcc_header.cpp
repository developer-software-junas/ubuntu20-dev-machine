//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRcc::assembleReceiptHeader (Json::Value& jheader,
                                          salesReceiptType receiptType)
{
    _txthdr = "";
    _receipt = "";
    _receiptType = receiptType;

    std::stringstream ss;
    ss << jheader;
    _log->logmsg("XXXXXX",ss.str().c_str ());

    //////////////////////////////////////
    //  Display the header
    bool originalDate = false;
    size_t  w = g_widthReceipt;
    string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
    Json::Value jtemp2, jtemp = jheader["hdr_branch"];
    Json::Value jHdrBranch = jheader["hdr_branch"];
    Json::Value jHdrSetting = jheader["hdr_settings"];
    Json::Value jHdrCompany = jheader["hdr_company"];
    Json::Value jHdrSetReg = jheader["settings_register"];
    Json::Value jHdrSetCshrShift = jheader["settings_cashier_shift"];
    //////////////////////////////////////
    //  Standard receipt header
    if (!receiptHeaderStart(jheader))
        return false;
    //////////////////////////////////////
    //  Make sure all objects are valid
    if (!jheader.isMember("hdr_branch"))
        return seterrormsg("No branch information found");
    Json::Value jbranch = jheader["hdr_branch"];
    if (!jheader.isMember("settings_transtype"))
        return seterrormsg("No transaction type information found");
    Json::Value jtranstype = jheader["settings_transtype"];
    //////////////////////////////////////
    //  PAC / BA / BRIDAL value(s)
    string pacNumber="",
        ridcNumber="",
        baReference="";
    if (jheader.isMember("pac_number"))
        pacNumber = jheader["pac_number"].asString();
    if (jheader.isMember("ridc_number"))
        ridcNumber = jheader["ridc_number"].asString();
    if (jheader.isMember("ba_redemption_reference"))
        baReference = jheader["ba_redemption_reference"].asString();
    _txthdr += "\n";
    //////////////////////////////////////
    //  Receipt type description
    if (receiptType == salesReceiptType::SRT_REPRINT) {
        originalDate = true;
        _txttype = "reprint";
        _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","REPRINT");
    }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {
        originalDate = true;
        _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","VOID");
    }  else if (receiptType == salesReceiptType::SRT_SUSPEND) {
        _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","SUSPEND");
    }  else if (receiptType == salesReceiptType::SRT_CANCEL) {
        _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","CANCEL");
    }  else if (receiptType == salesReceiptType::SRT_REFUND) {
        _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
        _log->logmsg("DEBUG RCC-RECEIPT ","RETURN");
    }  else {
        _log->logmsg("DEBUG RCC-RECEIPT ","REGULAR");
    }
    //////////////////////////////////////
    //  Register setting(s)
    if (!jheader.isMember("settings_register"))
        return seterrormsg("No register settings information found");
    Json::Value jregister = jheader["settings_register"];
    //////////////////////////////////////
    //  Formatted transaction #
    char sztrxnum[250];
    long lreg=0L, lstore=0L;
    memset (sztrxnum, 0x00, 250);
    if (jbranch.isMember("location_code"))
        lstore = atol (jbranch["location_code"].asString().c_str ());
    if (jregister.isMember("register_num"))
        lreg = atol (jregister["register_num"].asString().c_str ());
    sprintf (sztrxnum, "%02ld-%03ld", lstore , lreg);
    //////////////////////////////////////
    //  RCC date format
    string trans_time = _util->int_time(),
        trans_date = _util->date_eight();
    if (originalDate)  {
        if (jheader.isMember("transaction_date"))
            trans_date = jheader["transaction_date"].asString();
        if (jheader.isMember("transaction_time"))
            trans_time = jheader["transaction_time"].asString();
    }
    //////////////////////////////////////
    //  Transaction information
    string left, right;
    _txthdr += _util->cyAcrossText(40," ");
    if (receiptType == salesReceiptType::SRT_REFUND) {
        _isExtra = true;

        left =  "SI number   : ";
        left += refundTransnum(jheader);
        left += "\n";
        _txthdr += left;

        left  = "Date-time   : ";
        left += rccDate(trans_date,trans_time);
        left += "\n";
        _txthdr += left;
    }  else  {
        if (receiptType == salesReceiptType::SRT_RESUME) {
            left = "Suspend #   : ";
            right = legacyTransnum(jheader).c_str();
            _txthdr += _util->cyLRText(23,left,18,right);
        }  else if (receiptType != salesReceiptType::SRT_SUSPEND) {
            long ltrx = 0L;
            string trxcode="";
            if (jheader.isMember("transaction_code"))
                trxcode = jheader["transaction_code"].asString();
            if (trxcode != "30")  {
                left  = "SI Number  : ";
                left += sztrxnum;
                left += "-";
                if (jheader.isMember("transaction_number"))
                    ltrx = atol(jheader["transaction_number"].asString ().c_str ());

                sprintf (sztrxnum, "%09ld\n", ltrx);
                left += sztrxnum;
                _txthdr += left;

                left  = "Date-time  : ";
                left += rccDate(trans_date,trans_time);
                _txthdr += left;
                _txthdr += "\n";
            }  else  {
                left  = "Date-time      : ";
                left += rccDate(trans_date,trans_time);
                _txthdr += left;

                _txthdr += "Reference Code : ";
                _txthdr += jheader["ba_redemption_reference"].asString();
                _txthdr += "\n\n";
            }
        }
    }    
    if (receiptType != SRT_POSTVOID)  {
        if (jtranstype.isMember("description"))
            _txthdr += _util->cyCenterText(w, jtranstype["description"].asString());
    }
    _txthdr += _util->cyAcrossText(40, "=");
    _receipt += _txthdr;

    return true;
}
//*******************************************************************
//    FUNCTION:         receiptHeaderStart
// DESCRIPTION:         Receipt header
//*******************************************************************
bool                    CYReceiptRcc::receiptHeaderStart (Json::Value& jheader)
{
    _txthdr = "";
    _receipt = "";
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_widthReceipt;
    //////////////////////////////////////
    //  Make sure all objects are valid
    if (!jheader.isMember("hdr_branch"))
        return seterrormsg("No branch information found");
    Json::Value jbranch = jheader["hdr_branch"];
    if (!jheader.isMember("hdr_settings"))
        return seterrormsg("No settings information found");
    Json::Value jsettings = jheader["hdr_settings"];
    if (!jheader.isMember("settings_register"))
        return seterrormsg("No register settings information found");
    Json::Value jregister = jheader["settings_register"];
    //////////////////////////////////////
    //  Dsiplay the header message?
    if (jbranch.isMember("hdrmsg1"))
        _txthdr += _util->cyCenterText(w, jbranch["hdrmsg1"].asString());
    if (jbranch.isMember("hdrmsg2"))
        _txthdr += _util->cyCenterText(w, jbranch["hdrmsg2"].asString());
    if (jbranch.isMember("hdrmsg3"))
        _txthdr += _util->cyCenterText(w, jbranch["hdrmsg3"].asString());
    //////////////////////////////////////
    //  Display company information
    _txthdr += _util->cyCenterText(w, " ");
    if (jsettings.isMember("trade_name"))
        _txthdr += _util->cyCenterText(w, jsettings["trade_name"].asString());
    if (jbranch.isMember("description"))
        _txthdr += _util->cyCenterText(w, jbranch["description"].asString());
    if (jbranch.isMember("addr1"))
        _txthdr += _util->cyCenterText(w, jbranch["addr1"].asString());
    if (jbranch.isMember("addr2"))
        _txthdr += _util->cyCenterText(w, jbranch["addr2"].asString());
    //HARDCODED VALUE NEED ADD TO API
    _txthdr += _util->cyCenterText(w, "TEL NO. +632 865-7700");
    //////////////////////////////////////
    //  Device / tax information
    string line;
    if (jbranch.isMember("tin"))  {
        line  = "VAT-REG-TIN-";
        line += jbranch["tin"].asString();
        _txthdr += _util->cyCenterText(w, line);
    }
    if (jregister.isMember("serialno"))  {
        line = jregister["serialno"].asString();
        _txthdr += _util->cyCenterText(w, line);
    }
    if (jbranch.isMember("location_code") && jregister.isMember("register_num"))  {
        line  = "STORE # ";
        line += jbranch["location_code"].asString();
        line += " POS #";
        line += jregister["register_num"].asString();
        _txthdr += _util->cyCenterText(w, line);
    }
    //DEFAULT TEMPORARY
    _txthdr += "\nCUSTOMER ID: 999999\n";

    //////////////////////////////////////
    //  Zero rated???
    if (jheader.isMember("hdr_trxaccount"))  {
        Json::Value jtrxacct = jheader["hdr_trxaccount"];
        if (jtrxacct["is_zero_rated"].asString() == "1")  {
            _txthdr += "\nACCOUNT    : ";
            _txthdr += jtrxacct["account_type_desc"].asString();
            _txthdr += "\n";

            _txthdr += "CARD NO    : ";
            _txthdr += jtrxacct["account_number"].asString();
            _txthdr += "\n";

            _txthdr += "COMPANY    : ";
            _txthdr += jtrxacct["company_name"].asString();
            _txthdr += "\n";

            _txthdr += "NAME       : ";
            _txthdr += jtrxacct["first_name"].asString();
            _txthdr += " ";
            _txthdr += jtrxacct["last_name"].asString();
            _txthdr += "\n";
        }
    }

    return true;
}
