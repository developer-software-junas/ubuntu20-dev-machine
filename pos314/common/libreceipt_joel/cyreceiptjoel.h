//*******************************************************************
//        FILE:     cyrestreceipt.h
// DESCRIPTION:     POS sales objectt
//*******************************************************************
#ifndef cy_restreceipt_joel__h
#define cy_restreceipt_joel__h
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
#include "restsvc/storesales/cyrestcommon.h"
//*******************************************************************
//       CLASS:         CYRestReceipt
// DESCRIPTION:         POS sales object
//*******************************************************************
class                   CYReceiptJoel : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYReceiptJoel ();
  CYReceiptJoel (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Receipt
  bool                  saveReceipt ();
  bool                  assembleFspUrl (Json::Value& jheader,
                                        Json::Value& jdetail,
                                        Json::Value& jpayment);
  ////////////////////////////////////////
  //  Access method(s)
  std::string           receipt  () { return _receipt; }
  bool                  xreadSaveTax (std::string systransnum);
  ////////////////////////////////////////
  //  Receipt method(s)
  bool                  assembleReceipt (Json::Value& jheader,
                                         Json::Value& jdetail,
                                         Json::Value& jpayment,
                                         salesReceiptType receiptType);
  bool                  assembleFspAccount (Json::Value& jheader);
  bool                  assembleReceiptHeader (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptDetail (Json::Value& jdetail,
                                               Json::Value& jhdrtrx,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptTotals (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleVatBreakdown (salesReceiptType receiptType);
  bool                  assembleReceiptPayment (Json::Value& jheader,
                                                Json::Value& jpayment,
                                                salesReceiptType receiptType);
  bool                  assembleReceiptAccount (Json::Value& jheader);
  bool                  assembleReceiptSignature (Json::Value& jheader);
  bool                  assembleReceiptFooter (Json::Value& jheader,
                                               salesReceiptType receiptType);
};
#endif
