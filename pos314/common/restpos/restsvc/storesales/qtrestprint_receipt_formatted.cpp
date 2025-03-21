/********************************************************************
          FILE:         qtrestprint_receipt.cpp
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
#include <iomanip>
#include <string>
#include <sstream>
#include "cyw_aes.h"
#include "qtrestprint.h"
#include "restsvc/storesales/cyrestreceipt.h"
using std::string;
using std::stringstream;
#define DELIM  "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+"
/********************************************************************
      FUNCTION:         rest_receipt_formatted
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             qtrestprint::rest_receipt_formatted ()
{
    const string  RCPT_IMG = "";
    const string  RCPT_CUT = "</text>\n<cut />\n<text>\n";
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string salesType = cgiform("receipt");
    string systransnum = cgiform("reference");
    string printer_code = cgiform("printer_code");
    //  LOG
    _log->logmsg("QTRESTRECEIPT TYPE",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",systransnum.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Retrieve the transaction details
    Json::Value jheader;
    CYRestCommon* rcpt = receiptType();
    int nsalesType = atoi(salesType.c_str ());

    rcpt->resetEodTotals();
    if (!rcpt->retrieveSalesHeader(jheader, systransnum, (salesReceiptType) nsalesType))
        return _util->jsonerror(rcpt->errormsg());
    //  Header details
    string cashier = jheader["cashier"].asString ();
    string shift = jheader["cashier_shift"].asString ();
    string logical_date = jheader["logical_date"].asString ();
    string location_code = jheader["branch_code"].asString ();
    string register_num = jheader["register_number"].asString ();

    if (!rcpt->retrieveSalesHeaderAccount(jheader))
        return _util->jsonerror(rcpt->errormsg());

    Json::Value jdetail;
    jdetail.clear();
    if (!rcpt->retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
        return _util->jsonerror(rcpt->errormsg());
    jheader["det_sales"]=jdetail;

    Json::Value jpayment;
    jpayment.clear();
    if (!rcpt->retrieveSalesPayment(jpayment, systransnum))
        return _util->jsonerror(rcpt->errormsg());
    jheader["pay_sales"]=jpayment;

    if (!rcpt->assembleFspUrl(jheader, jdetail, jpayment))
        return _util->jsonerror(rcpt->errormsg());
    //////////////////////////////////////
    //  Create the receipt
    rcpt->_txttype = "print";
    if (!rcpt->assembleReceipt(jheader, jdetail, jpayment, (salesReceiptType) nsalesType))
        return _util->jsonerror((rcpt->errormsg()));
    //////////////////////////////////////
    //  Database connection
    string               sql;
    CYDbSql*             db = dbconn ();
    if (!db->ok())
        return _util->jsonerror(_env->errordbenv());

    //////////////////////////////////////
    //  RETRIEVE EXT_RCPT
    string txt_loyalty,txt_fspextend,txt_baextend;
    db->sql_reset();
    sql  = "select txt_extloyalty from tg_pos_receipt where systransnum = ";
    sql += db->sql_bind(1, systransnum);
    if (!db->sql_result(sql))
        return _util->jsonerror(db->errordb());
    if (!db->eof())
        txt_loyalty = db->sql_field_value("txt_extloyalty");
    _log->logmsg("DEBUG RECEIPT FORMAT",txt_loyalty.c_str());

    if (strlen(txt_loyalty.c_str()) > 0){
        size_t f;
        bool end = false;
        string del = "fsprafflegwp";
        string value = txt_loyalty.c_str();

        while (!end) {
            f = value.find(del.c_str());
            if (f != std::string::npos) {
                txt_baextend= value.substr(0,f);
                txt_baextend += RCPT_CUT;
                txt_fspextend = value.substr(f+12,value.size());
                txt_fspextend += RCPT_CUT;
                end = true;
            } else {
                txt_fspextend = txt_loyalty;
                txt_fspextend += RCPT_CUT;
                txt_baextend = "";
                end = true;
            }
        }
    }

    rcpt->_txtgift += rcpt->_txtpromoitem;

    //////////////////////////////////////
    //  Save to receipt journal
    db->sql_reset();
    sql  = "delete from tg_pos_receipt where systransnum = ";
    sql += db->sql_bind(1, systransnum); sql += " ";
    sql += " and type = ";
    sql += db->sql_bind(2, rcpt->_txttype); sql += " ";
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    db->sql_bind_reset();
    string ts = _util->strtime_t();
    if (rcpt->_txtcopies.length() < 1) rcpt->_txtcopies = "1";
    sql  = "insert into tg_pos_receipt (systransnum, type, txt_header, ";
    sql += "txt_detail, txt_totals, txt_payment, txt_vat, txt_signature, ";
    sql += "txt_footer, txt_giftwrap, copies, cashier, shift, logical_date, ";
    sql += "register_num, timestamp, location_code, printer_name, is_polled2) values (";
    sql += db->sql_bind(1,  systransnum); sql += ", ";
    sql += db->sql_bind(2,  rcpt->_txttype); sql += ", ";
    sql += db->sql_bind(3,  rcpt->_txthdr); sql += ", ";
    sql += db->sql_bind(4,  rcpt->_txtdet); sql += ", ";
    sql += db->sql_bind(5,  rcpt->_txttot); sql += ", ";
    sql += db->sql_bind(6,  rcpt->_txtpay); sql += ", ";
    sql += db->sql_bind(7,  rcpt->_txtvat); sql += ", ";
    sql += db->sql_bind(8,  rcpt->_txtsig); sql += ", ";
    sql += db->sql_bind(9,  rcpt->_txtfoot); sql += ", ";
    sql += db->sql_bind(10, rcpt->_txtgift); sql += ", ";
    sql += db->sql_bind(11, rcpt->_txtcopies); sql += ", ";
    sql += db->sql_bind(12, cashier); sql += ", ";
    sql += db->sql_bind(13, shift); sql += ", ";
    sql += db->sql_bind(14, logical_date); sql += ", ";
    sql += db->sql_bind(15, register_num); sql += ", ";
    sql += db->sql_bind(16, ts); sql += ", ";
    sql += db->sql_bind(17, location_code); sql += ", 'default', 0)";

    /*//FOR FUTURE USE
    sql  = "update tg_pos_receipt set ";
    sql += "systransnum="; sql += db->sql_bind(1,  systransnum); sql += ", ";
    sql += "type="; sql += db->sql_bind(2,  rcpt->_txttype); sql += ", ";
    sql += "txt_header="; sql += db->sql_bind(3,  rcpt->_txthdr); sql += ", ";
    sql += "txt_detail="; sql += db->sql_bind(4,  rcpt->_txtdet); sql += ", ";
    sql += "txt_totals="; sql += db->sql_bind(5,  rcpt->_txttot); sql += ", ";
    sql += "txt_payment="; sql += db->sql_bind(6,  rcpt->_txtpay); sql += ", ";
    sql += "txt_vat="; sql += db->sql_bind(7,  rcpt->_txtvat); sql += ", ";
    sql += "txt_signature="; sql += db->sql_bind(8,  rcpt->_txtsig); sql += ", ";
    sql += "txt_footer="; sql += db->sql_bind(9,  rcpt->_txtfoot); sql += ", ";
    sql += "txt_giftwrap="; sql += db->sql_bind(10, rcpt->_txtgift); sql += ", ";
    sql += "copies="; sql += db->sql_bind(11, rcpt->_txtcopies); sql += " ";
    sql += " where systransnum = "; sql += db->sql_bind(12, systransnum); sql += ";";*/

    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());

    //FIRST COPY
    //////////////////////////////////////
    //  Prepare the network XML print data
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

    strPrintData += "<text>";
    strPrintData += rcpt->_txthdr;
    strPrintData += rcpt->_txtdet;
    strPrintData += rcpt->_txttot;
    strPrintData += rcpt->_txtpay;
    strPrintData += rcpt->_txtacct;
    strPrintData += rcpt->_txtvat;
    strPrintData += rcpt->_txtsig;
    /////////////////////////////////
    if (strlen(rcpt->_txtfoot.c_str()) > 0){
        size_t f;
        bool end = false;
        string value = rcpt->_txtfoot.c_str();
        //INSERT QR CODE INTO RECEIPT
        //string pQrcode = "";
        //pQrcode  = "</text>\n";
        //pQrcode += "<symbol width=\"10\" type=\"qrcode_model_2\"  align=\"center\" level=\"level_m\">";
        //pQrcode += systransnum;
        //pQrcode += "</symbol>\n<text>\n";
        //
        //f = value.find("--+++--");
        //if (f != std::string::npos)
        //    value.replace(f,7,pQrcode.c_str());

        //RUN THROUGH TEXT IF AUTO CUT FOOTER
        while (!end) {
            f = value.find(DELIM);
            if (f != std::string::npos) {
                value.replace(f,39,RCPT_CUT);
            } else {
                end = true;
            }
        }
        //IF THERES A BA ENTENDED RECEIPT
        if (strlen(txt_baextend.c_str()) > 0){
            f = value.find("============= FSP DETAILS");
            if (f != std::string::npos)
                value.insert(f,txt_baextend.c_str());
            else
                value += txt_baextend.c_str();
        }
        //CLEAN DATA "|" "~"
        end = false;
        while (!end) {
            f = value.find("|");
            if (f != std::string::npos) {
                value.replace(f,1,"");
            } else {
                end = true;
            }
        }
        end = false;
        while (!end) {
            f = value.find("~");
            if (f != std::string::npos) {
                value.replace(f,1,"");
            } else {
                end = true;
            }
        }

        strPrintData += value.c_str();
    }
    if (strlen(txt_fspextend.c_str()) > 0){
        strPrintData += txt_fspextend.c_str();
    }
    /////////////////////////////////
    //RUN THROUGH TEXT IF AUOT CUT GIFT RECEIPT
    if (strlen(rcpt->_txtgift.c_str()) > 0){
        size_t f;
        bool end = false;
        string value = rcpt->_txtgift.c_str();

        while (!end) {
            f = value.find(DELIM);
            if (f != std::string::npos) {
                value.replace(f,39,RCPT_CUT);
            } else {
                end = true;
            }
        }

        strPrintData += value.c_str();
    }

    strPrintData += "</text>";

    ///epurse second copy
    if (rcpt->_isEpurse) {
        strPrintData += "<text>";
        strPrintData += rcpt->_txthdr;
        strPrintData += rcpt->_txtdet;
        strPrintData += rcpt->_txttot;
        strPrintData += rcpt->_txtpay;
        strPrintData += rcpt->_txtacct;
        strPrintData += rcpt->_txtvat;
        strPrintData += rcpt->_txtsig;

        /////////////////////////////////
        //RUN THROUGH TEXT IF AUOT CUT GIFT RECEIPT
        if (strlen(rcpt->_txtfoot.c_str()) > 0){
            size_t f;
            string value = rcpt->_txtfoot.c_str();

            f = value.find(DELIM);
            if (f != std::string::npos) {
                value = value.substr(0,f);
            } else
                value = rcpt->_txtfoot.c_str();

            strPrintData += value.c_str();
        }


        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, "I hereby agree that the amount above");
        strPrintData += _util->cyCenterText(40, "will be deducted from my account");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, "_________________________________");
        strPrintData += _util->cyCenterText(40, "Cardholder's Sign over print name");
        strPrintData += _util->cyCenterText(40, "_");
        strPrintData += _util->cyCenterText(40, "FMS COPY");
        strPrintData += RCPT_CUT;


        strPrintData += "</text>";
    }

    //strPrintData += "<text align=\"center\" />\r\n";
    //strPrintData += "<symbol width=\"6\" type=\"qrcode_model_2\"  level=\"level_m\">";
    //strPrintData += systransnum;
    //strPrintData += "</symbol>";
    //if(nKick == 1 ){
    //strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_200\" />\r\n";
    //}
    //strPrintData += "<cut />\r\n";
    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";
    //////////////////////////////////////
    //  Select the printer
    string strPrinterURL = printer_code;
    _log->logmsg("QTRESTRECEIPT URL",strPrinterURL.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINT-DATA",strPrintData.c_str ());
    //////////////////////////////////////
    //  Send to the network printer
    struct_cymem mem;
    cymem_init(&mem,1024);
    //////////////////////////////////////
    //  PRINT
    cymem_https_post(80,18,strPrinterURL.c_str (),strPrintData.c_str (),&mem);

    size_t st = mem.size;
    _log->logmsg("QTRESTRECEIPT HTTPRESULT",mem.memory);
    cymem_clean(&mem);

    if (st < 1)  {
        return _util->jsonerror("Unable to print");
    }


    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=rcpt->_receipt;

    stringstream out;
    out << root;
    return out.str ();

}
