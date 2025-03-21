/********************************************************************
          FILE:         cyposrest_network.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  QT header(s)
#include <QUrl>
#include <QImage>
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
//////////////////////////////////////
//  Image attribute(s)
#include <qbytearray.h>
extern QByteArray       _byteInit, _byteCenter,
                        _bytePrinter, _byteImage, _byteLocalImage;
/********************************************************************
      FUNCTION:         deviceNetwork
   DESCRIPTION:         Generic formatting, network printer
 ********************************************************************/
std::string             cyposrest::deviceNetwork (std::string buffer,
                                                  std::string reference,
                                                  std::string printurl)
{
    //////////////////////////////////////
    //  Print the logo
    string prefix = _ini->get_value("RECEIPT","TYPE");
    string logoFile = _ini->get_value("PATH","DOCROOT");
    logoFile += "/"; logoFile += prefix; logoFile += "logor.bmp";
    if (_util->file_exists(logoFile.c_str()))  {
        if (prefix != "elena")  {
            deviceNetworkImage(logoFile, printurl);
        }
    }
    qint64 retcode;
    QTcpSocket* _socket = new QTcpSocket();
    _socket->connectToHost(printurl.c_str(), 9100);

    if(_socket->waitForConnected(3000))  {
        qDebug() << "Connected!";
        //////////////////////////////////////
        //  Reset the printer
        QByteArray byteInit;
        byteInit.clear();
        //  Printer reset
        byteInit.append(char(27));
        byteInit.append(char(64));
        //  Left margin
        byteInit.append(char(29));
        byteInit.append(char(76));
        byteInit.append(char(22));
        byteInit.append(char(0));
        //  Initialize
        retcode = _socket->write(byteInit,byteInit.length());
        if (retcode == -1)  {
            return _util->jsonerror("Unable to access the network printer");
        }
        _socket->waitForBytesWritten(128);
        _socket->waitForReadyRead(128);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //  Print some stuff
        retcode = _socket->write(buffer.c_str(),buffer.length());
        if (retcode == -1)  {
            return _util->jsonerror("Unable to access the network printer");
        }
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  QR Code???
        char buf[250];
        string type = cgiform("type");
        if (type != "xread" && type != "zread" && reference.length() > 0)  {
            //////////////////////////////////////
            //  Reset the printer
            retcode = _socket->write(_byteInit,_byteInit.length());
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(256);
            _socket->waitForReadyRead(256);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //  Center
            retcode = _socket->write(_byteCenter,_byteCenter.length());
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(256);
            _socket->waitForReadyRead(256);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //
            //////////////////////////////////////
            //select the model
            char buf[1024];
            int store_len = reference.length() + 3;
            byte store_pL = (byte) (store_len % 256);
            byte store_pH = (byte) (store_len / 256);
            sprintf(buf, "%c%c%c%c%c%c%c%c%c", 29, 40, 107, 4, 0, 49, 65, 50, 0);
            //write (sfd,buf,9); usleep(1024*128);
            retcode = _socket->write(buf,9);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //set the size
            sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 67, 7); //7 as size
            //write (sfd,buf,8); usleep(1024*128);
            retcode = _socket->write(buf,8);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //set the error correction
            sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 69, 48);
            //write (sfd,buf,8); usleep(1024*128);
            retcode = _socket->write(buf,8);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //set the data 1
            sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, store_pL, store_pH, 49, 80, 48);
            //write (sfd,buf,8); usleep(1024*128);
            retcode = _socket->write(buf,8);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //set the data 2
            strcpy (buf,reference.c_str ());
            //write (sfd,buf,qrdata.length()); usleep(1024*128);
            retcode = _socket->write(buf,reference.length());
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            //write to storage
            sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 81, 48);
            //write (sfd,buf,8); usleep(1024*128);
            retcode = _socket->write(buf,8);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
            //////////////////////////////////////
            // Reset
            retcode = _socket->write(_byteInit,_byteInit.length());
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
        }
        //////////////////////////////////////
        //  Kick drawer
        if (type ==  "cashfund" || type == "cashtakeout" || type == "xread" || type == "zread")  {
            sprintf (buf, "%c%c%c%c%c", 27, 112, 48, 55, 121);
            retcode = _socket->write(buf,5);
            if (retcode == -1)  {
                return _util->jsonerror("Unable to access the network printer");
            }
            _socket->waitForBytesWritten(128);
            _socket->waitForReadyRead(128);
            qDebug() << "Reading: " << _socket->bytesAvailable();
            qDebug() << _socket->readAll();
        }
        //////////////////////////////////////
        //  Cut the paper - 27 105
        QByteArray byteDone;
        byteDone.clear();
        byteDone.append(char(27));
        byteDone.append(char(100));
        byteDone.append(char(5));
        byteDone.append(char(27));
        byteDone.append(char(105));
        retcode = _socket->write(byteDone, byteDone.length());
        if (retcode == -1)  {
            return _util->jsonerror("Unable to access the network printer");
        }
        _socket->waitForBytesWritten(128);
        _socket->waitForReadyRead(128);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        _socket->close();
    }  else  {
        return _util->jsonerror("Unable to access the network printer");
    }
