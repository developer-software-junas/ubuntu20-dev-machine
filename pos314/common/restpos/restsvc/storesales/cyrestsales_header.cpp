//*******************************************************************
//        FILE:     cyrestsales_header.cpp
// DESCRIPTION:     Sales header record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "cydb/cydbsql.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesHeader
// DESCRIPTION:     Retrieve the sales record
//*******************************************************************
bool CYRestCommon::retrieveSalesHeader(Json::Value& jheader,
                                       string systransnum,
                                       salesReceiptType type)
{
    string sql, table = "";

    _sqlSales = "";
    _exportType = "";
    _totalSplitAdjust = 0.00;

    if (systransnum.length() < 1)
        return seterrormsg("Invalid transaction number specified");
    //////////////////////////////////////
    //  Sales record type
    switch (type)  {
        case SRT_POSTVOID:
            _exportType = "postvoid";
            table = "tg_pos_void_header";
            sql = "select * from tg_pos_void_header where systransnum = ";
        break;
        case SRT_DAILY:
        case SRT_NORMAL:
            _exportType = "sale";
            table = "tg_pos_daily_header";
            sql = "select * from tg_pos_daily_header where systransnum = ";
        break;
        case SRT_REPRINT:
            table = "tg_pos_daily_header";
            sql = "select * from tg_pos_daily_header where systransnum = ";
        break;
        case SRT_MOBILE:
            table = "tg_pos_mobile_header";
            sql = "select * from tg_pos_mobile_header where systransnum = ";
        break;
        case SRT_CANCEL:
            _exportType = "cancel";
            table = "tg_pos_cancel_header";
            sql = "select * from tg_pos_cancel_header where systransnum = ";
        break;
        case SRT_RESUME:
        case SRT_SUSPEND:
            table = "tg_pos_suspend_header";
            sql = "select * from tg_pos_suspend_header where systransnum = ";
        break;
        case SRT_CONSIGN:
            table = "tg_pos_consign_header";
            sql = "select * from tg_pos_consign_header where systransnum = ";
        break;
        case SRT_REFUND:
            _exportType = "refund";
            table = "tg_pos_refund_header";
            sql = "select * from tg_pos_refund_header where systransnum = ";
        break;
        case SRT_FNB:
            table = "tg_pos_fnb_header";
            sql = "select * from tg_fnb_header where systransnum = ";
        break;
        case SRT_OPTICAL:
            table = "tg_pos_optical_header";
            sql = "select * from tg_pos_optical_header where systransnum = ";
        break;
    }
     _exportType = "";
    //////////////////////////////////////
    //  Database connection
    //CYDbSql* db = _ini->dbconn();
    //if (nullptr == db)
        //return seterrormsg(_ini->errormsg());
    CYDbSql* dbsel = _ini->dbconn();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Create the header record
    jheader.clear();
    //////////////////////////////////////
    //  Bind and select the header
    sql += dbsel->sql_bind (1, systransnum);
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jheader,sql,"Unable to retrieve the sales record"))  {
        _log->logmsg("SQLERROR",sql.c_str());
        _error = systransnum;
        _error += "==>";
        _error += sql;
        return false;
    }
    if (_exportType.length() > 0)  {
        assembleSqlInsert(dbsel,table,sql);
        _sqlSales += sql;
    }
    //////////////////////////////////////
    //  Header settings info
    Json::Value jsettings;
    //////////////////////////////////////
    //  FNB header
    jsettings.clear ();
    dbsel->sql_reset ();
    string split_flag = "0";
    string split_amount = "0.00";
    sql  = "select * from tg_pos_mobile_fnb where systransnum = ";
    sql += dbsel->sql_bind(1,systransnum);
    salesQueryType retCode = json_select_single(dbsel,jsettings,sql,"Unable to retrieve FNB information");
    if (retCode == salesQueryType::SQT_SUCCESS)  {
        split_flag = dbsel->sql_field_value("split_flag");
        split_amount = dbsel->sql_field_value("split_amount");
        if (_exportType.length() > 0)  {
            assembleSqlInsert(dbsel,"tg_pos_mobile_fnb",sql);
            _sqlSales += sql;
        }
    }
    if (split_flag == "888")  {
        if (_util->valid_decimal("Split amount",split_amount.c_str (),1,15,0.01,999999999))  {
            _totalSplitAdjust = _util->stodsafe(split_amount);
        }
    }
    jheader["hdr_fnb"]=jsettings;
    //////////////////////////////////////
    //  Lane buster
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from tg_pos_mobile_lanebust where systransnum = ";
    sql += dbsel->sql_bind(1,systransnum);
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve lanebust information"))
        jheader["hdr_lanebust"] = Json::nullValue;
    else
        jheader["hdr_lanebust"]=jsettings;
    //////////////////////////////////////
    //  Company
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from cy_company where company_code = ";
    sql += dbsel->sql_bind(1,jheader["company_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve company information"))
        return false;
    jheader["hdr_company"]=jsettings;
    //////////////////////////////////////
    //  Branch
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from cy_location where company_code = ";
    sql += dbsel->sql_bind(1,jheader["company_code"].asString());
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(2,jheader["branch_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve branch information"))
        return false;
    jheader["hdr_branch"]=jsettings;
    string branch_code = jheader["branch_code"].asString();
    //////////////////////////////////////
    //  City
    string city_code = jsettings["city_code"].asString();
    string state_code = jsettings["state_code"].asString();
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from sc_city where city_code = ";
    sql += dbsel->sql_bind(1,city_code);
    sql += " and   state_code = ";
    sql += dbsel->sql_bind(2,state_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve city information"))  {
        jsettings.clear();
        jsettings["city_code"]=city_code;
        jsettings["state_code"]=state_code;
        jsettings["description"]="";
        jsettings["is_default"]="0";
        jsettings["zip"]="";
    }
    jheader["hdr_city"]=jsettings;
    //////////////////////////////////////
    //  State
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from sc_state where state_code = '";
    sql += state_code; sql += "' ";
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,sql.c_str()))  {
        jsettings.clear();
        jsettings["city_code"]="MKT";
        jsettings["state_code"]="MM";
        jsettings["description"]="";
        jsettings["is_default"]="0";
        jsettings["zip"]="";
    }
    jheader["hdr_state"]=jsettings;
    //////////////////////////////////////
    //  POS settings
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from pos_settings where location_code = ";
    sql += dbsel->sql_bind(1,branch_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve the POS branch settings information"))
        return false;
    jheader["hdr_settings"]=jsettings;
    //////////////////////////////////////
    //  Device name ***///
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select ipad_alias from pos_register where register_num = ";
    sql += dbsel->sql_bind(1,jheader["register_number"].asString());

    if (!dbsel->sql_result (sql, true))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ()){
        string device_name = dbsel->sql_field_value("register_num");
        if (device_name.size() < 1)
            device_name = "null";
        jheader["device_name"]=device_name.c_str();
    }
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
currencyagain:
    db->sql_reset();
    jsettings.clear ();
    dbsel->sql_reset ();
    sql  = "select * from tg_pos_mobile_currency where systransnum = ";
    sql += dbsel->sql_bind(1,jheader["systransnum"].asString());
    salesQueryType sqt = json_select_single(dbsel,jsettings,sql,"Unable to retrieve currency information");
    if (salesQueryType::SQT_SUCCESS != sqt)  {
        if (salesQueryType::SQT_NOROWS == sqt)  {
            dbsel->sql_reset();
            sql = "select * from cy_currency where is_default = 1 ";
            sqt = json_select_single(dbsel,jsettings,sql,"Unable to retrieve currency information");
            if (salesQueryType::SQT_SUCCESS == sqt)  {
                db->sql_bind_reset();
                sql  = "insert into tg_pos_mobile_currency(systransnum, ";
                sql += "currency_code, description, is_multiply, default_rate) values (";
                sql += db->sql_bind(1,jheader["systransnum"].asString()); sql += ", ";
                sql += db->sql_bind(2,jsettings["currency_code"].asString()); sql += ", ";
                sql += db->sql_bind(3,jsettings["description"].asString()); sql += ", ";
                sql += db->sql_bind(4,jsettings["default_action"].asString()); sql += ", ";
                sql += db->sql_bind(5,jsettings["default_rate"].asString()); sql += ") ";
                if (!db->sql_only(sql,true))  {
                    _error = db->errordb();
                    return false;
                }
                if (!db->sql_commit())  {
                    _error = db->errordb();
                    return false;
                }  else  {
                    goto currencyagain;
                }
            }
        }
    }

    if (_exportType.length() > 0)  {
        assembleSqlInsert(db,"tg_pos_mobile_currency",sql);
        _sqlSales += sql;
    }
    jheader["hdr_currency"]=jsettings;
    //////////////////////////////////////
    //  Global discount account
    if (!retrieveSalesTransAccount(jheader))
        return false;
    //////////////////////////////////////
    //  Transaction type
    jsettings.clear ();
    dbsel->sql_reset ();

    string tmp = jheader["transaction_code"].asString();
    sql  = "select * from pos_transtype where transaction_cd = ";
    sql += dbsel->sql_bind(1,jheader["transaction_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve transaction type information"))
        return false;
    jheader["settings_transtype"]=jsettings;
    //  Set transaction flag(s)
    string pwd = jsettings["is_pwd"].asString();
    string sc = jsettings["is_sc"].asString();
    string zero = jsettings["is_zero_rated"].asString();
    //////////////////////////////////////
    //  Transaction type
    dbsel->sql_reset ();
    jsettings.clear ();
    Json::Value jtrxsales;
    tmp = jheader["systransnum"].asString();
    if (tmp.length() < 1)
        tmp = systransnum;
    sql  = "select * from tg_pos_mobile_transtype where systransnum = ";
    sql += dbsel->sql_bind(1,tmp);
    if (salesQueryType::SQT_SUCCESS != json_select_single(dbsel,jsettings,sql,"Unable to retrieve transaction information"))  {
        jsettings["systransnum"]=systransnum;

        //  No transaction type
        string is_internal = "0";
        string trxtype = jheader["transaction_code"].asString ();
        dbsel->sql_reset ();
        sql  = "select * from pos_transtype where transaction_cd = '";
        sql += trxtype; sql += "' ";
        if (dbsel->sql_result(sql,false))  {
            if (!dbsel->eof ())  {
                is_internal = dbsel->sql_field_value("is_internal");
                if (is_internal == "1")
                    _isInternal = true;
                db->sql_bind_reset ();
                sql  = "insert into tg_pos_mobile_transtype (transaction_code, ";
                sql += "description, systransnum, is_default, min_amount, max_amount, ";
                sql += "is_internal, is_zero_rated) values ('";
                sql += dbsel->sql_field_value("transaction_cd"); sql += "', '";
                sql += dbsel->sql_field_value("description"); sql += "', '";
                sql += tmp; sql += "', ";
                sql += dbsel->sql_field_value("is_default"); sql += ", ";
                sql += dbsel->sql_field_value("min_amount"); sql += ", ";
                sql += dbsel->sql_field_value("max_amount"); sql += ", ";
                sql += dbsel->sql_field_value("is_internal"); sql += ", ";
                sql += dbsel->sql_field_value("is_zero_rated"); sql += ") ";

                _log->logmsg("SQL",sql.c_str());
                if (_exportType.length() > 0)  {
                    assembleSqlInsert(db,"tg_pos_mobile_transtype",sql);
                    _sqlSales += sql;
                }
                if (!db->sql_only(sql,false))  {
                    _error = db->errordb();
                    return false;
                }
                if (!db->sql_commit())  {
                    _error = db->errordb();
                    return false;
                }
            }
        }
    }

    jtrxsales = jsettings;
    jheader["header_transtype"]=jsettings;

    //  Set transaction flag(s)
    _isPwd = (pwd == "1");
    _isSenior = (sc == "1");
    _isZeroRated = (zero == "1");
    jtrxsales["trx_type"]=jsettings;
    //////////////////////////////////////
    //  Transaction discount
    string discount_code;
    jsettings.clear ();
    db->sql_reset ();

    discount_code = "";
    sql  = "select * from tg_pos_mobile_discount where systransnum = ";
    sql += db->sql_bind(1,jheader["systransnum"].asString());
    salesQueryType retcode = json_select_single(db,jsettings,sql,"Unable to retrieve transaction discount information");
    if (retcode == salesQueryType::SQT_NOROWS)  {
        jsettings = Json::nullValue;
        _hdrDiscountDesc = "";
        _hdrDiscountCode = "";
        _hdrDiscountPct  = false;
        _hdrDiscountRate = 0.00;
    } else if (retcode == salesQueryType::SQT_ERROR)  {
        return false;
    } else  {
        discount_code = jsettings["discount_code"].asString();

        _hdrDiscountDesc = jsettings["description"].asString();
        _hdrDiscountCode = jsettings["discount_code"].asString();
        _hdrDiscountPct  = jsettings["discount_type"].asString() == "1";

        string tmp = jsettings["discount_value"].asString();
        if (_util->valid_decimal("Transaction discount value", tmp.c_str (), 1, 99, 1, 999999999))  {
            _hdrDiscountRate = _util->stodsafe(tmp);
        }  else  {
            return seterrormsg(_util->errormsg());
        }
        if (_exportType.length() > 0)  {
            assembleSqlInsert(db,"tg_pos_mobile_discount",sql);
            _sqlSales += sql;
        }
    }
    jtrxsales["trx_discount"]=jsettings;
    jheader["header_transtype_discount"]=jsettings;
    jtrxsales["trx_account"]=jheader["hdr_trxaccount"];
    //////////////////////////////////////
    //  Transaction discount
    string discount_type_code;
    jsettings.clear ();
    db->sql_reset ();

    discount_type_code = "";
    if (discount_code.length() > 0)  {
        sql  = "select * from pos_discount where discount_cd = ";
        sql += db->sql_bind(1,discount_code);
        salesQueryType retcode = json_select_single(db,jsettings,sql,"Unable to retrieve transaction discount information");
        if (retcode != salesQueryType::SQT_SUCCESS)
            jsettings = Json::nullValue;
        else
            discount_type_code = jsettings["discount_type_cd"].asString();
    }
    jheader["settings_transaction_discount"]=jsettings;
    //string max_amount = jsettings["max_amount"].asString();
    //_trxDiscountMaxAmount = _util->stodsafe(max_amount);

    jsettings.clear ();
    if (discount_type_code.length()>0)  {
        db->sql_reset ();
        sql  = "select * from pos_discount_type where discount_type_cd = ";
        sql += db->sql_bind(1,discount_type_code);
        if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve transaction discount type information"))
            return false;
    }
    jheader["settings_transaction_discount_type"]=jsettings;
    //////////////////////////////////////
    //  Register
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from pos_register where register_num = ";
    sql += db->sql_bind(1,jheader["register_number"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve register information"))
        return false;
    jheader["settings_register"]=jsettings;
    //////////////////////////////////////
    //  Cashier
    jsettings.clear ();
    db->sql_reset ();
    tmp = cgiform("cashier");
    if (tmp.length() < 1)
        tmp = jheader["cashier"].asString();
    sql  = "select * from cy_user where login = ";
    sql += db->sql_bind(1,tmp);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve cashier information."))  {
        db->sql_reset ();
        sql  = "select * from cy_user ";
        if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve cashier information."))
            return false;
    }

    string category_cd = jsettings["category_cd"].asString();
    string subcat_cd = jsettings["subcat_cd"].asString();
    string class_cd = jsettings["class_cd"].asString();
    string subclass_cd = jsettings["subclass_cd"].asString();
    jsettings["consessionaire"]="";
    //////////////////////////////////////
    //  Cashier concession
    if (subclass_cd.length() > 0)  {
        db->sql_reset ();
        sql  = "select description from cy_subclass ";
        sql += " where category_cd = ";  sql += db->sql_bind(1,category_cd);
        sql += " and   subcat_cd = ";  sql += db->sql_bind(2,subcat_cd);
        sql += " and   class_cd = ";  sql += db->sql_bind(3,class_cd);
        sql += " and   subclass_cd = ";  sql += db->sql_bind(4,subclass_cd);
        if (!db->sql_result (sql, true))
            return seterrormsg(db->errordb());
        if (!db->eof ())
            jsettings["consessionaire"]=db->sql_field_value("description");
    }
    jheader["settings_cashier"]=jsettings;
    //////////////////////////////////////
    //  Cashier shift
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from pos_cashier_xread_shift where cashier = '";
    sql += jheader["cashier"].asString();
    sql += "' and   register_num = ";
    sql += jheader["register_number"].asString();
    sql += " and   transaction_date = ";
    sql += jheader["logical_date"].asString();
    sql += " and   location_code = '";
    sql += jheader["branch_code"].asString();
    sql += "' and   cashier_shift = ";
    sql += jheader["cashier_shift"].asString();
    //_log->logmsg("DEBUGJOEL",sql.c_str ());


    string cashier = cgiform("cashier");
    string register_num  = jheader["register_number"].asString();
    string company_code  = cgiform("company_code");
    string location_code = jheader["branch_code"].asString();
    string cashier_shift = jheader["cashier_shift"].asString();
    string transaction_date = jheader["logical_date"].asString();
    string transaction_code =jheader["transaction_code"].asString();

    if (cashier.length() < 1)
        tmp = jheader["cashier"].asString();
    sql  = "select * from pos_cashier_xread_shift where cashier = ";
    sql += db->sql_bind(1,tmp);
    sql += " and   register_num = ";
    sql += db->sql_bind(2,register_num);
    sql += " and   transaction_date = ";
    sql += db->sql_bind(3,transaction_date);
    sql += " and   location_code = ";
    sql += db->sql_bind(4,location_code);
    sql += " and   cashier_shift = ";
    sql += db->sql_bind(5,cashier_shift);
    //_log->logmsg("DEBUGJOEL",sql.c_str ());


    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve cashier information.."))
        jheader["settings_cashier_shift"]=Json::nullValue;
    else
        jheader["settings_cashier_shift"]=jsettings;
    //////////////////////////////////////
    //  pos gift item footer ***///
    /*
    _log->logmsg("aaaaaa","footer start");
    db->sql_reset ();
    string name, value;
    Json::Value jpromoftr,jpromoftr2;
    size_t f;

    sql  = "select * from pos_giftitem_promo_rcpt where gift_code = 888; ";
    _log->logmsg("footer start select", sql.c_str ());

    if (!db->sql_result (sql, true))  {
        _log->logmsg("footer error", sql.c_str ());
        return seterrormsg(db->errordb ().c_str ());
    }
    if (!db->eof ())  {
        jpromoftr.clear();
        _log->logmsg("footer found", sql.c_str ());
        string permit_no = jheader["settings_register"]["permit"].asString();
        int j=0;
        do {
            for (int i = 0; i < db->numcols(); i++)  {
                name = db->sql_field_name(i);
                value = db->sql_field_value(name);
                if (value.length () < 1) value = "0";
                f = value.find("<PERMIT-NO>");
                if (f != std::string::npos)
                    value.replace(f,11,permit_no.c_str());
                jpromoftr[name]=value;
            }
            db->MoveNext();
            jpromoftr2["ftr_gift_rcpt"][j]=jpromoftr;
            j++;
        } while (!db->eof());
    }  else  {
        jpromoftr2 = Json::nullValue;
    }
    */

    //////////////////////////////////////
    //  Concessionaire
    /***
    string name = "";
    string value = "";
    db->sql_reset ();
    Json::Value jconsign;

    sql  = "select a.*, e.description as subclass_desc, ";
    sql += "       b.description as category_desc, ";
    sql += "       c.description as subcat_desc, d.description as class_desc ";
    sql += "from   tg_pos_mobile_consign a, cy_category b, cy_subcategory c, cy_class d, cy_subclass e ";
    sql += " where systransnum = ";
    sql += db->sql_bind (1, jheader["systransnum"].asString());
    sql += " and   a.category_cd = b.category_cd ";
    sql += " and   a.category_cd = c.category_cd ";
    sql += " and   a.subcat_cd = c.subcat_cd ";
    sql += " and   a.category_cd = d.category_cd ";
    sql += " and   a.subcat_cd = d.subcat_cd ";
    sql += " and   a.class_cd = d.class_cd ";
    sql += " and   a.category_cd = e.category_cd ";
    sql += " and   a.subcat_cd = e.subcat_cd ";
    sql += " and   a.class_cd = e.class_cd ";
    sql += " and   a.subclass_cd = e.subclass_cd ";

    if (!db->sql_result (sql, true))
        return seterrormsg(db->errordb ().c_str ());
    if (!db->eof ())  {
        jconsign.clear();
        for (int i = 0; i < db->numcols(); i++)  {
            name = db->sql_field_name(i);
            value = db->sql_field_value(name);
            if (value.length () < 1) value = "0";
            jconsign[name]=value;
        }
    }  else  {
        jconsign = Json::nullValue;
    }
    jheader["hdr_consign"]=jconsign;***/
    jheader["hdr_consign"]=Json::nullValue;
    //////////////////////////////////////
    //  Total transaction(s)
    db->sql_reset ();
    sql  = "select   count(*) as sales_count from tg_pos_daily_header ";
    sql += "where    cashier = ";
    sql += db->sql_bind(1,jheader["cashier"].asString());
    sql += " and     register_number = ";
    sql += db->sql_bind(2,jheader["register_number"].asString());
    sql += " and     cashier_shift = ";
    sql += db->sql_bind(3,jheader["cashier_shift"].asString());
    sql += " and     logical_date = ";
    sql += db->sql_bind(4,jheader["logical_date"].asString());

    if (!db->sql_result (sql,true))
        return seterrormsg(db->errordb());
    string sales_count = db->sql_field_value("sales_count");
    //////////////////////////////////////
    //  Suspended transaction(s)
    db->sql_reset ();
    sql  = "select   count(*) as suspend_count from tg_pos_suspend_header ";
    sql += "where    cashier = ";
    sql += db->sql_bind(1,jheader["cashier"].asString());
    sql += " and     register_number = ";
    sql += db->sql_bind(2,jheader["register_number"].asString());
    sql += " and     cashier_shift = ";
    sql += db->sql_bind(3,jheader["cashier_shift"].asString());
    sql += " and     logical_date = ";
    sql += db->sql_bind(4,jheader["logical_date"].asString());

    if (!db->sql_result (sql,false))
        return seterrormsg(db->errordb());
    string suspend_count = db->sql_field_value("suspend_count");
    //////////////////////////////////////
    //  Suspended FNB transaction(s)
    db->sql_reset ();
    sql  = "select   count(distinct(b.customer_number)) as fnb_count ";
    sql += "from     tg_branch_table a, tg_table_customer_checkout b ";
    sql += "where    a.branch_code = ";
    sql += db->sql_bind(1,jheader["branch_code"].asString());
    sql += " and     a.table_code = b.table_code ";

    if (!db->sql_result (sql,false))
        return seterrormsg(db->errordb());
    string fnbsuspend_count = db->sql_field_value("fnb_count");
    //////////////////////////////
    //  Account type count
    db->sql_reset ();
    sql  = "select count(*) as recs ";
    sql += "from   pos_trans_card a, tg_card_type b ";
    sql += "where  a.transaction_cd = ";
    sql += db->sql_bind (1, jheader["transaction_code"].asString());
    sql += " and   a.acct_type_code = b.acct_type_code ";
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb ().c_str ());
    string cardtyperecs = db->sql_field_value ("recs");
    //////////////////////////////
    //  Discount type count
    db->sql_reset ();
    string today = _util->date_eight ();
    sql = "select count(*) as recs from pos_discount_type a, pos_discount b ";
    sql += "where a.discount_type_cd = b.discount_type_cd ";
    sql += "and   b.start_date <= ";
    sql += today;
    sql += " and  b.end_date >= ";
    sql += today;
    sql += " and  a.is_global = 1 ";
    sql += " and   b.discount_cd in (select z.discount_cd from pos_trans_discount z ";
    sql += "                         where z.transaction_cd = ";
    sql += db->sql_bind(1, jheader["transaction_code"].asString());
    sql += ") order by b.description ";
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb ().c_str ());
    string discountrecs = db->sql_field_value ("recs");
    jtrxsales["trx_count_account"]=cardtyperecs;
    jtrxsales["trx_count_discount"]=discountrecs;
    //////////////////////////////////////
    //  Count
    jheader["sales_count"]=sales_count;
    jheader["suspended_count"]=suspend_count;
    jheader["fnbsuspended_count"]=fnbsuspend_count;
    //jheader["ftr_gift_rcpt"]=jpromoftr2;
    jheader["hdr_trx"]=jtrxsales;

    return true;
}
