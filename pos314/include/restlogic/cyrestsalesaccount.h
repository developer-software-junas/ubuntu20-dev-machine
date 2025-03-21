/*****************************************************************************
          FILE:          cyrestsalesaccount.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalesaccount
#define __cyrestsalesaccount
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"
/***************************************************************************
         CLASS:          cyrestsalesaccount
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalesaccount : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalesaccount (cyposrest* rest) : CYColumnArrayBase () {
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
    bool                   copy (cyrestsalesaccount obj);
    ///////////////////////////////////////
    //  Access method(s) - GET
    std::string            get_systransnum ()  {
      return get_column ("systransnum")->get_value ();
    }
    std::string            get_customer_id ()  {
      return get_column ("customer_id")->get_value ();
    }
    std::string            get_account_number ()  {
      return get_column ("account_number")->get_value ();
    }
    std::string            get_account_type_code ()  {
      return get_column ("account_type_code")->get_value ();
    }
    std::string            get_account_type_desc ()  {
      return get_column ("account_type_desc")->get_value ();
    }
    std::string            get_first_name ()  {
      return get_column ("first_name")->get_value ();
    }
    std::string            get_middle_name ()  {
      return get_column ("middle_name")->get_value ();
    }
    std::string            get_last_name ()  {
      return get_column ("last_name")->get_value ();
    }
    std::string            get_points_earned ()  {
      return get_column ("points_earned")->get_value ();
    }
    std::string            get_is_loyalty ()  {
      return get_column ("is_loyalty")->get_value ();
    }
    std::string            get_is_clerk ()  {
      return get_column ("is_clerk")->get_value ();
    }
    std::string            get_is_agent ()  {
      return get_column ("is_agent")->get_value ();
    }
    std::string            get_is_shopper ()  {
      return get_column ("is_shopper")->get_value ();
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

    bool                   set_customer_id(std::string val)  {
      if (!valid_key ("customer_id", val)) {
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

    bool                   set_account_type_code(std::string val)  {
      if (!valid_alphanum ("account_type_code", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_account_type_desc(std::string val)  {
      if (!valid_alphanum ("account_type_desc", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_first_name(std::string val)  {
      if (!valid_alphanum ("first_name", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_middle_name(std::string val)  {
      if (!valid_alphanum ("middle_name", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_last_name(std::string val)  {
      if (!valid_alphanum ("last_name", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_points_earned(std::string val)  {
      if (!valid_decimal ("points_earned", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_is_loyalty(std::string val)  {
      if (!valid_flag ("is_loyalty", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_is_clerk(std::string val)  {
      if (!valid_flag ("is_clerk", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_is_agent(std::string val)  {
      if (!valid_flag ("is_agent", val)) {
        throw std::invalid_argument (errormsg ());
        return false;
      }
      return true;
    }

    bool                   set_is_shopper(std::string val)  {
      if (!valid_flag ("is_shopper", val)) {
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
