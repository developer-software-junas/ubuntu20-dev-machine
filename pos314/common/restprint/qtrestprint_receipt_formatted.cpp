/********************************************************************
          FILE:         cyposrest_receipt.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         rest_receipt_formatted
   DESCRIPTION:         Print from the database
 ********************************************************************/
std::string             cyposrest::rest_receipt_formatted ()
{
    //////////////////////////////////////
    //  Receipt parameters
    string type = cgiform("type");
    string salesType = cgiform("receipt");
    string systransnum = cgiform("reference");
    string printer_code = cgiform("printer_code");
    //////////////////////////////////////
    //  LOG
    _log->logmsg("QTRESTRECEIPT TYPE:FORMATTED",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",systransnum.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Retrieve the transaction details
    Json::Value jheader;
    CYRestCommon *rcpt = _ini->receipt_type (_log,_cgi);
    int nsalesType = atoi(salesType.c_str ());
    if (type == "refund")
        nsalesType = SRT_REFUND;

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
    string               sql, extrcpt;
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
    //////////////////////////////////////
    //  Get the extended receipt
    db->sql_reset();
    sql  = "select * from tg_pos_receipt where systransnum = ";
    sql += db->sql_bind(1, systransnum); sql += " ";
    sql += " and type = ";
    sql += db->sql_bind(2, rcpt->_txttype); sql += " ";
    if (!db->sql_result(sql,true))
        return _util->jsonerror(db->errordb());
    if (!db->eof())  {
        extrcpt = db->sql_field_value ("txt_extloyalty");
        //_log->logmsg("#####################################",extrcpt.c_str ());
    }
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
    string ts = _util->date_eight();
    ts += _util->time_eight();

    if (rcpt->_txtcopies.length() < 1) rcpt->_txtcopies = "1";
    sql  = "insert into tg_pos_receipt (systransnum, type, txt_header, ";
    sql += "txt_detail, txt_totals, txt_payment, txt_vat, txt_signature, ";
    sql += "txt_footer, txt_giftwrap, txt_extloyalty, txt_extbeauty, ";
    sql += "txt_extpromo, copies, cashier, shift, logical_date, register_num, ";
    sql += "timestamp, location_code, printer_name, is_polled2) values (";
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
    sql += db->sql_bind(11, rcpt->_txtloyalty); sql += ", ";
    sql += db->sql_bind(12, rcpt->_txtbeauty); sql += ", ";
    sql += db->sql_bind(13, rcpt->_txtpromoitem); sql += ", ";
    sql += db->sql_bind(14, rcpt->_txtcopies); sql += ", ";
    sql += db->sql_bind(15, cashier); sql += ", ";
    sql += db->sql_bind(16, shift); sql += ", ";
    sql += db->sql_bind(17, logical_date); sql += ", ";
    sql += db->sql_bind(18, register_num); sql += ", ";
    sql += db->sql_bind(19, ts); sql += ", ";
    sql += db->sql_bind(20, location_code); sql += ", 'default', 0)";
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());
    //////////////////////////////////////
    //  Prepare the print data
    string strPrintData = "";

    strPrintData += rcpt->_txthdr;
    strPrintData += rcpt->_txtdet;
    strPrintData += rcpt->_txttot;
    strPrintData += rcpt->_txtpay;
    strPrintData += rcpt->_txtacct;
    strPrintData += rcpt->_txtvat;
    strPrintData += rcpt->_txtsig;

    if (!updateJournal(type,ts,systransnum,strPrintData))
        return _util->jsonerror(_error);

    string ret = "";
    if (printer_code != "DEFAULT")  {
        //////////////////////////////////////
        //  Print the logo
        string prefix = _ini->get_value("RECEIPT","TYPE");
        string logoFile = _ini->get_value("PATH","DOCROOT");
        logoFile += "/"; logoFile += prefix; logoFile += "logor.bmp";
        if (_util->file_exists(logoFile.c_str()))  {
            deviceNetworkImage(logoFile,printer_code);
        }
        /////////////////////////////////////
        //  Signature check
        string picSig = _ini->get_value("PATH","SIGNATURE");
        picSig += jheader["systransnum"].asString();
        picSig += ".png";
        if (_util->file_exists(picSig.c_str ()))  {
            string part1 = strPrintData;
            deviceNetworkText (part1,printer_code);
            deviceNetworkImage(picSig,printer_code);
            part1  = "========================================\n";
            part1 += "            CUSTOMER SIGNATURE\n\n";

            for (int i = 0; i < printCopy; i++)  {
                deviceNetworkText (part1,printer_code);
                ret = deviceNetwork (rcpt->_txtfoot,systransnum,printer_code);
            }
        }  else  {
            strPrintData += rcpt->_txtfoot;
            for (int i = 0; i < printCopy; i++)
                ret = deviceNetwork (strPrintData,systransnum,printer_code);
        }
    }  else  {
        //strPrintData += _byteLocalImage->toStdString();
        strPrintData += rcpt->_txtfoot;
        for (int i = 0; i < printCopy; i++)
            ret = deviceLocal (strPrintData.length(), strPrintData,
                         //(unsigned char*)strPrintData.c_str (),
                         type,systransnum);
    }
    return ret;
}

/********************************************************************
      FUNCTION:         rest_receipt_formatted
   DESCRIPTION:         Print from the database
 ********************************************************************/
