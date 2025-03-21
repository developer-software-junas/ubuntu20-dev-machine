//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrico.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRico::assembleReceiptHeader (Json::Value& jheader,
                                           salesReceiptType receiptType)
{
    //////////////////////////////////////
    //  Receipt type
    _receiptType = receiptType;
    //////////////////////////////////////
    //  Clear the receipt / header buffer
    _txthdr = "";
    _receipt = "";
    //////////////////////////////////////
    //  Receipt width
    _rcptWidth = g_widthReceipt;
    int left = 23, right = 17;
    string skinny = cgiform("receipt_skinny");
    if (skinny == "true")  {
        left = 18;
        right = 13;
        _rcptWidth = 30;
    }
    //////////////////////////////////////
    //  Header object(s)
    Json::Value jhdr_branch = jheader["hdr_branch"];
    Json::Value jhdr_setting = jheader["hdr_settings"];
    Json::Value jhdr_company = jheader["hdr_company"];
    Json::Value jsettings_register = jheader["settings_register"];
    Json::Value jsettings_cashier_shift = jheader["settings_cashier_shift"];
    //////////////////////////////////////
    //  Optional message(s)
    if (jhdr_branch["hdrmsg1"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["hdrmsg1"].asString());
    }
    if (jhdr_branch["hdrmsg2"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["hdrmsg2"].asString());
    }
    if (jhdr_branch["hdrmsg3"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["hdrmsg3"].asString());
    }
    //////////////////////////////////////
    //  Display the header
    string tmp, tmp2;
    _txthdr += _util->cyCenterText(_rcptWidth, " ");
    _txthdr += _util->cyCenterText(_rcptWidth, jhdr_setting["trade_name"].asString());
    _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["description"].asString());

    _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["addr1"].asString());
    _txthdr += _util->cyCenterText(_rcptWidth, jhdr_branch["addr2"].asString());

    tmp  = "VAT-REG-TIN-";
    tmp += jhdr_branch["tin"].asString();
    _txthdr += _util->cyCenterText(_rcptWidth, tmp);

    tmp = jsettings_register["serialno"].asString();
    if (tmp.length() > (size_t)(_rcptWidth + 1)) {
        tmp2 = tmp.substr(0, (_rcptWidth - 1));
    } else {
        tmp2 = tmp;
    }
    _txthdr += _util->cyCenterText(_rcptWidth, tmp2);

    tmp  = "STORE # ";
    tmp += jsettings_cashier_shift["location_code"].asString();
    tmp += " ";
    tmp += "POS #";
    tmp += jsettings_cashier_shift["register_num"].asString();

    _txthdr += _util->cyCenterText(_rcptWidth,tmp);
    _txthdr += _util->cyCenterText(_rcptWidth," ");
    //////////////////////////////////////
    //  Real invoice?
    if (!_isInternal &&
        (receiptType == salesReceiptType::SRT_DAILY ||
         receiptType == salesReceiptType::SRT_NORMAL)){
        _txthdr += _util->cyCenterText(_rcptWidth,"INVOICE");
        _txthdr += _util->cyCenterText(_rcptWidth," ");
    }
    //////////////////////////////////////
    //  Account header
    assembleReceiptAccount(jheader);
    _receipt += _txtacct;
    //////////////////////////////////////
    //  IPAD???
    string is_pos = jsettings_register["is_pos"].asString();
    if (is_pos == "0")  {
        //////////////////////////////////
        //  Customer buzzer information
        if (jheader["hdr_lanebust"] != Json::nullValue)  {
            Json::Value jhdr_lanebust = jheader["hdr_lanebust"];

            tmp = "Order         : ";
            tmp += jhdr_lanebust["is_dinein"].asString() == "1" ? "DINE IN" : "TAKEOUT";
            tmp += "\n";
            _txthdr += tmp;

            tmp = "Buzzer number : ";
            tmp += jhdr_lanebust["buzzer_number"].asString();
            tmp += "\n";
            _txthdr += tmp;

            tmp = "Customer      : ";
            tmp += jhdr_lanebust["customer_name"].asString();
            tmp += "\n";
            _txthdr += tmp;

        }
    }
    //////////////////////////////////////
    //  Header label(s)
    char sztmp[64];
    long lRegister = atol (jsettings_register["register_num"].asString().c_str ());
    long lStoreLocation = atol (jsettings_cashier_shift["location_code"].asString().c_str ());
    sprintf (sztmp, "%02ld-%03ld", lStoreLocation,lRegister);

    if (receiptType == salesReceiptType::SRT_REFUND) {
        //////////////////////////////////
        //  Refund transaction no
        tmp = "Trx No        : ";
        long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
        sprintf (sztmp, "%012ld", ltrx);
        _txthdr += sztmp;
        _txthdr += "\n";
    }  else if (_isInternal)  {
        //////////////////////////////////
        //  Gift card reference number
        string nst = _util->date_sequence();
        tmp  = "Reference     : ";
        tmp += nst; tmp += "\n";
        _txthdr += tmp;
    }  else  {
        if (receiptType != salesReceiptType::SRT_SUSPEND &&
            receiptType == salesReceiptType::SRT_RESUME) {

            tmp = jheader["transaction_number"].asString();
            long ltrxnum = atol (tmp.c_str ());
            sprintf (sztmp, "%012ld", ltrxnum);

            tmp = "Invoice Number: ";
            _txthdr += _util->cyLRText(left, tmp.c_str(), right, sztmp);
        }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {

            tmp = "Trx No        : ";
            long ltrx = atol(jheader["picture_url"].asString ().c_str ());
            sprintf (sztmp, "%012ld", ltrx);
            tmp += sztmp;

            _txthdr += tmp;
            _txthdr += "\n";


            tmp  = "Orig-Invoice# : ";
            ltrx = atol(jheader["transaction_number"].asString ().c_str ());
            sprintf (sztmp, "%012ld", ltrx);
            tmp += sztmp;

            _txthdr += tmp;
            _txthdr += "\n\n";


        }  else if (receiptType == salesReceiptType::SRT_RESUME) {
            tmp  = "Suspend #     : ";
            tmp2 = jheader["transaction_number"].asString ();
            _txthdr += _util->cyLRText(left, tmp, right, tmp2);
        }  else if (receiptType == salesReceiptType::SRT_CANCEL) {
            tmp = jheader["transaction_number"].asString();
            long ltrxnum = atol (tmp.c_str ());
            sprintf (sztmp, "%012ld", ltrxnum);

            tmp2 = "Trx Number    : ";
            tmp2 += sztmp;
            tmp2 += "\n";
            _txthdr += tmp2;
        }  else if (receiptType != salesReceiptType::SRT_SUSPEND) {
            tmp2 = "Invoice Number: ";
            long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
            sprintf (sztmp, "%012ld", ltrx);
            tmp2 += sztmp;

            _txthdr += tmp2.c_str();
            _txthdr += "\n";
        }
    }
    //////////////////////////////////////
    //  Special receipt header(s)
    if (receiptType == salesReceiptType::SRT_REPRINT) {
        _txttype = "reprint";
        _txthdr += _util->cyCenterText(_rcptWidth, "* * *  R E P R I N T * * *");
    }
    else if (receiptType == salesReceiptType::SRT_POSTVOID) {
        _txthdr += _util->cyCenterText(_rcptWidth, "* * *  V O I D * * *");
        _txthdr += _util->cyCenterText(_rcptWidth, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_SUSPEND) {
        _txthdr += _util->cyCenterText(_rcptWidth, "* * *  S U S P E N D  * * *");
    }
    else if (receiptType == salesReceiptType::SRT_CANCEL) {
        _txthdr += _util->cyCenterText(_rcptWidth, "* * *  C A N C E L  * * *");
        _txthdr += _util->cyCenterText(_rcptWidth, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_REFUND) {
        _txthdr += _util->cyCenterText(_rcptWidth, "* * *  R E T U R N S  * * *");
        _txthdr += _util->cyCenterText(_rcptWidth, jheader["remarks"].asString());
    }
    //////////////////////////////////////
    //  FSP account
    assembleFspAccount(jheader);
    _txthdr += _txtacct;
    //////////////////////////////////////
    //  Transaction type
    Json::Value jsettings_transtype;
    _txthdr += _util->cyCenterText(_rcptWidth, " ");
    jsettings_transtype = jheader["settings_transtype"];
    _txthdr += _util->cyCenterText(_rcptWidth, jsettings_transtype["description"].asString());

    _txthdr += _util->cyAcrossText(_rcptWidth, "=");
    _receipt += _txthdr;

    return true;
}
