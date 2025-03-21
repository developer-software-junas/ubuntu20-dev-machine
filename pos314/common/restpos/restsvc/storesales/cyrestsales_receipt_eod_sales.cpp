//*******************************************************************
//        FILE:     cyrestsales_receipt_eod_sales.cpp
// DESCRIPTION:     Sales statistics
//*******************************************************************
#include "cyrestcommon.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     salesLoop
// DESCRIPTION:     Go through every sales record
//*******************************************************************
bool                CYRestReceiptEod::salesLoop ()
{
    char szFunc [] = "salesLoop";
    //////////////////////////////////////
    //  receipt type
    string rcpt = _ini->get_value("RECEIPT","TYPE");
    //////////////////////////////////////
    //  Create the offline sales
    //  database for a ZREAD request
    if (argXread == false && rcpt == "elena")  {
        string backupFile = _ini->get_value("PATH","SALES");
        //backupFile += "/"; backupFile += argBranch; backupFile += ".";
        //backupFile += argSysdate;
        backupFile += "/sales.sqlite";
        if (_util->file_exists(backupFile.c_str()))  {
            _util->file_delete(backupFile.c_str ());
        }
        _liteEnv = new SqliteEnv;
        if (!_liteEnv->connect("localhost","brewery","cyware",backupFile,0))  {
            _error = _util->jsonerror(szFunc,_liteEnv->errordbenv());
            return false;
        }
        _liteDb = new SqliteDb (_liteEnv);
        _liteDb->sql_reset();
        string sql = "create table tg_pos_offline_sales";
        sql += "(systransnum varchar(64) , type varchar(24), sqldata text, jsondata text, is_polled integer); ";

        if (!_liteDb->sql_result(sql.c_str(),false))  {
            _error = _liteDb->errordb();
        }
        sql = "CREATE UNIQUE INDEX xpktg_pos_offline_sales on tg_pos_offline_sales (systransnum, type)";
        if (!_liteDb->sql_result(sql.c_str(),false))  {
            _error = _liteDb->errordb();
        }
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
    //  Go through all sales record(s)
    string sql;
    int idx = 1;
    dbsel->sql_reset ();
    rcptType = SRT_DAILY;
    sql = "select systransnum from tg_pos_daily_header ";
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
        receipt = new CYRestCommon (_log,_ini,_cgi);
        //////////////////////////////////
        //  For each sales record
        do  {
            trxCount++;
            //////////////////////////////
            //  Retrieve
            jheader.clear();
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
            //  Retrieved JSON
            stringstream ss;
            ss << jheader;
            //_log->logmsg("dump",ss.str().c_str());
            //////////////////////////////
            //  Offline sales database
            if (argXread == false && rcpt == "elena")  {
                if (!saveOfflineSales(systransnum,receipt->_exportType,receipt->_sqlSales,ss.str()))
                    return false;
            }
            //////////////////////////////
            //  Sales date
            checkDate(jheader);
            //////////////////////////////
            //  Payment
            if (!paymentTotals(jpayment))
                return false;
            //////////////////////////////
            //  Item(s)
            if (!itemTotals(jheader, jdetail))
                return false;
            dbsel->sql_next();
        } while (!dbsel->eof ());
        //////////////////////////////
        //  Cash fund
        if (!cashFund())
            return false;
        //////////////////////////////
        //  Cash pullout
        if (!cashPullout())
            return false;
        //////////////////////////////
        //  Cash declaration
        if (!cashDeclare())
            return false;
    }

    return true;
}
