//*******************************************************************
//        FILE:     cyresteod.h
// DESCRIPTION:     Modifier object
//*******************************************************************
#ifndef cy_rest_eod__h
#define cy_rest_eod__h
#include <string>
#include <sstream>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <list>
//////////////////////////////////////////
//  Header file(s)
#include "cyw_aes.h"
#include "cyw_log.h"
#include "cyw_mem.h"
#include "cyw_util.h"
#include "json/json.h"
#include "cydb/cyw_db.h"
#include "cyrestcommon.h"
#include "cyrestreceipt.h"
//*******************************************************************
//       CLASS:     CYRestEod
// DESCRIPTION:     POS sales object
//*******************************************************************
class               CYRestEod : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestEod ();
  CYRestEod (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Operation(s)
  bool                  xread_batch (std::string logical_date,
                                     std::string branch_code,
                                     std::string register_num,
                                     std::string cashier,
                                     std::string cashier_shift);
  bool                  rest_xread_receipt (salesReceiptType type,
                                            std::string logical_date,
                                            std::string branch_code,
                                            std::string register_num,
                                            std::string cashier,
                                            std::string cashier_shift);

  std::string           _cashier;
  std::string           _branch_code;
  std::string           _logical_date;
  std::string           _register_num;
  std::string           _cashier_shift;
  ////////////////////////////////////////
  //  Range attribute(s)
  std::string           _end_date, _end_time,
                        _start_date, _start_time;
  std::string           _systransstart, _systransend;
  ////////////////////////////////////////
  //  Xread stats
  struct_tax _xread_tax;
  struct_totals _xread_totals;
  std::vector<struct_bank*> _xread_bank;
  std::vector<struct_payment*> _xread_payment;
  std::vector<struct_discount*> _xread_discount;
  ////////////////////////////////////////
  //  Reset total(s)
  bool                  reset_totals (std::string branch_code,
                                      std::string logical_date,
                                      std::string register_num);
  bool                  reset_totals_register (std::string branch_code, std::string register_num);
};
#endif
