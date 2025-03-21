/********************************************************************
          FILE:         cyposrest_receipt_kitchen.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  QT header(s)
#include <QUrl>
#include <QEventLoop>
#include <QTcpSocket>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
//////////////////////////////////////////
//  REST class

#include <string>
#include <sstream>
#include "cyposrest.h"
#include "pystring.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         rest_receipt_kitchen
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             cyposrest::rest_receipt_kitchen ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string reference = cgiform("reference");
    string table_code = cgiform("register_num");
    string printer_code = cgiform("printer_code");
    string reference_count = cgiform("reference_count");

    _log->logmsg("QTRESTKITCHEN TYPE",type.c_str ());
    _log->logmsg("QTRESTKITCHEN REF",reference.c_str ());
    _log->logmsg("QTRESTKITCHEN PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Decode the receipt text
    string receipt = cgiform("receipt");
    receipt = _util->base64decode(receipt);
    _log->logmsg(receipt.c_str (),"......");

    //////////////////////////////////////
    //  Generate a reference number
    string strPrinterURL = "";
    string printNum = _util->strtime_t();
    printNum += _util->random_password(5);
    //////////////////////////////////////
    //  Select the printer
    struct_cymem mem;
    cymem_init (&mem,1024);
    if (reference_count == "1414")  {
        cymem_append(&mem,"ok");
        //////////////////////////////////////
        //  Save the file
        string filename  = _ini->get_value("PATH","RECEIPT");
        filename += "/";
        filename += printNum;
        filename += "-";
        filename += "kitchen.";
        filename += printer_code;
        filename += ".xml";
        _util->file_create(filename.c_str (),receipt);
    }  else  {
        db->sql_reset ();
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
        string copies = db->sql_field_value("printcopy");
        strPrinterURL = db->sql_field_value("printer_url");
        string strPrinterName = db->sql_field_value("printer_name");
        int copy;
        if (copies.empty())  {
            copy = 1;
        }  else  {
            copy = atoi(copies.c_str());
            if (copy == 0) copy = 1;
        }
        //////////////////////////////////////
        //  Override???
        string override_url = "";
        if (deviceOverride(override_url))
            strPrinterURL = override_url;
        //////////////////////////////////////
        //  Send the buffer
        string rcpt1 = "", rcpt2 = "";
        std::vector<string> splitReceipt;
        pystring::split(receipt, splitReceipt, "^");
        if (splitReceipt.size() >=2)  {
            rcpt1 = splitReceipt.at(0);
            rcpt2 = splitReceipt.at(1);
        }  else  {
            rcpt1 = receipt;
        }

        string copyKitchen;
        QTcpSocket* _socket = new QTcpSocket();
        _socket->connectToHost(strPrinterURL.c_str(), 9100);

        if(_socket->waitForConnected(3000))  {
            qDebug() << "Connected!";
            //////////////////////////////////////
            //  Reset the printer
            qint64 retcode;
            QByteArray byteInit;
            byteInit.clear();
            //  Printer reset
            byteInit.append(char(27));
            byteInit.append(char(64));
            for (int x = 0; x < copy; x++)  {
                //  Left margin
                byteInit.append(char(29));
                byteInit.append(char(76));
                byteInit.append(char(22));
                byteInit.append(char(0));
                //  Initialize
                retcode = _socket->write(byteInit,byteInit.length());
                if (retcode == -1)  {
                    return _util->jsonerror("KITCHEN", "Unable to print");
                }
                _socket->waitForBytesWritten(1000);
                _socket->waitForReadyRead(1000);
                qDebug() << "Reading: " << _socket->bytesAvailable();
                qDebug() << _socket->readAll();
                //  Print some stuff
                retcode = _socket->write(rcpt1.c_str(),rcpt1.length());
                if (retcode == -1)  {
                    return _util->jsonerror("KITCHEN", "Unable to print");
                }
                _socket->waitForBytesWritten(1000);
                _socket->waitForReadyRead(1000);
                qDebug() << "Reading: " << _socket->bytesAvailable();
                qDebug() << _socket->readAll();
                //////////////////////////////////////
                //  Big font
                QByteArray byteBig;
                byteBig.clear();
                byteBig.append(char(27));
                byteBig.append(char(33));
                byteBig.append(char(16));
                retcode = _socket->write(byteBig,byteBig.length());
                if (retcode == -1)  {
                    return _util->jsonerror("KITCHEN", "Unable to print");
                }
                _socket->waitForBytesWritten(1000);
                _socket->waitForReadyRead(1000);
                qDebug() << "Reading: " << _socket->bytesAvailable();
                qDebug() << _socket->readAll();
                ///////////////////////////////
                //  Copy for Ricos
                string rcptType = _ini->get_value("RECEIPT","TYPE");
                if (rcptType == "rico")  {
                    copyKitchen = rcpt2;
                    if (0 == x)  {
                        copyKitchen += _util->cyCenterText(40,"*** DISPATCH COPY ***");
                    }  else if (1 == x)  {
                        copyKitchen += _util->cyCenterText(40,"*** KITCHEN COPY ***");
                    }  else  {
                        copyKitchen += _util->cyCenterText(40,"*** COPY ***");
                    }
                    retcode = _socket->write(copyKitchen.c_str(),copyKitchen.length());
                    if (retcode == -1)  {
                        return _util->jsonerror("KITCHEN", "Unable to print");
                    }
                    copyKitchen = "";
                }  else  {
                    _socket->write(rcpt2.c_str(),rcpt2.length());
                }
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
                retcode = _socket->write(byteDone, byteDone.length());
                if (retcode == -1)  {
                    return _util->jsonerror("KITCHEN", "Unable to print");
                }
                _socket->waitForBytesWritten(1000);
                _socket->waitForReadyRead(1000);
                qDebug() << "Reading: " << _socket->bytesAvailable();
                qDebug() << _socket->readAll();
            }
            _socket->close();
        }  else  {
            qDebug() << "Not connected!";
            return _util->jsonerror("KITCHEN","Unable to connect to the printer");
        }
    }
    //////////////////////////////////////
    //  Update the print count
    if (strlen(mem.memory) > 0)  {
        std::vector<std::string> qtyList;
        std::vector<std::string> orderList;
        pystring::split (reference,orderList,",");
        pystring::split (reference_count,qtyList,",");
        int j = orderList.size();
        for (int i = 0; i < j; i++)  {
            string quantity = qtyList.at(i);
            string orderNumber = orderList.at(i);

            db->sql_reset ();
            sql  = "update tg_table_customer_orders ";
            sql += "set    print_count = print_count + ";
            sql += quantity;
            sql += ", printer_code = ";
            sql += db->sql_bind(1,printer_code);
            sql += ", print_reference = ";
            sql += db->sql_bind(2,printNum);
            sql += " where table_code = ";
            sql += db->sql_bind(3,table_code);
            sql += " and   order_number = ";
            sql += db->sql_bind(4,orderNumber);

            if (!db->sql_only(sql,true))
                return _util->jsonerror(db->errordb());
        }
        _log->logmsg("KITCHEN","SQL OK");

        db->sql_reset ();
        sql  = "update tg_table_customer_orders ";
        sql += "set    print_count = quantity ";
        sql += " where table_code = ";
        sql += db->sql_bind(1,table_code);
        sql += " and   order_number in (";
        sql += reference;
        sql += ") and  print_count > quantity ";

        if (!db->sql_only(sql,true))
            return _util->jsonerror(db->errordb());

        if (!db->sql_commit ())  {
            _log->logmsg("KITCHEN",db->errordb().c_str ());
            return _util->jsonerror("KITCHEN",db->errordb());
        }
        _log->logmsg("KITCHEN","COMMIT OK");
    }
    receipt = mem.memory;
    cymem_clean(&mem);
    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=receipt;

    stringstream out;
    out << root;
    _log->logmsg("KITCHEN",out.str().c_str ());
    return out.str ();
}
