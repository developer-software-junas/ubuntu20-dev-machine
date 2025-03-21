//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptSunmi::CYRestReceiptSunmi (cycgi* cgi) : CYRestCommon (cgi)
{
}
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptSunmi::CYRestReceiptSunmi (cylog* log, cyini* ini, cycgi* cgi)
    : CYRestCommon (log, ini, cgi)
{
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYRestReceiptSunmi::~CYRestReceiptSunmi ()
{
    resetEodTotals();
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Create a receipt
//*******************************************************************
bool                    CYRestReceiptSunmi::assembleReceipt (Json::Value& jheader,
                                                        Json::Value& jdetail,
                                                        Json::Value& jpayment,
                                                        salesReceiptType receiptType)
{
    _receiptType = receiptType;
    _totalServiceCharge = 0.00;
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
bool                    CYRestReceiptSunmi::saveReceipt()
{
    return true;
}
//*******************************************************************
//    FUNCTION:         assembleReceipt
// DESCRIPTION:         Placeholder for compatability
//*******************************************************************
bool                    CYRestReceiptSunmi::assembleFspUrl(Json::Value& jheader,
                                                       Json::Value& jdetail,
                                                       Json::Value& jpayment)
{
    (void)jheader; (void)jdetail; (void)jpayment;
    return true;
}
