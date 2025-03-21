/********************************************************************
          FILE:         rest_request.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
using std::string;
using std::stringstream;
#include "cyposrest.h"
/*******************************************************************
      FUNCTION:          assembleEodCashier
   DESCRIPTION:          EOD cashier audit
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodCashier ()
{
    double dtmp=0;
    char sztmp [64];
    _txtcashier = "";
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the cashier audit stats
    size_t  w = g_paperWidth;
    _txtcashier += _util->cyAcrossText(w, "-");
    _txtcashier += _util->cyCenterText(w,"Cashier's Audit");
    _txtcashier += _util->cyAcrossText(w, "-");

    left  = "No. Items Sold";
    right = _eod->FMTNumberComma(_eod->itemCount);
    _txtcashier += _util->cyLRText(24,left,17,right);

    long totalTxn = 0;
    left  = "Sales Txn #";
    dtmp  = _eod->cntCash+_eod->cntNonCash;
    sprintf (sztmp, "%ld", (long)dtmp);
    right = sztmp;
    totalTxn += atol(sztmp);
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Non Sales Txn #";
    //FIX
    _eod->nonTrxCount = _eod->cancelCount + _eod->voidCount + _eod->refundCount;
    sprintf (sztmp, "%d", _eod->nonTrxCount);
    right = sztmp;
    totalTxn += atol(sztmp);
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Total Txn #";
    sprintf (sztmp, "%ld", totalTxn);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    _eod->cancelTotal();
    left  = "Cancel Txn #";
    sprintf (sztmp, "%d", _eod->cancelCount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Cancel Amt";
    sprintf (sztmp, "%.02f", _eod->cancelAmount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    _eod->suspendTotal();
    left  = "Suspended Txn #";
    sprintf (sztmp, "%d", _eod->suspendRecs);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Suspended Txn Amt";
    sprintf (sztmp, "%.02f", _eod->suspendAmount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "No Items Suspended";
    sprintf (sztmp, "%.02f", _eod->suspendCount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    _eod->voidTotal();
    left  = "Total Void Txn #";
    sprintf (sztmp, "%d", _eod->voidCount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Total Void Amt";
    sprintf (sztmp, "%.02f", _eod->voidAmount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Discount Amt";
    sprintf (sztmp, "%.02f", _discountTotal);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    _eod->refundTotal ();
    left  = "Total Refund Txn #";
    sprintf (sztmp, "%d", _eod->refundCount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    left  = "Total Refund Amt";
    sprintf (sztmp, "%.02f", _eod->refundAmount);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);

    double averageSpend = 0.00;
    double d1 = (_eod->totCash+_eod->totNonCash);
    double d2 = (_eod->cntCash+_eod->cntNonCash);
    if (d1 == 0 || d2 == 0)  {
        averageSpend = 0.00;
    }  else  {
        averageSpend = (_eod->totCash+_eod->totNonCash) /
                       (_eod->cntCash+_eod->cntNonCash);
    }
    left  = "Average check";
    sprintf (sztmp, "%.02f", averageSpend);
    right = sztmp;
    _txtcashier += _util->cyLRText(24,left,17,right);
    /*
  int                   cntCash;    //  cash trx count
  double                totCash;    //  cash trx amount
  int                   cntNonCash; //  non cash trx count
  double                totNonCash; //  non cash trx amount
     */
    _receipt += _txtcashier;
    return true;
}
