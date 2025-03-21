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
      FUNCTION:          assembleEodPayment
   DESCRIPTION:          EOD payment portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodPayment ()
{
    _txtpay = "";
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the payment list
    size_t  w = g_paperWidth;
    _txtpay += _util->cyAcrossText(w, "-");
    _txtpay += _util->cyCenterText(w,"Payment type(s)");
    _txtpay += _util->cyAcrossText(w, "-");

    int j = _eod->listPay.size ();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = _eod->listPay.at(i);

        left  = ptr->tenderDesc;
        left += " (Count)";
        right = "Amount";
        _txtpay += _util->cyLRText(24,left,17,right);

        left  = " ";
        left += ptr->tenderDesc;
        left += " (";
        char szCount [16];
        sprintf (szCount, "%d", ptr->count);
        left += szCount;
        left += ")";
        right = _eod->FMTNumberComma(ptr->amount);
        _txtpay += _util->cyLRText(24,left,17,right);
    }
    _receipt += _txtpay;
    return true;
}
