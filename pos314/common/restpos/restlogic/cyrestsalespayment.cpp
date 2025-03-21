/*****************************************************************************
          FILE:          cyrestsalespayment.cpp
   DESCRIPTION:          Schema implementaion
 *****************************************************************************/
#include "cyrestsalespayment.h"
/***************************************************************************
      FUNCTION:          Constructor
   DESCRIPTION:          Initialize
 ***************************************************************************/
cyrestsalespayment::cyrestsalespayment (cyposrest* rest)
{
    _rest = rest;
    _discount = nullptr;
    
    column_array ();
    _discount = new cyrestsalesdiscountpayment (rest);
}
/***************************************************************************
      FUNCTION:          Destructor
   DESCRIPTION:          Cleanup
 ***************************************************************************/
cyrestsalespayment::~cyrestsalespayment ()
{
    if (nullptr != _discount)
        delete _discount;
}
/***************************************************************************
      FUNCTION:          map_argument_cgi
   DESCRIPTION:          Map a single CGI argument
 ***************************************************************************/
bool                     cyrestsalespayment::map_argument_cgi (std::string arg,
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
bool                     cyrestsalespayment::map_argument_cgikey (std::string suffix,
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
bool                     cyrestsalespayment::map_arguments (CYDbSql* db,
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
void                     cyrestsalespayment::reset () {
  _discount->reset();
  get_column ("systransnum")->set_value ("");
  get_column ("pay_seq")->set_value ("0");
  get_column ("pay_code")->set_value ("");
  get_column ("description")->set_value ("");
  get_column ("currency_code")->set_value ("");
  get_column ("discount_code")->set_value ("");
  get_column ("discount_amount")->set_value ("0");
  get_column ("payment_amount")->set_value ("0");
  get_column ("converted_payment_amount")->set_value ("0");
  get_column ("account_type_code")->set_value ("");
  get_column ("account_number")->set_value ("");
  get_column ("approval_code")->set_value ("");
  get_column ("approval_merchant")->set_value ("");
  get_column ("change_code")->set_value ("");
  get_column ("change_currency_code")->set_value ("");
  get_column ("change_amount")->set_value ("0");
}
/***************************************************************************
      FUNCTION:          copy
   DESCRIPTION:          Object copy
 ***************************************************************************/
bool                     cyrestsalespayment::copy (cyrestsalespayment obj) {
  if (!set_systransnum (obj.get_systransnum ()))  return false;
  if (!set_pay_seq (obj.get_pay_seq ()))  return false;
  if (!set_pay_code (obj.get_pay_code ()))  return false;
  if (!set_description (obj.get_description ()))  return false;
  if (!set_currency_code (obj.get_currency_code ()))  return false;
  if (!set_discount_code (obj.get_discount_code ()))  return false;
  if (!set_discount_amount (obj.get_discount_amount ()))  return false;
  if (!set_payment_amount (obj.get_payment_amount ()))  return false;
  if (!set_converted_payment_amount (obj.get_converted_payment_amount ()))  return false;
  if (!set_account_type_code (obj.get_account_type_code ()))  return false;
  if (!set_account_number (obj.get_account_number ()))  return false;
  if (!set_approval_code (obj.get_approval_code ()))  return false;
  if (!set_approval_merchant (obj.get_approval_merchant ()))  return false;
  if (!set_change_code (obj.get_change_code ()))  return false;
  if (!set_change_currency_code (obj.get_change_currency_code ()))  return false;
  if (!set_change_amount (obj.get_change_amount ()))  return false;
  return true;
}
/***************************************************************************
      FUNCTION:          string_insert
   DESCRIPTION:          Generate a INSERT statement for table export
 ***************************************************************************/
std::string              cyrestsalespayment::string_insert ()  {
  std::stringstream sql;
  sql << "insert into ";
  sql << schema_table ();
  sql << " (systransnum, pay_seq, pay_code, description";
  sql << ", currency_code, discount_code, discount_amount, payment_amount";
  sql << ", converted_payment_amount, account_type_code, account_number, approval_code";
  sql << ", approval_merchant, change_code, change_currency_code, change_amount";
  sql << ")  values (";
  sql << "'";  sql << get_column ("systransnum")->get_value ();  sql << "',";
  sql << get_column ("pay_seq")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("pay_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("description")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("currency_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("discount_code")->get_value ();  sql << "',";
  sql << get_column ("discount_amount")->get_value ();  sql << ",";
  sql << get_column ("payment_amount")->get_value ();  sql << ",";
  sql << get_column ("converted_payment_amount")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("account_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_number")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("approval_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("approval_merchant")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("change_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("change_currency_code")->get_value ();  sql << "',";
  sql << get_column ("change_amount")->get_value ();  sql << ")";
  return sql.str ();
}
  /////////////////////////////////////
  //  Create the column list
  bool                   cyrestsalespayment::column_array ()
  {
    CYColumn             *column;
    /////////////////////////////////////
    //  Database table
    m_schema_name  = "cyrestsalespayment";
    m_schema_table = "tg_pos_mobile_payment";
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
    //  Column - pay_seq
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 20;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 12;
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

    column->_column_name        = "pay_seq";
    column->_col_label          = "Pay Seq";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - pay_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 30;
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

    column->_column_name        = "pay_code";
    column->_col_label          = "Pay Code";
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
    column->_col_seq            = 40;
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
    //  Column - currency_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 50;
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
    //  Column - discount_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 60;
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

    column->_column_name        = "discount_code";
    column->_col_label          = "Discount Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - discount_amount
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 70;
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

    column->_column_name        = "discount_amount";
    column->_col_label          = "Discount Amount";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - payment_amount
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 80;
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

    column->_column_name        = "payment_amount";
    column->_col_label          = "Payment Amount";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - converted_payment_amount
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 90;
    column->_col_minlen         = 0;
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

    column->_column_name        = "converted_payment_amount";
    column->_col_label          = "Converted Payment Amount";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - account_type_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 100;
    column->_col_minlen         = 0;
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
    //  Column - account_number
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 110;
    column->_col_minlen         = 0;
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
    //  Column - approval_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 120;
    column->_col_minlen         = 0;
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

    column->_column_name        = "approval_code";
    column->_col_label          = "Approval Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - approval_merchant
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 130;
    column->_col_minlen         = 0;
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

    column->_column_name        = "approval_merchant";
    column->_col_label          = "Approval Merchant";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - change_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 140;
    column->_col_minlen         = 0;
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

    column->_column_name        = "change_code";
    column->_col_label          = "Change Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - change_currency_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 150;
    column->_col_minlen         = 0;
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

    column->_column_name        = "change_currency_code";
    column->_col_label          = "Change Currency Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - change_amount
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 160;
    column->_col_minlen         = 0;
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

    column->_column_name        = "change_amount";
    column->_col_label          = "Change Amount";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    return true;
  }
