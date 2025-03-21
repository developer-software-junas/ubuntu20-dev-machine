/********************************************************************
          FILE:         cypostrx_utility.cpp
   DESCRIPTION:         POS transaction utility methods
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
      FUNCTION:         parseFileError
   DESCRIPTION:         Handle JSON file parsing errors
 *******************************************************************/
CYDbSql*                    cypostrx::dbconnect ()
{

    CYDbSql* db = nullptr;
#ifdef _CYSTORESERVER
    db = new MySqlDb (_env);
#else
    db = new SqliteDb (_env);
#endif
    _dbList.push_back(db);
    return db;


}
/*******************************************************************
      FUNCTION:         parseFileError
   DESCRIPTION:         Handle JSON file parsing errors
 *******************************************************************/
bool                    cypostrx::parseFileError (Json::Value _jsales,
                                                  std::string filename)
{
    (void)_jsales;
    (void)filename;
    return true;
}

/*******************************************************************
      FUNCTION:         parseSalesFile
   DESCRIPTION:         Parse the JSON sales file
 *******************************************************************/
bool                    cypostrx::parseSales (std::string strdata)
{


    //////////////////////////////////
    //  Read the JSON file
    string payload = strdata;

    //////////////////////////////////////
    //  JSON result parse
    Json::Features features;
    Json::Reader reader(features);
    features = Json::Features::strictMode();


    Json::CharReaderBuilder builder;
    const std::string rawJson = payload;

    if (!reader.parse(rawJson.c_str(), _jsales, false)) {
        _jsales["errormessage"]="Invalid Sales JSON Data";
        //parseFileError(_jsales,filename);
        return false;
    }  else  {
        //////////////////////////////
        //  Save the sales record


        if (_jsales.isMember("systransnum"))
                _systransnum = _jsales["systransnum"].asString ();
        else  {
                _jsales["errormessage"]="The transaction request did NOT have a systransnum data";
                //parseFileError(_jsales,filename);
                return false;
        }



    }
    //////////////////////////////////////
    //  Database variable(s)
    _cashier       = _jsales["cashier"].asString();
    _branch_code   = _jsales["branch_code"].asString();
    _company_code  = _jsales["company_code"].asString();
    _register_num  = _jsales["register_number"].asString();
    _logical_date  = _jsales["logical_date"].asString();
    _cashier_shift = _jsales["cashier_shift"].asString();
    //////////////////////////////////////
    //  Setting(s)
    _jsettings.clear ();
    if (_jsales.isMember("hdr_settings"))
        _jsettings = _jsales["hdr_settings"];
    _jcurrency.clear ();
    if (_jsales.isMember("hdr_currency"))
        _jcurrency = _jsales["hdr_currency"];
    //////////////////////////////////////
    //  Header object(s)
    _jcompany.clear ();
    if (_jsales.isMember("hdr_company"))
        _jcompany = _jsales["hdr_company"];

    _jbranch.clear ();
    if (_jsales.isMember("hdr_branch"))
        _jbranch = _jsales["hdr_branch"];

    _jregister.clear ();
    if (_jsales.isMember("settings_register"))
        _jregister = _jsales["settings_register"];

    _jaccount.clear ();
    if (_jsales.isMember("hdr_trxaccount"))
        _jaccount = _jsales["hdr_trxaccount"];

    _jtrx.clear ();
    if (_jsales.isMember("hdr_trx"))
        _jtrx = _jsales["hdr_trx"];

    _jtrxtype.clear ();
    if (_jtrx.isMember("trx_type"))
        _jtrxtype = _jtrx["trx_type"];

    _jtrxaccount.clear ();
    if (_jsales.isMember("hdr_account"))
        _jtrxaccount = _jsales["hdr_account"];

    _jtrxdiscount.clear ();
    if (_jtrx.isMember("trx_discount"))
        _jtrxdiscount = _jtrx["trx_discount"];

    _settingstransdisc.clear();
    if(_jsales.isMember("settings_transaction_discount"))
        _settingstransdisc = _jsales["settings_transaction_discount"];


    //////////////////////////////////////
    //  Detail
    _jdetail.clear ();
    if (_jsales.isMember("det_sales"))
        _jdetail = _jsales["det_sales"];
    //////////////////////////////////////
    //  Payment
    _jpayment.clear ();
    if (_jsales.isMember("pay_sales"))
        _jpayment = _jsales["pay_sales"];


    //////////////////////////////////////
    // CashFund
    _jcashfund.clear ();
    if (_jsales.isMember("cash_fund"))
        _jcashfund = _jsales["cash_fund"];


    /////////////////////////////////////
    if (_jbranch.isMember("description"))
        _branch_name = _jbranch["description"].asString();
    if (_jcompany.isMember("description"))
        _company_name = _jcompany["description"].asString();


    /////////////////////////////////////
    /// identify the accounts
    ///
    ///
    /// _jtrxaccount
    if (_jtrxaccount.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != _jtrxaccount.size(); i++)  {
              Json::Value jtmpacct = _jtrxaccount[i];


              if( atoi(jtmpacct["is_clerk"].asString().c_str()) == 1 )
                  _jsclerk  = jtmpacct;
              if( atoi(jtmpacct["is_loyalty"].asString().c_str()) == 1 )
                  _jfsp  = jtmpacct;
              if( atoi(jtmpacct["is_beauty_addict"].asString().c_str()) == 1 )
                  _jba  = jtmpacct;


        }
    }





    return true;
}


















    /***
    //////////////////////////////////////
    //  Debug stream
    stringstream ss;
    ss << "JCOMPANY: " << jcompany["description"].asString () << endl;
    ss << "JBRANCH: " << jbranch["description"].asString () << endl;
    ss << "JCURRENCY: " << jcurrency["currency_code"].asString () << endl;
    ss << "JREGISTER: " << jregister["serialno"].asString () << endl;
    ss << "JSETTINGS: " << jsettings["trade_name"].asString () << endl;


    if (jaccount.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != jaccount.size(); i++)  {
            Json::Value jacct = jaccount[i];
            ss << "JACCOUNT: " << jacct["account_number"].asString () << endl;
        }
    }

    Json::Value jdetail = _jsales["det_sales"];
    Json::Value jdetail_discount;



    if (jdetail.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != jdetail.size(); i++)  {
            Json::Value jdet = jdetail[i];
            ss << "JDETAIL: " << jdet["description"].asString () << endl;
            jdetail_discount.clear ();
            if (jdet.isMember("det_discount"))  {
                jdetail_discount = jdet["det_discount"];
                if (jdetail_discount.isNull())
                    printf("NULL");
            }
            if (jdet.isMember("det_product"))
                printf ("has product");
        }
    }

    Json::Value jdetdiscount;

    if (jdetail.isMember("det_discount"))
        jdetdiscount = jdetail["det_discount"];
    ***/


