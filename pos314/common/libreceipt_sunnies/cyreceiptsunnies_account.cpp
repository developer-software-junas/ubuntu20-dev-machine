//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunnies.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptAccount
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptSunnies::assembleReceiptAccount (Json::Value& jheader,
                                            salesReceiptType receiptType)
{
    _txtacct = "";
    (void)jheader;
    (void)receiptType;
    /*
    string info, label,tmp;
    string today;
    _util->int_date(today,0);
    Json::Value jacct, jaccount,jcashier;
    if (jheader["hdr_account"] == Json::nullValue)
        return true;
    jaccount = jheader["hdr_account"];
    jcashier = jheader["settings_cashier"];

    int j = jaccount.size();
    for (int i = 0; i < j; i++)  {
        jacct = jaccount[i];
        if (jacct["is_shopper"].asString() == "1") {
            label = "PShopper ID";
        } else if (jacct["is_zero_rated"].asString() == "1") {
            label = "VAT-Exempt";
        } else if (jacct["is_agent"].asString() == "1") {
            label = "CRM account";
        } else {
            if (jacct["is_clerk"].asString() != "1")  {
                _txtacct += "\n";
                label  = "*** ";
                label += jacct["account_type_desc"].asString();
                label += " ***";
                _txtacct += _util->cyCenterText(g_widthReceipt, label);
            }
        }


        if (strlen(label.c_str()) > 0){
            label  = jacct["last_name"].asString();
            label +=  ", ";
            label +=  jacct["first_name"].asString();
            _txtacct += _util->cyCenterText(g_widthReceipt, label);

            label  =  "CARD: ";
            label +=  jacct["account_number"].asString();
            _txtacct += _util->cyCenterText(g_widthReceipt, label);
            _txtacct += "\n";
        }

        label = "";

    }

    _receipt += _txtacct;*/
    return true;
}
