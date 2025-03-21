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
#include "mallreport/cyw_mallreport.h"
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
    //if (!batch_sales_void()) return false;
    //if (!batch_sales_refund()) return false;
    return true;
}

/********************************************************************
      FUNCTION:         batch_sales
   DESCRIPTION:         Sales batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales ()
{
    CYRestCommon*       rest;
    Json::Value         jsales;
    string              sqlperm, systransnum;
    CYDbSql*            seldb  = _ini->dbconnini("default");
    CYDbSql*            posdb  = _ini->dbconnini("default");
    //////////////////////////////////
    //  Retrieve sales
    //sqlperm = "select systransnum from tg_pos_daily_header where is_polled2 <> 999 limit 1 ";

    // for ayala
    sqlperm  = "select systransnum from tg_pos_daily_header where systransnum not in ";
    sqlperm += "(select systransnum from tg_mallreport_record where client = 'ayala')";
    sqlperm += " and logicaL_date > 20250131 limit 5";


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
        rest = _ini->receipt_type(_log,_cgi);
        if (!rest->retrieveSalesRecord(jsales,systransnum,salesReceiptType::SRT_DAILY))  {
            return seterrormsg(rest->errormsg());
        }
        rest->_txttype = "print";
        rest->assembleReceipt(rest->_jheader, rest->_jdetail, rest->_jpayment,salesReceiptType::SRT_DAILY);
        jsales = rest->_jheader;

        char sztmp [64];
        jsales["cytotal_items"] = rest->_totalItems;

        sprintf(sztmp, "%.02f", (rest->_totalNetSales*10000) / 10000);
        jsales["cytotal_net_sales"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalVatAmount*10000) / 10000);
        jsales["cytotal_vat_amount"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalTrxDiscount*10000) / 10000);
        jsales["cytotal_trx_discount"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalGrossAmount*10000) / 10000);
        jsales["cytotal_gross_amount"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalItemDiscount*10000) / 10000);
        jsales["cytotal_item_discount"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalNetVat*10000) / 10000);
        jsales["cytotal_net_vat"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalNetZero*10000) / 10000);
        jsales["cytotal_net_zero"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalNetExempt*10000) / 10000);
        jsales["cytotal_net_exempt"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalAmtVat*10000) / 10000);
        jsales["cytotal_amt_vat"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalAmtZero*10000) / 10000);
        jsales["cytotal_amt_zero"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalAmtExempt*10000) / 10000);
        jsales["cytotal_amt_exempt"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalAddonAmount*10000) / 10000);
        jsales["cytotal_amt_addon"] = atof(sztmp);

        sprintf(sztmp, "%.02f", (rest->_totalServiceCharge*10000) / 10000);
        jsales["cytotal_service_charge"] = atof(sztmp);
        //////////////////////////////////////
        //  OVERRIDE THE GENERATED RECEIPT
        rest->_receipt  = rest->_txthdr;
        rest->_receipt += rest->_txtdet;
        rest->_receipt += rest->_txttot;
        rest->_receipt += rest->_txtpay;
        rest->_receipt += rest->_txtacct;
        rest->_receipt += rest->_txtvat;
        rest->_receipt += rest->_txtsig;
        rest->_receipt += rest->_txtfoot;
        rest->_receipt += rest->_txtgift;
        //////////////////////////////////////
        //  Include the separated receipt
        //  in the sales record
        jsales["rcpt_header"]=_util->base64encode(rest->_txthdr);

        if (rest->_txtdet.length() > 0)
            jsales["rcpt_detail"]=_util->base64encode(rest->_txtdet);
        else
            jsales["rcpt_detail"]="";

        if (rest->_txttot.length() > 0)
            jsales["rcpt_totals"]=_util->base64encode(rest->_txttot);
        else
            jsales["rcpt_totals"]="";

        if (rest->_txtpay.length() > 0)
            jsales["rcpt_pay"]=_util->base64encode(rest->_txtpay);
        else
            jsales["rcpt_pay"]="";

        if (rest->_txtvat.length() > 0)
            jsales["rcpt_vat"]=_util->base64encode(rest->_txtvat);
        else
            jsales["rcpt_vat"]="";

        if (rest->_txtfoot.length() > 0)
            jsales["rcpt_footer"]=_util->base64encode(rest->_txtfoot);
        else
            jsales["rcpt_footer"]="";

        if (rest->_txtgift.length() > 0)
            jsales["rcpt_giftwrap"]=_util->base64encode(rest->_txtgift);
        else
            jsales["rcpt_giftwrap"]="";

        delete (rest);

        stringstream ss;
        //ss << jsales; printf("%s",std::string(ss.str()).c_str());

        //////////////////////////////////////////
        // Mallreport implementation
        mallType type = MT_ROBINSON;
        cymallreport* mall = nullptr;
        if (type == MT_AYALA)  {
            mall = new cymallayala (_log,_ini,_cgi);
        }  else if (type == MT_MEGAWORLD)  {
            mall = new cymallmegaworld (_log,_ini,_cgi);
        }  else if (type == MT_SMSIA)  {
            mall = new cymallsmsia (_log,_ini,_cgi);
        } else if (type == MT_ROBINSON)  {
            mall = new cymallrobinson (_log,_ini,_cgi);
        }  else  {
            return seterrormsg("Unknown mall type. Please check your INI file");
        }

        if (!mall->generateReport(jsales)) {
            return seterrormsg(_error);
        }
        if (!mall->insertReport())  {
            return seterrormsg(_error);
        }
        delete (mall);


        //if (!online_sales(DAILY,jsales,systransnum))
        //    return false;
        //////////////////////////////////
        //  Mark the record a read
        //sqlperm  = "update tg_pos_daily_header set is_polled2 = 999 where systransnum = '";
        //sqlperm += systransnum; sqlperm += "' ";


        // for ayala
        sqlperm = "Insert into tg_mallreport_record values ('";
        sqlperm += systransnum; sqlperm += "','ayala') ";
        posdb->sql_reset();
        if (!posdb->sql_only(sqlperm,false))
            return seterrormsg(posdb->errordb());
        if (!posdb->sql_commit())
            return seterrormsg(posdb->errordb());



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
    //////////////////////////////////
    //  Retrieve voided file(s)
    sqlperm = "select systransnum from tg_pos_void_header where is_polled2 <> 999 limit 1 ";
    posdb->sql_reset();
    if (!posdb->sql_result(sqlperm,false))
        return seterrormsg(posdb->errordb());
    if (posdb->eof())
        return true;

    stringstream ss;
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
    return true;
}
/********************************************************************
      FUNCTION:         batch_sales_refund
   DESCRIPTION:         VOID batch processing
 ********************************************************************/
bool                    cyerpsales::batch_sales_refund ()
{
    CYRestCommon*       rest;
    Json::Value         jsales;
    string              sqlperm, systransnum;
    CYDbSql*            posdb  = _ini->dbconnini("default");
    //////////////////////////////////
    //  Retrieve refunded file(s)
    sqlperm = "select systransnum from tg_pos_refund_header where is_polled2 <> 999 limit 1 ";
    posdb->sql_reset();
    if (!posdb->sql_result(sqlperm,false))
        return seterrormsg(posdb->errordb());
    if (posdb->eof())
        return true;

    stringstream ss;
    ss.clear ();
    jsales.clear();
    ss.str(std::string(""));
    systransnum = posdb->sql_field_value("systransnum");
    //////////////////////////////////
    //  Retrieve refunded record(s)
    rest = _ini->receipt_type(_log,_cgi);
    if (!rest->retrieveSalesRecord(jsales,systransnum,salesReceiptType::SRT_REFUND))  {
        return seterrormsg(rest->errormsg());
    }
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
