/*****************************************************************************
          FILE:          cyrestsalespayment.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalespayment
#define __cyrestsalespayment
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"

#include "cyrestsalesdiscountpayment.h"
/***************************************************************************
         CLASS:          cyrestsalespayment
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalespayment : public CYColumnArrayBase
{
public:
    ////////////////////////////////////////
    //  Constructor
    cyrestsalespayment (cyposrest* rest);
    virtual ~cyrestsalespayment ();
    ////////////////////////////////////////
    //  Detail discount
    cyrestsalesdiscountpayment* _discount;
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
  bool                   copy (cyrestsalespayment obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_pay_seq ()  {
    return get_column ("pay_seq")->get_value ();
  }
  std::string            get_pay_code ()  {
    return get_column ("pay_code")->get_value ();
  }
  std::string            get_description ()  {
    return get_column ("description")->get_value ();
  }
  std::string            get_currency_code ()  {
    return get_column ("currency_code")->get_value ();
  }
  std::string            get_discount_code ()  {
    return get_column ("discount_code")->get_value ();
  }
  std::string            get_discount_amount ()  {
    return get_column ("discount_amount")->get_value ();
  }
  std::string            get_payment_amount ()  {
    return get_column ("payment_amount")->get_value ();
  }
  std::string            get_converted_payment_amount ()  {
    return get_column ("converted_payment_amount")->get_value ();
  }
  std::string            get_account_type_code ()  {
    return get_column ("account_type_code")->get_value ();
  }
  std::string            get_account_number ()  {
    return get_column ("account_number")->get_value ();
  }
  std::string            get_approval_code ()  {
    return get_column ("approval_code")->get_value ();
  }
  std::string            get_approval_merchant ()  {
    return get_column ("approval_merchant")->get_value ();
  }
  std::string            get_change_code ()  {
    return get_column ("change_code")->get_value ();
  }
  std::string            get_change_currency_code ()  {
    return get_column ("change_currency_code")->get_value ();
  }
  std::string            get_change_amount ()  {
    return get_column ("change_amount")->get_value ();
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

  bool                   set_pay_seq(std::string val)  {
    if (!valid_number ("pay_seq", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_pay_code(std::string val)  {
    if (!valid_alphanum ("pay_code", val)) {
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

  bool                   set_currency_code(std::string val)  {
    if (!valid_alphanum ("currency_code", val)) {
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

  bool                   set_discount_amount(std::string val)  {
    if (!valid_decimal ("discount_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_payment_amount(std::string val)  {
    if (!valid_decimal ("payment_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_converted_payment_amount(std::string val)  {
    if (!valid_decimal ("converted_payment_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_account_type_code(std::string val)  {
    if (!valid_alphanum ("account_type_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_account_number(std::string val)  {
    if (!valid_alphanum ("account_number", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_approval_code(std::string val)  {
    if (!valid_alphanum ("approval_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_approval_merchant(std::string val)  {
    if (!valid_alphanum ("approval_merchant", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_change_code(std::string val)  {
    if (!valid_alphanum ("change_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_change_currency_code(std::string val)  {
    if (!valid_alphanum ("change_currency_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_change_amount(std::string val)  {
    if (!valid_decimal ("change_amount", val)) {
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
