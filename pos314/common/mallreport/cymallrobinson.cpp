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
cymallrobinson::cymallrobinson (cylog* log, cyini* ini, cycgi* cgi) :
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
cymallrobinson::~cymallrobinson ()
{
    if (nullptr != _util)  {
        delete (_util);
        _util = nullptr;
    }
}
/********************************************************************
      FUNCTION:         insertReport ()
   DESCRIPTION:         Ayala insert into database
 ********************************************************************/
bool                    cymallrobinson::insertReport()
{
    CYDbSql*            insdb  = _ini->dbconnini("default");
    string              sql;
    stringstream        value;
    int                 j = 0;
    char                szValue[255];
    ////////////////////////////////
    j = _transList.size();
    for (int i=0; i<j;i++) {
        value.clear();
        string systransnum = "";
        systransnum = _transList.at(i)->systransnum;
        ///////////////////////////////////////
        // Build the insert statement
        sql  = "INSERT INTO  tg_mallreport_robinsons_transactions(";
        sql += "tenant_id,"; sprintf(szValue,"%s",_transList.at(i)->tenant_id.c_str()); value << insdb->sql_bind(1,szValue ) << ",";
        sql += "pos_terminal_no,"; sprintf(szValue,"%s",_transList.at(i)->pos_terminal_no.c_str()); value << insdb->sql_bind(2,szValue ) << ",";
        sql += "gross_sales,"; sprintf(szValue,"%.2f",_transList.at(i)->gross_sales); value << insdb->sql_bind(3,szValue ) << ",";
        sql += "total_tax_vat,"; sprintf(szValue,"%.2f",_transList.at(i)->total_tax_vat); value << insdb->sql_bind(4,szValue ) << ",";
        sql += "tot_amt_void,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_amt_void); value << insdb->sql_bind(5,szValue ) << ",";
        sql += "no_of_void,"; sprintf(szValue,"%d",_transList.at(i)->no_of_void); value << insdb->sql_bind(6,szValue ) << ",";
        sql += "tot_amt_disc,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_amt_disc); value << insdb->sql_bind(7,szValue ) << ",";
        sql += "no_of_disc,"; sprintf(szValue,"%d",_transList.at(i)->no_of_disc); value << insdb->sql_bind(8,szValue ) << ",";
        sql += "tot_amt_refund,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_amt_refund); value << insdb->sql_bind(9,szValue ) << ",";
        sql += "no_of_refund,"; sprintf(szValue,"%d",_transList.at(i)->no_of_refund); value << insdb->sql_bind(10,szValue ) << ",";
        sql += "other_negative_adj,"; sprintf(szValue,"%.2f",_transList.at(i)->other_negative_adj); value << insdb->sql_bind(11,szValue ) << ",";
        sql += "no_of_negative_adj,"; sprintf(szValue,"%d",_transList.at(i)->no_of_negative_adj); value << insdb->sql_bind(12,szValue ) << ",";
        sql += "tot_service_chrg,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_service_chrg); value << insdb->sql_bind(13,szValue ) << ",";
        sql += "prev_eod_ctr,"; sprintf(szValue,"%d",_transList.at(i)->prev_eod_ctr); value << insdb->sql_bind(14,szValue ) << ",";
        sql += "prev_acc_grand_tot,"; sprintf(szValue,"%.2f",_transList.at(i)->prev_acc_grand_tot); value << insdb->sql_bind(15,szValue ) << ",";
        sql += "current_eod_ctr,"; sprintf(szValue,"%d",_transList.at(i)->current_eod_ctr); value << insdb->sql_bind(16,szValue ) << ",";
        sql += "current_acc_grand_tot,"; sprintf(szValue,"%.2f",_transList.at(i)->current_acc_grand_tot); value << insdb->sql_bind(17,szValue ) << ",";
        sql += "sls_trans_date,"; sprintf(szValue,"%s",_transList.at(i)->sls_trans_date.c_str()); value << insdb->sql_bind(18,szValue ) << ",";
        sql += "novelty,"; sprintf(szValue,"%.2f",_transList.at(i)->novelty); value << insdb->sql_bind(19,szValue ) << ",";
        sql += "misc,"; sprintf(szValue,"%.2f",_transList.at(i)->misc); value << insdb->sql_bind(20,szValue ) << ",";
        sql += "local_tax,"; sprintf(szValue,"%.2f",_transList.at(i)->local_tax); value << insdb->sql_bind(21,szValue ) << ",";
        sql += "tot_credit_sls,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_credit_sls); value << insdb->sql_bind(22,szValue ) << ",";
        sql += "tot_credit_vat,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_credit_vat); value << insdb->sql_bind(23,szValue ) << ",";
        sql += "tot_nonvat_sls,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_nonvat_sls); value << insdb->sql_bind(24,szValue ) << ",";
        sql += "pharma_sls,"; sprintf(szValue,"%.2f",_transList.at(i)->pharma_sls); value << insdb->sql_bind(25,szValue ) << ",";
        sql += "non_pharma_sls,"; sprintf(szValue,"%.2f",_transList.at(i)->non_pharma_sls); value << insdb->sql_bind(26,szValue ) << ",";
        sql += "pwd_discount,"; sprintf(szValue,"%.2f",_transList.at(i)->pwd_discount); value << insdb->sql_bind(27,szValue ) << ",";
        sql += "gross_sls_fix_rate,"; sprintf(szValue,"%.2f",_transList.at(i)->gross_sls_fix_rate); value << insdb->sql_bind(28,szValue ) << ",";
        sql += "tot_reprinted_trx,"; sprintf(szValue,"%.2f",_transList.at(i)->tot_reprinted_trx); value << insdb->sql_bind(29,szValue ) << ",";
        sql += "no_of_reprint,"; sprintf(szValue,"%d",_transList.at(i)->no_of_reprint); value << insdb->sql_bind(30,szValue ) << ",";
        sql += "systransnum)"; sprintf(szValue,"%s",_transList.at(i)->systransnum.c_str()); value << insdb->sql_bind(31,szValue ) << ")";
        sql += "VALUES (";
        sql += value.str();

        if (!insdb->sql_only(sql,false)){
            _util->file_create("joelDebug.log",insdb->errordb().c_str());
            return seterrormsg(insdb->errordb());
        }
        if (!insdb->sql_commit())
            return seterrormsg(insdb->errordb());

    }
    return true;
}
/********************************************************************
      FUNCTION:         generateReport ()
   DESCRIPTION:         Ayala mallreport build per transactions
 ********************************************************************/
