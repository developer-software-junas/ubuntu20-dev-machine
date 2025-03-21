/********************************************************************
          FILE:         cypublish.cpp
   DESCRIPTION:         Publish implementation
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cypublish.h"

#include <string>
#include <sstream>
#include "cyw_aes.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
CYPublish::CYPublish (cyini* ini, cylog* log, cycgi* cgi,
                      CYDbEnv* env, CYDbEnv* schema)
{
    _ini = ini;
    _log = log;
    _cgi = cgi;
    _error = "";

    _env  = env;
    _schema = schema;

    _util = nullptr;
    _util = new cyutility ();

    _cmdLine = nullptr;
    _cmdLine = new CYCommandLine (_ini, _log, _util);
 }
/********************************************************************
      FUNCTION:         Destructor
   DESCRIPTION:         Class cleanup
 ********************************************************************/
CYPublish::~CYPublish ()
{
  if (nullptr != _util) delete _util;
  if (nullptr != _cmdLine) delete _cmdLine;

  size_t j = _dbList.size();
  for (size_t i = 0; i < j; i++)  {
      CYDbSql* db = _dbList.at(i);
      delete (db);
  }
  _dbList.clear();
}
/*******************************************************************
      FUNCTION:          dbconn
   DESCRIPTION:          Database object
     ARGUMENTS:          none
       RETURNS:          db object
 *******************************************************************/
CYDbSql*                 CYPublish::dbconn ()
{
    CYDbSql* db = nullptr;
#ifdef _CYSTORESERVER
    db = new MySqlDb (_env);
#else
    db = new SqliteDb (_env);
#endif
    _dbList.push_back(db);
    return db;
}
/*******************************************************************
      FUNCTION:          schemaconn
   DESCRIPTION:          Database object
     ARGUMENTS:          none
       RETURNS:          db object
 *******************************************************************/
CYDbSql*                 CYPublish::schemaconn ()
{
    CYDbSql* db = nullptr;
#ifdef _CYSTORESERVER
    db = new MySqlDb (_schema);
#else
    db = new SqliteDb (_schema);
#endif
    _dbList.push_back(db);
    return db;
}
/*******************************************************************
      FUNCTION:          cgiform
   DESCRIPTION:          Get the CGI argument
     ARGUMENTS:          name - the query parameter name
       RETURNS:          form value or a blank string
 *******************************************************************/
string                   CYPublish::cgiform (std::string name)
{
    (void) name;
    stringstream ss;
    ss.clear ();
    ss.str(string(""));
    ss << _util->string_cleanse(_cgi->form(name).c_str (),false);
    return ss.str ();
}
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
string                   CYPublish::errormsg ()
{
    return _error;
}
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
void                     CYPublish::publishLog (const char* msg1, const char* msg2="")
{
    _log->logmsg(msg1,msg2);
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* db;
    db = _ini->dbconnini();
    if (nullptr == db)  {
        return;
    }
    db->sql_reset ();
    string masterStatus = msg1; masterStatus += " "; masterStatus += msg2;

    sql  = "update tg_pos_master_offline set status = '";
    sql += masterStatus; sql += "' ";
    sql += "where branch_code = '"; sql += _locationCode; sql += "' ";
    sql += "and transaction_date = '"; sql += _logicalDate; sql += "' ";
    _log->logmsg(sql.c_str (),"");
    if (!db->sql_only (sql,false))
        return;
    if (!db->sql_commit())
        return;
}
/*******************************************************************
      FUNCTION:          digikey
   DESCRIPTION:          Create a digital key
     ARGUMENTS:          queryargs - the input buffer
       RETURNS:          the digitalcgimain key
 *******************************************************************/
string                   CYPublish::digikey (std::string queryargs)
{
    stringstream key;
    key << CalcHmacSHA256 (queryargs,CYHMACKEY);
    return key.str ();
}
/*******************************************************************
      FUNCTION:          validatekey
   DESCRIPTION:          Separate the key from the query arguments
                         and see if the resulting key matches
     ARGUMENTS:          queryargs - the input buffer
       RETURNS:          true if the key(s) match
 *******************************************************************/
bool                     CYPublish::validatekey(string hmac, string request,
                                                string session, string timestamp)
{
    ///////////////////////////////////////
    //  Generate the session id / hmac
    if (session.length() < 1)  {
        session = hmac;
    }
    ///////////////////////////////////////
    //  Generate the session id / hmac
    string qs  = "&cyw_request=";
           qs += request;
           qs += "&cyw_timestamp=";
           qs += timestamp;
    string digikey = CalcHmacSHA256 (qs,CYHMACKEY);
    digikey.resize(32);
    //////////////////////////////////////
    //  Compare the generated key
    bool bmatch = true;
    session.resize (32);
    for (int i = 0; i < 32; i++)  {
        if (session [i] == ' ')  {
            session [i] = '+';
        }
        if (session [i] != digikey [i])  {
            bmatch = false;
        }
    }
    //////////////////////////////////////
    //  Error message???
    if (false == bmatch)
        _error = "Invalid request key passed.  Please call techinical support";
    else
        _request = request;
    return bmatch;
}
/*******************************************************************
      FUNCTION:         digikey
   DESCRIPTION:         Create a digital key
     ARGUMENTS:         queryargs - the input buffer
       RETURNS:         the digital key
 *******************************************************************/
void                    CYPublish::appendkey (std::string& queryargs)
{
    queryargs += "&hmac=";
    queryargs += digikey (queryargs);
}
/*******************************************************************
      FUNCTION:         parsekey
   DESCRIPTION:         Separate the key from the query arguments
                        and see if the resulting key matches
     ARGUMENTS:         queryargs - the input buffer
       RETURNS:         the digital key
 *******************************************************************/
bool                    CYPublish::parsekey(std::string  queryargs,
                                            std::string& digikey,
                                            std::string& cleanargs)
{
    bool match = true;
    digikey = cleanargs = "";
    std::string keyval, hmac = "hmac";
    std::istringstream iss (queryargs);

    while(std::getline(iss, keyval, '&'))  {
        char* p = new char [keyval.length () + 1];
        memset (p, 0x00, keyval.length()+1);
        strncpy (p, keyval.c_str (), keyval.length());

        char* key = strtok (p,"=");
        char* val = strtok (nullptr, "&");

        if (key == hmac)  {
            if (nullptr == val)
                digikey = "";
            else
                digikey = val;
        }  else  {
            if (match)  {
                match = false;
            }  else
                cleanargs += "&";
            cleanargs += key;
            cleanargs += "=";
            if (nullptr != val)
                cleanargs += val;
        }
        delete [] p;
    }

    string mykey = CalcHmacSHA256(cleanargs,CYHMACKEY);
    if (mykey.length() > 65)
        mykey[65]='\0';
    //////////////////////////////////////
    //  Compare the generated key
    match = true;
    size_t i = 0, j = mykey.size ();
    if (j > 63)  j = 63;
    else if (j > 32) j = 32;
    else if (j > 16) j = 16;
    for (i = 0; i < j; i++)  {
        if (mykey [i] != digikey [i])  {
            match = false;
        }
    }
    return match;
}
/***************************************************************************
      FUNCTION:          seterrormsg
   DESCRIPTION:          Set the error message
 ***************************************************************************/
bool                     CYPublish::seterrormsg (string message)
{
    _error = message;
    publishLog("ERROR",_error.c_str ());
    return false;
}