/*
    //////////////////////////////////////
    //  Header json object(s)
    Json::Value jbranch, jaccount, jcompany, jregister,
                jcurrency, jsettings, jtrxtype, jtrx;
    //////////////////////////////////////
    //  Setting(s)
    jsettings.clear ();
    if (_jsales.isMember("hdr_settings"))
        jsettings = _jsales["hdr_settings"];
    jcurrency.clear ();
    if (_jsales.isMember("hdr_currency"))
        jcurrency = _jsales["hdr_currency"];
    //////////////////////////////////////
    //  Header object(s)
    jcompany.clear ();
    if (_jsales.isMember("hdr_company"))
        jcompany = _jsales["hdr_company"];

    jbranch.clear ();
    if (_jsales.isMember("hdr_branch"))
        jbranch = _jsales["hdr_branch"];

    jregister.clear ();
    if (_jsales.isMember("hdr_register"))
        jbranch = _jsales["hdr_register"];

    jaccount.clear ();
    if (_jsales.isMember("hdr_account"))
        jaccount = _jsales["hdr_account"];

    jtrx.clear ();
    if (_jsales.isMember("hdr_trx"))
        jtrx = _jsales["hdr_trx"];

    jtrxtype.clear ();
    if (_jsales.isMember("trx_type"))
        jtrxtype = _jsales["trx_type"];
    //////////////////////////////////////
    //  Debug stream
    stringstream ss;
    ss << "JCOMPANY: " << jcompany["description"].asString () << endl;
    ss << "JBRANCH: " << jbranch["description"].asString () << endl;
    ss << "JCURRENCY: " << jcurrency["currency_code"].asString () << endl;
    ss << "JREGISTER: " << jregister["serialno"].asString () << endl;
    ss << "JSETTINGS: " << jsettings["trade_name"].asString () << endl;


    if (jaccount.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != jaccount.size(); i++)  {
            Json::Value jacct = jaccount[i];
            ss << "JACCOUNT: " << jacct["account_number"].asString () << endl;
        }
    }

    Json::Value jdetail = _jsales["det_sales"];
    Json::Value jdetail_discount;



    if (jdetail.isArray())  {
        for (Json::Value::ArrayIndex i = 0; i != jdetail.size(); i++)  {
            Json::Value jdet = jdetail[i];
            ss << "JDETAIL: " << jdet["description"].asString () << endl;
            jdetail_discount.clear ();
            if (jdet.isMember("det_discount"))  {
                jdetail_discount = jdet["det_discount"];
                if (jdetail_discount.isNull())
                    printf("NULL");
            }
            if (jdet.isMember("det_product"))
                printf ("has product");
        }
    }

    Json::Value jdetdiscount;

    if (jdetail.isMember("det_discount"))
        jdetdiscount = jdetail["det_discount"];


    printf ("%s",ss.str().c_str());
    return true;
}
*/



