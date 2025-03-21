/*****************************************************************************
          FILE:          cyrestsalescurrency.cpp
   DESCRIPTION:          Schema implementaion
 *****************************************************************************/
#include "cyrestsalescurrency.h"
/***************************************************************************
      FUNCTION:          map_argument_cgi
   DESCRIPTION:          Map a single CGI argument
 ***************************************************************************/
bool                     cyrestsalescurrency::map_argument_cgi (std::string arg,
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
bool                     cyrestsalescurrency::map_argument_cgikey (std::string suffix,
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
bool                     cyrestsalescurrency::map_arguments (CYDbSql* db,
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
void                     cyrestsalescurrency::reset () {
  get_column ("systransnum")->set_value ("");
  get_column ("currency_code")->set_value ("");
  get_column ("description")->set_value ("");
  get_column ("is_multiply")->set_value ("0");
  get_column ("default_rate")->set_value ("0");
}
/***************************************************************************
      FUNCTION:          copy
   DESCRIPTION:          Object copy
 ***************************************************************************/
bool                     cyrestsalescurrency::copy (cyrestsalescurrency obj) {
  if (!set_systransnum (obj.get_systransnum ()))  return false;
  if (!set_currency_code (obj.get_currency_code ()))  return false;
  if (!set_description (obj.get_description ()))  return false;
  if (!set_is_multiply (obj.get_is_multiply ()))  return false;
  if (!set_default_rate (obj.get_default_rate ()))  return false;
  return true;
}
/***************************************************************************
      FUNCTION:          string_insert
   DESCRIPTION:          Generate a INSERT statement for table export
 ***************************************************************************/
std::string              cyrestsalescurrency::string_insert ()  {
  std::stringstream sql;
  sql << "insert into ";
  sql << schema_table ();
  sql << " (systransnum, currency_code, description, is_multiply";
  sql << ", default_rate)  values (";
  sql << "'";  sql << get_column ("systransnum")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("currency_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("description")->get_value ();  sql << "',";
  sql << get_column ("is_multiply")->get_value ();  sql << ",";
  sql << get_column ("default_rate")->get_value ();  sql << ")";
  return sql.str ();
}
  /////////////////////////////////////
  //  Create the column list
  bool                   cyrestsalescurrency::column_array ()
  {
    CYColumn             *column;
    /////////////////////////////////////
    //  Database table
    m_schema_name  = "cyrestsalescurrency";
    m_schema_table = "tg_pos_mobile_currency";
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
    //  Column - currency_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 20;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 10;
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

    column->_column_name        = "currency_code";
    column->_col_label          = "Currency Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - description
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

    column->_column_name        = "description";
    column->_col_label          = "Description";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_multiply
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 40;
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

    column->_column_name        = "is_multiply";
    column->_col_label          = "Is Multiply";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - default_rate
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 50;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 15;
    column->_col_decimal        = 2;
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

    column->_column_name        = "default_rate";
    column->_col_label          = "Default Rate";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    return true;
  }
