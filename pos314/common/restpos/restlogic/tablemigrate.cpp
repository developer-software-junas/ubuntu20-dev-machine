/********************************************************************
          FILE:         tableMigrate
   DESCRIPTION:         Migrate a table
 ********************************************************************/
//////////////////////////////////////////
//  REST class

#include <string>
#include <sstream>
#include "cysaleshost.h"
#include "cytable.h"
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:         tableMigrate
   DESCRIPTION:         Migrate a table
     ARGUMENTS:         none
       RETURNS:         true on success
 ***************************************************************************/
bool                    cysaleshost::tableMigrate (string tableName,
                                                   string systransnum,
                                                   CYDbSql* dbSource,
                                                   CYDbSql* dbDest,
                                                   bool withIndex)
{
    //////////////////////////////////////
    //  Set the table name
    string columnSelect = "";
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
        if (!sqlTable.columnInfo(withIndex))
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
    ////////////////////////////////////
    //  Select record(s) to migrate
    string sqlins,  sql = "select ";
    sql += columnSelect;
    sql += " from ";
    sql += tableName;
    sql += " where systransnum = '";
    sql += systransnum;
    sql += "' ";

    if (!dbSource->sql_result(sql,false))
        return seterrormsg(dbSource->errordb());
    if (dbSource->eof ())
        return true;

    while (!dbSource->eof())  {
        sqlins  = "insert into ";
        sqlins += tableName;
        sqlins += "(";
        sqlins += columnSelect;
        sqlins += ") values (";

        idx = 0;
        dbDest->sql_bind_reset();
        for (idx = 0; idx < dbSource->numcols(); idx++)  {
            sqlins += dbDest->sql_bind((idx+1),
                dbSource->sql_field_value(dbSource->sql_field_name(idx)));
            if ((idx+1) < dbSource->numcols())
                sqlins += ",";
        }
        sqlins += ")";
        if (!dbDest->sql_only(sqlins,true))
            return seterrormsg(dbDest->errordb());

        dbSource->sql_next();
    }
    return true;
}
