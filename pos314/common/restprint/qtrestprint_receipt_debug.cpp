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
#include "cyw_aes.h"
#include "cyposrest.h"
#include "restsvc/storesales/cyrestreceipt.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         rest_receipt_debug
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             cyposrest::rest_receipt_debug ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = "postvoid";
    string salesType = "5";
    string systransnum = "1-000004-20231213-11-1000";
    string printer_code = "DEFAULT";
    //  LOG
    _log->logmsg("QTRESTRECEIPT TYPE",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",systransnum.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Retrieve the transaction details
    Json::Value jheader;
    CYRestCommon *rcpt = _ini->receipt_type (_log,_cgi);
    int nsalesType = atoi(salesType.c_str ());

    rcpt->resetEodTotals();
    if (!rcpt->retrieveSalesHeader(jheader, systransnum, (salesReceiptType) nsalesType))
        return _util->jsonerror(rcpt->errormsg());
    //  Header details
    string cashier = jheader["cashier"].asString ();
    string shift = jheader["cashier_shift"].asString ();
    string logical_date = jheader["logical_date"].asString ();
    string location_code = jheader["branch_code"].asString ();
    string register_num = jheader["register_number"].asString ();

    if (!rcpt->retrieveSalesHeaderAccount(jheader))
        return _util->jsonerror(rcpt->errormsg());

    Json::Value jdetail;
    jdetail.clear();
    if (!rcpt->retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
        return _util->jsonerror(rcpt->errormsg());
    jheader["det_sales"]=jdetail;

    Json::Value jpayment;
    jpayment.clear();
    if (!rcpt->retrieveSalesPayment(jpayment, systransnum))
        return _util->jsonerror(rcpt->errormsg());
    jheader["pay_sales"]=jpayment;

    if (!rcpt->assembleFspUrl(jheader, jdetail, jpayment))
        return _util->jsonerror(rcpt->errormsg());
    //////////////////////////////////////
    //  Create the receipt
    rcpt->_txttype = "print";
    if (!rcpt->assembleReceipt(jheader, jdetail, jpayment, (salesReceiptType) nsalesType))
        return _util->jsonerror((rcpt->errormsg()));
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Save to receipt journal
    db->sql_reset();
    sql  = "delete from tg_pos_receipt where systransnum = ";
    sql += db->sql_bind(1, systransnum); sql += " ";
    sql += " and type = ";
    sql += db->sql_bind(2, rcpt->_txttype); sql += " ";
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    //////////////////////////////////////
    //  GIFT PROMO ITEM - APPEND TO GIFT TEXT
    rcpt->_txtgift += rcpt->_txtpromoitem;



    db->sql_bind_reset();
    string ts = _util->strtime_t();
    if (rcpt->_txtcopies.length() < 1) rcpt->_txtcopies = "1";
    sql  = "insert into tg_pos_receipt (systransnum, type, txt_header, ";
    sql += "txt_detail, txt_totals, txt_payment, txt_vat, txt_signature, ";
    sql += "txt_footer, txt_giftwrap, copies, cashier, shift, logical_date, ";
    sql += "register_num, timestamp, location_code, printer_name, is_polled2) values (";
    sql += db->sql_bind(1,  systransnum); sql += ", ";
    sql += db->sql_bind(2,  rcpt->_txttype); sql += ", ";
    sql += db->sql_bind(3,  rcpt->_txthdr); sql += ", ";
    sql += db->sql_bind(4,  rcpt->_txtdet); sql += ", ";
    sql += db->sql_bind(5,  rcpt->_txttot); sql += ", ";
    sql += db->sql_bind(6,  rcpt->_txtpay); sql += ", ";
    sql += db->sql_bind(7,  rcpt->_txtvat); sql += ", ";
    sql += db->sql_bind(8,  rcpt->_txtsig); sql += ", ";
    sql += db->sql_bind(9,  rcpt->_txtfoot); sql += ", ";
    sql += db->sql_bind(10, rcpt->_txtgift); sql += ", ";
    sql += db->sql_bind(11, rcpt->_txtcopies); sql += ", ";
    sql += db->sql_bind(12, cashier); sql += ", ";
    sql += db->sql_bind(13, shift); sql += ", ";
    sql += db->sql_bind(14, logical_date); sql += ", ";
    sql += db->sql_bind(15, register_num); sql += ", ";
    sql += db->sql_bind(16, ts); sql += ", ";
    sql += db->sql_bind(17, location_code); sql += ", 'default', 0)";
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());
    //////////////////////////////////////
    //  Prepare the network XML print data
    string strPrintData = "";
    strPrintData += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
    strPrintData += "<s:Header>\r\n";
    strPrintData += "<parameter xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<devid>local_printer</devid>\r\n";
    strPrintData += "</parameter>\r\n";
    strPrintData += "</s:Header>\r\n";
    strPrintData += "<s:Body>\r\n";
    strPrintData += "<epos-print xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<text lang=\"em\" />\r\n";
    strPrintData += "<text smooth=\"true\" />\r\n";

    strPrintData += "<text>";
    strPrintData += rcpt->_txthdr;
    strPrintData += rcpt->_txtdet;
    strPrintData += rcpt->_txttot;
    strPrintData += rcpt->_txtpay;
    strPrintData += rcpt->_txtacct;
    strPrintData += rcpt->_txtvat;
    strPrintData += rcpt->_txtsig;
    strPrintData += rcpt->_txtfoot;
    strPrintData += rcpt->_txtgift;
    strPrintData += "</text>";

    strPrintData += "<text align=\"center\" />\r\n";
    strPrintData += "<symbol width=\"6\" type=\"qrcode_model_2\"  level=\"level_m\">";
    strPrintData += systransnum;
    strPrintData += "</symbol>";

    //if(nKick == 1 ){
         //strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_200\" />\r\n";
    //}

    strPrintData += "<cut />\r\n";
    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";
    //////////////////////////////////////
    //  Select the printer
    string strPrinterURL = printer_code;
    _log->logmsg("QTRESTRECEIPT URL",strPrinterURL.c_str ());
    //////////////////////////////////////
    //  Send to the network printer
    struct_cymem mem;
    cymem_init(&mem,1024);
    //////////////////////////////////////
    //  PRINT
    cymem_https_post(80,18,strPrinterURL.c_str (),strPrintData.c_str (),&mem);

    size_t st = mem.size;
    _log->logmsg("QTRESTRECEIPT HTTPRESULT",mem.memory);
    cymem_clean(&mem);

    if (st < 1)  {
        return _util->jsonerror("Unable to print");
    }
    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=rcpt->_receipt;

    stringstream out;
    out << root;
    return out.str ();
}
