/********************************************************************
          FILE:          erpsales.cpp
   DESCRIPTION:          Publish data
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
using std::string;
using std::stringstream;

const std::string g_posnext_token = "Token 1f6c48a74a5f6c8:bc19053d585b440";
const std::string g_beelink_token = "Token 1f6c48a74a5f6c8:01c28b0bfec38f3";
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
cyerpsales::cyerpsales (cyini* ini, cylog* log, cycgi* cgi)
{
    _ini = ini;
    _log = log;
    _cgi = cgi;
    _error = "";
    _util = nullptr;
    _util = new cyutility ();    
    _token = _ini->get_value("erpapi","TOKEN");
    //////////////////////////////////////
    //  Database settings
    string key = "default";
    string type = _ini->get_value(key.c_str (),"TYPE");
    string host = _ini->get_value(key.c_str (),"HOST");
    string user = _ini->get_value(key.c_str (),"USER");
    string name = _ini->get_value(key.c_str (),"NAME");
    int port = atoi(_ini->get_value(key.c_str (),"PORT").c_str ());
    string pass = decode (_ini->get_value(key.c_str (),"PASS").c_str ());

    _pos = nullptr;
    _pos = new MySqlEnv;
    _pos->connect (host,user,pass,name,port);

    key = "erp";
    type = _ini->get_value(key.c_str (),"TYPE");
    host = _ini->get_value(key.c_str (),"HOST");
    user = _ini->get_value(key.c_str (),"USER");
    name = _ini->get_value(key.c_str (),"NAME");
    port = atoi(_ini->get_value(key.c_str (),"PORT").c_str ());
    pass = decode (_ini->get_value(key.c_str (),"PASS").c_str ());

    _erp = nullptr;
    _erp = new MySqlEnv;
    _erp->connect (host,user,pass,name,port);
}
/********************************************************************
      FUNCTION:         Destructor
   DESCRIPTION:         Class cleanup
 ********************************************************************/
cyerpsales::~cyerpsales ()
{
    cleanup_svc();
    cleanup_tax();
    cleanup_detail();
    cleanup_payment();

    if (nullptr != _pos)  {
        delete (_pos);
        _pos = nullptr;
    }
    if (nullptr != _erp)  {
        delete (_erp);
        _erp = nullptr;
    }
    if (nullptr != _util)  {
        delete (_util);
        _util = nullptr;
    }
}
/********************************************************************
      FUNCTION:         cleanup
   DESCRIPTION:         Cleanup data
 ********************************************************************/
void                    cyerpsales::cleanup_tax ()
{
    //////////////////////////////////////
    //  Cleanup the detail record(s)
    if (_invTax.size() > 0)  {
        for (size_t i = 0; i < _invTax.size(); i++)  {
            cylinetax* ptr = _invTax.at(i);
            delete ptr;
        }
        _invTax.clear();
    }
}
/********************************************************************
      FUNCTION:         cleanup
   DESCRIPTION:         Cleanup data
 ********************************************************************/
void                    cyerpsales::cleanup_svc ()
{
    //////////////////////////////////////
    //  Cleanup the detail record(s)
    if (_invSvc.size() > 0)  {
        for (size_t i = 0; i < _invSvc.size(); i++)  {
            cylinetax* ptr = _invSvc.at(i);
            delete ptr;
        }
        _invSvc.clear();
    }
}
/********************************************************************
      FUNCTION:         cleanup
   DESCRIPTION:         Cleanup data
 ********************************************************************/
void                    cyerpsales::cleanup_detail ()
{
    //////////////////////////////////////
    //  Cleanup the detail record(s)
    if (_invDetail.size() > 0)  {
        for (size_t i = 0; i < _invDetail.size(); i++)  {
            cylinedetail* ptr = _invDetail.at(i);
            delete ptr;
        }
        _invDetail.clear();
    }
}
/********************************************************************
      FUNCTION:         cleanup
   DESCRIPTION:         Cleanup data
 ********************************************************************/
void                    cyerpsales::cleanup_payment ()
{
    //////////////////////////////////////
    //  Cleanup the payment record(s)
    if (_invPayment.size() > 0)  {
        for (size_t i = 0; i < _invPayment.size(); i++)  {
            cylinepayment* ptr = _invPayment.at(i);
            delete ptr;
        }
        _invPayment.clear();
    }
}
/********************************************************************
      FUNCTION:         startup
   DESCRIPTION:         Initialize
 ********************************************************************/