/*******************************************************************
      FUNCTION:         deleteSalesFile
   DESCRIPTION:         Cleanup the JSON sales file
 *******************************************************************/
bool                    cypostrx::removeSalesFile (std::string filename)
{
    //////////////////////////////////////
    //  Get the sales path
    (void)filename;
    return true;
}





/*******************************************************************
      FUNCTION:          GetAccountDiscountValue
   DESCRIPTION:          Account Discount Lookup
     ARGUMENTS:          none
       RETURNS:          the JSON output
 *******************************************************************/
 /*
string                   cypostrx::GetAccountDiscountValue (char*  acct_type_code, char* item_code)
{
    ///////////////////////////////////////
    //  Connect to database


    if (!dbconnect())
        return _util->jsonerror("Unable to retrieve card information.",errormsg());

    MySqlDb              db (_env);
    MySqlDb              db2 (_env);

    string               sql,
                         strwhere,
                         dept,
                         subdept,
                         itmClass,
                         itmSubClass,
                         baseTypeCode;
    int                  i;

    Json::Value          jacctPromo;
    /////////////////////////////////////


    //////////////////////////////////////
    //Get account Promo
    db.sql_bind_reset();
    db.sql_reset();
    sql =  "select a.condition_code, a.discount_cd, b.*, c.disctype,c.discvalue from  ";
    sql += "cy_account_promo a, cy_discount_condition b, ";
    sql += "pos_discount c where  a.condition_code = b.condition_code ";
    sql += "and a.discount_cd = c.discount_cd ";
    sql += "and a.acct_type_code = ";
    sql += db.sql_bind(1,acct_type_code);

    //  Execute the query
    if (!db.sql_result (sql,true))
        return _util->jsonerror("Unable to retrieve the account discount information.",db.errordb());

    if (db.eof ())
        return _util->jsonerror("Unable to retrieve the account discount information.","No records(s) retrieved.");


    //////////////////////////////////////
    //  Transaction JSON formatting
      while (!db.eof ())  {

            strwhere = "";

            dept         = db.sql_field_value ("category_code");
            subdept      = db.sql_field_value ("subcategory_code");
            itmClass     = db.sql_field_value ("class_code");
            itmSubClass  = db.sql_field_value ("subclass_code");
            baseTypeCode = db.sql_field_value ("base_type_code");


            if (dept.length() > 0) {
                strwhere += " and category_cd = '";
                strwhere += dept;
                strwhere += "'";
            }
            if (subdept.length() > 0) {
                strwhere += " and subcat_cd = '";
                strwhere += subdept;
                strwhere += "'";
            }
            if (itmClass.length() > 0) {
                strwhere += " and class_cd = '";
                strwhere += itmClass;
                strwhere += "'";
            }
            if (itmSubClass.length() > 0) {
                strwhere += " and subclass_cd = '";
                strwhere += itmSubClass;
                strwhere += "'";
            }
            if (baseTypeCode.length() > 0) {
                strwhere += " and base_type_code = '";
                strwhere += baseTypeCode;
                strwhere += "'";
            }


            if (strwhere.length() > 0) {
                db2.sql_bind_reset();
                db2.sql_reset();
                sql = "select count(item_code) as cnt from cy_baseitem_hdr where item_code = ";
                sql += db2.sql_bind(1, item_code);
                sql += strwhere;


                if (!db2.sql_result (sql,true))
                    return _util->jsonerror("Unable to retrieve item information.",db2.errordb());

                //////////////////////////////////////
                //check if item is discounted
                if (atoi(db2.sql_field_value("cnt").c_str()) > 0){

                    jacctPromo.clear ();
                    for (i = 0; i < db.numcols(); i++)
                    jacctPromo[db.sql_field_name(i)]=db.sql_field_value (db.sql_field_name(i));
                    break;

            }

    }

        db.sql_next ();
    }

    //////////////////////////////////////
    //  Spit out the JSON output
    stringstream out;
    if(!jacctPromo.isNull()){
        out << jacctPromo << endl;
        return out.str ();
    }else
        return _util->jsonerror("Unable to retrieve the account discount information.","No records(s) retrieved.");


}*/






