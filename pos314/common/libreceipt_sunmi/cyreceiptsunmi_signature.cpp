//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptSignature
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptSunmi::assembleReceiptSignature (Json::Value& jheader)
{   
     _txtsig = "";
     if (jheader["hdr_tax_receipt"]==Json::nullValue)  {
         _txtsig += "\n OFFICIAL INVOICE DETAIL(S) \n";
         _txtsig += "Cust Name: ___________________\n";
         _txtsig += "Address:   ___________________\n";
         _txtsig += "TIN:       ___________________\n";
         _txtsig += "Bus style: ___________________\n";
         _txtsig += "OSCA/PWD/PNSTM/SPIC ID No:\n";
         _txtsig += "______________________________\n\n";
         _txtsig += "Signature: ___________________\n";
         _txtsig += "\n";
         return true;
     }

     Json::Value jacct;
     jacct = jheader["hdr_tax_receipt"];
     //////////////////////////////////////
     //  OR customer
     if (jacct["customer_name"].asString().length() > 0)
         _txtsig += "\n OFFICIAL INVOICE DETAIL(S) \n";
     if (jacct["company_name"].asString().length() > 0)  {
         _txtsig += "     Name: ";
         _txtsig += jacct["company_name"].asString();
         _txtsig += "\n";
     }
     if (jacct["address1"].asString().length() > 0)  {
         _txtsig += " Address1: ";
         _txtsig += jacct["address1"].asString();
         _txtsig += "\n";
     }
     if (jacct["address2"].asString().length() > 0)  {
         _txtsig += " Address2: ";
         _txtsig += jacct["address2"].asString();
         _txtsig += "\n";
     }
     if (jacct["tin"].asString().length() > 0)  {
         _txtsig += "      TIN: ";
         _txtsig += jacct["tin"].asString();
         _txtsig += "\n";
     }
     /*
     if (jacct["customer_name"].asString().length() > 0)  {
         _txtsig += "Bus Style: ";
         _txtsig += jacct["customer_name"].asString();
         _txtsig += "\n";
     }*/
     _txtsig += "\n";
     _receipt += _txtsig;
     return true;
}