bool                    cyerpsales::startup ()
{
    string               sql;
    cyutility            util;
    CYDbSql*             posdb  = _ini->dbconnini("default");
    if (nullptr == posdb)
        return seterrormsg (_ini->errormsg().c_str());
    CYDbSql*             erpdb  = _ini->dbconnini("erp");
    if (nullptr == erpdb)
        return seterrormsg (_ini->errormsg().c_str());
    //////////////////////////////////////
    //  Clear fields
    int j = salesinvoice::ENUM_SALES_INVOICE::si_end;
    for (int i = 0; i < j; i++)
        _invHdrValue[i]="";
    //////////////////////////////////////
    //  Get company / branch details
    sql  = "select * from cy_company ";
    if (!posdb->sql_result(sql,false))
        return seterrormsg(posdb->errordb().c_str());
    if (posdb->eof())
        return seterrormsg("Publish Sales - Unable to find company information\n");
    _abbr = posdb->sql_field_value ("abbr");
    _currency = posdb->sql_field_value ("currency_code");
    _company_name = posdb->sql_field_value ("description");
    _company_code = posdb->sql_field_value ("company_code");
    _company_tax_id = posdb->sql_field_value ("tax_id");

    sql  = "select * from cy_location where company_code = '";
    sql += _company_code; sql += "' ";
    if (!posdb->sql_result(sql,false))
        return seterrormsg(posdb->errordb().c_str());
    if (posdb->eof())
        return seterrormsg("Publish Sales - Unable to find company information\n");
    _branch_name = posdb->sql_field_value ("description");
    _branch_code = posdb->sql_field_value ("location_code");

    sql  = "select * from `tabBranch` where custom_cost_center like '";
    sql += _branch_code; sql += " -%'";
    if (!erpdb->sql_result(sql,false))
        return seterrormsg(erpdb->errordb().c_str());
    if (erpdb->eof())  {
        string tmp = "Publish Sales - Unable to find branch information: ";
        tmp += _branch_code;
        return seterrormsg(tmp.c_str());
    }
    //////////////////////////////////////
    //  ERP branch
    _jerpbranch.clear ();
    for (int i = 0; i < erpdb->numcols(); i++)
        _jerpbranch[erpdb->sql_field_name(i)]=erpdb->sql_field_value (erpdb->sql_field_name(i));
    //////////////////////////////////////
    //  ERP POS profile
    string posprofile = _jerpbranch["custom_profile"].asString();
    sql  = "select * from `tabPOS Profile` where name = '";
    sql += posprofile; sql += "' ";
    if (!erpdb->sql_result(sql,false))
        return seterrormsg(erpdb->errordb().c_str());
    if (erpdb->eof())
        return seterrormsg("Unable to retrieve the POS profile");

    _jposprofile.clear ();
    for (int i = 0; i < erpdb->numcols(); i++)
        _jposprofile[erpdb->sql_field_name(i)]=erpdb->sql_field_value (erpdb->sql_field_name(i));

    _price_list_currency = _currency;
    _cost_center  = _jerpbranch["custom_cost_center"].asString ();
    _tax_category = _jposprofile["tax_category"].asString ();
    _pos_profile  = _jerpbranch["custom_profile"].asString ();
    _set_warehouse = _jposprofile["warehouse"].asString ();
    _taxes_and_charges = _jposprofile["taxes_and_charges"].asString ();
    _selling_price_list = _jposprofile["selling_price_list"].asString ();
    _set_target_warehouse =_jerpbranch["custom_target_warehouse"].asString ();
    _account_for_change_amount = _jposprofile["account_for_change_amount"].asString ();
    _write_off_account = _jposprofile["write_off_account"].asString ();
    _write_off_cost_center = _jposprofile["write_off_cost_center"].asString ();
    _debit_to = _jerpbranch["custom_debit_to"].asString ();
    _party_account_currency = _currency;
    _expense_account =  _jerpbranch["custom_expense_account"].asString ();
    _against_income_account =  _jerpbranch["custom_against_income_account"].asString ();

    return true;
}
/********************************************************************
      FUNCTION:         salestohost
   DESCRIPTION:         transfer sales data
 ********************************************************************/
bool                    cyerpsales::salestohost (std::string systransnum)
{
    (void)systransnum;
    //////////////////////////////////////
    //  TODO
    return true;
}
/********************************************************************
      FUNCTION:         cancel_trx
   DESCRIPTION:         cancel a transaction
 ********************************************************************/
bool                    cyerpsales::cancel_trx (std::string systransnum)
{
    //////////////////////////////////////
    //  Cancel a transaction
    string postargs = "zzz=";  postargs += systransnum;

    std::string raw;
    Json::Value jresult;
    string port = _ini->get_value("erpapi","PORT");
    string host = _ini->get_value("erpapi","HOST");
    string protocol = _ini->get_value("erpapi","PROTOCOL");

    string apiurl  = "http://127.0.0.1:8000/api/method/brewedpos.api.poscanceltrx";
    //////////////////////////////////////
    //  API request
    bool ret = restpost(apiurl, postargs, jresult, raw);
    //////////////////////////////////////
    //  Check the results
    if (!ret)
        return seterrormsg("Unable to POST the sales record");

    //  TODO  parse and further validate success
    return ret;
}
/*******************************************************************
      FUNCTION:         restpost
   DESCRIPTION:         Post a rest api request
     ARGUMENTS:         url - the URL
                        postdata - the post data
       RETURNS:         true on success
 *******************************************************************/
