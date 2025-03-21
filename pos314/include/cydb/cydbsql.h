/*******************************************************************
   FILE:                 cyw_db.h
   DESCRIPTION:          Abstract database header
 *******************************************************************/
#ifndef __CYDBSQLKO_H
#define __CYDBSQLKO_H
///////////////////////////////////////////
//  Standard headers
#include <string.h>
#include <string>
#include <sstream>
#include "cyw_colbind.h"
//////////////////////////////////////////
//  Export format from SQL
typedef enum  {
    js_json, js_csv, js_query
}  JSCONST_DBRESULT;
/***************************************************************************
         CLASS:          CYDbEnv
   DESCRIPTION:          Database environment
 ***************************************************************************/
class                    CYDbEnv
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    CYDbEnv ();
    virtual ~CYDbEnv ();
    //////////////////////////////////////
    //  Acess methods
    virtual bool         ok ();
    virtual std::string  errordbenv ();
    //////////////////////////////////////
    //  Database handle(s)
    virtual void*        handle  ()=0;
    virtual bool         connect (std::string server, std::string login,
                                  std::string passwd, std::string database, int port)=0;
    //////////////////////////////////////
    //  Database params
    int _port;
    std::string _server, _login, _database, _passwd;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    bool                 m_ok;
    void*                m_handle;
    std::string          m_error;
};
/***************************************************************************
         CLASS:          CYDbSql
   DESCRIPTION:          BDB sql database
 ***************************************************************************/
class                    CYDbSql
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    CYDbSql  (CYDbEnv* dbenv);
    virtual ~CYDbSql ();
    //////////////////////////////////////
    //  SQL method(s)
    virtual bool         sql_next   ()=0;
    virtual bool         sql_only   (std::string  sql,
                                     bool bind=false)=0;
    virtual bool         sql_result (std::string  sql,
                                     bool bind=false)=0;
    //////////////////////////////////////
    //  Transaction method(s)
    virtual void         sql_reset    ()=0;
    virtual bool         sql_commit   ()=0;
    virtual bool         sql_rollback ()=0;
    virtual bool         sql_query_row (int row, std::string& query);

    virtual bool         sql_field_csv  ();
    virtual bool         sql_field_query(int norow=0);
    virtual bool         sql_field_json (std::string root,
                                         bool addinfo, bool append);
    virtual bool         sql_field_rest (std::string schema);
    //////////////////////////////////////
    //  Field method(s)
    virtual void         sql_bind_reset ();
    virtual std::string  sql_bind_value ();
    virtual std::string  sql_field_result ();
    virtual std::string  sql_bind_external (std::string value);
    virtual std::string  sql_bind (int idx,
                                   std::string value,
                                   bool blike = false);
    virtual std::string  sql_check_hash (std::string  szField,
                                         std::string& szHash);

    virtual std::string  sql_result_set (JSCONST_DBRESULT type,
                                         std::string name,
                                         bool addinfo,
                                         bool append);

    virtual std::string  sql_field_name (int nField)=0;
    virtual std::string  sql_field_type (std::string szField)=0;
    virtual std::string  sql_field_value(std::string szField,
                                         std::string szDefault="")=0;
    //////////////////////////////////////
    //  Utility method(s)
    virtual bool         ok      ();
    virtual bool         eof     ();
    virtual std::string  result  ();
    virtual std::string  errordb ();
    virtual std::string  sql_dump ();

    virtual int          numcols ();
    virtual int          numbind ();
    virtual long         numrows ();
    virtual void         notrans ();
    virtual void         logsqlmsg (std::string sql,
                                    std::string bind,
                                    bool isUpdate = false);
    ///////////////////////////////////////
    //  Bind variable(s)
    bool                 _logsql = true;

    int                  _idxbind;
    CYColumnBind         m_bind;
    //std::string          m_bindchar;
    ///////////////////////////////////////
    //  Legacy
    //  Pure virtual functions
    /*
    virtual int         SQLOnly (char* szSql)  {
        return sql_only(szSql,false);
    }
    virtual int         SQLOnly (const char* szSql)  {
        return sql_only(szSql,false);
    }
    virtual long        SQLOnly (std::string szSql)  {
        return sql_only(szSql,false);
    }

    virtual int         SQLOnlineOnly (char* szSql)  {
        return sql_only(szSql,false);
    }
    virtual int         SQLOnlineOnly (const char* szSql)  {
        return sql_only(szSql,false);
    }
    virtual int         SQLOnlineOnly (std::string szSql)  {
        return sql_only(szSql,false);
    }

    virtual long        SQLResult (char* szSql)  {
        return sql_result(szSql,false);
    }
    virtual long        SQLResult (const char* szSql)  {
        return sql_result(szSql,false);
    }
    virtual long        SQLResult (std::string szSql)  {
        return sql_result(szSql,false);
    }
    //  Operations
    int                 IsEof () { return eof (); }
    int                 Commit () { return sql_commit(); }
    int                 Rollback () { return sql_rollback (); }

    const char*         GetErrorMessage () { return m_error.c_str (); }

    //  Informational
    int                 IsOk () { return m_ok; }
    int                 RowCount () {return m_numrows; }
    int                 numcols () {return m_numcols; }
    const char*         FieldName (int nIdx) {
        m_temp = sql_field_name (nIdx);
        return m_temp.c_str ();
    }
    const char*         Fields (const char* szField)  {
        m_temp = sql_field_value (szField);
        return m_temp.c_str ();
    }
    //  Navigation
    int                 MoveNext () { return sql_next (); }
    */
