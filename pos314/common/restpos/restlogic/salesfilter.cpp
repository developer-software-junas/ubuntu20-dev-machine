/********************************************************************
          FILE:         salesfilter
   DESCRIPTION:         Filtered tyable list
 ********************************************************************/
//////////////////////////////////////////
//  REST class

#include <string>
#include <sstream>
#include "cyw_aes.h"
#include "cysaleshost.h"
#include "cyrestreceipt.h"
#include "cytable.h"
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilter ()
{
    string sql;

    sql  = " (";
    sql += "'tg_pos_mobile_fnb', ";
    sql += "'tg_pos_mobile_consign', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_xread_tax', ";
    sql += "'tg_pos_xread_points', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_header_receipt', ";
    sql += "'tg_pos_email_receipt', ";
    sql += "'tg_pos_mobile_alias', ";

    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_modifier', ";
    sql += "'tg_pos_mobile_addon_detail', ";
    sql += "'tg_pos_mobile_discount_detail', ";

    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_gc_detail', ";
    sql += "'tg_pos_refund', ";
    sql += "'tg_pos_mobile_lanebust', ";
    sql += "'tg_pos_refund_detail', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_bank_payment') ";

    return sql;
}
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
bool                    cysaleshost::processReceipt(string type, string systransnum)
{
    //////////////////////////////////////
    //  Schema / remote connection(s)
    _dbSchema = schemaconn();
    _dbRemote = remoteconn();
    if (nullptr == _dbSchema)
        return seterrormsg(_ini->errormsg());
    if (nullptr == _dbRemote)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get schema information for
    //  each required table
    _dbSchema->sql_reset ();
    _dbRemote->sql_reset ();
    //////////////////////////////////////
    //  Set the table name
    string columnSelect = "";
    string tableName = "tg_pos_receipt";
    string dbname = _ini->get_value ("default","NAME");
    CYTable sqlTable (_ini->_env, _ini->_envSchema, dbname, tableName);
    //////////////////////////////////////
    //  Check for pre-cached columns
    int idx = 0;
    string cacheFile = "/tmp/";
    cacheFile += tableName;
    cacheFile += ".cache";
    if (_util->file_exists(cacheFile.c_str ()))  {
        if (!_util->file_load(cacheFile.c_str (),columnSelect))
            return seterrormsg(_util->errormsg());
    }  else  {
        ////////////////////////////////////
        //  Get column information
        if (!sqlTable.columnInfo())
            return seterrormsg(sqlTable.errormsg());
        ////////////////////////////////////
        //  Build the column list
        bool done = false;
        while (!done)  {
            if (sqlTable._column[idx].columnName.length() < 1)
                done = true;
            else  {
                columnSelect += sqlTable._column[idx].columnName;
                if ((idx+1) < g_maxcolumns)  {
                    if (sqlTable._column[(idx+1)].columnName.length() > 0)
                        columnSelect += ",";
                }  else  {
                    done = true;
                }
                idx++;
            }
        }
        _util->file_create(cacheFile.c_str (),columnSelect);
    }
    //////////////////////////////////////
    //  Database connection(s)
    CYDbSql* dbsel = dbconn ();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    ////////////////////////////////////
    //  Select record(s) to migrate
    string sqlins,  sql = "select ";
    sql += columnSelect;
    sql += " from ";
    sql += tableName;
    sql += " where systransnum = '";
    sql += systransnum;
    sql += "' and type = '";
    sql += type;
    sql += "' ";

    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return true;

    string sqldel = "";
    while (!dbsel->eof())  {
        _dbRemote->sql_bind_reset();
        sqldel  = "delete from ";
        sqldel += tableName;
        sqldel += " where systransnum = '";
        sqldel += systransnum;
        sqldel += "' and type = '";
        sqldel += type;
        sqldel += "' ";

        if (!_dbRemote->sql_only(sqldel,false))
            return seterrormsg(_dbRemote->errordb());
        _dbRemote->sql_bind_reset();

        sqlins  = "insert into ";
        sqlins += tableName;
        sqlins += "(";
        sqlins += columnSelect;
        sqlins += ") values (";

        idx = 0;
        _dbRemote->sql_bind_reset();
        for (idx = 0; idx < dbsel->numcols(); idx++)  {
            string tmp = dbsel->sql_field_value(dbsel->sql_field_name(idx));
            sqlins += _dbRemote->sql_bind((idx+1),
                dbsel->sql_field_value(dbsel->sql_field_name(idx)));
            if ((idx+1) < dbsel->numcols())
                sqlins += ",";
        }
        sqlins += ")";
        if (!_dbRemote->sql_only(sqlins,true))
            return seterrormsg(_dbRemote->errordb());

        dbsel->sql_next();
    }
    //////////////////////////////////////
    //  Update the remote connection
    if (!_dbRemote->sql_commit())
        return seterrormsg(_dbRemote->errordb());    
    return true;
}
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
bool                    cysaleshost::processReceiptJournal(string type, string systransnum)
{
    //////////////////////////////////////
    //  Schema / remote connection(s)
    _dbSchema = schemaconn();
    _dbRemote = remoteconn();
    if (nullptr == _dbSchema)
        return seterrormsg(_ini->errormsg());
    if (nullptr == _dbRemote)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get schema information for
    //  each required table
    _dbSchema->sql_reset ();
    _dbRemote->sql_reset ();
    //////////////////////////////////////
    //  Set the table name
    string columnSelect = "";
    string tableName = "tg_pos_journal";
    string dbname = _ini->get_value ("default","NAME");
    CYTable sqlTable (_ini->_env, _ini->_envSchema, dbname, tableName);
    //////////////////////////////////////
    //  Check for pre-cached columns
    int idx = 0;
    string cacheFile = "/tmp/";
    cacheFile += tableName;
    cacheFile += ".cache";
    if (_util->file_exists(cacheFile.c_str ()))  {
        if (!_util->file_load(cacheFile.c_str (),columnSelect))
            return seterrormsg(_util->errormsg());
    }  else  {
        ////////////////////////////////////
        //  Get column information
        if (!sqlTable.columnInfo())
            return seterrormsg(sqlTable.errormsg());
        ////////////////////////////////////
        //  Build the column list
        bool done = false;
        while (!done)  {
            if (sqlTable._column[idx].columnName.length() < 1)
                done = true;
            else  {
                columnSelect += sqlTable._column[idx].columnName;
                if ((idx+1) < g_maxcolumns)  {
                    if (sqlTable._column[(idx+1)].columnName.length() > 0)
                        columnSelect += ",";
                }  else  {
                    done = true;
                }
                idx++;
            }
        }
        _util->file_create(cacheFile.c_str (),columnSelect);
    }
    //////////////////////////////////////
    //  Database connection(s)
    CYDbSql* dbsel = dbconn ();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    ////////////////////////////////////
    //  Select record(s) to migrate
    string sqlins,  sql = "select ";
    sql += columnSelect;
    sql += " from ";
    sql += tableName;
    sql += " where systransnum = '";
    sql += systransnum;
    sql += "' and type = '";
    sql += type;
    sql += "' ";

    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return true;
    string sqldel = "";
    while (!dbsel->eof())  {
        _dbRemote->sql_bind_reset();
        sqldel  = "delete from ";
        sqldel += tableName;
        sqldel += " where systransnum = '";
        sqldel += systransnum;
        sqldel += "' and type = '";
        sqldel += type;
        sqldel += "' ";

        if (!_dbRemote->sql_only(sqldel,false))
            return seterrormsg(_dbRemote->errordb());
        _dbRemote->sql_bind_reset();

        sqlins  = "insert into ";
        sqlins += tableName;
        sqlins += "(";
        sqlins += columnSelect;
        sqlins += ") values (";

        idx = 0;
        _dbRemote->sql_bind_reset();
        for (idx = 0; idx < dbsel->numcols(); idx++)  {
            string tmp = dbsel->sql_field_value(dbsel->sql_field_name(idx));
            sqlins += _dbRemote->sql_bind((idx+1),
                dbsel->sql_field_value(dbsel->sql_field_name(idx)));
            if ((idx+1) < dbsel->numcols())
                sqlins += ",";
        }
        sqlins += ")";
        if (!_dbRemote->sql_only(sqlins,true))
            return seterrormsg(_dbRemote->errordb());

        dbsel->sql_next();
    }
    //////////////////////////////////////
    //  Update the remote connection
    if (!_dbRemote->sql_commit())
        return seterrormsg(_dbRemote->errordb());
    return true;
}
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilterVoid (bool noSalesRecord)
{
    string sql = "(";
    sql += "'tg_pos_void_header' ";
    if (noSalesRecord)  {
        sql += ",'tg_pos_mobile_fnb', ";
        sql += "'tg_pos_mobile_consign', ";
        sql += "'tg_pos_discount_summary', ";
        sql += "'tg_pos_xread_tax', ";
        sql += "'tg_pos_mobile_tax', ";
        sql += "'tg_pos_xread_points', ";
        sql += "'tg_pos_mobile_trxaccount', ";
        sql += "'tg_pos_mobile_transtype', ";
        sql += "'tg_pos_mobile_discount', ";
        sql += "'tg_pos_mobile_currency', ";
        sql += "'tg_pos_mobile_account', ";
        sql += "'tg_pos_mobile_split', ";
        sql += "'tg_pos_mobile_audit', ";
        sql += "'tg_pos_mobile_header_receipt', ";
        sql += "'tg_pos_email_receipt', ";
        sql += "'tg_pos_mobile_alias', ";

        sql += "'tg_pos_mobile_detail', ";
        sql += "'tg_pos_mobile_modifier', ";
        sql += "'tg_pos_mobile_addon_detail', ";
        sql += "'tg_pos_mobile_discount_detail', ";

        sql += "'tg_retail_split', ";
        sql += "'tg_retail_split_trial', ";
        sql += "'tg_pos_mobile_detail_split', ";

        sql += "'tg_pos_mobile_payment', ";
        sql += "'tg_pos_mobile_gc_payment', ";
        sql += "'tg_pos_gc_detail', ";
        sql += "'tg_pos_refund', ";
        sql += "'tg_pos_mobile_lanebust', ";
        sql += "'tg_pos_refund_detail', ";
        sql += "'tg_pos_mobile_discount_payment', ";
        sql += "'tg_pos_mobile_bank_payment' ";
    }
    sql += ") ";
    return sql;
}
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilterCancel ()
{
    string sql;

    sql  = " (";
    sql += "'tg_pos_cancel_header', ";


    sql += "'tg_pos_mobile_fnb', ";
    sql += "'tg_pos_mobile_consign', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_xread_tax', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_xread_points', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_header_receipt', ";
    sql += "'tg_pos_email_receipt', ";
    sql += "'tg_pos_mobile_alias', ";

    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_modifier', ";
    sql += "'tg_pos_mobile_addon_detail', ";
    sql += "'tg_pos_mobile_discount_detail', ";

    sql += "'tg_retail_split', ";
    sql += "'tg_retail_split_trial', ";
    sql += "'tg_pos_mobile_detail_split', ";

    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_gc_detail', ";
    sql += "'tg_pos_refund', ";
    sql += "'tg_pos_refund_detail', ";
    sql += "'tg_pos_mobile_lanebust', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_bank_payment') ";

    return sql;
}

