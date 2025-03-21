//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunnies.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptSunnies::assembleReceiptSignature (Json::Value& jheader,
                                                             salesReceiptType receiptType)
{   
     _txtsig = "";
     (void)receiptType;

     if (jheader["hdr_tax_receipt"]==Json::nullValue)  {
         _txtsig += "\n      OFFICIAL INVOICE DETAIL(S)\n";
         _txtsig += "Customer name:   _______________________\n";
         _txtsig += "Address:         _______________________\n";
         _txtsig += "TIN:             _______________________\n";
         _txtsig += "Business style:  _______________________\n";
         _txtsig += "OSCA/PWD/PNSTM/SPIC ID No: _____________\n\n";
         _txtsig += "Signature: _____________________________\n";
         _txtsig += "\n";
         return true;
     }

     Json::Value jacct;
     jacct = jheader["hdr_tax_receipt"];
     //////////////////////////////////////
     //  OR customer
     if (jacct["customer_name"].asString().length() > 0)
         _txtsig += "\n      OFFICIAL INVOICE DETAIL(S)\n";
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
     _txtsig += "\n";
     _receipt += _txtsig;
     return true;
}
