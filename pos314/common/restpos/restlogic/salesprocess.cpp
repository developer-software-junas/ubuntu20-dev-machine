/********************************************************************
          FILE:         processbom
   DESCRIPTION:         Process the BOM table(s)
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
/********************************************************************
      FUNCTION:         checkSales
   DESCRIPTION:         Check for unprocessed sales record(s)
 ********************************************************************/
bool                    cysaleshost::checkSales ()
{
    //////////////////////////////////////
    //  Main loop db connection
    string sql, systransnum;
    CYDbSql* db = dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());

    CYDbSql* dbsel = dbconn ();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Check for sales record(s) that
    //  need to be moved
    dbsel->sql_reset();
    sql  = "select * from tg_pos_daily_header where is_polled2 <> 888 limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processSales(systransnum,SRT_DAILY))
                return false;
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    dbsel->sql_reset();
    sql  = "select * from tg_pos_receipt where is_polled2 <> 888 limit 10  ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            string type = dbsel->sql_field_value("type");
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processReceipt(type,systransnum))
                return false;

            db->sql_reset ();
            sql = "update tg_pos_receipt ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            sql += " and type = ";
            sql += db->sql_bind(2,type);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    dbsel->sql_reset();
    sql  = "select * from tg_pos_journal where is_polled2 <> 888 limit 10  ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            string type = dbsel->sql_field_value("type");
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processReceiptJournal(type,systransnum))
                return false;

            db->sql_reset ();
            sql = "update tg_pos_journal ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            sql += " and type = ";
            sql += db->sql_bind(2,type);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    dbsel->sql_reset();
    sql  = "select * from tg_pos_void_header where is_polled2 <> 888  limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processSales(systransnum,SRT_POSTVOID))
                return false;
            db->sql_reset ();
            sql = "update tg_pos_void_header ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    dbsel->sql_reset();
    sql  = "select * from tg_pos_refund_header where is_polled2 <> 888  limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processSales(systransnum,SRT_REFUND))
                return false;
            db->sql_reset ();
            sql = "update tg_pos_refund_header ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    return true;
}
/********************************************************************
      FUNCTION:         processSales
   DESCRIPTION:         Process the sales record
 ********************************************************************/
bool                    cysaleshost::processSales (string systransnum,
                                                   salesReceiptType type)
{
    //////////////////////////////////////
    //  Schema / remote connection(s)
    _dbSchema = schemaconn();
    _dbRemote = remoteconn();
    if (nullptr == _dbSchema)
        return seterrormsg("Unable to connect to the schema data source");
    if (nullptr == _dbRemote)
        return seterrormsg("Unable to connect to the remote data source");
    CYDbSql* db = dbconn();
    if (nullptr == db)
        return seterrormsg("Unable to connect to the database");
    //////////////////////////////////////
    //  See what filter to apply for VOID
    string sql = "select systransnum from tg_pos_daily_header where systransnum = ";
    sql += _dbRemote->sql_bind(1, systransnum);
    if (!_dbRemote->sql_result(sql,true))
        return seterrormsg(_dbRemote->errordb());
    bool noSalesRecord = _dbRemote->eof();
    //////////////////////////////////////
    //  List of tables to process
    string tableName, tableList;
    if (type == SRT_DAILY)
        tableList = tableFilterHeader();
    else if (type == SRT_CANCEL)
        tableList = tableFilterCancel();
    else if (type == SRT_REFUND)
        tableList = tableFilterRefund();
    else if (type == SRT_POSTVOID)  {
        tableList = tableFilterVoid(noSalesRecord);
    }  else if (type == SRT_SUSPEND)
        tableList = tableFilterSuspend();
    else  {
        return seterrormsg("Invalid sales header type");
    }
    //////////////////////////////////////
    //  Get schema information for
    //  each required table
    _dbSchema->sql_reset ();
    _dbRemote->sql_reset ();
    string schemaName = _ini->get_value("default","NAME");
    sql = "select * from TABLES where TABLE_SCHEMA = '";
    sql += schemaName;
    sql += "' and TABLE_NAME IN ";
    sql += tableList;

    if (!_dbSchema->sql_result(sql,false))
        return seterrormsg(_dbSchema->errordb());
    if (_dbSchema->eof ())
        return seterrormsg("No table(s) found to migrate");

    while (!_dbSchema->eof())  {
        tableName = _dbSchema->sql_field_value("TABLE_NAME");
        if (!tableMigrate(tableName,systransnum,db,_dbRemote))
            return false;
        _dbSchema->sql_next();
    }
    //////////////////////////////////////
    //  Update the remote connection
    if (!_dbRemote->sql_commit())
        return seterrormsg(_dbRemote->errordb());
    //////////////////////////////////////
    //  Mark the record as saved
    if (type == SRT_DAILY)
        tableName = "tg_pos_daily_header";
    else if (type == SRT_CANCEL)
        tableName = "tg_pos_cancel_header";
    else if (type == SRT_REFUND)
        tableName = "tg_pos_refund_header";
    else if (type == SRT_POSTVOID)
        tableName = "tg_pos_void_header";
    else if (type == SRT_SUSPEND)
        tableName = "tg_pos_suspend_header";


    db->sql_reset ();
    sql = "update ";
    sql += tableName;
    sql += " set is_polled2 = 888 where systransnum = ";
    sql += db->sql_bind(1,systransnum);
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    //////////////////////////////////////
    //  Save the changes
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
    //////////////////////////////////////
    //  HACK!!! remove suspended records
    //  that have been transferred
    if (type == SRT_SUSPEND)  {
        //////////////////////////////////////
        //  Get schema information for
        //  each required table
        db->sql_reset ();
        _dbSchema->sql_reset ();
        _dbRemote->sql_reset ();
        string schemaName = _ini->get_value("default","NAME");
        sql = "select * from TABLES where TABLE_SCHEMA = '";
        sql += schemaName;
        sql += "' and TABLE_NAME IN ";
        sql += tableList;

        if (!_dbSchema->sql_result(sql,false))
            return seterrormsg(_dbSchema->errordb());
        if (_dbSchema->eof ())
            return seterrormsg("No table(s) found to migrate");

        while (!_dbSchema->eof())  {
            db->sql_bind_reset();
            tableName = _dbSchema->sql_field_value("TABLE_NAME");
            sql = "delete from ";
            sql += tableName;
            sql += " where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            _dbSchema->sql_next();
        }
        if (!db->sql_commit())
            return seterrormsg(db->errordb());
    }
    return true;
}
