/*******************************************************************
          FILE:          cyw_log.h
   DESCRIPTION:          Message logging class header
 *******************************************************************/
#ifndef __CYWARE_LOG_H
#define __CYWARE_LOG_H
#include <string>
class cyini;
/***************************************************************************
         CLASS:          cylog
   DESCRIPTION:          Message log class definition
 ***************************************************************************/

class                    cylog
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cylog (std::string filename);
    virtual ~cylog ();
    ///////////////////////////////////////
    //  Method(s)
    virtual void         logblank ();
    virtual void         trace  (const char* function,
                                 const char* message);
    virtual void         logmsg (const char* function,
                                 const char* message,
                                 bool override = false);
    virtual void         logtemp  (const char* function,
                                   const char* message,
                                   bool override = false);
    virtual void         logdebug (const char* function,
                                   const char* message,
                                   bool override = false);
    virtual void         logjson (const char* josnString);
    std::string          _logfile;
protected:
    bool                 _islog,
                         _istrace,
                         _issyslog;
};
#endif
