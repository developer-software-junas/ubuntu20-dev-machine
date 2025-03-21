/*****************************************************************************
          FILE:          cyrestsalesdiscountdetail.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalesdiscountdetail
#define __cyrestsalesdiscountdetail
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"
/***************************************************************************
         CLASS:          cyrestsalesdiscountdetail
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalesdiscountdetail : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalesdiscountdetail (cyposrest* rest) : CYColumnArrayBase () {
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
  bool                   copy (cyrestsalesdiscountdetail obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_item_seq ()  {
    return get_column ("item_seq")->get_value ();
  }
  std::string            get_discount_code ()  {
    return get_column ("discount_code")->get_value ();
  }
  std::string            get_description ()  {
    return get_column ("description")->get_value ();
  }
  std::string            get_discount_type ()  {
    return get_column ("discount_type")->get_value ();
  }
  std::string            get_discount_value ()  {
    return get_column ("discount_value")->get_value ();
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

  bool                   set_discount_code(std::string val)  {
    if (!valid_key ("discount_code", val)) {
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

  bool                   set_discount_type(std::string val)  {
    if (!valid_number ("discount_type", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_discount_value(std::string val)  {
    if (!valid_decimal ("discount_value", val)) {
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
