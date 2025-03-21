//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceipteast.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptEast::assembleReceiptTotals (Json::Value& jheader,
                                           salesReceiptType receiptType)
{
    receipt_pay_totals(jheader,receiptType);
    return true;
}
