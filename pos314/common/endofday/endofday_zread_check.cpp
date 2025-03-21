/********************************************************************
          FILE:         rest_request.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
using std::string;
using std::stringstream;
#include "cyposrest.h"
/*******************************************************************
      FUNCTION:          request
   DESCRIPTION:          Process the REST request
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
string                   cyposrest::rest_zread_check ()
{
    string sql;
    char szfunc [] = "zReadCheck";
    //////////////////////////////////////
    //  Retrieve argument(s)
    string logical_date = cgiform("logical_date");
    string location_code = cgiform("location_code");
    string register_num = cgiform("register_num");
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    //////////////////////////////////////
    //  Make sure all cashiers are done
    Json::Value jline, jcashier;
    jcashier["status"]="ok";
    string cashierList = "Not all cashiers have performed an X reading: ";
    dbsel->sql_bind_reset();
    sql  = "select * from pos_cashier_xread_shift ";
    sql += "where  is_eod = 0 and transaction_date = ";
    sql += dbsel->sql_bind(1, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(2, location_code);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind(3, register_num);
    if (!dbsel->sql_result (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    if (!dbsel->eof ())  {
        int i = 0;
        do  {
            jline["cashier"]=dbsel->sql_field_value ("cashier");
            jline["register_num"]=dbsel->sql_field_value ("register_num");
            cashierList += dbsel->sql_field_value ("cashier");
            cashierList += ", ";
            jcashier["cashier"][i]=jline;
            i++;
            dbsel->sql_next();
        } while (!dbsel->eof ());
        jcashier["error_message"]=cashierList;
    }
    stringstream ss;
    ss << jcashier;
    return ss.str ();
}
