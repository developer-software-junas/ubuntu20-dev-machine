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
      FUNCTION:          assembleEodBank
   DESCRIPTION:          EOD bank portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodBank ()
{
    _txtbank = "";
    int totalCount = 0;
    double totalAmount = 0;
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    _txtbank += _util->cyAcrossText(w, "-");
    _txtbank += _util->cyCenterText(w,"Acquiring Bank");
    _txtbank += _util->cyAcrossText(w, "-");

    int j = _eod->listBank.size ();

    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_BANK* ptr = _eod->listBank.at(i);
        totalCount += ptr->count;
        totalAmount += ptr->amount;

        left  = "Bank";
        left += " (Count)";
        right = "Amount";
        _txtbank += _util->cyLRText(24,left,17,right);

        left  = " ";
        left += ptr->bankDesc;
        left += " (";
        char szCount [16];
        sprintf (szCount, "%d", ptr->count);
        left += szCount;
        left += ")";
        right = _eod->FMTNumberComma(ptr->amount);
        _txtbank += _util->cyLRText(24,left,17,right);
    }
    left  = "TOTAL (";
    char szCount [16];
    sprintf (szCount, "%d", totalCount);
    left += szCount;
    left += ")";
    right = _eod->FMTNumberComma(totalAmount);
    _txtbank += _util->cyLRText(24,left,17,right);

    _receipt += _txtbank;
    return true;
}
