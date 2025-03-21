//*******************************************************************
//        FILE:     cyrestsales_receipt_eod.cpp
// DESCRIPTION:     Sales receipt generation X AND Z RECEIPT
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     retrieveEodData
// DESCRIPTION:     Get relevant data
//*******************************************************************
bool                CYRestReceiptEod::retrieveFastX()
{
    //////////////////////////////////////
    //  Reset all total(s)
    reset ();
    //////////////////////////////////////
    //  Assigned to the EOD variable(s)
    //  Below code just notes....
    /*
    _eod->argXread = true;
    _eod->argManager = manager;
    _eod->argCashier = cashier;
    _eod->argShift = cashier_shift;
    _eod->argRegister = register_num;
    _eod->argSysdate = logical_date;
    _eod->argBranch = branch_code;
    _eod->argSuspended = clear_suspended;*/
    //////////////////////////////////////
    //  Z read flag
    if (!canZreadFast())
        return false;
    //////////////////////////////////////
    //  alt json object(s)
    if (!json_settings())
        return false;
    if (!json_settings_branch())
        return false;
    if (!json_settings_company())
        return false;
    if (!json_settings_sysdate())
        return false;
    if (!json_settings_cashier())
        return false;
    if (!json_settings_manager())
        return false;
    if (!json_settings_register())
        return false;
    if (!json_settings_currency())
        return false;
    //////////////////////////////////////
    //  EOD total(s)
    if (!listFspFast())
        return false;
    if (!listBankFast())
        return false;
    if (!listPaymentFast())
        return false;
    if (!listCashdecFast())
        return false;
    if (!listCashfundFast())
        return false;
    if (!cashPullout ())
        return false;
    if (!listDiscountFast())
        return false;
    if (!listTaxFast())
        return false;
    if (!listItemCountFast())
        return false;
    if (!cancelLoopFast())
        return false;
    if (!voidLoop())
        return false;
    /*CYONLINE
    if (!suspendLoopFast())
        return false;
     */

/*

  //  earliest / lastest - date & time
  time_t                endDate,   // max date
                        startDate; // earliest date
  //  start / end transaction
  int                   trxStart, trxEnd;
  std::string           endSystransnum,
                        startSystransnum;


  int                   xreadCount; //  xread attempts
  int                   zreadCount; //  zread attempts


  int                   cancelCount;  //  cancel trx count
  double                cancelAmount; //  cancel amount

  int                   suspendRecs;   //  suspended record(s)
  double                suspendCount;  //  no of items suspended
  double                suspendAmount; //  total suspended amount

  double                itemCount;     //  number of items sold
  double                giftWrapCount; //  number of gift wrap items

 */
    return true;
}
//*******************************************************************
//    FUNCTION:         canZRead
// DESCRIPTION:         Check if Zread ready
//*******************************************************************
bool                    CYRestReceiptEod::canZreadFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql, groupby = "";
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Sneak in the item count
    sql  = "select SUM(CONVERT(a.ba_redemption_reference,FLOAT)) AS items ";
    sql += " from tg_pos_daily_header a where a.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    groupby = " group by a.branch_code, a.logical_date, a.register_number";
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);

        groupby += ", a.cashier, a.cashier_shift ";
    }
    sql += groupby;

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string items = dbsel->sql_field_value("items");
    double ditems = _util->stodsafe(items);
    char sztmp[32];
    sprintf (sztmp, "%.02f", ditems);
    ditems = _util->stodsafe(sztmp);
    _totalItems = ditems;
    //////////////////////////////////////
    //  Sneak in the start / end dates
    string strDate;
    dbsel->sql_reset();
    sql  = "select transaction_date, transaction_time from tg_pos_daily_header a ";
    sql += "where a.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);

    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);

    }
    sql += " order by transaction_date, transaction_time limit 1 ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (dbsel->eof())  {
        strDate = _util->date_eight();
        strDate += _util->time_eight();
        startDate = atol(strDate.c_str ());
    }  else  {
        strDate = dbsel->sql_field_value("transaction_date");
        string tmp = dbsel->sql_field_value("transaction_time");
        if (tmp.length() < 4)  {
            strDate += "0";
            strDate += tmp;
        }  else  {
            strDate += tmp.substr(0,4);
        }
        startDate = atol(strDate.c_str ());
    }

    dbsel->sql_reset();
    sql  = "select transaction_date, transaction_time from tg_pos_daily_header a ";
    sql += "where a.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);

    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);

    }
    sql += " order by transaction_date desc, transaction_time desc limit 1 ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (dbsel->eof())  {
        strDate = _util->date_eight();
        strDate += _util->time_eight().substr(0,4);
        endDate = atol(strDate.c_str ());
    }  else  {
        strDate = dbsel->sql_field_value("transaction_date");
        string tmp = dbsel->sql_field_value("transaction_time");
        if (tmp.length() < 4)  {
            strDate += "0";
            strDate += tmp;
        }  else  {
            strDate += tmp.substr(0,4);
        }
        endDate = atol(strDate.c_str ());
    }
    //////////////////////////////////////
    //  Sneak in the start / end dates
    string strTrx;
    if (argXread)  {
        dbsel->sql_reset();
        sql  = "select transaction_number from tg_pos_daily_header a ";
        sql += "where a.register_number = ";
        sql += dbsel->sql_bind(1, argRegister);
        sql += " and   a.logical_date = ";
        sql += dbsel->sql_bind(2, argSysdate);
        sql += " and   a.branch_code = ";
        sql += dbsel->sql_bind(3, argBranch);
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
        sql += " order by transaction_number limit 1 ";

        if (!dbsel->sql_result (sql,true))  {
            _error = dbsel->errordb();
            return false;
        }
        if (dbsel->eof())  {
            strTrx = "0";
        }  else  {
            strTrx = dbsel->sql_field_value("transaction_number");
        }
        trxStart = atoi(strTrx.c_str());

        dbsel->sql_reset();
        sql  = "select transaction_number from tg_pos_daily_header a ";
        sql += "where a.register_number = ";
        sql += dbsel->sql_bind(1, argRegister);
        sql += " and   a.logical_date = ";
        sql += dbsel->sql_bind(2, argSysdate);
        sql += " and   a.branch_code = ";
        sql += dbsel->sql_bind(3, argBranch);
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
        sql += " order by transaction_number desc limit 1 ";

        if (!dbsel->sql_result (sql,true))  {
            _error = dbsel->errordb();
            return false;
        }
        if (dbsel->eof())  {
            strTrx = "0";
        }  else  {
            strTrx = dbsel->sql_field_value("transaction_number");
        }
        trxEnd = atoi(strTrx.c_str());
    }
    //////////////////////////////////////
    //  Update all servers / waiters
    dbsel->sql_reset();
    sql  = "update tg_pos_xread_cashier set xread_done = 1 ";
    sql += "where  cashier in (select a.login from cy_user a where a.group_code in (";
    sql += "                   select b.group_code from cy_user_groups b where b.is_pos_treasury = 1)); ";
    if (!dbsel->sql_only (sql,false))  {
        _error = dbsel->errordb();
        return false;
    }
    dbsel->sql_reset();
    sql  = "update pos_cashier_xread_shift set is_eod = 1 ";
    sql += "where  cashier in (select a.login from cy_user a where a.group_code in (";
    sql += "                   select b.group_code from cy_user_groups b where b.is_pos_treasury = 1)); ";
    if (!dbsel->sql_only (sql,false))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->sql_commit())  {
        _error = dbsel->errordb();
        return false;
    }

    /*  SLOW
    sql  = "select * ";
    sql += "from   tg_pos_xread_cashier ";
    sql += "where  register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    sql += " and xread_done = 0 ";
    */

    dbsel->sql_reset();
    sql  = "select a.*, b.first_name, b.last_name, b.login ";
    sql += "from   tg_pos_xread_cashier a, cy_user b  ";
    sql += " where a.cashier = b.login ";
    sql += " and   a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    sql += " and   a.xread_done = 0 ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            //process = true;
            if (argXread)  {
                string csh = dbsel->sql_field_value("cashier");
                string shift = dbsel->sql_field_value("shift");

                //if (csh == argCashier && shift == argShift)
                    //process = false;
            }
            /*
            if (process)  {
                found = false;
                int j = listUserXread.size();
                string login = dbsel->sql_field_value("login");
                string last_name = dbsel->sql_field_value("last_name");
                string first_name = dbsel->sql_field_value("first_name");
                string register_num = dbsel->sql_field_value("register_num");
                for (int i = 0; i < j; i++)  {
                    if (listUserXread.at(i)->login == login)  {
                        found = true;
                    }
                }
                if (!found)  {
                    STRUCT_EOD_USER* eodUser = new STRUCT_EOD_USER;
                    eodUser->login = login;
                    eodUser->last_name = last_name;
                    eodUser->first_name = first_name;
                    eodUser->register_num = register_num;
                    listUserXread.push_back(eodUser);
                }
            }*/
            //////////////////////////////
            //  Accumulated total(s)
            if (!voidTotal())
                return false;
            refundCount += abs(_util->stodsafe(dbsel->sql_field_value("audit_refund_count")));
            refundAmount += _util->stodsafe(dbsel->sql_field_value("audit_refund_amount"));
            trxCount += abs(_util->stodsafe(dbsel->sql_field_value("audit_trx_sales")));
            nonTrxCount += abs(_util->stodsafe(dbsel->sql_field_value("audit_trx_nonsales")));
            totCashFund += _util->stodsafe(dbsel->sql_field_value("total_drawer_amount"));
            cancelCount += abs(_util->stodsafe(dbsel->sql_field_value("audit_trx_cancel")));
            suspendCount += abs(_util->stodsafe(dbsel->sql_field_value("audit_trx_suspend")));

            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    if (!canZread())
        return false;
    return true;
}
//*******************************************************************
//    FUNCTION:         voidTotal
// DESCRIPTION:         VOID total(s)
//*******************************************************************
bool                    CYRestReceiptEod::voidTotal ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the void count
    sql  = "select count(*) as recs from tg_pos_void_header ";
    sql += " where register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string recs = dbsel->sql_field_value("recs");
    voidCount = _util->stodsafe(recs);

    dbsel->sql_reset();
    sql  = "select SUM(payment_amount-change_amount) as recs from tg_pos_mobile_payment ";
    sql += " where systransnum IN (";
    sql += "select z.systransnum from tg_pos_void_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    voidAmount = _util->stodsafe(recs);
    return true;
}
//*******************************************************************
//    FUNCTION:         voidTotal
// DESCRIPTION:         VOID total(s)
//*******************************************************************
bool                    CYRestReceiptEod::fnbOrderTotal()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the void count
    fnbCount = 0;
    sql  = "select count(*) as recs from tg_table_customer_orders ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string recs = dbsel->sql_field_value("recs");
    fnbCount += _util->stodsafe(recs);

    dbsel->sql_reset();
    sql  = "select count(*) as recs from tg_table_customer_orders ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    fnbCount += _util->stodsafe(recs);
    return true;
}