bool                    cyerpsales::restpost (std::string url,
                                              std::string postdata,
                                              Json::Value &jresult,
                                              std::string &rawResult)
{
    (void)jresult;
    //////////////////////////////////////
    //  Post synchronously
    QEventLoop eventLoop;
    QNetworkAccessManager manager;
    QNetworkRequest request (QUrl(url.c_str()));
    //////////////////////////////
    //  Send a JSON POST request
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setRawHeader("Authorization",_token.c_str ());

    QNetworkReply* reply = manager.post(request,postdata.c_str());
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    if (!reply)
        return seterrormsg("FATAL: Error reading the reply ");
    QString result = reply->readAll();

    std::string tmpResult = result.toStdString();
    rawResult = tmpResult;
    //parseAndVerify(tmpResult, jresult);
    return true;
}
/*******************************************************************
      FUNCTION:         restput
   DESCRIPTION:         Post a rest api request
     ARGUMENTS:         url - the URL
                        postdata - the post data
       RETURNS:         true on success
 *******************************************************************/
bool                    cyerpsales::restput (std::string url,
                                              std::string postdata,
                                              Json::Value &jresult,
                                              std::string &rawResult)
{
    (void)jresult;
    //////////////////////////////////////
    //  Post synchronously
    QEventLoop eventLoop;
    QNetworkAccessManager manager;
    QNetworkRequest request (QUrl(url.c_str()));
    //////////////////////////////
    //  Send a JSON POST request
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    request.setRawHeader("Authorization",_token.c_str ());

    QNetworkReply* reply = manager.put(request,postdata.c_str());
    QObject::connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    eventLoop.exec();
    if (!reply)
        return seterrormsg("FATAL: Error reading the reply ");
    QString result = reply->readAll();

    std::string tmpResult = result.toStdString();
    rawResult = tmpResult;
    //parseAndVerify(tmpResult, jresult);
    return true;
}
/*******************************************************************
      FUNCTION:         parseandverify
   DESCRIPTION:         Parse the JSON string and check errors
     ARGUMENTS:         strjson - the JSON string
                        root - the JSON root object
       RETURNS:         true on success
 *******************************************************************/
bool                    cyerpsales::parseAndVerify (std::string queryResult,
                                                    Json::Value& jresult)
{
    //////////////////////////////
    //  JSON parse
    JSONCPP_STRING err;
    Json::CharReaderBuilder builder;
    const std::string rawJson = queryResult;
    const auto rawJsonLength = static_cast<int>(rawJson.length());
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    jresult.clear();
    if (!reader->parse(rawJson.c_str(),
                       rawJson.c_str() + rawJsonLength,
                       &jresult, &err)) {
        _error = "Unable to parse the JSON reply";
        qInfo() << "FATAL: JSON parse error: "
                << rawJson.c_str ();
        return false;;
    }
    //////////////////////////////
    //  Error check
    if (jresult.isMember("error"))  {
        _error  = "Unable to parse the JSON reply: ";
        _error += jresult["error"].asString ();
        qInfo() << _error.c_str ();
        return false;
    }
    if (jresult.isMember("userErrors"))  {
        _error  = "Unable to parse the JSON reply: ";
        Json::Value jtmp = jresult["errors"];
        if (jtmp.size() > 0)  {
            Json::Value jtmp2 = jtmp[0];
            if (jtmp2.isMember("message"))  {
                _error = jtmp2["message"].asString();
                return true;
            }
        }
        qInfo() << _error.c_str ();
        return false;
    }
    return true;
}
/********************************************************************
      FUNCTION:         clear_sales_tables
   DESCRIPTION:         empty sales tables
 ********************************************************************/
bool                    cyerpsales::clear_sales_tables(std::string systransnum)
{
    string sql;
    MySqlDb db (_erp);
    MySqlDb* erpdb = &db;

    erpdb->sql_reset ();
    sql  = "delete from `tabSales Invoice Item` where parent like '";
    sql += systransnum; sql += "%' ";
    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());


    erpdb->sql_bind_reset ();
    sql  = "delete from `tabSales Taxes and Charges` where parent like '";
    sql += systransnum; sql += "%' ";
    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());

    erpdb->sql_bind_reset ();
    sql  = "delete from `tabSales Invoice Payment` where parent like '";
    sql += systransnum; sql += "%' ";
    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());


    erpdb->sql_bind_reset ();
    sql = "delete from `tabSales Invoice` where name = '"; sql += systransnum; sql += "' ";
    if (!erpdb->sql_only(sql,false))
        return seterrormsg(erpdb->errordb());


    if (!erpdb->sql_commit())
        return seterrormsg(erpdb->errordb());
    return true;
}
