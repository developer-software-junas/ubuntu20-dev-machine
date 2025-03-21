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
#include "cyw_aes.h"
#include "cyposrest.h"
#include "restsvc/storesales/cyrestreceipt.h"
#include "restsvc/storesales/cyrestcommon.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         fspaddpoints
   DESCRIPTION:         FSP / beauty addict transaction
 ********************************************************************/
std::string             cyposrest::rest_fspaddpoints ()
{
    //////////////////////////////////////
    //  CGI argument(s)
    string salesType = cgiform("type");
    string systransnum = cgiform("systransnum");
    //////////////////////////////////////
    //  Validate
    if (!_util->valid_generic_field("Transaction number",systransnum,1,64,true,true,"-_"))
        return _util->jsonerror(_util->errormsg());
    if (!_util->valid_number("Sales header type",salesType,1,1,enumHeader::MOBILE,enumHeader::REFUND))
        return _util->jsonerror(_util->errormsg());
    //////////////////////////////////////
    //  Convert the header type
    /*
        typedef enum  {
            MOBILE, SUSPEND, CANCEL, POSTVOID, DAILY, CONSIGN, REPRINT, REFUND
        }  enumHeader;
     */
    salesReceiptType srt = (salesReceiptType)atoi(salesType.c_str ());
    //////////////////////////////////////
    //  Retrieve the sales record
    Json::Value jheader;
    CYRestCommon *rcpt = _ini->receipt_type (_log,_cgi);

    rcpt->resetEodTotals();
    if (!rcpt->retrieveSalesHeader(jheader, systransnum, srt))
        return _util->jsonerror(rcpt->errormsg());

    if (!rcpt->retrieveSalesHeaderAccount(jheader))
        return _util->jsonerror(rcpt->errormsg());

    Json::Value jdetail;
    jdetail.clear();
    if (!rcpt->retrieveSalesDetail(jdetail,
        jheader["branch_code"].asString(), systransnum))
            return _util->jsonerror(rcpt->errormsg());

    jheader["det_sales"]=jdetail;

    Json::Value jpayment;
    jpayment.clear();
    if (!rcpt->retrieveSalesPayment(jpayment, systransnum))
        return _util->jsonerror(rcpt->errormsg());
    jheader["pay_sales"]=jpayment;
    //////////////////////////////////////
    //  Get relevant FSP field values
    if (!rcpt->assembleFspUrl(jheader,jdetail,jpayment))
        return _util->jsonerror(rcpt->errormsg());
    //////////////////////////////////////
    //  FSP url request
    int port = atoi(rcpt->_fspurl.port.c_str ());
    string host  = rcpt->_fspurl.host;
           host += rcpt->_fspurl.path;
    //////////////////////////////////////
    //  HTTPS get request
    struct_cymem mem;
    cymem_init(&mem,1024);
    if (!cymem_https_get(port,18,host.c_str (),rcpt->_fspurl.args.c_str (),&mem))  {
        cymem_clean (&mem);
        return _util->jsonerror("Unable to connect to the FSP server", host);
    }
    //////////////////////////////////////
    //  Build the JSON response
    Json::Value root;
    //////////////////////////////////////
    //  Assemble the JSON response
    root["status"]="ok";
    root["fspresult"]=_util->base64encode(mem.memory);
    cymem_clean (&mem);

    stringstream ss;
    ss << root;
    return ss.str ();
}
