/********************************************************************
          FILE:          main.cpp
   DESCRIPTION:          CGI entry point
 ********************************************************************/
#include <vector>
#include <sstream>
#include <sys/types.h>

#include "store.h"
#include "cyw_aes.h"
#include "cyw_util.h"
#include "cydb/cydbsql.h"
//#include "cydb/cymysql.h"

using std::string;
using std::vector;
using std::stringstream;
//////////////////////////////////////////
//  Global variable(s)
const std::string        g_cginame = "/cgi-bin/cyreststore.cgi";
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
    cylog log (cgi.getlogfile());
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
    //int port = atoi(ini.get_value("default","PORT").c_str ());
    //////////////////////////////////////
    //  Create the main REST object
    cyreststore* cyrest;
    cyrest = new cyreststore (&ini, &log, &cgi);
    string cyw_request = cyrest->cgiform("cyw_request");
    //////////////////////////////////////
    //  Is this an application payload???
    log.logmsg("PAYLOAD:", "checking...");
    if (nullptr != cgiContentType &&  cgiContentLength > 0 &&
        0 == strncmp (cgiContentType, "brewery/image/", strlen ("brewery/image/")))  {
            log.logmsg("CONTENT:", cgiContentType);
            ///////////////////////////////////////
            //  Read from standard input
            struct_cymem mem;
            cymem_init (&mem, (cgiContentLength+8));
            if (fread (mem.memory, 1, cgiContentLength, cgiIn)>0)  {
                char filetmp [512];
                memset (filetmp, 0x00, 512);
                strncpy (filetmp, cgiContentType + strlen("brewery/image/"),500);
                //////////////////////////////////////
                //  Parse ther recipe item code
                string picturefile = "";
                char* p = (char*)malloc(strlen(filetmp)+1);
                memset (p, 0x00, strlen(filetmp)+1);
                strcpy (p, filetmp);
                int i, j = strlen(p);
                for (i = 0; i < j; i++)  {
                    if (p[i]=='.')  {
                        picturefile = p+(i+1);
                        p[i]='\0';
                        i = (j+1);
                    }
                }
                log.logmsg ("recipe code",p);
                log.logmsg ("recipe picture",picturefile.c_str ());
                //////////////////////////////////////
                //  Retrieve argument(s)
                string recipe_item_code = p;
                free(p);

                string filename = ini.get_value("PATH","FNBIMAGE");
                log.logmsg ("recipe image directory",filename.c_str ());
                filename += recipe_item_code;
                mkdir (filename.c_str (),0777);
                filename += "/";

                string cmd = "rm ";
                cmd += filename;
                cmd += "*.*";
                if (system (cmd.c_str ()))
                    cyrest->_log->logmsg ("cleanup picture",cmd.c_str ());

                string url = cyrest->_ini->get_value("PATH","FNBURL");
                url += recipe_item_code;
                url += "/";
                url += picturefile;
                cyrest->_log->logmsg ("recipe picture database",url.c_str ());



                filename += picturefile;
                log.logmsg("CONTENT:", filename.c_str ());
                FILE* f = fopen (filename.c_str (), "w");
                fwrite (mem.memory, 1, cgiContentLength, f);
                fclose(f);
                log.logmsg("CONTENT:", "call rest");
                log.logmsg("CONTENT:", filetmp);


                Json::Value root;
                root["status"]="ok";
                root["picture_url"]=url;
                //////////////////////////////////////
                //  Spit it out...
                stringstream ss;
                ss << root;
                string output;
                output = ss.str ();
                log.logmsg("CONTENT:", output.c_str ());
                fprintf (cgiOut,"%s",output.c_str ());
                return EXIT_SUCCESS;
            }
    }    
    //////////////////////////////////////
    //  Retrieve argument(s)
    string cyw_hmac = cyrest->cgiform("hmac");
    cyw_request = cyrest->cgiform("cyw_request");
    string cyw_session = cyrest->cgiform("cyw_session");
    string cyw_timestamp = cyrest->cgiform("cyw_timestamp");
    log.logmsg("RESTURL:", cgi.formdump("test").c_str ());    
    //////////////////////////////////////
    //  Validate the REST key
    if (!cyrest->validatekey(cyw_hmac,cyw_request,cyw_session,cyw_timestamp))  {
        fprintf (cgiOut,"%s",cyrest->_util->jsonerror(cyrest->errormsg()).c_str ());
        delete cyrest;
        return EXIT_FAILURE;
    }
    //////////////////////////////////////
    //  REST request
    string output = cyrest->request(cyw_request);
    fprintf (cgiOut,"%s",output.c_str ());
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
