//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrico.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptFooter
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRico::assembleReceiptFooter (Json::Value& jheader,
                                                          salesReceiptType receiptType)
{
     _txtfoot = "";
     /////////////////////////////////////
     //  Gift card transaction / bypass
     if (_isInternal)  return true;
     /////////////////////////////////////
     //  Not an invoice receipt?
     if (receiptType == salesReceiptType::SRT_REPRINT ||
         receiptType == salesReceiptType::SRT_POSTVOID)  {
         _txtfoot += _util->cyCenterText(_rcptWidth, "THIS DOES NOT SERVE AS AN INVOICE");
         _txtfoot += _util->cyCenterText(_rcptWidth, "THIS DOCUMENT IS NOT VALID");
         _txtfoot += _util->cyCenterText(_rcptWidth, "FOR CLAIM OF INPUT TAX");
         _txtfoot += "\n";
     }
     /////////////////////////////////////
     //  Assemble the footer?
     if (receiptType != salesReceiptType::SRT_NORMAL &&
         receiptType != salesReceiptType::SRT_REPRINT &&
         receiptType != salesReceiptType::SRT_DAILY)  {
         //  DO NOTHING
     }  else  {
         string tmp;
         Json::Value jhdr_branch;
         jhdr_branch = jheader["hdr_branch"];

         _txtfoot += _util->cyCenterText(_rcptWidth, "CYWARE INCORPORATED");
         _txtfoot += _util->cyCenterText(_rcptWidth, "6057 Palma Street");
         _txtfoot += _util->cyCenterText(_rcptWidth, "Poblacion NCR Fourth District Makati");

         tmp  = "VAT-REG-TIN: ";
         tmp += jhdr_branch["tin"].asString();
         _txtfoot += _util->cyCenterText(_rcptWidth, tmp);

         tmp  = "ACCR.# ";
         tmp += jhdr_branch["bir_num"].asString();
         _txtfoot += _util->cyCenterText(_rcptWidth,tmp);

         string permitStart,PermitEnd,mm,dd,yyyy;
         permitStart = jhdr_branch["bir_issue"].asString();
         _util->date_split(permitStart.c_str(),yyyy,mm,dd);

         tmp  = "EFFECTIVITY DATE: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(_rcptWidth,tmp);

         permitStart = jhdr_branch["bir_expire"].asString();
         _util->date_split(permitStart.c_str(),yyyy,mm,dd);

         tmp  = "VALID UNTIL: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(_rcptWidth,tmp);

         tmp  = "Permit No: ";
         tmp  += jhdr_branch["permitno"].asString();
         _txtfoot += _util->cyCenterText(_rcptWidth,tmp);

         //format date mm/dd/yyyy
         jhdr_branch = jheader["settings_register"];
         string permitDate;
         permitDate = jhdr_branch["permit_start"].asString();
         _util->date_split(permitDate.c_str(),yyyy,mm,dd);

         tmp  = "Date Issued: ";
         tmp += mm.c_str();
         tmp += "/";
         tmp += dd.c_str();
         tmp += "/";
         tmp += yyyy.c_str();
         _txtfoot += _util->cyCenterText(_rcptWidth,tmp);
     }
     _txtfoot += _util->cyCenterText(_rcptWidth, " ");
     _receipt += _txtfoot;
     return true;
}
