/********************************************************************
          FILE:         eod_retrieve.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  Header file(s)
#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:          rest_loyalty_itewm_populate
   DESCRIPTION:          Populate loyalty account(s) assigned to a product
 ***************************************************************************/
bool                     cyposrest::sql_loyalty_item_populate()
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbacct = _ini->dbconn ();
    if (!dbacct->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get the item code
    string item_code = cgiform("item_code");
    if (item_code.length() < 1)
        return seterrormsg("The item code / sku is a required entry");
    //////////////////////////////////////
    //  Get the location code
    string location_code = cgiform("location_code");
    if (location_code.length() < 1)
        return seterrormsg("The location code is a required entry");
    //////////////////////////////////////
    //  For each loyalty account
    db->sql_reset ();
    dbsel->sql_reset ();
    string sql = "select * from tg_card_type where is_loyalty = 1 ";
    if (!dbsel->sql_result (sql,false))
        return seterrormsg(dbsel->errordb());
    if (dbsel->eof())
        return true;
    do  {
        string type = dbsel->sql_field_value("acct_type_code");
        //////////////////////////////////////
        //  Does the record exist?
        dbacct->sql_reset();
        string sql = "select count(*) as recs from cy_baseitem_detail_points ";
        sql += "where item_code = ";  sql += dbacct->sql_bind(1, item_code);
        sql += " and  location_code = ";  sql += dbacct->sql_bind(2, location_code);
        sql += " and  acct_type_code = ";  sql += dbacct->sql_bind(3, type);
        if (!dbacct->sql_result(sql,true))
            return seterrormsg(db->errordb());
        string count = dbacct->sql_field_value("recs");
        bool accountExists = (atoi(count.c_str()) > 0);
        //////////////////////////////////////
        //  Save the account
        int idx = 1;
        if (!accountExists)  {
            sql  = "insert into cy_baseitem_detail_points (acct_type_code, ";
            sql += "item_code, location_code, points_per_peso) values (";
            sql += db->sql_bind(idx,type); sql += ", "; idx++;
            sql += db->sql_bind(idx,item_code); sql += ", "; idx++;
            sql += db->sql_bind(idx,location_code); sql += ", 0)"; idx++;
        }
        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());
        dbsel->sql_next ();
    }  while (!dbsel->eof ());
    //////////////////////////////////////
    //  Save it...
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
    return true;
}

