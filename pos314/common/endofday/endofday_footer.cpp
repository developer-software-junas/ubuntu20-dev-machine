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
bool                     cyposrest::assembleEodFooter ()
{
    _txtfooter = "";
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the cashier audit stats
    size_t  w = g_paperWidth;

    //if (_eod->argXread == false)  {
        char sztmp [16];
        string lan = _eod->getAltValue("adjust_amount",_eod->alt_settings_register);
        double dlan = _util->stodsafe(lan);

        _txtfooter += _util->cyAcrossText(w, "-");
        if (dlan != 0)  {
            _txtfooter += _util->cyCenterText(w,"Adjustment transaction nos.");

            tmp  = "From ";
            tmp += _eod->getAltValue("last_adjust_transnum",_eod->alt_settings_register);

            tmp += " To ";
            left = _eod->getAltValue("adjust_transnum",_eod->alt_settings_register);
            int trx = atoi(left.c_str ());
            if (trx >= 1) trx--;

            sprintf (sztmp, "%d",trx);
            tmp += sztmp;
            _txtfooter += _util->cyCenterText(w,tmp);
            _txtfooter += "\n";
        }  else  {
            _txtfooter += _util->cyCenterText(w,"Adjustment details");
            tmp  = "Last Adjustment# ";
            tmp += _eod->getAltValue("last_adjust_transnum",_eod->alt_settings_register);
            _txtfooter += _util->cyCenterText(w,tmp);
            _txtfooter += "\n";
        }

        left = "Old adjusted amount";
        right = _eod->getAltValue("last_adjust_amount",_eod->alt_settings_register);
        _txtfooter += _util->cyLRText(24,left,17,right);

        left = "New adjusted amount";
        right = _eod->getAltValue("adjust_amount",_eod->alt_settings_register);
        _txtfooter += _util->cyLRText(24,left,17,right);

        if (_eod->trxStart != 0)  {
            _txtfooter += _util->cyAcrossText(w, "=");
            string rcptType = _ini->get_value("RECEIPT","TYPE");
            if (rcptType == "elena")  {
                _txtfooter += _util->cyCenterText(w,"Sales chit nos.");
            }  else  {
                _txtfooter += _util->cyCenterText(w,"Sales invoice nos.");
            }

            sprintf(sztmp, "%d", _eod->trxStart);
            tmp  = sztmp;
            tmp += " to ";

            int tempTrx = _eod->trxEnd;
            //if (tempTrx != 1) tempTrx--;
            sprintf(sztmp, "%d", tempTrx);
            tmp += sztmp;
            _txtfooter += _util->cyCenterText(w,tmp);
        }
        _txtfooter += "\n";
    //}

    tmp = "GENERATED ON ";
    _txtfooter += _util->cyCenterText(w,tmp);

    left  = _eod->date_rcc (_util->date_eight().c_str ());
    left += "-";
    left += _util->time_colon(_util->int_time().c_str ());
    _txtfooter += _util->cyCenterText(w,left);

    _txtfooter += _util->cyAcrossText(w, "=");
    //////////////////////////////////////
    //  Finally...
    string xread_count = _eod->getAltValue("xreadcount",_eod->alt_settings_register);
    string zread_count = _eod->getAltValue("zreadcount",_eod->alt_settings_register);
    string reset_count = _eod->getAltValue("resetcount",_eod->alt_settings_register);
    string old_amount  = _eod->getAltValue("transaction_amount",_eod->alt_settings_register);
    if (xread_count.length () < 1)  xread_count = "0";
    if (zread_count.length () < 1)  zread_count = "0";
    if (reset_count.length () < 1)  reset_count = "0";

    double dOldAmount = _util->stodsafe(old_amount);
    double dNewAmount = _eod->totCash + _eod->totNonCash;

    left = "OLD ACCUM SALES";
    if (false == _eod->argXread)
        dOldAmount -= dNewAmount;
    if (dOldAmount < 0.00)
        dOldAmount = 0;
    right = _eod->FMTNumberComma(dOldAmount);
    _txtfooter += _util->cyLRText(24,left,17,right);

    left = "NEW ACCUM SALES";
    //if (false == _eod->argXread)
        right = _eod->FMTNumberComma(dNewAmount+dOldAmount);
    //else
        //right = _eod->FMTNumberComma(dNewAmount);
    _txtfooter += _util->cyLRText(24,left,17,right);

    if (_eod->argXread)  {
        left = "XREAD COUNT: ";
        right = xread_count;
        _txtfooter += _util->cyLRText(24,left,17,right);
    }  else  {
        left = "ZREAD COUNT: ";
        right = zread_count;
        _txtfooter += _util->cyLRText(24,left,17,right);
    }
    left = "RESET COUNT: ";
    right = reset_count;
    _txtfooter += _util->cyLRText(24,left,17,right);
    _txtfooter += _util->cyAcrossText(w, "=");

    _txtfooter += _util->cyCenterText(w, "AUTHORIZED BY");
    tmp  = _eod->getAltValue("first_name",_eod->alt_settings_manager);
    tmp += " ";
    tmp += _eod->getAltValue("last_name",_eod->alt_settings_manager);
    tmp += " [";
    tmp += _eod->argManager;
    tmp += "]";
    _txtfooter += _util->cyCenterText(w, tmp);

    _receipt += _txtfooter;
    return true;
}
