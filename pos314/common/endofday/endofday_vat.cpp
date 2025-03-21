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
      FUNCTION:          assembleEodDiscount
   DESCRIPTION:          EOD discount portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodVat ()
{
    _txtvat = "";
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    _txtvat += _util->cyAcrossText(w, "-");
    _txtvat += _util->cyCenterText(w,"VAT");
    _txtvat += _util->cyAcrossText(w, "-");

    left  = "VATable";
    right = _eod->FMTNumberComma(_eod->rcptVatable);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "VAT Exempt";
    right = _eod->FMTNumberComma(_eod->rcptVatExempt);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "VAT Zero";
    right = _eod->FMTNumberComma(_eod->rcptVatZero);
    _txtvat += _util->cyLRText(24,left,17,right);


    double ns = _eod->rcptVatable + _eod->rcptVatExempt + _eod->rcptVatZero;
    left  = "NET Sales";
    right = _eod->FMTNumberComma(ns);//_eod->rcptNetSales);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "VAT";
    right = _eod->FMTNumberComma(_eod->rcptVat);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "Gross Sales";
    double gs = ns + _eod->rcptVat;
    //right = _eod->FMTNumberComma(_eod->rcptGrossSales);
    right = _eod->FMTNumberComma(gs);//_eod->rcptNetSales + _eod->rcptVat);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "Service Charge";
    right = _eod->FMTNumberComma(_eod->rcptServiceCharge);
    _txtvat += _util->cyLRText(24,left,17,right);

    left  = "GRAND TOTAL";
    //right = _eod->FMTNumberComma(_eod->rcptServiceCharge+_eod->rcptGrossSales);
    right = _eod->FMTNumberComma(_eod->totCash + _eod->totNonCash);
    _txtvat += _util->cyLRText(24,left,17,right);
/*
    _txtvat += _util->cyAcrossText(w, "-");
    _txtvat += _util->cyCenterText(w,"SERVICE CHARGE");
    _txtvat += _util->cyAcrossText(w, "-");
*/

/*
    left  = "Total Sales + Svc Chrg";
    right = _eod->FMTNumberComma((_eod->rcptNetSales + _eod->rcptVat) + _eod->rcptServiceCharge);
    _txtvat += _util->cyLRText(24,left,17,right);
*/
    _receipt += _txtvat;
    return true;
}
/*******************************************************************
      FUNCTION:          assembleEodFsp
   DESCRIPTION:          EOD FSP
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodFsp ()
{
    _txtvat = "";
    bool proceed = false;
    if (_eod->listFspCustomer.size() < 1)
        return true;
    string rcptType = _ini->get_value("RECEIPT","TYPE");
    if (rcptType == "joel")
        proceed = true;
    if (!proceed)
        return true;

    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    _txtvat += _util->cyAcrossText(w, "-");
    _txtvat += _util->cyCenterText(w,"FSP Transactions");
    _txtvat += _util->cyAcrossText(w, "-");

    _txtvat += _util->cyPostSpace(15, "Card");
    _txtvat += _util->cyPreSpace(10, "ePurse");
    _txtvat += _util->cyPreSpace(14, "Points");
    _txtvat += "\n";
    _txtvat += _util->cyPostSpace(15, "Number");
    _txtvat += _util->cyPreSpace(10, "Spent");
    _txtvat += _util->cyPreSpace(14, "Earned");
    _txtvat += "\n";
    _txtvat += _util->cyAcrossText(w, "-");

    double totalEpurse = 0.00;
    double totalPoints = 0.00;
    int j = _eod->listFspCustomer.size();
    for (int i = 0; i < j; i++)  {
        totalEpurse += _eod->listFspCustomer.at(i)->epurse;
        totalPoints += _eod->listFspCustomer.at(i)->points;
        _txtvat += _util->cyPostSpace(17, _eod->listFspCustomer.at(i)->account_number);
        _txtvat += _util->cyPreSpace(10, _eod->FMTNumberComma (_eod->listFspCustomer.at(i)->epurse));
        _txtvat += _util->cyPreSpace(12, _eod->FMTNumberComma (_eod->listFspCustomer.at(i)->points));
        _txtvat += "\n";
    }
    _txtvat += _util->cyAcrossText(w, "-");
    _txtvat += _util->cyPostSpace(17, "Total");
    _txtvat += _util->cyPreSpace(10, _eod->FMTNumberComma (totalEpurse));
    _txtvat += _util->cyPreSpace(12, _eod->FMTNumberComma (totalPoints));
    _txtvat += "\n";

    _receipt += _txtvat;
    return true;
}
