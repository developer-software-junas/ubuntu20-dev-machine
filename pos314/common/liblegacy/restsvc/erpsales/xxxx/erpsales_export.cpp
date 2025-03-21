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
      FUNCTION:         export_header
   DESCRIPTION:         Sales header
 ********************************************************************/
bool                    cyerpsales::online_sales  (enumHeader type,
                                                   Json::Value& jheader,
                                                   std::string systransnum)
{    
    if (!clear_sales_tables(systransnum))
        return false;

    _terms = false;
    _trxDiscount = 0;
    if (!export_header(type,jheader,systransnum))
        return false;
    if (!export_detail(type,jheader,systransnum))
        return false;
    //if (type != REFUND)  {
        if (!export_payment(type,jheader,systransnum))
            return false;
    //}

    Json::Value jsi;
    for (int i = 0; i < salesinvoice::ENUM_SALES_INVOICE::si_end; i++)  {
        string tmp = _invHdrValue[i];
        if (_util->valid_decimal(field_sales_invoice[i].c_str(),
                                 tmp.c_str(),1,28,0,999999999.99))  {

            jsi[field_sales_invoice[i]]=_util->stodsafe(_invHdrValue[i]);
        }  else  {
            jsi[field_sales_invoice[i]]=_invHdrValue[i];
        }
    }
    if (type == enumHeader::POSTVOID)
        jsi["docstatus"]=2;
    else if (type == enumHeader::REFUND)
        jsi["docstatus"]=0;
    else
        jsi["docstatus"]=1;
    jsi["custom_systransnum"]=systransnum;

    Json::Value items, taxes, line;
    for (size_t x = 0; x < _invDetail.size(); x++)  {
        //////////////////////////////////
        //  For each item sold
        line.clear();
        cylinedetail* detail = _invDetail.at(x);
        for (int i = 0; i < salesitem::ENUM_SALES_ITEM::si_end; i++)  {
            string tmp = detail->line[i];
            if (field_sales_item[i] != "item_code")  {
                if (_util->valid_decimal(field_sales_item[i].c_str(),
                                         tmp.c_str(),1,28,0,999999999.99))  {

                    line[field_sales_item[i]]=_util->stodsafe(detail->line[i]);
                }  else  {
                    line[field_sales_item[i]]=detail->line[i];
                }
            }  else  {
                line[field_sales_item[i]]=detail->line[i];
            }
        }
        //  Append an item
        items.append(line);
    }
    jsi["items"]=items;

    for (size_t x = 0; x < _invTax.size(); x++)  {
        //////////////////////////////////
        //  For each tax entry
        line.clear ();
        cylinetax* linetax = _invTax.at(x);
        for (int i = 0; i < salestax::ENUM_SALES_TAX::si_end; i++)  {
            string tmp = linetax->tax[i];
            string col = field_sales_tax[i];
            if (_util->valid_decimal(field_sales_tax[i].c_str(),
                                     tmp.c_str(),1,28,0,999999999.99))  {

                line[field_sales_tax[i]]=_util->stodsafe(linetax->tax[i]);
            }  else  {
                line[field_sales_tax[i]]=linetax->tax[i];
            }
        }
        //  Append a tax entry
        taxes.append(line);
    }
    for (size_t x = 0; x < _invSvc.size(); x++)  {
        //////////////////////////////////
        //  For each tax entry
        line.clear ();
        cylinetax* linetax = _invSvc.at(x);
        for (int i = 0; i < salestax::ENUM_SALES_TAX::si_end; i++)  {
            string tmp = linetax->tax[i];
            if (_util->valid_decimal(field_sales_tax[i].c_str(),
                                     tmp.c_str(),1,28,0,999999999.99))  {

                line[field_sales_tax[i]]=_util->stodsafe(linetax->tax[i]);
            }  else  {
                line[field_sales_tax[i]]=linetax->tax[i];
            }
        }
        //  Append a tax entry
        taxes.append(line);
    }
    jsi["taxes"]=taxes;

    Json::Value pay;
    for (size_t x = 0; x < _invPayment.size(); x++)  {
        line.clear();
        cylinepayment* payment = _invPayment.at(x);
        for (int i = 0; i < salespay::ENUM_SALES_PAY::si_end; i++)  {
            string tmp = payment->line[i];
            if (field_sales_pay[i] == "amount" ||
                field_sales_pay[i] == "base_amount")  {

                line[field_sales_pay[i]]=_util->stodsafe(payment->line[i]);
            }  else  {
                if (_util->valid_decimal(field_sales_pay[i].c_str(),
                                         tmp.c_str(),1,28,0,999999999.99))  {

                    line[field_sales_pay[i]]=_util->stodsafe(payment->line[i]);
                }  else
                    line[field_sales_pay[i]]=payment->line[i];
            }
        }
        //  Append a payment
        pay.append(line);
    }
    if (_terms)
        jsi["payments"]=Json::nullValue;
    else
        jsi["payments"]=pay;
    //////////////////////////////////////
    //  Save the sales record
    stringstream ss; ss << jsi;
    string postargs = ss.str();
    _log->logmsg("SSS", postargs.c_str());

    std::string raw;
    Json::Value jresult;
    string port = _ini->get_value("erpapi","PORT");
    string host = _ini->get_value("erpapi","HOST");
    string protocol = _ini->get_value("erpapi","PROTOCOL");

    string apiurl  = "http";  apiurl += "://";
    apiurl += "localhost";  apiurl += ":";  apiurl += "8000";
    apiurl += "/api/resource/Sales Invoice";
    //////////////////////////////////////
    //  API request
    string szretcode = "{\"data\":{\"name\":\"";
    szretcode += systransnum; szretcode += ",\"owner\":\"Administrator\"";
    //printf ("\n\n%s\n\n", postargs.c_str());
    bool ret = restpost(apiurl, postargs, jresult, raw);

    size_t w = szretcode.size();
    if (0 == strncmp (szretcode.c_str(),raw.c_str(),(w-1)))  {
        return seterrormsg("Unable to POST the sales receipt");
    }
    //////////////////////////////////////
    //  Save the results
/***
    erpdb->sql_bind_reset ();
    sql = "delete from `tabPOS API Audit` where name = '"; sql += systransnum; sql += "' ";
    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());

    int idx = 1;
    erpdb->sql_bind_reset ();
    string dt = _util->date_mysql();
    sql  = "insert into `tabPOS API Audit` (name,creation,modified,modified_by,owner,docstatus,idx,";
    sql += "timestamp, request_url, request_args, reply) values (";

    sql += erpdb->sql_bind (idx, systransnum); sql += ","; idx++;
    sql += erpdb->sql_bind (idx, dt); sql += ","; idx++;
    sql += erpdb->sql_bind (idx, dt); sql += ","; idx++;
    sql += "'Administrator', 'Administrator', 0, 0, ";
    sql += erpdb->sql_bind (idx, dt); sql += ","; idx++;
    sql += erpdb->sql_bind (idx, apiurl); sql += ","; idx++;
    sql += erpdb->sql_bind (idx, postargs); sql += ","; idx++;
    sql += erpdb->sql_bind (idx, raw); sql += ") "; idx++;

    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());

    if (!erpdb->sql_commit())
        return seterrormsg(erpdb->errordb());
***/
    if (!ret)
        return seterrormsg("Unable to POST the sales record");

    //  TODO  parse and further validate success
    return ret;
}
