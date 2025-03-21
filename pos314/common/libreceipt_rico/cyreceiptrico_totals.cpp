//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrico.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRico::assembleReceiptTotals (Json::Value& jheader,
                                           salesReceiptType receiptType)
{
    receipt_pay_totals(jheader,receiptType);
    return true;
}
