//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleFspAccount
// DESCRIPTION:     FSP detail(s)
//*******************************************************************
bool CYReceiptJoel::assembleFspAccount (Json::Value& jheader)
{
    string skinny = cgiform("receipt_skinny");
    string rcpt = _ini->get_value("RECEIPT","TYPE");
    int w = 40;
    if (skinny == "true")  {
        w = 30;
    }
    //////////////////////////////////////
    //  FSP
    string sql, m1, m2, m3, m4, m5, m6;
    Json::Value jhdr_account, jhdr_account_line;
    if (jheader["hdr_account"] == Json::nullValue)  {
    }  else  {
        jhdr_account = jheader["hdr_account"];
        int j = jhdr_account.size();
        if (j > 0)
            _txtacct += "\n\n";
        for (int i = 0; i < j; i++)  {
            jhdr_account_line = jhdr_account[i];
            if (jhdr_account_line["account_type_code"].asString() == "FSP" ||
                jhdr_account_line["account_type_code"].asString() == "FSPP") {
                if (skinny == "true")  {
                    _txtacct += "Account type: ";
                }  else  {
                    _txtacct += "Account type  : ";
                }
                    _txtacct += jhdr_account_line["account_type_desc"].asString();
                    _txtacct += "\n";

                    //_txtacct += "Customer name : xxxxxxxx xxxxxxxx";
                    //_txtacct += jhdr_trxaccount["last_name"].asString();
                    //_txtacct += ", ";
                    //_txtacct += jhdr_trxaccount["first_name"].asString();
                    //_txtacct += "\n";
                    //////////////////////
                    //  Points / Epurse
                    double fsp_points=0, fsp_epurse=0;
                    string fsptmp = jhdr_account_line["points_balance"].asString();
                    if (fsptmp.empty())
                        fsp_points = 0.00;
                    fsptmp = jhdr_account_line["epurse_balance"].asString();
                    if (fsptmp.empty())
                        fsp_epurse = 0.00;

                    if (fsp_epurse == 0 && fsp_points == 0)  {
                        CYDbSql* db = _ini->dbconn ();
                        if (nullptr == db)
                            return seterrormsg(_ini->errormsg());
                        db->sql_bind_reset ();
                        sql  = "select * from tg_card_type where acct_type_code = 'FSP' ";
                        sql += "or acct_type_code = 'FSPP' ";

                        if (!db->sql_result(sql,false))
                            return seterrormsg(db->errordb());
                        if (db->eof())  {
                            m1 = "You may have earned FSP loyalty points";
                            m2 = "A notification of FSP points awarded";
                            m3 = "will be sent to your FSP app shortly";
                            m4 = "\n\nYou can download the Rustans";
                            m5 = "FSP mobile app in Apple`s App Store";
                            m6 = "or the Google Play Store";
                        } else {
                            m1 = db->sql_field_value ("offline_msg1");
                            m2 = db->sql_field_value ("offline_msg2");
                            m3 = db->sql_field_value ("offline_msg3");
                            m4 = db->sql_field_value ("offline_msg4");
                            m5 = db->sql_field_value ("offline_msg5");
                            m6 = db->sql_field_value ("offline_msg6");
                        }
                        if (skinny == "true")  {
                            _txtacct += "Card number : ";
                            _txtacct += jhdr_account_line["account_number"].asString();
                            _txtacct += "\n";
                            _txtacct += "\n";
                            _txtacct += "Cur Points  : offline";
                            _txtacct += "\n";
                            _txtacct += "Cur ePurse  : offline";
                        }  else  {
                            _txtacct += "Card number   : ";
                            _txtacct += jhdr_account_line["account_number"].asString();
                            _txtacct += "\n";
                            _txtacct += "\n";
                            if (rcpt == "joel")  {
                                if (!m1.empty()) _txtacct += _util->cyCenterText(w, m1);
                                if (!m2.empty()) _txtacct += _util->cyCenterText(w, m2);
                                if (!m3.empty()) _txtacct += _util->cyCenterText(w, m3);
                                if (!m4.empty()) _txtacct += _util->cyCenterText(w, m4);
                                if (!m5.empty()) _txtacct += _util->cyCenterText(w, m5);
                                if (!m6.empty()) _txtacct += _util->cyCenterText(w, m6);
                                return true;
                                //_txtacct += "Current Available ePurse: ";
                            }  else  {
                                _txtacct += "Current Available Points: ";
                                _txtacct += _util->fmt_number_comma(jhdr_account_line["points_balance"].asString().c_str());
                                _txtacct += "\n";
                                _txtacct += "Current Available ePurse: ";
                            }
                        }
                    }  else  {
                        if (skinny == "true")  {
                            _txtacct += "Card number : ";
                            _txtacct += jhdr_account_line["account_number"].asString();
                            _txtacct += "\n";
                            _txtacct += "\n";
                            _txtacct += "Cur Points  : ";
                            _txtacct += _util->fmt_number_comma(jhdr_account_line["points_balance"].asString().c_str());
                            _txtacct += "\n";
                            _txtacct += "Cur ePurse  : ";
                        }  else  {
                            _txtacct += "Card number   : ";
                            _txtacct += jhdr_account_line["account_number"].asString();
                            _txtacct += "\n";
                            _txtacct += "\n";
                            if (rcpt == "joel")  {
                                if (!m1.empty()) _txtacct += _util->cyCenterText(w, m1);
                                if (!m2.empty()) _txtacct += _util->cyCenterText(w, m2);
                                if (!m3.empty()) _txtacct += _util->cyCenterText(w, m3);
                                if (!m4.empty()) _txtacct += _util->cyCenterText(w, m4);
                                if (!m5.empty()) _txtacct += _util->cyCenterText(w, m5);
                                if (!m6.empty()) _txtacct += _util->cyCenterText(w, m6);
                                return true;
                                //_txtacct += "Current Available ePurse: ";
                            }  else  {
                                _txtacct += "Current Available Points: ";
                                _txtacct += _util->fmt_number_comma(jhdr_account_line["points_balance"].asString().c_str());
                                _txtacct += "\n";
                                _txtacct += "Current Available ePurse: ";
                            }
                        }

                        string totalEPoints = _util->fmt_number_comma(jhdr_account_line["epurse_balance"].asString().c_str());
                        char sztmp [128];
                        memset (sztmp, 0x00, 128);
                        strcpy (sztmp, totalEPoints.c_str());
                        size_t y = strlen(sztmp);
                        for (size_t x = 0; x < y; x++)  {
                            if (sztmp[x]=='.') sztmp[x]='\0';
                        }
                        _txtacct += sztmp;
                    }
                    _txtacct += "\n";
            }
        }
    }
    return true;
}

