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
      FUNCTION:         export_detail_senior_tax
   DESCRIPTION:         Sales tax details
 ********************************************************************/
bool                    cyerpsales::export_detail_senior_tax (MySqlDb* taxdb,
                                                       MySqlDb* detdb,
                                                       std::string systransnum)
{
    ///////////////////////////////////
    //  Tax detail - madugo!!!
    string sql;
    int jtl = 0;
    char sztmp[64];
    cylinetax* linetax;
    Json::Value jdet, jline;
    Json::Value jwise, jwiseval, jtaxlist;
    taxdb->sql_reset();
    stringstream sstax;
    //////////////////////////////////////
    //  Frappe variable(s)
    string dt = _util->date_mysql();
    //  conversion variables
    string creation=dt, modified=dt, idx="0", docstatus="0",
           modified_by="Administrator", owner="Administrator";

    sql  = "select distinct(tax_code) from tg_pos_mobile_detail where systransnum = '";
    sql += systransnum; sql += "' ";
    if (!taxdb->sql_result(sql,false))
        return seterrormsg(taxdb->errordb());
    if (taxdb->eof())
        return seterrormsg("1-export_detail_senior_tax::No item tax...");

    jtl = 1;
    int taxidx = 0;
    double dtotal=0,dtotaltax=0;
    do  {
        detdb->sql_reset ();
        string tax_code =  taxdb->sql_field_value("tax_code");
        sql  = "select a.*, b.tax_value, b.description as tax_desc ";
        sql += "from tg_pos_mobile_detail a, cy_tax b ";
        sql += "where a.tax_code = b.tax_code and a.systransnum = '";
        sql += systransnum; sql += "' and a.tax_code = '";
        sql += tax_code; sql += "' ";
        if (!detdb->sql_result(sql,false))
            return seterrormsg(taxdb->errordb());
        if (detdb->eof())
            return seterrormsg("2-export_detail_senior_tax::No item tax...");

        jwise.clear();
        Json::Value jwline;

        std::string jwtaxval;
        std::string tdesc = "";
        dtotal=0,dtotaltax=0;
        string print_rate = "1";
        do  {
            detdb->sql_reset ();
            string tax_code =  taxdb->sql_field_value("tax_code");
            sql  = "select a.*, b.tax_value, b.description as tax_desc ";
            sql += "from tg_pos_mobile_detail a, cy_tax b ";
            sql += "where a.tax_code = b.tax_code and a.systransnum = '";
            sql += systransnum; sql += "' and a.tax_code = '";
            sql += tax_code; sql += "' ";
            if (!detdb->sql_result(sql,false))
                return seterrormsg(taxdb->errordb());
            if (detdb->eof())
                return seterrormsg("3-export_detail_senior_tax::No item tax...");

            jwise.clear();
            Json::Value jwline;

            std::string jwtaxval;
            std::string tdesc = "";
            dtotal=0,dtotaltax=0;
            string print_rate = "1";
            do  {
                string tax_desc  = "VATEX";
                string trx_flag  = detdb->sql_field_value("trx_flag");
                string tax_code  = detdb->sql_field_value("tax_code");
                string tax_value = detdb->sql_field_value("tax_value");

                double dqty = _util->stodsafe(detdb->sql_field_value("quantity"));
                double dretail = _util->stodsafe(detdb->sql_field_value("retail_price"));

                double dtaxamt = _util->stodsafe(jline["less_tax"].asString());
                //double dtrxamt = _util->stodsafe(jline["amount_trx"].asString());
                dretail = dretail - dtaxamt;

                double dlinetax = _util->stodsafe(detdb->sql_field_value("less_tax"));

                double dtrxamt = _util->stodsafe(jline["amount_trx"].asString());
                double dlinetotal = dqty * (dretail - dtrxamt);
                std::string trate="0";
                size_t b = jtaxlist.size();
                for (size_t a = 0; a < b; a++)  {
                    Json::Value jt;
                    jt = jtaxlist[(int)a];
                    string txcode = jt["tax_code"].asString();
                    if (jt["tax_code"].asString() == tax_code)  {
                        trate = tax_value;
                        tdesc = tax_desc;

                        if (_util->stodsafe(trate)==0)  {
                            print_rate = "0";
                            /*
                            if (dqty <= 1)  {
                                dlinetotal = dretail / 1.12;
                            }  else  {
                                dlinetotal = (dretail - (dlinetax+dtrxamt) * dqty;
                            }*/
                            //  rate zero, linetax zero after calculating the line total
                            dlinetax = 0;
                        }
                    }
                }
                dtotal += dlinetotal;
                dtotaltax += dlinetax;

                jwiseval.clear();
                jwiseval[0]=_util->stodsafe(trate);
                jwiseval[1]="0.0";
                string tmp = "`";
                tmp += _util->string_cleanse(detdb->sql_field_value("item_code").c_str(),false);
                tmp += "`";
                jwise[tmp]=jwiseval;
                //_svcCharge += dlinetax;

                detdb->sql_next();
            }  while (!detdb->eof ());
            sstax.clear();
            sstax.str(std::string());
            sstax << jwise;
            string item_wise_tax_detail, iwtd = sstax.str();
            //////////////////////////////////
            //  Remove newline
            char nl[2];
            size_t y = iwtd.size();
            for (size_t x = 0; x < y; x++)  {
                if (iwtd[x]!='\n')  {
                    nl[0] = iwtd[x];
                    nl[1]='\0';
                    item_wise_tax_detail += nl;
                }
            }
            //////////////////////////////////
            //  TAX breakdown
            sprintf(sztmp,"%d",jtl); jtl++;
            string taxname = systransnum; taxname += "-";
            taxname += sztmp; taxname += "-tax";

            string account_head = "Tax ";
            string description = taxdb->sql_field_value("tax_code");
            string rcptType = _ini->get_value("RECEIPT","TYPE");
            if (rcptType != "joel" && rcptType != "rico")  {
                description += " - ";
                description += _abbr;
            }
            account_head += description;
            //////////////////////////////////////
            //  TAX line
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
            linetax->tax[ENUM_SALES_TAX::description]=description;
            linetax->tax[ENUM_SALES_TAX::included_in_print_rate]="0";
            linetax->tax[ENUM_SALES_TAX::included_in_paid_amount]="0";
            string mainCost = "Main - "; mainCost += _abbr;
            linetax->tax[ENUM_SALES_TAX::cost_center]=mainCost;
            linetax->tax[ENUM_SALES_TAX::rate]="0";
            linetax->tax[ENUM_SALES_TAX::account_currency]=_currency;

            sprintf (sztmp,"%.02f",dtotaltax);
            linetax->tax[ENUM_SALES_TAX::tax_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::base_tax_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::tax_amount_after_discount_amount]=sztmp;
            linetax->tax[ENUM_SALES_TAX::base_tax_amount_after_discount_amount]=sztmp;

            sprintf (sztmp,"%.02f",dtotal);
            //linetax->tax[ENUM_SALES_TAX::base_total]=sztmp;
            //linetax->tax[ENUM_SALES_TAX::total]=sztmp;
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
        string rcptType = _ini->get_value("RECEIPT","TYPE");
        if (rcptType != "joel" && rcptType != "rico")  {
            account_head += " - ";
            account_head += _abbr;
        }

/*
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
        linetax->tax[ENUM_SALES_TAX::included_in_print_rate]=print_rate;
        linetax->tax[ENUM_SALES_TAX::included_in_paid_amount]="0";
        string mainCost = "Main - "; mainCost += _abbr;
        linetax->tax[ENUM_SALES_TAX::cost_center]=mainCost;
        linetax->tax[ENUM_SALES_TAX::rate]="0";
        linetax->tax[ENUM_SALES_TAX::account_currency]=_currency;

        sprintf (sztmp,"%.02f",dtotaltax);
        linetax->tax[ENUM_SALES_TAX::tax_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::base_tax_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::tax_amount_after_discount_amount]=sztmp;
        linetax->tax[ENUM_SALES_TAX::base_tax_amount_after_discount_amount]=sztmp;

        sprintf (sztmp,"%.02f",dtotal);
        linetax->tax[ENUM_SALES_TAX::base_total]=sztmp;
        linetax->tax[ENUM_SALES_TAX::total]=sztmp;

        linetax->tax[ENUM_SALES_TAX::item_wise_tax_detail]=item_wise_tax_detail;
        linetax->tax[ENUM_SALES_TAX::dont_recompute_tax]="1";
        linetax->tax[ENUM_SALES_TAX::parent]=systransnum;
        linetax->tax[ENUM_SALES_TAX::parentfield]="taxes";
        linetax->tax[ENUM_SALES_TAX::parenttype]="Sales Invoice";
*/
        taxdb->sql_next ();
    }  while (!taxdb->eof());
    return true;
}

