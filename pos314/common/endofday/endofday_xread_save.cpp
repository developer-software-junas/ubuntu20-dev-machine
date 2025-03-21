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
string                   cyposrest::rest_xread_save ()
{
    char szfunc [] = "xReadSave";
    //return _util->jsonerror(szfunc);
    //////////////////////////////////////
    //  Retrieve argument(s)
    string cashier = cgiform("cashier");
    string manager = cgiform("manager");
    string mgrpass = cgiform("mgrpass");
    string register_num = cgiform("register_num");
    string logical_date = cgiform("logical_date");
    string location_code = cgiform("location_code");
    string cashier_shift = cgiform("cashier_shift");
    string clear_suspended = cgiform("clear_suspended");
    //////////////////////////////////////
    //  Assign to the EOD variable(s)
    _eod->argXread = true;
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
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    CYDbSql* dbsel2 = _ini->dbconn ();
    if (!dbsel2->ok())
        return _util->jsonerror(szfunc,_ini->errormsg());
    //////////////////////////////////////
    //  Calculated value(s)
    char sztmp [64];
    int count = _eod->cntCash + _eod->cntNonCash;
    sprintf (sztmp, "%d", count);
    string transaction_count = sztmp;

    sprintf(sztmp,"%.02f",(_eod->totCash + _eod->totNonCash));
    string transaction_amount = sztmp;
    //////////////////////////////////////
    //  Register alias???
    string sql;
    dbsel->sql_reset();
    string hold_register="", ipad_alias = "0";
    sql  = " select ipad_alias from pos_register where register_num = ";
    sql += dbsel->sql_bind (1, register_num);
    sql += " and    location_code = ";
    sql += dbsel->sql_bind (2, location_code);
    if (!dbsel->sql_result (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    if (!dbsel->eof ()) {
        ipad_alias = dbsel->sql_field_value("ipad_alias");
        if (ipad_alias.length() > 0 && ipad_alias != "0")  {
            hold_register = register_num;
            register_num = ipad_alias;
        }
    }
alias:
    //////////////////////////////////////
    //  Validate the manager credentials
    //if (!_eod->managerauth(manager,mgrpass))
        //return _util->jsonerror(szfunc,"Invalid manager credentials");
    //////////////////////////////////////
    //  Get the xread count
    sprintf (sztmp, "%d", _eod->xreadCount);
    string xread_count = sztmp;
    //////////////////////////////////////
    //  Last SI number
    sprintf (sztmp,"%d",_eod->trxEnd);
    string trxnum = sztmp;
    string transaction_number = _eod->endSystransnum;
    //////////////////////////////////////
    //  Update delcared cash
    dbsel->sql_reset();
    sql  = "delete from cy_cash_dec where ";
    sql += " company_code = ";
    sql += dbsel->sql_bind ( 1, _eod->getAltValue("company_code",_eod->alt_settings_company));
    sql += " and location_code = ";
    sql += dbsel->sql_bind ( 2, location_code);
    sql += " and transaction_date = ";
    sql += dbsel->sql_bind ( 3, logical_date);
    sql += " and register_num = ";
    sql += dbsel->sql_bind ( 4, register_num);
    sql += " and cashier = ";
    sql += dbsel->sql_bind ( 5, cashier);
    sql += " and cashier_shift = ";
    sql += dbsel->sql_bind ( 6, cashier_shift);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());

    size_t j = _eod->listCashdec.size();
    for (size_t i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* scd = _eod->listCashdec.at(i);
        if (scd->cashdec)  {
            //  Get the declared amount
            string declaredAmount = cgiform (scd->tenderCode);
            if (declaredAmount.length() > 0)  {
                scd->declared = _util->stodsafe(declaredAmount);
            }
            //  Get the total cash takeout
            double takeOut = 0.00;
            int y = _eod->listPullout.size();
            for (int x = 0; x < y; x++)  {
                STRUCT_EOD_PAYMENT* to = _eod->listPullout.at(x);
                takeOut += to->amount;
            }

            double dVariance = scd->amount - (scd->declared + takeOut);

            char szActual[32];
            snprintf (szActual, 30, "%.02f", scd->amount);
            char szDeclared[32];
            snprintf (szDeclared, 30, "%.02f", scd->declared);
            char szVariance[32];
            snprintf (szVariance, 30, "%.02f", dVariance);

            string cytimestamp = _util->date_sequence();



            dbsel->sql_bind_reset();
            sql  = "insert into cy_cash_dec (company_code, location_code, ";
            sql += "transaction_date, register_num, currency_code, tender_code, ";
            sql += "cashier, cashier_shift, actual_amount, declared_amount, cytimestamp, variance, ";
            sql += "is_declared, conv_amount, conv_variance, conv_declared, is_sync) values (";
            sql += dbsel->sql_bind ( 1, _eod->getAltValue("company_code",_eod->alt_settings_company));  sql += ", ";
            sql += dbsel->sql_bind ( 2, location_code);  sql += ", ";
            sql += dbsel->sql_bind ( 3, logical_date);  sql += ", ";
            sql += dbsel->sql_bind ( 4, register_num);  sql += ", ";
            sql += dbsel->sql_bind ( 5, _eod->getAltValue("currency_code",_eod->alt_settings_currency));  sql += ", ";
            sql += dbsel->sql_bind ( 6, scd->tenderCode);  sql += ", ";
            sql += dbsel->sql_bind ( 7, cashier);  sql += ", ";
            sql += dbsel->sql_bind ( 8, cashier_shift);  sql += ", ";
            sql += dbsel->sql_bind ( 9, szActual);  sql += ", ";
            sql += dbsel->sql_bind (10, szDeclared);  sql += ", ";
            sql += dbsel->sql_bind (11, cytimestamp);  sql += ", ";
            sql += dbsel->sql_bind (12, szVariance);  sql += ", 1, 0, 0, 0, 0) ";

            if (!dbsel->sql_only (sql,true))
                return _util->jsonerror("...",dbsel->errordb());
        }
    }
    //////////////////////////////////////
    //  Update the Xread table(s)
    dbsel->sql_bind_reset ();
    sql  = "update pos_cashier_xread_shift set transaction_count = ";
    sql += dbsel->sql_bind( 1, transaction_count);
    sql += ", xread_count = ";
    sql += dbsel->sql_bind( 2, xread_count);
    sql += ", transaction_amount = ";
    sql += dbsel->sql_bind( 3, transaction_amount);
    sql += ", transaction_number = ";
    sql += dbsel->sql_bind( 4, trxnum);
    sql += ", is_eod = 1, trans_nonvat_amount = 0 ";
    sql += ", sales_adjustment = 0.00, trans_vatexempt_amt = 0 ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind( 5, location_code);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind( 6, logical_date);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind( 7, register_num);
    sql += " and   cashier = ";
    sql += dbsel->sql_bind(8, cashier);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(9, cashier_shift);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    //////////////////////////////////////
    //  Update the cash fund
    dbsel->sql_bind_reset ();
    sql  = "update pos_cash_fund ";
    sql += "set is_sync = 1 ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind( 1, location_code);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind( 2, logical_date);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind( 3, register_num);
    sql += " and   cashier = ";
    sql += dbsel->sql_bind( 4, cashier);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind( 5, cashier_shift);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    //////////////////////////////////////
    //  Update the cash takeout
    dbsel->sql_bind_reset ();
    sql  = "update pos_cash_takeout ";
    sql += "set is_sync = 1 ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind( 1, location_code);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind( 2, logical_date);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind( 3, register_num);
    sql += " and   cashier = ";
    sql += dbsel->sql_bind( 4, cashier);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind( 5, cashier_shift);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());
    //////////////////////////////////////
    //  Delete suspended record(s)???
    /***
    if (clear_suspended == "1" ||
        clear_suspended == "delete")  {
        string sqlwhere;
        sqlwhere  = "where  systransnum in (select z.systransnum from tg_pos_suspend_header z ";
        sqlwhere += "                       where  z.cashier = '";
        sqlwhere += cashier;
        sqlwhere += "'                      and    z.register_number = ";
        sqlwhere += register_num;
        sqlwhere += "                       and    z.cashier_shift = ";
        sqlwhere += cashier_shift;
        sqlwhere += "                       and    z.logical_date = ";
        sqlwhere += logical_date;
        sqlwhere += "                       and    z.branch_code = '";
        sqlwhere += location_code;
        sqlwhere += "') ";

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_tax "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_trxaccount "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_discount_summary "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_transtype "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_currency "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_audit "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());


        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_discount "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_account "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_gc_payment "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_bank_payment "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_discount "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_payment "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());


        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_discount_detail "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_mobile_detail "; sql += sqlwhere;
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());

        dbsel->sql_bind_reset ();
        sql  = "delete from tg_pos_suspend_header ";
        sql += "where  cashier = ";
        sql += dbsel->sql_bind(1, cashier);
        sql += " and   register_number = ";
        sql += dbsel->sql_bind(2, register_num);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(3, cashier_shift);
        sql += " and   logical_date = ";
        sql += dbsel->sql_bind(4, logical_date);
        sql += " and   branch_code = ";
        sql += dbsel->sql_bind(5, location_code);
        if (!dbsel->sql_only (sql,false))
            return _util->jsonerror(szfunc,dbsel->errordb());
    }***/
    //////////////////////////////////////
    //  Reset the header
    dbsel->sql_bind_reset ();
    sql  = "delete from tg_pos_mobile_header ";
    sql += "where  cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and   register_number = ";
    sql += dbsel->sql_bind(2, register_num);
    sql += " and   cashier_shift = ";
    sql += dbsel->sql_bind(3, cashier_shift);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind(4, logical_date);
    sql += " and   branch_code = ";
    sql += dbsel->sql_bind(5, location_code);
    if (!dbsel->sql_only (sql,false))
        return _util->jsonerror(szfunc,dbsel->errordb());
    //////////////////////////////////////
    //  Reset the cashier
    dbsel->sql_bind_reset ();
    sql  = "delete from pos_cashier_register ";
    sql += "where  cashier = ";
    sql += dbsel->sql_bind(1, cashier);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind(2, register_num);

    if (!dbsel->sql_only (sql,false))
        return _util->jsonerror(szfunc,dbsel->errordb());

    //////////////////////////////////////
    //  Set the xread flag
    dbsel->sql_bind_reset ();
    sql  = "update tg_pos_xread_cashier set xread_done = 1 ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind( 1, location_code);
    sql += " and   logical_date = ";
    sql += dbsel->sql_bind( 2, logical_date);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind( 3, register_num);
    sql += " and   cashier = ";
    sql += dbsel->sql_bind( 4, cashier);
    sql += " and   shift = ";
    sql += dbsel->sql_bind( 5, cashier_shift);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());

    //////////////////////////////////////
    //  Increment the xread count
    dbsel->sql_bind_reset ();
    sql  = "update pos_register set xreadcount = xreadcount + 1 ";
    sql += " where location_code = ";
    sql += dbsel->sql_bind( 1, location_code);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind( 2, register_num);

    if (!dbsel->sql_only (sql,true))
        return _util->jsonerror(szfunc,dbsel->errordb());

    if (!dbsel->sql_commit ())
        return _util->jsonerror(szfunc,dbsel->errordb());

    if (hold_register.length() > 0)  {
        register_num = hold_register;
        hold_register = "";
        goto alias;
    }
    if (ipad_alias.length() > 0 && ipad_alias != "0")  {
        _cgi->add_form("register_num",ipad_alias);
    }

    return rest_xread();
}
