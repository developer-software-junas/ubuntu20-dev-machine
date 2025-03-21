/********************************************************************
          FILE:         cypublish.h
   DESCRIPTION:         Publish class header
 ********************************************************************/
#ifndef __cypublish_H
#define __cypublish_H
//////////////////////////////////////////
//  CYWare class(es)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cydb/cyw_db.h"
//////////////////////////////////////////
//  Database object(s)
#include "cydb/cydbsql.h"
//////////////////////////////////////////
//  Command line class
#include "cycmd.h"
//////////////////////////////////////////
//  Classes used
class                   CYTable;
class                   CGIHandler;
/*****************************************
  MYSQL optimization:
    local_infile = 1
    innodb_buffer_pool_size = 12G
    innodb_log_buffer_size = 256M
    innodb_log_file_size = 1G
    innodb_flush_log_at_trx_commit = 0
 *****************************************/
/********************************************************************
      FUNCTION:         CYPublish
   DESCRIPTION:         Publish class
 ********************************************************************/
class                   CYPublish
{
public:
    //////////////////////////////////////
    //  Constructor - LOCAL POS or SERVER
    CYPublish (cyini* ini, cylog* log, cycgi* cgi,
               CYDbEnv* env, CYDbEnv* schema);
    virtual ~CYPublish ();
    //////////////////////////////////////
    //  Method(s)
    CYDbSql*            dbconn ();
    CYDbSql*            schemaconn ();
    std::string         cgiform(std::string name);

    bool                publishSqlite ();
    bool                publishMasterFile ();
    void                publishLog (const char* msg1, const char* msg2);
    //////////////////////////////////////
    //  Check unprocessed sales record(s)
    bool                checkSales  ();
    bool                processSales();
    bool                importSales (std::string salesFile);
    //////////////////////////////////////
    //  REST key validation
    bool                validatekey (std::string hmac,
                                     std::string request,
                                     std::string session,
                                     std::string timestamp);
    //////////////////////////////////////
    //  Database connection list
    std::vector<CYDbSql*> _dbList;
    //////////////////////////////////////
    //  Object access
    cyini               *_ini;
    cylog               *_log;
    cycgi               *_cgi;
    cyutility           *_util = nullptr;  // Useful function(s)

    CYDbEnv             *_env;
    CYDbEnv             *_schema;
    CYCommandLine       *_cmdLine;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    CGIHandler*         _cgiHandler;
    std::string         _error = "",
                        _request = "",
                        _logicalDate = "",
                        _locationCode = "";
    //////////////////////////////////////
    //  Internal key validation
    std::string         digikey  (std::string  queryargs);
    bool                parsekey (std::string  queryargs,
                                  std::string& digikey,
                                  std::string& cleanargs);
    void                appendkey(std::string& queryargs);
    //////////////////////////////////////
    //  Internal method(s)
    bool                appendBatch (bool reset, CYTable* table,
                                     std::string& columnList);
    bool                appendLiteImport (CYTable* table,
                                          std::string& columnList);
    bool                appendLiteSchema (bool reset, CYTable* table,
                                          std::string& columnList);
    bool                getLogicalDate (std::string& sysdate,
                                        std::string& locationCode);
    std::string         tableFilter ();
public:
    //////////////////////////////////////
    //  Error message access
    std::string         errormsg ();
    bool                seterrormsg (std::string message);
};
#endif
