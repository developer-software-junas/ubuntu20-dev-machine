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
/********************************************************************
      FUNCTION:         export_header
   DESCRIPTION:         Sales header
 ********************************************************************/
bool                    cyerpsales::export_header (enumHeader type,
                                                   Json::Value jheader,
                                                   std::string systransnum)
{
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
    //  Populate the header column(s)
    _invHdrValue[ENUM_SALES_INVOICE::name]=systransnum;
    _invHdrValue[ENUM_SALES_INVOICE::creation]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::modified]=modified;
    _invHdrValue[ENUM_SALES_INVOICE::modified_by]=modified_by;
    _invHdrValue[ENUM_SALES_INVOICE::owner]=owner;
    _invHdrValue[ENUM_SALES_INVOICE::docstatus]=docstatus;
    _invHdrValue[ENUM_SALES_INVOICE::idx]=idx;
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
    Json::Value jtrxacct;
    string customer="Walk In", customer_name="Walk In";
    if (jtrxtype.isMember("trx_account"))  {
        jtrxacct = jtrxtype["trx_account"];
        string first="",last="";
        if (jtrxacct.isMember("last_name"))
            last = jtrxacct["last_name"].asString();
        if (jtrxacct.isMember("first_name"))
            first = jtrxacct["first_name"].asString();
        size_t len = first.length() + last.length();
        if (len > 0)  {
            customer = first; customer_name = first;
            if (first.length() > 0)  {
                customer+= " "; customer_name += " ";
            }
            customer += last; customer_name += last;
        }
    }
    _invHdrValue[ENUM_SALES_INVOICE::customer]=customer;
    _invHdrValue[ENUM_SALES_INVOICE::customer_name]=customer_name;
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
    //_invHdrValue[ENUM_SALES_INVOICE::company]=cn;
    if (!jcompany.isMember("tax_id"))
        return seterrormsg("JSON SALES: No company tax id found");
    //_invHdrValue[ENUM_SALES_INVOICE::company_tax_id]=jcompany["tax_id"].asString();
    //  Posting
    string posting_date, posting_time;
    posting_time = "00:00:01";//_util->time_eight_colon();
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

    _invHdrValue[ENUM_SALES_INVOICE::ignore_pricing_rule]="0";
    _invHdrValue[ENUM_SALES_INVOICE::scan_barcode]="";
    _invHdrValue[ENUM_SALES_INVOICE::update_stock]="1";

    _invHdrValue[ENUM_SALES_INVOICE::set_warehouse]=_set_warehouse;  // from startup()
    _invHdrValue[ENUM_SALES_INVOICE::set_target_warehouse]=_set_target_warehouse;  // from startup()

    if (!jheader.isMember("cytotal_items"))
        return seterrormsg("JSON SALES: Total quantity missing");
    _invHdrValue[ENUM_SALES_INVOICE::total_qty]=jheader["cytotal_items"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::total_net_weight]="0";
    //  Gross sales amount
    if (!jheader.isMember("cytotal_gross_amount"))
        return seterrormsg("JSON SALES: Total gross sales missing");
    string gross_amount = jheader["cytotal_gross_amount"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::total]=jheader["cytotal_gross_amount"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::base_total]=jheader["cytotal_gross_amount"].asString();
    //  NET sales amount
    double net_vat=0, net_zero=0,
           net_exempt=0, vat_amount=0;
    if (!jheader.isMember("cytotal_net_vat"))
        return seterrormsg("JSON SALES: Net VAT total missing");
    else
        net_vat = _util->stodsafe(jheader["cytotal_net_vat"].asString());
    if (!jheader.isMember("cytotal_net_zero"))
        return seterrormsg("JSON SALES: Net Zero total missing");
    else
        net_zero = _util->stodsafe(jheader["cytotal_net_zero"].asString());
    if (!jheader.isMember("cytotal_net_exempt"))
        return seterrormsg("JSON SALES: Net exempt total missing");
    else
        net_exempt = _util->stodsafe(jheader["cytotal_net_exempt"].asString());
    if (!jheader.isMember("cytotal_vat_amount"))
        return seterrormsg("JSON SALES: Total VAT amount missing");
    else
        vat_amount = _util->stodsafe(jheader["cytotal_vat_amount"].asString());

    double dbase_net_total = net_vat + net_zero + net_exempt + vat_amount + _svcCharge;
    char base_net_total [32];
    sprintf (base_net_total, "%.02f", dbase_net_total);
    _invHdrValue[ENUM_SALES_INVOICE::net_total]=base_net_total;
    _invHdrValue[ENUM_SALES_INVOICE::base_net_total]=base_net_total;

    _invHdrValue[ENUM_SALES_INVOICE::tax_category]=_tax_category;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::taxes_and_charges]=_taxes_and_charges;  //  from startup()
    _invHdrValue[ENUM_SALES_INVOICE::shipping_rule]="";
    _invHdrValue[ENUM_SALES_INVOICE::incoterm]="";
    _invHdrValue[ENUM_SALES_INVOICE::named_place]="";

    char sztmp[64];
    sprintf (sztmp, "%.02f", (vat_amount + _svcCharge));
    _invHdrValue[ENUM_SALES_INVOICE::total_taxes_and_charges]=sztmp;
    _invHdrValue[ENUM_SALES_INVOICE::base_total_taxes_and_charges]=sztmp;

    _invHdrValue[ENUM_SALES_INVOICE::base_in_words]="";
    _invHdrValue[ENUM_SALES_INVOICE::rounding_adjustment]="0";
    _invHdrValue[ENUM_SALES_INVOICE::base_rounding_adjustment]="0";

    _invHdrValue[ENUM_SALES_INVOICE::grand_total]=jheader["cytotal_gross_amount"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::base_grand_total]=jheader["cytotal_gross_amount"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::base_rounded_total]=jheader["cytotal_gross_amount"].asString();

    _invHdrValue[ENUM_SALES_INVOICE::use_company_roundoff_cost_center]="0";
    _invHdrValue[ENUM_SALES_INVOICE::rounded_total]=jheader["cytotal_gross_amount"].asString();
    _invHdrValue[ENUM_SALES_INVOICE::in_words]="";
    _invHdrValue[ENUM_SALES_INVOICE::total_advance]="0";
    _invHdrValue[ENUM_SALES_INVOICE::outstanding_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::disable_rounded_total]="1";

    _invHdrValue[ENUM_SALES_INVOICE::apply_discount_on]="Grand Total";

    string base_discount_amount = "0";
    if (_trxDiscount > 0)  {
        char szd [32];
        sprintf (szd, "%.02f", _trxDiscount);
        base_discount_amount = szd;
    }
    _invHdrValue[ENUM_SALES_INVOICE::base_discount_amount]=base_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::is_cash_or_non_trade_discount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::discount_amount]=base_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::additional_discount_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::additional_discount_percentage]="0";

    _invHdrValue[ENUM_SALES_INVOICE::other_charges_calculation]="<pre></pre>";
    _invHdrValue[ENUM_SALES_INVOICE::total_billing_hours]="0";
    _invHdrValue[ENUM_SALES_INVOICE::total_billing_amount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::cash_bank_account]="";
    //  Payment / change
    Json::Value jpay, jline;
    string change_amount, payment_amount;
    double dpaid_amount=0, dchange_amount=0;
    if (jheader.isMember("pay_sales"))  {
        jpay = jheader["pay_sales"];
        for (int i = 0; i < (int)jpay.size(); i++)  {
            jline = jpay[i];
            change_amount = jline["change_amount"].asString();
            payment_amount = jline["payment_amount"].asString();
            if (change_amount.length() < 1) change_amount = "0";
            if (payment_amount.length() < 1) payment_amount = "0";
            dpaid_amount += _util->stodsafe(payment_amount);
            dchange_amount += _util->stodsafe(change_amount);
        }
    }
    sprintf (sztmp, "%.02f", dchange_amount);
    change_amount = sztmp;
    sprintf (sztmp, "%.02f", dpaid_amount);
    payment_amount = sztmp;

    _invHdrValue[ENUM_SALES_INVOICE::paid_amount]=payment_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_paid_amount]=payment_amount;
    _invHdrValue[ENUM_SALES_INVOICE::change_amount]=change_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_change_amount]=change_amount;
    _invHdrValue[ENUM_SALES_INVOICE::account_for_change_amount]=_account_for_change_amount;
    _invHdrValue[ENUM_SALES_INVOICE::allocate_advances_automatically]="0";
    _invHdrValue[ENUM_SALES_INVOICE::only_include_allocated_payments]="0";
    _invHdrValue[ENUM_SALES_INVOICE::write_off_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::base_write_off_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::write_off_outstanding_amount_automatically]="0";
    _invHdrValue[ENUM_SALES_INVOICE::write_off_account]=_write_off_account;
    _invHdrValue[ENUM_SALES_INVOICE::write_off_cost_center]=_write_off_cost_center;
    //  Loyalty TODO: add loyalty logic
    _invHdrValue[ENUM_SALES_INVOICE::redeem_loyalty_points]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_points]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_program]="";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_redemption_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::loyalty_redemption_cost_center]="";
