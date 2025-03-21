/********************************************************************
          FILE:          cy_daily_settings.cpp
   DESCRIPTION:          Import settings information
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
      FUNCTION:         cy_daily_settings
   DESCRIPTION:         Save the POS settings
 *******************************************************************/
bool                    cypostrx::cy_daily_settings (CYDbSql* db)
{
    int idx = 1;
    idx = 1;
    string sql03;
    db->sql_bind_reset();

    if (_jsettings.isMember("trade_name")&&_jbranch.isMember("location_code"))  {
        sql03  = "INSERT INTO ";  sql03 += _tblprefix; sql03 += "settings (company_cd,branch_cd, ";
        sql03 += "register_num,sys_trans_num,trade_name,company_name, ";
        sql03 += "branch_name,city,prov,zip,tin,bir,msg1,msg2,msg3,msg4,msg5, ";
        sql03 += "addr1,addr2,addr3,hdrmsg1,hdrmsg2,hdrmsg3,sd_branch_desc) values (";
        sql03 += db->sql_bind(idx,_company_code); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_branch_code); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_register_num); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_systransnum); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["trade_name"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_company_name); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_branch_name); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["city_code"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["state_code"].asString()); sql03 += ", '', "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["tin"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["bir_num"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["szmsg01"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["szmsg02"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["szmsg03"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["szmsg04"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jsettings["szmsg05"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["addr1"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["addr2"].asString()); sql03 += ", '', "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["hdrmsg1"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_jbranch["hdrmsg2"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx, _jbranch["hdrmsg3"].asString()); sql03 += ", "; idx++;
        sql03 += db->sql_bind(idx,_branch_code); sql03 += ") ";
        if (!db->sql_only((char*) sql03.c_str())){
            seterrormsg(db->errordb());
            return false; //seterrormsg(db->errordb());
        }
    }  else  {
        return false; //seterrormsg("No POS settings information found");
    }
    return true;
}




/*******************************************************************
      FUNCTION:         cy_daily_xread
   DESCRIPTION:         Save the POS xread
 *******************************************************************/
bool                    cypostrx::cy_daily_xread (CYDbSql* db)
{

    //////////////////////////////////////
    //  Retrieve all active promotions
    db->sql_reset();
    Json::Value jcf;
    string sql10,sql;
    int                 idx=1;

    sql  = "select   count(*) as recs from pos_cashier_xread where  location_code='";
    sql += _branch_code;
    sql += "' and transaction_date= ";
    sql += _jsales["logical_date"].asString();
    sql += " and     register_num= ";
    sql += _register_num;


    if(!db->sql_result(sql,false)){
        return false;
    }

    if (atoi(db->sql_field_value("recs", "0").c_str()) <=0){
        db->sql_reset();
        sql = "Insert into pos_cashier_xread (location_code, transaction_date, register_num, is_eod, is_sync,  ";
        sql10  = db->sql_bind(idx,_branch_code); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jsales["logical_date"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_register_num); sql10 += ", 0, 0"; idx++;


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
    //cash fund entry
    sql  = "select   count(*) as recs from pos_cash_fund where  location_code='";
    sql += _branch_code;
    sql += "' and transaction_date= ";
    sql += _jsales["logical_date"].asString();
    sql += " and     register_num= ";
    sql += _register_num;
    sql += " and     cashier= '";
    sql += _jsales["cashier"].asString();
    sql += "' and     cashier_shift= ";
    sql += _jsales["cashier_shift"].asString();


    if(!db->sql_result(sql,false)){
        return false;
    }

    if (atoi(db->sql_field_value("recs", "0").c_str()) <=0){

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

  return true;
}
