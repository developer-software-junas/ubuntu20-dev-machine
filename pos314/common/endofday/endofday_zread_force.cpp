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
//*******************************************************************
//    FUNCTION:     retrieveEodData
// DESCRIPTION:     Get relevant data
//*******************************************************************
bool                CYRestReceiptEod::retrieveEodForce()
{
    //  alt json object(s)
    if (!json_settings())
        return false;
    if (!json_settings_branch())
        return false;
    if (!json_settings_company())
        return false;
    if (!json_settings_sysdate())
        return false;
    if (!json_settings_cashier())
        return false;
    if (!json_settings_manager())
        return false;
    if (!json_settings_register())
        return false;
    if (!json_settings_currency())
        return false;

    return true;
}

/*******************************************************************
      FUNCTION:          request
   DESCRIPTION:          Process the REST request
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
string                   cyposrest::rest_zread_force ()
{
    string sql;
    char szfunc [] = "zReadForce";
    //return _util->jsonerror(szfunc);
    //////////////////////////////////////
    //  Retrieve argument(s)
    string cashier = cgiform("manager");
    string manager = cgiform("manager");
    string mgrpass = cgiform("mgrpass");
    if (!managerauth(manager,mgrpass))
        return _util->jsonerror("Invalid manager credentials");
    string register_num = cgiform("register_num");
    string logical_date = cgiform("logical_date");
    string location_code = cgiform("location_code");
    string cashier_shift = "";
    string clear_suspended = "0";
    //////////////////////////////////////
    //  Assign to the EOD variable(s)
    _eod->argXread = false;
    _eod->argManager = manager;
    _eod->argCashier = cashier;
    _eod->argShift = cashier_shift;
    _eod->argRegister = register_num;
    _eod->argSysdate = logical_date;
    _eod->argBranch = location_code;
    _eod->argSuspended = clear_suspended;
    //////////////////////////////////////
    //  Retrieve EOD data
    if (!_eod->retrieveEodForce())
        return errorjson(_eod->errormsg());

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    //////////////////////////////////////
    //  Sysdate...
    string sysdate = _eod->getAltValue("transaction_date",_eod->alt_settings_sysdate);
    string eod_timestamp = _eod->getAltValue("eod_timestamp",_eod->alt_settings_sysdate);

    if (eod_timestamp.length() > 0 && eod_timestamp != "0")
        return _util->jsonerror(szfunc,"End of day has already been completed for the business date requested.");
    if (sysdate != logical_date)
        return _util->jsonerror(szfunc,"The business date requested for terminal reading is invalid.");
    //////////////////////////////////////
    //  Update the cashier record(s)
    db->sql_reset ();
    sql  = "update pos_cashier_xread set is_eod = 1 ";
    sql += "where  transaction_date = ";
    sql += db->sql_bind(1, sysdate);
    sql += " and   location_code = ";
    sql += db->sql_bind(2, location_code);
    sql += " and   register_num = ";
    sql += db->sql_bind(3, register_num);
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Update the sysdate
    string transaction_count = "0";
    string transaction_amount = "0";
    eod_timestamp = _util->strtime_t();
    db->sql_bind_reset();
    sql  = "update pos_sysdate set is_sync = 1, eod_timestamp = ";
    sql += db->sql_bind(1, eod_timestamp);
    sql += ", transaction_count = ";
    sql += db->sql_bind( 2, transaction_count);
    sql += ", transaction_amount = ";
    sql += db->sql_bind( 3, transaction_amount);
    sql += " where transaction_date = ";
    sql += db->sql_bind(4, sysdate);
    sql += " and   location_code = ";
    sql += db->sql_bind(5, location_code);
    sql += " and   register_num = ";
    sql += db->sql_bind(6, register_num);
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Carry over the new total(s)
    string oldSysdate = sysdate;
    if (!_util->int_date_add(sysdate,1))
        return _util->jsonerror(szfunc,"Unable to generate the next business date");

    //////////////////////////////////////
    //  Increment the register totals
    db->sql_bind_reset ();
    sql  = "update pos_register set zreadcount = zreadcount + 1, ";
    sql += " last_sysdate = ";
    sql += db->sql_bind( 1, sysdate);
    sql += ", transaction_count = transaction_count + ";
    sql += db->sql_bind( 2, transaction_count);
    sql += ", transaction_amount = transaction_amount + ";
    sql += db->sql_bind( 3, transaction_amount);
    sql += ", daily_transnum = 1 where location_code = ";
    sql += db->sql_bind( 4, location_code);
    sql += " and register_num = ";
    sql += db->sql_bind( 5, register_num);

    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());

    db->sql_bind_reset();
    sql  = "insert into pos_sysdate (transaction_date, location_code, register_num, ";
    sql += "transaction_count, transaction_amount, is_sync, eod_timestamp) values (";
    sql += db->sql_bind(1, sysdate); sql += ", ";
    sql += db->sql_bind(2, location_code); sql += ", ";
    sql += db->sql_bind(3, register_num); sql += ", 0, 0.00, -888, 0) ";

    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Cleanup table(s)
    db->sql_reset ();
    sql = "truncate table pos_cashier_register ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
/*
    sql = "truncate table pos_cash_fund ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table pos_cash_takeout ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table tg_pos_receipt ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table tg_pos_journal ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());    
*/
    if (!db->sql_commit ())
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Reset the logfile(s)
    _cgi->cleanlogfile(true);
    //////////////////////////////////////
    //  Cleanup sales
    cleanupMobileSales (_eod->argBranch,_eod->argRegister,_eod->argSysdate,_eod->argCashier);
    string output = _util->jsonok("ZRead","Ordering station");
    return output;
}
