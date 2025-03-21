/********************************************************************
          FILE:          erpsales_detail.cpp
   DESCRIPTION:          Publish data
 ********************************************************************/
//////////////////////////////////////////
//  Header(s)
#include <string>
#include <sstream>
#include "pystring.h"
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
      FUNCTION:         export_detail
   DESCRIPTION:         Sales detail
 ********************************************************************/
bool                    cyerpsales::export_item_regular (enumHeader type,
                                                          Json::Value& jline,
                                                          cylinedetail* detail,
                                                          std::string systransnum)
{
    (void)type;
    char sztmp[64];
    string retail_price;
    double dretail_price=0;
    string rcptType = _ini->get_value("RECEIPT","TYPE");
    //////////////////////////////////////
    //  Frappe variable(s)
    string dt = _util->date_mysql();
    //  conversion variables
    string creation=dt, modified=dt, idx="0", docstatus="0",
           modified_by="Administrator", owner="Administrator";

    string qty = jline["quantity"].asString();
    retail_price = jline["retail_price"].asString();
    dretail_price = _util->stodsafe(retail_price);

    detail->line[ENUM_SALES_ITEM::price_list_rate]=retail_price;
    detail->line[ENUM_SALES_ITEM::base_price_list_rate]=retail_price;
    detail->line[ENUM_SALES_ITEM::margin_type]="";
    detail->line[ENUM_SALES_ITEM::margin_rate_or_amount]="0";
    detail->line[ENUM_SALES_ITEM::rate_with_margin]="0";
    detail->line[ENUM_SALES_ITEM::base_rate_with_margin]="0";
    //////////////////////////////////
    //  LINE Discount
    string discount_percentage="0", discount_amount="0";
    if (jline.isMember("det_discount"))  {
        Json::Value jdiscount;
        jdiscount = jline["det_discount"];
        discount_amount = jdiscount["discount_value"].asString();
    }  else
        discount_amount = jline["amount_discount"].asString();

    if (jline.isMember("settings_detail_discount"))  {
        Json::Value jdiscount;
        jdiscount = jline["settings_detail_discount"];
        discount_percentage = jdiscount["discvalue"].asString();
    }  else  {
        double dpct = _util->stodsafe(discount_amount) / dretail_price;
        dpct = dpct * 100;
        sprintf (sztmp,"%.02f",dpct);
        discount_percentage = sztmp;
    }
    detail->line[ENUM_SALES_ITEM::discount_amount]=discount_amount;
    detail->line[ENUM_SALES_ITEM::discount_percentage]=discount_percentage;
    //////////////////////////////////
    //  Price / quantity
    double dbase_rate = _util->stodsafe(retail_price)-_util->stodsafe(discount_amount);
    sprintf (sztmp,"%.02f",dbase_rate);  string base_rate = sztmp;

    detail->line[ENUM_SALES_ITEM::rate]=base_rate;
    detail->line[ENUM_SALES_ITEM::base_rate]=base_rate;

    double dquantity = _util->stodsafe(jline["quantity"].asString());
    double damount = dquantity * dbase_rate;
    sprintf(sztmp, "%.02f", damount);
    detail->line[ENUM_SALES_ITEM::amount]=sztmp;
    detail->line[ENUM_SALES_ITEM::base_amount]=sztmp;
    //////////////////////////////////
    //  Tax
    int jtl = 0;
    Json::Value jtaxlist;
    jtaxlist.clear();
    string item_tax_template, item_tax_rate,
           item_tax_desc, tax_value;
    if (jline.isMember("det_tax"))  {
        Json::Value jtax;
        jtax = jline["det_tax"];

        item_tax_template  = jtax["tax_code"].asString();
        if (rcptType != "joel" && rcptType != "rico")  {
            item_tax_template += " - ";  item_tax_template += _abbr;
        }
        item_tax_rate  = "{\"Tax ";
        item_tax_rate += item_tax_template;
        item_tax_rate += "\": ";
        item_tax_rate += jtax["tax_value"].asString();
        item_tax_rate += "}";

        tax_value = jtax["tax_value"].asString();
        item_tax_desc  = jtax["description"].asString();

        jtaxlist[jtl]=jtax;
        jtl++;
    }  else  {
        item_tax_template = "";
    }
    detail->line[ENUM_SALES_ITEM::item_tax_template]=item_tax_template;

    detail->line[ENUM_SALES_ITEM::stock_uom_rate]=retail_price;
    detail->line[ENUM_SALES_ITEM::is_free_item]="0";
    detail->line[ENUM_SALES_ITEM::grant_commission]="1";
    //////////////////////////////////
    //  Net rate
    double dnet_rate, damount_tax = _util->stodsafe(jline["amount_tax"].asString());
    if (dquantity <= 1.00)
        dnet_rate = damount_tax;
    else
        dnet_rate = damount_tax / dquantity;
    double dnet_amount = damount_tax;

    sprintf (sztmp, "%f", dnet_rate);
    detail->line[ENUM_SALES_ITEM::net_rate]=sztmp;
    detail->line[ENUM_SALES_ITEM::base_net_rate]=sztmp;

    sprintf (sztmp, "%f", dnet_amount);
    detail->line[ENUM_SALES_ITEM::net_amount]=sztmp;
    detail->line[ENUM_SALES_ITEM::base_net_amount]=sztmp;


    detail->line[ENUM_SALES_ITEM::delivered_by_supplier]="0";
    detail->line[ENUM_SALES_ITEM::income_account]=_against_income_account;
    detail->line[ENUM_SALES_ITEM::is_fixed_asset]="0";
    detail->line[ENUM_SALES_ITEM::expense_account]=_expense_account;
    detail->line[ENUM_SALES_ITEM::enable_deferred_revenue]="0";

    detail->line[ENUM_SALES_ITEM::weight_per_unit]="0";
    detail->line[ENUM_SALES_ITEM::total_weight]="0";
    detail->line[ENUM_SALES_ITEM::warehouse]=_set_warehouse;

    detail->line[ENUM_SALES_ITEM::incoming_rate]=retail_price;
    detail->line[ENUM_SALES_ITEM::item_tax_rate]=item_tax_rate;

    detail->line[ENUM_SALES_ITEM::actual_qty]="0";
    detail->line[ENUM_SALES_ITEM::actual_batch_qty]="0";
    detail->line[ENUM_SALES_ITEM::delivered_qty]="0";
    detail->line[ENUM_SALES_ITEM::cost_center]=_cost_center;
    detail->line[ENUM_SALES_ITEM::parent]=systransnum;
    detail->line[ENUM_SALES_ITEM::parentfield]="items";
    detail->line[ENUM_SALES_ITEM::parenttype]="Sales Invoice";

    detail->line[ENUM_SALES_ITEM::service_end_date]=creation;
    detail->line[ENUM_SALES_ITEM::service_start_date]=creation;
    detail->line[ENUM_SALES_ITEM::service_stop_date]=creation;
    detail->line[ENUM_SALES_ITEM::use_serial_batch_fields]="0";
    detail->line[ENUM_SALES_ITEM::allow_zero_valuation_rate]="0";
    detail->line[ENUM_SALES_ITEM::company_total_stock]="0";
    detail->line[ENUM_SALES_ITEM::page_break]="0";    //////////////////////////////////////
    return true;
}
