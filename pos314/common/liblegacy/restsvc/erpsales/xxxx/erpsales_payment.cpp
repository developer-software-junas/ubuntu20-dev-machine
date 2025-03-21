/********************************************************************
          FILE:          erpsales_payment.cpp
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
      FUNCTION:         export_payment
   DESCRIPTION:         Sales payment
 ********************************************************************/
bool                    cyerpsales::export_payment (enumHeader type,
                                                    Json::Value jheader,
                                                    std::string systransnum)
{
    (void)type;
    //////////////////////////////////////
    //  Detail exists?
    if (!jheader.isMember("pay_sales"))
        return true;
    //////////////////////////////////////
    //  Loop through each payment record
    char sztmp[64];
    cylinepayment* payment;
    Json::Value jpay, jline;
    jpay = jheader["pay_sales"];
    int j = (int)jpay.size();
    for (int npay = 0; npay < j; npay++)  {
        //////////////////////////////////////
        //  Create a detail record
        jline = jpay[npay];
        payment = new cylinepayment;
        _invPayment.push_back(payment);
        //////////////////////////////////////
        //  Check for payment terms
        Json::Value jcheck;
        if (jline.isMember("tender"))  {
            jcheck = jline["tender"];
            string payterms = jcheck["payment_terms"].asString();

            if (payterms.length() > 0)  {
                _terms = true;
                _invHdrValue[ENUM_SALES_INVOICE::is_pos]="0";
                _invHdrValue[ENUM_SALES_INVOICE::payment_terms_template]=payterms;
            }
        }

        //////////////////////////////////////
        //  Frappe variable(s)
        string dt = _util->date_mysql();
        //  conversion variables
        string creation=dt, modified=dt, idx="0", docstatus="0",
               modified_by="Administrator", owner="Administrator";
        //////////////////////////////////////
        //  Index / item sequence
        sprintf (sztmp, "%04d", (npay+1));
        idx = sztmp;
        string name = systransnum; name += "-"; name += sztmp;
        //////////////////////////////////////
        //  Populate the frappe column(s)
        payment->line[ENUM_SALES_PAY::name]=name;
        payment->line[ENUM_SALES_PAY::creation]=creation;
        payment->line[ENUM_SALES_PAY::modified]=modified;
        payment->line[ENUM_SALES_PAY::modified_by]=modified_by;
        payment->line[ENUM_SALES_PAY::owner]=owner;
        payment->line[ENUM_SALES_PAY::docstatus]=docstatus;
        payment->line[ENUM_SALES_PAY::idx]=idx;

        //payment->line[ENUM_SALES_PAY::xdefault]="0";
        payment->line[ENUM_SALES_PAY::mode_of_payment]=jline["pay_type_desc"].asString();
        payment->line[ENUM_SALES_PAY::amount]=jline["payment_amount"].asString();
        payment->line[ENUM_SALES_PAY::reference_no]=jline["approval_code"].asString();
        //  Tender account
        payment->line[ENUM_SALES_PAY::account]="Cash - JT";  //TODO: put mode of payment account settings in pos db
        //  Type
        Json::Value jtender;
        if (!jline.isMember("tender"))
            return seterrormsg("No tender information found");
        jtender = jline["tender"];
        payment->line[ENUM_SALES_PAY::type]="General";//jtender["description"].asString();

        payment->line[ENUM_SALES_PAY::base_amount]=jline["payment_amount"].asString();
        payment->line[ENUM_SALES_PAY::parent]=systransnum;
        payment->line[ENUM_SALES_PAY::parentfield]="payments";
        payment->line[ENUM_SALES_PAY::parenttype]="Sales Invoice";
    }
    return true;
}