//*******************************************************************
//    FUNCTION:         voidTotal
// DESCRIPTION:         VOID total(s)
//*******************************************************************
bool                    CYRestReceiptEod::refundTotal ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the void count
    sql  = "select count(*) as recs from tg_pos_refund_header ";
    sql += " where register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string recs = dbsel->sql_field_value("recs");
    refundCount = _util->stodsafe(recs);

    dbsel->sql_reset();
    sql  = "select SUM(quantity*less_discount) as recs from tg_pos_mobile_detail ";
    sql += " where systransnum IN (";
    sql += "select z.systransnum from tg_pos_refund_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    refundAmount = _util->stodsafe(recs);
    return true;
}
//*******************************************************************
//    FUNCTION:         voidTotal
// DESCRIPTION:         VOID total(s)
//*******************************************************************
bool                    CYRestReceiptEod::cancelTotal ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the void count
    sql  = "select count(*) as recs from tg_pos_cancel_header ";
    sql += " where register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string recs = dbsel->sql_field_value("recs");
    cancelCount = _util->stodsafe(recs);

    dbsel->sql_reset();
    sql  = "select SUM(quantity*less_discount) as recs from tg_pos_mobile_detail ";
    sql += " where systransnum IN (";
    sql += "select z.systransnum from tg_pos_cancel_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    cancelAmount = _util->stodsafe(recs);
    return true;
}
//*******************************************************************
//    FUNCTION:         voidSuspend
// DESCRIPTION:         VOID total(s)
//*******************************************************************
bool                    CYRestReceiptEod::suspendTotal ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the void count
    sql  = "select count(*) as recs from tg_pos_suspend_header ";
    sql += " where register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    string recs = dbsel->sql_field_value("recs");
    suspendRecs = _util->stodsafe(recs);

    dbsel->sql_reset();
    sql  = "select SUM(quantity*less_discount) as recs from tg_pos_mobile_detail ";
    sql += " where systransnum IN (";
    sql += "select z.systransnum from tg_pos_suspend_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    suspendAmount = _util->stodsafe(recs);


    dbsel->sql_reset();
    sql  = "select SUM(quantity) as recs from tg_pos_mobile_detail ";
    sql += " where systransnum IN (";
    sql += "select z.systransnum from tg_pos_suspend_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    recs = dbsel->sql_field_value("recs");
    suspendCount = _util->stodsafe(recs);
    return true;
}
//*******************************************************************
//    FUNCTION:         listTaxFast
// DESCRIPTION:         Tax total(s)
//*******************************************************************
bool                    CYRestReceiptEod::listTaxFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Retrieve the bank total(s)
    sql  = "select a.* from tg_pos_xread_tax a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each tax record
    double serviceCharge = 0.00;
    double netVat = 0.00, netZero = 0.00, netExempt = 0.00;
    double amtVat = 0.00, amtZero = 0.00, amtExempt = 0.00;
    double netAmount = 0.00, grossAmount = 0.00, vatAmount = 0.00;

    if (!dbsel->eof ())  {
        do  {
            netVat    += _util->stodsafe(dbsel->sql_field_value("net_vat"));
            netZero   += _util->stodsafe(dbsel->sql_field_value("net_zero"));
            netExempt += _util->stodsafe(dbsel->sql_field_value("net_exempt"));

            amtVat    += _util->stodsafe(dbsel->sql_field_value("amt_vat"));
            amtZero   += _util->stodsafe(dbsel->sql_field_value("amt_zero"));
            amtExempt += _util->stodsafe(dbsel->sql_field_value("amt_exempt"));

            netAmount   += _util->stodsafe(dbsel->sql_field_value("net_amount"));
            grossAmount += _util->stodsafe(dbsel->sql_field_value("gross_amount"));
            vatAmount   += _util->stodsafe(dbsel->sql_field_value("vat_amount"));

            serviceCharge += _util->stodsafe(dbsel->sql_field_value("service_charge"));

            dbsel->sql_next ();
        } while(!dbsel->eof());

        rcptVat = vatAmount;
        rcptVatZero = netZero;
        rcptVatExempt = netExempt;

        rcptVatable = netVat;

        rcptNetSales = netAmount - vatAmount;
        rcptGrossSales = grossAmount;
        rcptServiceCharge = serviceCharge;

    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listTaxFast
// DESCRIPTION:         Tax total(s)
//*******************************************************************
bool                    CYRestReceiptEod::listItemCountFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the item count
    sql  = "select SUM(quantity) as totqty from tg_pos_mobile_detail where systransnum in ";
    sql += "(select z.systransnum from tg_pos_daily_header z where z.logical_date = ";
    sql += dbsel->sql_bind(1, argSysdate);
    sql += " and register_number = ";
    sql += dbsel->sql_bind(2, argRegister);
    sql += " and  branch_code = ";
    sql += dbsel->sql_bind(3, argBranch);

    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += ") ";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each tax record
    double totalItems = 0.00;

    if (!dbsel->eof ())  {
        totalItems = _util->stodsafe(dbsel->sql_field_value("totqty"));
        itemCount = totalItems;
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listDiscountFast
// DESCRIPTION:         List of discount(s)
//*******************************************************************
bool                    CYRestReceiptEod::listDiscountFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    MySqlDb mdec (_ini->_env);
    CYDbSql* dbdec =&mdec;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the discount total(s)
    sql  = "select a.* from tg_pos_xread_discount a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += " and   a.systransnum NOT IN (";
    sql += "select z.systransnum from tg_pos_void_header z ";
    sql += " where z.register_number = ";
    sql += dbsel->sql_bind(6, argRegister);
    sql += " and   z.logical_date = ";
    sql += dbsel->sql_bind(7, argSysdate);
    sql += " and   z.branch_code = ";
    sql += dbsel->sql_bind(8, argBranch);
    if (argXread)  {
        sql += " and   z.cashier = ";
        sql += dbsel->sql_bind(9, argCashier);
        sql += " and   z.cashier_shift = ";
        sql += dbsel->sql_bind(10, argShift);
    }
    sql += ")";
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each discount record
    enum_discount_type discountType = enum_discount_type::DISC_ITEM;
    double amount = 0.00;
    string tmp, code="", desc, type, tdesc;

    if (!dbsel->eof ())  {
        do  {
            found = false;
            tmp = dbsel->sql_field_value("type");
            if (tmp == "1") discountType = enum_discount_type::DISC_ITEM;
            if (tmp == "0") discountType = enum_discount_type::DISC_GLOBAL;
            if (tmp == "2") discountType = enum_discount_type::DISC_PAYMENT;

            code = dbsel->sql_field_value("discount_code");
            desc = dbsel->sql_field_value("discount_desc");
            type = dbsel->sql_field_value("tender_type_code");
            amount = _util->stodsafe(dbsel->sql_field_value("amount"));
            //////////////////////////////
            //  Declared total(s)
            sql  = "select a.discount_type_cd, a.description ";
            sql += "from   pos_discount_type a, pos_discount b ";
            sql += " where a.discount_type_cd = b.discount_type_cd ";
            sql += " and   b.discount_cd = ";
            sql += dbdec->sql_bind(1, code);

            if (!dbdec->sql_result (sql,true))  {
                _error = dbdec->errordb();
                return false;
            }
            if (dbdec->eof())  {
                type = "";
                tdesc = "";
            }
            found = false;
            if (discountType == enum_discount_type::DISC_ITEM ||
                discountType == enum_discount_type::DISC_GLOBAL)  {
                int y = listItemDiscount.size();
                for (int x = 0; x < y; x++)  {
                    if (listItemDiscount.at(x)->discountTypeCode == type &&
                        listItemDiscount.at(x)->discountCode == code)  {
                            found = true;
                            listItemDiscount.at(x)->count++;
                            listItemDiscount.at(x)->amount += amount;

                    }
                }
                if (!found)  {
                        STRUCT_EOD_DISCOUNT* eodPay = new STRUCT_EOD_DISCOUNT;
                        eodPay->count = 1;
                        eodPay->amount = amount;
                        eodPay->discountCode = code;
                        eodPay->discountDesc = desc;
                        eodPay->discountTypeCode = type;
                        listItemDiscount.push_back(eodPay);
                }
            }

            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listPaymentFast
// DESCRIPTION:         List of bank(s)
//*******************************************************************
bool                    CYRestReceiptEod::listPaymentFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    MySqlDb mdec (_ini->_env);
    CYDbSql* dbdec =&mdec;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the bank total(s)
    sql  = "select a.* from tg_pos_xread_payment a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each payment record
    bool cash = false;
    double amount = 0.00;
    string code="", type="", desc, tdesc,
           declared, actual,  groupby="";
    if (!dbsel->eof ())  {
        do  {
            found = false;
            cash = dbsel->sql_field_value("cash")=="1";
            code = dbsel->sql_field_value("tender_code");
            desc = dbsel->sql_field_value("tender_desc");
            type = dbsel->sql_field_value("tender_type_code");
            tdesc = dbsel->sql_field_value("tender_type_desc");
            amount = _util->stodsafe(dbsel->sql_field_value("amount"));
            //////////////////////////////
            //  Declared total(s)
            sql  = "select SUM(a.actual_amount) as actual_amount, ";
            sql += "       SUM(a.declared_amount) as declared_amount ";
            sql += "from   cy_cash_dec a ";
            sql += " where a.register_num = ";
            sql += dbdec->sql_bind(1, argRegister);
            sql += " and   a.transaction_date = ";
            sql += dbdec->sql_bind(2, argSysdate);
            sql += " and   a.location_code = ";
            sql += dbdec->sql_bind(3, argBranch);

            groupby = " group by a.location_code, a.transaction_date, a.register_num";
            if (argXread)  {
                sql += " and   a.cashier = ";
                sql += dbdec->sql_bind(4, argCashier);
                sql += " and   a.cashier_shift = ";
                sql += dbdec->sql_bind(5, argShift);

                groupby += ", a.cashier, a.cashier_shift ";
            }
            sql += groupby;
            if (!dbdec->sql_result (sql,true))  {
                _error = dbdec->errordb();
                return false;
            }
            if (dbdec->eof())  {
                actual = "0.00";
                declared = "0.00";
            }  else  {
                found = false;
                actual = dbdec->sql_field_value("actual_amount");
                if (_util->stodsafe(actual) > 0)  {
                    int y = listFund.size();
                    for (int x = 0; x < y; x++)  {
                        if (listFund.at(x)->tenderTypeCode == type &&
                            listFund.at(x)->tenderCode == code)  {
                            found = true;
                            listFund.at(x)->count++;
                            listFund.at(x)->amount += amount;
                            listFund.at(x)->cashdec += _util->stodsafe(actual);
                            listFund.at(x)->declared += _util->stodsafe(declared);                            
                        }
                    }
                    if (!found)  {
                        STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                        eodPay->count = 1;
                        eodPay->amount = amount;
                        eodPay->cashdec = _util->stodsafe(actual);
                        eodPay->declared =_util->stodsafe(declared);
                        eodPay->tenderCode = code;
                        eodPay->tenderDesc = desc;
                        eodPay->tenderTypeCode = type;
                        eodPay->tenderTypeDesc = tdesc;
                        listFund.push_back(eodPay);
                    }
                }
                found = false;
                declared = dbdec->sql_field_value("declared_amount");
                if (_util->stodsafe(declared) > 0)  {
                    int y = listDeclare.size();
                    for (int x = 0; x < y; x++)  {
                        if (listDeclare.at(x)->tenderTypeCode == type &&
                            listDeclare.at(x)->tenderCode == code)  {
                            found = true;
                            listDeclare.at(x)->count++;
                            listDeclare.at(x)->amount += amount;
                            listDeclare.at(x)->cashdec += _util->stodsafe(actual);
                            listDeclare.at(x)->declared += _util->stodsafe(declared);
                        }
                    }
                    if (!found)  {
                        STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                        eodPay->count = 1;
                        eodPay->amount = amount;
                        eodPay->cashdec = _util->stodsafe(actual);
                        eodPay->declared =_util->stodsafe(declared);
                        eodPay->tenderCode = code;
                        eodPay->tenderDesc = desc;
                        eodPay->tenderTypeCode = type;
                        eodPay->tenderTypeDesc = tdesc;
                        listDeclare.push_back(eodPay);
                    }
                }
            }
            found = false;
            int y = listPay.size();
            for (int x = 0; x < y; x++)  {
                if (listPay.at(x)->tenderTypeCode == type &&
                    listPay.at(x)->tenderCode == code)  {
                    found = true;
                    listPay.at(x)->count++;
                    listPay.at(x)->amount += amount;
                    listPay.at(x)->cashdec += _util->stodsafe(actual);
                    listPay.at(x)->declared += _util->stodsafe(declared);
                }
            }
            if (!found)  {
                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = 1;
                eodPay->amount = amount;
                eodPay->iscash = cash;
                eodPay->cashdec = _util->stodsafe(actual);
                eodPay->declared =_util->stodsafe(declared);
                eodPay->tenderCode = code;
                eodPay->tenderDesc = desc;
                eodPay->tenderTypeCode = type;
                eodPay->tenderTypeDesc = tdesc;
                listPay.push_back(eodPay);
            }
            if (cash)  {
                cntCash++;
                totCash += amount;
            }  else  {
                cntNonCash++;
                totNonCash += amount;
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listCashfundFast
// DESCRIPTION:         List of bank(s)
//*******************************************************************
bool                    CYRestReceiptEod::listCashfundFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    MySqlDb mdec (_ini->_env);
    CYDbSql* dbdec =&mdec;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the bank total(s)
    listFund.clear();
    sql  = "select a.* from pos_cash_fund a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each payment record
    double amount = 0.00;
    string code="", type="", desc, tdesc,
           declared, actual,  groupby="";
    if (!dbsel->eof ())  {
        do  {
            found = false;
            code = dbsel->sql_field_value("tender_code");
            desc = dbsel->sql_field_value("tender_desc");
            amount = _util->stodsafe(dbsel->sql_field_value("cash_fund_amt"));
            //////////////////////////////
            //  Declared total(s)
            dbdec->sql_reset();
            sql  = "select * from pos_tender ";
            sql += " where tender_cd = ";
            sql += dbdec->sql_bind(1, code);

            if (!dbdec->sql_result (sql,true))  {
                _error = dbdec->errordb();
                return false;
            }
            if (!dbdec->eof())  {
                type = dbdec->sql_field_value("tender_type_cd");
                tdesc = "";//dbdec->sql_field_value("description");
            }
            found = false;
            int y = listFund.size();
            for (int x = 0; x < y; x++)  {
                if (listFund.at(x)->tenderTypeCode == type &&
                    listFund.at(x)->tenderCode == code)  {
                    found = true;
                    listFund.at(x)->count++;
                    listFund.at(x)->amount += amount;
                }
            }
            if (!found)  {
                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = 1;
                eodPay->amount = amount;
                eodPay->cashdec = 0.00;
                eodPay->declared = 0.00;
                eodPay->tenderCode = code;
                eodPay->tenderDesc = desc;
                eodPay->tenderTypeCode = type;
                eodPay->tenderTypeDesc = tdesc;
                listFund.push_back(eodPay);
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listCashdecFast
// DESCRIPTION:         List of bank(s)
//*******************************************************************
bool                    CYRestReceiptEod::listCashdecFast ()
{
    cashDeclare();
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;

    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the cashdec tender
    listCashdec.clear();
    sql  = "select a.description as tender_type_desc, ";
    sql += "       a.is_cashdec, b.* ";
    sql += "from   pos_tender_type a, pos_tender b ";
    sql += " where a.tender_type_cd = b.tender_type_cd ";
    sql += " and   a.is_cashdec = 1";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each payment record
    if (!dbsel->eof ())  {
        do  {
            found = false;
            string code = dbsel->sql_field_value("tender_cd");
            string desc = dbsel->sql_field_value("description");
            string type = dbsel->sql_field_value("tender_type_cd");
            string tdesc = dbsel->sql_field_value("tender_type_desc");
            int y = listCashdec.size();
            for (int x = 0; x < y; x++)  {
                if (listCashdec.at(x)->tenderTypeCode == type &&
                    listCashdec.at(x)->tenderCode == code)  {
                    found = true;
                    listCashdec.at(x)->count++;
                }
            }
            if (!found)  {
                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = 1;
                eodPay->amount = 0.00;
                eodPay->cashdec = true;
                eodPay->declared = 0.00;
                eodPay->tenderCode = code;
                eodPay->tenderDesc = desc;
                eodPay->tenderTypeCode = type;
                eodPay->tenderTypeDesc = tdesc;

                //  Get the declared amount
                string declaredAmount = cgiform (eodPay->tenderCode);
                if (declaredAmount.length() > 0)  {
                    eodPay->declared = _util->stodsafe(declaredAmount);
                }
                listCashdec.push_back(eodPay);
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listFspFast
// DESCRIPTION:         List of FSP customer(s)
//*******************************************************************
bool                    CYRestReceiptEod::listFspFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;

    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    CYDbSql* dbdec = _ini->dbconn ();
    if (nullptr == dbdec)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the FSP total(s)
    listFspCustomer.clear();
    sql  = "select a.* from tg_pos_xread_points a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each FSP record
    if (!dbsel->eof ())  {
        do  {
            found = false;
            string last_name = dbsel->sql_field_value("last_name");
            string first_name = dbsel->sql_field_value("first_name");
            string middle_name = dbsel->sql_field_value("middle_name");
            string account_number = dbsel->sql_field_value("account_number");
            string account_type_desc = dbsel->sql_field_value("account_type_desc");

            double epurse_spent = _util->stodsafe(dbsel->sql_field_value("epurse_spent"));
            double points_earned = _util->stodsafe(dbsel->sql_field_value("points_earned"));
            int y = listFspCustomer.size();
            for (int x = 0; x < y; x++)  {
                if (listFspCustomer.at(x)->acct_type_code == "FSP" &&
                    listFspCustomer.at(x)->account_number == account_number &&
                    (epurse_spent + points_earned) > 0)  {
                    found = true;
                    listFspCustomer.at(x)->count++;
                    listFspCustomer.at(x)->epurse += epurse_spent;
                    listFspCustomer.at(x)->points += points_earned;
                }
            }
            if (!found)  {
                if ((epurse_spent + points_earned) > 0)  {
                    STRUCT_EOD_FSP* eodFsp = new STRUCT_EOD_FSP;
                    eodFsp->count = 1;
                    eodFsp->epurse = epurse_spent;
                    eodFsp->points = points_earned;
                    eodFsp->last_name = last_name;
                    eodFsp->first_name = first_name;
                    eodFsp->middle_name = middle_name;
                    eodFsp->account_number = account_number;
                    eodFsp->acct_type_code = "FSP";
                    listFspCustomer.push_back(eodFsp);
                }
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         listBankFast
// DESCRIPTION:         List of bank(s)
//*******************************************************************
bool                    CYRestReceiptEod::listBankFast ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    MySqlDb msel (_ini->_env);
    CYDbSql* dbsel = &msel;
    /*CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }*/
    //////////////////////////////////////
    //  Retrieve the bank total(s)
    sql  = "select a.* from tg_pos_xread_bank a ";
    sql += " where a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   a.logical_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    //////////////////////////////////////
    //  For each bank record
    double amount = 0.00;
    string code="", desc="";
    if (!dbsel->eof ())  {
        do  {
            found = false;
            code = dbsel->sql_field_value("bank_code");
            desc = dbsel->sql_field_value("bank_desc");
            amount = _util->stodsafe(dbsel->sql_field_value("amount"));

            int y = listBank.size();
            for (int x = 0; x < y; x++)  {
                if (listBank.at(x)->bankCode == code &&
                    listBank.at(x)->bankDesc == desc)  {
                    found = true;
                    listBank.at(x)->count++;
                    listBank.at(x)->amount += amount;
                }
            }
            if (!found)  {
                STRUCT_EOD_BANK* eodBank = new STRUCT_EOD_BANK;
                eodBank->count = 1;
                eodBank->amount = amount;
                eodBank->bankCode = code;
                eodBank->bankDesc = desc;
                listBank.push_back(eodBank);
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
