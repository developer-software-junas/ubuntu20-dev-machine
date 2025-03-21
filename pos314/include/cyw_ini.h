/*******************************************************************
          FILE:          cyw_ini.h
   DESCRIPTION:          Persistent settings
 *******************************************************************/
#ifndef __CYWARE_INI_H
#define __CYWARE_INI_H
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
///////////////////////////////////////////
//  Classes used
class cylog;
class cycgi;
class CYDbEnv;
class CYDbSql;
class SqliteEnv;
class CYRestCommon;
//////////////////////////////////////////
//  INI vector structure
typedef struct  {
    std::string key1;
    std::string key2;
    std::string value;
} struct_inivector;
//////////////////////////////////////////
//  ONE time definition, for client(s)
const std::string        g_client = "joel";
/***************************************************************************
         CLASS:          cyini
   DESCRIPTION:          Settings class
 ***************************************************************************/
class                    cyini
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cyini ();
    virtual ~cyini ();
    //////////////////////////////////////
    //  Database method(s)
    bool                save (std::string filename);
    bool                load (std::string filename);

    CYDbSql*            dbconn ();
    CYDbSql*            dbconnschema ();
    CYDbSql*            dbconnremote ();
    CYDbSql*            dbconnini (std::string inikey="");

    void                connswitch (bool reverse);
    bool                connectEnv (std::string iniSection);
    //////////////////////////////////////
    //  Access method(s)
    std::string         filename () {return _file;}
    std::string         errormsg () {return _error;}
    std::string         get_value (const char* key1,
                                   const char* key2);
    bool                set_value (const char* key1,
                                   const char* key2,
                                   const char* value);
    //////////////////////////////////////
    //  Receipt type
    CYRestCommon*       receipt_type (cylog* log, cycgi* cgi);
    //////////////////////////////////////
    //  Database environment
    CYDbEnv*            _env;
    CYDbEnv*            _envSchema;
    CYDbEnv*            _envRemote;
    CYDbEnv*            _envFreestyle;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    bool                _switch;
    std::string         _file, _error;
    struct_inivector    _settings[255];
    //////////////////////////////////////
    //  Database connection
    std::vector<CYDbSql*> _dbList;
    std::vector<CYDbSql*> _dbListRemote;
    //////////////////////////////////////
    //  Internal method(s)
    bool                create_table (SqliteEnv* env);
};
#endif
