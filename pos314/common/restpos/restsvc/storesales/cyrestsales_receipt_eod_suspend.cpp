//*******************************************************************
//        FILE:     cyrestsales_receipt_eod_suspend.cpp
// DESCRIPTION:     Suspended sales
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     suspendLoop
// DESCRIPTION:     Go through every suspended sales record
//*******************************************************************
bool                CYRestReceiptEod::suspendLoop ()
{
    suspendRecs=0;
    suspendCount=0;
    suspendAmount=0;
    string receiptType = _ini->get_value("RECEIPT","TYPE");
    if (receiptType == "joel")  {
        _ini->connswitch(true);
    }
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbeod = _ini->dbconn ();
    if (nullptr == dbeod)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Go through all suspended record(s)
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_SUSPEND;
    string sql = "select systransnum from tg_pos_suspend_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, argBranch); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, argSysdate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, argRegister); idx++;

    if (argXread)  {
        sql += " and  cashier = "; sql += dbsel->sql_bind (idx, argCashier); idx++;
        sql += " and  cashier_shift = "; sql += dbsel->sql_bind (idx, argShift); idx++;
    }
    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        /*receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            suspendRecs++;
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, rcptType))
                return seterrormsg(receipt->errormsg());

            if (!receipt->retrieveSalesHeaderAccount(jheader))
                return seterrormsg(receipt->errormsg());

            Json::Value jdetail;
            jdetail.clear();
            if (!receipt->retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
                    return seterrormsg(receipt->errormsg());

            jheader["det_sales"]=jdetail;

            Json::Value jpayment;
            jpayment.clear();
            if (!receipt->retrieveSalesPayment(jpayment, systransnum))
                return seterrormsg(receipt->errormsg());
            jheader["pay_sales"]=jpayment;
            //////////////////////////////
            //  Item(s)
            if (!itemTotals(jheader, jdetail))
                return false;

            dbsel->sql_next();
        } while (!dbsel->eof ());
    }

    return true;
}
//*******************************************************************
//    FUNCTION:     cancelLoop
// DESCRIPTION:     Go through every cancelled sales record
//*******************************************************************
bool                CYRestReceiptEod::suspendLoopFast ()
{
    char szFunc [] = "suspendLoop";
    //////////////////////////////////////
    //  Online for Joel
    string receiptType = _ini->get_value("RECEIPT","TYPE");
    if (receiptType == "joel")  {
        _ini->connswitch(true);
    }
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbeod =_ini-> dbconn ();
    if (nullptr == dbeod){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Go through all suspended record(s)
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_SUSPEND;
    string sql = "select systransnum from tg_pos_suspend_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, argBranch); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, argSysdate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, argRegister); idx++;

    if (argXread)  {
        sql += " and  cashier = "; sql += dbsel->sql_bind (idx, argCashier); idx++;
        sql += " and  cashier_shift = "; sql += dbsel->sql_bind (idx, argShift); idx++;
    }
    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {

        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        /*receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            suspendRecs++;
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, rcptType))
                return seterrormsg(receipt->errormsg());

            if (!receipt->retrieveSalesHeaderAccount(jheader))
                return seterrormsg(receipt->errormsg());

            Json::Value jdetail;
            jdetail.clear();
            if (!receipt->retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
                    return seterrormsg(receipt->errormsg());

            jheader["det_sales"]=jdetail;

            Json::Value jpayment;
            jpayment.clear();
            if (!receipt->retrieveSalesPayment(jpayment, systransnum))
                return seterrormsg(receipt->errormsg());
            jheader["pay_sales"]=jpayment;

            //////////////////////////////
            //  Total
            suspendCount += receipt->_totalItems;
            suspendAmount += receipt->_totalGrossAmount;

            dbsel->sql_next();
        } while (!dbsel->eof ());
    }

    return true;
}