bool                    cymallrobinson::generateReport(Json::Value jsales)
{
    Json::Value jtrans,jdet,jpay,jsettings,jtender;
    bool        bSenior = true;
    double      dDiscAmt = 0.00;
    double      dPayAmt = 0.00;
    double      dChangeAmt = 0.00;
    int         j = 0;
    double      dSerCharge = 0.0;
    string      systransnum;
    ///////////////////////////////////////
    jtrans      = jsales;
    jdet        = jtrans["det_sales"];
    jpay        = jtrans["pay_sales"];
    jsettings   = jtrans["settings_transtype"];

    struct_transactions* trans = new struct_transactions;

    (!jtrans["systransnum"].empty()) ?
                systransnum = jtrans["systransnum"].asCString() :  systransnum = "0";
    trans->systransnum = systransnum;

    (!jtrans["logical_date"].empty()) ?
                trans->sls_trans_date = jtrans["logical_date"].asCString() :  trans->sls_trans_date= "0";


    (!jtrans["transaction_number"].empty()) ?
                trans->transaction_no = jtrans["transaction_number"].asCString() : trans->transaction_no  = "0";

    trans->total_tax_vat = jtrans["cytotal_amt_vat"].asDouble();
    trans->tot_nonvat_sls = jtrans["cytotal_net_exempt"].asDouble();
    dSerCharge = jtrans["cytotal_service_charge"].asDouble();
    trans->local_tax = 0.00;

    /////////////////////////////////
    // check if senior
    if (jsettings["is_sc"].empty()){
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
                    dDiscAmt += jdet[i]["item_discount_amount"].asDouble() * nQty;
                } else {
                    dDiscAmt += jdet[i]["item_discount_amount"].asDouble();
                }
            } else {
                dDiscAmt = 0.00;
            }
        } else {
            dDiscAmt = 0.00;
        }

    }

    if (dDiscAmt>0){
        if (bSenior) {
            trans->other_negative_adj = dDiscAmt;
            trans->no_of_negative_adj++;
        } else {
            trans->tot_amt_disc = dDiscAmt;
            trans->no_of_disc++;
        }
    }
    ////////////////////////////////
    // Service charge
    trans->tot_service_chrg = dSerCharge;
    // gross sales
    trans->gross_sales += jtrans["cytotal_amt_vat"].asDouble();
    trans->gross_sales +=jtrans["cytotal_net_vat"].asDouble();
    trans->gross_sales += jtrans["cytotal_net_zero"].asDouble();
    trans->gross_sales += jtrans["cytotal_net_exempt"].asDouble();
    trans->gross_sales += dDiscAmt;
    trans->gross_sales += dSerCharge;

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

        if (strcmp(jtender["is_credit_card"].asCString(),"1")==0) {
            trans->tot_credit_sls        += dPayAmt-dChangeAmt;
            trans->tot_credit_vat        += (dPayAmt-dChangeAmt)/1.12*.12;
        }
    }

    trans->tenant_id = "";
    (!jtrans["register_number"].empty()) ?
                trans->pos_terminal_no = jtrans["register_number"].asCString() : trans->pos_terminal_no;

    trans->tot_amt_void = 0.00;
    trans->no_of_void = 0;
    trans->tot_amt_refund = 0.00;
    trans->no_of_refund = 0;
    trans->prev_eod_ctr = 0.00;
    trans->prev_acc_grand_tot = 0.00;
    trans->current_eod_ctr = 0.00;
    trans->current_acc_grand_tot = 0.00;
    trans->novelty = 0.00;
    trans->misc = 0.00;
    trans->local_tax = 0.00;
    trans->pharma_sls = 0.00;
    trans->non_pharma_sls = 0.00;
    trans->pwd_discount = 0.00;
    trans->gross_sls_fix_rate = 0.00;
    trans->tot_reprinted_trx = 0.00;
    trans->no_of_reprint = 0.00;

    _transList.push_back(trans);

    return true;
}
