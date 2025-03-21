/********************************************************************
          FILE:         cyposrest_receipt.cpp
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
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
typedef unsigned char byte;
/********************************************************************
      FUNCTION:         rest_receipt_reprint
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             cyposrest::rest_receipt_reprint ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string receipt = cgiform("receipt");
    string reference = cgiform("reference");
    string printer_code = cgiform("printer_code");

    _log->logmsg("QTRESTRECEIPT TYPE: REPRINT",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",reference.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Retrieve receipt data
    db->sql_reset ();
    if (receipt.empty())  {
        sql  = "select * from tg_pos_journal where systransnum = '";
        sql += reference;
        sql += "' ";

        if (!db->sql_result(sql,false))
            return _util->jsonerror(db->errordb());
        if (db->eof ())  {
            return _util->jsonerror("Unable to retrieve the receipt journal entry");
        }  else  {
            //////////////////////////////////////
            //  Retrieve the original receipt
            receipt = db->sql_field_value("txt_receipt");
            string insert = "CUSTOMER ID: 999999\n\n";
            insert += _util->cyCenterText(40,"* * *  R E P R I N T * * *\n");
            size_t pos = receipt.find("CUSTOMER ID: 999999");
            if (pos != std::string::npos) {
                receipt.replace(pos,40,insert);
            }
            size_t newpos = receipt.find("CUSTOMER ID: 999999",pos+40);
            if (newpos != std::string::npos) {
                receipt.replace(newpos,39,insert);
            }
            pos = newpos + 40;
            insert = "\n";
            insert += _util->cyCenterText(40,"* * *  R E P R I N T * * *");
            do  {
                newpos = receipt.find(RCPT_REPRINT,pos);
                if (newpos != std::string::npos) {
                    receipt.replace(newpos,39,insert);
                    pos = newpos+40;
                }
            }  while (newpos != std::string::npos);
        }
    }  else  {
        receipt = _util->base64decode(receipt);
    }

    string ret = "";
        if (printer_code != "DEFAULT")  {
            //////////////////////////////////////
            //  Print the logo
            string logoFile = _ini->get_value("DEVICE","LOGO");
            if (_util->file_exists(logoFile.c_str()))  {
                deviceNetworkImage(logoFile,printer_code);
            }
            ret = deviceNetwork (receipt,reference,printer_code);
        }  else  {
            ret = deviceLocal (receipt.length(), receipt,
                                //(unsigned char*)receipt.c_str (),
                                type,reference);
        }

    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=receipt;

    stringstream out;
    out << root;
    return out.str ();
}
