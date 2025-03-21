/*******************************************************************
   FILE:                cymysql.h
   DESCRIPTION:         Postgres class header
 *******************************************************************/
#ifndef _CYMYSQL_H
#define _CYMYSQL_H
#include <mysql.h>
#include "cydbsql.h"
/***************************************************************************
         CLASS:         MySqlEnv
   DESCRIPTION:         Database environment
 ***************************************************************************/
class                   MySqlEnv : public CYDbEnv
{
public:
    //////////////////////////////////////
	//  Entry/exit point
	MySqlEnv ();
	virtual ~MySqlEnv ();
	virtual bool        connect (std::string server, std::string login,
                                 std::string passwd, std::string database, int port);
    //////////////////////////////////////
    //  Method(s)
	virtual void*       handle ();

protected:
    MYSQL*              m_realenv;
};
/***************************************************************************
         CLASS:         SqliteDb
   DESCRIPTION:         MYSql database
 ***************************************************************************/
class                   MySqlDb : public CYDbSql
{
public:
    //////////////////////////////////////
	//  Entry/exit point
    MySqlDb  (CYDbEnv* dbenv);
    virtual ~MySqlDb ();
	//////////////////////////////////////
	//  SQL method(s)
	virtual bool        sql_next   ();
	virtual bool        sql_only   (std::string  sql, bool bind);
	virtual bool        sql_result (std::string  sql, bool bind);
	virtual bool        sql_count  (std::string  sql, bool bind, long& lCount);
    //////////////////////////////////////
	//  Transaction method(s)
    virtual void        sql_reset    ();
	virtual bool        sql_commit   ();
	virtual bool        sql_rollback ();
    //////////////////////////////////////
	//  Field method(s)
    virtual std::string sql_bind (int idx,
                                  std::string value,
                                  bool blike = false);
	virtual std::string sql_field_name   (int nField);
    virtual std::string sql_field_type   (std::string szField);
	virtual std::string sql_field_value  (std::string szField,
                                          std::string szDefault="");
protected:
    //////////////////////////////////////
    //  SQL Attribute(s)
    MYSQL*              m_realenv;
    MYSQL_RES*          m_res;
    MYSQL_STMT*         m_stmt;
    MYSQL_FIELD*        m_field;
    MYSQL_BIND          m_params[CYBINDVARMAX];
    MYSQL_BIND          m_result[CYBINDVARMAX];

    std::string         _bindchar;
    struct_cymem        _cymem_params[CYBINDVARMAX];
    struct_cymem        _cymem_result[CYBINDVARMAX];


    int                 m_param_cnt;
    bool                m_result_err[CYBINDVARMAX],
                        m_result_nil[CYBINDVARMAX];
    bool                m_param_err [CYBINDVARMAX];
    unsigned long       m_result_len[CYBINDVARMAX];
    ///////////////////////////////////////
    //  Error check
    int                 sql_error ();
};
/***************************************************************************
         CLASS:         MYSqlArray
   DESCRIPTION:         DB sql collection
 ***************************************************************************/
class                   MYSqlArray : public CYDbArray
{
public:
    MYSqlArray ();
    virtual ~MYSqlArray ();
    virtual CYDbSql*    conn (std::string login,  std::string passwd,
                              std::string dbname, std::string server, int port=3306);
};
/***************************************************************************
         CLASS:          MYSqlTable
   DESCRIPTION:          Database table class
 ***************************************************************************/
class                    MYSqlTable : public CYDbTable
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    MYSqlTable (CYDbEnv* env,
                CYDbEnv* schema,
                std::string schemaName,
                std::string tableName);
    virtual ~MYSqlTable ();
    //////////////////////////////////////
    //  Method(s)
    virtual std::string  createTableSql ();
    virtual std::string  createTableIndex ();

    virtual bool         columnInfo ();
    virtual bool         createTableFile (std::string fileName,
                                          std::string tableName,
                                          std::string columnList);
};
#endif
