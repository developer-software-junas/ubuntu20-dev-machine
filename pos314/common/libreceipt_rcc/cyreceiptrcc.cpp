//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYReceiptRcc::CYReceiptRcc (cylog* log, cyini* ini, cycgi* cgi)
    : CYRestCommon (log, ini, cgi)
{
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYReceiptRcc::~CYReceiptRcc ()
{
    resetEodTotals();
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Create a receipt
//*******************************************************************
bool                    CYReceiptRcc::assembleReceipt (Json::Value& jheader,
                                                         Json::Value& jdetail,
                                                         Json::Value& jpayment,
                                                         salesReceiptType receiptType)
{
    //////////////////////////////////////
    //  Reset the payment flag(s)
    _isCash = false;
    _isExtra = false;
    _isEpurse = false;
    _isResRma = false;
    _isDrawer = false;
    _isSigLine = false;
    _isSigText = false;
    _isGiftPromo = false;
    _resRmaNumber = "";
    //////////////////////////////////////
    //  Determine the receipt type
    _receiptType = receiptType;
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;
/*
    std::stringstream ss;
    ss << jheader;
    string val = ss.str();
    _log->logmsg("API >> ", val.c_str());
*/
    Json::Value jline;
    _txtdet = "";
    int j = jdetail.size ();
    for (int i = 0; i < j; i++)  {
        jline = jdetail[i];
        Json::Value jhdrtrx = jheader["settings_transtype"];
        if (!assembleReceiptDetail(jline,jhdrtrx,receiptType))
            return false;
        jdetail[i]=jline;
    }

    if (!assembleReceiptTotals(jheader,receiptType))
        return false;

    if (!assembleReceiptPayment(jheader,jpayment,receiptType))
        return false;

    if (!assembleReceiptAccount(jheader,receiptType))
        return false;

    if (!assembleVatBreakdown(receiptType))
        return false;

    if (!assembleReceiptSignature(jheader,receiptType))
        return false;

    if (!assembleReceiptFooter(jheader,receiptType))
        return false;

    if (!assembleReceiptGiftWrap(jheader,jdetail))
        return false;

    if (!assembleReceiptClaimStub(jheader,jdetail))
        return false;

    if (!assembleReceiptPromoItem(jheader,jdetail))
        return false;

    return true;
}

//*******************************************************************
//    FUNCTION:         resetEodTotals
// DESCRIPTION:         Reset xread structure(s)
//*******************************************************************
bool                    CYReceiptRcc::saveReceipt()
{
    return true;
}
//*******************************************************************
//    FUNCTION:         rccDate
// DESCRIPTION:         Receipt date format
//*******************************************************************
string                  CYReceiptRcc::rccDate(string strDate, string strTime)
{
    string              trans_time,
                        trans_date,
                        dd,mm,yyyy;

    trans_date = strDate;
    trans_time = strTime;
    _util->date_split(trans_date.c_str(),yyyy,mm,dd);

    std::stringstream ss;
    ss << mm << "/" << dd << "/" << yyyy << "-" << _util->time_colon(trans_time.c_str()) << std::endl;

    return ss.str ();
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Placeholder for compatability
//*******************************************************************
bool                    CYReceiptRcc::assembleFspUrl(Json::Value& jheader,
                                                       Json::Value& jdetail,
                                                       Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
