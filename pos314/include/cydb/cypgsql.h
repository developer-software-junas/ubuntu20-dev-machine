/*******************************************************************
   FILE:                cypgsql.h
   DESCRIPTION:         Postgres class header
 *******************************************************************/
#ifndef _CYPGSQLINC
#define _CYPGSQLINC
#include "libpq-fe.h"
#include "cydbsql.h"
/***************************************************************************
         CLASS:         PGSqlEnv
   DESCRIPTION:         Database environment
 ***************************************************************************/
class                   PGSqlEnv : public CYDbEnv
{
public:
    //////////////////////////////////////
	//  Entry/exit point
	PGSqlEnv ();
	virtual ~PGSqlEnv ();
	virtual bool        connect (std::string server, std::string login,
                                 std::string passwd, std::string database, int port);
    //////////////////////////////////////
    //  Method(s)
	virtual void*       handle ();
protected:
    PGconn*             m_realenv;
};
/***************************************************************************
         CLASS:         PGSqlDb
   DESCRIPTION:         PGSql database
 ***************************************************************************/
class                   PGSqlDb : public CYDbSql
{
public:
    //////////////////////////////////////
	//  Entry/exit point
	PGSqlDb  (CYDbEnv* dbenv);
	virtual ~PGSqlDb ();
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
    virtual std::string  sql_bind_external (std::string value);
    virtual std::string  sql_bind (int idx,
                                   std::string value,
                                   bool blike = false);

	virtual std::string sql_field_name   (int nField);
    virtual std::string sql_field_type   (std::string szField) {
        (void)szField;
        return std::string("string");
    }
	virtual std::string sql_field_value  (std::string szField,
	                                      std::string szDefault=0);
protected:
    //////////////////////////////////////
    //  SQL Attribute(s)
    PGconn*             m_realenv;
    PGresult*           m_stmt;
    int                 sql_error (bool reset=false);
};
/***************************************************************************
         CLASS:         PGSqlArray
   DESCRIPTION:         DB sql collection
 ***************************************************************************/
class                   PGSqlArray : public CYDbArray
{
public:
    PGSqlArray ();
    virtual ~PGSqlArray ();
    virtual CYDbSql*    conn (std::string login,  std::string passwd,
                              std::string dbname, std::string server, int port=5432);
};
#endif
