/*****************************************************************************
          FILE:          cyrestsalesdetail.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalesdetail
#define __cyrestsalesdetail
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"

#include "cyrestsalesdiscountdetail.h"
/***************************************************************************
         CLASS:          cyrestsalesdetail
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalesdetail : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalesdetail (cyposrest* rest);
  virtual ~cyrestsalesdetail ();
  ////////////////////////////////////////
  //  Detail discount
  cyrestsalesdiscountdetail* _discount;
  ////////////////////////////////////////
  //  Mapping method(s)
  bool                   map_arguments (CYDbSql* db=nullptr,
                                        std::string suffix="",
                                        bool validate=true);
  bool                   map_argument_cgi (std::string arg,
                                           std::string name,
                                           std::string suffix="",
                                           bool validate=true);
  bool                   map_argument_cgikey (std::string suffix="",
                                              bool validate=true);
  ///////////////////////////////////////
  //  Reset all value(s)
  void                   reset ();
  ///////////////////////////////////////
  //  Create the column array
  bool                   column_array ();
  ///////////////////////////////////////
  //  Generate a SQL insert
  std::string            string_insert ();
  ///////////////////////////////////////
  //  Copy method
  bool                   copy (cyrestsalesdetail obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_item_seq ()  {
    return get_column ("item_seq")->get_value ();
  }
  std::string            get_item_code ()  {
    return get_column ("item_code")->get_value ();
  }
  std::string            get_description ()  {
    return get_column ("description")->get_value ();
  }
  std::string            get_uom_code ()  {
    return get_column ("uom_code")->get_value ();
  }
  std::string            get_quantity ()  {
    return get_column ("quantity")->get_value ();
  }
  std::string            get_retail_price ()  {
    return get_column ("retail_price")->get_value ();
  }
  std::string            get_discount_amount ()  {
    return get_column ("discount_amount")->get_value ();
  }
  std::string            get_discount_code ()  {
    return get_column ("discount_code")->get_value ();
  }
  std::string            get_subtotal ()  {
    return get_column ("subtotal")->get_value ();
  }
  std::string            get_picture_url ()  {
    return get_column ("picture_url")->get_value ();
  }
  std::string            get_tax_rate ()  {
    return get_column ("tax_rate")->get_value ();
  }
  std::string            get_tax_code ()  {
    return get_column ("tax_code")->get_value ();
  }
  ///////////////////////////////////////
  //  Access method(s) - SET
  bool                   set_systransnum(std::string val)  {
    if (!valid_key ("systransnum", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_item_seq(std::string val)  {
    if (!valid_number ("item_seq", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_item_code(std::string val)  {
    if (!valid_alphanum ("item_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_description(std::string val)  {
    if (!valid_alphanum ("description", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_uom_code(std::string val)  {
    if (!valid_alphanum ("uom_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_quantity(std::string val)  {
    if (!valid_decimal ("quantity", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_retail_price(std::string val)  {
    if (!valid_decimal ("retail_price", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_discount_amount(std::string val)  {
    if (!valid_decimal ("discount_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_discount_code(std::string val)  {
    if (!valid_alphanum ("discount_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_subtotal(std::string val)  {
    if (!valid_decimal ("subtotal", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_picture_url(std::string val)  {
    if (!valid_alphanum ("picture_url", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_tax_rate(std::string val)  {
    if (!valid_decimal ("tax_rate", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_tax_code(std::string val)  {
    if (!valid_alphanum ("tax_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

protected:
  ///////////////////////////////////////
  //  Attribute(s)
  cyposrest*             _rest;
  std::string            _error;
};
#endif
