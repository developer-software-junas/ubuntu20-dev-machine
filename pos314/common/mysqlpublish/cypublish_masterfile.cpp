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
bool                     CYPublish::publishMasterFile()
{   
    //////////////////////////////////////
    //  sysdate
    if (!getLogicalDate(_logicalDate,_locationCode))
        return false;
    //////////////////////////////////////
    //  Make sure the file exist(s)
    string filePath = _ini->get_value("PATH","DOCROOT");
    string fileName = "/"; fileName += _locationCode;
    fileName += "."; fileName += _logicalDate; fileName += ".zip";

    filePath += fileName;
    if (_util->file_exists(filePath.c_str()))  {
        //  No backup needed, check for sales upload(s)
        return processSales();
    }
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* db;
    db = _ini->dbconnini();
    if (nullptr == db)  {
        return seterrormsg(_ini->errormsg());
    }
    db->sql_reset ();
    sql  = "delete from tg_pos_master_offline ";
    sql += "where  branch_code = '"; sql += _locationCode; sql += "' ";
    sql += "and    transaction_date = '"; sql += _logicalDate; sql += "' ";
    if (!db->sql_only(sql,false))
        return seterrormsg(db->errordb());

    db->sql_bind_reset ();
    string masterStatus = "Masterfile creation request submitted";
    sql  = "insert into tg_pos_master_offline (branch_code, transaction_date, ";
    sql += "filename, date_created, date_finished, ready, status) values (";
    sql += "'"; sql += _locationCode; sql += "', ";
    sql += "'"; sql += _logicalDate; sql += "', ";
    sql += "'"; sql += filePath; sql += "', '', '', 0, '";
    sql += masterStatus; sql += "') ";
    if (!db->sql_only (sql,false))
        return seterrormsg(db->errordb());
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
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
    sql = "select * from TABLES where TABLE_SCHEMA = '";
    sql += dbname;
    sql += "' and TABLE_NAME IN ";
    sql += tableFilter(); //sql += " limit 5 ";
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
      publishLog("Exporting the table: ", table_name.c_str ());
      sqlTable.setTableName (table_name);
      ////////////////////////////////////
      //  Get column information
      if (!sqlTable.columnInfo())
          return false;
      ////////////////////////////////////
      //  Create the SQL batch file
      string columnList = "";
      if (!appendBatch(resetBatchFile,&sqlTable,columnList))
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
    publishLog("PUBLISH", "Done with table export!!!");
    //////////////////////////////////////
    //  Create the masterfile database
    //  SERVER SIDE    
    ttStart = time(NULL);
    if (!_cmdLine->compressMaster(_locationCode,_logicalDate))  {
        return false;
    }
    ttEnd = time(NULL);
    diff = ttEnd - ttStart;
    sprintf (sztmp, "Compression of the MYSQL database took %ld seconds\n", diff);
    publishLog("PUBLISH", sztmp);

    allEnd = time(NULL);
    diff = allEnd - allStart;
    sprintf (sztmp, "Export of the MYSQL database took %ld seconds\n", diff);
    publishLog("PUBLISH", sztmp);

    //////////////////////////////////////
    //  Create the masterfile database
    //  POS SIDE
    /*
    ttStart = time(NULL);
    if (!_cmdLine->importSchema(_locationCode,_logicalDate))  {
        return false;
    }
    ttEnd = time(NULL);
    diff = ttEnd - ttStart;
    sprintf (sztmp, "Creation of the MYSQL database took %ld seconds\n", diff);
    publishLog("PUBLISH", sztmp);*/
    //////////////////////////////////////
    //  Execution time
    //_cmdLine->cleanupFiles();
    allEnd = time (NULL);
    sprintf (sztmp, "The database backup took %ld seconds to process\n", (allEnd - allStart));
    publishLog("PUBLISH DONE!!!", sztmp);

    return true;
}
