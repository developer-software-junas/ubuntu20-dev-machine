//*******************************************************************
//        FILE:     cyrestreceipt.h
// DESCRIPTION:     POS sales objectt
//*******************************************************************
#ifndef cy_rest_sales__h
#define cy_rest_sales__h
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
#include "cydb/cymysql.h"
#include "restsvc/cyposrest.h"
#include "restsvc/storesales/cyrestcommon.h"
//////////////////////////////////////////
//  Client specific
#include "/Users/pos314/common/libreceipt_rcc/cyreceiptrcc.h"
#include "/Users/pos314/common/libreceipt_rico/cyreceiptrico.h"
#include "/Users/pos314/common/libreceipt_joel/cyreceiptjoel.h"
#include "/Users/pos314/common/libreceipt_east/cyreceipteast.h"
#include "/Users/pos314/common/libreceipt_elena/cyreceiptelena.h"
#include "/Users/pos314/common/libreceipt_sunmi/cyreceiptsunmi.h"
#include "/Users/pos314/common/libreceipt_sunnies/cyreceiptsunnies.h"
const std::string g_suspend_tables [] = {
    "tg_pos_suspend_header",
    "tg_pos_mobile_fnb",
    "tg_pos_mobile_consign",
    "tg_pos_discount_summary",
    "tg_pos_xread_tax",
    "tg_pos_mobile_tax",
    "tg_pos_mobile_trxaccount",
    "tg_pos_mobile_transtype",
    "tg_pos_mobile_discount",
    "tg_pos_mobile_currency",
    "tg_pos_mobile_account",
    "tg_pos_mobile_split",
    "tg_pos_mobile_audit",
    "tg_pos_mobile_header_receipt",
    "tg_pos_email_receipt",
    "tg_pos_mobile_alias",
    "tg_pos_email_receipt",

    "tg_pos_mobile_detail",
    "tg_pos_mobile_modifier",
    "tg_pos_mobile_addon_detail",
    "tg_pos_mobile_discount_detail",

    "tg_pos_mobile_payment",
    "tg_pos_mobile_gc_payment",
    "tg_pos_mobile_lanebust",
    "tg_pos_gc_detail",
    "tg_pos_refund",
    "tg_pos_refund_detail",
    "tg_pos_mobile_discount_payment",
    "tg_pos_mobile_bank_payment",

    "tg_end"
};
//*******************************************************************
//       CLASS:         CYRestReceipt
// DESCRIPTION:         POS sales object
//*******************************************************************
class                   CYRestReceipt : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestReceipt ();
  CYRestReceipt (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Receipt
  bool saveReceipt ();
  ////////////////////////////////////////
  std::string           receipt  () { return _receipt; }
  ////////////////////////////////////////
  //  Xread
  bool                  xreadSaveTax (std::string systransnum);
  ////////////////////////////////////////
  //  Loyalty method(s)
  bool                  assembleFspUrl (Json::Value& jheader,
                                        Json::Value& jdetail,
                                        Json::Value& jpayment);
  bool                  assembleFspHeader (Json::Value& jheader);
  bool                  assembleFspDetail (Json::Value& jdetail);
  bool                  assembleFspTotals (Json::Value& jheader);
  bool                  assembleFspPayment (Json::Value& jpayment);
  bool                  assembleFspAccount (Json::Value& jheader);
  ////////////////////////////////////////
  //  Receipt method(s)
  bool                  assembleReceipt (Json::Value& jheader,
                                         Json::Value& jdetail,
                                         Json::Value& jpayment,
                                         salesReceiptType receiptType);
  bool                  assembleVatBreakdown (salesReceiptType receiptType);
  bool                  assembleReceiptHeader (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptDetail (Json::Value& jdetail,
                                               Json::Value& jhdrtrx,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptTotals (Json::Value& jheader);
  bool                  assembleReceiptPayment (Json::Value& jheader,
                                                Json::Value& jpayment,
                                                salesReceiptType receiptType);
  bool                  assembleReceiptAccount (Json::Value& jheader);
  bool                  assembleReceiptSignature (Json::Value& jheader);
  bool                  assembleReceiptFooter (Json::Value& jheader,
                                               salesReceiptType receiptType);
  bool                  assembleReceiptGiftWrap (Json::Value& jheader,
                                                 Json::Value& jdetail);
  //////////////////////////////////////
  //  Xread update method(s)
protected:
  SqliteEnv*            _env;
};
#endif
