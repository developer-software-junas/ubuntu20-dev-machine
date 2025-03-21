/*****************************************************************************
          FILE:          cyrestsalesdetail.cpp
   DESCRIPTION:          Schema implementaion
 *****************************************************************************/
#include "cyrestsalesdetail.h"
/***************************************************************************
      FUNCTION:          Constructor
   DESCRIPTION:          Initialize
 ***************************************************************************/
cyrestsalesdetail::cyrestsalesdetail (cyposrest* rest)
{
    _rest = rest;
    _discount = nullptr;
    
    column_array ();
    _discount = new cyrestsalesdiscountdetail (rest);
}
/***************************************************************************
      FUNCTION:          Destructor
   DESCRIPTION:          Cleanup
 ***************************************************************************/
cyrestsalesdetail::~cyrestsalesdetail ()
{
    if (nullptr != _discount)
        delete _discount;
}
/***************************************************************************
      FUNCTION:          map_argument_cgi
   DESCRIPTION:          Map a single CGI argument
 ***************************************************************************/
bool                     cyrestsalesdetail::map_argument_cgi (std::string arg,
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
bool                     cyrestsalesdetail::map_argument_cgikey (std::string suffix,
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
bool                     cyrestsalesdetail::map_arguments (CYDbSql* db,
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
void                     cyrestsalesdetail::reset () {
  _discount->reset ();
  get_column ("systransnum")->set_value ("");
  get_column ("item_seq")->set_value ("0");
  get_column ("item_code")->set_value ("");
  get_column ("description")->set_value ("");
  get_column ("uom_code")->set_value ("");
  get_column ("quantity")->set_value ("0");
  get_column ("retail_price")->set_value ("0");
  get_column ("discount_amount")->set_value ("0");
  get_column ("discount_code")->set_value ("");
  get_column ("subtotal")->set_value ("0");
  get_column ("picture_url")->set_value ("");
  get_column ("tax_rate")->set_value ("0");
  get_column ("tax_code")->set_value ("");
}
/***************************************************************************
      FUNCTION:          copy
   DESCRIPTION:          Object copy
 ***************************************************************************/
bool                     cyrestsalesdetail::copy (cyrestsalesdetail obj) {
  if (!set_systransnum (obj.get_systransnum ()))  return false;
  if (!set_item_seq (obj.get_item_seq ()))  return false;
  if (!set_item_code (obj.get_item_code ()))  return false;
  if (!set_description (obj.get_description ()))  return false;
  if (!set_uom_code (obj.get_uom_code ()))  return false;
  if (!set_quantity (obj.get_quantity ()))  return false;
  if (!set_retail_price (obj.get_retail_price ()))  return false;
  if (!set_discount_amount (obj.get_discount_amount ()))  return false;
  if (!set_discount_code (obj.get_discount_code ()))  return false;
  if (!set_subtotal (obj.get_subtotal ()))  return false;
  if (!set_picture_url (obj.get_picture_url ()))  return false;
  if (!set_tax_rate (obj.get_tax_rate ()))  return false;
  if (!set_tax_code (obj.get_tax_code ()))  return false;
  return true;
}
/***************************************************************************
      FUNCTION:          string_insert
   DESCRIPTION:          Generate a INSERT statement for table export
 ***************************************************************************/
std::string              cyrestsalesdetail::string_insert ()  {
  std::stringstream sql;
  sql << "insert into ";
  sql << schema_table ();
  sql << " (systransnum, item_seq, item_code, description";
  sql << ", uom_code, quantity, retail_price, discount_amount";
  sql << ", discount_code, subtotal, picture_url, tax_rate";
  sql << ", tax_code)  values (";
  sql << "'";  sql << get_column ("systransnum")->get_value ();  sql << "',";
  sql << get_column ("item_seq")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("item_code")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("description")->get_value ();  sql << "',";
  sql << "'";  sql << get_column ("uom_code")->get_value ();  sql << "',";
  sql << get_column ("quantity")->get_value ();  sql << ",";
  sql << get_column ("retail_price")->get_value ();  sql << ",";
  sql << get_column ("discount_amount")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("discount_code")->get_value ();  sql << "',";
  sql << get_column ("subtotal")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("picture_url")->get_value ();  sql << "',";
  sql << get_column ("tax_rate")->get_value ();  sql << ",";
  sql << "'";  sql << get_column ("tax_code")->get_value ();  sql << "')";
  return sql.str ();
}
  /////////////////////////////////////
  //  Create the column list
  bool                   cyrestsalesdetail::column_array ()
  {
    CYColumn             *column;
    /////////////////////////////////////
    //  Database table
    m_schema_name  = "cyrestsalesdetail";
    m_schema_table = "tg_pos_mobile_detail";
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
    //  Column - item_seq
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

    column->_column_name        = "item_seq";
    column->_col_label          = "Item Seq";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - item_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 30;
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

    column->_column_name        = "item_code";
    column->_col_label          = "Item Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9]+$";

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
    //  Column - uom_code
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

    column->_column_name        = "uom_code";
    column->_col_label          = "Uom Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - quantity
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 60;
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

    column->_column_name        = "quantity";
    column->_col_label          = "Quantity";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - retail_price
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

    column->_column_name        = "retail_price";
    column->_col_label          = "Retail Price";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - discount_amount
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
    //  Column - discount_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 90;
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
    //  Column - subtotal
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 100;
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

    column->_column_name        = "subtotal";
    column->_col_label          = "Subtotal";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

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
    //  Column - tax_rate
    column = new CYColumnNumber ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 120;
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

    column->_column_name        = "tax_rate";
    column->_col_label          = "Tax Rate";
    column->_col_defvalue       = "0";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9.-+]+$";

    add_column (column);
    ///////////////////////////////////////
    //  Column - tax_code
    column = new CYColumnAlpha ();
    column->_col_case           = js_nocase;
    column->_col_seq            = 130;
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

    column->_column_name        = "tax_code";
    column->_col_label          = "Tax Code";
    column->_col_defvalue       = "";
    column->_col_searchvalue    = "";
    column->_col_drop_field     = "";
    column->_col_drop_script    = "";
    column->_col_display_css    = "";
    column->_col_display_pattern= "^[0-9A-Za-z_-().@#$]+$";

    add_column (column);
    return true;
  }