/*
    string strPrintData = "";
    strPrintData += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
    strPrintData += "<s:Header>\r\n";
    strPrintData += "<parameter xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<devid>local_printer</devid>\r\n";
    strPrintData += "</parameter>\r\n";
    strPrintData += "</s:Header>\r\n";
    strPrintData += "<s:Body>\r\n";
    strPrintData += "<epos-print xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<text lang=\"em\" />\r\n";
    strPrintData += "<text smooth=\"true\" />\r\n";
    strPrintData += "<command>1d4c0f00</command>\r\n";

    strPrintData += "<text>";
    strPrintData += buffer;
    strPrintData += "</text>";

    string type = cgiform("type");
    if (type != "xread" && type != "zread")  {
        strPrintData += "<text align=\"center\" />\r\n";
        strPrintData += "<symbol width=\"6\" type=\"qrcode_model_2\"  level=\"level_m\">";
        strPrintData += reference;
        strPrintData += "</symbol>";
    }
    if (type ==  "cashfund" || type == "cashtakeout" || type == "xread" || type == "zread")  {
        strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_100\" />\r\n";
        strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_200\" />\r\n";
    }
    strPrintData += "\r\n\r\n\r\n<cut />\r\n";
    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";

    if (!deviceNetworkRaw(strPrintData,printurl))
        return _util->jsonerror("Unable to print");
    if (type == "cashfund" || type == "cashpullout")  {
        //  2nd copy
        if (!deviceNetworkRaw(strPrintData,printurl))
            return _util->jsonerror("Unable to print");
    }*/
    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=buffer;

    stringstream out;
    out << root;
    return out.str ();
}
/********************************************************************
      FUNCTION:         deviceNetwork
   DESCRIPTION:         Generic formatting, network printer
 ********************************************************************/
