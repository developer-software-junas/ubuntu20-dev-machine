//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceipteast.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptHeader
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptEast::assembleReceiptHeader (Json::Value& jheader,
                                           salesReceiptType receiptType)
{
     _txthdr = "";
     _receipt = "";
     _receiptType = receiptType;
     //code for debugging only
     /*
     std::stringstream out;
     out << jheader;
     _txthdr = out.str();
     return true;*/
     //////////////////////////////////////
     //  Display the header
     size_t  w = g_widthReceipt;
     string tmp = "", tmp2 = "",fullAddress = "",tmp3="",newLine = "\n";
     Json::Value jtemp2, jtemp = jheader["hdr_branch"];
     Json::Value jHdrBranch = jheader["hdr_branch"];
     Json::Value jHdrSetting = jheader["hdr_settings"];
     Json::Value jHdrCompany = jheader["hdr_company"];
     Json::Value jHdrSetReg = jheader["settings_register"];
     Json::Value jHdrSetCshrShift = jheader["settings_cashier_shift"];

     if (jtemp["hdrmsg1"].asString().length() > 0)  {
         _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg1"].asString());
     }
     if (jtemp["hdrmsg2"].asString().length() > 0)  {
         _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg2"].asString());
     }
     if (jtemp["hdrmsg3"].asString().length() > 0)  {
         _txthdr += _util->cyCenterText(w, jHdrBranch["hdrmsg3"].asString());
     }
     _txthdr += _util->cyCenterText(w, " ");
     _txthdr += _util->cyCenterText(w, jHdrSetting["trade_name"].asString());
     //_txthdr += _util->cyCenterText(w, jHdrCompany["description"].asString());
     _txthdr += _util->cyCenterText(w, jHdrBranch["description"].asString());

     _txthdr += _util->cyCenterText(w, jHdrBranch["addr1"].asString());
     _txthdr += _util->cyCenterText(w, jHdrBranch["addr2"].asString());

     tmp  = "VAT-REG-TIN-";
     tmp += jtemp["tin"].asString();
     _txthdr += _util->cyCenterText(w, tmp);

     tmp = jHdrSetReg["serialno"].asString();
     if (tmp.length() > w + 1) {
         tmp2 = tmp.substr(0, (w - 1));
     } else {
         tmp2 = tmp;
     }
     _txthdr += _util->cyCenterText(w, tmp2);

     tmp  = "STORE # ";
     tmp += jHdrSetCshrShift["location_code"].asString();
     tmp += " ";
     tmp += "POS #";
     tmp += jHdrSetCshrShift["register_num"].asString();

     _txthdr += _util->cyCenterText(w,tmp);
     _txthdr += _util->cyCenterText(w," ");
     if (receiptType == salesReceiptType::SRT_DAILY ||
         receiptType == salesReceiptType::SRT_NORMAL){
         _txthdr += _util->cyCenterText(w,"INVOICE");
         _txthdr += _util->cyCenterText(w," ");
     }

     //////////////////////////////////////
     //  Account
     Json::Value jacct, jaccount, jaccttype;
     if (jheader["hdr_trxaccount"] == Json::nullValue)  {
         jacct = Json::nullValue;
         jaccount = Json::nullValue;
     }  else  {
         jacct = jheader["hdr_trxaccount"];
         string account_type  = "settings_account_";
         account_type += jacct["account_type_code"].asString();
         jaccttype = jheader[account_type];

         if (jaccttype["is_pwd"].asString() == "1" ||
             jaccttype["is_senior"].asString() == "1" ||
             jaccttype["is_zero_rated"].asString() == "1")  {

             _txthdr += "Customer name: ";
             tmp  = jacct["first_name"].asString();
             tmp += " ";
             tmp += jacct["last_name"].asString();
             tmp += "\n";
             _txthdr += tmp;

             _txthdr += "Account type: ";
             tmp  = jacct["account_type_desc"].asString();
             tmp += "\n";
             _txthdr += tmp;

             _txthdr += "OSCA/PWD/PNSTM/SPIC ID No: ";
             tmp  = jacct["account_number"].asString();
             tmp += "\n";
             _txthdr += tmp;


             _txthdr += "Signature: _____________________________\n";
         }  else  {
             tmp = "Account Type  : ";
             tmp += jacct["account_type_desc"].asString();
             tmp += "\n";
             _txthdr += tmp;

             tmp = "Account#      : ";
             tmp += jacct["account_number"].asString();
             tmp += "\n";
             _txthdr += tmp;

             tmp = "Name          : ";
             if (jacct["first_name"].asString().length() > 0)  {
                 tmp += jacct["first_name"].asString();
                 tmp += " ";
             }
             tmp += jacct["last_name"].asString();
             tmp += "\n";
             _txthdr += tmp;
         }
         _receipt += _txthdr;
     }
     //////////////////////////////////////
     //  Table number
     Json::Value jhdrfnb = jheader["hdr_fnb"];
     if (jhdrfnb != Json::nullValue)  {
         string server = jhdrfnb["server"].asString();
         string tabledesc = jhdrfnb["tabledesc"].asString();


         tmp2 = "Server        : ";
         tmp2 += server;
         tmp2 += "\n";
         _txthdr += tmp2;

         tmp2 = "Table         : ";
         tmp2 += tabledesc;
         tmp2 += "\n";
         _txthdr += tmp2;
     }
     //////////////////////////////////////
     //  IPAD???
     Json::Value jregister = jheader["settings_register"];
     string is_pos = jregister["is_pos"].asString();
     if (is_pos == "0")  {
         if (jheader["hdr_lanebust"] != Json::nullValue)  {
             Json::Value jfnb = jheader["hdr_lanebust"];

             tmp = "Order         : ";
             tmp += jfnb["is_dine"].asString() == "1" ? "DINE IN" : "TAKEOUT";
             tmp += "\n";
             _txthdr += tmp;

             tmp = "Buzzer number : ";
             tmp += jfnb["buzzer_number"].asString();
             tmp += "\n";
             _txthdr += tmp;

             tmp = "Customer      : ";
             tmp += jfnb["customer_name"].asString();
             tmp += "\n";
             _txthdr += tmp;

         }
     }     
     //////////////////////////////////////
     //  Header label(s)
     char sztmp[24];
     long lRegister = atol (jHdrSetReg["register_num"].asString().c_str ());
     long lStoreLocation = atol (jHdrSetCshrShift["location_code"].asString().c_str ());
     sprintf (sztmp, "%02ld-%03ld", lStoreLocation,lRegister);


     if (receiptType == salesReceiptType::SRT_REFUND) {
         tmp2 = "Trx No        : ";
         long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
         sprintf (sztmp, "%012ld", ltrx);
         tmp2 += sztmp;

         _txthdr += tmp2.c_str();
         _txthdr += newLine.c_str();

     }  else  {
         if (receiptType == salesReceiptType::SRT_RESUME) {
                 tmp2 = "Suspend #     : ";
                 tmp3 = jheader["transaction_number"].asString ();
                 _txthdr += _util->cyLRText(23,tmp2.c_str(),18,tmp3.c_str());
         }  else if (receiptType == salesReceiptType::SRT_POSTVOID) {

             tmp2 = "Trx No        : ";
             long ltrx = atol(jheader["picture_url"].asString ().c_str ());
             sprintf (sztmp, "%012ld", ltrx);
             tmp2 += sztmp;

             _txthdr += tmp2.c_str();
             _txthdr += "\n";


             tmp2 = "Orig-Invoice# : ";
             ltrx = atol(jheader["transaction_number"].asString ().c_str ());
             sprintf (sztmp, "%012ld", ltrx);
             tmp2 += sztmp;

             _txthdr += tmp2.c_str();
             _txthdr += "\n\n";


         }  else if (receiptType == salesReceiptType::SRT_RESUME ||
                     receiptType == salesReceiptType::SRT_OPTICAL) {
             if (receiptType == SRT_RESUME)
                 tmp2  = "Suspend#: ";
             else
                 tmp2 = "PO Number: ";
             tmp2 += jheader["transaction_number"].asString ();
             tmp2 += "\n";
             _txthdr += tmp2;
         }  else if (receiptType == salesReceiptType::SRT_CANCEL) {
             tmp = jheader["transaction_number"].asString();
             long ltrxnum = atol (tmp.c_str ());
             sprintf (sztmp, "%012ld", ltrxnum);

             tmp2 = "Trx Number    : ";
             tmp2 += sztmp;
             tmp2 += "\n";
             _txthdr += tmp2;
         }  else if (receiptType != salesReceiptType::SRT_SUSPEND) {
             tmp2 = "Invoice Number: ";
             long ltrx = atol(jheader["transaction_number"].asString ().c_str ());
             sprintf (sztmp, "%012ld", ltrx);
             tmp2 += sztmp;

             _txthdr += tmp2.c_str();
             _txthdr += newLine.c_str();
         }
     }
     //////////////////////////////////////
     //  Special receipt header(s)

     if (receiptType == salesReceiptType::SRT_REPRINT) {
         _txttype = "reprint";
         _txthdr += _util->cyCenterText(w, "* * *  R E P R I N T * * *");
     }
     else if (receiptType == salesReceiptType::SRT_POSTVOID) {
         _txthdr += _util->cyCenterText(w, "* * *  V O I D * * *");
         _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
     }
     else if (receiptType == salesReceiptType::SRT_SUSPEND) {
         _txthdr += _util->cyCenterText(w, "* * *  S U S P E N D  * * *");
     }
     else if (receiptType == salesReceiptType::SRT_CANCEL) {
         _txthdr += _util->cyCenterText(w, "* * *  C A N C E L  * * *");
         _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
     }
     else if (receiptType == salesReceiptType::SRT_REFUND) {
         _txthdr += _util->cyCenterText(w, "* * *  R E T U R N S  * * *");
         _txthdr += _util->cyCenterText(w, jheader["remarks"].asString());
     }
/*
     //CONSTRUCT DATE IN RCC FORMAT
     string trans_time,trans_date,dd,mm,yyyy;
     trans_date = jheader["transaction_date"].asString();
     trans_time = jheader["transaction_time"].asString();
     _util->date_split(trans_date.c_str(),yyyy,mm,dd);
     tmp2  = "Date-time     : ";
     tmp2 += mm;
     tmp2 += "/";
     tmp2 += dd;
     tmp2 += "/";
     tmp2 += yyyy;
     tmp2 += "-";
     tmp2 += _util->time_colon(trans_time.c_str());

     _txthdr += tmp2.c_str();
     _txthdr += newLine.c_str();


     Json::Value jcashier;
     if (jheader["settings_cashier"] == Json::nullValue)
         return seterrormsg("Unable to get the cashier details");
     jcashier = jheader["settings_cashier"];

     size_t len = jcashier["first_name"].asString().length();
     len += jcashier["last_name"].asString().length();
     len += 14;
     len += jcashier["login"].asString().length();

     if (len > 40)  {
         _txthdr += "Cashier       : ";
         _txthdr += jcashier["first_name"].asString();
         _txthdr += " ";
         _txthdr += jcashier["last_name"].asString();

         _txthdr += "\n           [";
         _txthdr += jcashier["login"].asString();
         _txthdr += "]\n";
     }  else  {
         _txthdr += "Cashier       : ";
         _txthdr += jcashier["first_name"].asString();
         _txthdr += " ";
         _txthdr += jcashier["last_name"].asString();
         _txthdr += "[";
         _txthdr += jcashier["login"].asString();
         _txthdr += "]\n";
     }

     if (receiptType == SRT_REFUND)  {

         _txthdr += "Orig Pos No   : ";
         _txthdr += jheader["orig_register_number"].asString();
         _txthdr += newLine.c_str();

         _txthdr += "Orig Bus Date : ";
         _txthdr += jheader["orig_transaction_date"].asString();
         _txthdr += newLine.c_str();

         _txthdr += "Orig-Invoice# : ";                    

         char sztmp[32];
         long ltrx = atol(jheader["orig_transaction_number"].asString ().c_str ());
         sprintf (sztmp, "%012ld", ltrx);
         _txthdr += sztmp;
         _txthdr += newLine.c_str();
     }
*/
     //////////////////////////////////////
     //  FSP
     if (jheader["hdr_account"] == Json::nullValue)  {
     }  else  {
         jaccount = jheader["hdr_account"];
         int j = jaccount.size();
         for (int i = 0; i < j; i++)  {
             jacct = jaccount[i];
             if (jacct["account_type_code"].asString() == "FSP" ||
                 jacct["account_type_code"].asString() == "FSPP") {
                     _txthdr += "Account type  : ";
                     _txthdr += jacct["account_type_desc"].asString();
                     _txthdr += "\n";

                     //_txthdr += "Customer name : xxxxxxxx xxxxxxxx";
                     //_txthdr += jacct["last_name"].asString();
                     //_txthdr += ", ";
                     //_txthdr += jacct["first_name"].asString();
                     //_txthdr += "\n";

                     _txthdr += "Card number   : ";
                     _txthdr += jacct["account_number"].asString();
                     _txthdr += "\n";
                     _txthdr += "\n";
                     _txthdr += "Current Available Points: ";
                     _txthdr += _util->fmt_number_comma(jacct["points_balance"].asString().c_str());
                     _txthdr += "\n";
                     _txthdr += "Current Available ePurse: ";

                     string totalEPoints = _util->fmt_number_comma(jacct["epurse_balance"].asString().c_str());
                     char sztmp [128];
                     memset (sztmp, 0x00, 128);
                     strcpy (sztmp, totalEPoints.c_str());
                     size_t y = strlen(sztmp);
                     for (size_t x = 0; x < y; x++)  {
                         if (sztmp[x]=='.') sztmp[x]='\0';
                     }
                     //if (totalEPoints.length() > 4)
                         //totalEPoints[totalEPoints.length()-3]='\0';
                     _txthdr += sztmp;//_util->fmt_number_comma(totalEPoints.c_str());
                     _txthdr += "\n";
             }
         }
     }
     //////////////////////////////////////
     //  Transaction type
     _txthdr += _util->cyCenterText(w, " ");
     jtemp = jheader["settings_transtype"];
     _txthdr += _util->cyCenterText(w, jtemp["description"].asString());


     _txthdr += _util->cyAcrossText(w, "=");
     _receipt += _txthdr;
     return true;
}