/*std::string             cyposrest::rest_receipt_formatted ()
{
    //////////////////////////////////////
    //  Receipt parameters    
    string type = cgiform("type");
    string salesType = cgiform("receipt");
    string systransnum = cgiform("reference");
    string printer_code = cgiform("printer_code");
    //////////////////////////////////////
    //  LOG
    _log->logmsg("QTRESTRECEIPT TYPE:FORMATTED",type.c_str ());
    _log->logmsg("QTRESTRECEIPT REF",systransnum.c_str ());
    _log->logmsg("QTRESTRECEIPT PRINTER",printer_code.c_str ());
    //////////////////////////////////////
    //  Retrieve the transaction details
    Json::Value jheader;
    CYRestCommon* rcpt = receiptType();
    int nsalesType = atoi(salesType.c_str ());
    if (type == "refund")
        nsalesType = SRT_REFUND;

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
    CYDbSql*             db = _ini->dbconn ();
    if (nullptr == db)
        return _util->jsonerror(_ini->errormsg());
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
    string ts = _util->date_eight();
           ts += _util->time_eight();

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
    if (!db->sql_only(sql,true))
        return _util->jsonerror(db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(db->errordb());
    //////////////////////////////////////
    //  GIFT PROMO ITEM - APPEND TO GIFT TEXT
    rcpt->_txtgift += rcpt->_txtpromoitem;
    //////////////////////////////////////
    //  Prepare the network XML print data
    string strPrintData = "";
    const string  PRINTER_CUT = "</text>\n<cut />\n<text>\n";
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
    strPrintData += "</text>";
    //////////////////////////////////////
    //  QR code
    strPrintData += "<text align=\"center\" />\r\n";
    strPrintData += "<symbol width=\"6\" type=\"qrcode_model_2\"  level=\"level_m\">";
    if (rcpt->_resRmaNumber.length() > 0)  {
        strPrintData += rcpt->_resRmaNumber;
    }  else  {
        strPrintData += systransnum;
    }
    strPrintData += "</symbol>";

    //strPrintData += "<text>";
    strPrintData += "<text align=\"left\">\r\n";
    //////////////////////////////////////
    // REPLACE CODE (-+-+-+-) WITH AUTCOCUT FOOTER
    if (strlen(rcpt->_txtfoot.c_str()) > 0){
        size_t f;
        bool end = false;
        string value = rcpt->_txtfoot.c_str();

        while (!end) {
            f = value.find(RCPT_CUT);
            if (f != std::string::npos) {
                value.replace(f,39,PRINTER_CUT);
            } else {
                end = true;
            }
        }
        strPrintData += value.c_str();
    }

    //strPrintData += rcpt->_txtgift;
    //////////////////////////////////////
    // REPLACE CODE (-+-+-+-) WITH AUTCOCUT GFT CERT
    if (strlen(rcpt->_txtgift.c_str()) > 0){
        size_t f;
        bool end = false;
        string value = rcpt->_txtgift.c_str();

        while (!end) {
            f = value.find(RCPT_CUT);
            if (f != std::string::npos) {
                value.replace(f,39,PRINTER_CUT);
            } else {
                end = true;
            }
        }
        strPrintData += value.c_str();
    }

    strPrintData += "</text>";

    /////////////////////////////////
    //EPURSE SECOND COPY
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

            f = value.find(RCPT_CUT);
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

        strPrintData += "</text>";
    }
    //////////////////////////////////////
    //  Signature second copy - FMS
    if (rcpt->_isSigLine)  {

        strPrintData += "<text align=\"left\" />\r\n";
        strPrintData += "<text>";
        strPrintData += rcpt->_txthdr;
        strPrintData += rcpt->_txtdet;
        strPrintData += rcpt->_txttot;
        strPrintData += rcpt->_txtpay;
        strPrintData += rcpt->_txtacct;
        strPrintData += rcpt->_txtvat;
        strPrintData += rcpt->_txtsig;
        strPrintData += rcpt->_txtfoot;

        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, " ");
        if (rcpt->_isSigText)  {
            strPrintData += _util->cyCenterText(42, "I promise to pay such total (plus charges)");
            strPrintData += _util->cyCenterText(42, "per agreemnent as to the use of such card.");
            strPrintData += _util->cyCenterText(42, "In case of litigation, I agree to pay");
            strPrintData += _util->cyCenterText(42, "3% monthly interest and penalty and");
            strPrintData += _util->cyCenterText(42, "25% attorney`s fee.");
            strPrintData += _util->cyCenterText(42, " ");
        }
        strPrintData += _util->cyCenterText(40, "_________________________________");
        strPrintData += _util->cyCenterText(40, "Cardholder's Sign over print name");
        strPrintData += _util->cyCenterText(40, " ");
        strPrintData += _util->cyCenterText(40, "FMS COPY");

        strPrintData += "</text>\n\n<cut />";
    }
    if ((salesReceiptType) nsalesType != SRT_REFUND)
        strPrintData += "<cut />\n";

    if (rcpt->_isDrawer &&
        (salesReceiptType) nsalesType != SRT_REPRINT) {                
        strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_100\" />\r\n";
        strPrintData += "<pulse drawer=\"drawer_1\" time=\"pulse_200\" />\r\n";
    }
    strPrintData += "</epos-print>\r\n";
    strPrintData += "</s:Body>\r\n";
    strPrintData += "</s:Envelope>";
    //////////////////////////////////////
    //  Select the printer
    string strPrinterURL = printer_code;
    strPrinterURL  = _ini->get_value("DEVICE","PRINTERURL");
    _log->logmsg("QTRESTRECEIPT URL",strPrinterURL.c_str ());
    _log->logmsg("QTRESTRECEIPT POSTPRINTER",strPrintData.c_str ());
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
    if (!updateJournal(type,ts,systransnum,strPrintData))
        return _util->jsonerror(_error);
    //////////////////////////////////////
    //  Assemble the JSON response
    Json::Value root;
    root["status"]="ok";
    root["receipt"]=rcpt->_receipt;

    stringstream out;
    out << root;
    return out.str ();
}
*/