//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     No account portion
//*******************************************************************
bool CYReceiptJoel::assembleReceiptAccount (Json::Value& jheader)
{
    _txtacct = "";
    //////////////////////////////////////
    //  Receipt width
    string skinny = cgiform("receipt_skinny");
    //////////////////////////////////////
    //  Account header
    string tmp;
    Json::Value jhdr_trxaccount, jhdr_trxaccounttype;
    if (jheader["hdr_trxaccount"] == Json::nullValue)  {
        jhdr_trxaccount = Json::nullValue;
    }  else  {
        jhdr_trxaccount = jheader["hdr_trxaccount"];
        string account_type  = "settings_account_";
               account_type += jhdr_trxaccount["account_type_code"].asString();
        jhdr_trxaccounttype = jheader[account_type];

        if (jhdr_trxaccounttype["is_pwd"].asString() == "1" ||
            jhdr_trxaccounttype["is_senior"].asString() == "1" ||
            jhdr_trxaccounttype["is_zero_rated"].asString() == "1")  {

            _txtacct += "Customer : ";
            tmp  = jhdr_trxaccount["first_name"].asString();
            tmp += " ";
            tmp += jhdr_trxaccount["last_name"].asString();
            tmp += "\n";
            _txtacct += tmp;

            _txtacct += "Account  : ";
            tmp  = jhdr_trxaccount["account_type_desc"].asString();
            tmp += "\n";
            _txtacct += tmp;

            _txtacct += "OSCA/PWD/PNSTM/SPIC ID No\n";
            _txtacct += jhdr_trxaccount["account_number"].asString();
            _txtacct += "\n";

            _txtacct += "SIGNATURE\n";
            _txtacct += _util->cyAcrossText(_rcptWidth,"_");

        }  else  {
            tmp = "Account Type  : ";
            tmp += jhdr_trxaccount["account_type_desc"].asString();
            tmp += "\n";
            _txtacct += tmp;

            tmp = "Account#      : ";
            tmp += jhdr_trxaccount["account_number"].asString();
            tmp += "\n";
            _txtacct += tmp;

            tmp = "Name          : ";
            if (jhdr_trxaccount["first_name"].asString().length() > 0)  {
                tmp += jhdr_trxaccount["first_name"].asString();
                tmp += " ";
            }
            tmp += jhdr_trxaccount["last_name"].asString();
            tmp += "\n";
            _txtacct += tmp;
        }
    }
    return true;
}
