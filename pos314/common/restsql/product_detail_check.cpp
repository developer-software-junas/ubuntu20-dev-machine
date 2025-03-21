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
      FUNCTION:          eod_retrieve
   DESCRIPTION:          Retrieve EOD totals
 ***************************************************************************/
bool                     cyposrest::sql_product_detail_check()
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get arguments
    string item_code = cgiform("item_code");
    if (item_code.length() < 1)
        return seterrormsg("The item code / sku is a required entry");
    string location_code = cgiform("location_code");
    if (location_code.length() < 1)
        return seterrormsg("The location code is a required entry");
    //////////////////////////////////////
    //  Does the header record exist?
    db->sql_reset();
    string sql = "select * from cy_baseitem_hdr where item_code = ";
    sql += db->sql_bind(1, item_code);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    if (db->eof ())
        return seterrormsg("Unable to retrieve the requested item");
    string type_code = db->sql_field_value("type_code");
    string uom_code = db->sql_field_value("default_uom_sell");
    string description = db->sql_field_value("description");
    //////////////////////////////////////
    //  Does the recipe record exist?
    db->sql_reset();
    sql = "select * from tg_recipe_item where recipe_item_code = ";
    sql += db->sql_bind(1, item_code);
    string retail_price = "0.00";
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    if (!db->eof ())  {
        retail_price = db->sql_field_value("suggested_retail_price");
    }
    //////////////////////////////////////
    //  Does the detail item exist
    db->sql_reset();
    sql = "select * from cy_baseitem_detail where item_code = ";
    sql += db->sql_bind(1, item_code);
    sql += " and location_code = ";
    sql += db->sql_bind(2, location_code);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    bool detailAdd = db->eof();
    //////////////////////////////////////
    //  Save the detail record
    CYDbSql* dbupd = _ini->dbconn ();
    if (nullptr == dbupd)
        return seterrormsg(_ini->errormsg());

    int idx = 1;
    dbupd->sql_reset ();
    if (detailAdd)  {
        sql  = "insert into cy_baseitem_detail (location_code, item_code, ";
        sql += "type_code, uom_code, retail_price, quantity, cost, avg_cost, ";
        sql += "low_quantity, high_quantity, xitem, qty_on_order, is_local, ";
        sql += "is_price_prompt, is_individual_line, is_vat, commission_rate, ";
        sql += "is_service_charge, total_service_charge, service_charge, ";
        sql += "service_charge_vat, is_auto_servicecharge) values (";
        sql += dbupd->sql_bind(idx,location_code); sql += ", "; idx++;
        sql += dbupd->sql_bind(idx,item_code); sql += ", "; idx++;
        sql += dbupd->sql_bind(idx,type_code); sql += ", "; idx++;
        sql += dbupd->sql_bind(idx,uom_code); sql += ", "; idx++;
        sql += dbupd->sql_bind(idx,retail_price); sql += ", "; idx++;
        sql += "0.00, 0.00, 0.00, 1, 999999999, 0, 0.00, 1, 0, 0, 0, 0.00, 0, 0.00, 0, 0, 0 ";
        sql += ") "; idx++;
        if (!dbupd->sql_only(sql,true))
            return seterrormsg(dbupd->errordb());
/*
        idx = 1;
        dbupd->sql_bind_reset();
        sql  = "insert into cy_baseitem_detail_points (location_code, item_code, ";
        sql += "acct_type_code, points_per_peso) values (";
        sql += dbupd->sql_bind(idx,location_code); sql += ", "; idx++;
        sql += dbupd->sql_bind(idx,item_code); sql += ", 'FSP', 1.00) ";
        if (!dbupd->sql_only(sql,true))
            return seterrormsg(db->errordb());
*/
        if (!dbupd->sql_commit())
            return seterrormsg(db->errordb());
    }
    return true;
}

