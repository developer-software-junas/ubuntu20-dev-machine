/********************************************************************
          FILE:          main.cpp
   DESCRIPTION:          CGI entry point
 ********************************************************************/
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <QCoreApplication>
#include "cyw_aes.h"
#include "cyw_util.h"
#include "erpsales/erpsales.h"

using std::string;
using std::vector;
using std::stringstream;
//////////////////////////////////////////
//  Global variable(s)
std::string             g_cginame = "/cgi-bin/sales.to.erp.cgi";
//////////////////////////////////////////
//  INI file intialization
bool                    debugmode (int argc, char** argv, cycgi* cgi);
/*******************************************************************
      FUNCTION:          cgimain
   DESCRIPTION:          Program entry point
 *******************************************************************/
int                      main (int argc, char **argv)
{    
    //////////////////////////////////////
    //  QT application
    QCoreApplication app(argc, argv);
    //////////////////////////////////////
    //  Create the log object
    cycgi cgi (argc,argv);
    cylog log (cgi.getlogfile(g_client));
    //////////////////////////////////////
    //  Create the settings object
    cyini ini;
    ini.load(cgi.getinifile(g_client));
    //////////////////////////////////////
    //  Create the CGI object
    debugmode(argc,argv,&cgi);
    //////////////////////////////////////
    //  Output is JSON
    //char jsonhdr [] = "application/json";
    //cgiHeaderContentType(jsonhdr);
    //////////////////////////////////////
    //  Create the main REST object
    cyerpsales* cyrest;
    cyrest = new cyerpsales (&ini, &log, &cgi);
    //////////////////////////////////////
    //  Process the sales table(s)
    //printf ("Check for sales...\n");
    if (!cyrest->batch())
        printf ("An error occurred: %s", cyrest->errormsg().c_str ());
    //////////////////////////////////////
    //  Cleanup
    delete (cyrest);
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
