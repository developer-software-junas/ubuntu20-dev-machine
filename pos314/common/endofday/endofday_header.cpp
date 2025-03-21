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
      FUNCTION:          assembleEodHeader
   DESCRIPTION:          EOD header portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodHeader ()
{
    _txthdr = "";
    _receipt = "";
    string tmp = "";
    //////////////////////////////////////
    //  Display the header
    size_t  w = g_paperWidth;

    _txthdr += _util->cyCenterText(w, " ");
    _txthdr += _util->cyCenterText(w, _eod->getAltValue("trade_name",_eod->alt_settings));
    _txthdr += _util->cyCenterText(w, _eod->getAltValue("description",_eod->alt_settings_branch));
    _txthdr += _util->cyCenterText(w, _eod->getAltValue("addr1",_eod->alt_settings_branch));
    _txthdr += _util->cyCenterText(w, _eod->getAltValue("addr2",_eod->alt_settings_branch));

    tmp  = "VAT-REG-TIN-";
    tmp += _eod->getAltValue("tin",_eod->alt_settings_branch);
    _txthdr += _util->cyCenterText(w, tmp);

    tmp = _eod->getAltValue("serialno",_eod->alt_settings_register);
    if (tmp.length() > w) {
        tmp = tmp.substr(0, w);
    }
    _txthdr += _util->cyCenterText(w, tmp);

    tmp  = "Permit No. ";
    tmp += _eod->getAltValue("permit",_eod->alt_settings_register);
    _txthdr += _util->cyCenterText(w, tmp);

    tmp  = "ACCR.# ";
    tmp += _eod->getAltValue("bir_num",_eod->alt_settings_branch);
    _txthdr += _util->cyCenterText(w, tmp);

    _txthdr += _util->cyCenterText(w, " ");
    _txthdr += _util->cyCenterText(w, _eod->argXread ? "X READING" : "Z READING");

    tmp  = "POS - ";
    tmp += _eod->getAltValue("register_num",_eod->alt_settings_register);
    _txthdr += _util->cyCenterText(w,tmp);

    if (_eod->argXread)  {
        tmp  = "Shift ";
        tmp += _eod->argShift;
        tmp += " of ";
        tmp += _eod->getAltValue("last_name",_eod->alt_settings_cashier);
        tmp += ", ";
        tmp += _eod->getAltValue("first_name",_eod->alt_settings_cashier);
        tmp += " - ";
        tmp += _eod->getAltValue("login",_eod->alt_settings_cashier);
        _txthdr += _util->cyCenterText(w,tmp);
    }

    char szFullDate [64];
    sprintf (szFullDate, "%012ld", (long)_eod->startDate);
    string fullDate = szFullDate;
    string start_date = fullDate.substr(0, 8);
    string start_time = fullDate.substr(8, 4);

    sprintf (szFullDate, "%012ld", (long)_eod->endDate);
    fullDate = szFullDate;
    string end_date = fullDate.substr(0, 8);
    string end_time = fullDate.substr(8, 4);

    tmp  = "From: ";
    tmp += _eod->date_rcc (start_date);
    tmp += "-";
    tmp += _util->time_colon(start_time.c_str());
    _txthdr += _util->cyCenterText(w,tmp);

    tmp  = "To: ";
    tmp += _eod->date_rcc (end_date);
    tmp += "-";
    tmp += _util->time_colon(end_time.c_str());
    _txthdr += _util->cyCenterText(w,tmp);
    _receipt += _txthdr;
    return true;
}
