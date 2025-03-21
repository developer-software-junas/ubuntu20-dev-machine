//*******************************************************************
//        FILE:     cyrestsales_receipt_rcc_eod.cpp
// DESCRIPTION:     Sales receipt generation X AND Z RECEIPT
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;

//*******************************************************************
//    FUNCTION:     assembleReceipt
// DESCRIPTION:     Create JSON for xread
//*******************************************************************
bool CYRestReceiptRCCEod::retrieveZread (Json::Value& jheader,
                                         std::string logical_date,
                                         std::string location_code,
                                         std::string register_num,
                                         std::string manager,
                                         std::string mgrpass)
{
    (void)mgrpass;
    char szFunc [] = "zRead";
    //////////////////////////////////////
    //  Database connection
    CYDbSql* dbsel = dbconn ();
    if (!dbsel->ok()){
        _error = dbsel->errordb();
        return false;
    }

    CYDbSql* dbsel2 = dbconn ();
    if (!dbsel2->ok()){
        _error = dbsel2->errordb();
        return false;
    }

    if (logical_date.length() < 1)  {
        _error = "Invalid business date";
        return false;
    }
    dbsel->sql_reset();
    string sql, hold_register="", register_alias = "0";
    sql  = " select register_alias from pos_register where register_num = ";
    sql += dbsel->sql_bind (1, register_num);
    sql += " and    location_code = ";
    sql += dbsel->sql_bind (2, location_code);
    if (!dbsel->sql_result (sql,true)){
        _error =  dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ()) {
        register_alias = dbsel->sql_field_value("register_alias");
        if (register_alias.length() > 0 && register_alias != "0")  {
            hold_register = register_num;
            register_num = register_alias;
        }
    }

    ///////////////////////////////////////////////////////
    //  zread json
    ///////////////////////////////////////////////////////
    CYDbSql* dbeod = dbconn ();
    if (!dbeod->ok()){
        _error = _util->jsonerror(szFunc,_env->errordbenv());
        return false;
    }
    //////////////////////////
    //  Get the manager name
    dbsel->sql_reset();
    //if (manager.length() < 1)  manager = cashier;
    sql  = "select * from cy_user ";
    sql += "where  login = ";
    sql += dbsel->sql_bind(1, manager);

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }

    string authorized_by  = dbsel->sql_field_value("first_name");
    authorized_by += " ";
    authorized_by += dbsel->sql_field_value("last_name");
    authorized_by += " (";
    authorized_by += dbsel->sql_field_value("login");
    authorized_by += ")";
    //////////////////////////////////////
    //  cash declaration
    int idx = 0;
    Json::Value jline, jcashdec;

    dbsel->sql_reset ();
    sql  = "select   a.declared_amount, b.description, c.is_cash  ";
    sql += "from     cy_cash_dec a, pos_tender b, pos_tender_type c ";
    sql += "where    a.tender_code = b.tender_cd ";
    sql += " and     b.tender_type_cd = c.tender_type_cd ";
    sql += " and     a.register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     a.transaction_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     a.location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " order by b.description ";

    if (!dbsel->sql_result (sql,true)){
        _error = sql.c_str();
        return false;
    }
    if (dbsel->eof ())
        jcashdec = Json::nullValue;
    else  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jcashdec[idx]=jline;
            idx++;
            dbsel->sql_next ();
        }  while(!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Cashier from / to
    //Json::Value root;
    //  replace with the transaction number

    dbeod->sql_reset ();
    sql = "select * from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(3,register_num);
    sql += " order by start_time desc limit 1";
    if (!dbeod->sql_result(sql,true)){
        _error =  dbsel->errordb();
        return false;
    }

    if (dbeod->eof ()){
        _error = "Unable to retrieve the EOD totals";
        return false;
    }

    jheader["start_num"]=dbeod->sql_field_value("start_trxnum");
    jheader["start_date"]=dbeod->sql_field_value("start_date");
    jheader["start_time"]=dbeod->sql_field_value("start_time");
    jheader["start_trans"]=dbeod->sql_field_value("start_trxnum");
    jheader["start_balance"]=dbeod->sql_field_value("start_balance");
    jheader["start_adjust_balance"]=dbeod->sql_field_value("start_adjust_balance");
    jheader["start_trxadjust"]=dbeod->sql_field_value("start_trxadjust");

    dbeod->sql_reset ();
    sql = "select * from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(3,register_num);
    sql += " order by start_time asc limit 1";
    if (!dbeod->sql_result(sql,true)){
        _error =  dbsel->errordb();
        return false;
    }

    if (dbeod->eof ()){
        _error = "Unable to retrieve the EOD totals";
        return false;
    }

    jheader["end_num"]=dbeod->sql_field_value("end_trxnum");
    jheader["end_date"]=dbeod->sql_field_value("end_date");
    jheader["end_time"]=dbeod->sql_field_value("end_time");
    jheader["end_trans"]=dbeod->sql_field_value("end_trxnum");
    jheader["end_balance"]=dbeod->sql_field_value("end_balance");
    jheader["end_trxadjust"]=dbeod->sql_field_value("end_trxadjust");
    jheader["end_adjust_balance"]=dbeod->sql_field_value("end_adjust_balance");

    //////////////////////////////////////
    //  Payment summary
    idx = 0;
    int idx2 = 0;
    Json::Value jline2;
    Json::Value jpaytype;
    Json::Value jpay;

    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, ";
    sql += "         tender_type_code as pay_type_code,tender_type_desc as pay_type_desc ";
    sql += "from     tg_pos_xread_payment ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by tender_type_code,tender_type_desc  ";
    sql += "order by tender_type_code,tender_type_desc ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            string ptc = dbsel->sql_field_value("pay_type_code");

            jpay.clear ();
            dbsel2->sql_reset ();
            sql  = "select   count(*) as pay_count, sum(amount) as payamt, ";
            sql += "         a.tender_type_code as pay_type_code, a.tender_code as pay_code, ";
            sql += "         a.tender_type_desc as pay_type_desc, a.tender_desc as pay_desc ";
            sql += "from     tg_pos_xread_payment a ";
            sql += " where     a.register_num = ";
            sql += dbsel2->sql_bind(1, register_num);
            sql += " and     a.logical_date = ";
            sql += dbsel2->sql_bind(2, logical_date);
            sql += " and     a.location_code = ";
            sql += dbsel2->sql_bind(3, location_code);
            sql += " and     a.tender_type_code = ";
            sql += dbsel2->sql_bind(4, ptc);
            sql += " group by a.tender_type_code, a.tender_type_desc, a.tender_code, a.tender_desc  ";
            sql += "order by tender_desc ";

            if (!dbsel2->sql_result (sql,true)){
                _error = dbsel2->errordb();
                return false;
            }
            if (!dbsel2->eof ())  {
                idx2 = 0;
                jpay.clear ();
                do  {
                    jline2.clear ();
                    for (int x = 0; x < dbsel2->numcols(); x++)
                        jline2[dbsel2->sql_field_name(x)]=dbsel2->sql_field_value (dbsel2->sql_field_name(x));
                    jpay[idx2]=jline2;
                    idx2++;
                    dbsel2->sql_next ();
                } while(!dbsel2->eof());
                jline["payment"]=jpay;
            }
            jpaytype[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  Bank summary
    idx = 0;
    Json::Value jbank;

    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, bank_desc as bankdesc ";
    sql += "from     tg_pos_xread_bank ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by bank_desc order by bank_desc ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jbank[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  cash summary
    Json::Value jcash;

    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, cash as is_cash ";
    sql += "from     tg_pos_xread_payment ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " and cash = 1 group by cash  ";
    sql += "order by cash ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jcash[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  non cash summary
    Json::Value jnoncash;

    dbsel->sql_reset ();
    dbsel->sql_reset ();
    sql  = "select   count(*) as pay_count, sum(amount) as payamt, cash as is_cash ";
    sql += "from     tg_pos_xread_payment ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " and cash <> 1 group by cash  ";
    sql += "order by cash ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jnoncash[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  Tax summary
    Json::Value jtax;

    dbsel->sql_reset ();
    sql  = "select   SUM(net_vat) as netvat, SUM(net_zero) as netzero, ";
    sql += "         SUM(net_exempt) as netexempt, SUM(amt_vat) as amtvat, ";
    sql += "         SUM(amt_zero) as amtzero, SUM(amt_exempt) as amtexempt, ";
    sql += "         SUM(net_amount) as salesnet, SUM(gross_amount) as salesgross, ";
    sql += "         SUM(service_charge) as servicecharge ";
    sql += "from     tg_pos_xread_tax ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by register_num ";

    if (!dbsel->sql_result (sql,true)){
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jtax[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    //////////////////////////////////////
    //  Total item(s)
    dbsel->sql_reset ();
    sql  = "select   SUM(quantity) as item_count from tg_pos_mobile_detail ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_daily_header z ";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,false)){
        _error = dbsel->errordb();
        return false;
    }
    string item_count = dbsel->sql_field_value("item_count");
    jtax["item_count"]=item_count;
    //////////////////////////////////////
    //  discount summary
    Json::Value jdiscount;

    dbsel->sql_reset ();
    sql  = "select   count(*) as discount_count, sum(amount) as discount_amount, ";
    sql += "         discount_desc as description,discount_code as discount_cd ";
    sql += "from     tg_pos_xread_discount ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and     logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and     location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by discount_desc,discount_code order by discount_desc ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    idx = 0;
    while (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jdiscount[idx]=jline;
        idx++;
        dbsel->sql_next ();
    }
    //////////////////////////////////////
    //  Cash fund summary
    idx = 0;
    Json::Value jfund;

    dbsel->sql_reset ();

    sql  = "select   count(*) as fund_count, sum(cash_fund_amt) as fundamt, ";
    sql += "         tender_code, tender_desc ";
    sql += "from     pos_cash_fund ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by tender_code, tender_desc ";
    sql += "order by tender_code, tender_desc ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jfund[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    //////////////////////////////////////
    //  Cash takeout summary
    idx = 0;
    Json::Value jtakeout;

    dbsel->sql_reset ();

    sql  = "select   count(*) as takeout_count, sum(denomination * qty) as takeoutamt, ";
    sql += "         tender_code, tender_desc ";
    sql += "from     pos_cash_takeout ";
    sql += " where     register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " group by tender_code, tender_desc ";
    sql += "order by tender_code, tender_desc ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            jline.clear ();
            for (int x = 0; x < dbsel->numcols(); x++)
                jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
            jtakeout[idx]=jline;
            idx++;
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }  else  {
        dbsel->sql_reset();
        sql  = "select   '1' as takeout_count, '0' as takeoutamt, ";
        sql += "         'CP' as tender_code, 'CASH PESO' as tender_desc ";

        if (!dbsel->sql_result (sql,true))
        {
            _error = dbsel->errordb();
            return false;
        }

        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jtakeout[0]=jline;
    }
    //////////////////////////////////////
    //  Voided transaction(s)
    dbeod->sql_reset ();
    sql = "select sum(audit_trx_void) as void_count, ";
    sql += "      sum(audit_trx_void_amount) as void_total,sum(audit_trx_nonsales) as nonsalestxn ";
    sql += "from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date = ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(3,register_num);
    sql += " order by start_time asc limit 1";
    if (!dbeod->sql_result(sql,true)){
        _error =  dbsel->errordb();
        return false;
    }

    if (dbeod->eof ()){
        _error = "Unable to retrieve the EOD totals";
        return false;
    }

    string void_count = dbeod->sql_field_value("void_count");
    string void_amount = dbeod->sql_field_value("void_total");
    //////////////////////////////////////
    //  NONE sales transaction(s)
    jtax["nonsalestxn"]=dbeod->sql_field_value("nonsalestxn");


    //////////////////////////////////////
    //  OLD ACCUMULATED
    sql = "select sum(end_balance) as oldgt ";
    sql += "from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date < ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(3,register_num);
    sql += " order by start_time asc limit 1";
    if (!dbeod->sql_result(sql,true)){
        _error =  dbsel->errordb();
        return false;
    }

    if (dbeod->eof ()){
        _error = "Unable to retrieve the EOD totals";
        return false;
    }

    jheader["oldgt"]=dbeod->sql_field_value("oldgt");
    //////////////////////////////////////
    //  NEW ACCUMULATED
    sql = "select sum(end_balance) as newgt ";
    sql += "from tg_pos_xread_cashier ";
    sql += "where location_code = ";
    sql += dbeod->sql_bind(1,location_code);
    sql += " and logical_date <= ";
    sql += dbeod->sql_bind(2,logical_date);
    sql += " and register_num = ";
    sql += dbeod->sql_bind(3,register_num);
    sql += " order by start_time asc limit 1";
    if (!dbeod->sql_result(sql,true)){
        _error =  dbsel->errordb();
        return false;
    }

    if (dbeod->eof ()){
        _error = "Unable to retrieve the EOD totals";
        return false;
    }

    jheader["newgt"]=dbeod->sql_field_value("newgt");
    //////////////////////////////////////
    //  Suspended item count
    dbsel->sql_reset ();
    sql  = "select   SUM(quantity) as suspend_item_count from tg_pos_mobile_detail ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_suspend_header z ";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string suspend_item_count = dbsel->sql_field_value("suspend_item_count");
    //////////////////////////////////////
    //  Suspend count
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_suspend_header z ";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string suspend_count = dbsel->sql_field_value("recs");
    //////////////////////////////////////
    //  Cancel count
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_cancel_header z ";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string cancel_count = dbsel->sql_field_value("recs");

    //////////////////////////////////////
    //  Total transaction(s)
    dbsel->sql_reset ();
    sql  = "select   count(*) as recs from tg_pos_daily_header z ";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);

    if (!dbsel->sql_result (sql,false))
    {
        _error = dbsel->errordb();
        return false;
    }
    string sales_count = dbsel->sql_field_value("recs");
    //////////////////////////////////////
    //  Refund count
    dbsel->sql_reset ();
    sql  = "select   count(*) as refund_count from tg_pos_refund_header ";
    sql += " where     register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(3, location_code);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string refund_count = dbsel->sql_field_value("refund_count");
    //////////////////////////////////////
    //  Refund amount
    dbsel->sql_reset ();
    sql  = "select   sum(payment_amount) as refund_amount from tg_pos_mobile_payment ";
    sql += "where    systransnum in (select z.systransnum from tg_pos_refund_header z";
    sql += "                           where z.register_number = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += "                           and z.logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += "                           and z.branch_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += ") ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string refund_amount = dbsel->sql_field_value("refund_amount");
    //////////////////////////////////////
    //  Z read item count
    dbsel->sql_reset ();
    sql  = "select * from pos_register ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string xread_count = dbsel->sql_field_value("xreadcount");
    string zread_count = dbsel->sql_field_value("zreadcount");
    string reset_count = dbsel->sql_field_value("resetcount");
    if (xread_count.length () < 1)  xread_count = "0";
    if (zread_count.length () < 1)  zread_count = "0";
    if (reset_count.length () < 1)  reset_count = "0";
    //////////////////////////////////////
    //  Can Zread
    dbsel->sql_reset ();
    sql  = "select count(*) as xread_cashier from tg_pos_xread_cashier ";
    sql += "where  register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, logical_date);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, location_code);
    sql += " and   xread_done = 0 ";
    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    string xread_cashier = dbsel->sql_field_value("xread_cashier");
    if (xread_cashier.length () < 1)  xread_cashier = "0";
    //////////////////////////////////////
    //  Sysdate
    Json::Value jsysdate;
    dbsel->sql_reset ();
    sql  = "select   * ";
