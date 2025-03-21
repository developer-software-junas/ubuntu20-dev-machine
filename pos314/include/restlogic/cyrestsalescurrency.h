/*****************************************************************************
          FILE:          cyrestsalescurrency.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalescurrency
#define __cyrestsalescurrency
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"
/***************************************************************************
         CLASS:          cyrestsalescurrency
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalescurrency : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalescurrency (cyposrest* rest) : CYColumnArrayBase () {
    _rest = rest;
    column_array ();
  }
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
  bool                   copy (cyrestsalescurrency obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_currency_code ()  {
    return get_column ("currency_code")->get_value ();
  }
  std::string            get_description ()  {
    return get_column ("description")->get_value ();
  }
  std::string            get_is_multiply ()  {
    return get_column ("is_multiply")->get_value ();
  }
  std::string            get_default_rate ()  {
    return get_column ("default_rate")->get_value ();
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

  bool                   set_currency_code(std::string val)  {
    if (!valid_key ("currency_code", val)) {
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

  bool                   set_is_multiply(std::string val)  {
    if (!valid_flag ("is_multiply", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_default_rate(std::string val)  {
    if (!valid_decimal ("default_rate", val)) {
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
