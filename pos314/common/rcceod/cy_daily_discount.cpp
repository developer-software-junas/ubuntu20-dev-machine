/********************************************************************
          FILE:          cy_daily_account.cpp
   DESCRIPTION:          Import account information
 ********************************************************************/
//////////////////////////////////////////
//  Main header
#include "cypostrx.h"
//////////////////////////////////////////
//  Dereference the STD namespace
using std::endl;
using std::string;
using std::vector;
using std::stringstream;
/*******************************************************************
      FUNCTION:         cy_daily_discount
   DESCRIPTION:         Save the discount details
 *******************************************************************/
bool                    cypostrx::cy_daily_discount (CYDbSql* db)
{


    int idx = 1;
    //db->sql_bind_reset();
    string sql09, sql10, tmp;

    db->sql_bind_reset();
    sql09  = "INSERT INTO ";  sql09 += _tblprefix; sql09 += "discount (company_cd,branch_cd,register_num, sys_trans_num,discount_type_cd, discount_cd,";
    sql10  = db->sql_bind(idx,_company_code); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_branch_code); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_register_num); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_systransnum); sql10 += ", "; idx++;

    //////////////////////////////
    //no data source after this line yet
    sql10 += db->sql_bind(idx,_settingstransdisc["discount_type_cd"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["discount_cd"].asString()); sql10 += ", "; idx++;




    sql09 += "description, is_percentage, discount_value, discount_amount,";
    sql10 += db->sql_bind(idx,_settingstransdisc["description"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["disctype"].asString()=="0"? "1" :"0"); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["discvalue"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jtrxdiscount["discount_value"].asString()); sql10 += ", "; idx++;


    sql09 += "is_readonly, min_amount, max_amount, start_date, start_time, ";

    sql10 += db->sql_bind(idx,_settingstransdisc["readonly"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["min_amount"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["max_amount"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["start_date"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["start_time"].asString() ); sql10 += ", "; idx++;




    sql09 += "end_date, end_time, total_discount,  ";
    sql10 += db->sql_bind(idx,_settingstransdisc["end_date"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["end_time"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_util->doubletostring(_transdiscamt)); sql10 += ", "; idx++;


    sql09 += "cust_acct_type, cust_number, cust_company, cust_first, cust_mi, cust_last, cust_location, cust_currency, ";

    if(!_jtrxaccount.empty()){

        sql10 += db->sql_bind(idx,_jsales["cust_acct_type"].asString()); sql10 += ",  "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_number"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_company"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_first"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jtrx["cust_mi"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_last"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_location"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jtrx["cust_currency"].asString()); sql10 += ", "; idx++;

    }else{
        sql10 += db->sql_bind(idx,""); sql10 += ",  "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;

    }




    sql09 += " auth_manager, tin) ";
    sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,""); sql10 += "); "; idx++;



    sql09 += " values (";
    sql09 += sql10;
    if (!db->sql_only ((char*) sql09.c_str(), true))  {
        return  false ;//seterrormsg(db->errordb());
    }

    return true;

}