/***************************************************************************
      FUNCTION:          json_select_single
   DESCRIPTION:          JSON formatted single row query
 ***************************************************************************/
bool            cypostrx::json_select_single (CYDbSql* db,
                                                           Json::Value& jval,
                                                           string sql)
{
    if (!db->sql_result (sql,true))
        return false;// seterrorquery(db->errordb());
    if (db->eof ())  {
        _util->jsonerror("No record(s) found");
        //seterrorquery("No record(s) found");
        return false;
    }

    jval.clear ();
    string name, val;
    for (int i = 0; i < db->numcols(); i++)  {
        name = db->sql_field_name(i);
        val = db->sql_field_value(name);
        jval[name]=val;
    }
    return true;
}

/***************************************************************************
      FUNCTION:          json_select_multiple
   DESCRIPTION:          JSON formatted multi row query
 ***************************************************************************/
bool            cypostrx::json_select_multiple (CYDbSql* db,
                                                             Json::Value& jval,
                                                             string sql)
{
    //db->sql_reset ();
    if (!db->sql_result (sql,db->m_bind.m_count > 0))  {
        _util->jsonerror(db->errordb().c_str());        
        return false;// seterrorquery(db->errordb());
    }
    if (db->eof ())
        return false; // salesQueryType::SQT_NOROWS;

    int idx = 0;
    jval.clear ();
    string name, val;
    Json::Value jline;
    jline.clear ();

    do  {
        for (int i = 0; i < db->numcols(); i++)  {
            name = db->sql_field_name(i);
            val = db->sql_field_value(name);
            jline[name]=val;
        }
        jval[idx] = jline;
        idx++;
        db->sql_next ();
    } while (!db->eof ());
    return true;
}
