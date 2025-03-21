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
cymallsmsia::cymallsmsia (cylog* log, cyini* ini, cycgi* cgi) :
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
cymallsmsia::~cymallsmsia ()
{
    if (nullptr != _util)  {
        delete (_util);
        _util = nullptr;
    }
}
/********************************************************************
      FUNCTION:         insertReport ()
   DESCRIPTION:         smsia insert into database
 ********************************************************************/
bool                    cymallsmsia::insertReport()
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
        insdb->sql_reset();
        insdb->sql_bind_reset();
        sql  = "INSERT INTO  tg_mallreport_smsia_transactions (";
        sql +="systransnum,"; sprintf(szValue,"%s",_transList.at(i)->systransnum.c_str()); value << insdb->sql_bind(1,szValue) << ",";
        sql +="bill_num,"; sprintf(szValue,"%d",_transList.at(i)->bill_num); value << insdb->sql_bind(2,szValue) << ",";
        sql +="business_day,"; sprintf(szValue,"%s",_transList.at(i)->business_day.c_str()); value << insdb->sql_bind(3,szValue) << ",";
        sql +="check_open,"; sprintf(szValue,"%s",_transList.at(i)->check_open.c_str()); value << insdb->sql_bind(4,szValue) << ",";
        sql +="check_close,"; sprintf(szValue,"%s",_transList.at(i)->check_close.c_str()); value << insdb->sql_bind(5,szValue) << ",";
        sql +="sales_type,"; sprintf(szValue,"%s",_transList.at(i)->sales_type.c_str()); value << insdb->sql_bind(6,szValue) << ",";
        sql +="transaction_type,"; sprintf(szValue,"%s",_transList.at(i)->transaction_type.c_str()); value << insdb->sql_bind(7,szValue) << ",";
        sql +="is_void,"; sprintf(szValue,"%d",_transList.at(i)->is_void); value << insdb->sql_bind(8,szValue) << ",";
        sql +="void_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->void_amount); value << insdb->sql_bind(9,szValue) << ",";
        sql +="refund,"; sprintf(szValue,"%d",_transList.at(i)->refund); value << insdb->sql_bind(10,szValue) << ",";
        sql +="refund_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->refund_amount); value << insdb->sql_bind(11,szValue) << ",";
        sql +="guest_count,"; sprintf(szValue,"%d",_transList.at(i)->guest_count); value << insdb->sql_bind(12,szValue) << ",";
        sql +="guest_count_senior,"; sprintf(szValue,"%d",_transList.at(i)->guest_count_senior); value << insdb->sql_bind(13,szValue) << ",";
        sql +="guest_count_pwd,"; sprintf(szValue,"%d",_transList.at(i)->guest_count_pwd); value << insdb->sql_bind(14,szValue) << ",";
        sql +="gross_sales_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->gross_sales_amount); value << insdb->sql_bind(15,szValue) << ",";
        sql +="net_salesamount,"; sprintf(szValue,"%.2f",_transList.at(i)->net_salesamount); value << insdb->sql_bind(16,szValue) << ",";
        sql +="total_tax,"; sprintf(szValue,"%.2f",_transList.at(i)->total_tax); value << insdb->sql_bind(17,szValue) << ",";
        sql +="other_local_tax,"; sprintf(szValue,"%.2f",_transList.at(i)->other_local_tax); value << insdb->sql_bind(18,szValue) << ",";
        sql +="total_service_charge,"; sprintf(szValue,"%.2f",_transList.at(i)->total_service_charge); value << insdb->sql_bind(19,szValue) << ",";
        sql +="total_tip,"; sprintf(szValue,"%.2f",_transList.at(i)->total_tip); value << insdb->sql_bind(20,szValue) << ",";
        sql +="total_discount,"; sprintf(szValue,"%.2f",_transList.at(i)->total_discount); value << insdb->sql_bind(21,szValue) << ",";
        sql +="less_tax_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->less_tax_amount); value << insdb->sql_bind(22,szValue) << ",";
        sql +="tax_exempt_sales,"; sprintf(szValue,"%.2f",_transList.at(i)->tax_exempt_sales); value << insdb->sql_bind(23,szValue) << ",";
        sql +="regular_other_discount,"; sprintf(szValue,"%s",_transList.at(i)->regular_other_discount.c_str()); value << insdb->sql_bind(24,szValue) << ",";
        sql +="regular_other_discount_amnt,"; sprintf(szValue,"%.2f",_transList.at(i)->regular_other_discount_amnt); value << insdb->sql_bind(25,szValue) << ",";
        sql +="employee_discount_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->employee_discount_amount); value << insdb->sql_bind(26,szValue) << ",";
        sql +="senior_citizen_discount_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->senior_citizen_discount_amt); value << insdb->sql_bind(27,szValue) << ",";
        sql +="vip_discount_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->vip_discount_amt); value << insdb->sql_bind(28,szValue) << ",";
        sql +="pwd_discount_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->pwd_discount_amt); value << insdb->sql_bind(29,szValue) << ",";
        sql +="NCAMOV_disc_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->NCAMOV_disc_amount); value << insdb->sql_bind(30,szValue) << ",";
        sql +="SMAC_disc_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->SMAC_disc_amount); value << insdb->sql_bind(31,szValue) << ",";
        sql +="online_deals_discount_name,"; sprintf(szValue,"%s",_transList.at(i)->online_deals_discount_name.c_str()); value << insdb->sql_bind(32,szValue) << ",";
        sql +="online_deals_discount_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->online_deals_discount_amount); value << insdb->sql_bind(33,szValue) << ",";
        sql +="discount_field_1_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_1_name.c_str()); value << insdb->sql_bind(34,szValue) << ",";
        sql +="discount_field_2_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_2_name.c_str()); value << insdb->sql_bind(35,szValue) << ",";
        sql +="discount_field_3_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_3_name.c_str()); value << insdb->sql_bind(36,szValue) << ",";
        sql +="discount_field_4_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_4_name.c_str()); value << insdb->sql_bind(37,szValue) << ",";
        sql +="discount_field_5_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_5_name.c_str()); value << insdb->sql_bind(38,szValue) << ",";
        sql +="discount_field_6_name,"; sprintf(szValue,"%s",_transList.at(i)->discount_field_6_name.c_str()); value << insdb->sql_bind(39,szValue) << ",";
        sql +="discount_field_1_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_1_amt); value << insdb->sql_bind(40,szValue) << ",";
        sql +="discount_field_2_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_2_amt); value << insdb->sql_bind(41,szValue) << ",";
        sql +="discount_field_3_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_3_amt); value << insdb->sql_bind(42,szValue) << ",";
        sql +="discount_field_4_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_4_amt); value << insdb->sql_bind(43,szValue) << ",";
        sql +="discount_field_5_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_5_amt); value << insdb->sql_bind(44,szValue) << ",";
        sql +="discount_field_6_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->discount_field_6_amt); value << insdb->sql_bind(45,szValue) << ",";
        sql +="payment_type_1,"; sprintf(szValue,"%s",_transList.at(i)->payment_type_1.c_str()); value << insdb->sql_bind(46,szValue) << ",";
        sql +="payment_amount_1,"; sprintf(szValue,"%.2f",_transList.at(i)->payment_amount_1); value << insdb->sql_bind(47,szValue) << ",";
        sql +="payment_type_2,"; sprintf(szValue,"%s",_transList.at(i)->payment_type_2.c_str()); value << insdb->sql_bind(48,szValue) << ",";
        sql +="payment_amount_2,"; sprintf(szValue,"%.2f",_transList.at(i)->payment_amount_2); value << insdb->sql_bind(49,szValue) << ",";
        sql +="payment_type_3,"; sprintf(szValue,"%s",_transList.at(i)->payment_type_3.c_str()); value << insdb->sql_bind(50,szValue) << ",";
        sql +="payment_amount_3,"; sprintf(szValue,"%.2f",_transList.at(i)->payment_amount_3); value << insdb->sql_bind(51,szValue) << ",";
        sql +="total_cash_sales_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_cash_sales_amt); value << insdb->sql_bind(52,szValue) << ",";
        sql +="total_gift_cheque_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_gift_cheque_amt); value << insdb->sql_bind(53,szValue) << ",";
        sql +="total_debit_card__amount,"; sprintf(szValue,"%.2f",_transList.at(i)->total_debit_card__amount); value << insdb->sql_bind(54,szValue) << ",";
        sql +="total_ewallet_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_ewallet_amt); value << insdb->sql_bind(55,szValue) << ",";
        sql +="total_other_tender_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->total_other_tender_amount); value << insdb->sql_bind(56,szValue) << ",";
        sql +="total_mastercard_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->total_mastercard_amount); value << insdb->sql_bind(57,szValue) << ",";
        sql +="total_visa_amount,"; sprintf(szValue,"%.2f",_transList.at(i)->total_visa_amount); value << insdb->sql_bind(58,szValue) << ",";
        sql +="total_american_exp_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_american_exp_amt); value << insdb->sql_bind(59,szValue) << ",";
        sql +="total_diners_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_diners_amt); value << insdb->sql_bind(60,szValue) << ",";
        sql +="total_jcb_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_jcb_amt); value << insdb->sql_bind(61,szValue) << ",";
        sql +="total_other_credit_amt,"; sprintf(szValue,"%.2f",_transList.at(i)->total_other_credit_amt); value << insdb->sql_bind(62,szValue) << ",";
        sql +="terminal_number,"; sprintf(szValue,"%s",_transList.at(i)->terminal_number.c_str()); value << insdb->sql_bind(63,szValue) << ",";
        sql +="serial_number) values ("; sprintf(szValue,"%s",_transList.at(i)->serial_number.c_str()); value << insdb->sql_bind(64,szValue) << ")";
        sql += value.str();

        if (!insdb->sql_only(sql,false)){
            return seterrormsg(insdb->errordb());
        }
        if (!insdb->sql_commit())
            return seterrormsg(insdb->errordb());
    }

    ////////////////////////////////
    // Item list for sia
    j = _itemList.size();
    for (int i=0; i<j;i++) {
        value.str("");
        string systransnum = "";
        systransnum = _itemList.at(i)->systransnum;
        ///////////////////////////////////////
        // Build the insert statement
        insdbItm->sql_reset();
        insdbItm->sql_bind_reset();
        sql  = "INSERT INTO  tg_mallreport_smsia_trxitems (";
        sql +="systransnum,"; sprintf(szValue,"%s",_itemList.at(i)->systransnum.c_str()); value << insdbItm->sql_bind(1,szValue) << ",";
        sql +="bill_num,"; sprintf(szValue,"%d",_itemList.at(i)->bill_num); value << insdbItm->sql_bind(2,szValue) << ",";
        sql +="item_id,"; sprintf(szValue,"%s",_itemList.at(i)->item_id.c_str()); value << insdbItm->sql_bind(3,szValue) << ",";
        sql +="item_name,"; sprintf(szValue,"%s",_itemList.at(i)->item_name.c_str()); value << insdbItm->sql_bind(4,szValue) << ",";
        sql +="item_parent_category,"; sprintf(szValue,"%s",_itemList.at(i)->item_parent_category.c_str()); value << insdbItm->sql_bind(5,szValue) << ",";
        sql +="item_category,"; sprintf(szValue,"%s",_itemList.at(i)->item_category.c_str()); value << insdbItm->sql_bind(6,szValue) << ",";
        sql +="item_sub_category,"; sprintf(szValue,"%s",_itemList.at(i)->item_sub_category.c_str()); value << insdbItm->sql_bind(7,szValue) << ",";
        sql +="item_quantity,"; sprintf(szValue,"%.2f",_itemList.at(i)->item_quantity); value << insdbItm->sql_bind(8,szValue) << ",";
        sql +="transaction_item_price,"; sprintf(szValue,"%.2f",_itemList.at(i)->transaction_item_price); value << insdbItm->sql_bind(9,szValue) << ",";
        sql +="menu_item_price,"; sprintf(szValue,"%.2f",_itemList.at(i)->menu_item_price); value << insdbItm->sql_bind(10,szValue) << ",";
        sql +="discount_code,"; sprintf(szValue,"%s",_itemList.at(i)->discount_code.c_str()); value << insdbItm->sql_bind(11,szValue) << ",";
        sql +="discount_amount,"; sprintf(szValue,"%.2f",_itemList.at(i)->discount_amount); value << insdbItm->sql_bind(12,szValue) << ",";
        sql +="modifier_1_name,"; sprintf(szValue,"%s",_itemList.at(i)->modifier_1_name.c_str()); value << insdbItm->sql_bind(13,szValue) << ",";
        sql +="modifier_1_quantity,"; sprintf(szValue,"%.2f",_itemList.at(i)->modifier_1_quantity); value << insdbItm->sql_bind(14,szValue) << ",";
        sql +="modifier_2_name,"; sprintf(szValue,"%s",_itemList.at(i)->modifier_2_name.c_str()); value << insdbItm->sql_bind(15,szValue) << ",";
        sql +="modifier_2_quantity,"; sprintf(szValue,"%.2f",_itemList.at(i)->modifier_2_quantity); value << insdbItm->sql_bind(16,szValue) << ",";
        sql +="is_void,"; sprintf(szValue,"%d",_itemList.at(i)->is_void); value << insdbItm->sql_bind(17,szValue) << ",";
        sql +="void_amount,"; sprintf(szValue,"%.2f",_itemList.at(i)->void_amount); value << insdbItm->sql_bind(18,szValue) << ",";
        sql +="refund,"; sprintf(szValue,"%d",_itemList.at(i)->refund); value << insdbItm->sql_bind(19,szValue) << ",";
        sql +="refund_amount ) values ("; sprintf(szValue,"%.2f",_itemList.at(i)->refund_amount); value << insdbItm->sql_bind(20,szValue) << ")";
        sql += value.str();

        if (!insdbItm->sql_only(sql,false)){
            _util->file_create("joelDebug.log",insdbItm->errordb());
            return seterrormsg(insdbItm->errordb());
        }
        if (!insdbItm->sql_commit())
            return seterrormsg(insdbItm->errordb());
    }

    return true;
}
/********************************************************************
      FUNCTION:         generateReport ()
   DESCRIPTION:         smsia mallreport build per transactions
 ********************************************************************/
