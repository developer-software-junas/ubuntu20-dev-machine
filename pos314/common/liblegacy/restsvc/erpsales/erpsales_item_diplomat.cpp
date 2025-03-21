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
bool                    cyerpsales::export_item_diplomat (enumHeader type,
                                                          Json::Value& jline,
                                                          cylinedetail* detail,
                                                          std::string systransnum)
{
    string sql;
    MySqlDb xposdb (_pos);
    MySqlDb xtaxdb (_pos);
    MySqlDb xdetdb (_pos);
    MySqlDb* posdb = &xposdb;

    (void)type;
    char sztmp[64];
    string retail_price;
    //double dqty=0,
    double dretail_price=0;
    string rcptType = _ini->get_value("RECEIPT","TYPE");
    //////////////////////////////////////
    //  Frappe variable(s)
    string dt = _util->date_mysql();
    //  conversion variables
    string creation=dt, modified=dt, idx="0", docstatus="0",
           modified_by="Administrator", owner="Administrator";

    double dtaxamt = _util->stodsafe(jline["less_tax"].asString());
    //double dtrxamt = _util->stodsafe(jline["amount_trx"].asString());

    string qty = jline["quantity"].asString();
    //double dqty = _util->stodsafe(qty);
    retail_price = jline["retail_price"].asString();
    dretail_price = _util->stodsafe(retail_price);

    //  retail first
    dretail_price = dretail_price - dtaxamt;
    sprintf (sztmp, "%.02f", dretail_price);
    retail_price = sztmp;
    //  then no discount
    _trxDiscount = 0;
    //////////////////////////////////////
    //  Populate the frappe column(s)
    detail->line[ENUM_SALES_ITEM::name]=systransnum;
    detail->line[ENUM_SALES_ITEM::creation]=creation;
    detail->line[ENUM_SALES_ITEM::modified]=modified;
    detail->line[ENUM_SALES_ITEM::modified_by]=modified_by;
    detail->line[ENUM_SALES_ITEM::owner]=owner;
    detail->line[ENUM_SALES_ITEM::docstatus]=docstatus;
    detail->line[ENUM_SALES_ITEM::idx]=idx;

    detail->line[ENUM_SALES_ITEM::barcode]="";
    detail->line[ENUM_SALES_ITEM::has_item_scanned]="0";
    detail->line[ENUM_SALES_ITEM::item_code]=jline["item_code"].asString();
    detail->line[ENUM_SALES_ITEM::item_name]=jline["description"].asString();
    detail->line[ENUM_SALES_ITEM::customer_item_code]="";
    detail->line[ENUM_SALES_ITEM::description]=jline["description"].asString();

    Json::Value jproduct;
    if (!jline.isMember("detail_product_header"))
        return seterrormsg("Not product detail found");
    jproduct = jline["detail_product_header"];
    //////////////////////////////////
    //  Subclass code
    string subclass_cd  = "";
    if (rcptType != "joel" && rcptType != "rico")  {
        subclass_cd += jproduct["subclass_cd"].asString();
    }  else  {
        subclass_cd  = jproduct["category_cd"].asString();  subclass_cd += "-";
        subclass_cd += jproduct["subcat_cd"].asString();  subclass_cd += "-";
        subclass_cd += jproduct["class_cd"].asString();  subclass_cd += "-";
        subclass_cd += jproduct["subclass_cd"].asString();
    }

    posdb->sql_reset();
    sql  = "select * from cy_subclass where subclass_cd = '";
    sql += subclass_cd; sql += "' ";
    if (!posdb->sql_result(sql,false))
        return seterrormsg (posdb->errordb().c_str());
    if (posdb->eof())  {
        sql  = "select * from cy_subclass ";
        sql += "where category_cd = '";  sql += jproduct["category_cd"].asString(); sql += "' ";
        sql += "and   subcat_cd = '";    sql += jproduct["subcat_cd"].asString(); sql += "' ";
        sql += "and   class_cd = '";     sql += jproduct["class_cd"].asString(); sql += "' ";
        sql += "and   subclass_cd = '";  sql += jproduct["subclass_cd"].asString(); sql += "' ";
        if (!posdb->sql_result(sql,false))
            return seterrormsg (posdb->errordb().c_str());
        if (posdb->eof())  {
            string tmp = "Unable to retrieve the category: ";
            tmp += sql;
            return seterrormsg(tmp.c_str());
        }
    }
    string item_group = posdb->sql_field_value("description");
    if (rcptType != "joel" && rcptType != "rico")  {
        item_group += " (";
        item_group += subclass_cd;
        item_group += ")";
    }
    detail->line[ENUM_SALES_ITEM::item_group]=item_group;


    detail->line[ENUM_SALES_ITEM::brand]="";
    detail->line[ENUM_SALES_ITEM::qty]=jline["quantity"].asString();
    //////////////////////////////////
    //  UOM
    Json::Value jdetprod;
    if (!jline.isMember("det_product"))
        return seterrormsg("No product detail information found");
    jdetprod = jline["det_product"];
    string uom  = jproduct["type_code"].asString(); uom += "-";
           uom += jdetprod["type_code"].asString();

    detail->line[ENUM_SALES_ITEM::uom]=uom;
    detail->line[ENUM_SALES_ITEM::stock_uom]=uom;
    detail->line[ENUM_SALES_ITEM::conversion_factor]="1";
    detail->line[ENUM_SALES_ITEM::stock_qty]=jline["quantity"].asString();




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
    //  Price
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

            item_tax_template  = "0 - JT";;
            item_tax_rate  = "{\"Tax ";
            item_tax_rate += item_tax_template;
            item_tax_rate += "\": 0}";

            tax_value = "0";
            item_tax_desc = "VAT Exempt";

            jtax["tax_code"]="0 - JT";
            jtax["tax_value"]="0";
            jtax["description"]="VAT Exempt";


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
    detail->line[ENUM_SALES_ITEM::page_break]="0";
    //////////////////////////////////////

    return true;
}
