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
      FUNCTION:          offline_sales
   DESCRIPTION:          Generate offline sales
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::exportOfflineSales (std::string branchCode,
                                                         std::string registerNumber,
                                                         std::string logicalDate)
{
    char szFunc [] = "offlineSales";
    (void)szFunc; (void)branchCode; (void)registerNumber; (void)logicalDate;
/*
    string backupFile = _ini->get_value("PATH","SALES");
    backupFile += "/"; backupFile += logicalDate; backupFile += ".sqlite";

    if (_util->file_exists(backupFile.c_str()))  {
        _util->file_delete(backupFile.c_str ());
    }
    _liteEnv = new SqliteEnv;
    if (!_liteEnv->connect("localhost","brewery","cyware",backupFile,0))  {
        _error = _liteEnv->errordbenv();
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
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbeod = _ini->dbconn ();
    if (!dbeod->ok()){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok()){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Go through all sales record(s)
    int idx = 1;
    dbsel->sql_reset ();
    CYRestCommon* receipt = new CYRestCommon (_log,_ini,_cgi);

    sql  = "select systransnum from tg_pos_daily_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, branchCode); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, logicalDate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, registerNumber); idx++;

    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            jheader.clear();
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, SRT_DAILY))
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

            _liteDb->sql_reset();
            stringstream ss; ss << jheader;
            string sql = "insert into tg_pos_offline_sales (systransnum,type,sqldata,jsondata)values(";
            sql += _liteDb->sql_bind(1,systransnum);  sql += ",";
            sql += _liteDb->sql_bind(2,receipt->_exportType);  sql += ",";
            sql += _liteDb->sql_bind(3,receipt->_sqlSales);  sql += ",";
            sql += _liteDb->sql_bind(4,ss.str());  sql += ")";
            if (!_liteDb->sql_only(sql,true))
                return seterrormsg(_liteDb->errordb());
            if (!_liteDb->sql_commit())
                return seterrormsg(_liteDb->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Go through all voided record(s)
    idx = 1;
    dbsel->sql_reset ();

    sql  = "select systransnum from tg_pos_void_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, branchCode); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, logicalDate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, registerNumber); idx++;

    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            jheader.clear();
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, SRT_POSTVOID))
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

            _liteDb->sql_reset();
            stringstream ss; ss << jheader;
            string sql = "insert into tg_pos_offline_sales (systransnum,type,sqldata,jsondata)values(";
            sql += _liteDb->sql_bind(1,systransnum);  sql += ",";
            sql += _liteDb->sql_bind(2,receipt->_exportType);  sql += ",";
            sql += _liteDb->sql_bind(3,receipt->_sqlSales);  sql += ",";
            sql += _liteDb->sql_bind(4,ss.str());  sql += ")";
            if (!_liteDb->sql_only(sql,true))
                return seterrormsg(_liteDb->errordb());
            if (!_liteDb->sql_commit())
                return seterrormsg(_liteDb->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Go through all refunded record(s)
    idx = 1;
    dbsel->sql_reset ();

    sql  = "select systransnum from tg_pos_refund_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, branchCode); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, logicalDate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, registerNumber); idx++;

    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            jheader.clear();
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, SRT_REFUND))
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

            _liteDb->sql_reset();
            stringstream ss; ss << jheader;
            string sql = "insert into tg_pos_offline_sales (systransnum,type,sqldata,jsondata)values(";
            sql += _liteDb->sql_bind(1,systransnum);  sql += ",";
            sql += _liteDb->sql_bind(2,receipt->_exportType);  sql += ",";
            sql += _liteDb->sql_bind(3,receipt->_sqlSales);  sql += ",";
            sql += _liteDb->sql_bind(4,ss.str());  sql += ")";
            if (!_liteDb->sql_only(sql,true))
                return seterrormsg(_liteDb->errordb());
            if (!_liteDb->sql_commit())
                return seterrormsg(_liteDb->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Go through all cancelled record(s)
    idx = 1;
    dbsel->sql_reset ();

    sql  = "select systransnum from tg_pos_cancel_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, branchCode); idx++;
    sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, logicalDate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, registerNumber); idx++;

    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  Determine the receipt type
        Json::Value jheader;
        //////////////////////////////////
        //  For each sales record
        do  {
            //////////////////////////////
            //  Retrieve
            jheader.clear();
            receipt->resetEodTotals();
            string systransnum = dbsel->sql_field_value("systransnum");

            if (!receipt->retrieveSalesHeader(jheader, systransnum, SRT_CANCEL))
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

            _liteDb->sql_reset();
            stringstream ss; ss << jheader;
            string sql = "insert into tg_pos_offline_sales (systransnum,type,sqldata,jsondata)values(";
            sql += _liteDb->sql_bind(1,systransnum);  sql += ",";
            sql += _liteDb->sql_bind(2,receipt->_exportType);  sql += ",";
            sql += _liteDb->sql_bind(3,receipt->_sqlSales);  sql += ",";
            sql += _liteDb->sql_bind(4,ss.str());  sql += ")";
            if (!_liteDb->sql_only(sql,true))
                return seterrormsg(_liteDb->errordb());
            if (!_liteDb->sql_commit())
                return seterrormsg(_liteDb->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }    
*/
    return true;
}
