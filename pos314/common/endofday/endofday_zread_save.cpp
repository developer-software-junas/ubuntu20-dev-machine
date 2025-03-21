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
string                   cyposrest::rest_zread_save ()
{
    string sql;
    char szfunc [] = "zReadSave";
    //return errorjson("ZREAD DEBUG");
    //////////////////////////////////////
    //  Retrieve argument(s)
    string cashier = cgiform("manager");
    string manager = cgiform("manager");
    string mgrpass = "";
    string register_num = cgiform("register_num");
    string logical_date = cgiform("logical_date");
    string location_code = cgiform("location_code");
    string cashier_shift = "";
    string clear_suspended = "0";
    //////////////////////////////////////
    //  Assign to the EOD variable(s)
    _eod->argXread = false;
    _eod->argManager = manager;
    _eod->argCashier = cashier;
    _eod->argShift = cashier_shift;
    _eod->argRegister = register_num;
    _eod->argSysdate = logical_date;
    _eod->argBranch = location_code;
    _eod->argSuspended = clear_suspended;
    //////////////////////////////////////
    //  Retrieve EOD data
    if (!_eod->retrieveFastX())
        return errorjson(_eod->errormsg());

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    //////////////////////////////////////
    //  Sysdate...
    string sysdate = _eod->getAltValue("transaction_date",_eod->alt_settings_sysdate);
    string eod_timestamp = _eod->getAltValue("eod_timestamp",_eod->alt_settings_sysdate);

    if (eod_timestamp.length() > 0 && eod_timestamp != "0")
        return _util->jsonerror(szfunc,"End of day has already been completed for the business date requested.");
    if (sysdate != logical_date)  {
        _eod->argSysdate = sysdate;
        //string errtmp = "The business date requested for terminal reading is invalid. ";
        //errtmp += sysdate; errtmp += ", "; errtmp += logical_date;
        //return _util->jsonerror(szfunc,errtmp);
    }
    //////////////////////////////////////
    //  Make sure all cashiers are done
    string cashierList = "Not all cashiers have performed an X reading: ";
    dbsel->sql_reset();
    sql  = "select * from pos_cashier_xread_shift ";
    sql += "where  is_eod = 0 and transaction_date = ";
    sql += dbsel->sql_bind(1, sysdate);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(2, location_code);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind(3, register_num);
    sql += " and   register_num in (";
    sql += "           select z.register_num from pos_register z where z.ipad_alias = '0') ";

    if (!dbsel->sql_result (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    if (!dbsel->eof())  {
        int i = 0;
        Json::Value jline, jcashier;
        jcashier["status"]="error";
        do  {
            jline["cashier"]=dbsel->sql_field_value ("cashier");
            jline["register_num"]=dbsel->sql_field_value ("register_num");
            cashierList += dbsel->sql_field_value ("cashier");
            cashierList += ", ";
            jcashier["cashier"][i]=jline;
            i++;
            dbsel->sql_next();
        } while (!dbsel->eof ());
        jcashier["error_message"]=cashierList;
        stringstream ss;
        ss << jcashier;
        _log->logmsg("EODZ",ss.str().c_str());
        return ss.str();
    }
    //////////////////////////////////////
    //  Calculated value(s)
    char sztmp [64];
    int count = _eod->cntCash + _eod->cntNonCash;
    sprintf (sztmp, "%d", count);
    string transaction_count = sztmp;

    sprintf(sztmp,"%.02f",(_eod->totCash + _eod->totNonCash));
    string transaction_amount = sztmp;
    //////////////////////////////////////
    //  Update the cashier record(s)
    db->sql_reset ();
    sql  = "update pos_cashier_xread set is_eod = 1 ";
    sql += "where  transaction_date = ";
    sql += db->sql_bind(1, sysdate);
    sql += " and   location_code = ";
    sql += db->sql_bind(2, location_code);
    sql += " and   register_num = ";
    sql += db->sql_bind(3, register_num);
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    db->sql_bind_reset();
    sql  = "update pos_cashier_xread_shift set is_eod = 1 ";
    sql += "where  transaction_date = ";
    sql += db->sql_bind(1, sysdate);
    sql += " and   location_code = ";
    sql += db->sql_bind(2, location_code);
    sql += " and   register_num = ";
    sql += db->sql_bind(3, register_num);
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Update the sysdate
    eod_timestamp = _util->strtime_t();
    db->sql_bind_reset();
    sql  = "update pos_sysdate set is_sync = 1, eod_timestamp = ";
    sql += db->sql_bind(1, eod_timestamp);
    sql += ", transaction_count = ";
    sql += db->sql_bind( 2, transaction_count);
    sql += ", transaction_amount = ";
    sql += db->sql_bind( 3, transaction_amount);
    sql += " where transaction_date = ";
    sql += db->sql_bind(4, sysdate);
    sql += " and   location_code = ";
    sql += db->sql_bind(5, location_code);
    sql += " and   register_num = ";
    sql += db->sql_bind(6, register_num);
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Carry over the new total(s)
    if (!_util->int_date_add(sysdate,1))
        return _util->jsonerror(szfunc,"Unable to generate the next business date");

    //////////////////////////////////////
    //  Increment the register totals
    db->sql_bind_reset ();
    sql  = "update pos_register set zreadcount = zreadcount + 1, ";
    sql += " last_sysdate = ";
    sql += db->sql_bind( 1, sysdate);
    sql += ", transaction_count = transaction_count + ";
    sql += db->sql_bind( 2, transaction_count);
    sql += ", transaction_amount = transaction_amount + ";
    sql += db->sql_bind( 3, transaction_amount);
    sql += ", daily_transnum = 1 ";
    //sql += ", last_adjust_transnum = adjust_transnum ";
    //sql += ", last_adjust_amount = adjust_amount ";
    sql += " where location_code = ";
    sql += db->sql_bind( 4, location_code);
    sql += " and register_num = ";
    sql += db->sql_bind( 5, register_num);

    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Increment the sysdate
    db->sql_bind_reset();
    string rcpt = _ini->get_value("RECEIPT","TYPE");
    sql  = "insert into pos_sysdate (transaction_date, location_code, register_num, ";
    sql += "transaction_count, transaction_amount, is_sync, eod_timestamp) values (";
    sql += db->sql_bind(1, sysdate); sql += ", ";
    sql += db->sql_bind(2, location_code); sql += ", ";
    if (rcpt == "elena")  {
        sql += db->sql_bind(3, register_num); sql += ", 0, 0.00, 1414, 1414) ";
    }  else  {
        sql += db->sql_bind(3, register_num); sql += ", 0, 0.00, -888, 0) ";
    }

    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Cleanup table(s)
    db->sql_reset ();
    sql = "truncate table pos_cashier_register ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
/*
    sql = "truncate table pos_cash_fund ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table pos_cash_takeout ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table tg_pos_receipt ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
    sql = "truncate table tg_pos_journal ";
    if (!db->sql_only (sql,true))
        return _util->jsonerror(szfunc,db->errordb());
     ***/
    //////////////////////////////////////
    //  Delete suspended record(s)???
    //////////////////////////////////////
    //  Go through all sales record(s)
    int i, idx = 1;
    dbsel->sql_reset();
    db->sql_bind_reset ();

    sql  = "select systransnum from tg_pos_suspend_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, location_code); idx++;
    sql += " and  logical_date <= "; sql += dbsel->sql_bind (idx, sysdate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, register_num); idx++;

    if (!dbsel->sql_result (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  For each suspended record
        do  {
            //////////////////////////////
            //  Retrieve
            i = 0;
            string systransnum = db->sql_field_value("systransnum");
            while (g_sales_tables[i].tableName != "eof")  {
                db->sql_bind_reset ();
                string table = g_sales_tables[i].tableName;
                sql  = "delete from ";
                sql += table;
                sql += " where systransnum = ";
                sql += db->sql_bind (1, systransnum); idx++;
                if (!db->sql_only(sql,true))
                    return _util->jsonerror(szfunc,db->errordb());
                i++;
            }
            sql  = "delete from tg_pos_suspend_header ";
            sql += " where systransnum = ";
            sql += db->sql_bind (1, systransnum); idx++;
            if (!db->sql_only(sql,true))
                return _util->jsonerror(szfunc,db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    if (!db->sql_commit ())
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Reset the logfile(s)
    _cgi->cleanlogfile(true);
    //////////////////////////////////////
    //  Cleanup sales
    cleanupMobileSales (_eod->argBranch,_eod->argRegister,_eod->argSysdate,_eod->argCashier);
    string output = rest_zread();
    //////////////////////////////////////
    //  Compress / publish the sales file
    /*
    string zipFile = _ini->get_value("PATH","SALES");
    zipFile += "/"; zipFile += location_code;
    zipFile += "."; zipFile += logical_date;
    zipFile += ".sqlite.zip";

    string backupFile = _ini->get_value("PATH","SALES");
    backupFile += "/sales.sqlite";

    if (_util->file_exists(zipFile.c_str()))  {
        _util->file_delete(zipFile.c_str ());
    }
    string dumpCmd;
    dumpCmd  = "/usr/bin/zip -j ";
    dumpCmd += zipFile;
    dumpCmd += " ";
    dumpCmd += backupFile;
    dumpCmd += " > /tmp/zip.temp";

    int ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        return errorjson(_error);
    }
    backupFile += "/var/www/html/"; backupFile += location_code;
    backupFile += "."; backupFile += logical_date;
    backupFile += ".sqlite.zip";
    if (_util->file_exists(backupFile.c_str()))  {
        _util->file_delete(backupFile.c_str ());
    }

    dumpCmd  = "cp ";
    dumpCmd += zipFile;
    dumpCmd += " ";
    dumpCmd += "/var/www/html";

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        return errorjson(_error);
    }*/
    return output;
}
/*******************************************************************
      FUNCTION:          rest_zread_poll
   DESCRIPTION:          Move sales to RCC sales tables
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
string                   cyposrest::rest_zread_poll ()
{
    char szfunc [] = "zread_poll";
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    //////////////////////////////////////
    //  Cashier list
    string sql = "delete from pos_cashier_register ";
    if (!db->sql_only(sql.c_str(),false))
        return _util->jsonerror(szfunc,db->errordb());
    sql  = "insert into pos_cashier_register (location_code, register_num, cashier) ";
    sql += "select z.branch_code, z.register_number, z.cashier from tg_pos_daily_header z ";
    sql += "group by z.branch_code, z.register_number, z.cashier ";
    if (!db->sql_only(sql.c_str(),false))
        return _util->jsonerror(szfunc,db->errordb());
    if (!db->sql_commit())
        return _util->jsonerror(szfunc,db->errordb());
    //////////////////////////////////////
    //  Receipt object
    CYRestCommon rc (_log,_ini,_cgi);
    //////////////////////////////////////
    //  Process sales
    Json::Value jsales;
    dbsel->sql_reset ();
    sql  = "select systransnum, logical_date from tg_pos_daily_header where is_polled2 = 0 order by systransnum asc ";
    if(EQT_SUCCESS != json_select_multiple(dbsel, jsales, sql))
        return _util->jsonerror(szfunc,"No sales record(s) found");

    Json::Value jline;
    int j = jsales.size();
    for (int i = 0; i < j; i++)  {
        jline = jsales[i];
        Json::Value jheader;
        Json::Value jdetail;
        Json::Value jpayment;
        string systransnum = jline["systransnum"].asString();
        //////////////////////////////////
        //  Sales record
        if (!rc.retrieveSalesHeader(jheader, systransnum, SRT_DAILY))
            _util->jsonerror(rc.errormsg());
        if (!rc.retrieveSalesHeaderAccount(jheader))
            _util->jsonerror(rc.errormsg());
        if (!rc.retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
            _util->jsonerror(rc.errormsg());
        jheader["det_sales"]=jdetail;
        if (!rc.retrieveSalesPayment(jpayment, systransnum))
            _util->jsonerror(rc.errormsg());
        jheader["pay_sales"]=jpayment;
        //////////////////////////////////
        //  Cash fund
        int idx = 1;
        Json::Value jfund;
        dbsel->sql_reset();
        sql = " select * from pos_cash_fund where transaction_date =";
        sql += dbsel->sql_bind(idx, jheader["logical_date"].asString()); idx++;
        sql += " and register_num=";
        sql += dbsel->sql_bind(idx, jheader["register_num"].asString()); idx++;
        sql += " and cashier = ";
        sql += dbsel->sql_bind(idx, jheader["cashier"].asString()); idx++;
        sql += " and cashier_shift =";
        sql += dbsel->sql_bind(idx, jheader["cashier_shift"].asString()); idx++;
        if(EQT_SUCCESS != json_select_single(dbsel, jfund, sql))
            return _util->jsonerror(szfunc,"No sales record(s) found");
        //////////////////////////////////////
        //  Account information
        if (!cy_daily_account(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_header");
        //////////////////////////////////////
        //  Currency
        if (!cy_daily_currency(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_currency");
        //////////////////////////////////////
        //  POS settings
        if (!cy_daily_settings(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_settings");
        //////////////////////////////////////
        //  Sales detail
        if (!cy_daily_detail(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_detail");
        /////////////////////////////////////
        /// Trans discount
        if (!cy_daily_trxdiscount(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_discount");
        //////////////////////////////////////
        //  Sales payment
        if (!cy_daily_payment(db, jheader))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_payment");
        //////////////////////////////////////
        //  Sales header
        if (!cy_daily_header(db, jheader, 0, 0))
            return _util->jsonerror(szfunc,"Unable to import cy_daily_header");

        //////////////////////////////////////
        //  Save all changes
        if (!db->sql_commit ())
            return _util->jsonerror(szfunc,db->errordb());

        //////////////////////////////////////
        //  Sales xread and cash fund
        if (!cy_daily_xread(db, jheader))
            return _util->jsonerror(szfunc,db->errordb());
    }
    return _util->jsonok((""));
}
/*******************************************************************
      FUNCTION:         cy_daily_xread
   DESCRIPTION:         Save the xread details
 *******************************************************************/
bool                    cyposrest::cy_daily_xread (CYDbSql* db, Json::Value jheader)
{
    //////////////////////////////////////
    //  Retrieve all active promotions
    db->sql_reset();
    Json::Value jcf;
    string sql10,sql;
    int                 idx=1;

    sql  = "select   count(*) as recs from pos_cashier_xread where  location_code='";
    sql += jheader["branch_code"].asString();
    sql += "' and transaction_date= ";
    sql += jheader["logical_date"].asString();
    sql += " and     register_num= ";
    sql += jheader["register_number"].asString();


    if(!db->sql_result(sql,false)){
        return false;
    }
    string recs = db->sql_field_value("recs");
    if (atoi(recs.c_str()) <=0){
        db->sql_reset();
        sql = "Insert into pos_cashier_xread (location_code, transaction_date, register_num, is_eod, is_sync,  ";
        sql10  = db->sql_bind(idx,jheader["branch_code"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,jheader["logical_date"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,jheader["register_number"].asString()); sql10 += ", 0, 0"; idx++;


        sql   += "sales_adjustment, trans_nonvat_amount, trans_vatexempt_amt, transaction_amount, transaction_count, transaction_number, xread_count) values (  ";
        sql10 += ", 0 , 0, 0, 0, 0, 0, 0);";

        sql += sql10;


        if (!db->sql_only (sql, true))  {
            printf("ERROR: %s\n", db->errordb().c_str());
            return false ;//seterrormsg(db->errordb());
        }

        if (!db->sql_commit ())
            return false; //seterrormsg(db.errordb());
    }
    ////////////////////////////
    //  cash fund entry
    db->sql_reset();
    Json::Value _jcashfund;
    sql  = "select count(*) as recs from pos_cash_fund where  location_code='";
    sql += jheader["branch_code"].asString();
    sql += "' and transaction_date= ";
    sql += jheader["logical_date"].asString();
    sql += " and     register_num= ";
    sql += jheader["register_number"].asString();
    sql += " and     cashier= '";
    sql += jheader["cashier"].asString();
    sql += "' and     cashier_shift= ";
    sql += jheader["cashier_shift"].asString();
    if(!db->sql_result(sql,false)){
        return false;
    }
    recs = db->sql_field_value("recs");
    if (atoi(recs.c_str()) <=0) {
        db->sql_reset();
        Json::Value _jcashfund;
        sql  = "select * from pos_cash_fund where  location_code='";
        sql += jheader["branch_code"].asString();
        sql += "' and transaction_date= ";
        sql += jheader["logical_date"].asString();
        sql += " and     register_num= ";
        sql += jheader["register_number"].asString();
        sql += " and     cashier= '";
        sql += jheader["cashier"].asString();
        sql += "' and     cashier_shift= ";
        sql += jheader["cashier_shift"].asString();
        if(EQT_SUCCESS == json_select_multiple(db, _jcashfund, sql))  {
            if (atoi(recs.c_str()) <=0){

                if (_jcashfund.isArray())  {
                    for (Json::Value::ArrayIndex i = 0; i != _jcashfund.size(); i++)  {

                        jcf = _jcashfund[i];

                        idx = 1;

                        sql = " Insert into pos_cash_fund (transaction_date, location_code, register_num, cashier, cashier_shift, ";
                        sql10  = db->sql_bind(idx, jcf["transaction_date"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["location_code"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["register_num"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["cashier"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["cashier_shift"].asString()); sql10 += ", "; idx++;


                        sql += " currency_code,  tender_code, tender_desc, cash_fund_amt, is_sync) values (";
                        sql10  += db->sql_bind(idx, jcf["currency_code"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["tender_code"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["tender_desc"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["cash_fund_amt"].asString()); sql10 += ", "; idx++;
                        //sql10  += db->sql_bind(idx, jcf["nonsalestrxnum"].asString()); sql10 += ", "; idx++;
                        sql10  += db->sql_bind(idx, jcf["is_sync"].asString()); sql10 += ");"; idx++;

                        sql += sql10;
                        db->sql_reset();
                        if(!db->sql_only(sql,true)){
                            printf("ERROR: %s\n", db->errordb().c_str());
                            return false;
                        }

                        if (!db->sql_commit ())
                            return false; //seterrormsg(db.errordb());
                    }
                }
           }
       }
   }

   return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_payment
   DESCRIPTION:         Save the payment details
 *******************************************************************/
bool                    cyposrest::cy_daily_payment (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    db->sql_bind_reset();

    string sql07, sql08, tmp, sequence;
    Json::Value jtender;
    Json::Value jtendertype;
    Json::Value jbank;
    Json::Value jgc;

    Json::Value _jcurrency = Json::nullValue;
    if (jheader["hdr_currency"]==Json::nullValue)
        return true;
    _jcurrency = jheader["hdr_currency"];

    Json::Value _jpayment = Json::nullValue;
    if (jheader["pay_sales"]==Json::nullValue)
        return true;
    _jpayment = jheader["pay_sales"];

    if (_jpayment.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != _jpayment.size(); i++)  {
            idx = 1;

            Json::Value jpay = _jpayment[i];
            sequence = _util->longtostring(i+1);

            jtender.clear ();
            if (jpay.isMember("tender"))
                jtender = jpay["tender"];

            jtendertype.clear ();
            if (jpay.isMember("tendertype"))
                jtendertype = jpay["tendertype"];

            jbank.clear ();
            if (jpay.isMember("bank"))
                jbank = jpay["bank"];

            jgc.clear ();
            if (jpay.isMember("gc"))
                jgc = jpay["gc"];



            if (jpay.isMember("pay_code"))  {
                sql07  = "INSERT INTO cy_daily_payment (company_cd,branch_cd,register_num,sys_trans_num,";
                sql08  = db->sql_bind(idx,jheader["company_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jheader["branch_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jheader["register_number"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jheader["systransnum"].asString()); sql08 += ", "; idx++;

                sql07 += "pay_seq,alt_tender_code,tender_type_code,tender_desc,tender_code,";
                //sql08 += db->sql_bind(idx,jpay["pay_seq"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,sequence); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtender["mms_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["pay_type_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtender["description"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["pay_code"].asString()); sql08 += ", "; idx++;

                sql07 += "is_change,is_manual,is_validate,is_default,is_refund_type,";
                tmp = jpay["change_amount"].asString();
                if (tmp.length () < 1)  tmp = "0";
                if (atof (tmp.c_str()) > 0)
                    sql08 += "1,0,0,0,0,";
                else
                    sql08 += "0,0,0,0,0,";

                sql07 += "is_validation_space,is_display_total,min_amount,max_amount,max_change,";
                sql08 += "0,0,";
                sql08 += db->sql_bind(idx,jtender["min_amount"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtender["max_amount"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtender["max_change"].asString()); sql08 += ", "; idx++;

                sql07 += "max_refund,currency_cd,currency_action,currency_rate,currency_local_rate,";
                sql08 += db->sql_bind(idx,jtender["max_refund"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["currency_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["is_multiply"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["default_rate"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["default_rate"].asString()); sql08 += ", "; idx++;


                sql07 += "change_cd,change_action,change_rate,change_local_rate,pay_amount,change_amount,";
                sql08 += db->sql_bind(idx,_jcurrency["currency_code"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["is_multiply"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["default_rate"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_jcurrency["default_rate"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["change_amount"].asString()); sql08 += ", "; idx++;

                sql07 += "gc_change_amount,pay_amount_due,promo_type,promo_calc,promo_code,promo_desc,";
                if (jtendertype["is_gc"].asString() == "1")  {

                    if (jpay["gc"].isArray ())  {
                        sql08 += "0,";
                        // TODO:  add the GC logic
                    }else
                        sql08 += "0,"; //????????????


                }  else  {
                    sql08 += "0,";
                }

                sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", "; idx++;
                sql08 += "0,0,'','',"; //???


                sql07 += "promo_min,promo_max,promo_amt,promo_value,conv_pay_amount,conv_change_amount,";
                sql08 += "0,0,0,0,";
                sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["change_amount"].asString()); sql08 += ", "; idx++;

                sql07 += "conv_gc_change_amount,conv_pay_amount_due,is_expire,is_gc,is_check,is_charge,is_epurse,";
                sql08 += "0,";
                sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", "; idx++;
                sql08 += "0,";
                sql08 += db->sql_bind(idx,jtendertype["is_gc"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtendertype["is_check"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtendertype["is_charge"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtendertype["is_epurse"].asString()); sql08 += ", "; idx++;


                sql07 += "is_debitcard,is_creditcard,amt_limit,amt_balance,amt_original,first_name,mi,last_name,";
                sql08 += db->sql_bind(idx,jtendertype["is_debit_card"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtendertype["is_credit_card"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["credit_limit"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["credit_balance"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["credit_limit"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["first_name"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["middle_name"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jpay["last_name"].asString()); sql08 += ", "; idx++;



                sql07 += "ref_num,";

                if(atof(jtendertype["is_debit_card"].asString().c_str()) == 1 || atof(jtendertype["is_credit_card"].asString().c_str()) == 1   ){
                    sql08 += db->sql_bind(idx,"1"); sql08 += " ,"; idx++;
                }else if (atof(jtendertype["is_gc"].asString().c_str()) == 1){


                    if (jgc.isArray())  {
                        for (Json::Value::ArrayIndex i = 0; i != jgc.size(); i++)  {
                            Json::Value  gc = jgc[i];

                            sql08 += db->sql_bind(idx,gc["gcnum"].asString()); sql08 += " ,"; idx++; //GCREFNUM from gc

                        }
                    } else {
                        sql08 += db->sql_bind(idx,""); sql08 += " ,"; idx++; //GCREFNUM from gc
                    }

                }else if(atof(jtendertype["is_charge"].asString().c_str()) == 1    ){
                      sql08 += db->sql_bind(idx,jpay["approval_code"].asString()); sql08 += ", "; idx++; // refnum on charge

                }else if(atof(jtendertype["is_check"].asString().c_str()) == 1    ){
                    sql08 += db->sql_bind(idx,jpay["change_currency_code"].asString()); sql08 += ", "; idx++; // refnum on charge

                }else{
                    sql08 += db->sql_bind(idx,""); sql08 += " ,"; idx++; //GCRefnum blank

                }



                sql07 += "terminal,approval,approved_by,acct_type_code,acct_num,company,bank_code,c3_resptxt,";
                sql08 += db->sql_bind(idx,jbank["terminal_number"].asString()); sql08 += ", "; idx++;


                if(atof(jtendertype["is_debit_card"].asString().c_str()) == 1 || atof(jtendertype["is_credit_card"].asString().c_str()) == 1   ){
                    sql08 += db->sql_bind(idx,jpay["approval_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["approval_merchant"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_type_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_number"].asString()); sql08 += ", '', "; idx++; //company
                    sql08 += db->sql_bind(idx,jbank["bank_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jbank["payment_terms"].asString()); sql08 += ", "; idx++;
                }else if(atof(jtendertype["is_charge"].asString().c_str()) == 1    ){
                    sql08 += db->sql_bind(idx,jpay["approval_merchant"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,""); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_type_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_number"].asString()); sql08 += ", '', "; idx++; //company
                    sql08 += db->sql_bind(idx,jbank["bank_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jbank["payment_terms"].asString()); sql08 += ", "; idx++;
                } else{

                    sql08 += db->sql_bind(idx,jpay["approval_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["approval_merchant"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_type_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jpay["account_number"].asString()); sql08 += ", '', "; idx++; //company
                    sql08 += db->sql_bind(idx,jbank["bank_code"].asString()); sql08 += ", "; idx++;
                    sql08 += db->sql_bind(idx,jbank["payment_terms"].asString()); sql08 += ", "; idx++;
                }




                sql07 += "date_paid,is_deposit,amount_tendered,conv_tendered,new_payment,is_cash,";
                sql08 += "0,0,0,0,1,";
                //sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", "; idx++;
                //sql08 += db->sql_bind(idx,jpay["payment_amount"].asString()); sql08 += ", 1, "; idx++;
                sql08 += db->sql_bind(idx,jtendertype["is_cash"].asString()); sql08 += ", "; idx++;

                sql07 += "discount_acct_type_code,discount_customer_id,is_garbage,is_declare,auth_manager,";
                sql08 += "'',0,0,0,'',";


                sql07 += "is_cashfund,is_takeout) VALUES (";
                sql08 += db->sql_bind(idx,jtender["is_cashfund"].asString()); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,jtender["is_takeout"].asString()); sql08 += ") "; idx++;

                sql07 += sql08;
                if (!db->sql_only((char*) sql07.c_str())){
                    seterrormsg(db->errordb());
                    return false;//seterrormsg(db->errordb());
                }
            }
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_header
   DESCRIPTION:         Save the currency details
 *******************************************************************/
bool                    cyposrest::cy_daily_header (CYDbSql* db, Json::Value jheader,
                                                    int nvoid, int nrefund)
{
    int idx = 1;
    string sql09, sql10, tmp;

    Json::Value _jtrx;
    if (jheader["hdr_trx"]==Json::nullValue)
        return true;
    _jtrx = jheader["hdr_trx"];

    Json::Value _jregister;
    if (jheader["settings_register"]==Json::nullValue)
        return true;
    _jregister = jheader["settings_register"];

    db->sql_bind_reset();
    sql09  = "INSERT INTO cy_daily_header (company_cd,branch_cd,register_num,trans_num, sys_trans_num,";
    sql10  = db->sql_bind(idx,jheader["company_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["branch_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["register_number"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["transaction_number"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["systransnum"].asString()); sql10 += ", "; idx++;

    sql09 += "trans_date,trans_time,is_void,logical_date,is_cancel,is_suspend,is_layaway,";
    sql10 += db->sql_bind(idx,_jsales["transaction_date"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["transaction_time"].asString()); sql10 += ", "; idx++;

    if(nvoid > 0)
       sql10 += "1";
    else
       sql10 += "0";

    sql10 +=" , ";
    sql10 += db->sql_bind(idx,_jsales["logical_date"].asString()); sql10 += ", 0, 0, 0,"; idx++;

    sql09 += "cashier,is_settled,trans_cd,cashier_shift,trans_alt_code,trans_desc,";
    sql10 += db->sql_bind(idx,_jsales["cashier"].asString()); sql10 += ", 0, "; idx++;
    sql10 += db->sql_bind(idx,_jsales["transaction_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["cashier_shift"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["transaction_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jtrx["description"].asString()); sql10 += ", "; idx++;

    sql09 += "trans_flag_return,trans_flag_layaway,pos_serial_num,trans_flag_cancel,trans_flag_deposit,trans_flag_account,";

    if(nrefund > 0)
        sql10 += "1";
    else
        sql10 += "0";

    sql10 += ",0,";
    sql10 += db->sql_bind(idx,_jregister["serialno"].asString()); sql10 += ", "; idx++;
    //sql10 += ""; sql10 += ", "; idx++;
    sql10 += "0,0,0,";

    sql09 += "trans_min,trans_max,trans_cancel,trans_deposit,lay_stamp,is_sync,";
    sql10 += db->sql_bind(idx,_jtrx["min_amount"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jtrx["max_amount"].asString()); sql10 += ", "; idx++;
    sql10 += "0,0,0,0,";

    sql09 += "auth_manager,is_zero_rated,trans_flag_zero_rated,sclerk,pshopper,is_pac,trans_flag_pac,ridc, ";

    //////////////////////////////
    //voided and refund transaction
    if(nvoid > 0 || nrefund >0 ){
        sql10 += db->sql_bind(idx,_jtrx["cashier"].asString()); sql10 += ", "; idx++;

    }else
        sql10 += "'0',";

    sql10 += db->sql_bind(idx,_jtrx["is_zero_rated"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jtrx["is_zero_rated"].asString()); sql10 += ", '', '', "; idx++;


    string is_pac = "0";
    tmp = _jsales["pac_number"].asString();
    if (tmp.length() > 1)
        is_pac = "1";
    if (tmp == "0")
        is_pac = "0";
    sql10 += db->sql_bind(idx,is_pac); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx, is_pac); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["ridc_number"].asString()); sql10 += ", "; idx++;

    sql09 += "acct_type_code,account_number,cur_points,prev_points,cur_epurse,prev_epurse,ref_trans_num,first_name,middle_initial,last_name,  ";

    if (_jfsp.isMember("account_type_code")    )  {

        //sql10 += "'','',0,0,0,0,'','','','' ";


        char szprevpoints [24];
        double dbal = atof (_jfsp["points_balance"].asString().c_str ());
        double dearned = atof (_jfsp["points_earned"].asString().c_str ());
        double dprevpoints = dbal -  dearned;
        sprintf (szprevpoints, "%.02f", dprevpoints);

        sql10 += db->sql_bind(idx,_jfsp["account_type_code"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jfsp["account_number"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,szprevpoints); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jfsp["points_balance"].asString()); sql10 += ", "; idx++;
        //  TODO:  cur ep, prev ep

        double    epbal= atof (_jfsp["epurse_balance"].asString().c_str ());;
        double    epspent = atof (_jfsp["epurse_spent"].asString().c_str ()); ;
        double    prev_ep = epbal - epspent;
        sprintf (szprevpoints, "%.02f", prev_ep);

        sql10 += db->sql_bind(idx,_jfsp["epurse_balance"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,szprevpoints); sql10 += ", "; idx++;
        sql10 += "'',";
        sql10 += db->sql_bind(idx,_jfsp["first_name"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jfsp["middle_name"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jfsp["last_name"].asString()); sql10 += ", "; idx++;


    }  else  {
        sql10 += "'','',0,0,0,0,'','','','', ";
    }

    sql09 += "baccount_number, bfirst_name, bmiddle_initial, blast_name,bcur_points, bprev_points)  ";
    sql10 += "'','','','',0,0 ) ";


    sql09 += " values (";
    sql09 += sql10;
    if (!db->sql_only ((char*) sql09.c_str(), true ))  {
        seterrormsg(db->errordb());
        return  false;//seterrormsg(db->errordb());
    }


    ///////////////////////
    //Update pos register for non void transaction
    if(nvoid <=0 ){
        idx=1;
        db->sql_bind_reset();
        sql09 = "Update pos_register set transaction_number = ";
        sql09 += db->sql_bind(idx,_jsales["transaction_number"].asString()); idx++;
        sql09 += " where register_num = ";
        sql09 += jheader["register_number"].asString();
        if (!db->sql_only ((char*) sql09.c_str()))  {
            seterrormsg(db->errordb());
            return false ;//seterrormsg(db->errordb());
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_account
   DESCRIPTION:         Save the account details
 *******************************************************************/
bool                    cyposrest::cy_daily_account (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    string sql01, acct_type_code;
    Json::Value _jtrxaccount = Json::nullValue;
    if (jheader["hdr_trxaccount"]==Json::nullValue)
        return true;

    _jfsp = Json::nullValue;
    _jtrxaccount = jheader["hdr_trxaccount"];
    //////////////////////////////////////
    //  Account flag(s) for later use
    if (_jtrxaccount.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != _jtrxaccount.size(); i++)  {
            idx = 1;
            db->sql_bind_reset();
            Json::Value jacct = _jtrxaccount[i];
            if (jacct.isMember("customer_id"))  {
                acct_type_code = jacct["account_type_code"].asString();
                if (acct_type_code == "BA") _is_ba = "1";
                if (acct_type_code == "PWD") _is_pwd = "1";
                if (acct_type_code == "SC") _is_senior = "1";
                if (acct_type_code == "FSP") _jfsp = jacct;
                if (acct_type_code == "FSPP") _jfsp = jacct;
                if (jacct["is_clerk"].asString()=="1")
                    _sclerk = jacct["customer_id"].asString();
                if (jacct["is_shopper"].asString()=="1")
                    _pshopper = jacct["customer_id"].asString();


                sql01  = "INSERT INTO cy_daily_account (company_cd,branch_cd, ";
                sql01 += "register_num,sys_trans_num,company_name,acct_type_code, ";
                sql01 += "first_name, account_number, customer_id, mi, last_name, ";
                sql01 += "currency_cd, crm_acct_num, crm_first_name, crm_last_name) ";
                sql01 += "values (";
                sql01 += db->sql_bind(idx,jheader["company_code"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jheader["branch_code"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jheader["register_number"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jheader["systransnum"].asString()); sql01 += ", '', "; idx++;
                sql01 += db->sql_bind(idx,jacct["account_type_code"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["first_name"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["account_number"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["customer_id"].asString()); sql01 += ", '', "; idx++;
                sql01 += db->sql_bind(idx,jacct["last_name"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,"PHP"); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx, jacct["account_number"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["first_name"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["last_name"].asString()); sql01 += ") ";
                if (!db->sql_only( (char*) sql01.c_str(),true)){
                    seterrormsg(db->errordb());
                    return false;//seterrormsg(db->errordb());
                }
            }
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_currency
   DESCRIPTION:         Save the currency details
 *******************************************************************/
bool                    cyposrest::cy_daily_currency (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    string sql02;
    Json::Value jcurrency = Json::nullValue;
    if (jheader["hdr_currency"]==Json::nullValue)
        return true;
    jcurrency = jheader["hdr_currency"];
    //////////////////////////////////////
    db->sql_bind_reset();
    sql02  = "INSERT INTO cy_daily_currency (company_cd,branch_cd, ";
    sql02 += "register_num,sys_trans_num,currency_cd,currency_desc, ";
    sql02 += "is_multiply, ex_rate, local_ex_rate) values (";
    sql02 += db->sql_bind(idx,jheader["company_code"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jheader["branch_code"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jheader["register_number"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jheader["company_code"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jcurrency["currency_code"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jcurrency["description"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jcurrency["is_multiply"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,jcurrency["default_rate"].asString()); sql02 += ", "; idx++;
    sql02 += db->sql_bind(idx,"0"); sql02 += ") ";
    if (!db->sql_only( (char*) sql02.c_str(),true)){
        seterrormsg(db->errordb());
        return false;//seterrormsg(db->errordb());
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_settings
   DESCRIPTION:         Save the currency details
 *******************************************************************/
bool                    cyposrest::cy_daily_settings (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    string sql03;

    Json::Value jsettings = Json::nullValue;
    if (jheader["hdr_settings"]==Json::nullValue)
        return true;
    jsettings = jheader["hdr_settings"];

    Json::Value jbranch = Json::nullValue;
    if (jheader["hdr_branch"]==Json::nullValue)
        return true;
    jbranch = jheader["hdr_branch"];

    Json::Value jcompany = Json::nullValue;
    if (jheader["hdr_company"]==Json::nullValue)
        return true;
    jcompany = jheader["hdr_company"];
    //////////////////////////////////////
    db->sql_bind_reset();
    sql03  = "INSERT INTO cy_daily_settings (company_cd,branch_cd, ";
    sql03 += "register_num,sys_trans_num,trade_name,company_name, ";
    sql03 += "branch_name,city,prov,zip,tin,bir,msg1,msg2,msg3,msg4,msg5, ";
    sql03 += "addr1,addr2,addr3,hdrmsg1,hdrmsg2,hdrmsg3,sd_branch_desc) values (";
    sql03 += db->sql_bind(idx,jheader["company_code"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jheader["branch_code"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jheader["register_number"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jheader["systransnum"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["trade_name"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jcompany["description"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["description"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["city_code"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["state_code"].asString()); sql03 += ", '', "; idx++;
    sql03 += db->sql_bind(idx,jbranch["tin"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["bir_num"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["szmsg01"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["szmsg02"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["szmsg03"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["szmsg04"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jsettings["szmsg05"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["addr1"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["addr2"].asString()); sql03 += ", '', "; idx++;
    sql03 += db->sql_bind(idx,jbranch["hdrmsg1"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["hdrmsg2"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jbranch["hdrmsg3"].asString()); sql03 += ", "; idx++;
    sql03 += db->sql_bind(idx,jheader["branch_code"].asString()); sql03 += ") ";
    if (!db->sql_only((char*) sql03.c_str())){
        seterrormsg(db->errordb());
        return false; //seterrormsg(db->errordb());
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_detail
   DESCRIPTION:         Save the sales detail
 *******************************************************************/
bool                    cyposrest::cy_daily_detail (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    string sql03;
    bool is_giftwrap;
    bool is_discounted;

    Json::Value _jtrx;
    Json::Value _jdettax;
    Json::Value _jdetdiscount;

    if (jheader["hdr_trx"]==Json::nullValue)
        return true;
    _jtrx = jheader["hdr_trx"];

    Json::Value jsettings = Json::nullValue;
    if (jheader["hdr_settings"]==Json::nullValue)
        return true;
    jsettings = jheader["hdr_settings"];

    Json::Value jbranch = Json::nullValue;
    if (jheader["hdr_branch"]==Json::nullValue)
        return true;
    jbranch = jheader["hdr_branch"];

    Json::Value jcompany = Json::nullValue;
    if (jheader["hdr_company"]==Json::nullValue)
        return true;
    jcompany = jheader["hdr_company"];

    Json::Value _jdetail = Json::nullValue;
    if (jheader["det_sales"]==Json::nullValue)
        return true;
    _jdetail = jheader["det_sales"];


    double    dretail,
              ddiscperqty;

    string sql04, sql05, sql06, sequence;

    if (_jdetail.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != _jdetail.size(); i++)  {
            idx = 1;
            dretail=0;
            ddiscperqty=0;
            is_discounted=false;

            sequence = _util->longtostring(i+1);

            Json::Value jdet = _jdetail[i];
            db->sql_bind_reset();
            is_giftwrap = false;
            if (jdet.isMember("gift_wrap_quantity"))  {
                sql06 = jdet["gift_wrap_quantity"].asString ();
                is_giftwrap = (_util->stodsafe(sql06) > 0);
            }
            if (jdet.isMember("item_code"))  {
                sql04  = "INSERT INTO cy_daily_detail (company_cd,branch_cd,register_num,sys_trans_num, ";
                sql05  = db->sql_bind(idx,jheader["company_code"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jheader["branch_code"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jheader["register_number"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jheader["systransnum"].asString()); sql05 += ", "; idx++;


                Json::Value jprod, jprodhdr;
                if (jdet.isMember("det_product"))  {
                    jprod.clear ();
                    jprod = jdet["det_product"];
                    jprodhdr.clear();
                    jprodhdr=jdet["detail_product_header"];
                }  else  {
                    return false; //seterrormsg(db->errordb());
                }
                sql04 += "item_code,seq_num,upc_uom,item_description,category_cd,subcat_cd,class_cd,subclass_cd, ";
                sql05 += db->sql_bind(idx,jdet["item_code"].asString()); sql05 += ", "; idx++;

                //sql05 += db->sql_bind(idx,jdet["item_seq"].asString ()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,sequence); sql05 += ", "; idx++;

                sql05 += db->sql_bind(idx, ""); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jdet["description"].asString ()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jprodhdr["category_cd"].asString ()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jprodhdr["subcat_cd"].asString ()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jprodhdr["class_cd"].asString ()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jprodhdr["subclass_cd"].asString ()); sql05 += ", "; idx++;


                sql04 += "base_type_code,clerk,sell_uom,cost,qty,upc_qty,orig_retail,is_vat,is_scanned, ";
                sql05 += db->sql_bind(idx,jprodhdr["base_type_code"].asString ()); sql05 += ", '', "; idx++;
                sql05 += db->sql_bind(idx,jdet["uom_code"].asString ()); sql05 += ", 0.00, "; idx++;
                sql05 += db->sql_bind(idx,jdet["quantity"].asString ()); sql05 += ", 1.00, "; idx++;
                sql05 += db->sql_bind(idx,jdet["retail_price"].asString ()); sql05 += ", "; idx++;

                dretail = atof(jdet["retail_price"].asString ().c_str());


                sql05 += db->sql_bind(idx,jdet["is_vat"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;

                sql05 += db->sql_bind(idx,jdet["is_scanned"].asString ()); sql05 += ", "; idx++;

                sql04 += "is_lookup,is_manual,is_local,is_allow_discount, vat_code,is_price_prompt,is_own_line, ";
                sql05 += db->sql_bind(idx,jdet["is_scanned"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jdet["is_scanned"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,"1"); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jdet["xitem"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;

                _jdettax.clear ();
                if (jdet.isMember("det_tax"))
                    _jdettax = jdet["det_tax"];
                if (_jdettax.isMember("tax_code"))  {
                    sql05 += db->sql_bind(idx,_jdettax["tax_code"].asString()); sql05 += ", "; idx++;
                }  else  {
                    //_errorMessage  = "No tax code found for the item: ";
                    //_errorMessage += jdet["item_code"].asString ();
                    return false;
                }

                sql05 += db->sql_bind(idx,jdet["is_price_prompt"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,jdet["is_individual_line"].asString()=="0" ? "1" : "0"); sql05 += ", "; idx++;



                sql04 += "tax1,tax2,  promo_type,  promo_calc,  promo_code,  promo_desc,  promo_min, promo_max, promo_amt,promo_value,retail,  ";
                sql05 += "'', '', ";

                //line discount
                is_discounted = false;
                _jdetdiscount.clear ();
                if (jdet.isMember("det_discount"))  {
                    is_discounted = true;
                    _jdetdiscount = jdet["det_discount"];
                }

                if (_jdetdiscount.isMember("discount_code"))  {
                    is_discounted = true;
                    //break total discount per qty
                    ddiscperqty = atof(jdet["discount_amount"].asString().c_str()) / atof(jdet["quantity"].asString().c_str()) ;
                    sql05 += db->sql_bind(idx,_jdetdiscount["discount_type"].asString()); sql05 += ", "; idx++;
                    sql05 += db->sql_bind(idx,_util->doubletostring(ddiscperqty)); sql05 += ", "; idx++;
                    sql05 += db->sql_bind(idx,_jdetdiscount["discount_code"].asString()); sql05 += ", "; idx++;
                    sql05 += db->sql_bind(idx,_jdetdiscount["description"].asString()); sql05 += ", 0.00, 0, "; idx++;



                    sql05 += db->sql_bind(idx,_util->doubletostring(ddiscperqty)); sql05 += ", "; idx++;

                    sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;

                    dretail = atof(jdet["retail_price"].asString ().c_str());
                    dretail -= ddiscperqty;
                    sql05 += db->sql_bind(idx,_util->doubletostring(dretail)); sql05 += ", "; idx++;  //retail
                }  else
                    is_discounted = false;

                //////////////
                /// check for global discount
                /// promo_type,  promo_calc,  promo_code,  promo_desc,
                if(_jsales.isMember("header_transtype_discount")){
                    Json::Value jglobdiscount = _jsales["header_transtype_discount"];
                    if (jglobdiscount.isMember("discount_code"))  {
                        is_discounted = true;
                        sql05 += "0, 0,";


                        if(atof(jdet["override_price"].asString ().c_str() )> 0){
                            sql05 += db->sql_bind(idx,"RPCN"); sql05 += ", "; idx++;
                            sql05 += db->sql_bind(idx,"MANUAL PRICE"); sql05 += ", "; idx++;

                        }else{
                            sql05 += "'', '', ";

                        }

                        sql05 += " 0.00, 0.00, 0.00,";





                        //sql05 += db->sql_bind(idx,jglobdiscount["discount_type"].asString()); sql05 += ", "; idx++;
                       // sql05 += db->sql_bind(idx,jglobdiscount["discount_value"].asString()); sql05 += ", "; idx++;
                        //sql05 += db->sql_bind(idx,jglobdiscount["discount_code"].asString()); sql05 += ", "; idx++;
                        //sql05 += db->sql_bind(idx,jglobdiscount["description"].asString()); sql05 += ", 0.01, 0, "; idx++;
                        //sql05 += db->sql_bind(idx,jdet["trx_discount_amount"].asString()); sql05 += ", "; idx++;
                        sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;

                        dretail = atof(jdet["retail_price"].asString ().c_str());
                        //dretail -= atof(jdet["trx_discount_amount"].asString().c_str());
                        sql05 += db->sql_bind(idx,_util->doubletostring(dretail)); sql05 += ", "; idx++;  //retail
                        dretail -= atof(jdet["trx_discount_amount"].asString().c_str());
                    }

                }

                if(!is_discounted)  {

                    if(atof(jdet["override_price"].asString ().c_str() )> 0){
                        sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;
                        sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;
                        sql05 += db->sql_bind(idx,"RPCN"); sql05 += ", "; idx++;
                        sql05 += db->sql_bind(idx,"MANUAL PRICE"); sql05 += ", 0.00, 0, "; idx++;
                        sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;
                        sql05 += db->sql_bind(idx,"0"); sql05 += ", "; idx++;

                        dretail = atof(jdet["retail_price"].asString ().c_str());
                        //dretail -= atof(jdet["discount_amount"].asString().c_str());
                        sql05 += db->sql_bind(idx,_util->doubletostring(dretail)); sql05 += ", "; idx++;  //retail


                    }else{

                       sql05 += "0, 0, '', '', 0.00, 0.00, 0, 0, ";
                       sql05 += db->sql_bind(idx,jdet["retail_price"].asString ()); sql05 += ", "; idx++;
                    }
                }


                sql04 += "return_company_cd,return_branch_cd,return_register_num,return_sys_trans_num,return_seq,return_qty,return_amount,";
                sql05 += "'', '', 0, '', 0, 0, 0.000000,";


                sql04 += "cust_acct_type,cust_number,cust_company,cust_first,cust_mi,cust_last,cust_location,cust_currency,";
                sql05 += "'', '', '', '', '', '', '', '',";

                sql04 += "tax_cd,tax_desc,tax_min,tax_max,tax_value,tax_pct,";
                sql05 += db->sql_bind(idx,_jdettax["tax_code"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_jdettax["description"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_jdettax["min_amount"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_jdettax["max_amount"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_jdettax["tax_value"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_jdettax["is_percentage"].asString()); sql05 += ", "; idx++;


                sql04 += "promo_auth,is_vat_zero,net_retail,alt_category_cd,";
                sql05 += "'',";
                sql05 += db->sql_bind(idx,_jtrx["is_zero_rated"].asString()); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_util->doubletostring(dretail)); sql05 += ", "; idx++;  //net retail
                sql05 += "'',";






                sql04 += "acct_promo_code,pay_promo_code,acct_promo_amt,pay_promo_amt,pay_promo_acctnum,acct_promo_acctnum,";
                sql05 += "'','',0,0.000,'','',";


                sql04 += "price_overide,";

                if(atof(jdet["override_price"].asString ().c_str()) > 0){
                    sql05 += db->sql_bind(idx,jdet["override_price"].asString ()); sql05 += ", "; idx++;
                }
                else
                    sql05 += "0,";




                sql04 += "upc_code,is_promo_event,business_rule_id,version_id,group_id,hierarchy_type_id,attribute_id, ";
                sql05 += "'',0.0000,'','','','','',";

                sql04 += "orig_zero_rated,cust_tin,is_sc,is_pwd,is_svc,is_ba) values (";
                sql05 += "0.00000,'',";

                if(_is_senior.length() <= 0)
                    _is_senior = "0";
                else
                    _is_senior = "1";

                sql05 +=db->sql_bind(idx, _is_senior); sql05 += ","; idx++;
                //sql05 += db->sql_bind(idx,_is_senior == "0" ? "1" : "0"); sql05 += ", "; idx++;




                sql05 += db->sql_bind(idx,_is_pwd); sql05 += ",0,"; idx++;
                sql05 += db->sql_bind(idx,_is_ba); sql05 += ") "; idx++;
                sql04 += sql05;

                if (!db->sql_only((char*) sql04.c_str(), true)){
                    seterrormsg(db->errordb());
                    return false; //seterrormsg(db->errordb());
                }

                if (is_discounted)  {
                    idx = 1;
                    db->sql_bind_reset();
                    sql06  = "INSERT INTO cy_daily_detail_discount (company_code,branch_code,";
                    sql06 += "register_num,trans_num,trans_date,sys_trans_num,trans_time,item_code,";
                    sql06 += "seq_num,discount_cd,description,discount_type,discount_value) values (";
                    sql06 += db->sql_bind(idx,jheader["company_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["branch_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["register_number"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jsales["transaction_number"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jsales["transaction_date"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["systransnum"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jsales["transaction_time"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jdet["item_code"].asString()); sql06 += ", "; idx++;
                    //sql06 += db->sql_bind(idx,jdet["item_seq"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,sequence); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jdetdiscount["discount_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jdetdiscount["description"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jdetdiscount["discount_type"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jdetdiscount["discount_value"].asString()); sql06 += ") "; idx++;

                    if (!db->sql_only((char*) sql06.c_str(), true)){
                        seterrormsg(db->errordb());
                        return false; //seterrormsg(db->errordb());
                    }
                }
                if (is_giftwrap)  {
                    idx = 1;
                    db->sql_bind_reset();
                    sql06  = "INSERT INTO cy_daily_giftwrap(company_cd,branch_cd,register_num,";
                    sql06 += "gw_register_num,sys_trans_num,item_code,description,qty,seq_num,";
                    sql06 += "category_cd) values (";
                    sql06 += db->sql_bind(idx,jheader["company_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["branch_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["register_number"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["register_number"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jheader["systransnum"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jdet["item_code"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jprod["description"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jdet["gift_wrap_quantity"].asString()); sql06 += ", "; idx++;
                    //sql06 += db->sql_bind(idx,jdet["item_seq"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,sequence); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,jprod["category_cd"].asString()); sql06 += ") "; idx++;

                    if (!db->sql_only((char*) sql06.c_str(),true )){
                        seterrormsg(db->errordb());
                        return false ;//seterrormsg(db->errordb());
                    }
                }

                ////////////////
                /// track transaction discount amount
                if(!jdet["trx_discount_amount"].empty() )
                    _transdiscamt += (_util->stodsafe(jdet["trx_discount_amount"].asString())  * atof(jdet["quantity"].asString().c_str())) ;




            }
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:         cy_daily_currency
   DESCRIPTION:         Save the currency details
 *******************************************************************/
bool                    cyposrest::cy_daily_trxdiscount (CYDbSql* db, Json::Value jheader)
{
    int idx = 1;
    string sql02;

    Json::Value _jtrx = Json::nullValue;
    if (jheader["hdr_trx"]==Json::nullValue)
        return true;
    _jtrx = jheader["hdr_trx"];

    Json::Value jcurrency = Json::nullValue;
    if (jheader["hdr_currency"]==Json::nullValue)
        return true;
    jcurrency = jheader["hdr_currency"];

    Json::Value _jtrxdiscount = Json::nullValue;
    if (jheader["header_transtype_discount"]==Json::nullValue)
        return true;
    _jtrxdiscount = jheader["header_transtype_discount"];

    Json::Value _jtrxaccount = Json::nullValue;
    if (jheader["hdr_trxaccount"]==Json::nullValue)
        return true;
    _jtrxaccount = jheader["hdr_trxaccount"];

    Json::Value _settingstransdisc = Json::nullValue;
    if (jheader["settings_transaction_discount"]==Json::nullValue)
        return true;
    _settingstransdisc = jheader["settings_transaction_discount"];
    idx = 1;
    //db->sql_bind_reset();
    string sql09, sql10, tmp;

    db->sql_bind_reset();
    sql09  = "INSERT INTO cy_daily_discount (company_cd,branch_cd,register_num, sys_trans_num,discount_type_cd, discount_cd,";
    sql10  = db->sql_bind(idx,jheader["company_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["branch_code"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["register_number"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,jheader["systransnum"].asString()); sql10 += ", "; idx++;

    //////////////////////////////
    //no data source after this line yet
    sql10 += db->sql_bind(idx,_settingstransdisc["discount_type_cd"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["discount_cd"].asString()); sql10 += ", "; idx++;




    sql09 += "description, is_percentage, discount_value, discount_amount,";
    sql10 += db->sql_bind(idx,_settingstransdisc["description"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["disctype"].asString()=="0"? "1" :"0"); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["discvalue"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jtrxdiscount["discount_value"].asString()); sql10 += ", "; idx++;


    sql09 += "is_readonly, min_amount, max_amount, start_date, start_time, ";

    sql10 += db->sql_bind(idx,_settingstransdisc["readonly"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["min_amount"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["max_amount"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["start_date"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["start_time"].asString() ); sql10 += ", "; idx++;




    sql09 += "end_date, end_time, total_discount,  ";
    sql10 += db->sql_bind(idx,_settingstransdisc["end_date"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_settingstransdisc["end_time"].asString() ); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_util->doubletostring(_transdiscamt)); sql10 += ", "; idx++;


    sql09 += "cust_acct_type, cust_number, cust_company, cust_first, cust_mi, cust_last, cust_location, cust_currency, ";

    if(!_jtrxaccount.empty()){

        sql10 += db->sql_bind(idx,_jsales["cust_acct_type"].asString()); sql10 += ",  "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_number"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_company"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_first"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jtrx["cust_mi"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_last"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["cust_location"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jtrx["cust_currency"].asString()); sql10 += ", "; idx++;

    }else{
        sql10 += db->sql_bind(idx,""); sql10 += ",  "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;

    }




    sql09 += " auth_manager, tin) ";
    sql10 += db->sql_bind(idx,""); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,""); sql10 += "); "; idx++;



    sql09 += " values (";
    sql09 += sql10;
    if (!db->sql_only ((char*) sql09.c_str(), true))  {
        return  false ;//seterrormsg(db->errordb());
    }

    return true;
}
