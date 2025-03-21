/********************************************************************
          FILE:         cyposrest.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
typedef unsigned char byte;
/********************************************************************
      FUNCTION:         rest_receipt_raw
   DESCRIPTION:         Print the raw buffer
 ********************************************************************/
std::string             cyposrest::rest_receipt_validate ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string cashier = cgiform("cashier");
    string receipt = cgiform("receipt");
    string systransnum = cgiform("reference");
    string paysequence = cgiform("pay_sequence");
    string position = cgiform("reference_count");
    string printer_code = cgiform("printer_code");
    if (position.length() < 1) position = "1";
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    _log->logmsg ("TYPEVALIDATE:", type.c_str ());
    //////////////////////////////////////
    //  Retrieve the header record
    db->sql_reset();
    sql  = "select * from tg_pos_daily_header where systransnum = ";
    sql += db->sql_bind(1, systransnum);
    if (!db->sql_result(sql,true))  {
        _log->logmsg ("TYPEERROR:", db->errordb().c_str ());
        return _util->jsonerror(db->errordb());
    }
    if (db->eof ())  {
        _log->logmsg ("TYPEERROR:", "No header");
        return _util->jsonerror("Unable to retrieve the payment record");
    }

    _log->logmsg ("TYPEHEADER:", type.c_str ());
    string curDate = _util->date_eight();
    string curTime = _util->int_time();
    cashier = db->sql_field_value("cashier");
    string branch = db->sql_field_value("branch_code");
    string regNum = db->sql_field_value("register_number");
    string trCode = db->sql_field_value("transaction_code");
    string trxNum = db->sql_field_value("transaction_number");
    //////////////////////////////////////
    //  Retrieve the payment record
    db->sql_reset();
    if (type == "validate_gc")  {
        _log->logmsg ("TYPEGC:", type.c_str ());
        sql  = "select * from tg_pos_mobile_gc_payment where systransnum = ";
    }  else  {
        sql  = "select * from tg_pos_mobile_payment where systransnum = ";
    }
    sql += db->sql_bind(1, systransnum); sql += " ";
    sql += " and pay_seq = ";
    sql += db->sql_bind(2, paysequence); sql += " ";
    if (!db->sql_result(sql,true))
        return _util->jsonerror(db->errordb());
    if (db->eof ())
        return _util->jsonerror("Unable to retrieve the payment record");
    _log->logmsg ("TYPEDETAIL:", type.c_str ());
    //////////////////////////////////////
    //  Format
    receipt = "";
    if (type == "validate_gc")  {


        receipt += "            ";
        receipt += curDate; receipt += " ";
        receipt += curTime; receipt += " ";
        receipt += branch;  receipt += " ";
        receipt += regNum;  receipt += "\n";

        receipt += "            ";
        receipt += trxNum;  receipt += " ";
        receipt += cashier; receipt += " ";
        receipt += trCode;  receipt += "\n\n";

        receipt += "            ";
        receipt += "GC series No: ";
        receipt += db->sql_field_value("gcnum");
        receipt += "\n";

        receipt += "            ";
        receipt += "Old balance: ";
        receipt += db->sql_field_value("original_balance");
        receipt += "\n";


        receipt += "            ";
        receipt += "Amt. purchased: ";
        receipt += db->sql_field_value("amount_paid");
        receipt += "\n";

        receipt += "            ";
        receipt += "New balance: ";
        receipt += db->sql_field_value("remaining_balance");
        receipt += "\n";
        _log->logdebug("VALIDATEGC", receipt.c_str ());
    }  else   {
        receipt += "            ";
        receipt += curDate; receipt += " ";
        receipt += curTime; receipt += " ";
        receipt += branch;  receipt += " ";
        receipt += regNum;  receipt += "\n";

        receipt += "            ";
        receipt += trxNum;  receipt += " ";
        receipt += cashier; receipt += " ";
        receipt += trCode;  receipt += "\n\n";

        receipt += "            ";
        receipt += "Account number: ";
        receipt += db->sql_field_value("account_number");
        receipt += "\n";

        receipt += "            ";
        receipt += "Old balance: ";
        receipt += db->sql_field_value("original_balance");
        receipt += "\n";

        double paid = _util->stodsafe(db->sql_field_value("payment_amount"));
        double balance = _util->stodsafe(db->sql_field_value("credit_balance"));
        double dpayAmt = balance - paid;

        char sztmp [32];
        sprintf (sztmp, "%.02f", dpayAmt);

        receipt += "            ";
        receipt += "Amt. purchased: ";
        receipt += db->sql_field_value("payment_amount");
        receipt += "\n";

        receipt += "            ";
        receipt += "New balance: ";
        receipt += sztmp;
        receipt += "\n";
    }
    _log->logmsg("VALIDATE", receipt.c_str ());
    return rest_validate (atoi(position.c_str ()),receipt);
}
