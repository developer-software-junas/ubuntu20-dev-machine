//*******************************************************************
//        FILE:     cyrestsales_headerswitch.cpp
// DESCRIPTION:     Sales header record switch
//*******************************************************************
#include "cyrestsales.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesHeader
// DESCRIPTION:     Retrieve the sales record
//*******************************************************************
bool CYRestSales::salesHeaderSwitch (std::string systransold,
                                     enumHeader typeOld,
                                     std::string& systransnew,
                                     enumHeader typeNew,
                                     std::string branch_code,
                                     std::string register_num)
{
    //////////////////////////////////////
    //  Retrieve the old header
    Json::Value jheaderOld;
    jheaderOld.clear();
    if (!retrieveSalesHeader(jheaderOld, systransold, typeOld))
        return false;
    if (!retrieveSalesHeaderAccount(jheaderOld))
        return false;
    //////////////////////////////////////
    //  Default branch
    string location_code = branch_code;
    if (location_code.length() < 1)
        location_code = jheaderOld["branch_code"].asString();
    //////////////////////////////////////
    //  Default register
    string register_number = register_num;
    if (register_number.length() < 1)
        register_number = jheaderOld["register_number"].asString();
    //////////////////////////////////////
    //  Generate a transaction number???
    if (systransnew.length() < 1)  {
        //HDR_MOBILE, HDR_SUSPEND, HDR_CANCEL, HDR_POSTVOID, HDR_DAILY, HDR_CONSIGN
        if (typeNew == enumHeader::HDR_MOBILE)  {

        }

    }
    /*
    string sql;
    //////////////////////////////////////
    //  Sales record type
    switch (type)  {
        case HDR_POSTVOID:
            sql = "select * from tg_pos_void_header where systransnum = ";
        break;
        case HDR_DAILY:
            sql = "select * from tg_pos_daily_header where systransnum = ";
        break;
        case HDR_MOBILE:
            sql = "select * from tg_pos_mobile_header where systransnum = ";
        break;
        case HDR_CANCEL:
            sql = "select * from tg_pos_cancel_header where systransnum = ";
        break;
        case HDR_SUSPEND:
            sql = "select * from tg_pos_suspend_header where systransnum = ";
        break;
        case HDR_CONSIGN:
            sql = "select * from tg_pos_consign_header where systransnum = ";
        break;
    }
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = dbconn ();
    if (!db->ok())
        return seterrormsg(db->errordb());
    //////////////////////////////////////
    //  Create the header record
    jheader.clear();
    //////////////////////////////////////
    //  Bind and select the header
    sql += db->sql_bind (1, systransnum);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jheader,sql,"Unable to retrieve the sales record"))
        return false;
    //////////////////////////////////////
    //  Header settings info
    Json::Value jsettings;
    //////////////////////////////////////
    //  Company
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from cy_company where company_code = ";
    sql += db->sql_bind(1,jheader["company_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve company information"))
        return false;
    jheader["settings_company"]=jsettings;
    //////////////////////////////////////
    //  Branch
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from cy_location where company_code = ";
    sql += db->sql_bind(1,jheader["company_code"].asString());
    sql += " and   location_code = ";
    sql += db->sql_bind(2,jheader["branch_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve branch information"))
        return false;
    jheader["settings_branch"]=jsettings;
    string branch_code = jheader["branch_code"].asString();
    //////////////////////////////////////
    //  City
    string city_code = jsettings["city_code"].asString();
    string state_code = jsettings["state_code"].asString();

    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from sc_city where city_code = ";
    sql += db->sql_bind(1,city_code);
    sql += " and   state_code = ";
    sql += db->sql_bind(2,state_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve city information"))
        return false;
    jheader["settings_city"]=jsettings;
    //////////////////////////////////////
    //  State
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from sc_state where state_code = ";
    sql += db->sql_bind(1,state_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve state information"))
        return false;
    jheader["settings_state"]=jsettings;
    //////////////////////////////////////
    //  Branch
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from pos_settings where location_code = ";
    sql += db->sql_bind(1,branch_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve the POS branch settings information"))
        return false;
    jheader["settings_pos"]=jsettings;
    //////////////////////////////////////
    //  Currency
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from tg_pos_mobile_currency where systransnum = ";
    sql += db->sql_bind(1,jheader["systransnum"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve currency information"))
        return false;
    jheader["header_currency"]=jsettings;
    //////////////////////////////////////
    //  Global discount account
    if (!retrieveSalesTransAccount(jheader))
        return false;
    //////////////////////////////////////
    //  Transaction type
    jsettings.clear ();
    db->sql_reset ();
    sql  = "select * from tg_pos_mobile_transtype where systransnum = ";
    sql += db->sql_bind(1,jheader["systransnum"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve transaction information"))
        return false;
    jheader["header_transtype"]=jsettings;
    //////////////////////////////////////
    //  Transaction
    jsettings.clear ();
    db->sql_reset ();

    sql  = "select * from pos_transtype where transaction_cd = ";
    sql += db->sql_bind(1,jheader["transaction_code"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve transaction type information"))
        return false;
    jheader["settings_transtype"]=jsettings;
    //////////////////////////////////////
    //  Transaction discount
    string discount_code;
    jsettings.clear ();
    db->sql_reset ();

    discount_code = "";
    sql  = "select * from tg_pos_mobile_discount where systransnum = ";
    sql += db->sql_bind(1,jheader["systransnum"].asString());
    salesQueryType retcode = json_select_single(db,jsettings,sql,"Unable to retrieve transaction discount information");
    if (retcode == salesQueryType::SQT_NOROWS)
        jsettings = Json::nullValue;
    else if (retcode == salesQueryType::SQT_ERROR)
        return false;
    else
        discount_code = jsettings["discount_code"].asString();
    jheader["header_transtype_discount"]=jsettings;
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
    sql  = "select * from cy_user where login = ";
    sql += db->sql_bind(1,jheader["cashier"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve cashier information"))
        return false;

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
    sql  = "select * from pos_cashier_xread_shift where cashier = ";
    sql += db->sql_bind(1,jheader["cashier"].asString());
    sql += " and   register_num = ";
    sql += db->sql_bind(2,jheader["register_number"].asString());
    sql += " and   transaction_date = ";
    sql += db->sql_bind(3,jheader["logical_date"].asString());
    sql += " and   location_code = ";
    sql += db->sql_bind(4,jheader["branch_code"].asString());
    sql += " and   cashier_shift = ";
    sql += db->sql_bind(5,jheader["cashier_shift"].asString());
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jsettings,sql,"Unable to retrieve cashier information"))
        return false;
    jheader["settings_cashier_shift"]=jsettings;*/
    return true;
}
