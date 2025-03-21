/********************************************************************
          FILE:          cyw_mallreport.cpp
   DESCRIPTION:          Base class method(s)
 ********************************************************************/
//////////////////////////////////////////
//  Header(s)
#include <string>
#include <sstream>
//////////////////////////////////////////
//  Qt header files
#include <QObject>
#include <QThread>
#include <QUrlQuery>
#include <QEventLoop>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QCoreApplication>
#include <QNetworkAccessManager>
//////////////////////////////////////////
#include "cyrestcommon.h"
#include "mallreport/cyw_mallreport.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
cymallreport::cymallreport (cylog* log, cyini* ini, cycgi* cgi) :
    CYRestCommon(log, ini, cgi)
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
cymallreport::~cymallreport ()
{
    if (nullptr != _util)  {
        delete (_util);
        _util = nullptr;
    }
}
