/********************************************************************
          FILE:         cyposrest_receipt.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
#include <QImage>
#include "cyposrest.h"
#include "cyrestcommon.h"
#include "cyrestreceipt.h"
#include "osxport/serialport.hpp"
using std::string;
using std::stringstream;
typedef unsigned char byte;
//////////////////////////////////////
//  Image attribute(s)
#include <qbytearray.h>
QByteArray              _byteInit, _byteCenter,
                        _bytePrinter, _byteImage, _byteLocalImage;
/***************************************************************************
      FUNCTION:          rest_drawer
   DESCRIPTION:          Check if the drawer is open
 ***************************************************************************/
std::string              cyposrest::rest_drawer ()
{
    //////////////////////////////////////
    //  Parameter(s)
    char bufCheck [12];
    string receiptType = _ini->get_value("RECEIPT","TYPE");
    sprintf (bufCheck, "%c%c%c", 29, 114, 50);
    string usbPort = _ini->get_value ("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    const int baudRate = 19200;
    int sfd = openAndConfigureSerialPort(usbPort.c_str (), baudRate);
    if (sfd < 0) {
        return _util->jsonerror("Drawer","Invalid printer port");
    }

    int status = 0;
    char bufResponse [12];
    memset (bufResponse, 0x00, 12);
    writeSerialData(bufCheck, 3);
    readSerialData(bufResponse, 8);
    closeSerialPort();

    if (bufResponse[1]==16)  {
        status = 1;
    } else if (bufResponse[1]==20)  {
        status = 2;
    }

    Json::Value jroot;
    jroot["status"]="ok";
    if (status == 1) jroot["drawer"]="open";
    else if (status == 2) jroot["drawer"]="close";
    else jroot["drawer"]="unknown";

    stringstream out;
    out << jroot;
    return out.str ();
}
/***************************************************************************
      FUNCTION:          rest_drawer
   DESCRIPTION:          Check if the drawer is open
 ***************************************************************************/
std::string              cyposrest::rest_drawer_open ()
{
    deviceKick();
    Json::Value jroot;
    jroot["status"]="ok";

    stringstream out;
    out << jroot;
    return out.str ();
}
/***************************************************************************
      FUNCTION:          updateJournal
   DESCRIPTION:          Update the receipt journal
 ***************************************************************************/
bool                     cyposrest::updateJournal (std::string type,
                                                      std::string cytimestamp,
                                                      std::string systransnum,
                                                      std::string receiptdata)
{
    return true;
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg (_ini->errormsg());
    //////////////////////////////////////
    //  Save to receipt journal
    db->sql_reset();
    sql  = "delete from tg_pos_journal where systransnum = ";
    sql += db->sql_bind(1, systransnum); sql += " ";
    sql += " and type = ";
    sql += db->sql_bind(2, type); sql += " ";
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());

    db->sql_reset();
    sql  = "insert into tg_pos_journal (systransnum, type, timestamp, ";
    sql += "txt_receipt, is_polled2) values (";
    sql += db->sql_bind(1, systransnum); sql += ", ";
    sql += db->sql_bind(2, type); sql += ", ";
    sql += db->sql_bind(3, cytimestamp); sql += ", ";
    sql += db->sql_bind(4, receiptdata); sql += ", 0) ";
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
    return true;
}
/***************************************************************************
      FUNCTION:          assembleImage
   DESCRIPTION:          Assemble ESC / POS commands to print an image
 ***************************************************************************/
