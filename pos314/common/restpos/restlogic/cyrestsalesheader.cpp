/*****************************************************************************
          FILE:          cyrestsalesheader.cpp
   DESCRIPTION:          Schema implementaion
 *****************************************************************************/
#include "cyrestsalesheader.h"
/***************************************************************************
      FUNCTION:          Constructor
   DESCRIPTION:          Initialize
 ***************************************************************************/
cyrestsalesheader::cyrestsalesheader (cyposrest* rest) : CYColumnArrayBase ()
{
    _rest = rest;
    _currency = nullptr;
    _discount = nullptr;
    _transtype = nullptr;
    
    column_array ();
    _listAcct.empty();
    
    _currency = new cyrestsalescurrency (rest);
    _discount = new cyrestsalesdiscount (rest);
    _transtype = new cyrestsalestranstype (rest);
}
/***************************************************************************
      FUNCTION:          Destructor
   DESCRIPTION:          Cleanup
 ***************************************************************************/
cyrestsalesheader::~cyrestsalesheader ()
{
    if (nullptr != _currency)
        delete _currency;
    if (nullptr != _discount)
        delete _discount;
    if (nullptr != _transtype)
        delete _transtype;

    for (size_t i = 0; i < _listAcct.size(); i++)  {
        cyrestsalesaccount* acct = _listAcct.at(i);
        delete (acct);
    }
    _listAcct.clear ();
    _listAcct.empty ();
    for (size_t i = 0; i < _listDetail.size(); i++)  {
        cyrestsalesdetail* det = _listDetail.at(i);
        delete (det);
    }
    _listDetail.clear ();
    _listDetail.empty ();
    for (size_t i = 0; i < _listPayment.size(); i++)  {
        cyrestsalespayment* pay = _listPayment.at(i);
        delete (pay);
    }
    _listPayment.clear ();
    _listPayment.empty ();
}
/***************************************************************************
      FUNCTION:          map_argument_cgi
   DESCRIPTION:          Map a single CGI argument
 ***************************************************************************/
