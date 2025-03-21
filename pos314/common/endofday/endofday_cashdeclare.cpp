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
      FUNCTION:          assembleEodCashDeclare
   DESCRIPTION:          EOD cash declaration
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodCashDeclare ()
{
    _txtdeclare = "";
    double totalAmount = 0;
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    string receiptType = _ini->get_value("RECEIPT","TYPE");
    //if (receiptType == "east")  {
        _txtdeclare += _util->cyAcrossText(w, "-");
        _txtdeclare += _util->cyCenterText(w,"Total Declaration");
        _txtdeclare += _util->cyAcrossText(w, "-");
    //}  else  {
        //_txtdeclare += _util->cyAcrossText(w, "-");
        //_txtdeclare += _util->cyCenterText(w,"Cash Declaration");
        //_txtdeclare += _util->cyAcrossText(w, "-");
   // }

    left  = "Tender type";
    right = "Amount";
    _txtdeclare += _util->cyLRText(24,left,17,right);


   // if (receiptType == "east")  {
        int j = _eod->listPay.size ();
        for (int i = 0; i < j; i++)  {
            STRUCT_EOD_PAYMENT* ptr = _eod->listPay.at(i);
            totalAmount += ptr->amount;
            left  = ptr->tenderDesc;
            right = _eod->FMTNumberComma(ptr->amount);
            if (false == ptr->iscash)
                _txtdeclare += _util->cyLRText(24,left,17,right);
        }
        if (j > 0)
            totalAmount -= _eod->totCash;
    //}
        j = _eod->listDeclare.size ();
        for (int i = 0; i < j; i++)  {
            STRUCT_EOD_PAYMENT* ptr = _eod->listDeclare.at(i);
            if (ptr->cashdec && ptr->declared > 0)  {
                totalAmount += ptr->declared;

                left  = ptr->tenderDesc;
                right = _eod->FMTNumberComma(ptr->declared);
                _txtdeclare += _util->cyLRText(24,left,17,right);
            }
        }


    left  = "Total Declaration";
    right = _eod->FMTNumberComma(totalAmount);
    _txtdeclare += _util->cyLRText(24,left,17,right);

    if (_eod->argXread)  {
        _txtdeclare += _util->cyAcrossText(w, "-");
        _txtdeclare += _util->cyCenterText(w,"Over / Short");
        _txtdeclare += _util->cyAcrossText(w, "-");

        if (_drawer == (totalAmount - _eod->totNonCash))  {
            left = "Over / Short";
            right = "";
        }  else  {
            if (_drawer > (totalAmount - _eod->totNonCash))  {
                left  = "SHORT";
                right = "-";
            }  else  {
                left = "OVER";
                right = "+";
            }
        }
        right += _eod->FMTNumberComma(_drawer - (totalAmount - _eod->totNonCash));
        _txtdeclare += _util->cyLRText(24,left,17,right);
    }

    _receipt += _txtdeclare;
    return true;
}
