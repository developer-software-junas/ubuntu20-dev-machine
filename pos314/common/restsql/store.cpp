/********************************************************************
          FILE:         rest.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "store.h"
#include <string>
#include <sstream>
#include "cyw_aes.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
cyreststore::cyreststore (cyini* ini, cylog* log, cycgi* cgi)
{
    _ini = ini;
    _log = log;
    _cgi = cgi;
    _error = "";

    _util = nullptr;
    _util = new cyutility ();

 }
/********************************************************************
      FUNCTION:         Destructor
   DESCRIPTION:         Class cleanup
 ********************************************************************/
cyreststore::~cyreststore ()
{
  if (nullptr != _util) delete _util;
}
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
string                   cyreststore::errormsg ()
{
    return _error;
}
/*******************************************************************
      FUNCTION:          errorjson
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error in JSON
 *******************************************************************/
string                   cyreststore::errorjson (string errormsg)
{
    return _util->jsonerror(errormsg);
}
/*******************************************************************
      FUNCTION:          digikey
   DESCRIPTION:          Create a digital key
     ARGUMENTS:          queryargs - the input buffer
       RETURNS:          the digitalcgimain key
 *******************************************************************/
string                   cyreststore::digikey (std::string queryargs)
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
bool                     cyreststore::validatekey(string hmac, string request,
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
void                    cyreststore::appendkey (std::string& queryargs)
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
bool                    cyreststore::parsekey(std::string  queryargs,
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
bool                     cyreststore::seterrormsg (string message)
{
    _error = message;
    return false;
}
/***************************************************************************
      FUNCTION:          json_select_single
   DESCRIPTION:          JSON formatted single row query
 ***************************************************************************/
enumQueryType            cyreststore::json_select_single (CYDbSql* db,
                                                        Json::Value& jval,
                                                        string sql)
{
    if (!db->sql_result (sql,true))
        return seterrorquery(db->errordb());
    if (db->eof ())  {
        seterrorquery("No record(s) found");
        return enumQueryType::EQT_NOROWS;
    }

    jval.clear ();
    string name, val;
    for (int i = 0; i < db->numcols(); i++)  {
        name = db->sql_field_name(i);
        val = db->sql_field_value(name);
        jval[name]=val;
    }
    return enumQueryType::EQT_SUCCESS;
}
/***************************************************************************
      FUNCTION:          json_select_multiple
   DESCRIPTION:          JSON formatted multi row query
 ***************************************************************************/
enumQueryType            cyreststore::json_select_multiple (CYDbSql* db,
                                                          Json::Value& jval,
                                                          string sql)
{
    //db->sql_reset ();
    if (!db->sql_result (sql,db->m_bind.m_count > 0))
        return seterrorquery(db->errordb());
    if (db->eof ())
        return enumQueryType::EQT_NOROWS;

    int idx = 0;
    jval.clear ();
    string name, val;
    Json::Value jline;
    jline.clear ();

    do  {
        for (int i = 0; i < db->numcols(); i++)  {
            name = db->sql_field_name(i);
            val = db->sql_field_value(name);
            jline[name]=val;
        }
        jval[idx] = jline;
        idx++;
        db->sql_next ();
    } while (!db->eof ());
    return enumQueryType::EQT_SUCCESS;
}
