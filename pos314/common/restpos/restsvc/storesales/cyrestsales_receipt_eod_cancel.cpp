//*******************************************************************
//        FILE:     cyrestsales_receipt_eod_refund.cpp
// DESCRIPTION:     Voided sales statistics
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     cancelLoop
// DESCRIPTION:     Go through every cancelled sales record
//*******************************************************************
bool                CYRestReceiptEod::cancelLoop ()
{
    cancelCount = 0;
    //char szFunc [] = "cancelLoop";
    //////////////////////////////////////
    //  Database connection
    //MySqlDb meod (_ini->_env);
    //CYDbSql* dbeod =&meod;//_ini-> dbconn ();
    //if (nullptr == dbeod){
        //_error = _util->jsonerror(szFunc,_ini->errormsg());
        //return false;
    //}
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;//_ini->dbconn ();
    //if (nullptr == dbsel){
        //_error = _util->jsonerror(szFunc,_ini->errormsg());
        //return false;
    //}
    //////////////////////////////////////
    //  Go through all cancelled record(s)
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_CANCEL;
    string sql = "select systransnum from tg_pos_cancel_header ";
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
        /*
        string receiptType = _ini->get_value("RECEIPT","TYPE");
        receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            cancelCount++;
            nonTrxCount++;
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
            //  Offline sales database
            stringstream ss;  ss << jheader;
            if (argXread == false)  {
                if (!saveOfflineSales(systransnum,receipt->_exportType,receipt->_sqlSales,ss.str()))
                    return false;
            }
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
bool                CYRestReceiptEod::cancelLoopFast ()
{
    cancelCount = 0;
    listCancel.clear();
    STRUCT_EOD_VOIDCANCEL strCancel;
    //char szFunc [] = "cancelLoop";
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*
    CYDbSql* dbeod =_ini->dbconn ();
    if (nullptr == dbeod){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Go through all cancelled record(s)
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_CANCEL;
    string sql = "select * from tg_pos_cancel_header ";
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
        /*string receiptType = _ini->get_value("RECEIPT","TYPE");
        receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            cancelCount++;
            receipt->resetEodTotals();
            string ordernum = dbsel->sql_field_value("pac_number");
            string systransnum = dbsel->sql_field_value("systransnum");
            string transnum = dbsel->sql_field_value("transaction_number");

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
            cancelAmount += receipt->_totalGrossAmount;
            //////////////////////////////
            //  Add the void list
            STRUCT_EOD_VOIDCANCEL* strVoid = new STRUCT_EOD_VOIDCANCEL;
            strVoid->trxnum = systransnum;
            strVoid->ordernum = ordernum;
            strVoid->amount = receipt->_totalGrossAmount;
            listCancel.push_back(strVoid);

            dbsel->sql_next();
        } while (!dbsel->eof ());
    }

    return true;
}
