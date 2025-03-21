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
bool                     cyposrest::sql_product_save()
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get the item code
    string item_code = cgiform("item_code");
    if (item_code.length() < 1)
        return seterrormsg("The item code / sku is a required entry");
    //////////////////////////////////////
    //  Does the header record exist?
    db->sql_reset();
    string sql = "select count(*) as recs from cy_baseitem_hdr where item_code = ";
    sql += db->sql_bind(1, item_code);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    string count = db->sql_field_value("recs");
    bool headerExists = (atoi(count.c_str()) > 0);
    //////////////////////////////////////
    //  Does the recipe record exist?
    db->sql_reset();
    sql = "select count(*) as recs from tg_recipe_item where recipe_item_code = ";
    sql += db->sql_bind(1, item_code);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    count = db->sql_field_value("recs");
    bool recipeExists = (atoi(count.c_str()) > 0);
    //////////////////////////////////////
    //  Save the product information
    int idx = 1;
    db->sql_reset ();
    if (headerExists)  {
        sql  = "update cy_baseitem_hdr set ";
        sql += "  category_cd = "; sql += db->sql_bind(idx,cgiform("category_cd")); idx++;
        sql += ", subcat_cd = "; sql += db->sql_bind(idx,cgiform("subcat_cd")); idx++;
        sql += ", class_cd = "; sql += db->sql_bind(idx,cgiform("class_cd")); idx++;
        sql += ", subclass_cd = "; sql += db->sql_bind(idx,cgiform("subclass_cd")); idx++;
        sql += ", type_code = "; sql += db->sql_bind(idx,cgiform("type_code")); idx++;
        sql += ", base_type_code = "; sql += db->sql_bind(idx,cgiform("base_type_code")); idx++;
        sql += ", description = "; sql += db->sql_bind(idx,cgiform("description")); idx++;
        sql += ", is_active = "; sql += db->sql_bind(idx,cgiform("is_active")); idx++;
        sql += ", is_consignment = "; sql += db->sql_bind(idx,cgiform("is_consignment")); idx++;
        sql += ", default_uom_buy = "; sql += db->sql_bind(idx,cgiform("default_uom_buy")); idx++;
        sql += ", default_uom_sell = "; sql += db->sql_bind(idx,cgiform("default_uom_sell")); idx++;
        sql += ", buy_uom_type_code = "; sql += db->sql_bind(idx,cgiform("type_code")); idx++;
        sql += "  where item_code = "; sql += db->sql_bind(idx,item_code); idx++;
    }  else  {
        sql  = "insert into cy_baseitem_hdr (category_cd, subcat_cd, class_cd, subclass_cd, ";
        sql += "type_code, base_type_code, description, is_active, is_consignment, default_uom_buy, ";
        sql += "default_uom_sell, buy_uom_type_code, item_code) values (";
        sql += db->sql_bind(idx,cgiform("category_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("subcat_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("class_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("subclass_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("type_code")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("base_type_code")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("description")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("is_active")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("is_consignment")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("default_uom_buy")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("default_uom_sell")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("type_code")); sql += ", "; idx++;
        sql += db->sql_bind(idx,item_code); sql += ") "; idx++;
    }
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    //////////////////////////////////////
    //  Save the FNB information
    idx = 1;
    db->sql_bind_reset ();
    if (recipeExists)  {
        sql  = "update tg_recipe_item set ";
        sql += "  category_code = "; sql += db->sql_bind(idx,cgiform("class_cd")); idx++;
        sql += ", subcat_code = "; sql += db->sql_bind(idx,cgiform("subclass_cd")); idx++;
        sql += ", sort_order = "; sql += db->sql_bind(idx,cgiform("sort_order")); idx++;
        sql += ", uom_type_code = "; sql += db->sql_bind(idx,cgiform("type_code")); idx++;
        sql += ", description = "; sql += db->sql_bind(idx,cgiform("description")); idx++;
        sql += "  where recipe_item_code = "; sql += db->sql_bind(idx,item_code); idx++;
    }  else  {
        sql  = "insert into tg_recipe_item (category_code, subcat_code, sort_order, ";
        sql += "uom_type_code, description, recipe_item_code, suggested_retail_price, ";
        sql += "price01, price02, price03, recipe_type_code) values (";
        sql += db->sql_bind(idx,cgiform("class_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("subclass_cd")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("sort_order")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("type_code")); sql += ", "; idx++;
        sql += db->sql_bind(idx,cgiform("description")); sql += ", "; idx++;
        sql += db->sql_bind(idx,item_code); sql += ", 0.00, 0.00, 0.00, 0.00, 4) "; idx++;
    }
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    if (!db->sql_commit())
        return seterrormsg(db->errordb());
    return true;
}

