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
      FUNCTION:         deviceImage
   DESCRIPTION:         Send the buffer to the local printer
 ********************************************************************/
bool                    cyposrest::deviceImage (std::string imageFile)
{
    //////////////////////////////////////
    //  Port settings
    size_t st = 0;
    const int baudRate = 19200;
    const string usbport = _ini->get_value("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
    if (sfd < 0) {
        _error  = "Unable to connect to serial port: ";
        _error += usbport;
        return false;
    }
    usleep(10000);
    //////////////////////////////////////
    //  assemble the image command(s)
    assembleImage(imageFile);
    //////////////////////////////////////
    //  Reset the printer
    st = write (sfd, _byteInit, _byteInit.length());
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //  Center
    st = write (sfd, _byteCenter, _byteCenter.length());
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //  Load the image
    QImage image;
    image.load (imageFile.c_str ());
    //////////////////////////////////////
    // Send the print command
    st = write (sfd, _bytePrinter, _bytePrinter.length());
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    // Send the image data
    st = write (sfd, _byteImage, _byteImage.length());
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    // Reset
    st = write (sfd, _byteInit, _byteInit.length());
    if (st > 0)
        sleep(1);
    //usleep(1024*128);
    //////////////////////////////////////
    //  DONE
    closeSerialPort();

    return true;
}
/********************************************************************
      FUNCTION:         deviceCut
   DESCRIPTION:         Send the buffer to the local printer
 ********************************************************************/
bool                    cyposrest::deviceCut ()
{
    //////////////////////////////////////
    //  Port settings
    const int baudRate = 19200;
    const string usbport = _ini->get_value("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
    if (sfd < 0) {
        _error  = "Unable to connect to serial port: ";
        _error += usbport;
        return false;
    }
    //////////////////////////////////////
    //  reset printer
    char buf [250];
    strcpy (buf, "\n\n\n\n\n\n\x1b\x69");
    size_t st = write (sfd,buf,8);
    if (st > 0)
        usleep(1024*128);
    closeSerialPort();

    return true;
}
/********************************************************************
      FUNCTION:         deviceDrawerKick
   DESCRIPTION:         Send the buffer to the local printer
 ********************************************************************/
bool                    cyposrest::deviceKick()
{
    //////////////////////////////////////
    //  Port settings
    const int baudRate = 19200;
    const string usbport = _ini->get_value("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
    if (sfd < 0) {
        _error  = "Unable to connect to serial port: ";
        _error += usbport;
        return false;
    }
    //////////////////////////////////////
    //  reset printer
    char buf [250];
    sprintf (buf, "%c%c%c%c%c", 27, 112, 48, 55, 121);
    size_t st = write (sfd,buf,5);
    if (st > 0)
        usleep(1024*128);
    closeSerialPort();

    return true;
}
/********************************************************************
      FUNCTION:         deviceQrPrint
   DESCRIPTION:         Send the buffer to the local printer
 ********************************************************************/
bool                    cyposrest::deviceQrPrint (string qrdata)
{
    //////////////////////////////////////
    //  Port settings
    const int baudRate = 19200;
    const string usbport = _ini->get_value("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
    if (sfd < 0) {
        _error  = "Unable to connect to serial port: ";
        _error += usbport;
        return false;
    }
    //////////////////////////////////////
    //  reset printer
    char buf [250];
    size_t st = 0;
    sprintf(buf, "%c%c", 27, 64);
    st = write (sfd,buf,2);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //align center
    sprintf(buf, "%c%c%c", 27, 97, 1);
    st = write (sfd,buf,3);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //select the model
    int store_len = qrdata.length() + 3;
    byte store_pL = (byte) (store_len % 256);
    byte store_pH = (byte) (store_len / 256);
    sprintf(buf, "%c%c%c%c%c%c%c%c%c", 29, 40, 107, 4, 0, 49, 65, 50, 0);
    st = write (sfd,buf,9);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //set the size
    sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 67, 7); //7 as size
    st = write (sfd,buf,8);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //set the error correction
    sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 69, 48);
    st = write (sfd,buf,8);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //set the data 1
    sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, store_pL, store_pH, 49, 80, 48);
    st = write (sfd,buf,8);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //set the data 2
    strcpy (buf,qrdata.c_str ());
    st = write (sfd,buf,qrdata.length());
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //write to storage
    sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 81, 48);
    st = write (sfd,buf,8);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //initiate printing
    //sprintf(buf, "%c%c%c%c%c%c%c%c", 29, 40, 107, 3, 0, 49, 82, 48);
    //write (sfd,buf,8); usleep(1024*128);

/*
    // QR Code: Select the model
    //              Hex     1D      28      6B      04      00      31      41      n1(x32)     n2(x00) - size of model
    // set n1 [49 x31, model 1] [50 x32, model 2] [51 x33, micro qr code]
    // https://reference.epson-biz.com/modules/ref_escpos/index.php?content_id=140
    byte modelQR [] = {(byte)0x1d, (byte)0x28, (byte)0x6b, (byte)0x04, (byte)0x00, (byte)0x31, (byte)0x41, (byte)0x32, (byte)0x00};
    write (sfd,modelQR,9); usleep(1024*128);

    // QR Code: Set the size of module
    // Hex      1D      28      6B      03      00      31      43      n
    // n depends on the printer
    // https://reference.epson-biz.com/modules/ref_escpos/index.php?content_id=141
    byte sizeQR [] = {(byte)0x1d, (byte)0x28, (byte)0x6b, (byte)0x03, (byte)0x00, (byte)0x31, (byte)0x43, (byte)0x03};
    write (sfd,sizeQR,8); usleep(1024*128);


    //          Hex     1D      28      6B      03      00      31      45      n
    // Set n for error correction [48 x30 -> 7%] [49 x31-> 15%] [50 x32 -> 25%] [51 x33 -> 30%]
    // https://reference.epson-biz.com/modules/ref_escpos/index.php?content_id=142
    byte errorQR [] = {(byte)0x1d, (byte)0x28, (byte)0x6b, (byte)0x03, (byte)0x00, (byte)0x31, (byte)0x45, (byte)0x31};
    write (sfd,errorQR,8); usleep(1024*128);

    // QR Code: Store the data in the symbol storage area
    // Hex      1D      28      6B      pL      pH      31      50      30      d1...dk
    // https://reference.epson-biz.com/modules/ref_escpos/index.php?content_id=143
    //                        1D          28          6B         pL          pH  cn(49->x31) fn(80->x50) m(48->x30) d1…dk
    byte storeQR [] = {(byte)0x1d, (byte)0x28, (byte)0x6b, store_pL, store_pH, (byte)0x31, (byte)0x50, (byte)0x30};
    write (sfd,storeQR,8); usleep(1024*128);

    // QR Code: Print the symbol data in the symbol storage area
    // Hex      1D      28      6B      03      00      31      51      m
    // https://reference.epson-biz.com/modules/ref_escpos/index.php?content_id=144
    byte printQR [] = {(byte)0x1d, (byte)0x28, (byte)0x6b, (byte)0x03, (byte)0x00, (byte)0x31, (byte)0x51, (byte)0x30};
    write (sfd,printQR,8); usleep(1024*128);
***/
    closeSerialPort();

    return true;
}

