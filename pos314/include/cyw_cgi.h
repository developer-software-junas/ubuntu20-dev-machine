/*******************************************************************
          FILE:          cyw_cgi.h
   DESCRIPTION:          CGI class header
 *******************************************************************/
#ifndef __CYWARE_CGI_H
#define __CYWARE_CGI_H
#include <string>
#include <pthread.h>
///////////////////////////////////////////
//  The CGI library
extern "C"  {
#include "cgic.h"
}
#include "cyw_mem.h"
#include "cyw_form.h"
///////////////////////////////////////////
//  Classes used
class                    cycgiform;
///////////////////////////////////////////
//  Day of week
const std::string g_weekday [] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
/***************************************************************************
         CLASS:          cycgi
   DESCRIPTION:          CGI class definition
 ***************************************************************************/

class                    cycgi
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cycgi (int argc, char** argv);
    virtual ~cycgi ();
    ///////////////////////////////////////
    //  Operation(s)
    void                 die  ();
    void                 echo (std::string value);
    void                 reset_cookie (std::string name,
                                       std::string domain);
    void                 reset_output (std::string htmlhdr);

    std::string          cleanlogfile (bool pos=true);
    std::string          getinifile (std::string variant, bool pos=true);
    std::string          getlogfile (std::string variant, bool pos=true);

    cycgiform*           arglist ();
    std::string          cookiedump ();    
    std::string          formdump (std::string script="");

    std::string          form (int idx);
    std::string          formname (int idx);
    std::string          form (const char* name);
    std::string          form (std::string name,
                               std::string defvalue="");

    bool                 form (std::string name,
                               struct_cymem* value);

    bool                 cookie (const char* name,
                                 struct_cymem* value);

    std::string          header (std::string name);
    std::string          cookie (std::string name);
    std::string          cookie_create (std::string name,
                                        std::string value,
                                        std::string domain,
                                        long lexpire);
    ///////////////////////////////////////
    //  Access method(s)
    bool                 hdr  ();
    bool                 dead ();
    bool                 buffered ();
    void                 set_buffered (bool buffer);
    void                 setnohdr ()  { _hdr = true; }
    ///////////////////////////////////////
    //  Form argument method(s)
    bool                 add_form (std::string name,
                                   std::string value);
    bool                 add_cookie (std::string name,
                                     std::string value);
    bool                 add_header (std::string name,
                                     std::string value);
    ///////////////////////////////////////
    //  CGI environment variable(s)
    std::string          buffer ();
    std::string          referrer ();
    std::string          user_agent  ();
    std::string          remote_host ();
    std::string          remote_addr ();
    std::string          server_port ();
    std::string          server_name ();
    std::string          script_name ();
    std::string          request_method ();
protected:
    ///////////////////////////////////////
    //  CGI flag(s)
    bool                 _hdr,_die,_buffer;
    ///////////////////////////////////////
    //  HTML buffer / INI file
    std::string          _output;
    std::string          _inifile;
    std::string          _cgiPath;
    ///////////////////////////////////////
    //  HTML output mutex locker
    pthread_mutex_t      _lock;
    ///////////////////////////////////////
    //  Form attribute(s)
    cycgiform            _cgiform;
    struct_cymem         _cookiename  [CYBINDVARMAX],
                         _cookievalue [CYBINDVARMAX];
    struct_cymem         _shopifyname [CYBINDVARMAX],
                         _shopifyvalue[CYBINDVARMAX];
};
#endif
