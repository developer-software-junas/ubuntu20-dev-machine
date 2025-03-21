/********************************************************************
          FILE:          cy_daily_header.cpp
   DESCRIPTION:          Import sales information
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
      FUNCTION:         cy_daily_header
   DESCRIPTION:         Save the sales header
 *******************************************************************/
bool                    cypostrx::cy_daily_header (CYDbSql* db, int nvoid, int nrefund)
{
    int idx = 1;

    string sql09, sql10, tmp;

    db->sql_bind_reset();
    sql09  = "INSERT INTO ";  sql09 += _tblprefix; sql09 += "header (company_cd,branch_cd,register_num,trans_num, sys_trans_num,";
    sql10  = db->sql_bind(idx,_company_code); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_branch_code); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_register_num); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsales["transaction_number"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_systransnum); sql10 += ", "; idx++;

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
    sql10 += db->sql_bind(idx,_jtrx["is_zero_rated"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_jsclerk["account_number"].asString()); sql10 += ", "; idx++;
    sql10 += db->sql_bind(idx,_pshopper); sql10 += ", "; idx++;


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

    if (_jba.isMember("account_type_code")    )  {

        sql10 += db->sql_bind(idx,_jba["account_number"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jba["first_name"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jba["middle_name"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,_jba["last_name"].asString()); sql10 += ") "; idx++;

        char szprevpoints [24];
        double dbal = atof (_jba["points_balance"].asString().c_str ());
        double dearned = atof (_jba["points_earned"].asString().c_str ());
        double dcurpoints = dbal -  dearned;
        sprintf (szprevpoints, "%.02f", dcurpoints);


        sql10 += db->sql_bind(idx,_jfsp["points_balance"].asString()); sql10 += ", "; idx++;
        sql10 += db->sql_bind(idx,szprevpoints); sql10 += ") "; idx++;

    }else{

        sql10 += "'','','','',0,0 ) ";

    }









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
        sql09 += _register_num;
        if (!db->sql_only ((char*) sql09.c_str()))  {
            seterrormsg(db->errordb());
            return false ;//seterrormsg(db->errordb());
        }
    }
    //////////////////////////////////////
    //  If suspend, add the PAC entries
    /*
    if (_importType == eit_suspend)  {
        db->sql_bind_reset ();
        string sql  = "insert into rcc.cy_suspend_pac (branch_cd, sys_trans_num, pac_number, is_released, in_use) values (";
        sql += db->sql_bind (1, _branch_code); sql += ", ";
        sql += db->sql_bind (2, _systransnum); sql += ", ";
        sql += db->sql_bind (3, _suspendnum); sql += ", 1, 0) ";
        if (!db->sql_only (sql, true))  {
            return seterrormsg(db->errordb());
        }
    }*/
    return true;
}
