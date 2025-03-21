//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptElena::assembleReceiptHeader (Json::Value& jheader,
                                                salesReceiptType receiptType)
{
    _txthdr = "";
    _receipt = "";
    _receiptType = receiptType;
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_widthReceipt;
    int wleft = 23, wright = 18;
    if (_isSkinny)   {
        w = 30;
        wleft = 19;
        wright = 12;
    }
    string rcpt = _ini->get_value("RECEIPT","TYPE");

    string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
    Json::Value jtemp2, jtemp = jheader["hdr_branch"];
    if (jtemp["hdrmsg1"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jtemp["hdrmsg1"].asString());
    }
    if (jtemp["hdrmsg2"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jtemp["hdrmsg2"].asString());
    }
    if (jtemp["hdrmsg3"].asString().length() > 0)  {
        _txthdr += _util->cyCenterText(w, jtemp["hdrmsg3"].asString());
    }
    /*
    _txthdr += _util->cyCenterText(w, " ");
    jtemp = jheader["hdr_settings"];
    _txthdr += _util->cyCenterText(w, jtemp["trade_name"].asString());
    */

    jtemp = jheader["hdr_company"];
    _txthdr += _util->cyCenterText(w, jtemp["description"].asString());
    jtemp = jheader["hdr_branch"];

    // full address
    fullAddress  = jtemp["addr1"].asString();
    if (!jtemp["addr2"].isNull())  {
        fullAddress += ", ";
        fullAddress += jtemp["addr2"].asString();
    }
    fullAddress = fullAddress.substr(0, (w-1));
    _txthdr += _util->cyCenterText(w, fullAddress.c_str());
    jtemp = jheader["hdr_branch"];
    tmp  = "VAT-REG-TIN-";
    tmp += jtemp["tin"].asString();
    _txthdr += _util->cyCenterText(w, tmp);

    jtemp = jheader["settings_register"];
    tmp = jtemp["serialno"].asString();
    if (tmp.length() > w + 1) {
        tmp2 = tmp.substr(0, (w - 1));
    } else {
        tmp2 = tmp;
    }
    _txthdr += _util->cyCenterText(w, tmp2);

    jtemp = jheader["hdr_branch"];
    //CHECK IF CHARGE PAYMENT
    Json::Value jtmp, jpay,jpayment;
    jpayment = jheader["pay_sales"];
    size_t j = jpayment.size();
    bool isCharge = false;
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];

        if (jpay["tendertype"] == Json::nullValue)
            isCharge=false;
        jtmp = jpay["tendertype"];
        isCharge = jtmp["is_charge"].asString() == "1";
        if (isCharge)
            break;
    }
    //////////////////////////////////////
    //  Branch location?
    string tablename;    
    /***
    if (jheader.isMember("hdr_fnb"))  {
        jtemp = jheader["hdr_fnb"];
        tmp = jtemp["sectiondesc"].asString();
        _txthdr += "\n";
        _txthdr += _util->cyCenterText(w, tmp);
        _txthdr += "\n";
        tablename = jtemp["tabledesc"].asString ();
    }  else  {
        jtemp = jheader["hdr_branch"];
        tmp = jtemp["sd_branch_desc"].asString();
        _txthdr += "\n";
        _txthdr += _util->cyCenterText(w, tmp);
        _txthdr += "\n";
    }
    ***/
    //////////////////////////////////////
    //  Special receipt header(s)
    _txthdr += "\n";
    if (receiptType == salesReceiptType::SRT_REPRINT) {
        _txttype = "reprint";
        _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
    }
    else if (receiptType == salesReceiptType::SRT_POSTVOID) {
        _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_SUSPEND) {
        if (rcpt == "elena" || rcpt == "sunnies")
            _txthdr += _util->cyCenterText(w, "* * *  R U N N I N G  B I L L  * * *");
        else
            _txthdr += _util->cyCenterText(w,      "* * *  S U S P E N D  * * *");
    }
    else if (receiptType == salesReceiptType::SRT_CANCEL) {
        _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
    }
    else if (receiptType == salesReceiptType::SRT_REFUND) {
        _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
        _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
    }
    _txthdr += "\n";

    char sztmp[24];
    jtemp = jheader["settings_register"];
    long lRegister = atol (jtemp["register_num"].asString().c_str ());
    sprintf (sztmp, "%03ld", lRegister);

    if (receiptType == salesReceiptType::SRT_REFUND) {
        tmp2 = "CHIT number:  ";
        tmp3 = refundTransnum(jheader).c_str();
        _txthdr += _util->cyLRText(wleft,tmp2.c_str(),wright,tmp3.c_str());

    }  else  {
        if (receiptType != salesReceiptType::SRT_SUSPEND &&
            receiptType == salesReceiptType::SRT_RESUME) {
            tmp2 = "CHIT number: ";
            tmp3 = legacyTransnum(jheader).c_str();
            _txthdr += _util->cyLRText(wleft,tmp2.c_str(),wright,tmp3.c_str());
        }  else if (receiptType == salesReceiptType::SRT_SUSPEND) {

            tmp2 = "Suspend #  : ";
            tmp2 += sztmp;
            tmp2 += "-";
            tmp2 += jheader["transaction_number"].asString ();
            tmp2 += "\n";
            _txthdr += tmp2;//_util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
        }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {
            tmp2 = "Reference# : ";
            tmp2 += sztmp;
            tmp2 += "-";

            tmp3 = jheader["transaction_number"].asString ();
            long lTrans = atol(tmp3.c_str ());
            sprintf (sztmp, "%05ld\n", lTrans);
            tmp2 += sztmp;


            _txthdr += tmp2;
        }  else  {
            tmp2 = "CHIT Number: ";
            tmp2 += sztmp;
            tmp2 += "-";
            //tmp3 = _suspendNum;
            long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
            sprintf (sztmp, "%05ld\n", ltrx);
            tmp2 += sztmp;
            _txthdr += tmp2;
        }
    }
    tmp2  = "Date-Time  : ";
    tmp2 += _util->date_nice(jheader["transaction_date"].asString ().c_str ());//_util->date_pos_elena();
    tmp2 += "-";
    tmp2 += _util->time_colon(jheader["transaction_time"].asString ().c_str ());//_util->date_pos_elena();
    _txthdr += tmp2.c_str();
    _txthdr += newLine;

    //////////////////////////////////////
    //  MEMBER INFORMATION
    jtemp2 = jheader["hdr_trxaccount"];
    if (jtemp2.isMember("first_name"))  {
        isCharge = jtemp2["account_type_code"].asString()=="MEMBER";

        tmp2 = isCharge ? "Member ID  : " : "Account#   : ";
        tmp3 = jtemp2["account_number"].asString();
        _txthdr += tmp2.c_str();
        _txthdr += tmp3.c_str();
        _txthdr += newLine.c_str();

        tmp2 = isCharge ? "Member Name: " : "Name       : ";
        tmp3 = jtemp2["first_name"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["last_name"].asString();

        _txthdr += tmp2.c_str();
        _txthdr += '\n';
        if (tmp3.length() >= (w-1))  {
            tmp3[(w)]='\0';
        }

        _txthdr += tmp3.c_str();
        _txthdr += '\n';
    }
    //////////////////////////////////////
    //CASHIER/TABLE/SERVER INFORMATION
    Json::Value jcashier;
    if (jheader["settings_cashier"] == Json::nullValue)
        return seterrormsg("Unable to get the cashier details");

    jcashier = jheader["settings_cashier"];
    size_t len = jcashier["first_name"].asString().length();
    len += jcashier["last_name"].asString().length();
    len += 11;

    if (len > g_widthReceipt)  {
        tmp3  = "Cashier    : ";
        tmp3 += jcashier["first_name"].asString();
        tmp3 += " ";
        tmp3 += jcashier["last_name"].asString();
        tmp3 += "\n";
        _txthdr += tmp3;
/*
        tmp2 = "Table      : ";
        if (jtemp["tabledesc"].asString().length() > 5)
            tmp3 = jtemp["tabledesc"].asString();
        else
            tmp3 = "";
        _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
*/

    }  else  {
        tmp2  = "Cashier    : ";
        tmp2 += jcashier["first_name"].asString();
        tmp2 += " ";
        tmp2 += jcashier["last_name"].asString();
        tmp2 += "\n";
        _txthdr += tmp2;
/*
        tmp3  = "Table      : ";
        len = jtemp["tabledesc"].asString().length();
        if (len > 5)
            tmp3 += jtemp["tabledesc"].asString().substr(6,len-6);
        else
            tmp3 += "";
        _txthdr += _util->cyLRText(29,tmp2.c_str(),12,tmp3.c_str());
*/
    }

    jtemp = jheader["hdr_fnb"];
    tmp2   = "Server     : ";
    tmp2  += jtemp["first_name"].asString();
    tmp2  +=  " ";
    tmp2  += jtemp["last_name"].asString();
    tmp2 += "\n";
    _txthdr += tmp2.c_str();

    tablename = jtemp["tabledesc"].asString();
    tmp2   = "Table      : ";
    tmp2  += tablename;
    tmp2 += "\n";
    _txthdr += tmp2.c_str();

    if (receiptType == SRT_REFUND)  {
        _txthdr += newLine.c_str();
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

    if (_isSkinny)
        _txthdr += RCPT_REPRINT_SKINNY;
    else
        _txthdr += RCPT_REPRINT;
    _txthdr += "\n";
    _receipt += _txthdr;
    return true;
}
