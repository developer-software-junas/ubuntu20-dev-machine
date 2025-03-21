//*******************************************************************
//        FILE:     cyrestsales_receipt_eod_void.cpp
// DESCRIPTION:     Voided sales statistics
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     voidLoop
// DESCRIPTION:     Go through every voided sales record
//*******************************************************************
bool                CYRestReceiptEod::voidLoop ()
{
    //////////////////////////////////////
    //  Database connection
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
/*
    CYDbSql* dbeod = _ini->dbconn ();
    if (nullptr == dbeod)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Go through all sales record(s)
    int idx = 1;
    listVoid.clear();
    dbsel->sql_reset ();
    rcptType = SRT_POSTVOID;
    string sql = "select * from tg_pos_void_header ";
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
        string receiptType = _ini->get_value("RECEIPT","TYPE");
        /*receipt = new CYReceiptJoel (_log,_ini,_cgi);
        _log->logmsg("RECEIPT TYPE", "JOEL");*/
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            //voidCount++;
            nonTrxCount++;
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
            //  Offline sales database
            stringstream ss;  ss << jheader;
            if (argXread == false)  {
                if (!saveOfflineSales(systransnum,receipt->_exportType,receipt->_sqlSales,ss.str()))
                    return false;
            }
            //////////////////////////////
            //  Payment
            if (!paymentTotals(jpayment))
                return false;
            //////////////////////////////
            //  Item(s)
            if (!itemTotals(jheader, jdetail))
                return false;
            //////////////////////////////
            //  Add the void list
            STRUCT_EOD_VOIDCANCEL* strVoid = new STRUCT_EOD_VOIDCANCEL;
            strVoid->trxnum = transnum;
            strVoid->ordernum = ordernum;
            strVoid->amount = receipt->_totalGrossAmount;
            listVoid.push_back(strVoid);

            dbsel->sql_next();
        } while (!dbsel->eof ());
    }

    return true;
}
