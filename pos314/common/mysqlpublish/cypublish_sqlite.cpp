/********************************************************************
          FILE:         cypublish_masterfile.cpp
   DESCRIPTION:         Masterfile publish implementation
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cypublish.h"
#include "cytable.h"
#include <string>
#include <sstream>
using std::string;
using std::stringstream;
/*******************************************************************
      FUNCTION:          publishMasterFile
   DESCRIPTION:          Create the masterfile for POS download
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     CYPublish::publishSqlite()
{   
    //////////////////////////////////////
    //  sysdate
    if (!getLogicalDate(_logicalDate,_locationCode))
        return false;
    //////////////////////////////////////
    //  Database schema
    _cmdLine->cleanupFiles();
    //////////////////////////////////////
    //  schema database
    CYDbSql* dbs = schemaconn();
    if (!dbs->ok())
        return seterrormsg(dbs->errordb());
    //////////////////////////////////////
    //  retrieve all tables
    string dbname = _ini->get_value("default","NAME");
    string sql = "select * from TABLES where TABLE_SCHEMA = '";
    sql += dbname; sql += "' ";
    if (!dbs->sql_result(sql,false))
        return seterrormsg(dbs->errordb());
    if (dbs->eof ())
        return seterrormsg("No table(s) found to migrate");
    //////////////////////////////////////
    //  For each table
    char sztmp[250];
    stringstream ss;
    bool resetBatchFile = true;
    time_t diff, ttStart, ttEnd, allStart, allEnd;
    allStart = time(NULL);
    publishLog("PUBLISH", "Starting the POS publish task");

    while (!dbs->eof ())  {
      string table_name = dbs->sql_field_value("TABLE_NAME");
      CYTable sqlTable (_env, _schema, dbname, table_name);
      ////////////////////////////////////
      //  Start the timer
      ttStart = time (NULL);
      ////////////////////////////////////
      //  Get the table name
      table_name = dbs->sql_field_value("TABLE_NAME");
      publishLog("TABLE", table_name.c_str ());
      sqlTable.setTableName (table_name);
      ////////////////////////////////////
      //  Get column information
      if (!sqlTable.columnInfo())
          return false;
      ////////////////////////////////////
      //  Create the SQL batch file
      string columnList = "";
      if (!appendLiteSchema(resetBatchFile,&sqlTable,columnList))
          return false;
      else
          resetBatchFile = false;

      ttEnd = time (NULL);
      diff = ttEnd - ttStart;
      sprintf (sztmp, "Table: %s took %ld seconds to export\n", table_name.c_str (), diff);
      publishLog("PUBLISH", sztmp);

      dbs->sql_next();
      if (!dbs->eof ())
          table_name = dbs->sql_field_value("TABLE_NAME");
    };
    publishLog("PUBLISH", "Done with schema export!!!");


    //////////////////////////////////////
    //  retrieve all tables
    sql = "select * from TABLES where TABLE_SCHEMA = '";
    sql += dbname;
    sql += "' and TABLE_NAME IN ";
    sql += tableFilter(); sql += " ";
    if (!dbs->sql_result(sql,false))
        return seterrormsg(dbs->errordb());
    if (dbs->eof ())
        return seterrormsg("No table(s) found to migrate");
    //////////////////////////////////////
    //  For each table
    allStart = time(NULL);
    publishLog("PUBLISH", "Starting the POS export task");

    while (!dbs->eof ())  {
      string table_name = dbs->sql_field_value("TABLE_NAME");
      CYTable sqlTable (_env, _schema, dbname, table_name);
      ////////////////////////////////////
      //  Start the timer
      ttStart = time (NULL);
      ////////////////////////////////////
      //  Get the table name
      table_name = dbs->sql_field_value("TABLE_NAME");
      publishLog("TABLE", table_name.c_str ());
      sqlTable.setTableName (table_name);
      ////////////////////////////////////
      //  Get column information
      //if (!sqlTable.columnInfo())
          //return false;
      ////////////////////////////////////
      //  Create the SQL batch file
      string columnList = "";
      if (!appendLiteImport(&sqlTable,columnList))
          return false;

      ttEnd = time (NULL);
      diff = ttEnd - ttStart;
      sprintf (sztmp, "Table: %s took %ld seconds to export\n", table_name.c_str (), diff);
      publishLog("PUBLISH", sztmp);

      dbs->sql_next();
      if (!dbs->eof ())
          table_name = dbs->sql_field_value("TABLE_NAME");
    };
    publishLog("PUBLISH", "Done with sqlite export!!!");
    //////////////////////////////////////
    //  Create the SQLITE database
    ttStart = time(NULL);
    if (!_cmdLine->generateSqlite(_locationCode,_logicalDate))  {
        return false;
    }
    ttEnd = time(NULL);
    diff = ttEnd - ttStart;
    sprintf (sztmp, "Creation of the SQLITE database took %ld seconds\n", diff);
    publishLog("PUBLISH", sztmp);
    //////////////////////////////////////
    //  Execution time
    allEnd = time (NULL);
    sprintf (sztmp, "The database backup took %ld seconds to process\n", (allEnd - allStart));
    publishLog("PUBLISH DONE!!!", sztmp);
    return true;
}
