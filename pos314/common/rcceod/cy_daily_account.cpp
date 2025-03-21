/********************************************************************
          FILE:          cy_daily_account.cpp
   DESCRIPTION:          Import account information
 ********************************************************************/
//////////////////////////////////////////
//  Main header
#include "cypostrx.h"
//////////////////////////////////////////
//  Dereference the STD namespace
using std::endl;
using std::string;
using std::vector;
using std::stringstream;
/*******************************************************************
      FUNCTION:         cy_daily_account
   DESCRIPTION:         Save the account details
 *******************************************************************/
bool                    cypostrx::cy_daily_account (CYDbSql* db)
{
    int idx = 1;
    string sql01, acct_type_code;
    //////////////////////////////////////
    //  Account flag(s) for later use
    _sclerk = "", _pshopper = "";
    _is_pwd="0", _is_senior="0", _is_ba="0";

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
                if (jacct["is_clerk"].asString()=="1")
                    _sclerk = jacct["customer_id"].asString();
                if (jacct["is_shopper"].asString()=="1")
                    _pshopper = jacct["customer_id"].asString();


                sql01  = "INSERT INTO ";  sql01 += _tblprefix; sql01 += "account (company_cd,branch_cd, ";
                sql01 += "register_num,sys_trans_num,company_name,acct_type_code, ";
                sql01 += "first_name, account_number, customer_id, mi, last_name, ";
                sql01 += "currency_cd, crm_acct_num, crm_first_name, crm_last_name) ";
                sql01 += "values (";
                sql01 += db->sql_bind(idx,_company_code); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,_branch_code); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,_register_num); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,_systransnum); sql01 += ", '', "; idx++;
                sql01 += db->sql_bind(idx,jacct["account_type_code"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["first_name"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["account_number"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,jacct["customer_id"].asString()); sql01 += ", '', "; idx++;
                sql01 += db->sql_bind(idx,jacct["last_name"].asString()); sql01 += ", "; idx++;
                sql01 += db->sql_bind(idx,_jcurrency["currency_code"].asString()); sql01 += ", "; idx++;
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
      FUNCTION:         cy_daily_account_discount
   DESCRIPTION:         Save the account discount details
 *******************************************************************/
bool                    cypostrx::cy_daily_account_discount (CYDbSql* db)
{
    //TODO:
    (void)db;
    return true;
}


/*******************************************************************
      FUNCTION:         pos_cashier_register
   DESCRIPTION:         Save the cashier session details
 *******************************************************************/
bool                    cypostrx::pos_cashier_register(  Json::Value jcashier, int nreset )
{
    int idx = 1,
        recordexist;
    string sql, strtemp;

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconnect();

    if(nreset){

        db->sql_reset();
        idx = 1;
        db->sql_bind_reset();
        sql  = " delete from pos_cashier_register where location_code=";
        sql += db->sql_bind(idx,jcashier["location_code"].asString()); idx++;
        sql += " and register_num=";
        sql += db->sql_bind(idx,jcashier["register_num"].asString()); idx++;
        sql += " and cashier=";
        sql += db->sql_bind(idx,jcashier["cashier"].asString()); idx++;

        if (!db->sql_only( (char*) sql.c_str(),true)){
            seterrormsg(db->errordb());
            return false;//seterrormsg(db->errordb());
        }

        if (!db->sql_commit()){
            seterrormsg(db->errordb());
            return false;//seterrormsg(db->errordb());
        }

        return true;
    }





    idx = 1;
    db->sql_bind_reset();

    sql  = "select count(*) as recs from pos_cashier_register where register_num=";
    sql += db->sql_bind(idx,jcashier["register_num"].asString()); idx++;
    sql += " and cashier =";
    sql += db->sql_bind(idx,jcashier["cashier"].asString());

    if (!db->sql_result (sql,1)){
            _util->jsonerror(db->errordb().c_str());
            return false;// seterrorquery(db->errordb());
    }

    strtemp     = db->sql_field_value("recs").c_str();
    recordexist =    atoi(strtemp.c_str());

    db->sql_reset();

   if(recordexist)
       return true;



    idx = 1;
    db->sql_bind_reset();
    sql  = "INSERT INTO pos_cashier_register (location_code, register_num, cashier) values (";

    sql += db->sql_bind(idx,jcashier["location_code"].asString()); sql+= ", "; idx++;
    sql += db->sql_bind(idx,jcashier["register_num"].asString()); sql += ", "; idx++;
    sql += db->sql_bind(idx,jcashier["cashier"].asString()); idx++;
    sql += ") ";
    if (!db->sql_only( (char*) sql.c_str(),true)){
        seterrormsg(db->errordb());
        return false;//seterrormsg(db->errordb());
    }

    if (!db->sql_commit()){
        seterrormsg(db->errordb());
        return false;//seterrormsg(db->errordb());
    }

    return true;
}


/*******************************************************************
      FUNCTION:         pos_zread
   DESCRIPTION:         Save the register eod status on server
 *******************************************************************/
bool                    cypostrx::pos_zread(  Json::Value jcashier, Json::Value jcashdec, Json::Value jcashtakeout)
{
    int idx = 1,
        recordexist;
    string sql, sql10, strtemp;
    Json::Value jcf;

    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconnect();
    idx = 1;


    sql = "select count(*) as recs from pos_cashier_xread where is_eod = 1 and location_code=";
    sql += db->sql_bind(idx,jcashier["location_code"].asString()); idx++;
    sql += " and register_num=";
    sql += db->sql_bind(idx,jcashier["register_num"].asString()); idx++;
    sql += " and transaction_date=";
    sql += db->sql_bind(idx,jcashier["transaction_date"].asString()); idx++;

    if (!db->sql_result (sql,1)){
        _util->jsonerror(db->errordb().c_str());
        return false;// seterrorquery(db->errordb());
    }

    strtemp     = db->sql_field_value("recs").c_str();
    recordexist =    atoi(strtemp.c_str());

    db->sql_reset();

    if(recordexist)  {
        return true;
    }





        db->sql_reset();
        idx = 1;
        db->sql_bind_reset();
        sql  = " update pos_cashier_xread set is_eod = 1 where is_eod = 0 and location_code=";
        sql += db->sql_bind(idx,jcashier["location_code"].asString()); idx++;
        sql += " and register_num=";
        sql += db->sql_bind(idx,jcashier["register_num"].asString()); idx++;
        sql += " and transaction_date=";
        sql += db->sql_bind(idx,jcashier["transaction_date"].asString()); idx++;

        if (!db->sql_only( sql,true)){
            seterrormsg(db->errordb());
            return false;//seterrormsg(db->errordb());
        }

        if (!db->sql_commit()){
            seterrormsg(db->errordb());
            return false;//seterrormsg(db->errordb());
        }



        ////////////////////////////////////////
        /// upload cash dec
        ///
        ///
        if (jcashdec.isArray())  {
            for (Json::Value::ArrayIndex i = 0; i != jcashdec.size(); i++)  {

                jcf = jcashdec[i];

                idx = 1;

                sql = " Insert into cy_cash_dec (company_code, location_code, transaction_date, register_num, currency_code, ";
                sql10  = db->sql_bind(idx, jcf["company_code"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["location_code"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["transaction_date"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["register_num"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["currency_code"].asString()); sql10 += ", "; idx++;


                sql += " tender_code,  cashier, actual_amount, cashier_shift, declared_amount, ";
                sql10  += db->sql_bind(idx, jcf["tender_code"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["cashier"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["actual_amount"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["cashier_shift"].asString()); sql10 += ", "; idx++;
                //sql10  += db->sql_bind(idx, jcf["nonsalestrxnum"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["declared_amount"].asString()); sql10 += ","; idx++;

                sql += " variance,  is_declared, conv_amount, conv_variance, conv_declared, is_sync) values (";
                sql10  += db->sql_bind(idx, jcf["variance"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["is_declared"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["conv_amount"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["conv_variance"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["conv_declared"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["is_sync"].asString()); sql10 += ")"; idx++;


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


        ////////////////////////////////////////
        /// upload pos_cash_takeout
        ///
        ///
        if (jcashtakeout.isArray())  {
            for (Json::Value::ArrayIndex i = 0; i != jcashtakeout.size(); i++)  {

                jcf = jcashtakeout[i];

                idx = 1;


                sql = " Insert into pos_cash_takeout (transaction_date, location_code, cashier, register_num, currency_code, ";
                sql10  = db->sql_bind(idx, jcf["transaction_date"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["location_code"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["cashier"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["register_num"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["currency_code"].asString()); sql10 += ", "; idx++;


                sql += " cashier_shift,  seqno, cash_takeout, is_sync, tender_code,";
                sql10  += db->sql_bind(idx, jcf["cashier_shift"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["seqno"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["cash_takeout"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["is_sync"].asString()); sql10 += ", "; idx++;
                //sql10  += db->sql_bind(idx, jcf["nonsalestrxnum"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["tender_code"].asString()); sql10 += ","; idx++;

                sql += " tender_desc,  denomination, qty, sys_trans_num) values (";
                sql10  += db->sql_bind(idx, jcf["tender_desc"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["denomination"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["qty"].asString()); sql10 += ", "; idx++;
                sql10  += db->sql_bind(idx, jcf["sys_trans_num"].asString()); sql10 += "); "; idx++;


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

        return true;



}
