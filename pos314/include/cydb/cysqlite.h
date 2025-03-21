/*******************************************************************
   FILE:                cysqlite.h
   DESCRIPTION:         SQLITE class header
 *******************************************************************/
#ifndef __CYSQLITEXX_H
#define __CYSQLITEXX_H
///////////////////////////////////////////
//  Header file(s)
//#include "db/osx/db.h"
//#include "db/osx/dbsql.h"
#include "db/sqlite3.h"
#include "cydb/cydbsql.h"
/***************************************************************************
         CLASS:         SqliteEnv
   DESCRIPTION:         Database environment
 ***************************************************************************/

class                   SqliteEnv : public CYDbEnv
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    SqliteEnv ();
    virtual ~SqliteEnv ();
    virtual bool        connect (std::string server, std::string login,
                                 std::string passwd, std::string database, int port=0);
    //////////////////////////////////////
    //  Method(s)
    virtual void*       handle ();
protected:
    sqlite3*            m_realenv;
};


/***************************************************************************
         CLASS:         SqliteDb
   DESCRIPTION:         Sqlite database
 ***************************************************************************/

class                   SqliteDb : public CYDbSql
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    SqliteDb  (CYDbEnv* dbenv);
    virtual ~SqliteDb ();
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
    virtual std::string sql_field_name   (int nField);
    virtual std::string sql_field_type   (std::string szField) {
        (void)szField;
        return std::string("string");
    }

    virtual std::string sql_field_value  (std::string szField,
                                          std::string szDefault="");
protected:
    //////////////////////////////////////
    //  SQL Attribute(s)
    sqlite3*            m_realenv;
    sqlite3_stmt*       m_stmt;
    int                 sql_error (bool reset=false);
};


/***************************************************************************
         CLASS:         SqliteArray
   DESCRIPTION:         DB sql collection
 ***************************************************************************/

class                   SqliteArray : public CYDbArray
{
public:
    SqliteArray ();
    virtual ~SqliteArray ();
    virtual CYDbSql*    conn (std::string login,  std::string passwd,
                              std::string dbname, std::string server, int port=0);
};
#endif
