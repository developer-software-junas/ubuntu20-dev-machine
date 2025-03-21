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
cymallayala::cymallayala (cylog* log, cyini* ini, cycgi* cgi) :
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
cymallayala::~cymallayala ()
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
bool                    cymallayala::insertReport()
{
    CYDbSql*            insdb  = _ini->dbconnini("default");
    CYDbSql*            insdbItm  = _ini->dbconnini("default");
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
        sql  = "INSERT INTO  tg_mallreport_ayala_transactions";
        sql += "(systransnum,";	value << insdb->sql_bind(1,systransnum) << ",";
        sql += "cccode,";		value << insdb->sql_bind(2,"COMPANY" ) << ",";
        sql += "merchant_name,"; value << insdb->sql_bind(3, "JOEL" ) << ",";
        sql += "trn_date,";	     value << insdb->sql_bind(4,_transList.at(i)->trn_date ) << ",";
        sql += "no_trn,";	value << insdb->sql_bind(5,"0" ) << ",";
        sql += "cdate,";	value << insdb->sql_bind(6,_transList.at(i)->trn_date) << ",";
        sql += "trn_time,";	value << insdb->sql_bind(7,_transList.at(i)->trn_time) << ",";
        sql += "ter_no,";	value << insdb->sql_bind(8, "0" ) << ",";
        sql += "transaction_no,"; value << insdb->sql_bind(9,_transList.at(i)->transaction_no) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->gross_sls);
        sql += "gross_sls,";	value << insdb->sql_bind(10,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->vat_amnt);
        sql += "vat_amnt,";	value << insdb->sql_bind(11,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->vatable_sls);
        sql += "vatable_sls,";	value << insdb->sql_bind(12,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->nonvat_sls);
        sql += "nonvat_sls,";	value << insdb->sql_bind(13,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->vatexempt_sls);
        sql += "vatexempt_sls,";	value << insdb->sql_bind(14,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->vatexempt_amnt);
        sql += "vatexempt_amnt,";	value << insdb->sql_bind(15,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->local_tax);
        sql += "local_tax,";	value << insdb->sql_bind(16,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->pwd_disc);
        sql += "pwd_disc,";	value << insdb->sql_bind(17,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->snrcit_disc);
        sql += "snrcit_disc,";	value << insdb->sql_bind(18,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->emplo_disc);
        sql += "emplo_disc,";	value << insdb->sql_bind(19, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->ayala_disc);
        sql += "ayala_disc,";	value << insdb->sql_bind(20, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->store_disc);
        sql += "store_disc,";	value << insdb->sql_bind(21, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->other_disc);
        sql += "other_disc,";	value << insdb->sql_bind(22, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->refund_amt);
        sql += "refund_amt,";	value << insdb->sql_bind(23, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->schrge_amt);
        sql += "schrge_amt,";	value << insdb->sql_bind(24, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->other_schr);
        sql += "other_schr,";	value << insdb->sql_bind(25, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->cash_sls);
        sql += "cash_sls,";	value << insdb->sql_bind(26, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->card_sls);
        sql += "card_sls,";	value << insdb->sql_bind(27, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->epay_sls);
        sql += "epay_sls,";	value << insdb->sql_bind(28, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->dcard_sls);
        sql += "dcard_sls,";	value << insdb->sql_bind(29, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->othersl_sls);
        sql += "othersl_sls,";	value << insdb->sql_bind(30, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->check_sls);
        sql += "check_sls,";	value << insdb->sql_bind(31, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->gc_sls);
        sql += "gc_sls,";	value << insdb->sql_bind(32, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->mastercard_sls);
        sql += "mastercard_sls,";	value << insdb->sql_bind(33, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->visa_sls);
        sql += "visa_sls,";	value << insdb->sql_bind(34,szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->amex_sls);
        sql += "amex_sls,";	value << insdb->sql_bind(35, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->diners_sls);
        sql += "diners_sls,";	value << insdb->sql_bind(36, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->jcb_sls);
        sql += "jcb_sls,";	value << insdb->sql_bind(37, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->gcash_sls);
        sql += "gcash_sls,";	value << insdb->sql_bind(38, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->paymaya_sls);
        sql += "paymaya_sls,";	value << insdb->sql_bind(39, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->alipay_sls);
        sql += "alipay_sls,";	value << insdb->sql_bind(40, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->wechat_sls);
        sql += "wechat_sls,";	value << insdb->sql_bind(41, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->grab_sls);
        sql += "grab_sls,";	value << insdb->sql_bind(42, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->foodpanda_sls);
        sql += "foodpanda_sls,";	value << insdb->sql_bind(43, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->masterdebit_sls);
        sql += "masterdebit_sls,";	value << insdb->sql_bind(44, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->visadebit_sls);
        sql += "visadebit_sls,";	value << insdb->sql_bind(45, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->paypal_sls);
        sql += "paypal_sls,";	value << insdb->sql_bind(46, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->online_sls);
        sql += "online_sls,";	value << insdb->sql_bind(47, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales);
        sql += "open_sales,";	value << insdb->sql_bind(48, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_2);
        sql += "open_sales_2,";	value << insdb->sql_bind(49, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_3);
        sql += "open_sales_3,";	value << insdb->sql_bind(50, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_4);
        sql += "open_sales_4,";	value << insdb->sql_bind(51, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_5);
        sql += "open_sales_5,";	value << insdb->sql_bind(52, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_6);
        sql += "open_sales_6,";	value << insdb->sql_bind(53, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_7);
        sql += "open_sales_7,";	value << insdb->sql_bind(54, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_8);
        sql += "open_sales_8,";	value << insdb->sql_bind(55, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_9);
        sql += "open_sales_9,";	value << insdb->sql_bind(56, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_10);
        sql += "open_sales_10,";	value << insdb->sql_bind(57, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->open_sales_11);
        sql += "open_sales_11,";	value << insdb->sql_bind(58, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->gc_excess);
        sql += "gc_excess,";	value << insdb->sql_bind(59, szValue) << ",";
        sql += "mobile_no,";	value << insdb->sql_bind(60, _transList.at(i)->mobile_no) << ",";
        sprintf(szValue,"%d",_transList.at(i)->no_cust);
        sql += "no_cust,";	value << insdb->sql_bind(61, szValue) << ",";
        sql += "trn_type,";	value << insdb->sql_bind(62, _transList.at(i)->trn_type) << ",";
        sql += "sls_flag,";	value << insdb->sql_bind(63, _transList.at(i)->sls_flag) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->vat_pct);
        sql += "vat_pct,";	value << insdb->sql_bind(64, szValue) << ",";
        sprintf(szValue,"%.2f",_transList.at(i)->qty_sld);
        sql += "qty_sld)";	value << insdb->sql_bind(65, szValue) << ")";
        sql += "VALUES (";
        sql += value.str();

        if (!insdb->sql_only(sql,false))
            return seterrormsg(insdb->errordb());
        if (!insdb->sql_commit())
            return seterrormsg(insdb->errordb());
        int x = _transList.at(i)->listItem.size();
        /////////////////////////////////////
        // Insert items

        for(int y=0; y < x; y++) {
            stringstream valueItm;
            valueItm.clear();
            insdbItm->sql_reset();
            insdbItm->sql_bind_reset();
            sql = "Insert into tg_mallreport_ayala_trxitems (";
            sprintf(szValue,"%s",systransnum.c_str());
            sql += "systransnum,"; valueItm << insdbItm->sql_bind(1,szValue) << ",";
            sprintf(szValue,"%.2f",_transList.at(i)->listItem.at(y)->item_seq);
            sql += "item_seq,"; valueItm << insdbItm->sql_bind(2,szValue) << ",";
            sprintf(szValue,"%.2f",_transList.at(i)->listItem.at(y)->qty);
            sql += "qty,"; valueItm <<  insdbItm->sql_bind(3,szValue) << ",";
            sprintf(szValue,"%s",_transList.at(i)->listItem.at(y)->item_code.c_str());
            sql += "itemcode,"; valueItm << insdbItm->sql_bind(4,szValue) << ",";
            sprintf(szValue,"%.2f",_transList.at(i)->listItem.at(y)->price);
            sql += "price,"; valueItm << insdbItm->sql_bind(5,szValue) << ",";
            sprintf(szValue,"%.2f",_transList.at(i)->listItem.at(y)->ldisc);
            sql += "ldisc) "; valueItm <<  insdbItm->sql_bind(6,szValue) << ")";
            sql += "values (";
            sql += valueItm.str();
            _util->file_create("debugTest.log",sql.c_str());

            if (!insdbItm->sql_only(sql,false))
                return seterrormsg(insdb->errordb());
            if (!insdbItm->sql_commit())
                return seterrormsg(insdb->errordb());

        }


    }
    return true;
}
/********************************************************************
      FUNCTION:         generateReport ()
   DESCRIPTION:         Ayala mallreport build per transactions
 ********************************************************************/
