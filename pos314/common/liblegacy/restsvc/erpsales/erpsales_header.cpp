/********************************************************************
          FILE:          erpsales_header.cpp
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

using salestax::ENUM_SALES_TAX;
using salespay::ENUM_SALES_PAY;
using salesitem::ENUM_SALES_ITEM;
using salesinvoice::ENUM_SALES_INVOICE;
using erpcustomer::ENUM_ERPCUSTOMER;

string                  time_eight_colon ();
/********************************************************************
      FUNCTION:         export_header
   DESCRIPTION:         Sales header
 ********************************************************************/
bool                    cyerpsales::export_header (enumHeader type,
                                                   Json::Value jheader,
                                                   std::string systransnum)
{
    stringstream sss;
    sss << jheader;
    string zz = sss.str();
    _log->logjson(zz.c_str());
    //////////////////////////////////////
    //  Reset total(s)
    _trxDiscount = 0.00;
    _lineDiscount = 0.00;
    //////////////////////////////////////
    //  Start
    if (!startup())
        return false;
    //////////////////////////////////////
    //  Frappe variable(s)
    string dt = _util->date_mysql();
    //  conversion variables
    string creation=dt, modified=dt, idx="0", docstatus="0",
           modified_by="Administrator", owner="Administrator",
           naming_series="ACC-SINV-.YYYY.-";
    //////////////////////////////////////
    //  Populate the frappe column(s)
    _invHdrValue[ENUM_SALES_INVOICE::name]=systransnum;
    _invHdrValue[ENUM_SALES_INVOICE::creation]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::modified]=modified;
    _invHdrValue[ENUM_SALES_INVOICE::modified_by]=modified_by;
    _invHdrValue[ENUM_SALES_INVOICE::owner]=owner;
    _invHdrValue[ENUM_SALES_INVOICE::docstatus]=docstatus;
    _invHdrValue[ENUM_SALES_INVOICE::idx]=idx;
    //////////////////////////////////////
    //  Write off columns
    _invHdrValue[ENUM_SALES_INVOICE::write_off_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::base_write_off_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::write_off_outstanding_amount_automatically]="0";
    _invHdrValue[ENUM_SALES_INVOICE::write_off_account]=_write_off_account;
    _invHdrValue[ENUM_SALES_INVOICE::write_off_cost_center]=_write_off_cost_center;
    //////////////////////////////////////
    //  Loyalty TODO: add loyalty logic
    _invHdrValue[ENUM_SALES_INVOICE::redeem_loyalty_points]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_points]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_program]="";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_redemption_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_redemption_cost_center]="";
    //////////////////////////////////////
    //  Default(s)
    _invHdrValue[ENUM_SALES_INVOICE::allocate_advances_automatically]="0";
    _invHdrValue[ENUM_SALES_INVOICE::only_include_allocated_payments]="0";
    _invHdrValue[ENUM_SALES_INVOICE::ignore_default_payment_terms_template]="0";
    _invHdrValue[ENUM_SALES_INVOICE::payment_terms_template]="";
    _invHdrValue[ENUM_SALES_INVOICE::debit_to]=_debit_to;
    _invHdrValue[ENUM_SALES_INVOICE::party_account_currency]=_currency;
    _invHdrValue[ENUM_SALES_INVOICE::is_opening]="No";
    _invHdrValue[ENUM_SALES_INVOICE::unrealized_profit_loss_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::against_income_account]=_against_income_account;
    _invHdrValue[ENUM_SALES_INVOICE::sales_partner]="";

    _invHdrValue[ENUM_SALES_INVOICE::commission_rate]="0";
    _invHdrValue[ENUM_SALES_INVOICE::total_commission]="0";

    _invHdrValue[ENUM_SALES_INVOICE::language]="en";
    _invHdrValue[ENUM_SALES_INVOICE::status]="Draft";
    _invHdrValue[ENUM_SALES_INVOICE::is_internal_customer]="0";
    _invHdrValue[ENUM_SALES_INVOICE::is_discounted]="0";

    _invHdrValue[ENUM_SALES_INVOICE::tax_category]="";//_tax_category;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::taxes_and_charges]="";//_taxes_and_charges;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::shipping_rule]="";
    _invHdrValue[ENUM_SALES_INVOICE::incoterm]="";
    _invHdrValue[ENUM_SALES_INVOICE::named_place]="";

    _invHdrValue[ENUM_SALES_INVOICE::po_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::group_same_items]="0";
    _invHdrValue[ENUM_SALES_INVOICE::from_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::to_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::scan_barcode]="";
    /*TODO:  customer and shipping information
                customer_address, address_display, contact_person, contact_display, contact_mobile, contact_email,
                territory, shipping_address_name, shipping_address, dispatch_address_name, dispatch_address,
                company_address, company_address_display,

                tc_name, terms, po_no, po_date,
    */
    //////////////////////////////////////
    //  Title
    Json::Value jtrxtype;
    if (!jheader.isMember("hdr_trx"))
        return seterrormsg("JSON SALES: No transaction type found");
    jtrxtype = jheader["hdr_trx"];
    if (!jtrxtype.isMember("description"))
        return seterrormsg("JSON SALES: No transaction description found");
    _invHdrValue[ENUM_SALES_INVOICE::title]=jtrxtype["description"].asString();
    //  Naming series ==>  maybe change to "systransnum"
    _invHdrValue[ENUM_SALES_INVOICE::naming_series]=naming_series;
    //  Customer
    _isPwd=false;
    _isSen5=false;
    _isSenior=false;
    _isDiplomat=false;

    Json::Value jtrxacct;
    string first="",last="";
    string customer, customer_name, customer_group;
    if (jtrxtype.isMember("trx_account"))  {
        jtrxacct = jtrxtype["trx_account"];
        customer = jtrxacct["customer_id"].asString();
        if (customer_exists(customer))  {
            if (jtrxacct.isMember("last_name"))
                last = jtrxacct["last_name"].asString();
            if (jtrxacct.isMember("first_name"))
                first = jtrxacct["first_name"].asString();
            customer_group = jtrxacct["account_type_desc"].asString();
            if (!customer.empty())
                customer = jtrxacct["account_number"].asString();

            size_t len = first.length() + last.length();
            if (len > 0)  {
                customer_name = first;
                if (first.length() > 0)  {
                    customer_name += " ";
                }
                customer_name += last;
            }
            cleanup_customer();
            string customer_id = jtrxacct["account_number"].asString();

            _customerInfo[ENUM_ERPCUSTOMER::creation]=creation;
            _customerInfo[ENUM_ERPCUSTOMER::modified]=modified;
            _customerInfo[ENUM_ERPCUSTOMER::modified_by]=modified_by;
            _customerInfo[ENUM_ERPCUSTOMER::owner]=owner;
            _customerInfo[ENUM_ERPCUSTOMER::docstatus]=docstatus;
            _customerInfo[ENUM_ERPCUSTOMER::idx]=idx;

            _customerInfo[ENUM_ERPCUSTOMER::is_internal_customer]="0";
            _customerInfo[ENUM_ERPCUSTOMER::active]=1;
            _customerInfo[ENUM_ERPCUSTOMER::application_date]=dt;
            _customerInfo[ENUM_ERPCUSTOMER::application_type]="";
            _customerInfo[ENUM_ERPCUSTOMER::card_number]=jtrxacct["account_number"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::card_type]=jtrxacct["account_type_desc"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::card_type_code]=jtrxacct["account_type_code"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::creation]=dt;
            _customerInfo[ENUM_ERPCUSTOMER::card_type]=jtrxacct["account_type_desc"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_card_number]=jtrxacct["account_number"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_company_name]=jtrxacct["company_name"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_first_name]=jtrxacct["first_name"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_middle_name]=jtrxacct["middle_name"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_last_name]=jtrxacct["last_name"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::custom_systransnum]=systransnum;
            _customerInfo[ENUM_ERPCUSTOMER::customer_group]=jtrxacct["account_type_desc"].asString();
            _customerInfo[ENUM_ERPCUSTOMER::customer_name]=customer_name;
            _customerInfo[ENUM_ERPCUSTOMER::customer_type]="Individual";
            _customerInfo[ENUM_ERPCUSTOMER::default_currency]="PHP";
            _customerInfo[ENUM_ERPCUSTOMER::disabled]="0";
            _customerInfo[ENUM_ERPCUSTOMER::is_frozen]="0";
            _customerInfo[ENUM_ERPCUSTOMER::is_internal_customer]="0";
            _customerInfo[ENUM_ERPCUSTOMER::customer_pos_id]=customer_id;
            _customerInfo[ENUM_ERPCUSTOMER::name]=customer_name;
        }  else  {
            customer = "";
        }

        Json::Value jtranstype;
        jtranstype = jtrxtype["settings_transtype"];
        if (jtranstype["is_pwd"].asString()=="1")
            _isPwd = true;
        if (jtranstype["is_senior"].asString()=="1")
            _isSenior = true;
        if (jtranstype["is_zero_rated"].asString()=="1")
            _isDiplomat = true;
    }
    if (customer.empty() || last == "Guest")  {
        customer = "1";
        customer_name = "Walk In";
        customer_group = "Individual";
    }
    _invHdrValue[ENUM_SALES_INVOICE::customer]=customer;
    _invHdrValue[ENUM_SALES_INVOICE::customer_name]=customer_name;
    _invHdrValue[ENUM_SALES_INVOICE::customer_group]=customer_group;
    //  Tax Id ==> for some reason, it's blank in the ERP sales invoice
    _invHdrValue[ENUM_SALES_INVOICE::tax_id]="";  //  blank, for now
    //  Company
    Json::Value jcompany;
    if (!jheader.isMember("hdr_company"))
        return seterrormsg("JSON SALES: No company information found");
    jcompany = jheader["hdr_company"];
    if (!jcompany.isMember("description"))
        return seterrormsg("JSON SALES: No company name found");
    string cn = jcompany["description"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::company]=cn;
    string tin = "00000";
    if (jcompany.isMember("tax_id"))
        tin = jcompany["tax_id"].asString();
    //_invHdrValue[ENUM_SALES_INVOICE::company_tax_id]=jcompany["tax_id"].asString();
    //  Posting
    string posting_date, posting_time;
    posting_time = time_eight_colon();
    posting_date = _util->date_dash(_util->date_eight().c_str(),"-");
    _invHdrValue[ENUM_SALES_INVOICE::due_date]=posting_date;
    _invHdrValue[ENUM_SALES_INVOICE::posting_time]=posting_time;
    _invHdrValue[ENUM_SALES_INVOICE::posting_date]=posting_date;
    _invHdrValue[ENUM_SALES_INVOICE::set_posting_time]="0";

    _invHdrValue[ENUM_SALES_INVOICE::is_pos]="1";
    _invHdrValue[ENUM_SALES_INVOICE::pos_profile]=_pos_profile;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::is_consolidated]="0";

    if (type == REFUND)  {
        _invHdrValue[ENUM_SALES_INVOICE::is_return]="1";
        _invHdrValue[ENUM_SALES_INVOICE::return_against]=jheader["orig_systransnum"].asString();
    }  else  {
        _invHdrValue[ENUM_SALES_INVOICE::is_return]="0";
        _invHdrValue[ENUM_SALES_INVOICE::return_against]="";
    }
    _invHdrValue[ENUM_SALES_INVOICE::update_outstanding_for_self]="1";
    _invHdrValue[ENUM_SALES_INVOICE::update_billed_amount_in_sales_order]="0";
    _invHdrValue[ENUM_SALES_INVOICE::update_billed_amount_in_delivery_note]="1";
    _invHdrValue[ENUM_SALES_INVOICE::is_debit_note]="0";
    _invHdrValue[ENUM_SALES_INVOICE::amended_from]="";
    //  Cost center
    _invHdrValue[ENUM_SALES_INVOICE::cost_center]=_cost_center;  //  from startup()

    _invHdrValue[ENUM_SALES_INVOICE::currency]=_currency;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::conversion_rate]="1";

    _invHdrValue[ENUM_SALES_INVOICE::selling_price_list]=_selling_price_list;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::price_list_currency]=_currency;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::plc_conversion_rate]="1";

    _invHdrValue[ENUM_SALES_INVOICE::scan_barcode]="";
    _invHdrValue[ENUM_SALES_INVOICE::update_stock]="1";

    _invHdrValue[ENUM_SALES_INVOICE::set_warehouse]=_set_warehouse;  // from startup()
    _invHdrValue[ENUM_SALES_INVOICE::set_target_warehouse]=_set_target_warehouse;  // from startup()

    if (!jheader.isMember("cytotal_items"))
        return seterrormsg("JSON SALES: Total quantity missing");
    _invHdrValue[ENUM_SALES_INVOICE::total_qty]=jheader["cytotal_items"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::total_net_weight]="0";


/*
            amount_eligible_for_commission, commission_rate, total_commission, letter_head,
            group_same_items, select_print_heading, language, subscription, from_date, auto_repeat, to_date, status,
            inter_company_invoice_reference, campaign, represents_company, source, customer_group, is_internal_customer, is_discounted,
            remarks, _user_tags, _comments, _assign, _liked_by, _seen, company_contact_person, si_end
*/
    return true;
}
string                  time_eight_colon ()
{
    tm                  *ltm;
    time_t              today;
    string              strtime;
    char                tmp [64];

    today = time (0);
    today += 314L;
    ltm = localtime(&today);
    snprintf (tmp, 63, "%02d:%02d:%02d", ltm->tm_hour,
             ltm->tm_min, ltm->tm_sec);
    strtime = tmp;
    return strtime;
}
