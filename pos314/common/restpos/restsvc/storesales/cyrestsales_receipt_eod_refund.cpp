//*******************************************************************
//        FILE:     cyrestsales_receipt_eod_refund.cpp
// DESCRIPTION:     Voided sales statistics
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     refundLoop
// DESCRIPTION:     Go through every refunded sales record
//*******************************************************************
bool                CYRestReceiptEod::refundLoop ()
{
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
    //  Go through all sales record(s)
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_REFUND;
    string sql = "select systransnum from tg_pos_refund_header ";
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
        refundCount++;
        nonTrxCount++;
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        string receiptType = _ini->get_value("RECEIPT","TYPE");
        /*receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
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
            //  Payment
            /*
            if (!paymentTotals(jpayment))
                return false;
             */
            //////////////////////////////
            //  Item(s)
            if (!itemTotals(jheader, jdetail))
                return false;

            dbsel->sql_next();
        } while (!dbsel->eof ());
    }

    return true;
}