bool                    cymallayala::generateReport(Json::Value jsales)
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

    struct_transactions* trans = new struct_transactions;

    (jtrans["systransnum"].asCString() !=nullptr) ?
    systransnum = jtrans["systransnum"].asCString() :  systransnum = "0";
    trans->systransnum = systransnum;

    (jtrans["logical_date"].asCString() !=nullptr) ?
    trans->trn_date = jtrans["logical_date"].asCString() :  trans->trn_date = "0";

    (jtrans["transaction_time"].asCString() !=nullptr) ?
    trans->trn_time = jtrans["transaction_time"].asCString() : trans->trn_time = "0";

    (jtrans["transaction_number"].asCString() !=nullptr) ?
    trans->transaction_no = jtrans["transaction_number"].asCString() : trans->transaction_no  = "0";

    trans->vat_amnt = jtrans["cytotal_amt_vat"].asDouble();
    trans->vatable_sls =jtrans["cytotal_net_vat"].asDouble();
    trans->nonvat_sls = jtrans["cytotal_net_zero"].asDouble();
    trans->vatexempt_sls = jtrans["cytotal_net_exempt"].asDouble();
    trans->vatexempt_amnt = jtrans["cytotal_amt_exempt"].asDouble();
    dSerCharge = jtrans["cytotal_service_charge"].asDouble();
    trans->local_tax = 0.00;

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
            trans->snrcit_disc += dDiscAmt;
        } else {
            trans->other_disc += dDiscAmt;
        }
        trans->emplo_disc += 0;
        trans->ayala_disc += 0;
        trans->store_disc += 0;
        trans->pwd_disc += 0;
        ntotQty += atof(jdet[i]["quantity"].asCString());

        struct_trx_items* itemList = new struct_trx_items;
        string itemCode = jdet[i]["item_code"].asCString();
        double retailPrice = atof(jdet[i]["retail_price"].asCString());
        double itemSeq = atof(jdet[i]["item_seq"].asCString());
        itemList->qty = nQty;
        itemList->item_code = itemCode;
        itemList->price = retailPrice;
        itemList->ldisc = dDiscAmt;
        itemList->item_seq = itemSeq;
        trans->listItem.push_back(itemList);
    }

    ////////////////////////////////
    // Service charge
    trans->other_schr += 0.00;
    trans->schrge_amt = dSerCharge;

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
            trans->card_sls        += dPayAmt-dChangeAmt;
            trans->mastercard_sls  += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_debit_card"].asCString(),"1")==0) {
            trans->dcard_sls       += dPayAmt-dChangeAmt;
            trans->masterdebit_sls += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_check"].asCString(),"1")==0) {
            trans->check_sls       += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_gc"].asCString(),"1")==0) {
            trans->gc_sls          += dPayAmt-dChangeAmt;
        } else {
          trans->othersl_sls       += dPayAmt-dChangeAmt;
        }
        trans->epay_sls            +=0.00;
    }

    trans->visa_sls += 0.00;
    trans->amex_sls += 0.00;
    trans->diners_sls += 0.00;
    trans->jcb_sls += 0.00;
    trans->gcash_sls += 0.00;
    trans->paymaya_sls += 0.00;
    trans->alipay_sls += 0.00;
    trans->wechat_sls += 0.00;
    trans->grab_sls += 0.00;
    trans->foodpanda_sls += 0.00;
    trans->visadebit_sls += 0.00;
    trans->paypal_sls += 0.00;
    trans->online_sls += 0.00;
    trans->open_sales += 0.00;
    trans->open_sales_2 += 0.00;
    trans->open_sales_3 += 0.00;
    trans->open_sales_4 += 0.00;
    trans->open_sales_5 += 0.00;
    trans->open_sales_6 += 0.00;
    trans->open_sales_7 += 0.00;
    trans->open_sales_8 += 0.00;
    trans->open_sales_9 += 0.00;
    trans->open_sales_10 += 0.00;
    trans->open_sales_11 += 0.00;
    trans->gc_excess += 0.00;
    trans->mobile_no = "NA";
    trans->no_cust += 1;
    trans->trn_type = "D";
    trans->sls_flag = "S";
    trans->vat_pct = 1.12;
    trans->qty_sld = ntotQty;

    trans->gross_sls  +=0.00;
    trans->refund_amt +=0.00;

    _transList.push_back(trans);

    return true;
}
