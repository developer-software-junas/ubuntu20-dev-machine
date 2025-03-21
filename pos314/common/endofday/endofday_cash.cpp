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
      FUNCTION:          assembleEodCash
   DESCRIPTION:          cash / non cash portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodCash ()
{
    _txtcash = "";
    string tmp, left, right;
    //////////////////////////////////////
    //  Cash / Non cash statistics
    char szCount[16];
    size_t  w = g_paperWidth;

    _txtcash += _util->cyAcrossText(w, "-");
    sprintf(szCount, "%d", _eod->cntCash);
    left  = "TOTAL CASH";
    left += " (";
    left += szCount;
    left += ") ";
    right = _eod->FMTNumberComma(_eod->totCash);
    _txtcash += _util->cyLRText(20,left,21,right);

    sprintf(szCount, "%d", _eod->cntNonCash);
    left  = "TOTAL NON CASH";
    left += " (";
    left += szCount;
    left += ") ";
    right = _eod->FMTNumberComma(_eod->totNonCash);
    _txtcash += _util->cyLRText(20,left,21,right);

    sprintf(szCount, "%d", _eod->cntNonCash + _eod->cntCash);
    left  = "TOTAL SALES";
    left += " (";
    left += szCount;
    left += ") ";
    right = _eod->FMTNumberComma(_eod->totNonCash + _eod->totCash);
    _txtcash += _util->cyLRText(20,left,21,right);

    _receipt += _txtcash;
    return true;
}
