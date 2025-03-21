//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptElena::assembleReceiptFooter (Json::Value& jheader,
                                                               salesReceiptType receiptType)
{
    _txtfoot = "";
    string tmp = "";
    if (jheader["hdr_settings"]==Json::nullValue)
        return seterrormsg("Unable to retrieve the POS settings information");

    Json::Value jpos;
    size_t  w = g_widthReceipt;
    if (_isSkinny)   {
        w = 30;
    }
    jpos = jheader["hdr_settings"];

        Json::Value jtemp;

        jtemp = jheader["hdr_company"];

        //HARDCODED VALUE - NO DETAILS IN API
        _txtfoot += _util->cyCenterText(w, " ");
        _txtfoot += _util->cyCenterText(w, "CYWARE INCORPORATED");
        _txtfoot += _util->cyCenterText(w, "6057 R. Palma Street Poblacion");
        _txtfoot += _util->cyCenterText(w, "Makati, 1210 Metro Manila");
        _txtfoot += _util->cyCenterText(w, "VAT REGTIN:201-140-102-00000");
        _txtfoot += _util->cyCenterText(w, "ACCR.# 049-201140102-0000462151");
        _txtfoot += _util->cyCenterText(w, "EFFECTIVITY DATE 08/01/2020");
        _txtfoot += _util->cyCenterText(w, "VALID UNTIL 07/31/2025");
        _txtfoot += _util->cyCenterText(w, "Date Issued: 05/11/2005");


    if (receiptType == SRT_REFUND)  {
        _txtfoot += "Customer Name :________________________\n";
        _txtfoot += "Address :______________________________\n";
        _txtfoot += "Phone No. :____________________________\n\n";
        _txtfoot += "Approving Manager :____________________\n\n\n\n";

        _txtfoot += "     Return and Exchange Slip (RES)\n";
        _txtfoot += "----------------------------------------\n";
        _txtfoot += "Please present RES upon exchange of item\n";
        _txtfoot += "         For one-time use only\n";
        _txtfoot += "       Not replaceable when lost\n";
        _txtfoot += "Can be used in Rustan`s Department Store\n";
        _txtfoot += "                  only\n";
    }


    //////////////////////////////////////
    //  Footer
    //FOR SAMPLE ONLY INSERT QR CODE HERE
    /*
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    _txtfoot += _util->cyCenterText(w, " ");
    */
    // END OF QR CODE AREA

    if (jpos["szmsg01"].asString().length() > 0)  {
        _txtfoot +=  _util->cyCenterText(w, jpos["szmsg01"].asString());
    }
    if (jpos["szmsg02"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg02"].asString());
    }
    if (jpos["szmsg03"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg03"].asString());
    }
    if (jpos["szmsg04"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, jpos["szmsg04"].asString());
    }

    if (jpos["szmsg05"].asString().length() > 0)  {
        _txtfoot += _util->cyCenterText(w, " ");
        _txtfoot += _util->cyCenterText(w, jpos["szmsg05"].asString());
    }


    _txtfoot += _util->cyCenterText(w, " ");
    _receipt += _txtfoot;
    return true;
}
