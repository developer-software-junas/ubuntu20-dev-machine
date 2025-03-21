/********************************************************************
          FILE:         cyposrest.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  QT header(s)
#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
//////////////////////////////////////////
//  REST class

#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
typedef unsigned char byte;
/********************************************************************
      FUNCTION:         rest_receipt_eod
   DESCRIPTION:         X/Z read receipt
 ********************************************************************/
std::string             cyposrest::rest_receipt_eod ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string cashier = cgiform("cashier");
    string reference = cgiform("reference");
    string register_num = cgiform("register_num");
    string location_code = cgiform("location_code");

    _log->logmsg("QTRESTRECEIPT TYPE",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",reference.c_str ());
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Get the transaction date
    sql  = "select * from pos_sysdate ";
    sql += "where location_code = '";
    sql += location_code;
    sql += "' and register_num = ";
    sql += register_num;
    sql += " and   is_sync = -888 and (eod_timestamp = 0 or eod_timestamp is null) ";
    sql += " order by transaction_date desc ";
    _log->logmsg("ZREADPRINT",sql.c_str());
    //sql  = "select * from pos_sysdate order by transaction_date desc limit    1";
    if (!db->sql_result(sql,false))
        return _util->jsonerror(db->errordb());
    if (db->eof())
        return _util->jsonerror("Unable to retrieve the transaction date");
    string transaction_date = db->sql_field_value("transaction_date");
    //////////////////////////////////////
    //  ...under construction
    string encreceipt = cgiform("receipt");
    string receipt = _util->base64decode(encreceipt);
    //_log->logmsg("EOD",receipt.c_str ());
    //////////////////////////////////////
    //  Save the file
    string filename  = _ini->get_value("PATH","RECEIPT");
    filename += "/";
    filename += reference;
    filename += ".txt";
    _util->file_create(filename.c_str (),receipt);

    string timestamp = _util->strtime_t();
    if (!updateJournal(type,timestamp,reference,receipt))
        return _util->jsonerror(_error);

    db->sql_reset ();
    sql  = "insert into tg_pos_receipt (systransnum, type, txt_header, ";
    sql += "txt_detail, txt_totals, txt_payment, txt_vat, txt_signature, ";
    sql += "txt_footer, txt_giftwrap, copies, cashier, shift, logical_date, ";
    sql += "register_num, location_code, timestamp, printer_name, is_polled2) values (";
    sql += db->sql_bind(1,  timestamp); sql += ", ";
    sql += db->sql_bind(2,  type); sql += ", '', ";
    sql += db->sql_bind(3,  receipt); sql += ", '', '', '', '', '', '', 2, ";
    sql += db->sql_bind(4,  cashier); sql += ", 0, ";
    sql += db->sql_bind(5,  transaction_date); sql += ", ";
    sql += db->sql_bind(6,  register_num); sql += ", ";
    sql += db->sql_bind(7,  location_code); sql += ", ";
    sql += db->sql_bind(8, timestamp); sql += ",'default', 0)";
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());

    //////////////////////////////////////
    //  Increment the register totals
    if (type == "zread" || type == "zreadprinted")  {
        db->sql_reset ();
        sql  = "update pos_register set ";
        sql += "  last_adjust_transnum = adjust_transnum ";
        sql += ", last_adjust_amount = last_adjust_amount + adjust_amount ";
        sql += " where location_code = ";
        sql += db->sql_bind( 1, location_code);
        sql += " and register_num = ";
        sql += db->sql_bind( 2, register_num);

        if (!db->sql_only (sql,true))
            return _util->jsonerror(db->errordb());

        db->sql_bind_reset ();
        sql  = "update pos_register set ";
        sql += "adjust_amount = 0.00 ";
        sql += " where location_code = ";
        sql += db->sql_bind( 1, location_code);
        sql += " and register_num = ";
        sql += db->sql_bind( 2, register_num);

        if (!db->sql_only (sql,true))
            return _util->jsonerror(db->errordb());
    }



    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());

    string ret = _util->jsonok("xzread");
    if (type == "xread" || type == "zread")  {
        ret = "";
        for (int i = 0; i < printCopy; i++)
          ret = deviceLocal (receipt.length(),  receipt,
                     //(unsigned char*)receipt.c_str (),
                     type,reference);
    }
    deviceKick();
    return ret;
}
