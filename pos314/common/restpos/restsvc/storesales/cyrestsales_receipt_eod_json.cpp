//*******************************************************************
//        FILE:     cyrestsales_receipt_eod.cpp
// DESCRIPTION:     Sales receipt generation X AND Z RECEIPT
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
//*******************************************************************
//    FUNCTION:     checkDate
// DESCRIPTION:     min / max date(s)
//*******************************************************************
bool                CYRestReceiptEod::json_settings ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from pos_settings ";
    if (!dbsel->sql_result (sql,false))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:     json_settings_company
// DESCRIPTION:     Get company info
//*******************************************************************
bool                CYRestReceiptEod::json_settings_company ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from cy_company ";
    if (!dbsel->sql_result (sql,false))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_company.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:     json_settings_sysdate
// DESCRIPTION:     Get the logical date
//*******************************************************************
bool                CYRestReceiptEod::json_settings_sysdate ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from pos_sysdate ";
    sql += "where location_code = ";
    sql += dbsel->sql_bind(1, argBranch);
    sql += " and  register_num = ";
    sql += dbsel->sql_bind(2, argRegister);
    sql += " order by transaction_date desc ";

    if (!dbsel->sql_result (sql,true))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_sysdate.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:     json_settings_currency
// DESCRIPTION:     Get currency info
//*******************************************************************
bool                CYRestReceiptEod::json_settings_currency ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from cy_currency ";
    if (!dbsel->sql_result (sql,false))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_currency.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         json_settings_branch
// DESCRIPTION:         Branch information
//*******************************************************************
bool                    CYRestReceiptEod::json_settings_branch ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from cy_location where location_code = ";
    sql += dbsel->sql_bind(1,argBranch);

    if (!dbsel->sql_result (sql,true))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_branch.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         json_settings_cashier
// DESCRIPTION:         Branch information
//*******************************************************************
bool                    CYRestReceiptEod::json_settings_cashier ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from cy_user where login = ";
    sql += dbsel->sql_bind(1,argCashier);

    if (!dbsel->sql_result (sql,true))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_cashier.push_back(ptr);
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         json_settings_manager
// DESCRIPTION:         Manager information
//*******************************************************************
bool                    CYRestReceiptEod::json_settings_manager ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from cy_user where login = ";
    sql += dbsel->sql_bind(1,argManager);

    if (!dbsel->sql_result (sql,true))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_manager.push_back(ptr);
        }
    }  else  {
        STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
        ptr->key = "login";
        ptr->val = "cyware";
        alt_settings_manager.push_back(ptr);

        ptr = new STRUCT_ALTJSON;
        ptr->key = "first_name";
        ptr->val = "CYWare";
        alt_settings_manager.push_back(ptr);

        ptr = new STRUCT_ALTJSON;
        ptr->key = "last_name";
        ptr->val = "Admin";
        alt_settings_manager.push_back(ptr);
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         json_settings_register
// DESCRIPTION:         Register information
//*******************************************************************
bool                    CYRestReceiptEod::json_settings_register ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    string sql  = "select * from pos_register where location_code = ";
    sql += dbsel->sql_bind(1,argBranch);
    sql += " and register_num = ";
    sql += dbsel->sql_bind(2,argRegister);

    if (!dbsel->sql_result (sql,true))   {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        for (int x = 0; x < dbsel->numcols(); x++)  {
            STRUCT_ALTJSON* ptr = new STRUCT_ALTJSON;
            ptr->key = dbsel->sql_field_name(x);
            ptr->val = dbsel->sql_field_value (ptr->key);
            alt_settings_register.push_back(ptr);
        }
    }
    return true;
}
