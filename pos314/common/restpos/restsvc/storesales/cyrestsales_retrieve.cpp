//*******************************************************************
//        FILE:     cyrestsales_retrieve.cpp
// DESCRIPTION:     Sales record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "restsvc/cyposrest.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesRecord
// DESCRIPTION:     Retrieve the sales record
//*******************************************************************
bool CYRestCommon::retrieveSalesRecord(Json::Value& jroot,
                                       string systransnum,
                                       salesReceiptType type)
{
    //////////////////////////////////////
    //  Save the receipt data
    _receiptType = type;
    //////////////////////////////////////
    //  Reste the EOD total(s)
    resetEodTotals();
    //////////////////////////////////////
    //  Retrieve the header record
    _jheader.clear();
    if (!retrieveSalesHeader(_jheader, systransnum, type))  {
        jroot["status"]="error";
        jroot["errmsg"]=errormsg();
        return false;
    }
    if (!retrieveSalesHeaderAccount(_jheader))  {
        jroot["status"]="error";
        jroot["errmsg"]=errormsg();
        return false;
    }
    //////////////////////////////////////
    //  Retrieve the detail record
    _jdetail.clear();
    if (!retrieveSalesDetail(_jdetail,
                             _jheader["branch_code"].asString(),
                             systransnum))  {
        jroot["status"]="error";
        jroot["errmsg"]=errormsg();
        return false;
    }
    _jheader["det_sales"]=_jdetail;

    //////////////////////////////////////
    //  Retrieve the payment record
    _jpayment.clear();
    if (!retrieveSalesPayment(_jpayment, systransnum))  {
        jroot["status"]="error";
        jroot["errmsg"]=errormsg();
        return false;
    }
    _jheader["pay_sales"]=_jpayment;
    return true;
}
//*******************************************************************
//    FUNCTION:     assembleSqlInsert
// DESCRIPTION:     Generate a SQL insert statement
//*******************************************************************
bool CYRestCommon::assembleSqlInsert (CYDbSql* db,
                                      std::string tableName,
                                      std::string& sqlInsert)
{
    int idx = 1;
    string fieldName = "", fieldList = "",
           fieldValue = "", fieldType = "";

    sqlInsert  = "insert into "; sqlInsert += tableName; sqlInsert += "(";

    int y = db->numcols();
    for (int x = 0; x < y; x++)  {
        fieldName = db->sql_field_name(x);
        fieldList += fieldName;
        if ((x+1)<y)
            fieldList += ", ";
        else
            fieldList += ") values (";
        fieldType = db->sql_field_type(fieldName);
        if (fieldType == "string")  {
            fieldValue += "'";
            fieldValue += db->sql_field_value(fieldName);
            fieldValue += "'";
        }  else  {
            if (db->sql_field_value(fieldName).length() > 0)
                fieldValue += db->sql_field_value(fieldName);
            else
            fieldValue += "0";
        }
        if ((x+1)<y)
            fieldValue += ", ";
        else
            fieldValue += ");\n\n";
        idx++;
    }
    sqlInsert += fieldList; sqlInsert += fieldValue;
    //_log->logmsg("SQL INSERT",sqlInsert.c_str ());
    return true;
}
//*******************************************************************
//    FUNCTION:     retrieveFnbRecord
// DESCRIPTION:     Retrieve the FNB sales record
//*******************************************************************
bool CYRestCommon::retrieveFnbRecord(Json::Value& jroot,
                                     cyposrest* rest,
                                     string table_code,
                                     string customer_number,
                                     string register_number)
{
    (void)rest;
    (void)jroot;
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Check for an existing transaction
    dbsel->sql_reset();
    sql  = "select * from tg_table_customer where table_code = ";
    sql += dbsel->sql_bind(1, table_code);
    sql += " and customer_number = ";
    sql += dbsel->sql_bind(2, customer_number);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the customer detail(s)");
    string systransnum = dbsel->sql_field_value ("systransnum");
    string picture_url = dbsel->sql_field_value ("picture_link");

    string last_name = dbsel->sql_field_value ("last_name");
    string first_name = dbsel->sql_field_value ("first_name");
    string middle_name = dbsel->sql_field_value ("middle_name");

    string transaction_code = dbsel->sql_field_value("transaction_code");

    string customer_id = dbsel->sql_field_value ("customer_number");
    string account_number = dbsel->sql_field_value ("account_number");
    string account_type_code = dbsel->sql_field_value ("account_type_code");
    string account_type_desc = dbsel->sql_field_value ("account_type_desc");
    //////////////////////////////////////
    //  Get the branch settings
    dbsel->sql_reset();
    sql  = "select * from tg_branch_table where table_code = ";
    sql += dbsel->sql_bind(1, table_code);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the branch detail(s)");
    string branch_code = dbsel->sql_field_value ("branch_code");
    //////////////////////////////////////
    //  Get the company settings
    dbsel->sql_reset();
    sql  = "select * from cy_location where location_code = ";
    sql += dbsel->sql_bind(1, branch_code);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the company detail(s)");
    string company_code = dbsel->sql_field_value ("company_code");
    //////////////////////////////////////
    //  Get the logical date
    dbsel->sql_reset();
    sql  = "select * from pos_register where register_num = ";
    sql += dbsel->sql_bind(1, register_number);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the company detail(s)");
    string logical_date = dbsel->sql_field_value ("last_sysdate");
    //////////////////////////////////////
    //  Get the cashier / server
    dbsel->sql_reset();
    sql  = "select * from tg_table_customer_orders where table_code = ";
    sql += dbsel->sql_bind(1, table_code);
    sql += " and customer_number = ";
    sql += dbsel->sql_bind(2, customer_number);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the order detail(s)");
    string cashier = dbsel->sql_field_value ("server");
    //////////////////////////////////////
    //  Get the transaction type
    dbsel->sql_reset();
    sql  = "select * from pos_transtype where transaction_cd = ";
    sql += dbsel->sql_bind(1, transaction_code);

    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the order detail(s)");
    string is_sc = dbsel->sql_field_value ("is_sc");
    string min_amount = dbsel->sql_field_value ("min_amount");
    string max_amount = dbsel->sql_field_value ("max_amount");
    string is_default = dbsel->sql_field_value ("is_default");
    string is_internal = dbsel->sql_field_value ("is_internal");
    string discount_code = dbsel->sql_field_value ("discount_cd");
    string is_zero_rated = dbsel->sql_field_value ("is_zero_rated");
    string transaction_desc = dbsel->sql_field_value ("description");
    //////////////////////////////////////
    //  Get the default currency
    dbsel->sql_reset();
    sql  = "select * from cy_currency where is_default = 1 ";

    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof ())
        return seterrormsg("Unable to retrieve the currency detail(s)");
    string default_rate = dbsel->sql_field_value("default_rate");
    string currency_desc = dbsel->sql_field_value("description");
    string currency_code = dbsel->sql_field_value("currency_code");
    string is_multiply = dbsel->sql_field_value("default_action");
    //////////////////////////////////////
    //  Get the global discount
    string discount_type_code, discount_desc,
           discount_type, discount_value, is_global;

    if (discount_code.length() > 0)  {
        dbsel->sql_reset();
        sql  = "select * from pos_discount where discount_cd = ";
        sql += dbsel->sql_bind(1, discount_code);

        if (!dbsel->sql_result(sql,true))
            return seterrormsg(dbsel->errordb());
        if (dbsel->eof ())
            return seterrormsg("Unable to retrieve the transaction discount detail(s)");
        discount_type = dbsel->sql_field_value("disctype");
        discount_value = dbsel->sql_field_value("discvalue");
        discount_desc = dbsel->sql_field_value("description");
        discount_type_code = dbsel->sql_field_value("discount_type_cd");

        dbsel->sql_reset();
        sql  = "select * from pos_discount_type where discount_type_cd = ";
        sql += dbsel->sql_bind(1, discount_type_code);

        if (!dbsel->sql_result(sql,true))
            return seterrormsg(dbsel->errordb());
        if (dbsel->eof ())
            return seterrormsg("Unable to retrieve the transaction discount type detail(s)");
        is_global = dbsel->sql_field_value("is_global");
        if (is_global != "1")
            return seterrormsg("Invalid transaction discount type");
    }
    //////////////////////////////////////
    //  Other value(s)
    string transaction_date;
    _util->int_date(transaction_date,0);
    string transaction_time = _util->int_time();

    int idx = 1;
    string transnum = "";
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    db->sql_reset ();
    if (systransnum.length() < 1)  {
        transnum = _util->date_sequence();
        systransnum  = customer_number;
        systransnum += "-";
        systransnum += transnum;

        idx = 1;
        sql  = "insert into tg_fnb_header (systransnum, company_code, branch_code, ";
        sql += "register_number, transaction_number, transaction_date, transaction_time, ";
        sql += "logical_date, cashier, transaction_code, cashier_shift, picture_url, ";
        sql += "clerk, cancel_flag, is_polled, is_polled2, pac_number, ba_redemption_reference, ";
        sql += "is_beauty_addict, ridc_number) values (";
        sql += db->sql_bind (idx,systransnum); sql += ", "; idx++;
        sql += db->sql_bind (idx,company_code); sql += ", "; idx++;
        sql += db->sql_bind (idx,branch_code); sql += ", "; idx++;
        sql += db->sql_bind (idx,register_number); sql += ", "; idx++;
        sql += db->sql_bind (idx,transnum); sql += ", "; idx++;
        sql += db->sql_bind (idx,transaction_date); sql += ", "; idx++;
        sql += db->sql_bind (idx,transaction_time); sql += ", "; idx++;
        sql += db->sql_bind (idx,logical_date); sql += ", "; idx++;
        sql += db->sql_bind (idx,cashier); sql += ", "; idx++;
        sql += db->sql_bind (idx,transaction_code); sql += ", 1, "; idx++;
        sql += db->sql_bind (idx,picture_url); sql += ", "; idx++;
        sql += db->sql_bind (idx,cashier); sql += ", 0, 0, 0, '', '', 0, '')"; idx++;

        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());

        idx = 1;
        db->sql_bind_reset();
        sql  = "insert into tg_pos_mobile_currency (systransnum, currency_code, ";
        sql += "description, is_multiply, default_rate) values (";
        sql += db->sql_bind (idx,systransnum); sql += ", "; idx++;
        sql += db->sql_bind (idx,currency_code); sql += ", "; idx++;
        sql += db->sql_bind (idx,currency_desc); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_multiply); sql += ", "; idx++;
        sql += db->sql_bind (idx,default_rate); sql += ") "; idx++;

        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());

        idx = 1;
        db->sql_bind_reset();
        sql  = "insert into tg_pos_mobile_transtype (systransnum, transaction_code, ";
        sql += "description, is_default, is_internal, min_amount, max_amount, is_zero_rated) values (";
        sql += db->sql_bind (idx,systransnum); sql += ", "; idx++;
        sql += db->sql_bind (idx,transaction_code); sql += ", "; idx++;
        sql += db->sql_bind (idx,transaction_desc); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_default); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_internal); sql += ", "; idx++;
        sql += db->sql_bind (idx,min_amount); sql += ", "; idx++;
        sql += db->sql_bind (idx,max_amount); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_zero_rated); sql += ") "; idx++;

        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());

        if (discount_code.length() > 0)  {
            idx = 1;
            db->sql_bind_reset();
            sql  = "delete from tg_pos_mobile_discount where systransnum = ";
            sql += db->sql_bind(1, systransnum);

            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());

            idx = 1;
            db->sql_bind_reset();
            sql  = "insert into tg_pos_mobile_discount (systransnum, discount_code, ";
            sql += "description, discount_type, discount_value) values (";
            sql += db->sql_bind (idx,systransnum); sql += ", "; idx++;
            sql += db->sql_bind (idx,discount_code); sql += ", "; idx++;
            sql += db->sql_bind (idx,discount_desc); sql += ", "; idx++;
            sql += db->sql_bind (idx,discount_type); sql += ", "; idx++;
            sql += db->sql_bind (idx,discount_value); sql += ") "; idx++;

            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
        }

        idx = 1;
        db->sql_bind_reset();
        sql  = "insert into tg_pos_mobile_trxaccount (systransnum, customer_id, ";
        sql += "account_number, account_type_code, first_name, middle_name, ";
        sql += "last_name, points_earned, is_loyalty, is_clerk, points_balance, ";
        sql += "account_type_desc, is_zero_rated, is_senior, is_pwd, is_agent, ";
        sql += "is_shopper, is_beauty_addict, is_bridal) values (";
        sql += db->sql_bind (idx,systransnum); sql += ", "; idx++;
        sql += db->sql_bind (idx,customer_id); sql += ", "; idx++;
        sql += db->sql_bind (idx,account_number); sql += ", "; idx++;
        sql += db->sql_bind (idx,account_type_code); sql += ", "; idx++;
        sql += db->sql_bind (idx,first_name); sql += ", "; idx++;
        sql += db->sql_bind (idx,middle_name); sql += ", "; idx++;
        sql += db->sql_bind (idx,last_name); sql += ", 0, 0, 0, 0, "; idx++;
        sql += db->sql_bind (idx,account_type_desc); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_zero_rated); sql += ", "; idx++;
        sql += db->sql_bind (idx,is_sc); sql += ", 0, 0, 0, 0, 0) "; idx++;

        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());

        dbsel->sql_reset();
        sql  = "select * from tg_table_customer_orders where table_code = ";
        sql += dbsel->sql_bind(1, table_code);
        sql += " and customer_number = ";
        sql += dbsel->sql_bind(2, customer_number);

        if (!dbsel->sql_result(sql,true))
            return seterrormsg(dbsel->errordb());
        if (dbsel->eof ())
            return seterrormsg("Unable to retrieve the order detail(s)");

        do  {
            string item_seq = "";
            string consign_reference = "";
            string quantity = dbsel->sql_field_value("quantity");
            string item_code = dbsel->sql_field_value("recipe_item_code");
            string location_code = branch_code;
            string retail_price = dbsel->sql_field_value("price");

            _cgi->add_form("item_seq",item_seq);
            _cgi->add_form("quantity",quantity);
            _cgi->add_form("item_code",item_code);
            _cgi->add_form("retail_price",retail_price);
            _cgi->add_form("location_code",location_code);
            _cgi->add_form("consign_reference",consign_reference);

            //output = rest->rest_updatedetail ("tg_fnb_header");

            dbsel->sql_next();
        }  while (!dbsel->eof ());

        if (!db->sql_commit())
            return seterrormsg("Unable to save the transaction detail(s)");

    }
    return true;
}
