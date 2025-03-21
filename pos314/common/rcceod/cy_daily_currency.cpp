/********************************************************************
          FILE:          cy_daily_currency.cpp
   DESCRIPTION:          Import currency information
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
      FUNCTION:         cy_daily_currency
   DESCRIPTION:         Save the currency details
 *******************************************************************/
bool                    cypostrx::cy_daily_currency (CYDbSql* db)
{
    int idx = 1;
    string sql02;
    db->sql_bind_reset();
    if (_jcurrency.isMember("currency_code"))  {
        sql02  = "INSERT INTO ";  sql02 += _tblprefix; sql02 += "currency (company_cd,branch_cd, ";
        sql02 += "register_num,sys_trans_num,currency_cd,currency_desc, ";
        sql02 += "is_multiply, ex_rate, local_ex_rate) values (";
        sql02 += db->sql_bind(idx,_company_code); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_branch_code); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_register_num); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_systransnum); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_jcurrency["currency_code"].asString()); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_jcurrency["description"].asString()); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_jcurrency["is_multiply"].asString()); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,_jcurrency["default_rate"].asString()); sql02 += ", "; idx++;
        sql02 += db->sql_bind(idx,"0"); sql02 += ") ";
        if (!db->sql_only( (char*) sql02.c_str(),true)){
            seterrormsg(db->errordb());
            return false;//seterrormsg(db->errordb());
        }
    }  else  {
        return false;//seterrormsg("No POS currency information found");
    }
    return true;
}