bool                    cymallsmsia::generateReport(Json::Value jsales)
{
    Json::Value jtrans,jdet,jpay,jsettings,jtender;
    bool        bSenior = true;
    double      dDiscAmt = 0.00;
    double      dPayAmt = 0.00;
    double      dChangeAmt = 0.00;
    int         j = 0;
    double      ntotQty = 0.0;
    double      dSerCharge = 0.0;
    double      tmpTot_disc = 0.0;
    double      tmpVat_amnt = 0.0;
    double      tmpVatable_sls = 0.0;
    double      tmpNonvat_sls = 0.0;
    double      tmpVatexempt_sls = 0.0;
    string      systransnum;
    int         ntransNum;
    ///////////////////////////////////////
    jtrans      = jsales;
    jdet        = jtrans["det_sales"];
    jpay        = jtrans["pay_sales"];
    jsettings   = jtrans["settings_transtype"];

    struct_transactions* trans = new struct_transactions;

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

    (jtrans["systransnum"].asCString() !=nullptr) ?
                systransnum = jtrans["systransnum"].asCString() :  systransnum = "0";
    trans->systransnum = systransnum;

    (jtrans["transaction_number"].asCString() !=nullptr) ?
                ntransNum = atoi(jtrans["transaction_number"].asCString()) : ntransNum  = 0;
    trans->bill_num  = ntransNum;

    (jtrans["logical_date"].asCString() !=nullptr) ?
                trans->business_day = jtrans["logical_date"].asCString() :  trans->business_day = "0";
    (jtrans["transaction_time"].asCString() !=nullptr) ?
                trans->check_open = jtrans["transaction_time"].asCString() : trans->check_open= "0";
    (jtrans["transaction_time"].asCString() !=nullptr) ?
                trans->check_close = jtrans["transaction_time"].asCString() : trans->check_close = "0";

    trans->sales_type = "SMS01";
    trans->transaction_type = "";
    trans->is_void = 0;
    trans->void_amount = 0.00;
    trans->refund = 0;
    trans->refund_amount = 0.00;

    if (!bSenior) {
        trans->guest_count = 1;
    } else {
        trans->guest_count = 0;
    }

    if (bSenior) {
        trans->guest_count_senior = 1;
    } else {
        trans->guest_count_senior = 0;
    }

    tmpVat_amnt = jtrans["cytotal_amt_vat"].asDouble();
    tmpVatable_sls =jtrans["cytotal_net_vat"].asDouble();
    tmpNonvat_sls = jtrans["cytotal_net_zero"].asDouble();
    tmpVatexempt_sls = jtrans["cytotal_net_exempt"].asDouble();
    dSerCharge = jtrans["cytotal_service_charge"].asDouble();

    trans->guest_count_pwd = 0;
    trans->gross_sales_amount =  jtrans["cytotal_gross_amount"].asDouble();
    trans->net_salesamount = tmpNonvat_sls + tmpVatable_sls + tmpVatexempt_sls;
    trans->total_tax = tmpVat_amnt;
    trans->other_local_tax = 0.00;
    trans->total_service_charge = dSerCharge;
    trans->total_tip = 0.00;

    ////////////////////////////////
    // read per item thinincludes
    // * Compute discounts
    Json::Value   jdisc;
    double        nQty = 0;
    int           nDiscCnt = 0;
    string        discCode;
    stringstream  dd;
    dd.clear();
    j = jdet.size();
    for (int i = 0; i < j;i++) {
        if (!jdet[i]["det_discount"].empty())
        {
            jdisc = jdet[i]["det_discount"];
            nQty  = atof(jdet[i]["quantity"].asCString());

            if (jdisc["discount_value"].asCString() != nullptr){
                if (jdet[i]["quantity"].asCString() != nullptr ){
                    if (nQty > 1){
                        dDiscAmt = atof(jdisc["discount_value"].asCString()) * nQty;
                    } else {
                        dDiscAmt = atof(jdisc["discount_value"].asCString());
                    }
                    if (nDiscCnt > 0) {
                        dd << "::";
                    }
                    discCode = jdisc["discount_code"].asCString();
                    dd << discCode;
                    dd << "=";
                    dd << jdisc["description"].asCString();
                    nDiscCnt++;
                } else {
                    dDiscAmt = 0.00;
                }
            } else {
                dDiscAmt = 0.00;
            }
        }

        if (bSenior) {
            trans->senior_citizen_discount_amt += dDiscAmt;
        } else {
            trans->regular_other_discount_amnt += dDiscAmt;
        }
        trans->employee_discount_amount += 0.00;
        trans->vip_discount_amt += 0.00;
        trans->pwd_discount_amt += 0.00;
        trans->NCAMOV_disc_amount += 0.00;
        trans->SMAC_disc_amount += 0.00;
        trans->online_deals_discount_name += 0.00;
        trans->online_deals_discount_amount += 0.00;
        trans->discount_field_1_name += "";
        trans->discount_field_2_name += "";
        trans->discount_field_3_name += "";
        trans->discount_field_4_name += "";
        trans->discount_field_5_name += "";
        trans->discount_field_6_name += "";
        trans->discount_field_1_amt += 0.00;
        trans->discount_field_2_amt += 0.00;
        trans->discount_field_3_amt += 0.00;
        trans->discount_field_4_amt += 0.00;
        trans->discount_field_5_amt += 0.00;
        trans->discount_field_6_amt += 0.00;

        tmpTot_disc += dDiscAmt;
        ntotQty += atof(jdet[i]["quantity"].asCString());
        double detRetailPrice = 0.00;
        struct_trx_items* itemList = new struct_trx_items;
        string detItemCode    = jdet[i]["item_code"].asCString();
        string detItemDesc    = jdet[i]["description"].asCString();
        detRetailPrice        = atof(jdet[i]["retail_price"].asCString());
        string detItemCat     = "";
        string detItemSubCat  = "";
        string detItemClass   = "";
        if (!jdet[i]["detail_product_header"].empty()) {
            Json::Value jdetProd = jdet[i]["detail_product_header"];

            detItemCat     = jdetProd["category_cd"].asCString();
            detItemSubCat  = jdetProd["subcat_cd"].asCString();
            detItemClass   = jdetProd["class_cd"].asCString();

            itemList->systransnum = systransnum;
            itemList->bill_num	 = ntransNum;
            itemList->item_id = detItemCode;
            itemList->item_name = detItemDesc;
            itemList->item_parent_category = detItemCat;
            itemList->item_category  = detItemSubCat;
            itemList->item_sub_category = detItemClass;
            itemList->item_quantity = nQty;
            itemList->transaction_item_price = detRetailPrice;
            itemList->menu_item_price = detRetailPrice;
            itemList->discount_code = discCode;
            itemList->discount_amount = dDiscAmt;
            itemList->modifier_1_name = "";
            itemList->modifier_1_quantity = 0.00;
            itemList->modifier_2_name = "";
            itemList->modifier_2_quantity = 0.00;
            itemList->is_void = 0;
            itemList->void_amount = 0.00;
            itemList->refund = 0;
            itemList->refund_amount = 0.00;
            _itemList.push_back(itemList);
        }

    }

    trans->total_discount = tmpTot_disc;
    trans->less_tax_amount = 0.00;
    trans->tax_exempt_sales = tmpVatexempt_sls;
    trans->regular_other_discount = dd.str();


    ////////////////////////////////
    // Payment Sales
    j = jpay.size();
    string tmpPayDesc;
    for(int i = 0; i < j; i++){
        jtender = jpay[i]["tendertype"];
        tmpPayDesc = jpay[i]["pay_type_desc"].asCString();
        if (jpay[i]["payment_amount"].asCString() != nullptr
                && jpay[i]["change_amount"].asCString() != nullptr) {
            dPayAmt    = atof(jpay[i]["payment_amount"].asCString());
            dChangeAmt = atof(jpay[i]["change_amount"].asCString());
        } else {
            dPayAmt    = 0.00;
            dChangeAmt = 0.00;
        }

        if (strcmp(jtender["is_cash"].asCString(),"1")==0){
            trans->total_cash_sales_amt     += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_credit_card"].asCString(),"1")==0) {
            trans->total_other_credit_amt   += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_debit_card"].asCString(),"1")==0) {
            trans->total_debit_card__amount += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_check"].asCString(),"1")==0) {
            trans->total_gift_cheque_amt    += dPayAmt-dChangeAmt;
        } else if (strcmp(jtender["is_gc"].asCString(),"1")==0) {
            trans->total_gift_cheque_amt    += dPayAmt-dChangeAmt;
        } else {
            trans->total_other_tender_amount  += dPayAmt-dChangeAmt;
        }
        trans->total_mastercard_amount  += 0.00;
        trans->total_visa_amount        += 0.00;
        trans->total_american_exp_amt   += 0.00;
        trans->total_diners_amt         += 0.00;
        trans->total_jcb_amt            += 0.00;

        if (i == 0){
            trans->payment_type_1    = tmpPayDesc;
            trans->payment_amount_1 += dPayAmt-dChangeAmt;
        } else if (i==1){
            trans->payment_type_2    = tmpPayDesc;
            trans->payment_amount_2 += dPayAmt-dChangeAmt;
        } else if (i==2){
            trans->payment_type_3    = tmpPayDesc;
            trans->payment_amount_3 += dPayAmt-dChangeAmt;
        }
    }

    (jtrans["register_number"].asCString() !=nullptr) ?
                trans->terminal_number = jtrans["register_number"].asCString() : trans->terminal_number  = "0";
    (jtrans["settings_register"]["serialno"].asCString() !=nullptr) ?
                trans->serial_number = jtrans["settings_register"]["serialno"].asCString() : trans->serial_number  = "0";

    _transList.push_back(trans);

    return true;
}
