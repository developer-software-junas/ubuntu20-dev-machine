/********************************************************************
          FILE:          erpsales_detail.cpp
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
      FUNCTION:         export_detail
   DESCRIPTION:         Sales detail
 ********************************************************************/
bool                    cyerpsales::export_detail (enumHeader type,
                                                   Json::Value jheader,
                                                   std::string systransnum)
{
    string sql;
    MySqlDb xposdb (_pos);
    MySqlDb xtaxdb (_pos);
    MySqlDb xdetdb (_pos);
    MySqlDb* posdb = &xposdb;
    MySqlDb* taxdb = &xtaxdb;
    MySqlDb* detdb = &xdetdb;

    (void)type;
    _svcCharge = 0.00;
    //////////////////////////////////////
    //  Cleanup the detail record(s)
    cleanup_tax();
    cleanup_svc();
    cleanup_detail();
    //////////////////////////////////////
    //  Detail exists?
    if (!jheader.isMember("det_sales"))
        return true;
    //////////////////////////////////////
    //  Loop through each detail record
    char sztmp[64];
    cylinetax* linetax;
    cylinedetail* detail;
    Json::Value jdet,
                jline;
    Json::Value jwise,
                jwiseval,
                jtaxlist;

    int jtl = 0;
    jtaxlist.clear();
    jdet = jheader["det_sales"];
    string rcptType = _ini->get_value("RECEIPT","TYPE");
    //////////////////////////////////////
    //  Frappe variable(s)
    string dt = _util->date_mysql();
    //  conversion variables
    string creation=dt, modified=dt, idx="0", docstatus="0",
           modified_by="Administrator", owner="Administrator",
           naming_series="ACC-SINV-.YYYY.-";

    int j = (int)jdet.size();
    for (int ndet = 0; ndet < j; ndet++)  {
        //////////////////////////////////////
        //  Create a detail record
        jline = jdet[ndet];
        detail = new cylinedetail;
        //////////////////////////////////////
        //  Index / item sequence
        sprintf (sztmp, "%04d", (ndet+1));
        idx = sztmp;
        string invname = systransnum; invname += "-"; invname += sztmp;
        //////////////////////////////////////
        //  Discount totals
        if (jline["trx_flag"].asString() == "senior")  {
            double dtrxamt = _util->stodsafe(jline["amount_trx"].asString());
            double dtaxamt = _util->stodsafe(jline["less_tax"].asString());
            _trxDiscount += dtrxamt + dtaxamt;
        }
        //////////////////////////////////////
        //  Populate the frappe column(s)
        detail->line[ENUM_SALES_ITEM::name]=invname;
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
        //////////////////////////////////
        //  Item group
        Json::Value jproduct;
        if (!jline.isMember("detail_product_header"))
            return seterrormsg("Not product detail found");
        jproduct = jline["detail_product_header"];
        //////////////////////////////////
        //  Subclass code
        string subclass_cd  = "";
        if (rcptType == "joel")  {
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
        if (rcptType == "joel")  {
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
        detail->line[ENUM_SALES_ITEM::price_list_rate]=jline["retail_price"].asString();
        detail->line[ENUM_SALES_ITEM::base_price_list_rate]=jline["retail_price"].asString();
        detail->line[ENUM_SALES_ITEM::margin_type]="";
        detail->line[ENUM_SALES_ITEM::margin_rate_or_amount]="0";
        detail->line[ENUM_SALES_ITEM::rate_with_margin]="0";
        detail->line[ENUM_SALES_ITEM::base_rate_with_margin]="0";
        //////////////////////////////////
        //  Discount
        string discount_percentage, discount_amount;
        if (jline.isMember("det_dicount"))  {
            Json::Value jdiscount;
            jdiscount = jline["det_discount"];
            discount_percentage = jdiscount["discount_value"].asString();
        }  else  {
            discount_percentage = "0";
        }
        discount_amount = jline["amount_discount"].asString();
        detail->line[ENUM_SALES_ITEM::discount_amount]=discount_amount;
        detail->line[ENUM_SALES_ITEM::discount_percentage]=discount_percentage;
        //////////////////////////////////
        //  Price
        detail->line[ENUM_SALES_ITEM::rate]=jline["retail_price"].asString();
        detail->line[ENUM_SALES_ITEM::base_rate]=jline["retail_price"].asString();

        double dquantity = _util->stodsafe(jline["quantity"].asString());
        double dretail = _util->stodsafe(jline["retail_price"].asString());
        double damount = dquantity * dretail;
        sprintf(sztmp, "%.02f", damount);
        detail->line[ENUM_SALES_ITEM::amount]=sztmp;
        detail->line[ENUM_SALES_ITEM::base_amount]=sztmp;
        //////////////////////////////////
        //  Tax
        string item_tax_template, item_tax_rate,
               item_tax_desc, tax_value;
        if (jline.isMember("det_tax"))  {
            Json::Value jtax;
            jtax = jline["det_tax"];
            item_tax_template  = jtax["tax_code"].asString();
            if (rcptType != "joel")  {
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

        detail->line[ENUM_SALES_ITEM::stock_uom_rate]=jline["retail_price"].asString();
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

        detail->line[ENUM_SALES_ITEM::incoming_rate]=jline["retail_price"].asString();
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

        _invDetail.push_back(detail);
    }
    ///////////////////////////////////
    //  Tax detail - madugo!!!
    taxdb->sql_reset();
    stringstream sstax;

    sql  = "select distinct(tax_code) from tg_pos_mobile_detail where systransnum = '";
    sql += systransnum; sql += "' ";
    if (!taxdb->sql_result(sql,false))
        return seterrormsg(taxdb->errordb());
    if (taxdb->eof())
        return seterrormsg("No item tax...");

    jtl = 1;
    int taxidx = 0;
    double dtotal=0,dtotaltax=0;
    do  {
        detdb->sql_reset ();
        sql  = "select * from tg_pos_mobile_detail where systransnum = '";
        sql += systransnum; sql += "' and tax_code = '";
        sql += taxdb->sql_field_value("tax_code"); sql += "' ";
        if (!detdb->sql_result(sql,false))
            return seterrormsg(taxdb->errordb());
        if (detdb->eof())
            return seterrormsg("No item tax...");

        jwise.clear();
        Json::Value jwline;

        std::string jwtaxval;
        std::string tdesc = "";
        dtotal=0,dtotaltax=0;
        do  {
            double dqty = _util->stodsafe(detdb->sql_field_value("quantity"));
            double dretail = _util->stodsafe(detdb->sql_field_value("retail_price"));
            double dlinetax = _util->stodsafe(detdb->sql_field_value("less_tax"));
            double dlinetotal = dqty * dretail;

            dtotal += dlinetotal;
            dtotaltax += dlinetax;

            std::string trate="0";
            size_t b = jtaxlist.size();
            for (size_t a = 0; a < b; a++)  {
                Json::Value jt;
                jt = jtaxlist[(int)a];
                if (jt["tax_code"].asString() == detdb->sql_field_value("tax_code"))  {
                    trate = jt["tax_value"].asString();
                    tdesc = jt["description"].asString();
                }
            }

            jwiseval.clear();
            jwiseval[0]=_util->stodsafe(trate);
            jwiseval[1]=dlinetax;
            jwise[detdb->sql_field_value("item_code")]=jwiseval;
            _svcCharge += dlinetax;

            detdb->sql_next();
        }  while (!detdb->eof ());

        sstax.clear();
        sstax.str(std::string());
        sstax << jwise;
        string item_wise_tax_detail = sstax.str();
        //////////////////////////////////
        //  TAX breakdown
        sprintf(sztmp,"%d",jtl); jtl++;
        string taxname = systransnum; taxname += "-";
        taxname += sztmp; taxname += "-tax";

        string account_head = "Tax ";
        account_head += taxdb->sql_field_value("tax_code");
        if (rcptType != "joel")  {
            account_head += " - ";
            account_head += _abbr;
        }

        linetax = new cylinetax;
        _invTax.push_back(linetax);
        linetax->tax[ENUM_SALES_TAX::name]=taxname;
        linetax->tax[ENUM_SALES_TAX::creation]=creation;
        linetax->tax[ENUM_SALES_TAX::modified]=modified;
        linetax->tax[ENUM_SALES_TAX::modified_by]=modified_by;
        linetax->tax[ENUM_SALES_TAX::owner]=owner;
        linetax->tax[ENUM_SALES_TAX::docstatus]=docstatus;

        sprintf(sztmp,"%d",taxidx);
        linetax->tax[ENUM_SALES_TAX::idx]=sztmp; taxidx++;
        linetax->tax[ENUM_SALES_TAX::charge_type]="On Net Total";
        linetax->tax[ENUM_SALES_TAX::account_head]=account_head;
        linetax->tax[ENUM_SALES_TAX::description]=account_head;
        linetax->tax[ENUM_SALES_TAX::included_in_print_rate]="1";
        linetax->tax[ENUM_SALES_TAX::included_in_paid_amount]="0";
        string mainCost = "Main - "; mainCost += _abbr;
        linetax->tax[ENUM_SALES_TAX::cost_center]=mainCost;
        linetax->tax[ENUM_SALES_TAX::rate]="0";
        linetax->tax[ENUM_SALES_TAX::account_currency]=_currency;

        sprintf (sztmp,"%.02f",dtotaltax);
        linetax->tax[ENUM_SALES_TAX::tax_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::base_tax_amount]=sztmp;

        sprintf (sztmp,"%.02f",dtotaltax);
        linetax->tax[ENUM_SALES_TAX::base_total]=sztmp;
        linetax->tax[ENUM_SALES_TAX::tax_amount_after_discount_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::base_tax_amount_after_discount_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::total]=sztmp;

        linetax->tax[ENUM_SALES_TAX::item_wise_tax_detail]=item_wise_tax_detail;
        linetax->tax[ENUM_SALES_TAX::dont_recompute_tax]="0";
        linetax->tax[ENUM_SALES_TAX::parent]=systransnum;
        linetax->tax[ENUM_SALES_TAX::parentfield]="taxes";
        linetax->tax[ENUM_SALES_TAX::parenttype]="Sales Invoice";

        taxdb->sql_next ();
    }  while (!taxdb->eof());
    //////////////////////////////////////
    //  Service charge
    jwise.clear();
    taxdb->sql_reset();
    Json::Value jwline;

    sql  = "select * from tg_pos_mobile_detail where systransnum = '";
    sql += systransnum; sql += "' ";
    if (!taxdb->sql_result(sql,false))
        return seterrormsg(taxdb->errordb());
    dtotal=0,dtotaltax=0;
    if (!taxdb->eof())  {
        do  {
            string isSvc = taxdb->sql_field_value("is_service_charge");
            string isAuto = taxdb->sql_field_value("is_auto_servicecharge");
            double dsvc = _util->stodsafe(taxdb->sql_field_value("total_service_charge"));

            double dqty = _util->stodsafe(taxdb->sql_field_value("quantity"));
            double dretail = _util->stodsafe(taxdb->sql_field_value("retail_price"));
            double dlinetax = _util->stodsafe(taxdb->sql_field_value("less_tax"));
            double dlinetotal = dqty * dretail;


            dtotaltax += dsvc;
            bool addTotal = (isSvc=="1"&&isAuto=="1");
            string incVat = taxdb->sql_field_value("service_charge_vat");
            if (incVat == "1")  {
                if (addTotal)
                    dtotal += dlinetotal;
            }  else  {
                if (addTotal)
                    dtotal += (dlinetotal - dlinetax);
            }

            //if (addTotal)  {
                jwiseval.clear();
                jwiseval[0]="0.0";
                jwiseval[1]=dsvc;
                jwise[taxdb->sql_field_value("item_code")]=jwiseval;
            //}

            taxdb->sql_next();
        } while (!taxdb->eof ());

        sstax.clear();
        sstax.str(std::string());
        sstax << jwise;
        string item_wise_tax_detail = sstax.str();
        //////////////////////////////////
        //  TAX breakdown
        string taxname = systransnum; taxname += "-";
        taxname += "svc"; taxname += "-tax";

        string account_head = "Service Charge";
        account_head += " - "; account_head += _abbr;

        if (dtotal > 0)  {
            linetax = new cylinetax;
            _invSvc.push_back(linetax);
            linetax->tax[ENUM_SALES_TAX::name]=taxname;
            linetax->tax[ENUM_SALES_TAX::creation]=creation;
            linetax->tax[ENUM_SALES_TAX::modified]=modified;
            linetax->tax[ENUM_SALES_TAX::modified_by]=modified_by;
            linetax->tax[ENUM_SALES_TAX::owner]=owner;
            linetax->tax[ENUM_SALES_TAX::docstatus]=docstatus;

            sprintf(sztmp,"%d",taxidx);
            linetax->tax[ENUM_SALES_TAX::idx]=sztmp;
            linetax->tax[ENUM_SALES_TAX::charge_type]="Actual";
            linetax->tax[ENUM_SALES_TAX::account_head]=account_head;
            linetax->tax[ENUM_SALES_TAX::description]="Service Charge";
            linetax->tax[ENUM_SALES_TAX::included_in_print_rate]="0";
            linetax->tax[ENUM_SALES_TAX::included_in_paid_amount]="0";
            string mainCost = "Main - "; mainCost += _abbr;
            linetax->tax[ENUM_SALES_TAX::cost_center]=mainCost;
            linetax->tax[ENUM_SALES_TAX::rate]="0";
            linetax->tax[ENUM_SALES_TAX::account_currency]=_currency;

            sprintf (sztmp,"%.02f",dtotaltax);
            linetax->tax[ENUM_SALES_TAX::tax_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::base_tax_amount]=sztmp;

            sprintf (sztmp,"%.02f",dtotal);
            linetax->tax[ENUM_SALES_TAX::base_total]=sztmp;
            linetax->tax[ENUM_SALES_TAX::tax_amount_after_discount_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::base_tax_amount_after_discount_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::total]=sztmp;

            linetax->tax[ENUM_SALES_TAX::item_wise_tax_detail]=item_wise_tax_detail;
            linetax->tax[ENUM_SALES_TAX::dont_recompute_tax]="1";
            linetax->tax[ENUM_SALES_TAX::parent]=systransnum;
            linetax->tax[ENUM_SALES_TAX::parentfield]="taxes";
            linetax->tax[ENUM_SALES_TAX::parenttype]="Sales Invoice";
        }
    }

    string base_discount_amount = "0";
    if (_trxDiscount > 0)  {
        char szd [32];
        sprintf (szd, "%.02f", _trxDiscount);
        base_discount_amount = szd;
    }
    _invHdrValue[ENUM_SALES_INVOICE::base_discount_amount]=base_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::is_cash_or_non_trade_discount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::discount_amount]=base_discount_amount;
    return true;
}
