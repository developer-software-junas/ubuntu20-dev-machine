/*****************************************************************************
          FILE:          cyrestsalestranstype.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalestranstype
#define __cyrestsalestranstype
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"
/***************************************************************************
         CLASS:          cyrestsalestranstype
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalestranstype : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalestranstype (cyposrest* rest) : CYColumnArrayBase () {
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
  bool                   copy (cyrestsalestranstype obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_transaction_code ()  {
    return get_column ("transaction_cd")->get_value ();
  }
  std::string            get_description ()  {
    return get_column ("description")->get_value ();
  }
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_is_default ()  {
    return get_column ("is_default")->get_value ();
  }
  std::string            get_min_amount ()  {
    return get_column ("min_amount")->get_value ();
  }
  std::string            get_max_amount ()  {
    return get_column ("max_amount")->get_value ();
  }
  std::string            get_is_zero_rated_tax ()  {
    return get_column ("is_zero_rated_tax")->get_value ();
  }
  ///////////////////////////////////////
  //  Access method(s) - SET
  bool                   set_transaction_code(std::string val)  {
    if (!valid_key ("transaction_code", val)) {
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

  bool                   set_systransnum(std::string val)  {
    if (!valid_key ("systransnum", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_is_default(std::string val)  {
    if (!valid_flag ("is_default", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_min_amount(std::string val)  {
    if (!valid_decimal ("min_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_max_amount(std::string val)  {
    if (!valid_decimal ("max_amount", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_is_zero_rated_tax(std::string val)  {
    if (!valid_flag ("is_zero_rated_tax", val)) {
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
