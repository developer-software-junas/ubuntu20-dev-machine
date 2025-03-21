//*******************************************************************
//        FILE:     cyrestreceipt.h
// DESCRIPTION:     POS sales objectt
//*******************************************************************
#ifndef cy_rest_sales_east__h
#define cy_rest_sales_east__h
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
#include "restsvc/cyposrest.h"
#include "restsvc/storesales/cyrestcommon.h"
//*******************************************************************
//       CLASS:     CYRestReceipt
// DESCRIPTION:     POS sales object
//*******************************************************************
class               CYReceiptEast : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYReceiptEast ();
  CYReceiptEast (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Receipt
  bool saveReceipt ();
  bool assembleFspUrl (Json::Value& jheader,
                       Json::Value& jdetail,
                       Json::Value& jpayment);
  ////////////////////////////////////////
  std::string           receipt  () { return _receipt; }
  bool                  xreadSaveTax (std::string systransnum);
  ////////////////////////////////////////
  //  Receipt method(s)
  bool                  assembleVatBreakdown (salesReceiptType receiptType);
  bool                  assembleReceipt (Json::Value& jheader,
                                         Json::Value& jdetail,
                                         Json::Value& jpayment,
                                         salesReceiptType receiptType);
  bool                  assembleReceiptHeader (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptDetail (Json::Value& jdetail,
                                               Json::Value& jhdrtrx,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptTotals (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptPayment (Json::Value& jheader,
                                                Json::Value& jpayment,
                                                salesReceiptType receiptType);
  bool                  assembleReceiptAccount (Json::Value& jheader,
                                                salesReceiptType receiptType);
  bool                  assembleReceiptAcctFSPBA (Json::Value& jheader);
  bool                  assembleReceiptSignature (Json::Value& jheader,
                                                  salesReceiptType receiptType);
  bool                  assembleReceiptFooter (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptGiftWrap (Json::Value& jheader,
                                                 Json::Value& jdetail);
  bool                  assembleReceiptClaimStub (Json::Value& jheader,
                                                 Json::Value& jdetail);
  //////////////////////////////////////
  //  Xread update method(s)
protected:
  SqliteEnv*            _env;
};
//*******************************************************************
//       CLASS:     CYRestReceipt
// DESCRIPTION:     POS sales object
//*******************************************************************
class               CYReceiptEastEod : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYReceiptEastEod ();
  CYReceiptEastEod (CYDbEnv* env, cycgi* cgi);
  CYReceiptEastEod (CYDbEnv* env, cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Receipt
  bool saveReceipt ();
  bool assembleEodVat     (Json::Value jheader);
  bool assembleEodBank    (Json::Value jheader);
  bool assembleEodTotals  (Json::Value jheader);
  bool assembleEodHeader  (Json::Value jheader,std::string type);
  bool assembleEodCashier (Json::Value jheader, std::string type);
  bool assembleEodPayment (Json::Value jheader, Json::Value jpayment);
  bool assembleEodReceipt (Json::Value jheader,std::string type);

  bool retrieveXread      (Json::Value& jheader, std::string logical_date,
                           std::string location_code, std::string register_num,
                           std::string shift, std::string cashier,
                           std::string manager, std::string mgrpass);
  bool retrieveZread (Json::Value& jheader, std::string logical_date,
                      std::string location_code, std::string register_num,
                      std::string manager, std::string mgrpass);
  ////////////////////////////////////////
  std::string         receipt  () { return _receipt; }
  bool                xreadSaveTax (std::string systransnum);
  //////////////////////////////////////
  //  Xread update method(s)
protected:
  SqliteEnv*            _env;
};
#endif
