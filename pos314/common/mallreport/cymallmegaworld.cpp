/********************************************************************
          FILE:          cyw_mallreport.cpp
   DESCRIPTION:          Base class method(s)
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
#include "cyrestcommon.h"
#include "mallreport/cyw_mallreport.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
cymallmegaworld::cymallmegaworld (cylog* log, cyini* ini, cycgi* cgi) :
    cymallreport(log, ini, cgi)
{
    _ini = ini;
    _log = log;
    _cgi = cgi;
    _error = "";

    _util = nullptr;
    _util = new cyutility ();
}
/********************************************************************
      FUNCTION:         Destructor
   DESCRIPTION:         Class cleanup
 ********************************************************************/
cymallmegaworld::~cymallmegaworld ()
{
    if (nullptr != _util)  {
        delete (_util);
        _util = nullptr;
    }
}
/********************************************************************
      FUNCTION:         insertReport
   DESCRIPTION:         Insert data into mallreport table
 ********************************************************************/
bool                    cymallmegaworld::insertReport()
{
    CYDbSql*            insdb  = _ini->dbconnini("default");
    string              sql;
    stringstream        value;
    int                 j = 0;
    char                szValue[255];
    ////////////////////////////////
    j = _megawordList.size();
    for (int i=0; i<j;i++) {
        value.clear();
        ///////////////////////////////////////
        // Build the insert statement
        sql  = "INSERT INTO  tg_mallreport_megaworld_transactions (";
        sprintf(szValue,"%s",_megawordList.at(i)->systransnum.c_str());
        sql += "systransnum,"; value << insdb->sql_bind(1,szValue) << ",";
        sprintf(szValue,"%s",_megawordList.at(i)->transaction_date.c_str());
        sql += "transaction_date,"; value << insdb->sql_bind(2,szValue) << ",";
        sprintf(szValue,"%s",_megawordList.at(i)->transaction_no.c_str());
        sql += "transaction_no,"; value << insdb->sql_bind(3,szValue) << ",";
        sprintf(szValue,"%d",_megawordList.at(i)->terminal_no);
        sql += "terminal_no,"; value << insdb->sql_bind(4,szValue) << ",";
        sprintf(szValue,"%d",_megawordList.at(i)->trans_time);
        sql += "trans_time,"; value << insdb->sql_bind(5,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->gross_sales);
        sql += "gross_sales,"; value << insdb->sql_bind(6,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->vatable_sls);
        sql += "vatable_sls,"; value << insdb->sql_bind(7,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->nonvat_sls);
        sql += "nonvat_sls,"; value << insdb->sql_bind(8,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->vatexempt_sls);
        sql += "vatexempt_sls,"; value << insdb->sql_bind(9,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->senior_disc);
        sql += "senior_disc,"; value << insdb->sql_bind(10,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->other_disc);
        sql += "other_disc,"; value << insdb->sql_bind(11,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->refund_amt);
        sql += "refund_amt,"; value << insdb->sql_bind(12,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->vat_amt);
        sql += "vat_amt,"; value << insdb->sql_bind(13,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->service_charge);
        sql += "service_charge,"; value << insdb->sql_bind(14,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->net_sales_amt);
        sql += "net_sales_amt,"; value << insdb->sql_bind(15,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->cash_sls);
        sql += "cash_sls,"; value << insdb->sql_bind(16,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->credit_sls);
        sql += "credit_sls,"; value << insdb->sql_bind(17,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->gc_sls);
        sql += "gc_sls,"; value << insdb->sql_bind(18,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->void_amt);
        sql += "void_amt,"; value << insdb->sql_bind(19,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->customer_cnt);
        sql += "customer_cnt,"; value << insdb->sql_bind(20,szValue) << ",";
        sprintf(szValue,"%.2f",_megawordList.at(i)->control_no);
        sql += "control_no,"; value << insdb->sql_bind(21,szValue) << ",";
        sprintf(szValue,"%d",_megawordList.at(i)->transaction_cnt);
        sql += "transaction_cnt,"; value << insdb->sql_bind(22,szValue) << ",";
        sprintf(szValue,"%s",_megawordList.at(i)->sales_type.c_str());
        sql += "sales_type"; value << insdb->sql_bind(23,szValue) << ")";
        sql += ") VALUES(";
        sql += value.str();

        if (!insdb->sql_only(sql,false))
            return seterrormsg(insdb->errordb());
        if (!insdb->sql_commit())
            return seterrormsg(insdb->errordb());

    }

    return true;
}
/********************************************************************
      FUNCTION:         generateReport
   DESCRIPTION:         build data for insert
 ********************************************************************/