#ifdef _CYSTORESERVER
    sql += "from     pos_sysdate ";
#else
    sql += "from     sales.pos_sysdate ";
#endif
    sql += "where transaction_date = ";
    sql += dbsel->sql_bind(1,logical_date);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    for (int x = 0; x < dbsel->numcols(); x++)
        jsysdate[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
    jsysdate["location_code"]="";
    //////////////////////////////////////
    //  Update the shift total(s)
    //  Total sales
    string totcashpay = jcash["payamt"].asString();
    if (totcashpay.length() < 1)
        totcashpay = "0";
    string totnoncash = jnoncash["payamt"].asString();
    if (totnoncash.length() < 1)
        totnoncash = "0";
    double d1 = stod(totcashpay);
    double d2 = stod(totnoncash);

    char sztmp[64];
    snprintf (sztmp, 30, "%.02f", (d1 + d2));
    string transaction_amount = sztmp;
    //  VAT amount
    string netvat = jtax["netvat"].asString();
    if (netvat.length() < 1)
        netvat = "0";
    string netamt = jtax["netamt"].asString();
    if (netamt.length() < 1)
        netamt = "0";
    //  NONVAT amount
    string netzero = jtax["netzero"].asString();
    if (netzero.length() < 1)
        netzero = "0";
    string netexempt = jtax["netexempt"].asString();
    if (netexempt.length() < 1)
        netexempt = "0";

    d1 = stod(netamt);
    d2 = stod(netvat);
    snprintf (sztmp, 30, "%.02f", (d2-d1));
    transaction_amount = sztmp;

    //////////////////////////////////////
    //  REGISTER / BRANCH
    Json::Value jreg,jbranch,jsettings;
    Json::Value jSetCashier;

    dbsel->sql_reset ();
    //////////////////////////////////////
    //REGISTER SETTINGS
    sql  = "select   * ";
    sql += "from     pos_register ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, register_num);


    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jreg[idx]=jline;
    }

    ////////////////////////////////////////
    //BRANCH SETTINGS
    sql  = "select   * ";
    sql += "from     cy_location ";
    sql += "where    location_code = ";
    sql += dbsel->sql_bind(1, location_code);


    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jbranch[idx]=jline;
    }
    ////////////////////////////////////////
    //POS SETTINGS
    sql  = "select   * ";
    sql += "from     pos_settings ";

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jsettings[idx]=jline;
    }
    //////////////////////////////////////
    //CASHIER SETTINGS
    sql  = "select   a.first_name,a.last_name,a.mi,a.login,";
    sql += "         b.start_date,b.start_time,b.end_date,b.end_time,b.shift ";
    sql += "from     cy_user a inner join tg_pos_xread_cashier b";
    sql += " ON       a.login = b.cashier ";
    sql += " AND      b.register_num = ";
    sql += dbsel->sql_bind(1, register_num);
    //sql += " AND      b.cashier = ";
    //sql += dbsel->sql_bind(2, cashier);
    sql += " AND      b.logical_date = ";
    sql += dbsel->sql_bind(3, logical_date);
    //sql += " AND      b.shift = ";
    //sql += dbsel->sql_bind(4, shift);

    if (!dbsel->sql_result (sql,true))
    {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        jline.clear ();
        for (int x = 0; x < dbsel->numcols(); x++)
            jline[dbsel->sql_field_name(x)]=dbsel->sql_field_value (dbsel->sql_field_name(x));
        jSetCashier[idx]=jline;
    }



    //////////////////////////////////////
    //  Assemble all JSON object(s)
    jheader["status"]="ok";
    jheader["tax"]=jtax;
    jheader["bank"]=jbank;
    jheader["cash"]=jcash;
    jheader["fund"]=jfund;
    jheader["sysdate"]=jsysdate;
    jheader["cashdec"]=jcashdec;
    jheader["takeout"]=jtakeout;
    jheader["notcash"]=jnoncash;
    jheader["paytype"]=jpaytype;
    jheader["discount"]=jdiscount;
    jheader["settings_register"]=jreg;
    jheader["hdr_branch"]=jbranch;
    jheader["hdr_settings"]=jsettings;
    jheader["settings_cashier"]=jSetCashier;

    jheader["void_count"]=void_count;
    jheader["zread_count"]=zread_count;
    jheader["xread_count"]=xread_count;
    jheader["reset_count"]=reset_count;
    jheader["void_amount"]=void_amount;
    jheader["sales_count"]=sales_count;
    jheader["cancel_count"]=cancel_count;
    jheader["suspend_count"]=suspend_count;
    jheader["suspend_item_count"]=suspend_item_count;

    jheader["refund_count"]=refund_count;
    jheader["refund_amount"]=refund_amount;

    jheader["suspend_count_pac"]="0";
    jheader["suspend_item_count_pac"]="0";

    jheader["authorized_by"]=authorized_by;
    jheader["can_zread"]=(atoi(xread_cashier.c_str()) == 0);
    _log->trace("xread cashier count",xread_cashier.c_str ());


    return true;
}
