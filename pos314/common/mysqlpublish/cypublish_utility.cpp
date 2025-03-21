/********************************************************************
          FILE:         cypublish_utility.cpp
   DESCRIPTION:         Utility function(s)
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cypublish.h"
#include <string>
#include <sstream>
#include "cytable.h"
using std::string;
using std::stringstream;
/*******************************************************************
      FUNCTION:          appendBatch
   DESCRIPTION:          Create the masterfile sql file
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     CYPublish::appendBatch (bool reset, CYTable* table,
                                                 string& columnList)
{
    //////////////////////////////////////
    //  SQL filename
    FILE *f;
    string path = _ini->get_value("PATH","PUBLISH");
    path += "/";
    string tmp, fileName = path; fileName += "createdb.sql";
    //////////////////////////////////////
    //  FILE MODE
    if (reset) tmp = "w";
    else tmp = "a";
    //////////////////////////////////////
    //  Open the SQL file
    f = fopen (fileName.c_str (),tmp.c_str ());
    if (nullptr == f)  {
        _error  = "Unable to open the file: ";
        _error += fileName;
        _error += strerror(errno);
        return false;
    }
    //////////////////////////////////////
    //  Create database???
    if (reset)  {
        tmp  = "SET GLOBAL local_infile=1;\n";
        tmp += "DROP DATABASE IF EXISTS offline;\n";
        //tmp += _locationCode; tmp += "_";
        //tmp += _logicalDate; tmp += ";\n";

        tmp += "CREATE DATABASE offline;";
        //tmp += _locationCode; tmp += "_";
        //tmp += _logicalDate; tmp += ";\n";

        tmp += "use offline;";
        //tmp += _locationCode; tmp += "_";
        //tmp += _logicalDate; tmp += ";\n\n\n";

        fwrite (tmp.c_str(), 1, tmp.length(), f);
        fclose (f);
        _cmdLine->generateSchema(_locationCode,_logicalDate);
    }
    //////////////////////////////////////
    //  Assemble the mysql content
    string dataFile = path;
    dataFile += table->tableName();
    dataFile += ".csv.sql";

    //////////////////////////////////////
    //  Open the SQL file
    f = fopen (fileName.c_str (),"a");
    if (nullptr == f)  {
        _error  = "Unable to open the file: ";
        _error += fileName;
        _error += strerror(errno);
        return false;
    }

    string contents = "LOAD DATA LOCAL INFILE '";
    contents += dataFile;
    contents += "'\n";
    contents += "INTO TABLE ";
    contents += table->tableName();
    contents += "\n CHARACTER SET LATIN1 FIELDS TERMINATED BY ',' ENCLOSED BY '\"'\n(";

    int idx = 0;
    bool done = false;
    columnList = "";
    while (!done)  {
        if (table->getColumn(idx)->columnName.length() < 1)  {
            done = true;
        }  else  {
            //////////////////////////////
            //  First time?
            if (idx != 0)
                columnList += ",";
            columnList += table->getColumn(idx)->columnName;
            idx++;
        }
    }
    contents += columnList;
    contents += ");\n\n";
    fwrite (contents.c_str(), 1, contents.length(), f);
    fclose (f);

    return (table->createTableFile(dataFile,table->tableName(),columnList));
}
/*******************************************************************
      FUNCTION:          appendBatch
   DESCRIPTION:          Create the masterfile sql file
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     CYPublish::appendLiteSchema (bool reset,
                                                      CYTable* table,
                                                      string& columnList)
{
    //////////////////////////////////////
    //  SQL filename
    FILE *flite;
    string path = _ini->get_value("PATH","PUBLISH");
    path += "/";
    string sql, tmp, fileLite = path;

    fileLite += _locationCode; fileLite += ".";
    fileLite += _logicalDate; fileLite += "."; fileLite += "sqlite";
    //////////////////////////////////////
    //  FILE MODE
    if (reset) tmp = "w";
    else tmp = "a";
    //////////////////////////////////////
    //  Open the SQL file
    flite = fopen (fileLite.c_str (),tmp.c_str ());
    if (nullptr == flite)  {
        _error  = "Unable to open the file: ";
        _error += fileLite;
        _error += strerror(errno);
        return false;
    }
    //////////////////////////////////////
    //  Create database???
    if (reset)  {
        tmp  = "pragma synchronous = normal;\n";
        tmp += "pragma temp_store = memory;\n";
        tmp += ".mode csv\n\n";
        fwrite (tmp.c_str(), 1, tmp.length(), flite);
    }
    //////////////////////////////////////
    //  Assemble the mysql content
    string dataFile = path;
    dataFile += table->tableName();
    dataFile += ".csv.sql";
    //////////////////////////////////////
    //  Open the SQL file
    int idx = 0;
    bool done = false;
    columnList = "";
    while (!done)  {
        if (table->getColumn(idx)->columnName.length() < 1)  {
            done = true;
        }  else  {
            //////////////////////////////
            //  First time?
            if (idx != 0)
                columnList += ",";
            columnList += table->getColumn(idx)->columnName;
            idx++;
        }
    }
    //////////////////////////////////////
    //  SQLITE
    tmp = table->createTableSql();
    fwrite (tmp.c_str(), 1, tmp.length(), flite);
    fclose (flite);

    return (table->createTableFile(dataFile,table->tableName(),columnList));
}
/*******************************************************************
      FUNCTION:          getLogicalDate
   DESCRIPTION:          Create the masterfile for POS download
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     CYPublish::getLogicalDate (string& sysdate,
                                                    string& locationCode)
{
    sysdate = "";

    CYDbSql* db = dbconn ();
    if (!db->ok())
        return seterrormsg(db->errordb());

    string sql = "select * from pos_sysdate "
                 "where eod_timestamp is not null and eod_timestamp <> 0 "
                 "order by transaction_date desc limit 1 ";
    if (!db->sql_result (sql,false))
        return seterrormsg(db->errordb());

    sysdate = db->sql_field_value("transaction_date");
    locationCode = db->sql_field_value("location_code");
    return true;
}
/*******************************************************************
      FUNCTION:          appendBatch
   DESCRIPTION:          Create the masterfile sql file
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     CYPublish::appendLiteImport (CYTable* table,
                                                      string& columnList)
{
    (void)columnList;
    //////////////////////////////////////
    //  SQL filename
    FILE *flite;
    string path = _ini->get_value("PATH","PUBLISH");
    path += "/";
    string tmp, fileLite = path;

    fileLite += _locationCode; fileLite += ".";
    fileLite += _logicalDate; fileLite += "."; fileLite += "sqlite";
    //////////////////////////////////////
    //  FILE MODE
    tmp = "a";
    //////////////////////////////////////
    //  Open the SQL file
    flite = fopen (fileLite.c_str (),tmp.c_str ());
    if (nullptr == flite)  {
        _error  = "Unable to open the file: ";
        _error += fileLite;
        _error += strerror(errno);
        return false;
    }
    //////////////////////////////////////
    //  Assemble the mysql content
    string dataFile = path;
    dataFile += table->tableName();
    dataFile += ".csv.sql";
    //////////////////////////////////////
    //  SQLITE
    tmp = "\nBEGIN TRANSACTION;";
    tmp += "\n.import "; tmp += dataFile; tmp += " "; tmp += table->tableName();
    fwrite (tmp.c_str(), 1, tmp.length(), flite);
    tmp = "\n"; tmp += table->createTableIndex();
    tmp += "\nCOMMIT;\n\n";
    fwrite (tmp.c_str(), 1, tmp.length(), flite);
    fclose (flite);

    return true;
}
/***************************************************************************
      FUNCTION:         tableFilter
   DESCRIPTION:         Exclude table(s)
     ARGUMENTS:         none
       RETURNS:         list of excluded table(s) in SQL format
 ***************************************************************************/