bool                    cymallmegaworld::generateReport(Json::Value jsales)
{
    Json::Value jtrans,jdet,jpay,jsettings,jtender;
    bool        bSenior = true;
    double      dDiscAmt;
    double      dPayAmt = 0.00;
    double      dChangeAmt = 0.00;
    int         j = 0;
    double      ntotQty = 0.0;
    double      dSerCharge = 0.0;
    string      systransnum;
    ///////////////////////////////////////
    jtrans      = jsales;
    jdet        = jtrans["det_sales"];
    jpay        = jtrans["pay_sales"];
    jsettings   = jtrans["settings_transtype"];

    struct_megaworld_daily* trans = new struct_megaworld_daily;

    (jtrans["systransnum"].asCString() !=nullptr) ?
    systransnum = jtrans["systransnum"].asCString() :  systransnum = "0";
    trans->systransnum = systransnum;
    //printf( "%s\n", systransnum.c_str());

    (jtrans["logical_date"].asCString() !=nullptr) ?
    trans->transaction_date = jtrans["logical_date"].asCString() :  trans->transaction_date = "0";

    (jtrans["transaction_time"].asCString() !=nullptr) ?
    trans->trans_time = atoi(jtrans["transaction_time"].asCString()) : trans->trans_time = 0;

    (jtrans["transaction_number"].asCString() !=nullptr) ?
    trans->transaction_no = jtrans["transaction_number"].asCString() : trans->transaction_no  = "0";

    trans->vat_amt = jtrans["cytotal_amt_vat"].asDouble();
    trans->vatable_sls =jtrans["cytotal_net_vat"].asDouble();
    trans->nonvat_sls = jtrans["cytotal_net_zero"].asDouble();
    trans->vatexempt_sls = jtrans["cytotal_net_exempt"].asDouble();
    dSerCharge = jtrans["cytotal_service_charge"].asDouble();

    /////////////////////////////////
    // check if senior
    if (jsettings["is_sc"].asCString() !=nullptr){
        if (strcmp(jsettings["is_sc"].asCString(),"1") == 0){
            bSenior = true;
        } else {
            bSenior = false;
        }
    } else {
        bSenior = false;
    }

    ////////////////////////////////
    // Compute discounts
    j = jdet.size();
    double nQty = 0;
    for (int i = 0; i < j;i++) {
        nQty = atof(jdet[i]["quantity"].asCString());
        if (jdet[i]["item_discount_amount"].asDouble() > 0){
            if (jdet[i]["quantity"].asCString() != nullptr ){
                if (nQty > 1){
                    dDiscAmt = jdet[i]["item_discount_amount"].asDouble() * nQty;
                } else {
                    dDiscAmt = jdet[i]["item_discount_amount"].asDouble();
                }
            } else {
                 dDiscAmt = 0.00;
            }
        } else {
            dDiscAmt = 0.00;
        }

        if (bSenior) {
            trans->senior_disc += dDiscAmt;
        } else {
            trans->other_disc += dDiscAmt;
        }
        ntotQty += atof(jdet[i]["quantity"].asCString());

    }

    ////////////////////////////////
    // Service charge
    trans->service_charge = dSerCharge;

    ////////////////////////////////
    // Payment Sales
    j = jpay.size();
    for(int i = 0; i < j; i++){
        jtender = jpay[i]["tendertype"];
        if (jpay[i]["payment_amount"].asCString() != nullptr
            && jpay[i]["change_amount"].asCString() != nullptr) {
            dPayAmt    = atof(jpay[i]["payment_amount"].asCString());
            dChangeAmt = atof(jpay[i]["change_amount"].asCString());
        } else {
            dPayAmt    = 0.00;
            dChangeAmt = 0.00;
        }

        if (strcmp(jtender["is_cash"].asCString(),"1")==0){
            trans->cash_sls        += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_credit_card"].asCString(),"1")==0) {
            trans->credit_sls        += dPayAmt-dChangeAmt;
        }  else {
          trans->gc_sls          += dPayAmt-dChangeAmt;
        }

    }

    trans->refund_amt +=0.00;
    _megawordList.push_back(trans);

    return true;
}
