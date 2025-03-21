/*******************************************************************
   FILE:                cycmd.h
   DESCRIPTION:         Command-line header
 *******************************************************************/
#ifndef __CYCMD_H
#define __CYCMD_H
///////////////////////////////////////////
//  Standard headers
#include <string.h>
#include <string>
#include <sstream>
///////////////////////////////////////////
//  User header file(s)
#include "cyw_aes.h"
#include "cyw_log.h"
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
class                   CYTable;
/***************************************************************************
         CLASS:         CYCommandLine
   DESCRIPTION:         Command line class
 ***************************************************************************/
class                   CYCommandLine
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    CYCommandLine (cyini* ini, cylog* log, cyutility* util);
    virtual ~CYCommandLine ();
    //////////////////////////////////////
    //  Operation(s)
    bool                cleanupFiles ();
    bool                generateSqlite (std::string locationCode,
                                        std::string logicalDate);
    bool                generateSchema (std::string locationCode,
                                        std::string logicalDate);
    bool                importSchema (std::string locationCode,
                                      std::string logicalDate);
    bool                compressMaster (std::string locationCode,
                                        std::string logicalDate);
    bool                compressAndPublish (std::string locationCode,
                                            std::string logicalDate);
    //////////////////////////////////////
    //  Acess methods
    virtual std::string errormsg ();
protected:
    //////////////////////////////////////
    //  Attribute(s)
    cyini*              _ini;
    cylog*              _log;
    cyutility*          _util;
    std::string         _tar = "",
                        _error = "";
};
#endif
