/*****************************************************************************
          FILE:          cyrestsalesaccount.cpp
   DESCRIPTION:          Schema implementaion
 *****************************************************************************/
#include "cyrestsalesaccount.h"
/***************************************************************************
      FUNCTION:          map_argument_cgi
   DESCRIPTION:          Map a single CGI argument
 ***************************************************************************/
bool                     cyrestsalesaccount::map_argument_cgi (std::string arg,
                                                      std::string name,
                                                      std::string suffix,
                                                      bool validate)  {
    std::string cgiarg  = arg;
                cgiarg += suffix;
    std::string tmp = _rest->cgiform (cgiarg.c_str ());
    CYColumn* col = get_column (name.c_str ());

    ////////////////////////////////////////
    //  Are we validating???
    if (!validate)  {
      col->_value = tmp;
    }  else  {
      ////////////////////////////////
      //  Primary key?
      if (col->_col_key)  {
        if (!valid_key(col->_column_name, tmp))
          return false;
      }  else  {
        switch (col->_col_type)  {
          case COLTYPE_ALPHA:
          case COLTYPE_PASSWD:
          case COLTYPE_DEFAULT:
          case COLTYPE_DROPALPHA:
            if (!valid_alphanum(col->_column_name,tmp))
              return false;
            break;
          case COLTYPE_NUMERIC:
          case COLTYPE_DROPNUMERIC:
            if (!valid_number(col->_column_name,tmp))
              return false;
            break;
          case COLTYPE_DECIMAL:
            if (!valid_decimal(col->_column_name,tmp))
            return false;
          break;
          case COLTYPE_YESNO:
            if (!valid_flag(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_DATE:
            if (!valid_date(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_TIME:
            if (!valid_time(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_EMAIL:
            if (!valid_email(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_MOBILE:
            if (!valid_number(col->_column_name,tmp))
              return false;
          break;
          default:
            if (!valid_alphanum(col->_column_name,tmp))
              return false;
          break;
        }
      }
    }
  return true;
}
/***************************************************************************
      FUNCTION:          map_argument_cgikey
   DESCRIPTION:          Map all primary key column(s) from CGI
 ***************************************************************************/
bool                     cyrestsalesaccount::map_argument_cgikey (std::string suffix,
                                                         bool validate)  {
  for (int i = 0; i < size (); i++)  {
    CYColumn* col = m_column[i];
    std::string tmp, colname  = col->_column_name;
                     colname += suffix;
    tmp = _rest->cgiform (colname);
    ////////////////////////////////////
    //  Primary key???
    if (col->_col_key)  {
      if (!validate)  {
        col->_value = tmp;
      }  else  {
        if (!valid_key(col->_column_name, tmp))
          return false;
      }
    }
  }
  return true;
}
/***************************************************************************
      FUNCTION:          map_arguments
   DESCRIPTION:          Map all column(s) from CGI or a database
 ***************************************************************************/
bool                     cyrestsalesaccount::map_arguments (CYDbSql* db,
                                                   std::string suffix,
                                                   bool validate)  {
  for (int i = 0; i < size (); i++)  {
    CYColumn* col = m_column[i];
    std::string tmp, colname  = col->_column_name;
                     colname += suffix;
    if (nullptr == db)
      tmp = _rest->cgiform (colname);
    else
      tmp = db->sql_field_value (colname,col->_col_defvalue);
    ////////////////////////////////////
    //  Are we validating???
    if (!validate)  {
      col->_value = tmp;
    }  else  {
      ////////////////////////////////
      //  Primary key?
      if (col->_col_key)  {
        if (!valid_key(col->_column_name, tmp))
          return false;
      }  else  {
        switch (col->_col_type)  {
          case COLTYPE_ALPHA:
          case COLTYPE_PASSWD:
          case COLTYPE_DEFAULT:
          case COLTYPE_DROPALPHA:
            if (!valid_alphanum(col->_column_name,tmp))
              return false;
            break;
          case COLTYPE_NUMERIC:
          case COLTYPE_DROPNUMERIC:
            if (!valid_number(col->_column_name,tmp))
              return false;
            break;
          case COLTYPE_DECIMAL:
            if (!valid_decimal(col->_column_name,tmp))
            return false;
          break;
          case COLTYPE_YESNO:
            if (!valid_flag(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_DATE:
            if (!valid_date(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_TIME:
            if (!valid_time(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_EMAIL:
            if (!valid_email(col->_column_name,tmp))
              return false;
          break;
          case COLTYPE_MOBILE:
            if (!valid_number(col->_column_name,tmp))
              return false;
          break;
          default:
            if (!valid_alphanum(col->_column_name,tmp))
              return false;
          break;
        }
      }
    }
  }
  return true;
}
/***************************************************************************
      FUNCTION:          reset
   DESCRIPTION:          Reset all column value(s)
 ***************************************************************************/
void                     cyrestsalesaccount::reset () {
  get_column ("systransnum")->set_value ("");
  get_column ("customer_id")->set_value ("");
  get_column ("account_number")->set_value ("");
  get_column ("account_type_code")->set_value ("");
  get_column ("account_type_desc")->set_value ("");
  get_column ("first_name")->set_value ("");
  get_column ("middle_name")->set_value ("");
  get_column ("last_name")->set_value ("");
  get_column ("points_earned")->set_value ("0");
  get_column ("is_loyalty")->set_value ("0");
  get_column ("is_clerk")->set_value ("0");
  get_column ("is_agent")->set_value ("0");
  get_column ("is_shopper")->set_value ("0");
}
/***************************************************************************
      FUNCTION:          copy
   DESCRIPTION:          Object copy
 ***************************************************************************/
bool                     cyrestsalesaccount::copy (cyrestsalesaccount obj) {
  if (!set_systransnum (obj.get_systransnum ()))  return false;
  if (!set_customer_id (obj.get_customer_id ()))  return false;
  if (!set_account_number (obj.get_account_number ()))  return false;
  if (!set_account_type_code (obj.get_account_type_code ()))  return false;
  if (!set_account_type_desc (obj.get_account_type_desc ()))  return false;
  if (!set_first_name (obj.get_first_name ()))  return false;
  if (!set_middle_name (obj.get_middle_name ()))  return false;
  if (!set_last_name (obj.get_last_name ()))  return false;
  if (!set_points_earned (obj.get_points_earned ()))  return false;
  if (!set_is_loyalty (obj.get_is_loyalty ()))  return false;
  if (!set_is_clerk (obj.get_is_clerk ()))  return false;
  if (!set_is_agent (obj.get_is_agent ()))  return false;
  if (!set_is_shopper (obj.get_is_shopper ()))  return false;
  return true;
}
/***************************************************************************
      FUNCTION:          string_insert
   DESCRIPTION:          Generate a INSERT statement for table export
 ***************************************************************************/
std::string              cyrestsalesaccount::string_insert ()  {
  std::stringstream sql;
  sql << "insert into ";
  sql << schema_table ();
  sql << " (systransnum, customer_id, account_number, account_type_code";
  sql << ", account_type_desc, first_name, middle_name, last_name";
  sql << ", points_earned, is_loyalty, is_clerk, is_agent";
  sql << ", is_shopper)  values (";
  sql << "'";  sql << get_column ("systransnum")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("customer_id")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_number")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_type_desc")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("first_name")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("middle_name")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("last_name")->get_value ();  sql << "',";
  sql << get_column ("points_earned")->get_value ();  sql << ",";
  sql << get_column ("is_loyalty")->get_value ();  sql << ",";
  sql << get_column ("is_clerk")->get_value ();  sql << ",";
  sql << get_column ("is_agent")->get_value ();  sql << ",";
  sql << get_column ("is_shopper")->get_value ();  sql << ")";
  return sql.str ();
}
  /////////////////////////////////////
  //  Create the column list
  bool                   cyrestsalesaccount::column_array ()
  {
    CYColumn             *column;
    /////////////////////////////////////
    //  Database table
    m_schema_name  = "cyrestsalesaccount";
    m_schema_table = "tg_pos_mobile_account";
    /////////////////////////////////////
    //  Cleanup the column list
    cleanup ();
    ///////////////////////////////////////
    //  Column - systransnum
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 10;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 64;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 1;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 0;
    column->_col_display        = 0;
    column->_col_select_where   = 1;
    column->_col_update_where   = 1;
    column->_col_delete_where   = 1;
    column->_col_display_search = 1;

    column->_column_name        = "systransnum";
    column->_col_label          = "Systransnum";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - customer_id
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 20;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 64;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 1;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 0;
    column->_col_display        = 0;
    column->_col_select_where   = 1;
    column->_col_update_where   = 1;
    column->_col_delete_where   = 1;
    column->_col_display_search = 1;

    column->_column_name        = "customer_id";
    column->_col_label          = "Customer Id";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - account_number
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 30;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 64;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "account_number";
    column->_col_label          = "Account Number";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - account_type_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 40;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 20;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "account_type_code";
    column->_col_label          = "Account Type Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - account_type_desc
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 50;
    column->_col_minlen         = 0;
    column->_col_maxlen         = 128;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = -99999;
    column->_col_maxval         = 99999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "account_type_desc";
    column->_col_label          = "Description";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - first_name
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 60;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 32;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "first_name";
    column->_col_label          = "First Name";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - middle_name
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 70;
    column->_col_minlen         = 0;
    column->_col_maxlen         = 24;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "middle_name";
    column->_col_label          = "Middle Name";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - last_name
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 80;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 64;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "last_name";
    column->_col_label          = "Last Name";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - points_earned
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 90;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 15;
    column->_col_decimal        = 2;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 99999999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "points_earned";
    column->_col_label          = "Points Earned";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_loyalty
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 100;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 1;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "is_loyalty";
    column->_col_label          = "Is Loyalty";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_clerk
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 110;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 1;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "is_clerk";
    column->_col_label          = "Is Clerk";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_agent
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 120;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 1;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "is_agent";
    column->_col_label          = "Is Agent";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_shopper
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 130;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 1;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999;
    column->_col_key            = 0;
    column->_col_sort           = 0;
    column->_col_insert         = 1;
    column->_col_select         = 1;
    column->_col_update         = 1;
    column->_col_display        = 0;
    column->_col_select_where   = 0;
    column->_col_update_where   = 0;
    column->_col_delete_where   = 0;
    column->_col_display_search = 0;

    column->_column_name        = "is_shopper";
    column->_col_label          = "Is Shopper";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    return true;
  }
