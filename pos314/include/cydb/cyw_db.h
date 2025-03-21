/*******************************************************************
          FILE:          cyw_db.h
   DESCRIPTION:          Database class header
 *******************************************************************/
#ifndef __CYWARE_DBXX_H
#define __CYWARE_DBXX_H
#include <string>
#include "cydbsql.h"
#include "cydb/cysqlite.h"
#include "cyw_ini.h"
#ifdef _CYMYSQL
#include "cydb/cymysql.h"
#endif
#ifdef _CYPGSQL
#include "cydb/cypgsql.h"
#endif
//////////////////////////////////////////
//  Database connection structure
typedef struct  {
    int nport;
    std::string dbtype, dbname, dbhost, dbuser, dbpass;
} struct_cydb;
/********************************************************************
         CLASS:          cywaredb
   DESCRIPTION:          CYWare database class
 ********************************************************************/
class                    cywaredb
{
public:
    ///////////////////////////////////////
    //  Constructor / Destructor
    cywaredb ();
    cywaredb (cyini* ini);
    virtual  ~cywaredb ();
    ///////////////////////////////////////
    //  Access method(s)
    std::string          errormsg ();
    std::string          connstring ();
    bool                 validptr (CYDbSql* db);
    ///////////////////////////////////////
    //  Database connection
    CYDbSql*             dbconn (std::string conndesc);
    CYDbSql*             dbconn (std::string ini_section,
                                 std::string ini_database);
    CYDbSql*             dbconnpos (std::string conndesc);
    CYDbSql*             dbconnstruct (struct_cydb strdb);
    CYDbSql*             dbconnmanual (std::string dbtype,
                                       std::string dbhost,
                                       std::string dbuser,
                                       std::string dbpass,
                                       std::string dbname,
                                       int dbport);
    CYDbSql*             dbconnlite (std::string dbname);
    ///////////////////////////////////////
    //  JS array from database result(s)
    bool                 dbschema  (CYDbSql* db,
                                    std::string  name,
                                    std::string& result);
    void                 dbtoarray (CYDbSql* db,
                                    std::string& result);
    //////////////////////////////////////
    //  Database connection parameter(s)
    std::string          _dbtype, _dbuser, _dbpass,
                         _dbname, _dbhost, _dbport;
protected:
    ///////////////////////////////////////
    //  DB settings
    cyini*               _ini;
    ///////////////////////////////////////
    //  Attribute(s)
#ifdef _CYPGSQL
    PGSqlArray*          _arr;
#endif // _CYPGSQL
#ifdef _CYMYSQL
    MYSqlArray*          _myarr;
#endif // _CYMYSQL
    SqliteArray*         _arrlite;
    ///////////////////////////////////////
    //  Error message
    std::string          _error, _connstring;
};
#endif