/*TODO:  customer and shipping information
            customer_address, address_display, contact_person, contact_display, contact_mobile, contact_email,
            territory, shipping_address_name, shipping_address, dispatch_address_name, dispatch_address,
            company_address, company_address_display,

            tc_name, terms, po_no, po_date,
*/
    _invHdrValue[ENUM_SALES_INVOICE::ignore_default_payment_terms_template]="0";
    _invHdrValue[ENUM_SALES_INVOICE::payment_terms_template]="";
    _invHdrValue[ENUM_SALES_INVOICE::debit_to]=_debit_to;
    _invHdrValue[ENUM_SALES_INVOICE::party_account_currency]=_currency;
    _invHdrValue[ENUM_SALES_INVOICE::is_opening]="No";
    _invHdrValue[ENUM_SALES_INVOICE::unrealized_profit_loss_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::against_income_account]=_against_income_account;
    _invHdrValue[ENUM_SALES_INVOICE::sales_partner]="";
    _invHdrValue[ENUM_SALES_INVOICE::amount_eligible_for_commission]=base_net_total;
    _invHdrValue[ENUM_SALES_INVOICE::commission_rate]="0";
    _invHdrValue[ENUM_SALES_INVOICE::total_commission]="0";

    _invHdrValue[ENUM_SALES_INVOICE::language]="en";
    _invHdrValue[ENUM_SALES_INVOICE::status]="Draft";
    _invHdrValue[ENUM_SALES_INVOICE::is_internal_customer]="0";
    _invHdrValue[ENUM_SALES_INVOICE::is_discounted]="0";

    _invHdrValue[ENUM_SALES_INVOICE::po_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::group_same_items]="0";
    _invHdrValue[ENUM_SALES_INVOICE::from_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::to_date]=creation;
    _invHdrValue[ENUM_SALES_INVOICE::scan_barcode]="";

/*
            amount_eligible_for_commission, commission_rate, total_commission, letter_head,
            group_same_items, select_print_heading, language, subscription, from_date, auto_repeat, to_date, status,
            inter_company_invoice_reference, campaign, represents_company, source, customer_group, is_internal_customer, is_discounted,
            remarks, _user_tags, _comments, _assign, _liked_by, _seen, company_contact_person, si_end
*/
    return true;
}