/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilterRefund ()
{
    string sql;

    sql  = " (";
    sql += "'tg_pos_refund_header', ";
    sql += "'tg_pos_mobile_fnb', ";
    sql += "'tg_pos_mobile_consign', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_xread_tax', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_xread_points', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_header_receipt', ";
    sql += "'tg_pos_email_receipt', ";
    sql += "'tg_pos_mobile_alias', ";

    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_modifier', ";
    sql += "'tg_pos_mobile_addon_detail', ";
    sql += "'tg_pos_mobile_discount_detail', ";

    sql += "'tg_retail_split', ";
    sql += "'tg_retail_split_trial', ";
    sql += "'tg_pos_mobile_detail_split', ";

    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_gc_detail', ";
    sql += "'tg_pos_refund', ";
    sql += "'tg_pos_refund_detail', ";
    sql += "'tg_pos_mobile_lanebust', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_bank_payment') ";

    return sql;
}

/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilterHeader ()
{
    string sql;

    sql  = " (";
    sql += "'tg_pos_daily_header', ";



    sql += "'tg_pos_mobile_fnb', ";
    sql += "'tg_pos_mobile_consign', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_xread_tax', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_xread_points', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_header_receipt', ";
    sql += "'tg_pos_email_receipt', ";
    sql += "'tg_pos_mobile_alias', ";

    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_modifier', ";
    sql += "'tg_pos_mobile_addon_detail', ";
    sql += "'tg_pos_mobile_discount_detail', ";

    sql += "'tg_retail_split', ";
    sql += "'tg_retail_split_trial', ";
    sql += "'tg_pos_mobile_detail_split', ";

    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_gc_detail', ";
    sql += "'tg_pos_refund', ";
    sql += "'tg_pos_refund_detail', ";
    sql += "'tg_pos_mobile_lanebust', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_bank_payment') ";

    return sql;
}

/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Include table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  cysaleshost::tableFilterSuspend ()
{
    string sql;

    sql  = " (";
    sql += "'tg_pos_suspend_header', ";



    sql += "'tg_pos_mobile_fnb', ";
    sql += "'tg_pos_mobile_consign', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_xread_tax', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_xread_points', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_header_receipt', ";
    sql += "'tg_pos_email_receipt', ";
    sql += "'tg_pos_mobile_alias', ";

    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_modifier', ";
    sql += "'tg_pos_mobile_addon_detail', ";
    sql += "'tg_pos_mobile_discount_detail', ";

    sql += "'tg_retail_split', ";
    sql += "'tg_retail_split_trial', ";
    sql += "'tg_pos_mobile_detail_split', ";

    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_gc_detail', ";
    sql += "'tg_pos_refund', ";
    sql += "'tg_pos_refund_detail', ";
    sql += "'tg_pos_mobile_lanebust', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_bank_payment') ";

    return sql;
}
