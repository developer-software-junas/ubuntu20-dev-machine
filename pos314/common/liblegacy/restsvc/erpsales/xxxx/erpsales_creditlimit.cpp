/********************************************************************
          FILE:          erpsales_creditlimit.cpp
   DESCRIPTION:          Credit limit check
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
      FUNCTION:         credit_limit
   DESCRIPTION:         Customer credit
 ********************************************************************/
bool                    cyerpsales::credit_limit (std::string party,
                                                  std::string company,
                                                  double &debit, double &credit,
                                                  double &credit_limit, double &outstanding_balance)
{
    string              sql;
    CYDbSql*            erpdb  = _ini->dbconnini("erp");
    if (nullptr == erpdb)
        return seterrormsg(_ini->errormsg().c_str());
    //////////////////////////////////////
    //  Get the credit limit
    erpdb->sql_reset ();
    outstanding_balance = 0.00;
    sql  = "select credit_limit from `tabCustomer Credit Limit` where parent = '";
    sql += party; sql += "' ";
    if (!erpdb->sql_result(sql,false))
        return seterrormsg(erpdb->errordb());
    if (erpdb->eof())
        return seterrormsg("Unable to retrieve the credit limit.  Customer not found.");
    string scredit_limit = erpdb->sql_field_value("credit_limit");
    //////////////////////////////////////
    //  Get the outstanding balance
    erpdb->sql_reset ();
    sql  = "select party, company, sum(debit) as debit, sum(credit) as credit from `tabGL Entry` ";
    sql += "where party_type = 'Customer' and is_cancelled = 0 ";
    sql += "and party = '"; sql += party; sql += "' and company = '";
    sql += company; sql += "' ";
    if (!erpdb->sql_result(sql,false))
        return seterrormsg(erpdb->errordb());
    if (erpdb->eof())
        return seterrormsg("Unable to retrieve the outstanding balance.  Customer not found.");
    string sdebit = erpdb->sql_field_value("debit");
    string scredit = erpdb->sql_field_value("credit");

    debit = _util->stodsafe(sdebit);
    credit = _util->stodsafe(scredit);
    credit_limit = _util->stodsafe(scredit_limit);

    outstanding_balance = credit_limit - (debit - credit);
    return true;
}
