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
      FUNCTION:          assembleEodCashfund
   DESCRIPTION:          EOD cash fund / takeout portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodCashfund ()
{
    _txtfund = "";
    _drawer  = 0.00;
    int totalCount = 0;
    double totalAmount = 0;
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    _txtfund += _util->cyAcrossText(w, "-");
    _txtfund += _util->cyCenterText(w,"Cash fund / pullout");
    _txtfund += _util->cyAcrossText(w, "-");

    left  = "Change fund";
    right = "Amount";
    double totalCashfund = 0;
    _txtfund += _util->cyLRText(24,left,17,right);

    int j = _eod->listFund.size ();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = _eod->listFund.at(i);
        totalCount += ptr->count;
        totalCashfund += ptr->amount;

        left  = " ";
        left += ptr->tenderDesc;
        left += " (";
        char szCount [16];
        sprintf (szCount, "%d", ptr->count);
        left += szCount;
        left += ")";
        right = _eod->FMTNumberComma(ptr->amount);
        _txtfund += _util->cyLRText(24,left,17,right);
    }

    left  = "Total Cash Fund (";
    char szCount [16];
    sprintf (szCount, "%d", totalCount);
    left += szCount;
    left += ")";
    right = _eod->FMTNumberComma(totalCashfund);
    _txtfund += _util->cyLRText(24,left,17,right);
    _txtfund += "\n";

    //  Sum up the cash takeout
    totalCount = 0;
    totalAmount = 0;
    string strhdr = "";
    double totalPullout = 0;
    j = _eod->listPullout.size();


    strhdr += _util->cyCenterText(w, " ");
    strhdr += _util->cyCenterText(w, "C A S H  P U L L O U T");
    strhdr += _util->cyCenterText(w, " ");
    strhdr += "Tender    Denomination    Qty     Amount\n";
    strhdr += _util->cyAcrossText(40, "-");
    _txtfund += strhdr;

    Json::Value jline, jtakeout;

    for (int i = 0; i < j; i++)  {
        char sztmp[255];
        totalCount++;
        //////////////////////////////
        //  Receipt detail
        STRUCT_EOD_PAYMENT* ptr = _eod->listPullout.at(i);
        string denom = ptr->tenderCode;
        string tender_desc = ptr->tenderDesc;

        tmp = _util->cyPostSpace(13,tender_desc);
        tmp += _util->cyPreSpace(9,denom);

        double dqty = ptr->count;
        sprintf (sztmp, "%d", (int)dqty);
        tmp += _util->cyPreSpace(7,sztmp); //tmp2 += tmp;

        double damt = ptr->amount;
        sprintf (sztmp, "%.02f", damt);
        totalPullout += damt;
        tmp += _util->cyPreSpace(11, sztmp); //tmp2 += tmp;

        tmp += "\n";
        _txtfund += tmp;
    }

    left  = "Total Pullout (";
    sprintf (szCount, "%d", totalCount);
    left += szCount;
    left += ")";
    right = _eod->FMTNumberComma(totalPullout);
    _txtfund += _util->cyLRText(24,left,17,right);
    _txtfund += "\n";


    string receiptType = _ini->get_value("RECEIPT","TYPE");
    //if (receiptType == "east")  {
    _drawer = 0;
    totalAmount = 0;
        j = _eod->listPay.size ();
        for (int i = 0; i < j; i++)  {
            STRUCT_EOD_PAYMENT* ptr = _eod->listPay.at(i);
            if (ptr->iscash)
                totalAmount += ptr->amount;
            //left  = ptr->tenderDesc;
            //right = _eod->FMTNumberComma(ptr->amount);
            //_txtdeclare += _util->cyLRText(24,left,17,right);
        }
        _drawer = (totalAmount + totalCashfund) - totalPullout;
    //}  else  {
        //_drawer -= totalAmount;
        //_drawer += _eod->totCash;
    //}

    left  = "TOTAL IN DRAWER";
    right = _eod->FMTNumberComma(_drawer);
    _txtfund += _util->cyLRText(24,left,17,right);

    _receipt += _txtfund;
    return true;
}
