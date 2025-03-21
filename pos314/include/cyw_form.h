/*******************************************************************
          FILE:          cyw_form.h
   DESCRIPTION:          CGI variable class header
 *******************************************************************/
#ifndef __CYWARE_FORM_H
#define __CYWARE_FORM_H
#include <string>
#include <sstream>
#include <iostream>
#include "cyw_mem.h"
#include "cyw_colbind.h"
/***************************************************************************
         CLASS:          cycgiform
   DESCRIPTION:          CGI variable class definition
 ***************************************************************************/
class                    cycgiform
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cycgiform ();
    virtual ~cycgiform ();
    ///////////////////////////////////////
    //  Operation(s)
    void                 reset  ();
    bool                 append (const char* name,
                                 const char* value);

    std::string          args ();
    std::string          args_h ();
    std::string          args_hmac ();
    std::string          shopifyargs ();
    std::string          dump (const char* script);

    std::string          form (int idx);
    std::string          name (int idx);
    std::string          form (std::string name);
    std::string          form  (const char* name,
                                const char* defval,
                                bool partial=false);
    std::stringstream    _error;
    std::string          _buffer;

    ///////////////////////////////////////
    //  Form agument array
    int                  _idx;
    struct_cymem         _argname     [CYBINDVARMAX],
                         _argvalue    [CYBINDVARMAX];
};
#endif
