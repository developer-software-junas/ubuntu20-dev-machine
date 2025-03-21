/********************************************************************
          FILE:          cy_daily_detail.cpp
   DESCRIPTION:          Import sales detail information
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
      FUNCTION:         cy_daily_detail
   DESCRIPTION:         Save the sales details
 *******************************************************************/
bool                    cypostrx::cy_daily_detail (CYDbSql* db)
{
    int idx = 1;
    bool is_giftwrap;
    bool is_discounted;
    _transdiscamt  = 0;

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
                sql04  = "INSERT INTO ";  sql04 += _tblprefix; sql04 += "detail (company_cd,branch_cd,register_num,sys_trans_num, ";
                sql05  = db->sql_bind(idx,_company_code); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_branch_code); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_register_num); sql05 += ", "; idx++;
                sql05 += db->sql_bind(idx,_systransnum); sql05 += ", "; idx++;


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
                    sql06  = "INSERT INTO ";  sql06 += _tblprefix; sql06 += "detail_discount (company_code,branch_code,";
                    sql06 += "register_num,trans_num,trans_date,sys_trans_num,trans_time,item_code,";
                    sql06 += "seq_num,discount_cd,description,discount_type,discount_value) values (";
                    sql06 += db->sql_bind(idx,_company_code); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_branch_code); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_register_num); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jsales["transaction_number"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_jsales["transaction_date"].asString()); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_systransnum); sql06 += ", "; idx++;
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
                    sql06  = "INSERT INTO ";  sql06 += _tblprefix; sql06 += "giftwrap(company_cd,branch_cd,register_num,";
                    sql06 += "gw_register_num,sys_trans_num,item_code,description,qty,seq_num,";
                    sql06 += "category_cd) values (";
                    sql06 += db->sql_bind(idx,_company_code); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_branch_code); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_register_num); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_register_num); sql06 += ", "; idx++;
                    sql06 += db->sql_bind(idx,_systransnum); sql06 += ", "; idx++;
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