bool                     cyposrest::assembleImage (std::string imageFile)
{
    //////////////////////////////////////
    //  Reset the printer
    _byteInit.clear();
    _byteInit.append(char(27));
    _byteInit.append(char(64));
    //////////////////////////////////////
    //  Center
    _byteCenter.clear ();
    _byteCenter.append(char(27));
    _byteCenter.append(char(97));
    _byteCenter.append(char(1));
    //////////////////////////////////////
    //  Load the image
    QImage image;
    image.load (imageFile.c_str ());

    qreal scaleFactor = 6.0; // Increase DPI by a factor of 2
    qreal darknessFactor =1.0; // Adjust darkness level (experiment with this value)
    // Scale the image for higher DPI (dots per inch)
    QImage highDpiImage = image;

    // Disable device pixel ratio scaling (anti-aliasing)
    highDpiImage.setDevicePixelRatio(1);

    QImage monochromeImage = highDpiImage.convertToFormat(QImage::Format_Mono);
    highDpiImage = monochromeImage.scaled(monochromeImage.width() * scaleFactor, monochromeImage.height()
                                          * scaleFactor, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Calculate the width of the image in bytes
    int bytesPerLine = (monochromeImage.width() + 7) / 8;
    // Send the ESC/POS command to print the image
    _bytePrinter.clear();
    _bytePrinter.append(char(0x1D));  // ESC
    _bytePrinter.append(char(0x76));  // 'v'
    _bytePrinter.append(char(0x30));  // '0'
    _bytePrinter.append(char(0x00));  // Mode (0 for normal)

    // Append the image width and height
    _bytePrinter.append(char(bytesPerLine & 0xFF));
    _bytePrinter.append(char((bytesPerLine >> 8) & 0xFF));
    _bytePrinter.append(char(image.width() & 0xFF));
    _bytePrinter.append(char((image.height() >> 8) & 0xFF));

    // Image data
    _byteImage.clear();
    for (int y = 0; y < monochromeImage.height(); ++y)
    {
        for (int x = 0; x < bytesPerLine; ++x)
        {
            uchar byte = 0;
            for (int bit = 0; bit < 8; ++bit)
            {
                int pixelX = x * 8 + bit;
                if (pixelX < monochromeImage.width())
                {
                    bool isBlackPixel = monochromeImage.pixel(pixelX, y) == qRgb(0, 0, 0);
                    byte |= (isBlackPixel ? 0x01 : 0x00) << (7 - bit); // Corrected bit order

                }
            }
            byte *= darknessFactor; // Adjust this factor for darkness level

            _byteImage.append(byte);
        }
    }
    return true;
}
/***************************************************************************
      FUNCTION:          rest_validate
   DESCRIPTION:          Print Validate on specified postion
 ***************************************************************************/
std::string              cyposrest::rest_validate (int position, std::string data)
{
    //////////////////////////////////////
    //  Parameter(s)
    char buf [32];
    sprintf (buf, "%c%c%c", 29, 114, 50);// = "\x1D\x72\x32";
    string usbPort = _ini->get_value ("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    const int baudRate = 19200;
    int sfd = openAndConfigureSerialPort(usbPort.c_str (), baudRate);
    if (sfd < 0) {
        return _util->jsonerror("Print Validate","Invalid printer port");
    }



    sprintf (buf, "%c%c%c", 0x1b, 0x3d, 0x01);
    writeSerialData(buf, 3);

    sprintf (buf, "%c%c%c%c", 0x1b, 0x63, 0x30, 4);
    writeSerialData(buf, 4);

    sprintf (buf, "%c%c%c%c", 0x1b, 0x66, 1, 1);
    writeSerialData(buf, 4);

    //position feed
    for(int j=0; j<position; j++ ){

        sprintf (buf, "\n\n\n\n\n\n\n");
        writeSerialData(buf, 7);

    }

    //data

    size_t st, length = data.length();
    if (length < 100)  {
        st = write (sfd, data.c_str(), length);
        if (st != length)
            _log->logdebug("UNABLE TO WRITE\n",data.c_str());
    }  else  {
        st = write (sfd, data.c_str(), 100);
        if (st != length)
            usleep(256000);
        if (length > 200)  {
            st = write (sfd, data.c_str() + 100, 100);
        }  else  {
            st = write (sfd, data.c_str() + 100, length - 100);
        }
        if (st > 0)
            usleep(256000);
        if (length > 300)  {
            st = write (sfd, data.c_str() + 200, 100);
        }  else  {
            st = write (sfd, data.c_str() + 200, length - 200);
        }
        if (st > 0)
            usleep(256000);
        if (length > 400)  {
            st = write (sfd, data.c_str() + 400, 100);
        }  else  {
            st = write (sfd, data.c_str() + 400, length - 300);
        }
        if (st > 0)
            usleep(128000);
    }
    //writeSerialData(data.c_str(), length);
    _log->logdebug("VALIDATE\n",data.c_str());

    sprintf (buf, "%c", 0x0c);
    writeSerialData(buf, 1);

    sprintf (buf, "%c", 0x18);
    writeSerialData(buf, 1);

    sprintf (buf, "%c%c%c%c", 0x1b, 0x63, 0x30, 2);
    writeSerialData(buf, 4);

    sprintf (buf, "%c%c", 0x1b, 0x40);
    writeSerialData(buf, 2);
    closeSerialPort();

    Json::Value jroot;
    jroot["status"]="ok";
    jroot["validate"]="done";

    stringstream out;
    out << jroot;
    return out.str ();
}
/********************************************************************
      FUNCTION:         rest_receipt
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             cyposrest::rest_receipt ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string reference = cgiform("reference");
    string printer_code = cgiform("printer_code");

    _log->logmsg("QTRESTRECEIPT TYPE REC:STSELENA",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",reference.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Retrieve receipt data
    db->sql_reset ();
    string receipt = "";
    sql  = "select * from tg_pos_receipt where systransnum = '";
    sql += reference;
    sql += "' and type = '";
    sql += type;
    sql += "' ";

    if (!db->sql_result(sql,false))
        return _util->jsonerror(db->errordb());
    if (db->eof ())  {
        receipt = cgiform("receipt");
        receipt = _util->base64decode(receipt);
        _log->logmsg("QTRESTRECEIPT ALTERNATE",receipt.c_str ());
    }  else  {
        //////////////////////////////////////
        //  Assemble the receipt
        string txt_header, txt_detail, txt_account, txt_totals,
            txt_payment, txt_vat, txt_signature, txt_footer,
            txt_giftwrap, txt_extloyalty, copies, printer_name;

        copies = db->sql_field_value("copies");
        printer_name = db->sql_field_value("printer_name"); // kitchen or default-future use

        txt_header = db->sql_field_value("txt_header");
        txt_detail = db->sql_field_value("txt_detail");
        txt_account = db->sql_field_value("txt_account");
        txt_totals = db->sql_field_value("txt_totals");
        txt_payment = db->sql_field_value("txt_payment");
        txt_vat = db->sql_field_value("txt_vat");
        txt_signature = db->sql_field_value("txt_signature");
        txt_footer = db->sql_field_value("txt_footer");
        txt_giftwrap += db->sql_field_value("txt_giftwrap");
        txt_extloyalty += db->sql_field_value("txt_giftwrap");

        receipt  = txt_header;
        receipt += txt_detail;
        receipt += txt_totals;
        receipt += txt_payment;
        receipt += txt_account;
        receipt += txt_vat;
        receipt += txt_signature;
        receipt += txt_footer;
        receipt += txt_giftwrap;
        receipt += txt_extloyalty;
    }
    db->sql_reset ();
    sql  = "select count(*) as recs from tg_pos_mobile_payment where systransnum = '";
    sql += reference;
    sql += "'  and pay_type_code in (select z.tender_type_cd from pos_tender_type z where is_cash = 1)";
    if (!db->sql_result(sql,false))
        return _util->jsonerror(db->errordb());
    int recs = atoi(db->sql_field_value("recs").c_str());

    if (printCopy < 1) printCopy = 1;
    if (printer_code == "DEFAULT")  {        
        string codedReceipt = _util->base64encode(receipt);
        _cgi->add_form("receipt", codedReceipt);

        for (int i = 0; i < printCopy; i++)
            rest_receipt_raw();
        if (recs > 0)
            deviceKick();
        //////////////////////////////////////
        //  Assemble the JSON response
        Json::Value root;
        root["status"]="ok";
        root["receipt"]=receipt;

        stringstream out;
        out << root;
        return out.str ();
    }
    //////////////////////////////////////
    //  Select the printer
    db->sql_reset ();
    string strPrinterURL="";
    sql  = "select * from tg_pos_fnb_printer where printer_code = ";
    sql += db->sql_bind(1,printer_code);
    if (!db->sql_result(sql,true))
        return _util->jsonerror(db->errordb());
    if (db->eof ())  {
        db->sql_reset ();
        sql  = "select * from tg_pos_fnb_printer where printer_url = ";
        sql += db->sql_bind(1,printer_code);
        if (!db->sql_result(sql,true))
            return _util->jsonerror(db->errordb());
        if (db->eof ())
            return _util->jsonerror(printer_code.c_str(),"Unable to retrieve the printer code detail(s)");
    }
    strPrinterURL = db->sql_field_value("printer_url");
    for (int i = 0; i < printCopy; i++)
        deviceNetwork(receipt,reference,strPrinterURL);

    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=receipt;

    stringstream out;
    out << root;
    return out.str ();
}
bool                    cyposrest::signatureCheck (std::string referenceNumber)  {
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Retrieve receipt data
    db->sql_reset ();
    string receipt = "";
    sql  = "select a.*, b.is_cashgate_pay as signature ";
    sql += "from   tg_pos_mobile_payment a, pos_tender_type b ";
    sql += "where  a.pay_type_code = b.tender_type_cd ";
    sql += "and    b.is_cashgate_pay = 1 ";
    sql += "and    systransnum = '";
    sql += referenceNumber;
    sql += "' ";

    if (!db->sql_result(sql,false))
        return seterrormsg(db->errordb());
    return (db->eof ()==false);
}