string                  CYPublish:: tableFilter ()
{
    string sql;

    sql  = " (";
    //sql += "'tg_card_type', ";
    //sql += "'sc_card_type' ";
    //sql += ") ";
    //return sql;


    sql += "'tg_branch_area', ";
    sql += "'tg_branch_section', ";
    sql += "'tg_branch_table', ";
    sql += "'tg_card_type', ";
    sql += "'tg_modifier', ";
    sql += "'tg_modifier_detail', ";
    sql += "'tg_modifier_subrecipes', ";
    sql += "'tg_pos_cancel_header', ";
    sql += "'tg_pos_daily_header', ";
    sql += "'tg_pos_device', ";
    sql += "'tg_pos_discount_summary', ";
    sql += "'tg_pos_download', ";
    sql += "'tg_pos_mobile_account', ";
    sql += "'tg_pos_mobile_audit', ";
    sql += "'tg_pos_mobile_bank_payment', ";
    sql += "'tg_pos_mobile_currency', ";
    sql += "'tg_pos_mobile_detail', ";
    sql += "'tg_pos_mobile_discount', ";
    sql += "'tg_pos_mobile_discount_detail', ";
    sql += "'tg_pos_mobile_discount_payment', ";
    sql += "'tg_pos_mobile_gc_payment', ";
    sql += "'tg_pos_mobile_header', ";
    sql += "'tg_pos_mobile_payment', ";
    sql += "'tg_pos_mobile_queue', ";
    sql += "'tg_pos_mobile_split', ";
    sql += "'tg_pos_mobile_tax', ";
    sql += "'tg_pos_mobile_transtype', ";
    sql += "'tg_pos_mobile_trxaccount', ";
    sql += "'tg_pos_mobile_xitem', ";
    sql += "'tg_pos_mode', ";
    sql += "'tg_pos_mode_branch', ";
    //sql += "'tg_pos_nonsales_trx', ";
    sql += "'tg_pos_publish', ";
    sql += "'tg_pos_suspend_header', ";
    sql += "'tg_pos_void_header', ";
    sql += "'tg_pos_xread_audit', ";
    sql += "'tg_pos_xread_cashier', ";
    sql += "'tg_pos_xread_cashier_bank', ";
    sql += "'tg_pos_xread_cashier_discount', ";
    sql += "'tg_pos_xread_cashier_fund', ";
    sql += "'tg_pos_xread_cashier_payment', ";
    sql += "'tg_pos_xread_cashier_pullout', ";
    sql += "'tg_recipe_category', ";
    sql += "'tg_recipe_category_branch', ";
    sql += "'tg_recipe_ingredients', ";
    sql += "'tg_recipe_item', ";
    sql += "'tg_recipe_modifiers', ";
    sql += "'tg_recipe_subcategory', ";
    sql += "'tg_recipe_subrecipes', ";
    sql += "'tg_recipe_type', ";
    sql += "'tg_section_menu_detail', ";
    sql += "'tg_subrecipe', ";
    sql += "'tg_subrecipe_detail', ";
    sql += "'tg_table_customer', ";
    sql += "'tg_table_customer_modifier', ";
    sql += "'tg_table_customer_orders', ";
    sql += "'tg_table_orders', ";
    sql += "'tg_table_split', ";

    sql += "'cy_account_promo', ";
    sql += "'cy_discount_condition', ";
    sql += "'cy_baseitem_detail', ";
    sql += "'cy_baseitem_expiry', ";
    sql += "'cy_baseitem_group', ";
    sql += "'cy_baseitem_promo', ";
    sql += "'cy_baseitem_hdr', ";
    sql += "'cy_baseitem_type', ";
    sql += "'cy_baseitem_tax', ";
    sql += "'cy_baseitem_upc', ";
    sql += "'cy_category', ";
    sql += "'cy_class', ";
    sql += "'cy_company', ";
    sql += "'cy_currency', ";
    sql += "'cy_discount_condition', ";
    sql += "'cy_location', ";
    sql += "'cy_location_type', ";
    sql += "'cy_price_events', ";
    sql += "'cy_promo_condition', ";
    sql += "'cy_promo_result', ";
    sql += "'cy_promo_hdr', ";
    sql += "'cy_subcategory', ";
    sql += "'cy_subclass', ";
    sql += "'cy_sku_group', ";
    sql += "'cy_tax', ";
    sql += "'cy_uom', ";
    sql += "'cy_uom_type', ";
    sql += "'cy_user', ";
    sql += "'cy_user_groups', ";
    sql += "'cy_user_changefund', ";
    sql += "'pos_register', ";
    sql += "'pos_beauty_cat', ";
    sql += "'pos_card_tender', ";
    sql += "'pos_cashier_xread', ";
    sql += "'pos_creditcard', ";
    sql += "'pos_discount', ";
    sql += "'pos_discount_account', ";
    sql += "'pos_discount_card', ";
    sql += "'pos_discount_category', ";
    sql += "'pos_discount_class', ";
    sql += "'pos_discount_group', ";
    sql += "'pos_discount_item', ";
    sql += "'pos_discount_location', ";
    sql += "'pos_discount_subcategory', ";
    sql += "'pos_discount_subclass', ";
    sql += "'pos_discount_tag', ";
    sql += "'pos_discount_tender', ";
    sql += "'pos_discount_type', ";
    sql += "'pos_price', ";
    sql += "'pos_price_item', ";
    sql += "'pos_price_location', ";
    sql += "'pos_price_priority', ";
    sql += "'pos_price_transaction', ";
    sql += "'pos_register', ";
    sql += "'pos_registry', ";
    sql += "'pos_registry_sales', ";
    sql += "'pos_settings', ";
    sql += "'pos_settlement_bank', ";
    sql += "'pos_sysdate', ";
    sql += "'pos_tender', ";
    sql += "'pos_tender_type', ";
    sql += "'pos_trans_card', ";
    sql += "'pos_trans_cardprefix', ";
    sql += "'pos_trans_discount', ";
    sql += "'pos_trans_event', ";
    sql += "'pos_trans_loc', ";
    sql += "'pos_trans_receipt', ";
    sql += "'pos_trans_tender', ";
    sql += "'pos_transtype', ";
    sql += "'pos_modules', ";
    sql += "'pos_modules_receipt', ";
    sql += "'pos_manager_item', ";
    sql += "'pos_module_manager', ";
    sql += "'pos_giftitem_promo', ";
    sql += "'pos_giftitem_promo_dept', ";
    sql += "'pos_giftitem_promo_rcpt', ";
    sql += "'pos_giftitem_promo_redeem', ";
    sql += "'pos_giftitem_promo_sku', ";
    sql += "'pos_giftitem_promo_trans', ";
    sql += "'rg_card_type', ";
    sql += "'rg_customer_type', ";

    sql += "'sc_pin', ";
    sql += "'sc_add_points', ";
    sql += "'sc_add_type', ";
    sql += "'sc_attr', ";
    sql += "'sc_attr_type', ";
    sql += "'sc_card_type', ";
    sql += "'sc_city', ";
    sql += "'sc_state', ";
    sql += "'sc_card_type', ";
    sql += "'sc_customer', ";
    sql += "'sc_customer_card', ";
    sql += "'sc_customer_attr', ";
    sql += "'sc_customer_contact', ";
    sql += "'sc_customer_addr' ";
    sql += ") ";

    return sql;
}