bool                     cyrestsalesheader::map_argument_cgi (std::string arg,
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
bool                     cyrestsalesheader::map_argument_cgikey (std::string suffix,
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
bool                     cyrestsalesheader::map_arguments (CYDbSql* db,
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
void                     cyrestsalesheader::changetablename (std::string table)
{
    m_schema_table = table;
}
/***************************************************************************
      FUNCTION:          reset
   DESCRIPTION:          Reset all column value(s)
 ***************************************************************************/
void                     cyrestsalesheader::reset () {
  get_column ("systransnum")->set_value ("");
  get_column ("company_code")->set_value ("");
  get_column ("location_code")->set_value ("");
  get_column ("register_number")->set_value ("0");
  get_column ("transaction_number")->set_value ("0");
  get_column ("transaction_date")->set_value ("0");
  get_column ("transaction_time")->set_value ("0");
  get_column ("logical_date")->set_value ("0");
  get_column ("cashier")->set_value ("");
  get_column ("transaction_cd")->set_value ("");
  get_column ("picture_url")->set_value ("");
  get_column ("cancel_flag")->set_value ("0");
  get_column ("is_polled")->set_value ("0");
  get_column ("is_polled2")->set_value ("0");
  get_column ("clerk_type_code")->set_value ("");
  get_column ("agent_type_code")->set_value ("");
  get_column ("shopper_type_code")->set_value ("");
  get_column ("account_type_code")->set_value ("");
  get_column ("clerk_number")->set_value ("");
  get_column ("agent_number")->set_value ("");
  get_column ("shopper_number")->set_value ("");
  get_column ("account_number")->set_value ("");
}
/***************************************************************************
      FUNCTION:          copy
   DESCRIPTION:          Object copy
 ***************************************************************************/
bool                     cyrestsalesheader::copy (cyrestsalesheader obj) {
  if (!set_systransnum (obj.get_systransnum ()))  return false;
  if (!set_company_code (obj.get_company_code ()))  return false;
  if (!set_branch_code (obj.get_branch_code ()))  return false;
  if (!set_register_number (obj.get_register_number ()))  return false;
  if (!set_transaction_number (obj.get_transaction_number ()))  return false;
  if (!set_transaction_date (obj.get_transaction_date ()))  return false;
  if (!set_transaction_time (obj.get_transaction_time ()))  return false;
  if (!set_logical_date (obj.get_logical_date ()))  return false;
  if (!set_cashier (obj.get_cashier ()))  return false;
  if (!set_transaction_code (obj.get_transaction_code ()))  return false;
  if (!set_picture_url (obj.get_picture_url ()))  return false;
  if (!set_cancel_flag (obj.get_cancel_flag ()))  return false;
  if (!set_is_polled (obj.get_is_polled ()))  return false;
  if (!set_is_polled2 (obj.get_is_polled2 ()))  return false;
  if (!set_clerk_type_code (obj.get_clerk_type_code ()))  return false;
  if (!set_agent_type_code (obj.get_agent_type_code ()))  return false;
  if (!set_shopper_type_code (obj.get_shopper_type_code ()))  return false;
  if (!set_account_type_code (obj.get_account_type_code ()))  return false;
  if (!set_clerk_number (obj.get_clerk_number ()))  return false;
  if (!set_agent_number (obj.get_agent_number ()))  return false;
  if (!set_shopper_number (obj.get_shopper_number ()))  return false;
  if (!set_account_number (obj.get_account_number ()))  return false;
  return true;
}
/***************************************************************************
      FUNCTION:          string_insert
   DESCRIPTION:          Generate a INSERT statement for table export
 ***************************************************************************/
std::string              cyrestsalesheader::string_insert ()  {
    std::string tmp = get_column ("transaction_code")->get_value ();
  std::stringstream sql;
  sql << "insert into ";
  sql << schema_table ();
  sql << " (systransnum, company_code, branch_code, register_number";
  sql << ", transaction_number, transaction_date, transaction_time, logical_date";
  sql << ", cashier, transaction_code, picture_url, cancel_flag";
  sql << ", is_polled, is_polled2, clerk_type_code, agent_type_code";
  sql << ", shopper_type_code, account_type_code, clerk_number, agent_number";
  sql << ", shopper_number, account_number)  values (";
  sql << "'";  sql << get_column ("systransnum")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("company_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("branch_code")->get_value ();  sql << "',";
  sql << get_column ("register_number")->get_value ();  sql << ",";
  sql << get_column ("transaction_number")->get_value ();  sql << ",";
  sql << get_column ("transaction_date")->get_value ();  sql << ",";
  sql << get_column ("transaction_time")->get_value ();  sql << ",";
  sql << get_column ("logical_date")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("cashier")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("transaction_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("picture_url")->get_value ();  sql << "',";
  sql << get_column ("cancel_flag")->get_value ();  sql << ",";
  sql << get_column ("is_polled")->get_value ();  sql << ",";
  sql << get_column ("is_polled2")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("clerk_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("agent_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("shopper_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_type_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("clerk_number")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("agent_number")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("shopper_number")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("account_number")->get_value ();  sql << "')";
  return sql.str ();
}
  /////////////////////////////////////
  //  Create the column list
  bool                   cyrestsalesheader::column_array ()
  {
    CYColumn             *column;
    /////////////////////////////////////
    //  Database table
    m_schema_name  = "cyrestsalesheader";
    m_schema_table = "tg_pos_mobile_header";
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
    column->_col_label          = "Transaction Number";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - company_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 20;
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

    column->_column_name        = "company_code";
    column->_col_label          = "Company Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - branch_code
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

    column->_column_name        = "branch_code";
    column->_col_label          = "Branch Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - register_number
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 40;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 12;
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

    column->_column_name        = "register_number";
    column->_col_label          = "Register Number";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - transaction_number
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 50;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 12;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 999999999999;
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

    column->_column_name        = "transaction_number";
    column->_col_label          = "Transaction Number";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - transaction_date
    column = new CYColumnDate ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 60;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 8;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 99999999;
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

    column->_column_name        = "transaction_date";
    column->_col_label          = "Transaction Date";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - transaction_time
    column = new CYColumnTime ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 70;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 4;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 99999999;
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

    column->_column_name        = "transaction_time";
    column->_col_label          = "Transaction Time";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - logical_date
    column = new CYColumnDate ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 80;
    column->_col_minlen         = 1;
    column->_col_maxlen         = 8;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 99999999;
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

    column->_column_name        = "logical_date";
    column->_col_label          = "Logical Date";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - cashier
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 90;
    column->_col_minlen         = 1;
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

    column->_column_name        = "cashier";
    column->_col_label          = "Cashier";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - transaction_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 100;
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

    column->_column_name        = "transaction_code";
    column->_col_label          = "Transaction Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - picture_url
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 110;
    column->_col_minlen         = 0;
    column->_col_maxlen         = 1024;
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

    column->_column_name        = "picture_url";
    column->_col_label          = "Picture Url";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z _-#@*(),.:;]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - cancel_flag
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

    column->_column_name        = "cancel_flag";
    column->_col_label          = "Cancel Flag";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_polled
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

    column->_column_name        = "is_polled";
    column->_col_label          = "Is Polled";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - is_polled2
    column = new CYColumnYesNo ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 140;
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

    column->_column_name        = "is_polled2";
    column->_col_label          = "Is Polled2";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-1]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - clerk_type_code
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

    column->_column_name        = "clerk_type_code";
    column->_col_label          = "Clerk Type Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - agent_type_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 160;
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

    column->_column_name        = "agent_type_code";
    column->_col_label          = "Agent Type Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - shopper_type_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 170;
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

    column->_column_name        = "shopper_type_code";
    column->_col_label          = "Shopper Type Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - account_type_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 180;
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
    column->_col_display_pattern= "^[^[0-9A-Za-z_-().@#$]+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - clerk_number
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 190;
    column->_col_minlen         = 0;
    column->_col_maxlen         = 64;
    column->_col_decimal        = 0;
    column->_col_zeropad        = 0;
    column->_col_minval         = 0;
    column->_col_maxval         = 100000000000000000;
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

    column->_column_name        = "clerk_number";
    column->_col_label          = "Clerk Number";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - agent_number
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 200;
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

    column->_column_name        = "agent_number";
    column->_col_label          = "Agent Number";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - shopper_number
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 210;
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

    column->_column_name        = "shopper_number";
    column->_col_label          = "Shopper Number";
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
    column->_col_seq            = 220;
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

    column->_column_name        = "account_number";
    column->_col_label          = "Account Number";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    return true;
  }
