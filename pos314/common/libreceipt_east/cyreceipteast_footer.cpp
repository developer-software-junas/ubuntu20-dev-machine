//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceipteast.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptEast::assembleReceiptFooter (Json::Value& jheader,
                                                          salesReceiptType receiptType)
{
     _txtfoot = "";
     Json::Value jtemp;
     string tmp = "",label = "";
     if (jheader["hdr_settings"]==Json::nullValue)
         return seterrormsg("Unable to retrieve the POS settings information");

     Json::Value jpos,jcashier,jaccount,jacct;
     int w = g_widthReceipt;
     jpos = jheader["hdr_settings"];

     if (receiptType == salesReceiptType::SRT_REPRINT ||
         receiptType == salesReceiptType::SRT_POSTVOID)  {
         _txtfoot += _util->cyCenterText(w, "THIS DOES NOT SERVE AS AN INVOICE");
         _txtfoot += _util->cyCenterText(w, "THIS DOCUMENT IS NOT VALID");
         _txtfoot += _util->cyCenterText(w, "FOR CLAIM OF INPUT TAX");
         _txtfoot += "\n";
     }


     if (receiptType != salesReceiptType::SRT_NORMAL &&
             receiptType != salesReceiptType::SRT_REPRINT &&
             receiptType != salesReceiptType::SRT_DAILY)  {
     }  else  {
         _txtfoot += _util->cyCenterText(w, "CYWARE INCORPORATED");
         _txtfoot += _util->cyCenterText(w, "6057 Palma Street");
         _txtfoot += _util->cyCenterText(w, "Poblacion NCR Fourth District Makati");

         jtemp = jheader["hdr_branch"];
         tmp  = "VAT-REG-TIN: ";
         tmp += jtemp["tin"].asString();
         _txtfoot += _util->cyCenterText(w, tmp);

         tmp  = "ACCR.# ";
         tmp += jtemp["bir_num"].asString();
         _txtfoot += _util->cyCenterText(w,tmp);

         string permitStart,PermitEnd,mm,dd,yyyy;
         permitStart = jtemp["bir_issue"].asString();
         _util->date_split(permitStart.c_str(),yyyy,mm,dd);

         tmp  = "EFFECTIVITY DATE: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(w,tmp);

         permitStart = jtemp["bir_expire"].asString();
         _util->date_split(permitStart.c_str(),yyyy,mm,dd);

         tmp  = "VALID UNTIL: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(w,tmp);

         tmp  = "Permit No: ";
         tmp  += jtemp["permitno"].asString();
         _txtfoot += _util->cyCenterText(w,tmp);

         //format date mm/dd/yyyy
         jtemp = jheader["settings_register"];
         string permitDate;
         permitDate = jtemp["permit_start"].asString();
         _util->date_split(permitDate.c_str(),yyyy,mm,dd);

         tmp  = "Date Issued: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(w,tmp);
     }

     _txtfoot += _util->cyCenterText(w, " ");
     _receipt += _txtfoot;
     return true;
}
