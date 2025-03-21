//*******************************************************************
//        FILE:     cyrestreceipt.h
// DESCRIPTION:     POS sales objectt
//*******************************************************************
#ifndef cy_restreceipt_rcc__h
#define cy_restreceipt_rcc__h
#include <string>
#include <pthread.h>
//////////////////////////////////////////
//  Header file(s)
#include "cyw_log.h"
#include "json/json.h"
#include "restsvc/storesales/cyrestcommon.h"

//*******************************************************************
//       CLASS:         CYRestReceipt
// DESCRIPTION:         POS sales object
//*******************************************************************
class                   CYReceiptRcc : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYReceiptRcc ();
  CYReceiptRcc (cylog* log, cyini* ini, cycgi* cgi);
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
  bool                  assembleVatBreakdown (salesReceiptType receiptType);
  bool                  assembleReceiptAcctFSPBA (Json::Value& jheader);
  bool                  assembleReceiptSignature (Json::Value& jheader,
                                                  salesReceiptType receiptType);
  bool                  assembleReceiptFooter (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptGiftWrap (Json::Value& jheader,
                                                 Json::Value& jdetail);
  bool                  assembleReceiptClaimStub (Json::Value& jheader,
                                                 Json::Value& jdetail);
  bool                  assembleReceiptPromoItem (Json::Value& jheader,
                                                  Json::Value& jdetail);
  //////////////////////////////////////
  //  Xread update method(s)
protected:
  CYDbEnv*              _env;
  ////////////////////////////////////////
  //  Breakdown method(s)
  bool                  receiptHeaderStart (Json::Value& jheader);
  ////////////////////////////////////////
  //  Utility method(s)
  std::string           rccDate (std::string strDate, std::string strTime);
};
#endif
