//*******************************************************************
//        FILE:     cyrestsales_headeraccount.cpp
// DESCRIPTION:     Sales header record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "cydb/cydbsql.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesHeaderAccount
// DESCRIPTION:     Retrieve the sales account(S)
//*******************************************************************
bool CYRestCommon::retrieveSalesHeaderAccount(Json::Value& jheader)
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Create the account array
    Json::Value jaccount;
    jaccount.clear();
    //////////////////////////////////////
    //  Bind and select the header
    string sql = "select * from tg_pos_mobile_account where systransnum = ";
    sql += db->sql_bind (1, jheader["systransnum"].asString());
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    if (!db->eof())  {
        int idx = 0;
        do  {

            string name, val;
            Json::Value jline,jval;
            for (int i = 0; i < db->numcols(); i++)  {
                name = db->sql_field_name(i);
                val = db->sql_field_value(name);
                jline[name]=val;
            }
            if (_exportType.length() > 0)  {
                assembleSqlInsert(db,"tg_pos_mobile_account",sql);
                _sqlSales += sql;
            }
            jaccount[idx] = jline;
            idx++;
            db->sql_next ();
        } while (!db->eof ());
    }  else  {
        jaccount = Json::nullValue;
        return true;
    }
    //////////////////////////////////////
    //  Header settings info
    string code, account_type;
    Json::Value jsettings, jtmp, jline;

    //  Account type
    jsettings.clear ();
    db->sql_reset ();
    int j = jaccount.size();
    for (int i = 0; i < j; i++)  {
        jline.clear();
        jtmp = jaccount[i];
        code = jtmp["account_type_code"].asString();
        sql  = "select * from tg_card_type where acct_type_code = ";
        sql += db->sql_bind(1,code);
        if (salesQueryType::SQT_SUCCESS == json_select_single(db,jline,sql,"Unable to retrieve account server information"))  {
            account_type  = "settings_account_";
            account_type += jtmp["account_type_code"].asString();
            jheader[account_type]=jline;
        }
    }
    jheader["hdr_account"]=jaccount;

    //////////////////////////////////////
    //  Does the account exist
    db->sql_reset();
    sql  = "select * from tg_pos_mobile_header_receipt where systransnum = '";
    sql += jheader["systransnum"].asString(); sql += "' ";
    //sql += db->sql_bind(1, systransnum);
    Json::Value jtaxaccount;
    salesQueryType retCode = json_select_single(db, jtaxaccount, sql);
    if (salesQueryType::SQT_NOROWS == retCode)
        jtaxaccount["customer_name"]=Json::nullValue;
    else if (salesQueryType::SQT_SUCCESS != retCode)
        return seterrormsg(db->errordb());
    jheader["hdr_taxaccount"]=jtaxaccount;
    return true;
}
//*******************************************************************
//    FUNCTION:     retrieveSalesHeaderAccount
// DESCRIPTION:     Retrieve the sales account(S)
//*******************************************************************
bool CYRestCommon::retrieveSalesTransAccount(Json::Value& jheader)
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Create the account object
    string acct_type_code;
    Json::Value jaccount;
    jaccount.clear();
    string systransnum = jheader["systransnum"].asString();
    //////////////////////////////////////
    //  Transaction account
    acct_type_code = "";
    db->sql_reset();
    string sql  = "select * from tg_pos_mobile_trxaccount ";
    sql += " where systransnum = ";
    sql += db->sql_bind(1,systransnum);
    salesQueryType retcode = json_select_single(db,jaccount,sql,"Unable to retrieve transaction account information");
    if (retcode == salesQueryType::SQT_NOROWS)
        jaccount = Json::nullValue;
    else if (retcode == salesQueryType::SQT_ERROR)
        return false;
    else  {
        acct_type_code = jaccount["account_type_code"].asString();
        if (_exportType.length() > 0)  {
            assembleSqlInsert(db,"tg_pos_mobile_trxaccount",sql);
            _sqlSales += sql;
        }
    }
    jheader["hdr_trxaccount"]=jaccount;
    //////////////////////////////////////
    //  Account settings
    Json::Value jsettings;
    jsettings.clear ();

    if (acct_type_code.length() > 0)  {
        db->sql_reset ();
        sql  = "select * from tg_card_type where acct_type_code = ";
        sql += db->sql_bind(1,acct_type_code);
        if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve the transaction account settings information"))
            jsettings=Json::nullValue;
        string account_type  = "settings_account_";
        account_type += acct_type_code;
        jheader[account_type]=jsettings;
    }
    //////////////////////////////////////
    //  Account O.R.
    jsettings.clear ();

        db->sql_reset ();
        sql  = "select * from tg_pos_mobile_header_receipt where systransnum = ";
        sql += db->sql_bind(1,jheader["systransnum"].asString());
        retcode = json_select_single(db,jsettings,sql,"Unable to retrieve OR account information");
        if (retcode == salesQueryType::SQT_NOROWS)
            jaccount = Json::nullValue;
        else if (retcode == salesQueryType::SQT_ERROR)
            return false;
    jheader["hdr_tax_receipt"]=jsettings;

    return true;
}
