/********************************************************************
          FILE:          cy_daily_payment.cpp
   DESCRIPTION:          Import payment information
 ********************************************************************/
#include <unistd.h>
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
      FUNCTION:         cy_daily_payment
   DESCRIPTION:         Save the sales payment details
 *******************************************************************/
bool                    cypostrx::cy_daily_payment (CYDbSql* db)
{
    int idx = 1;
    db->sql_bind_reset();

    string sql07, sql08, tmp, sequence;
    Json::Value jtender;
    Json::Value jtendertype;
    Json::Value jbank;
    Json::Value jgc;


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
                sql07  = "INSERT INTO ";  sql07 += _tblprefix; sql07 += "payment (company_cd,branch_cd,register_num,sys_trans_num,";
                sql08  = db->sql_bind(idx,_company_code); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_branch_code); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_register_num); sql08 += ", "; idx++;
                sql08 += db->sql_bind(idx,_systransnum); sql08 += ", "; idx++;

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
