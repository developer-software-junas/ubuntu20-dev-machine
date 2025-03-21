/********************************************************************
          FILE:         cyposrest.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  QT header(s)
#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
//////////////////////////////////////////
//  REST class

#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
typedef unsigned char byte;
/********************************************************************
      FUNCTION:         rest_receipt_raw
   DESCRIPTION:         Print the raw buffer
 ********************************************************************/
std::string             cyposrest::rest_receipt_raw ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string receipt = cgiform("receipt");
    string reference = cgiform("reference");
    string printer_code = cgiform("printer_code");

    receipt = _util->base64decode(receipt);
    _log->logmsg("QTRAWRECEIPT TYPE",type.c_str ());
    _log->logmsg("QTRAWRECEIPT REF",reference.c_str ());
    _log->logmsg("QTRAWRECEIPT PRINTER",printer_code.c_str ());
    _log->logmsg("QTRAWRECEIPT TEXT",receipt.c_str ());

    string timestamp = _util->strtime_t();
    if (!updateJournal(type,timestamp,reference,receipt))
        return _util->jsonerror(_error);

    string ret = "";
    if (printer_code != "DEFAULT")  {
        //////////////////////////////////////
        //  Print the logo
        string logoFile = _ini->get_value("DEVICE","LOGO");
        if (_util->file_exists(logoFile.c_str()))  {
            deviceNetworkImage(logoFile,printer_code);
        }
        if (type == "suspend")  {
            deviceNetwork (receipt,reference,printer_code);
            deviceNetworkImage(logoFile,printer_code);
        }
        for (int i = 0; i < printCopy; i++)
            ret = deviceNetwork (receipt,reference,printer_code);
    }  else  {        
        for (int i = 0; i < printCopy; i++)
            ret = deviceLocal (receipt.length(), receipt,
                         //(unsigned char*)receipt.c_str (),
                         type,reference);
    }
    return ret;
}
