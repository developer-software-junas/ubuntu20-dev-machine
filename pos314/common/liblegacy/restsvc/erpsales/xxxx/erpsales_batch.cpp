/********************************************************************
          FILE:          erpsales_export_header.cpp
   DESCRIPTION:          ERP sales export
 ********************************************************************/
//////////////////////////////////////////
//  Header(s)
#include <string>
#include <sstream>
//////////////////////////////////////////
//  Qt header files
#include <QObject>
#include <QThread>
#include <QUrlQuery>
#include <QEventLoop>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QCoreApplication>
#include <QNetworkAccessManager>
//////////////////////////////////////////
#include "erpsales.h"
#include "cyposrest.h"
using std::string;
using std::stringstream;

using salestax::ENUM_SALES_TAX;
using salespay::ENUM_SALES_PAY;
using salesitem::ENUM_SALES_ITEM;
using salesinvoice::ENUM_SALES_INVOICE;
/********************************************************************
      FUNCTION:         batch
   DESCRIPTION:         Sales batch processing
 ********************************************************************/
bool                    cyerpsales::batch ()
{
    if (!batch_sales()) return false;
    if (!batch_sales_void()) return false;
    if (!batch_sales_refund()) return false;
    return true;
}

/********************************************************************
      FUNCTION:         batch_sales
   DESCRIPTION:         Sales batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales ()
{
    cyposrest*          rest;
    Json::Value         jsales;
    string              sqlperm, systransnum;
    CYDbSql*            seldb  = _ini->dbconnini("default");
    CYDbSql*            posdb  = _ini->dbconnini("default");
    printf ("Check the daily file...\n");
    sqlperm = "select systransnum from tg_pos_daily_header where is_polled2 <> 999 limit 10 ";
    seldb->sql_reset();
    if (!seldb->sql_result(sqlperm,false))
        return seterrormsg(seldb->errordb());

    stringstream ss;
    while (!seldb->eof())  {
        ss.clear ();
        jsales.clear();
        ss.str(std::string(""));
        systransnum = seldb->sql_field_value("systransnum");
        //////////////////////////////////
        //  Get the sales record
        rest = new cyposrest (_ini, _log, _cgi);
        if (!rest->jsonfmt_sales(DAILY,jsales,systransnum,false))
            return seterrormsg(_error.c_str());
        delete (rest);
        //ss << jsales; printf ("\n\n%s\n\n",ss.str().c_str());
        //////////////////////////////////
        //  Mark the record a read
        sqlperm  = "update tg_pos_daily_header set is_polled2 = 999 where systransnum = '";
        sqlperm += systransnum; sqlperm += "' ";
        posdb->sql_reset();
        if (!posdb->sql_only(sqlperm,false))
            return seterrormsg(posdb->errordb());
        if (!posdb->sql_commit())
            return seterrormsg(posdb->errordb());

        if (!online_sales(DAILY,jsales,systransnum))
            return false;
        seldb->sql_next();
    };
    return true;
}
/********************************************************************
      FUNCTION:         batch_sales_void
   DESCRIPTION:         VOID batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales_void ()
{
    Json::Value         jsales;
    string              sqlperm, systransnum;
    CYDbSql*            posdb  = _ini->dbconnini("default");
    printf ("Check the void file...\n");
    sqlperm = "select systransnum from tg_pos_void_header where is_polled2 <> 999 limit 1 ";
    posdb->sql_reset();
    if (!posdb->sql_result(sqlperm,false))
        return seterrormsg(posdb->errordb());
    if (posdb->eof())
        return true;

    stringstream ss;
    //while (!posdb->eof())  {
        ss.clear ();
        jsales.clear();
        ss.str(std::string(""));
        systransnum = posdb->sql_field_value("systransnum");

        Json::Value jresult;
        string apiurl  = "http";
        apiurl += "://";
        apiurl += "localhost";
        apiurl += ":";
        apiurl += "8000";
        apiurl += "/api/resource/Sales Invoice/";
        apiurl += systransnum;
        string raw, postargs = "{\"docstatus\": 2}";                
        restput(apiurl, postargs, jresult, raw);

        //////////////////////////////////
        //  Mark the record a read
        sqlperm  = "update tg_pos_void_header set is_polled2 = 999 where systransnum = '";
        sqlperm += systransnum; sqlperm += "' ";
        posdb->sql_reset();
        if (!posdb->sql_only(sqlperm,false))
            return seterrormsg(posdb->errordb());
        if (!posdb->sql_commit())
            return seterrormsg(posdb->errordb());

        posdb->sql_next();
    //};
    return true;
}
/********************************************************************
      FUNCTION:         batch_sales_refund
   DESCRIPTION:         VOID batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales_refund ()
{
    cyposrest*          rest;
    Json::Value         jsales;
    string              sqlperm, systransnum;
    CYDbSql*            posdb  = _ini->dbconnini("default");
    printf ("Check the refund file...\n");
    sqlperm = "select systransnum from tg_pos_refund_header where is_polled2 <> 999 limit 1 ";
    posdb->sql_reset();
    if (!posdb->sql_result(sqlperm,false))
        return seterrormsg(posdb->errordb());
    if (posdb->eof())
        return true;

    stringstream ss;
    //while (!posdb->eof())  {
        ss.clear ();
        jsales.clear();
        ss.str(std::string(""));
        systransnum = posdb->sql_field_value("systransnum");
        //////////////////////////////////
        //  HACK - TODO FIX
        rest = new cyposrest (_ini, _log, _cgi);
        if (!rest->jsonfmt_sales(REFUND,jsales,systransnum,false))
            return seterrormsg(_error.c_str());
        delete (rest);
        //ss << jsales; printf ("\n\n%s\n\n",ss.str().c_str());
        //////////////////////////////////
        //  Mark the record a read
        sqlperm  = "update tg_pos_refund_header set is_polled2 = 999 where systransnum = '";
        sqlperm += systransnum; sqlperm += "' ";
        posdb->sql_reset();
        if (!posdb->sql_only(sqlperm,false))
            return seterrormsg(posdb->errordb());
        if (!posdb->sql_commit())
            return seterrormsg(posdb->errordb());

        if (!online_sales(REFUND,jsales,systransnum))
            return false;
        if (!batch_sales_refund_submit(systransnum))
            return false;
        posdb->sql_next();
    //};
    return true;
}
/********************************************************************
      FUNCTION:         batch_sales_refund_submit
   DESCRIPTION:         VOID batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales_refund_submit (string systransnum)
{
    Json::Value         jsales;

    stringstream ss;

        ss.clear ();
        jsales.clear();
        ss.str(std::string(""));

        Json::Value jresult;
        string apiurl  = "http";
        apiurl += "://";
        apiurl += "localhost";
        apiurl += ":";
        apiurl += "8000";
        apiurl += "/api/resource/Sales Invoice/";
        apiurl += systransnum;
        string raw, postargs = "{\"docstatus\": 1}";
        restput(apiurl, postargs, jresult, raw);

    return true;
}
