/********************************************************************
          FILE:         store_update.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  Header file(s)
#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:          store_update
   DESCRIPTION:          SQL update
 ***************************************************************************/
std::string              cyposrest::sql_update()
{
    //////////////////////////////////////
    //  Retrieve argument(s)
    string payload = cgiform("payload");
    string sql = _util->base64decode(payload);
    _log->logmsg("RESTSQL",sql.c_str());
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Update
    if (!db->sql_only(sql,false))
        return _util->jsonerror(db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());
    //////////////////////////////////////
    //  Post query request???
    Json::Value root;
    payload = cgiform("postrequest");
    if (payload.length() > 0)  {
        sql = _util->base64decode(payload);
        //////////////////////////////////
        //  Retrieve record(s)
        if (!db->sql_result(sql,false))
            return _util->jsonerror(db->errordb());
        //////////////////////////////////
        //  Assemble the JSON response
        int numrows = 0, numcols = 0;
        Json::Value result, jline;
        if (!db->eof())  {
            numcols = db->numcols();
            do  {
                jline.clear();
                for (int i = 0; i < numcols; i++)  {
                    string name = db->sql_field_name(i);
                    string val = db->sql_field_value(name);
                    jline[name]=val;
                }
                result[numrows] = jline;
                numrows++;
                db->sql_next();
            }  while (!db->eof());
        }  else  {
            result = Json::nullValue;
        }
        //////////////////////////////////////
        //  Spit it out
        root["status"]="ok";
        root["numrows"]=numrows;
        root["numcols"]=numcols;
        root["resultset"]=result;
    }  else  {
        root ["status"]="ok";
        root["numrows"]=0;
        root["numcols"]=0;
        root["resultset"]=Json::nullValue;
    }

    stringstream ss;
    ss << root;
    return ss.str();
}

