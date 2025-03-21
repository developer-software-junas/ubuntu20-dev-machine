//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrico.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleFspAccount
// DESCRIPTION:     FSP detail(s)
//*******************************************************************
bool CYReceiptRico::assembleFspAccount (Json::Value& jheader)
{
    _txtacct = "";
    string skinny = cgiform("receipt_skinny");
    //////////////////////////////////////
    //  FSP
    Json::Value jhdr_account, jhdr_account_line;
    if (jheader["hdr_account"] == Json::nullValue)  {
    }  else  {
        jhdr_account = jheader["hdr_account"];
        int j = jhdr_account.size();
        for (int i = 0; i < j; i++)  {
            jhdr_account_line = jhdr_account[i];
            if (jhdr_account_line["account_type_code"].asString() == "FSP" ||
                jhdr_account_line["account_type_code"].asString() == "FSPP") {
                    _txtacct += "Account type: ";
                    _txtacct += jhdr_account_line["account_type_desc"].asString();
                    _txtacct += "\n";

                    //_txtacct += "Customer name : xxxxxxxx xxxxxxxx";
                    //_txtacct += jhdr_trxaccount["last_name"].asString();
                    //_txtacct += ", ";
                    //_txtacct += jhdr_trxaccount["first_name"].asString();
                    //_txtacct += "\n";

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
                        _txtacct += "Current Available Points: ";
                        _txtacct += _util->fmt_number_comma(jhdr_account_line["points_balance"].asString().c_str());
                        _txtacct += "\n";
                        _txtacct += "Current Available ePurse: ";
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
bool CYReceiptRico::assembleReceiptAccount (Json::Value& jheader)
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
            tmp = "Account Type : ";
            tmp += jhdr_trxaccount["account_type_desc"].asString();
            tmp += "\n";
            _txtacct += tmp;

            tmp = "Account#     : ";
            tmp += jhdr_trxaccount["account_number"].asString();
            tmp += "\n";
            _txtacct += tmp;

            tmp = "Name         : ";
            if (jhdr_trxaccount["first_name"].asString().length() > 0)  {
                tmp += jhdr_trxaccount["first_name"].asString();
                tmp += " ";
            }
            tmp += jhdr_trxaccount["last_name"].asString();
            tmp += "\n";
            _txtacct += tmp;
        }
    }
    _txtacct += "\n";
    return true;
}
