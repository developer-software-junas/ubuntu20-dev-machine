//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptelena.h"
using std::string;
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYReceiptElena::CYReceiptElena (cylog* log, cyini* ini, cycgi* cgi)
    : CYRestCommon (log, ini, cgi)
{
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYReceiptElena::~CYReceiptElena ()
{
    resetEodTotals();
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Create a receipt
//*******************************************************************
bool                    CYReceiptElena::assembleReceipt (Json::Value& jheader,
                                                         Json::Value& jdetail,
                                                         Json::Value& jpayment,
                                                         salesReceiptType receiptType)
{
    //////////////////////////////////////
    //  Regular or terminal receipt
    _isSkinny = false;
    if (cgiform ("receipt_skinny")=="true")
        _isSkinny = true;
    //////////////////////////////////////
    if (!assembleReceiptHeader(jheader,receiptType))
        return false;

    Json::Value jline;
    _txtdet = "";
    _splitSeq = "-999.99";
    int j = jdetail.size ();
    for (int i = 0; i < j; i++)  {
        jline = jdetail[i];
        Json::Value jhdrtrx = jheader["settings_transtype"];
        if (!assembleReceiptDetail(jline,jhdrtrx,receiptType))
            return false;
        jdetail[i]=jline;
    }

    if (!assembleReceiptTotals(jheader, receiptType))
        return false;

    if (!assembleReceiptPayment(jheader,jpayment,receiptType))
        return false;

    if (!assembleReceiptAccount(jheader))
        return false;

    if (!assembleVatBreakdown(receiptType))
        return false;

    if (!assembleReceiptSignature(jheader))
        return false;

    if (!assembleReceiptFooter(jheader,receiptType))
        return false;

    return true;
}

//*******************************************************************
//    FUNCTION:         resetEodTotals
// DESCRIPTION:         Reset xread structure(s)
//*******************************************************************
bool                    CYReceiptElena::saveReceipt()
{
    return true;
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Placeholder for compatability
//*******************************************************************
bool                    CYReceiptElena::assembleFspUrl(Json::Value& jheader,
                                                       Json::Value& jdetail,
                                                       Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
