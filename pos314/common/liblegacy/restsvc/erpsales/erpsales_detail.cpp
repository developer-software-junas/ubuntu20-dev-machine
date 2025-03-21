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
bool                    cyerpsales::export_detail (enumHeader type,
                                                   Json::Value jheader,
                                                   std::string systransnum)
{
    (void)type;
    //////////////////////////////////////
    //  Database connection(s)
    MySqlDb xposdb (_pos);
    MySqlDb xtaxdb (_pos);
    MySqlDb xdetdb (_pos);
    MySqlDb* posdb = &xposdb;
    MySqlDb* taxdb = &xtaxdb;
    MySqlDb* detdb = &xdetdb;
    //////////////////////////////////////
    //  Cleanup and reset
    cleanup_tax();
    cleanup_svc();
    cleanup_detail();    
    _svcCharge = 0.00;
    //////////////////////////////////////
    //  Detail exists?
    if (!jheader.isMember("det_sales"))
        return true;
    //////////////////////////////////////
    //  Update the tax code(s)
    //  for senior line items
    string sql;
    taxdb->sql_reset();
    sql  = "update tg_pos_mobile_detail set tax_code = '0 - JT' where systransnum = '";
    sql += systransnum; sql += "' and trx_flag = 'senior' ";
    if (!taxdb->sql_only(sql,false))
        return seterrormsg(taxdb->errordb());
    if (!taxdb->sql_commit())
        return seterrormsg("Unable to update the tax flag");
    //////////////////////////////////////
    //  Loop through each detail record    
    char sztmp[64];
    cylinedetail* detail;
    Json::Value jdet,
                jline;
    Json::Value jwise,
                jwiseval;

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
        stringstream xx;
        xx << jline;
        printf("\n%s\n",xx.str().c_str());
        detail = new cylinedetail;
        //////////////////////////////////////
        //  Index / item sequence
        sprintf (sztmp, "%04d", (ndet+1));
        idx = sztmp;
        string invname = systransnum; invname += "-"; invname += sztmp;
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
        //////////////////////////////////
        //  Detail record type???
        //////////////////////////////////
        string trx_flag = jline["trx_flag"].asString();
        if (trx_flag == "senior")  {
            if (!export_item_senior (type,jline,detail,systransnum))
                return false;
        }  else if (_isDiplomat)  {
            if (!export_item_diplomat(type,jline,detail,systransnum))
                return false;
        }  else  {
            if (!export_item_regular (type,jline,detail,systransnum))
                return false;
        }

        _invDetail.push_back(detail);
    }
    ///////////////////////////////////
    //  Tax detail
    if (_isPwd || _isSenior || _isDiplomat)  {
        if (!export_detail_senior_tax(taxdb,detdb,systransnum))
            return false;
    }  else  {
        if (!export_detail_tax(taxdb,detdb,systransnum))
            return false;
    }
    if (!export_detail_svccharge(taxdb,systransnum))
        return false;

    string base_discount_amount = "0";
    _invHdrValue[ENUM_SALES_INVOICE::base_discount_amount]=base_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::is_cash_or_non_trade_discount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::discount_amount]=base_discount_amount;
    //////////////////////////////////////
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
    _invHdrValue[ENUM_SALES_INVOICE::amount_eligible_for_commission]=base_net_total;


    sprintf (sztmp, "%.02f", (vat_amount + _svcCharge));
    _invHdrValue[ENUM_SALES_INVOICE::total_taxes_and_charges]=sztmp;
    _invHdrValue[ENUM_SALES_INVOICE::base_total_taxes_and_charges]=sztmp;

    _invHdrValue[ENUM_SALES_INVOICE::base_in_words]="";
    _invHdrValue[ENUM_SALES_INVOICE::rounding_adjustment]="0";
    _invHdrValue[ENUM_SALES_INVOICE::base_rounding_adjustment]="0";



    _invHdrValue[ENUM_SALES_INVOICE::use_company_roundoff_cost_center]="0";
    _invHdrValue[ENUM_SALES_INVOICE::in_words]="";
    _invHdrValue[ENUM_SALES_INVOICE::total_advance]="0";
    _invHdrValue[ENUM_SALES_INVOICE::outstanding_amount]="0";
    _invHdrValue[ENUM_SALES_INVOICE::disable_rounded_total]="1";
    //////////////////////////////////////
    //  Gross sales amount
    if (!jheader.isMember("cytotal_gross_amount"))
        return seterrormsg("JSON SALES: Total gross sales missing");
    string gross_amount = jheader["cytotal_gross_amount"].asString();
    double dgross_amount = _util->stodsafe(gross_amount);
    if (_isPwd || _isSenior || _isDiplomat)  {
        //double total_amount_exempt = _util->stodsafe(jheader["cytotal_amt_exempt"].asString());
        //dgross_amount -= total_amount_exempt;
        _invHdrValue[ENUM_SALES_INVOICE::ignore_pricing_rule]="1";
    }  else  {
        dgross_amount -= vat_amount;
        _invHdrValue[ENUM_SALES_INVOICE::ignore_pricing_rule]="0";
    }
    sprintf (sztmp, "%.02f", dgross_amount);
    gross_amount = sztmp;
    _invHdrValue[ENUM_SALES_INVOICE::total]=gross_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_total]=gross_amount;
    _invHdrValue[ENUM_SALES_INVOICE::rounded_total]=gross_amount;
    _invHdrValue[ENUM_SALES_INVOICE::grand_total]=gross_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_grand_total]=gross_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_rounded_total]=gross_amount;
    //////////////////////////////////////
    //  Global discount
    _invHdrValue[ENUM_SALES_INVOICE::apply_discount_on]="Grand Total";

    string global_discount_amount = "0";
    if (_trxDiscount > 0)  {
        char szd [32];
        sprintf (szd, "%.02f", _trxDiscount);
        global_discount_amount = szd;

        //double dpct = (_trxDiscount / dgross_amount) * 100;
        //sprintf (szd, "%.02f", dpct);
        //global_discount_amount = szd;
    }
    _invHdrValue[ENUM_SALES_INVOICE::base_discount_amount]=global_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::is_cash_or_non_trade_discount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::discount_amount]=global_discount_amount;
    _invHdrValue[ENUM_SALES_INVOICE::additional_discount_account]="";
    _invHdrValue[ENUM_SALES_INVOICE::additional_discount_percentage]="0";

    _invHdrValue[ENUM_SALES_INVOICE::other_charges_calculation]="<pre></pre>";
    _invHdrValue[ENUM_SALES_INVOICE::total_billing_hours]="0";
    _invHdrValue[ENUM_SALES_INVOICE::total_billing_amount]="0";

    _invHdrValue[ENUM_SALES_INVOICE::cash_bank_account]="";
    //  Payment / change
    Json::Value jpay;
    string change_amount, payment_amount;
    double dpaid_amount=0, dchange_amount=0;
    if (jheader.isMember("pay_sales"))  {
        jline.clear ();
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

    dchange_amount = dpaid_amount - dbase_net_total;
    sprintf (sztmp, "%.02f", dchange_amount);
    change_amount = sztmp;

    _invHdrValue[ENUM_SALES_INVOICE::paid_amount]=payment_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_paid_amount]=payment_amount;
    _invHdrValue[ENUM_SALES_INVOICE::change_amount]=change_amount;
    _invHdrValue[ENUM_SALES_INVOICE::base_change_amount]=change_amount;
    _invHdrValue[ENUM_SALES_INVOICE::account_for_change_amount]=_account_for_change_amount;

    //_invHdrValue[ENUM_SALES_INVOICE::customer_group]="Family";
    return true;
}
