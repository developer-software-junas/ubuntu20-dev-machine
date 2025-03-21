//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunnies.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptSunnies::assembleVatBreakdown (salesReceiptType receiptType)
{
    string nullString = "";
    receipt_vat_breakdown(receiptType, false, nullString);
    return true;
}
