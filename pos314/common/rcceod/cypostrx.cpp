/********************************************************************
          FILE:         cypostrx.cpp
   DESCRIPTION:         POS transaction implementation
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
         CLASS:         cypostrx
   DESCRIPTION:         One-time initialization
 *******************************************************************/
cypostrx::cypostrx()
{
    //////////////////////////////////////
    //  Create the settings object
    //_ini = new cyini;
    //_ini->load(g_inifile);
    //////////////////////////////////////
    //  Create the log / utility object
    //_log = new cylog (g_logfile);
    //_util = new cyutility;
}
/*******************************************************************
         CLASS:         cypostrx
   DESCRIPTION:         Class cleanup
 *******************************************************************/
cypostrx::~cypostrx()
{
    /*
    if (nullptr != _log)  delete (_log);
    if (nullptr != _ini)  delete (_ini);
    if (nullptr != _util) delete (_util);
    */
}
/*******************************************************************
      FUNCTION:         seterrormessage
   DESCRIPTION:         Save and log the error message
 *******************************************************************/
bool                    cypostrx::seterrormsg (std::string msg)
{
    (void)msg;
    //_log->logmsg("ERROR",msg.c_str ());
    //_errorMessage = msg;
    return false;
}



/*******************************************************************
      FUNCTION:         saveSqlSales
   DESCRIPTION:         Save the sales details
 *******************************************************************/
bool                    cypostrx::saveSqlSales ()
{

    _tblprefix = "cy_daily_";

    //////////////////////////////////////
    //  Database connectio
    CYDbSql* db = dbconnect();
    if (!db->ok())
        return false;

    //////////////////////////////////////
    //  Account information
    if (!cy_daily_account(db))
        return false;

    if (!cy_daily_account_discount(db))
        return false;

    //////////////////////////////////////
    //  Audit information
    if (!cy_daily_audit(db))
        return false;

    if (!cy_daily_audit_info(db))
        return false;
    if (!cy_daily_audit_display(db))
        return false;
    //////////////////////////////////////
    //  Currency
    if (!cy_daily_currency(db))
        return false;
    //////////////////////////////////////
    //  POS settings
    if (!cy_daily_settings(db))
        return false;
    //////////////////////////////////////
    //  Sales detail
    if (!cy_daily_detail(db))
        return false;
    /////////////////////////////////////
    /// Trans discount
    if (!cy_daily_discount(db))
        return false;

    //////////////////////////////////////
    //  Sales payment
    if (!cy_daily_payment(db))
        return false;
    //////////////////////////////////////
    //  Sales header
    if (!cy_daily_header(db))
        return false;

    //////////////////////////////////////
    //  Save all changes
    if (!db->sql_commit ())
        return false; //seterrormsg(db.errordb());

    //////////////////////////////////////
    //  Sales xread and cash fund
    if (!cy_daily_xread(db))
        return false;



    return true;
}



/*******************************************************************
      FUNCTION:         saveSqlVoid
   DESCRIPTION:         Save the sales details
 *******************************************************************/
bool                    cypostrx::saveSqlVoid ()
{

    _tblprefix = "cy_daily_";

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconnect();
    string   strtemp;
    int      recordexist = 0;
    if (!db->ok())
        return false;


    string sql;
    db->sql_reset();

    sql = "select count(*) as recs from cy_daily_header where sys_trans_num = '";
    sql += _systransnum;
    sql += "'";

    if (!db->sql_result (sql,0)){
        _util->jsonerror(db->errordb().c_str());
        return false;// seterrorquery(db->errordb());
    }



    strtemp = db->sql_field_value("recs").c_str();
    recordexist =    atoi(strtemp.c_str());

    db->sql_reset();


    if(recordexist > 0){

        //////////////////////
        //update void flag
        sql = "update cy_daily_header set is_void = 1, auth_manager = '";
        sql += _jsales["cashier"].asString();
        sql += "' where sys_trans_num = '";
        sql += _systransnum;
        sql += "'";

        if(!db->sql_only(sql.c_str())){
            _util->jsonerror("Failed Updating Void Sales Record. Pls Call Technical Support (1) \n.....Exiting");
            return false ;
        }

        if(!db->sql_commit()){
            _util->jsonerror("Failed Updating Commit on void Sales Record. Pls Call Technical Support (1)\n.....Exiting");

            return false ;
        }



        return true;

    }






    //Otherwise proceed with insert as void
    //////////////////////////////////////
    //  Account information
    if (!cy_daily_account(db))
        return false;

    if (!cy_daily_account_discount(db))
        return false;

    //////////////////////////////////////
    //  Audit information
    if (!cy_daily_audit(db))
        return false;

    if (!cy_daily_audit_info(db))
        return false;
    if (!cy_daily_audit_display(db))
        return false;
    //////////////////////////////////////
    //  Currency'cy_daily_currency(db)

    if (!cy_daily_currency(db))
        return false;

    //////////////////////////////////////
    //  POS settings

    if (!cy_daily_settings(db))
        return false;
    //////////////////////////////////////
    //  Sales detail
    if (!cy_daily_detail(db))
        return false;
    //////////////////////////////////////
    //  Sales payment
    if (!cy_daily_payment(db))
        return false;
    //////////////////////////////////////
    //  Sales header
    if (!cy_daily_header(db, 1))
        return false;





    //////////////////////////////////////
    //  Save all changes
    if (!db->sql_commit ())
        return false; //seterrormsg(db.errordb());

    //////////////////////////////////////
    //  Sales header
    //if (!cy_daily_xread(db))
    //    return false;



    return true;
}

/*******************************************************************
      FUNCTION:         saveSqlRefund
   DESCRIPTION:         Save the refund details
 *******************************************************************/
bool                    cypostrx::saveSqlRefund ()
{

    _tblprefix = "cy_daily_";

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconnect();
    string   strtemp;

    if (!db->ok())
        return false;






    //Otherwise proceed with insert as void
    //////////////////////////////////////
    //  Account information
    if (!cy_daily_account(db))
        return false;

    if (!cy_daily_account_discount(db))
        return false;

    //////////////////////////////////////
    //  Audit information
    if (!cy_daily_audit(db))
        return false;

    if (!cy_daily_audit_info(db))
        return false;
    if (!cy_daily_audit_display(db))
        return false;
    //////////////////////////////////////
    //  Currency'cy_daily_currency(db)

    if (!cy_daily_currency(db))
        return false;

    //////////////////////////////////////
    //  POS settings

    if (!cy_daily_settings(db))
        return false;
    //////////////////////////////////////
    //  Sales detail
    if (!cy_daily_detail(db))
        return false;
    //////////////////////////////////////
    //  Sales payment
    if (!cy_daily_payment(db))
        return false;
    //////////////////////////////////////
    //  Sales header
    if (!cy_daily_header(db, 0,1))
        return false;





    //////////////////////////////////////
    //  Save all changes
    if (!db->sql_commit ())
        return false; //seterrormsg(db.errordb());



    return true;
}

