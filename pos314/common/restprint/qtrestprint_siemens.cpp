/********************************************************************
          FILE:         cyposrest_local.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  QT header(s)
#include <QUrl>
#include <QImage>
#include <qimage.h>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
#include "cyposrest.h"
#include "osxport/serialport.hpp"
using std::string;
using std::stringstream;
typedef unsigned char byte;
//////////////////////////////////////
//  Image attribute(s)
#include <qbytearray.h>
extern QByteArray       _byteInit, _byteCenter,
                        _bytePrinter, _byteImage, _byteLocalImage;
/********************************************************************
      FUNCTION:         deviceSiemensImage
   DESCRIPTION:         Print a barcode
 ********************************************************************/
bool                    cyposrest::deviceSiemensImage (string qrdata)
{
    string cmd   = "/usr/bin/qrencode --dpi=600 --size=4 -o ";
    string image = "/usr/local/data/"; image+= qrdata; image += ".png";
    cmd += image; cmd += " \""; cmd += qrdata; cmd += "\"";

    cmd = "/usr/bin/qrencode --size=4 -o ";
    cmd += image; cmd += " \""; cmd += qrdata; cmd += "\"";
    //8.png -t png \"8\"";
    system (cmd.c_str());

    deviceImage(image);
    return true;
}
