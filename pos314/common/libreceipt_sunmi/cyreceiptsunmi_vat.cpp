//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYRestReceiptSunmi::assembleVatBreakdown (salesReceiptType receiptType)
{
    _txtvat = "";
    int w = 30;

    if (receiptType == SRT_SUSPEND)
        return 0;

    _txtvat += "VAT Breakdown:";
    _txtvat += "\n";
    _txtvat += _util->cyPostSpace(10, "VAT");
    _txtvat += _util->cyPreSpace(10, "Net Sale");
    _txtvat += _util->cyPreSpace(10, "Amount");
    _txtvat += "\n";

    //////////////////////////////////////
    //  VAT amounts
    char szNetVat[32]; char szNetZero[32]; char szNetExempt[32];
    char szAmtVat[32]; char szAmtZero[32]; char szAmtExempt[32];

    if (receiptType == SRT_REFUND)  {
        if (_totalNetVat != 0)
            sprintf (szNetVat, "%s", FMTNumberComma((_totalNetVat * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetVat, "%s", FMTNumberComma((_totalNetVat * 1000) / 1000).c_str());
        if (_totalNetZero != 0)
            sprintf (szNetZero, "%s", FMTNumberComma((_totalNetZero * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetZero, "%s", FMTNumberComma((_totalNetZero * 1000) / 1000).c_str());
        if (_totalNetExempt != 0)
            sprintf (szNetExempt, "%s", FMTNumberComma((_totalNetExempt * 1000) / 1000 * -1).c_str());
        else
            sprintf (szNetExempt, "%s", FMTNumberComma((_totalNetExempt * 1000) / 1000).c_str());

        if (_totalAmtVat != 0)
            sprintf (szAmtVat, "%s", FMTNumberComma((_totalAmtVat * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtVat,  "%s", FMTNumberComma((_totalAmtVat * 1000) / 1000).c_str());
        if (_totalAmtZero != 0)
            sprintf (szAmtZero, "%s", FMTNumberComma((_totalAmtZero * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtZero, "%s", FMTNumberComma((_totalAmtZero * 1000) / 1000).c_str());
        if (_totalAmtExempt != 0)
            sprintf (szAmtExempt, "%s", FMTNumberComma((_totalAmtExempt * 1000) / 1000 * -1).c_str());
        else
            sprintf (szAmtExempt, "%s", FMTNumberComma((_totalAmtExempt * 1000) / 1000).c_str());

        _txtvat += _util->cyPostSpace(10, "VATable");
        _txtvat += _util->cyPreSpace(10, szNetVat);
        _txtvat += _util->cyPreSpace(10, szAmtVat);
        _txtvat += "\n";

        _txtvat += _util->cyPostSpace(10, "VATExempt");
        _txtvat += _util->cyPreSpace(10, szNetExempt);
        _txtvat += _util->cyPreSpace(10, "0.00");//szAmtExempt);
        _txtvat += "\n";

        _txtvat += _util->cyPostSpace(10, "Zero VAT");
        _txtvat += _util->cyPreSpace(10, szNetZero);
        _txtvat += _util->cyPreSpace(10, "0.00");//szAmtZero);
        _txtvat += "\n";

    }  else  {

        _txtvat += _util->cyPostSpace(10, "VAT");
        _txtvat += _util->cyPreSpace(10, FMTNumberComma((_totalNetVat * 1000) / 1000));
        _txtvat += _util->cyPreSpace(10, FMTNumberComma((_totalAmtVat * 1000) / 1000));
        _txtvat += "\n";

        _txtvat += _util->cyPostSpace(10, "VATExempt");
        _txtvat += _util->cyPreSpace(10,  FMTNumberComma((_totalNetExempt * 1000) / 1000));
        _txtvat += _util->cyPreSpace(10, "0.00");//szAmtExempt);
        _txtvat += "\n";

        _txtvat += _util->cyPostSpace(10, "Zero-VAT");
        _txtvat += _util->cyPreSpace(10, FMTNumberComma((_totalNetZero * 1000) / 1000));
        _txtvat += _util->cyPreSpace(10, "0.00");//szAmtZero);
        _txtvat += "\n";

    }


    double totalNetVat = _totalNetVat + _totalNetZero + _totalNetExempt;
    char szTotalNetVat[32];
    double totalAmtVat = _totalAmtVat;// + _totalAmtZero + _totalAmtExempt;
    char szTotalAmtVat[32];

    if (totalNetVat != 0)
        sprintf (szTotalNetVat, "%s", FMTNumberComma((totalNetVat * 1000) / 1000).c_str());
    else
        strcpy (szTotalNetVat, "0.00");

    if (totalAmtVat != 0)
        sprintf (szTotalAmtVat, "%s", FMTNumberComma((totalAmtVat * 1000) / 1000).c_str());
    else
        strcpy (szTotalAmtVat, "0.00");

    _txtvat += _util->cyPostSpace(10, "Total");
    _txtvat += _util->cyPreSpace (10, szTotalNetVat);
    _txtvat += _util->cyPreSpace (10, szTotalAmtVat);
    _txtvat += "\n";
    _txtvat += _util->cyAcrossText(w,"-");
    _receipt += _txtvat;
    return true;
}