/********************************************************************
      FUNCTION:         deviceLocal
   DESCRIPTION:         Send the buffer to the local printer
 ********************************************************************/
std::string             cyposrest::deviceLocal (size_t length,
                                                   //unsigned char* buffer,
                                                   std::string receipt,
                                                   std::string type,
                                                   std::string reference)
{
    (void)length;
    //////////////////////////////////////
    //  Print the logo    
    string prefix = _ini->get_value("RECEIPT","TYPE");
    string logoFile = _ini->get_value("PATH","DOCROOT");
    logoFile += "/"; logoFile += prefix; logoFile += "logor.bmp";
    if (_util->file_exists(logoFile.c_str()))  {
        if (prefix != "elena")  {
            deviceImage(logoFile);
        }
    }
    //////////////////////////////////////
    //  Port settings
    size_t st = 0;
    const int baudRate = 19200;
    const string usbport = _ini->get_value("DEVICE","PRINTERPORT");
    //////////////////////////////////////
    //  Open the port and connect
    int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
    if (sfd < 0) {
        _error  = "Unable to connect to serial port: ";
        _error += usbport;
        return _util->json_error(_error);
    }
    //////////////////////////////////////
    //  QR code reference
    char buf[250];
    //////////////////////////////////////
    //  Convert the print buffer
    //std::string receipt (reinterpret_cast<char const*>(buffer), length) ;
    //////////////////////////////////////
    //  reset printer
    sprintf(buf, "%c%c", 27, 64); buf[2]='\0';
    st = write(sfd, buf, 2);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //  Set the left margin
    byte lm[] = { 0x1d, 0x4c, 0x16, 0x00 };
    st = write(sfd, lm, 4);
    if (st > 0)
        usleep(1024*128);
    //////////////////////////////////////
    //  receipt
    //_util->file_create("/usr/local/data/sonny.txt",receipt.c_str());
    _log->logdebug("RECEIPT",receipt.c_str());
    writeSerialData(receipt.c_str (), receipt.size());
    sleep(1);
    closeSerialPort();
    //////////////////////////////////////
    //  Print the QR code / barcode
    string rcpt = _ini->get_value("DEVICE","PRINTERTYPE");
    if (rcpt == "siemens") {
        deviceSiemensImage(reference);
    } else
        deviceQrPrint(reference);
    //////////////////////////////////////
    //  Cut the paper
    deviceCut();
    //////////////////////////////////////
    //  Signature required???
    if (signatureCheck(reference))  {
        //////////////////////////////////////
        //  Open the port and connect
        int sfd = openAndConfigureSerialPort(usbport.c_str (), baudRate);
        if (sfd < 0) {
            _error  = "Unable to connect to serial port: ";
            _error += usbport;
            return _util->json_error(_error);
        }
        //////////////////////////////////////
        //  QR code reference
        char buf[250];
        //////////////////////////////////////
        //  Convert the print buffer
        //std::string receipt (reinterpret_cast<char const*>(buffer), length) ;
        //////////////////////////////////////
        //  reset printer
        sprintf(buf, "%c%c", 27, 64); buf[2]='\0';
        st = write(sfd, buf, 2);
        if (st > 0)
            usleep(1024*128);
        //////////////////////////////////////
        //  Set the left margin
        byte lm[] = { 0x1d, 0x4c, 0x16, 0x00 };
        st = write(sfd, lm, 4);
        if (st > 0)
            usleep(1024*128);
        receipt += _util->cyCenterText(39,"I hereby agree that the amount above");
        receipt += _util->cyCenterText(39,"will be deducted from my account");
        receipt += "\n\n";
        receipt += _util->cyAcrossText(39,"=");
        receipt += _util->cyCenterText(39,"Customer`s signature over printed name\n");
        //////////////////////////////////////
        //  receipt
        //_util->file_create("/usr/local/data/sonny.txt",receipt.c_str());
        _log->logdebug("RECEIPT",receipt.c_str());
        writeSerialData(receipt.c_str (), receipt.size());
        sleep(1);
        closeSerialPort();

        //////////////////////////////////////
        //  Print the QR code / barcode
        string rcpt = _ini->get_value("DEVICE","PRINTERTYPE");
        if (rcpt == "siemens") {
            deviceSiemensImage(reference);
        } else
            deviceQrPrint(reference);
        //////////////////////////////////////
        //  Cut the paper
        deviceCut();
        if (type == "cashpullout")  {
            deviceKick();
        }
    }
    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=receipt;

    stringstream out;
    out << root;
    return out.str ();
}
