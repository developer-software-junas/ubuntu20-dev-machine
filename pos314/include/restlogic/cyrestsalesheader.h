/*****************************************************************************
          FILE:          cyrestsalesheader.h
   DESCRIPTION:          Schema header
 *****************************************************************************/
#ifndef __cyrestsalesheader
#define __cyrestsalesheader
#include "cyposrest.h"
#include "../cydb/cydbsql.h"
#include "../col/column.h"
#include "../col/column_array_base.h"

#include "cyrestsalesdetail.h"
#include "cyrestsalespayment.h"

#include "cyrestsalesaccount.h"
#include "cyrestsalescurrency.h"
#include "cyrestsalesdiscount.h"
#include "cyrestsalestranstype.h"
/***************************************************************************
         CLASS:          cyrestsalesheader
   DESCRIPTION:          Database table class definition
 ***************************************************************************/
class                    cyrestsalesheader : public CYColumnArrayBase
{
public:
  ////////////////////////////////////////
  //  Constructor
  cyrestsalesheader (cyposrest* rest);
    virtual ~cyrestsalesheader ();
  ////////////////////////////////////////
  // Change table(s), cancel/suspend/etc
  void                  changetablename (std::string table);
  ////////////////////////////////////////
  //  Child object(s)
  cyrestsalescurrency   *_currency;
  cyrestsalesdiscount   *_discount;
  cyrestsalestranstype  *_transtype;
      
  std::vector<cyrestsalesaccount*> _listAcct;
  //////////////////////////////////////
  //  Sales list(s)
  std::vector<cyrestsalesdetail*>  _listDetail;
  std::vector<cyrestsalespayment*> _listPayment;
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
  bool                   copy (cyrestsalesheader obj);
  ///////////////////////////////////////
  //  Access method(s) - GET
  std::string            get_systransnum ()  {
    return get_column ("systransnum")->get_value ();
  }
  std::string            get_company_code ()  {
    return get_column ("company_code")->get_value ();
  }
  std::string            get_branch_code ()  {
    return get_column ("branch_code")->get_value ();
  }
  std::string            get_register_number ()  {
    return get_column ("register_number")->get_value ();
  }
  std::string            get_transaction_number ()  {
    return get_column ("transaction_number")->get_value ();
  }
  std::string            get_transaction_date ()  {
    return get_column ("transaction_date")->get_value ();
  }
  std::string            get_transaction_time ()  {
    return get_column ("transaction_time")->get_value ();
  }
  std::string            get_logical_date ()  {
    return get_column ("logical_date")->get_value ();
  }
  std::string            get_cashier ()  {
    return get_column ("cashier")->get_value ();
  }
  std::string            get_transaction_code ()  {
    return get_column ("transaction_cd")->get_value ();
  }
  std::string            get_picture_url ()  {
    return get_column ("picture_url")->get_value ();
  }
  std::string            get_cancel_flag ()  {
    return get_column ("cancel_flag")->get_value ();
  }
  std::string            get_is_polled ()  {
    return get_column ("is_polled")->get_value ();
  }
  std::string            get_is_polled2 ()  {
    return get_column ("is_polled2")->get_value ();
  }
  std::string            get_clerk_type_code ()  {
    return get_column ("clerk_type_code")->get_value ();
  }
  std::string            get_agent_type_code ()  {
    return get_column ("agent_type_code")->get_value ();
  }
  std::string            get_shopper_type_code ()  {
    return get_column ("shopper_type_code")->get_value ();
  }
  std::string            get_account_type_code ()  {
    return get_column ("account_type_code")->get_value ();
  }
  std::string            get_clerk_number ()  {
    return get_column ("clerk_number")->get_value ();
  }
  std::string            get_agent_number ()  {
    return get_column ("agent_number")->get_value ();
  }
  std::string            get_shopper_number ()  {
    return get_column ("shopper_number")->get_value ();
  }
  std::string            get_account_number ()  {
    return get_column ("account_number")->get_value ();
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

  bool                   set_company_code(std::string val)  {
    if (!valid_alphanum ("company_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_branch_code(std::string val)  {
    if (!valid_alphanum ("branch_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_register_number(std::string val)  {
    if (!valid_number ("register_number", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_transaction_number(std::string val)  {
    if (!valid_number ("transaction_number", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_transaction_date(std::string val)  {
    if (!valid_date ("transaction_date", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_transaction_time(std::string val)  {
    if (!valid_time ("transaction_time", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_logical_date(std::string val)  {
    if (!valid_date ("logical_date", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_cashier(std::string val)  {
    if (!valid_alphanum ("cashier", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_transaction_code(std::string val)  {
    if (!valid_alphanum ("transaction_code", val)) {
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

  bool                   set_cancel_flag(std::string val)  {
    if (!valid_flag ("cancel_flag", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_is_polled(std::string val)  {
    if (!valid_flag ("is_polled", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_is_polled2(std::string val)  {
    if (!valid_flag ("is_polled2", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_clerk_type_code(std::string val)  {
    if (!valid_alphanum ("clerk_type_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_agent_type_code(std::string val)  {
    if (!valid_alphanum ("agent_type_code", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_shopper_type_code(std::string val)  {
    if (!valid_alphanum ("shopper_type_code", val)) {
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

  bool                   set_clerk_number(std::string val)  {
    if (!valid_alphanum ("clerk_number", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_agent_number(std::string val)  {
    if (!valid_alphanum ("agent_number", val)) {
      throw std::invalid_argument (errormsg ());
      return false;
    }
    return true;
  }

  bool                   set_shopper_number(std::string val)  {
    if (!valid_alphanum ("shopper_number", val)) {
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

protected:
  ///////////////////////////////////////
  //  Attribute(s)
  cyposrest*             _rest;
  std::string            _error;
};
#endif
