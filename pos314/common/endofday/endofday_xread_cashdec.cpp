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
      FUNCTION:          request
   DESCRIPTION:          Process the REST request
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
string                   cyposrest::rest_xread_cashdec ()
{
    //return errorjson("xread");
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
    cashier = "angie";
    manager = "cyware";
    mgrpass = "bypasspos314";
    register_num = "1";
    logical_date = "20240216";
    branch_code = "03";
    cashier_shift = "1";
    clear_suspended = "1";
*/
    //////////////////////////////////////
    //  Assign to the EOD variable(s)
    _eod->argXread = true;
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
    //  Cleanup suspended
    if (_eod->argSuspended == "1")
        cleanupSuspended(_eod->argBranch,_eod->argRegister,_eod->argSysdate,_eod->argCashier);
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
    if (!assembleEodCashfund())
        return errorjson(_eod->errormsg());
    if (!assembleEodCashDeclare())
        return errorjson(_eod->errormsg());
    if (!assembleEodCashier())
        return errorjson(_eod->errormsg());
    if (!assembleEodFooter())
        return errorjson(_eod->errormsg());
    //////////////////////////////////////
    //  Cash declaration
    int idx = 0;
    Json::Value jcashdec, jline;

    jcashdec = Json::nullValue;
    int j = _eod->listPay.size();
    for (int i = 0; i < j; i++)  {
        jline.clear ();
        STRUCT_EOD_PAYMENT* pay = _eod->listPay.at(i);
        if (pay->cashdec)  {
            jline["pay_code"]=pay->tenderCode;
            jline["pay_count"]=pay->count;
            jline["pay_desc"]=pay->tenderDesc;
            jline["pay_type_code"]=pay->tenderTypeCode;
            jline["pay_type_desc"]=pay->tenderTypeDesc;
            jline["pay_amt"]=pay->amount;

            jcashdec[idx]=jline;
            idx++;
        }
    }
    //////////////////////////////////////
    //  return JSON success
    Json::Value root;
    root["status"]="ok";
    root["declare"]=jcashdec;
    root["can_zread"]=_eod->listUserXread.size()==0;
    //////////////////////////////////////
    //  Save the receipt
    //_log->logmsg("zzzzz",_receipt.c_str ());
    string base64Receipt = _util->base64encode(_receipt);
    root["z64_receipt"]=base64Receipt;

    stringstream ss;
    ss << root;
    return ss.str ();
}
