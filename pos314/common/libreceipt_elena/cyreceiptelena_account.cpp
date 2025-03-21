//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptElena::assembleReceiptAccount (Json::Value& jheader)
{
    _txtacct = "";
    string info, label;
    bool isFsp = false;
    bool isBeauty = false;
    Json::Value jacct, jaccount;

    int w = g_widthReceipt;
    if (_isSkinny)   {
        w = 30;
    }

    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    int j = jaccount.size();
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_loyalty"].asString() == "1") {
            isFsp = true;
            label = "FSP Details";
        } else if (jacct["is_shopper"].asString() == "1") {
            label = "Personal shopper";
        } else if (jacct["is_clerk"].asString() == "1") {
            label = "Sales clerk";
        } else if (jacct["is_zero_rated"].asString() == "1") {
            label = "VAT exempt";
        } else if (jacct["is_beauty_addict"].asString() == "1") {
            isBeauty = true;
            label = "Beauty Card Details";
        } else if (jacct["is_agent"].asString() == "1") {
            label = "CRM account";
        } else {
            label = jacct["account_type_desc"].asString();
        }

        if (isFsp || isBeauty) {
            _txtacct += _util->cyCenterText(w, " ");
            _txtacct += _util->cyCenterText(w, label);

            info  = "Account Number: ";
            info += jacct["account_number"].asString().substr(0,22);
            _txtacct += _util->cyCenterText(w, info);

            info = "Name: ";
            info += jacct["first_name"].asString();
            info += " ";
            info += jacct["last_name"].asString();
            _txtacct += _util->cyCenterText(w, info);
            /***TODO  ADD POINTS HERE ***/
        }  else  {
            _txtacct += _util->cyCenterText(w, " ");
            _txtacct += _util->cyCenterText(w, label);

            info  = "Account Number: ";
            info += jacct["account_number"].asString().substr(0,22);
            _txtacct += _util->cyCenterText(w, info);

            info = "Name: ";
            info += jacct["first_name"].asString();
            info += " ";
            info += jacct["last_name"].asString();
            _txtacct += _util->cyCenterText(w, info);
        }
    }
    _receipt += _txtacct;
    return true;
}
