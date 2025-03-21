//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptJoel::assembleReceiptSignature (Json::Value& jheader)
{   
     _txtsig = "";
     /////////////////////////////////////
     //  Gift card transaction / exit
     if (_isInternal)  {
         _txtsig += "\n";
         _txtsig += _util->cyCenterText(_rcptWidth, "THIS SERVES AS YOUR");
         _txtsig += _util->cyCenterText(_rcptWidth, "ACKNOWLEDGEMENT RECEIPT");
         _txtsig += _util->cyCenterText(_rcptWidth, "\"THIS DOCUMENT IS NOT VALID");
         _txtsig += _util->cyCenterText(_rcptWidth, "FOR CLAIM OF INPUT TAX\"");
         _txtsig += "\n";

         return true;
     }
     //////////////////////////////////////
     //  Receipt width
     int left = 18, right = 22;
     string skinny = cgiform("receipt_skinny");
     if (skinny == "true")  {
         left = 18;
         right = 11;
     }
     //////////////////////////////////////
     //  Official receipt
     string tmp = _util->cyAcrossText(right,"_");
     tmp[tmp.length()-1]='\0';
     if (jheader["hdr_tax_receipt"]==Json::nullValue)  {
         _txtsig += "\n";
         _txtsig += _util->cyCenterText(_rcptWidth, "OFFICIAL INVOICE DETAIL(S)");
         _txtsig += "CUSTOMER NAME\n";
         _txtsig += _util->cyAcrossText(_rcptWidth,"_");
         _txtsig += "ADDRESS\n";
         _txtsig += _util->cyAcrossText(_rcptWidth,"_");
         _txtsig += _util->cyLRText(12,"TIN:", right, tmp);
         _txtsig += "BUSINESS STYLE\n";
         _txtsig += _util->cyAcrossText(_rcptWidth,"_");
         //_txtsig += _util->cyLRText(left,"Business style:", right, tmp);

         _txtsig += "OSCA/PWD/PNSTM/SPIC ID Number\n";
         _txtsig += _util->cyAcrossText(_rcptWidth,"_");

         _txtsig += _util->cyLRText(left,"Signature:", right, tmp);
         _txtsig += "\n";
     }  else  {
         Json::Value jacct;
         jacct = jheader["hdr_tax_receipt"];
         //////////////////////////////////////
         //  OR customer
         if (jacct["customer_name"].asString().length() > 0)  {
             _txtsig += "\n";
             _txtsig += _util->cyCenterText(_rcptWidth, "OFFICIAL INVOICE DETAILS");
         }
         if (jacct["company_name"].asString().length() > 0)  {
             _txtsig += "           Name: ";
             _txtsig += jacct["company_name"].asString();
             _txtsig += "\n";
         }
         if (jacct["address1"].asString().length() > 0)  {
             _txtsig += "       Address1: ";
             _txtsig += jacct["address1"].asString();
             _txtsig += "\n";
         }
         if (jacct["address2"].asString().length() > 0)  {
             _txtsig += "       Address2: ";
             _txtsig += jacct["address2"].asString();
             _txtsig += "\n";
         }
         if (jacct["tin"].asString().length() > 0)  {
             _txtsig += "            TIN: ";
             _txtsig += jacct["tin"].asString();
             _txtsig += "\n";
         }
         if (jacct["customer_name"].asString().length() > 0)  {
             _txtsig += " Business Style: ";
             _txtsig += jacct["customer_name"].asString();
             _txtsig += "\n";
         }
     }

     _txtsig += "\n";
     _receipt += _txtsig;
     return true;
}