bool                    cyposrest::deviceNetworkText (std::string buffer,
                                                         std::string printurl)
{
    QTcpSocket* _socket = new QTcpSocket();
    _socket->connectToHost(printurl.c_str(), 9100);

    if(_socket->waitForConnected(3000))  {
        qDebug() << "Connected!";
        //////////////////////////////////////
        //  Reset the printer
        QByteArray byteInit;
        byteInit.clear();
        //  Printer reset
        byteInit.append(char(27));
        byteInit.append(char(64));
        //  Left margin
        byteInit.append(char(29));
        byteInit.append(char(76));
        byteInit.append(char(22));
        byteInit.append(char(0));
        //  Initialize
        _socket->write(byteInit,byteInit.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //  Print some stuff
        _socket->write(buffer.c_str(),buffer.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  Cut the paper - 27 105
        QByteArray byteDone;
        byteDone.clear();
        byteDone.append(char(27));
        byteDone.append(char(100));
        byteDone.append(char(5));
        byteDone.append(char(27));
        byteDone.append(char(105));
        _socket->write(byteDone, byteDone.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        _socket->close();
    }  else  {
        _error = "Unable to access the network printer";
        return false;
    }
/*
    string strPrintData = "";
    strPrintData += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
    strPrintData += "<s:Header>\r\n";
    strPrintData += "<parameter xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<devid>local_printer</devid>\r\n";
    strPrintData += "</parameter>\r\n";
    strPrintData += "</s:Header>\r\n";
    strPrintData += "<s:Body>\r\n";
    strPrintData += "<epos-print xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<text lang=\"em\" />\r\n";
    strPrintData += "<text smooth=\"true\" />\r\n";
    strPrintData += "<command>1d4c0f00</command>\r\n";

    strPrintData += "<text>";
    strPrintData += buffer;
    strPrintData += "</text>";

    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";

    if (!deviceNetworkRaw(strPrintData,printurl))
        return false;*/
    return true;
}
/********************************************************************
      FUNCTION:         deviceNetworkRaw
   DESCRIPTION:         Print to a networked printer
 ********************************************************************/
bool                    cyposrest::deviceNetworkRaw (std::string buffer,
                                                     std::string printurl)
{
    string printer = "";
    if (printurl.length() < 1)  {
        //////////////////////////////////
        //  Database connection
        string               sql;
        CYDbSql*             db = _ini->dbconn ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());
        //////////////////////////////////
        //  Select the printer
        db->sql_reset ();
        string printer_code = cgiform("printer_code");
        sql  = "select * from tg_pos_fnb_printer where printer_code = ";
        sql += db->sql_bind(1,printer_code);
        if (!db->sql_result(sql,true))
            return seterrormsg(db->errordb());
        if (!db->eof ())
            printer = db->sql_field_value("printer_url");
    }  else {
        printer = printurl;
    }
    _log->logmsg("QTRESTRECEIPT URL",printer.c_str ());
    //////////////////////////////////////
    //  Override???
    string override_url = "";
    if (deviceOverride(override_url))
        printer = override_url;
    //////////////////////////////////////
    //  Send the buffer
    QTcpSocket* _socket = new QTcpSocket();
    _socket->connectToHost(printurl.c_str(), 9100);

    if(_socket->waitForConnected(3000))  {
        qDebug() << "Connected!";
        //////////////////////////////////////
        //  Reset the printer
        QByteArray byteInit;
        byteInit.clear();
        //  Printer reset
        byteInit.append(char(27));
        byteInit.append(char(64));
        //  Left margin
        byteInit.append(char(29));
        byteInit.append(char(76));
        byteInit.append(char(22));
        byteInit.append(char(0));
        //  Initialize
        _socket->write(byteInit,byteInit.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //  Print some stuff
        _socket->write(buffer.c_str(),buffer.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  Cut the paper - 27 105
        QByteArray byteDone;
        byteDone.clear();
        byteDone.append(char(27));
        byteDone.append(char(100));
        byteDone.append(char(5));
        byteDone.append(char(27));
        byteDone.append(char(105));
        _socket->write(byteDone, byteDone.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        _socket->close();
    }  else  {
        _error = "Unable to access the network printer";
        return false;
    }
    /*
    struct_cymem mem;
    cymem_init (&mem,1024);
    cymem_https_post(80,18,printer.c_str (),buffer.c_str (),&mem);
    size_t st = mem.size;
    _log->logmsg("QTRESTRECEIPT HTTPRESULT",mem.memory);
    cymem_clean(&mem);
    if (st < 1)  {
        _error = "Unable to access the network printer";
        return false;
    }*/
    return true;
}
/********************************************************************
      FUNCTION:         deviceNetworkRaw
   DESCRIPTION:         Print to a networked printer
 ********************************************************************/
bool                    cyposrest::deviceNetworkImage (std::string imageFile,
                                                       std::string printurl)
{
    string printer = "";
    if (printurl.length() < 1)  {
        //////////////////////////////////
        //  Database connection
        string               sql;
        CYDbSql*             db = _ini->dbconn ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());
        //////////////////////////////////
        //  Select the printer
        db->sql_reset ();
        string printer_code = cgiform("printer_code");
        sql  = "select * from tg_pos_fnb_printer where printer_code = ";
        sql += db->sql_bind(1,printer_code);
        if (!db->sql_result(sql,true))
            return seterrormsg(db->errordb());
        if (db->eof ())  {
            db->sql_reset ();
            sql  = "select * from tg_pos_fnb_printer where printer_url = ";
            sql += db->sql_bind(1,printer_code);
            if (!db->sql_result(sql,true))  {
                _error = _util->jsonerror(db->errordb());
                return false;
            }
            if (db->eof ())  {
                _error = _util->jsonerror(printer_code.c_str(),"Unable to retrieve the printer code detail(s)");
                return false;
            }
        }
        printer = db->sql_field_value("printer_url");
    }  else {
        printer = printurl;
    }
    _log->logmsg("QTRESTRECEIPT URL",printer.c_str ());
    //////////////////////////////////////
    //  Override???
    string override_url = "";
    if (deviceOverride(override_url))
        printer = override_url;
    //////////////////////////////////////
    //  Print data
    QTcpSocket* _socket = new QTcpSocket();
    _socket->connectToHost(printer.c_str(), 9100);
    if(_socket->waitForConnected(3000))  {
        qDebug() << "Connected!";
        //////////////////////////////////////
        //  assemble the image command(s)
        assembleImage(imageFile);
        //////////////////////////////////////
        //  Reset the printer
        _socket->write(_byteInit,_byteInit.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  Center
        _socket->write(_byteCenter,_byteCenter.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  Load the image
        QImage image;
        image.load (imageFile.c_str ());
        //////////////////////////////////////
        // Send the print command
        _socket->write(_bytePrinter,_bytePrinter.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        // Send the image data
        _socket->write(_byteImage,_byteImage.length());
        _socket->waitForBytesWritten(1000);
        _socket->waitForReadyRead(1000);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        usleep(1024*128);
        //////////////////////////////////////
        // Reset
        _socket->write(_byteInit,_byteInit.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        _socket->close();
    }  else  {
        _error = "Unable to access the network printer";
        return false;
    }
    /*
    string strPrintData = "";
    strPrintData += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
    strPrintData += "<s:Header>\r\n";
    strPrintData += "<parameter xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<devid>local_printer</devid>\r\n";
    strPrintData += "</parameter>\r\n";
    strPrintData += "</s:Header>\r\n";
    strPrintData += "<s:Body>\r\n";
    strPrintData += "<epos-print xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<text lang=\"em\" />\r\n";
    strPrintData += "<text smooth=\"true\" />\r\n";

    strPrintData += "<command>";
    strPrintData += _command;
    strPrintData += "</command>";

    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";
    //////////////////////////////////////
    //  Send the buffer
    struct_cymem mem;
    cymem_init (&mem,1024);
    cymem_https_post(80,18,printer.c_str (),strPrintData.c_str (),&mem);
    size_t st = mem.size;
    _log->logmsg("QTRESTRECEIPT HTTPRESULT",mem.memory);
    cymem_clean(&mem);
    if (st < 1)  {
        _error = "Unable to access the network printer";
        return false;
    }*/
    return true;
}
/********************************************************************
      FUNCTION:         deviceNetworkRaw
   DESCRIPTION:         Print to a networked printer
 ********************************************************************/
bool                    cyposrest::deviceImageCommand(std::string imageFile)
{
    //////////////////////////////////////
    //  Load the image
    string command = "";
    assembleImage(imageFile);

    _byteLocalImage.clear();
    _byteLocalImage += _byteInit;
    _byteLocalImage += _byteCenter;
    _byteLocalImage += _bytePrinter;
    _byteLocalImage += _byteImage;
    _byteLocalImage += _byteInit;

    for (int i = 0; i < _byteLocalImage.size(); i++)  {
        char sztmp[8];
        unsigned char ch;

        ch = _byteLocalImage[i];
        sprintf (sztmp, "%02x", ch);
        command += sztmp;
    }
    _command = command;
    return true;
}
/********************************************************************
      FUNCTION:         deviceOverride
   DESCRIPTION:         Override a network printer???
 ********************************************************************/
bool                    cyposrest::deviceOverride (std::string& printer_url)
{
    //////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////
    //  Retrieve argiment(s)
    string branch_code = cgiform("location_code");
    string register_num = cgiform("register_num");
    string printer_code = cgiform("printer_code");
    //////////////////////////////////
    //  Select the printer
    db->sql_reset ();
    sql  = "select * from tg_pos_fnb_printer_override ";
    sql += "where  location_code = "; sql += db->sql_bind(1,branch_code);
    sql += " and   register_num = "; sql += db->sql_bind(2,register_num);
    sql += " and   printer_code = "; sql += db->sql_bind(3,printer_code);
    if (!db->sql_result(sql,true))  {
        _log->logmsg("QTRESTRECEIPT OVERRIDE ERROR",db->errordb().c_str ());
        return seterrormsg(db->errordb());
    }
    if (!db->eof ()) {
        printer_url = db->sql_field_value("printer_url");
        _log->logmsg("QTRESTRECEIPT OVERRIDE FOUND",printer_url.c_str ());
        return true;
    }  else {
        _log->logmsg("QTRESTRECEIPT OVERRIDE ","NOT FOUND");
        printer_url = "";
    }
    return false;
}
/********************************************************************
      FUNCTION:         deviceNetworkQr
   DESCRIPTION:         Print to a networked printer
 ********************************************************************/
bool                    cyposrest::deviceNetworkQr (std::string qrcode,
                                                    std::string printurl)
{
    string printer = "";
    if (printurl.length() < 1)  {
        //////////////////////////////////
        //  Database connection
        string               sql;
        CYDbSql*             db = _ini->dbconn ();
        if (nullptr == db)
            return seterrormsg(_ini->errormsg());
        //////////////////////////////////
        //  Select the printer
        db->sql_reset ();
        string printer_code = cgiform("printer_code");
        sql  = "select * from tg_pos_fnb_printer where printer_code = ";
        sql += db->sql_bind(1,printer_code);
        if (!db->sql_result(sql,true))
            return seterrormsg(db->errordb());
        if (db->eof ())  {
            db->sql_reset ();
            sql  = "select * from tg_pos_fnb_printer where printer_url = ";
            sql += db->sql_bind(1,printer_code);
            if (!db->sql_result(sql,true))  {
                _error = _util->jsonerror(db->errordb());
                return false;
            }
            if (db->eof ())  {
                _error = _util->jsonerror(printer_code.c_str(),"Unable to retrieve the printer code detail(s)");
                return false;
            }
        }
        printer = db->sql_field_value("printer_url");
    }  else {
        printer = printurl;
    }
    _log->logmsg("QTRESTRECEIPT URL",printer.c_str ());
    //////////////////////////////////////
    //  Override???
    string override_url = "";
    if (deviceOverride(override_url))
        printer = override_url;
    //////////////////////////////////////
    //  Print data
    QTcpSocket* _socket = new QTcpSocket();
    _socket->connectToHost(printer.c_str(), 9100);
    if(_socket->waitForConnected(3000))  {
        qDebug() << "Connected!";
        //////////////////////////////////////
        //  Reset the printer
        _socket->write(_byteInit,_byteInit.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //  Center
        _socket->write(_byteCenter,_byteCenter.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //
        //////////////////////////////////////
        //select the model
        char buf[1024];
        int store_len = qrcode.length() + 3;
        byte store_pL = (byte) (store_len % 256);
        byte store_pH = (byte) (store_len / 256);
        sprintf(buf, "%c%c%c%c%c%c%c%c%c", 29, 40, 107, 4, 0, 49, 65, 50, 0);
        //write (sfd,buf,9); usleep(1024*128);
        _socket->write(buf,9);
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //set the size
        sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 67, 7); //7 as size
        //write (sfd,buf,8); usleep(1024*128);
        _socket->write(buf,8);
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //set the error correction
        sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 69, 48);
        //write (sfd,buf,8); usleep(1024*128);
        _socket->write(buf,8);
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //set the data 1
        sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, store_pL, store_pH, 49, 80, 48);
        //write (sfd,buf,8); usleep(1024*128);
        _socket->write(buf,8);
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //set the data 2
        strcpy (buf,qrcode.c_str ());
        //write (sfd,buf,qrdata.length()); usleep(1024*128);
        _socket->write(buf,qrcode.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        //write to storage
        sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 81, 48);
        //write (sfd,buf,8); usleep(1024*128);
        _socket->write(buf,8);
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        //////////////////////////////////////
        // Reset
        _socket->write(_byteInit,_byteInit.length());
        _socket->waitForBytesWritten(512);
        _socket->waitForReadyRead(512);
        qDebug() << "Reading: " << _socket->bytesAvailable();
        qDebug() << _socket->readAll();
        _socket->close();
    }  else  {
        _error = "Unable to access the network printer";
        return false;
    }
    /*
    string strPrintData = "";
    strPrintData += "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">\r\n";
    strPrintData += "<s:Header>\r\n";
    strPrintData += "<parameter xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<devid>local_printer</devid>\r\n";
    strPrintData += "</parameter>\r\n";
    strPrintData += "</s:Header>\r\n";
    strPrintData += "<s:Body>\r\n";
    strPrintData += "<epos-print xmlns=\"http://www.epson-pos.com/schemas/2011/03/epos-print\">\r\n";
    strPrintData += "<text lang=\"em\" />\r\n";
    strPrintData += "<text smooth=\"true\" />\r\n";

    strPrintData += "<command>";
    strPrintData += _command;
    strPrintData += "</command>";

    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";
    //////////////////////////////////////
    //  Send the buffer
    struct_cymem mem;
    cymem_init (&mem,1024);
    cymem_https_post(80,18,printer.c_str (),strPrintData.c_str (),&mem);
    size_t st = mem.size;
    _log->logmsg("QTRESTRECEIPT HTTPRESULT",mem.memory);
    cymem_clean(&mem);
    if (st < 1)  {
        _error = "Unable to access the network printer";
        return false;
    }*/
    return true;
}
