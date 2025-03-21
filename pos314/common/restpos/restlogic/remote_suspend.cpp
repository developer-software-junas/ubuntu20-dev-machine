/********************************************************************
          FILE:         remote_suspend
   DESCRIPTION:         Move a suspended record into the remote db
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
      FUNCTION:         remote_suspend
   DESCRIPTION:         Move a suspended record
 ********************************************************************/
bool                    cysaleshost::remoteSuspend (bool suspendIt, std::string systransnum)
{
    CYDbSql* db;
    CYDbSql* dbsel;
    CYDbSql* dbremote;

    if (suspendIt)  {
        db = _ini->dbconn ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());

        dbsel = _ini->dbconn ();
        if (nullptr == dbsel)
            return seterrormsg(_ini->errormsg());

        dbremote = _ini->dbconnremote();
        if (nullptr == dbremote)
            return seterrormsg(_ini->errormsg());
    }  else  {
        db = _ini->dbconnremote ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());

        dbsel = _ini->dbconnremote ();
        if (nullptr == dbsel)
            return seterrormsg(_ini->errormsg());

        dbremote = _ini->dbconn();
        if (nullptr == dbremote)
            return seterrormsg(_ini->errormsg());
    }
    //////////////////////////////////////
    //  Main loop db connection
    string sql;
    int idx = 0;

    //////////////////////////////////////
    //  Move suspended records to the host
    dbsel->sql_reset();
    string columnList, tableList = tableFilterSuspend();

    sql  = "select * from tg_pos_suspend_header where systransnum = ";
    sql += dbsel->sql_bind(1,systransnum);
    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        idx = 0;
        systransnum = dbsel->sql_field_value("systransnum");
        do  {
            string tableName = g_suspend_tables[idx];
            idx++;
            ////////////////////////////////////
            //  Select record(s) to migrate
            string sql, sqlins, sqlarg="";
            sql = "select * ";
            sql += " from ";
            sql += tableName;
            sql += " where systransnum = '";
            sql += systransnum;
            sql += "' ";

            if (!db->sql_result(sql,false))
                return seterrormsg(db->errordb());
            if (!db->eof ())  {
                dbremote->sql_bind_reset();
                sql = "delete ";
                sql += " from ";
                sql += tableName;
                sql += " where systransnum = '";
                sql += systransnum;
                sql += "' ";

                if (!dbremote->sql_only(sql,false))
                    return seterrormsg(dbremote->errordb());

                do  {
                    sqlins  = "insert into ";
                    sqlins += tableName;
                    sqlins += "(";

                    columnList = "";
                    int y = db->numcols();
                    for (int x = 0; x < y; x++)  {
                        columnList += db->sql_field_name(x);
                        if ((x+1) < y)
                            columnList += ", ";
                    }
                    sqlins += columnList;
                    sqlins += ") values (";

                    dbremote->sql_bind_reset();
                    y = db->numcols();
                    for (int x = 0; x < y; x++)  {
                        string fld = db->sql_field_name(x);
                        string val = db->sql_field_value(fld);
                        sqlins += dbremote->sql_bind((x+1),val);
                        sqlarg += val;
                        if ((x+1) < db->numcols())  {
                            sqlins += ", ";
                            sqlarg += ", ";
                        }
                    }
                    sqlins += ")";
                    _log->logdebug(sqlins.c_str(),sqlarg.c_str());
                    if (!dbremote->sql_only(sqlins,true))
                        return seterrormsg(dbremote->errordb());
                    db->sql_next();
                } while (!db->eof());
            }
        }  while (g_suspend_tables[idx] != "tg_end");

        if (!dbremote->sql_commit())
            return seterrormsg(dbremote->errordb());
    }
    return true;
}
/********************************************************************
      FUNCTION:         remote_suspend
   DESCRIPTION:         Move a suspended record
 ********************************************************************/
bool                    cysaleshost::remoteSuspendCleanup (std::string register_num,
                                                           std::string location_code)
{
    CYDbSql* db;
    CYDbSql* dbsel;

        db = _ini->dbconnremote ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());

        dbsel = _ini->dbconnremote ();
        if (nullptr == dbsel)
            return seterrormsg(_ini->errormsg());

    //////////////////////////////////////
    //  Main loop db connection
    string sql;
    int idx = 0;

    //////////////////////////////////////
    //  Move suspended records to the host
    dbsel->sql_reset();
    string columnList, systransnum,
           tableList = tableFilterSuspend();

    sql  = "select * from tg_pos_suspend_header where location_code = ";
    sql += dbsel->sql_bind(1,location_code);
    sql += " and register_number = ";
    sql += dbsel->sql_bind(2,register_num);
    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    while (!dbsel->eof ())  {
        idx = 0;
        systransnum = dbsel->sql_field_value("systransnum");
        do  {
            string tableName = g_suspend_tables[idx];
            idx++;
            ////////////////////////////////////
            //  Select record(s) to migrate
            string sql;
            sql = "delete ";
            sql += " from ";
            sql += tableName;
            sql += " where systransnum = '";
            sql += systransnum;
            sql += "' ";

            if (!db->sql_only(sql,false))
                return seterrormsg(db->errordb());
        }  while (g_suspend_tables[idx] != "tg_end");
        dbsel->sql_next ();
    }
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
    return true;
}