protected:
    ///////////////////////////////////////
    //  SQL Attribute(s)
    CYDbEnv*             m_env;
    ///////////////////////////////////////
    //  Flag(s)
    bool                 m_ok,
                         m_eof,
                         m_trans,
                         m_select;

    int                  m_row,
                         m_numcols;
    int                  m_numrows;
    ///////////////////////////////////////
    //  Error handling
    std::stringstream    m_sstmp;
    std::string          m_temp,
                         m_error,
                         m_database;
    std::string          _result,
                         _dumpargs,
                         _last_sql;
};
/***************************************************************************
         CLASS:          CYDbArray
   DESCRIPTION:          DB sql collection
 ***************************************************************************/
class                    CYDbArray
{
public:
    CYDbArray ();
    virtual ~CYDbArray ();

    virtual std::string  errmsg  ();
    virtual bool         isvalid (CYDbSql* db);
    virtual void         addconn (CYDbSql* db);
    virtual void         addenv  (CYDbEnv* env, std::string name);

    virtual CYDbEnv*     getenv  (std::string name);
    virtual CYDbSql*     conn    (std::string login,  std::string passwd,
                                  std::string dbname, std::string server, int port=0)=0;
protected:
    std::string          m_error;
    CYDbEnv*             m_dbenv  [CYBINDVARMAX];
    CYDbSql*             m_dbconn [CYBINDVARMAX];
    std::string          m_dbname [CYBINDVARMAX];
    ///////////////////////////////////////
    //  Internal method(s)
    virtual bool         uniquedb (std::string name);
};
///////////////////////////////////////////
//  Maximum column(s) to parse
const int g_maxindex = 16;
const int g_maxcolumns = 814;
const std::string g_universalfile = "/usr/local/data/universal.sql";
//////////////////////////////////////////
//  Column structure
struct tableColumn {
    bool numeric;
    bool nullable;
    int columnSeq;
    std::string columnKey;
    std::string columnLen;
    std::string columnName;
    std::string columnType;
};
struct tableIndex {
    std::string indexName;
    std::string indexColumn[g_maxcolumns];
};
/***************************************************************************
         CLASS:          CYDbTable
   DESCRIPTION:          Database table class
 ***************************************************************************/
class                    CYDbTable
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    CYDbTable (CYDbEnv* env,
               CYDbEnv* schema,
               std::string schemaName,
               std::string tableName);
    virtual ~CYDbTable ();
    //////////////////////////////////////
    //  Schema method(s)
    virtual bool         columnInfo ()=0;
    virtual std::string  createTableSql ()=0;
    virtual std::string  createTableIndex ()=0;
    //////////////////////////////////////
    //  Utility method(s)
    virtual bool         createTableFile (std::string fileName,
                                          std::string tableName,
                                          std::string columnList)=0;
    /////////////////////////////////////
    //  Generated SQL
    std::string          sqlCreate;
    std::string          indexCreate;
//protected:
    //////////////////////////////////////
    //  Attribute(s)
    CYDbEnv              *_env, *_schema;
    tableIndex           _index [g_maxindex];
    tableColumn          _column [g_maxcolumns];
    std::string          _tmp, _error, _dbname, _tablename;
    //////////////////////////////////////
    //  Internal method(s)
    void                 resetColumns ()  {
        for (int i = 0; i < g_maxcolumns; i++)  {
            _column[i].columnName = "";
            _column[i].columnType = "";
            _column[i].nullable = false;

        }
    }
    void                 resetIndex ()  {
        for (int x = 0; x < g_maxindex; x++)  {
            _index[x].indexName = "";
            for (int i = 0; i < g_maxcolumns; i++)  {
                _index[x].indexColumn[i] = "";
            }
        }
    }
public:
    //////////////////////////////////////
    //  Acess methods
    virtual std::string  errormsg ()  { return _error; }
    virtual std::string  tableName ()  { return _tablename; }
    virtual bool seterrormsg (std::string msg)  { _error = msg; return false; }
    virtual void setTableName (std::string val)  { _tablename = val; }

    virtual tableIndex* getIndex (int idx) { return &_index[idx]; }
    virtual tableColumn* getColumn (int idx) { return &_column[idx]; }
};
#endif
