//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptjoel.h"
using std::string;
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYReceiptJoel::CYReceiptJoel (cylog* log, cyini* ini, cycgi* cgi)
    : CYRestCommon (log, ini, cgi)
{
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYReceiptJoel::~CYReceiptJoel ()
{
    resetEodTotals();
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Create a receipt
//*******************************************************************
bool                    CYReceiptJoel::assembleReceipt (Json::Value& jheader,
                                                        Json::Value& jdetail,
                                                        Json::Value& jpayment,
                                                        salesReceiptType receiptType)
{
    _receiptType = receiptType;
    _totalServiceCharge = 0.00;
    //////////////////////////////////////
    //  Internal transaction type?
    Json::Value jtrx;
    _isInternal = false;
    jtrx = jheader["settings_transtype"];
    _isInternal = (jtrx["is_internal"].asString() == "1");
    //////////////////////////////////////
    //  NO VAT if not discounted?
    Json::Value jhdrset = jheader["hdr_settings"];
    string ps_nodiscountvatexempt = jhdrset["nodiscountvatexempt"].asString();
    _noVatNotDiscounted = ps_nodiscountvatexempt == "1";
    //////////////////////////////////////
    //  Receipt header portion
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;

    if (!assembleReceiptAccount(jheader))
        return false;

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

    if (receiptType != salesReceiptType::SRT_SUSPEND)  {

        if (!assembleVatBreakdown(receiptType))
            return false;

        if (!assembleReceiptSignature(jheader))
            return false;

        if (!assembleReceiptFooter(jheader,receiptType))
            return false;
    }


    return true;
}

//*******************************************************************
//    FUNCTION:         resetEodTotals
// DESCRIPTION:         Reset xread structure(s)
//*******************************************************************
bool                    CYReceiptJoel::saveReceipt()
{
    return true;
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Placeholder for compatability
//*******************************************************************
bool                    CYReceiptJoel::assembleFspUrl(Json::Value& jheader,
                                                       Json::Value& jdetail,
                                                       Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
