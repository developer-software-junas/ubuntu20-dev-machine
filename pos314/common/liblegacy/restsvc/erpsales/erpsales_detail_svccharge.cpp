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
      FUNCTION:         export_detail_svccharge
   DESCRIPTION:         Sales tax details
 ********************************************************************/
bool                    cyerpsales::export_detail_svccharge (MySqlDb* taxdb,
                                                             std::string systransnum)
{
    ///////////////////////////////////
    //  Tax detail - madugo!!!
    string sql;
    char sztmp[64];
    cylinetax* linetax;
    Json::Value jdet, jline;
    Json::Value jwise, jwiseval, jtaxlist;
    taxdb->sql_reset();
    stringstream sstax;

    //////////////////////////////////////
    //  Service charge
    jwise.clear();
    taxdb->sql_reset();
    Json::Value jwline;

    sql  = "select * from tg_pos_mobile_detail where systransnum = '";
    sql += systransnum; sql += "' ";
    if (!taxdb->sql_result(sql,false))
        return seterrormsg(taxdb->errordb());
    double dtotal=0,dtotaltax=0;
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
        //////////////////////////////////////
        //  Frappe variable(s)
        string dt = _util->date_mysql();
        //  conversion variables
        string creation=dt, modified=dt, idx="0", docstatus="0",
               modified_by="Administrator", owner="Administrator";
        //////////////////////////////////
        //  TAX breakdown
        int taxidx = 0;
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
            linetax->tax[ENUM_SALES_TAX::dont_recompute_tax]="0";
            linetax->tax[ENUM_SALES_TAX::parent]=systransnum;
            linetax->tax[ENUM_SALES_TAX::parentfield]="taxes";
            linetax->tax[ENUM_SALES_TAX::parenttype]="Sales Invoice";
        }
    }
    _svcCharge += dtotaltax;
    return true;
}

