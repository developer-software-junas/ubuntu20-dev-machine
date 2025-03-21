//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRcc::assembleReceiptAccount (Json::Value& jheader, salesReceiptType receiptType)
{
    _txtacct = "";
    string info, label,label2,tmp;
    string today;
    _util->int_date(today,0);

    Json::Value jacct, jaccount,jcashier;
    if (jheader["hdr_account"] != Json::nullValue)  {
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


            }

            label = "";

        }
    }

    /////////////
    /// \brief ridc_number
    ///
    string ridc_number = jheader["ridc_number"].asString();
    if (strlen(ridc_number.c_str()) > 0) {
        label =  "RIDC #: ";
        label += jheader["ridc_number"].asString();

        _txtacct += label.c_str();
        _txtacct += "\n";
    }



    if (receiptType == salesReceiptType::SRT_POSTVOID || receiptType == salesReceiptType::SRT_SUSPEND){

        info  = "Txn No. : ";
        info += jheader["picture_url"].asString();
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
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //GET IPAD DETAILS
    /*
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
*/
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
    if (receiptType == salesReceiptType::SRT_REFUND){
/*
        _txtacct += "Business Date: ";
        if (jheader.isMember("logical_date"))
            _txtacct += jheader["logical_date"].asString();
        _txtacct += "\n";
*/
        _txtacct += "Orig Pos No  : ";
        if (jheader.isMember("orig_register_number"))
            _txtacct += jheader["orig_register_number"].asString();
        _txtacct += "\n";

        _txtacct += "Orig Bus Date: ";
        if (jheader.isMember("orig_transaction_number"))
            _txtacct += jheader["orig_transaction_date"].asString();
        _txtacct += "\n";

        _txtacct += "Original SI# : ";
        if (jheader.isMember("orig_systransnum"))
            _txtacct += jheader["orig_systransnum"].asString();
        _txtacct += "\n";

    }
    if (receiptType != SRT_SUSPEND)
        _txtacct += _util->cyAcrossText(40,"-");




    _receipt += _txtacct;
    return true;
}
