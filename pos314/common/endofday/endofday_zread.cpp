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
#include "cydb/cysqlite.h"
/*******************************************************************
      FUNCTION:          request
   DESCRIPTION:          Process the REST request
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
string                   cyposrest::rest_zread ()
{
    //////////////////////////////////////
    //  Retrieve argument(s)
    string cashier = cgiform("cashier");
    string manager = cgiform("manager");
    string mgrpass = cgiform("mgrpass");
    string register_num = cgiform("register_num");
    string logical_date = cgiform("logical_date");
    string branch_code = cgiform("location_code");
    string cashier_shift = cgiform("cashier_shift");
    string clear_suspended = cgiform("clear_suspended");
    /*
    cashier = "st000199";
    manager = "NMABAT";
    mgrpass = "bypasspos314";
    register_num = "7";
    logical_date = "20231219";
    branch_code = "000004";
    cashier_shift = "1";
    clear_suspended = "N";
    */
    //////////////////////////////////////
    //  Assign to the EOD variable(s)
    _eod->argXread = false;
    _eod->argManager = manager;
    _eod->argCashier = cashier;
    _eod->argShift = cashier_shift;
    _eod->argRegister = register_num;
    _eod->argSysdate = logical_date;
    _eod->argBranch = branch_code;
    _eod->argSuspended = clear_suspended;    
    //////////////////////////////////////
    //  Retrieve EOD data
    if (!_eod->retrieveFastX())
        return errorjson(_eod->errormsg());
    //////////////////////////////////////
    //  Assemble
    if (!assembleEodHeader())
        return errorjson(_eod->errormsg());
    if (!assembleEodPayment())
        return errorjson(_eod->errormsg());
    if (!assembleEodCash())
        return errorjson(_eod->errormsg());
    if (!assembleEodDiscount())
        return errorjson(_eod->errormsg());
    if (!assembleEodVat())
        return errorjson(_eod->errormsg());
    if (!assembleEodBank())
        return errorjson(_eod->errormsg());
    if (!assembleEodFsp())
        return errorjson(_eod->errormsg());

    if (!assembleEodCashfund())
        return errorjson(_eod->errormsg());
    if (!assembleEodCashDeclare())
        return errorjson(_eod->errormsg());

    if (!assembleEodCashier())
        return errorjson(_eod->errormsg());
    if (!assembleEodFooter())
        return errorjson(_eod->errormsg());
    //////////////////////////////////////
    //  return JSON success
    Json::Value root;
    root["status"]="ok";
    root["can_zread"]=_eod->listUserXread.size()==0;
    //////////////////////////////////////
    //  Save the receipt
    string base64Receipt = _util->base64encode(_receipt);
    root["z64_receipt"]=base64Receipt;

    stringstream ss;
    ss << root;
    return ss.str ();
}
