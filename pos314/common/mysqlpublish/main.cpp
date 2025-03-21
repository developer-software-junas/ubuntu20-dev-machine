/********************************************************************
          FILE:          main.cpp
   DESCRIPTION:          CGI entry point
 ********************************************************************/
#include <vector>
#include <sstream>
#include <sys/types.h>

#include "cyw_aes.h"
#include "cyw_log.h"
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cydb/cydbsql.h"
#include "cydb/cymysql.h"

#include "cypublish.h"
using std::string;
using std::vector;
using std::stringstream;
//////////////////////////////////////////
//  Global variable(s)
const std::string        g_cginame = "/cgi-bin/mysqlpublish.cgi";
const std::string        g_logfile = "/usr/local/log/mysqlpublish.log";
//////////////////////////////////////////
//  INI file intialization
bool                     debugmode (int argc, char** argv, cycgi* cgi);
/*******************************************************************
      FUNCTION:          cgimain
   DESCRIPTION:          Program entry point
 *******************************************************************/
int                      main (int argc, char **argv)
{
    //////////////////////////////////////
    //  Create the log object
    cycgi cgi (argc,argv);
    cylog log (g_logfile);
    //////////////////////////////////////
    //  Create the settings object
    cyini ini;
    ini.load(cgi.getinifile());
    //////////////////////////////////////
    //  Create the CGI object
    debugmode(argc,argv,&cgi);
    //////////////////////////////////////
    //  Output is JSON
    char jsonhdr [] = "application/json";
    cgiHeaderContentType(jsonhdr);
    //////////////////////////////////////
    //  Database settings
    string host = ini.get_value("default","HOST");
    string user = ini.get_value("default","USER");
    string name = ini.get_value("default","NAME");
    string pass = decode (ini.get_value("default","PASS").c_str ());
    int port = atoi(ini.get_value("default","PORT").c_str ());
    //////////////////////////////////////
    //  Database connection
    bool ret;
    char szerror[250];
    CYDbEnv *env, *schema, *remote;
    env = new MySqlEnv;
    schema = new MySqlEnv;
    remote = new MySqlEnv;
    //////////////////////////////////////
    //  Check for database error(s)
    ret = env->connect (host, user, pass, name, port);
    if (!ret)  {
        Json::Value err;
        err["status"]="error";
        string tmp = env->errordbenv();
        err["error_message"]=tmp;

        std::stringstream ss;
        ss << err;
        sprintf (szerror,"%s", ss.str().c_str ());
        log.logmsg("PUBLISH",szerror);
        delete (remote);
        delete (schema);
        delete(env);
        return 0;
    }
    ret = schema->connect (host, user, pass, "INFORMATION_SCHEMA", port);
    if (!ret)  {
        Json::Value err;
        err["status"]="error";
        string tmp = schema->errordbenv();
        err["error_message"]=tmp;

        std::stringstream ss;
        ss << err;
        sprintf (szerror,"%s", ss.str().c_str ());
        log.logmsg("PUBLISH",szerror);
        delete (remote);
        delete (schema);
        delete(env);
        return 0;
    }
    //////////////////////////////////////
    //  Remote database
    host = ini.get_value("master","HOST");
    user = ini.get_value("master","USER");
    name = ini.get_value("master","NAME");
    pass = decode (ini.get_value("master","PASS").c_str ());
    port = atoi(ini.get_value("master","PORT").c_str ());
    ret = remote->connect (host, user, pass, name, port);
    if (!ret)  {
        Json::Value err;
        err["status"]="error";
        string tmp = remote->errordbenv();
        err["error_message"]=tmp;

        std::stringstream ss;
        ss << err;
        sprintf (szerror,"%s", ss.str().c_str ());
        log.logmsg("PUBLISH",szerror);
        delete(env);
        delete(schema);
        delete(remote);
        return 0;
    }
    //////////////////////////////////////
    //  Publish...
    CYPublish publish (&ini, &log, &cgi, env, schema);
    /*
    if (!publish.publishSqlite())  {
      sprintf (szerror,"%s\n", publish.errormsg().c_str ());
      log.logmsg("PUBLISH",szerror);
    }
    */
    if (!publish.publishMasterFile())  {
      sprintf (szerror,"%s\n", publish.errormsg().c_str ());
      log.logmsg("PUBLISH",szerror);
    }

    //////////////////////////////////////
    //  Cleanup
    delete(env);
    delete(schema);
    delete(remote);
    return 0;
}
/*******************************************************************
      FUNCTION:          debugmode
   DESCRIPTION:          Check if debug mode was requested
                         from the command line argument(s)
 *******************************************************************/
bool                     debugmode (int argc, char** argv, cycgi* cgi)
{
    cyutility util;
    std::string webargs = "noargs=true";


    if (argc >= 3)  {
        ///////////////////////////////////
        //  Query arguments
        string flag = "-debug";
        if (nullptr != argv [1] && flag == argv[1])  {
            if (nullptr == argv [2])
                return false;
            ///////////////////////////////
            //  DEBUG mode active
            if (nullptr != argv [3])
                webargs = argv[3];
            ///////////////////////////////
            //  Parse the query string
            int mode = 0;
            string key, val, tmp;
            vector<std::string> test = util.string_tokenize (webargs,"&");
            size_t x, y, i, j = test.size ();
            for (i = 0; i < j; i++)  {
                key = "";
                val = "";
                tmp = test.at (i);
                y = tmp.size ();
                mode = 0;
                for (x = 0; x < y; x++)  {
                    if (tmp.at (x) == '=' && 0 == mode)  {
                        mode = 1;
                        ++x;
                    }
                    if (x < y)  {
                        if (0 == mode)
                            key += tmp.at(x);
                        else
                            val += tmp.at(x);
                    }
                }
                cgi->add_form (key, val);
            }
        }
    }
    return true;
}
