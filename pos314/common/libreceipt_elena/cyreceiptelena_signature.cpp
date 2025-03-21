//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptElena::assembleReceiptSignature (Json::Value& jheader)
{
    _txtsig = "";
    size_t  w = g_widthReceipt;
    if (_isSkinny)   {
        w = 30;
    }


    Json::Value jacct,jtemp,jtemp2;
    string tmp3="",tmp2="",newLine = "\n";
    jacct = jheader["hdr_tax_receipt"];
    jtemp = jheader["hdr_trxaccount"];

    jtemp2 = jheader["hdr_trxaccount"];
    tmp3 = jtemp2["first_name"].asString();
    tmp3 += " ";
    tmp3 += jtemp2["last_name"].asString();

    if (jtemp2["first_name"].asString().length() < 1 && jtemp2["last_name"].asString().length() < 1){
        _txtsig += newLine.c_str();
        if (_isSkinny)  {
            _txtsig += "Customer Name:   _____________\n";
            _txtsig += "Address:         _____________\n";
            _txtsig += "TIN:             _____________\n";
            _txtsig += "Business Style:  _____________\n";
        }  else  {
            _txtsig += "Customer Name:   _______________________\n";
            _txtsig += "Address:         _______________________\n";
            _txtsig += "TIN:             _______________________\n";
            _txtsig += "Business Style:  _______________________\n";
        }
        _txtsig += newLine.c_str();
    } else {
        _txtsig += newLine.c_str();
        tmp2 = "Customer Name:   ";
        tmp2 += newLine.c_str();
        _txtsig += tmp2;

        tmp2  = jtemp2["first_name"].asString();
        tmp2 += " ";
        tmp2 += jtemp2["last_name"].asString();

        if (tmp2.length() >= (w-1))  {
            tmp3 = tmp2.substr(0, (w-1));
        }
        _txtsig += tmp3;
        _txtsig += "\n";
        /*
        if (tmp3.length()+17 <= w){
            tmp2 = "Customer Name:   ";
            tmp2 += tmp3.c_str();
            tmp2 += newLine.c_str();

            _txtsig += tmp2.c_str();
        } else {
            tmp2 = "Customer Name:   ";
            tmp2 += newLine.c_str();

            tmp2 += jtemp2["first_name"].asString();
            //tmp2 += newLine.c_str();
            tmp2 += jtemp2["last_name"].asString();
            tmp2 += newLine.c_str();

            if (tmp2.length() >= (w-1))
                tmp2[w-1]='\0';

            _txtsig += tmp2.c_str();
        }*/

        tmp3 = ""; tmp2 = "";
        jtemp2 = jheader["hdr_tax_receipt"];
        tmp3 = jtemp2["address1"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["address2"].asString();

        if (jtemp2["address1"].asString().length() < 1 && jtemp2["address2"].asString().length() < 1){            
            _txtsig += "Address: _____________________";
            if (false == _isSkinny) _txtsig += "__________";
            _txtsig += newLine.c_str();
        } else if (tmp3.length() + 9 <= w){
            _txtsig += "Address: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Address: ";
            _txtsig += jtemp2["address1"].asString();
            _txtsig += newLine.c_str();
            _txtsig += jtemp2["address2"].asString();
            _txtsig += newLine.c_str();
        }

        tmp3 = jtemp2["tin"].asString();
        if (tmp3.length() < 1) {
            _txtsig += "TIN : ________________________";
            if (false == _isSkinny) _txtsig += "__________";
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "TIN: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        }

        tmp3 = jtemp2["business_style"].asString();

        if (tmp3.length() < 1) {
            _txtsig += "Business Style:_______________";
            if (false == _isSkinny) _txtsig += "__________";
            _txtsig += newLine.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Business Style: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
            _txtsig += newLine.c_str();
        }
    }
    _txtsig += _util->cyAcrossText(w, "-");


    jtemp2 = jheader["header_transtype_discount"];

    if (jtemp["is_senior"].asString() == "1" || jtemp2["discount_code"].asString() == "SENIOR") {

        jtemp2 = jheader["hdr_trxaccount"];
        tmp3 = jtemp2["first_name"].asString();
        tmp3 += " ";
        tmp3 += jtemp2["last_name"].asString();

        _txtsig += _util->cyCenterText(w,"DISCOUNT CLAIM");
        _txtsig += newLine.c_str();
        _txtsig += "Signature: _____________________________\n";
        _txtsig += "Discount: ";

        jtemp2 = jheader["header_transtype_discount"];
        _txtsig += jtemp2["description"].asString();
        _txtsig += newLine.c_str();

        if (tmp3.length()+6 <= w){
            _txtsig += "Name: ";
            _txtsig += tmp3.c_str();
            _txtsig += newLine.c_str();
        } else {
            _txtsig += "Name: ";
            _txtsig += jtemp2["first_name"].asString();
            _txtsig += newLine.c_str();
            _txtsig += jtemp2["last_name"].asString();
            _txtsig += newLine.c_str();
        }
        _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n";
    }

    _receipt += _txtsig;
    return true;
}
