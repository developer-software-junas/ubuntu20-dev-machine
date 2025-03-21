//*******************************************************************
//        FILE:     cyrestcommon.h
// DESCRIPTION:     Common variables
//*******************************************************************
#ifndef cy_rest_common__h
#define cy_rest_common__h
#include <string>
#include <pthread.h>
#include <vector>
//////////////////////////////////////////
//  Header file(s)
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_log.h"
#include "cyw_util.h"
#include "json/json.h"
#include "cydb/cydbsql.h"
#include "cydb/cysqlite.h"
//////////////////////////////////////////
//  Query result
typedef enum  {
    SQT_ERROR, SQT_SUCCESS, SQT_NOROWS
}  salesQueryType;
//////////////////////////////////////////
//  Query result
typedef enum  {
    SRT_NORMAL, SRT_DAILY, SRT_CONSIGN, SRT_MOBILE, SRT_REPRINT,
    SRT_POSTVOID, SRT_SUSPEND, SRT_RESUME, SRT_CANCEL, SRT_REFUND,
    SRT_FNB, SRT_OPTICAL
}  salesReceiptType;
//////////////////////////////////////////
//  Xread typedef, enum and structure
typedef struct  {
    double amount;
    std::string bank_code;
    std::string bank_desc;
} struct_bank;
typedef struct  {
    bool cash;
    double amount;
    std::string tender_code;
    std::string tender_desc;
    std::string tender_type_code;
    std::string tender_type_desc;
} struct_payment;
typedef enum  {
    DISC_GLOBAL, DISC_ITEM, DISC_PAYMENT
} enum_discount_type;
typedef struct  {
    double amount;
    enum_discount_type type;
    std::string discount_code;
    std::string discount_desc;
    std::string discount_type_code;
    std::string discount_type_desc;
} struct_discount;
typedef struct  {
    double net_sales;
    double gross_sales;
    double service_charge;

    double vat_amount;

    double net_vat;
    double net_zero;
    double net_exempt;

    double amt_vat;
    double amt_zero;
    double amt_exempt;
}  struct_tax;
typedef struct  {
    int txn_count;
    int txnsales_count;
    int txnnonsales_count;

    double item_count;

    int item_txnsuspend_count;
    int item_txnsuspendpac_count;

    double average_check;
    double discount_amount;

    int txnvoid_count;
    double txnvoid_amount;

    int txnrefund_count;
    double refund_amount;

    int txncancel_count;
    double txncancel_amount;

    int txnsuspend_count;
    int txnsuspendpac_count;
} struct_totals;
//  Loyalty URL structure
struct STRUCT_FSPURL {
    std::string state="";
    std::string card_number="";
    std::string amount="";
    std::string trans_no="";
    std::string company_cd="";
    std::string branch_cd="";
    std::string pos_no="";
    std::string trans_cd="";
    std::string tender_cd="";
    std::string tender_amount="";
    std::string epurse="";
    std::string item_code="";
    std::string category_cd="";
    std::string item_quantity="";
    std::string item_amount="";
    std::string item_type="";
    std::string is_beauty = "";
    std::string beauty_card="";

    std::string port="";
    std::string host="";
    std::string path="";
    std::string args="";
};
//////////////////////////////////////////
//  Header type
typedef enum  {
    MOBILE, SUSPEND, CANCEL, POSTVOID, DAILY,
    CONSIGN, REPRINT, REFUND, FNB, OPTICAL
}  enumHeader;
//////////////////////////////////////////
//  Login type
typedef enum  {
    CASHIER, MANAGER
}  enumLogin;
//////////////////////////////////////////
//  Query result
typedef enum  {
    EQT_ERROR, EQT_SUCCESS, EQT_NOROWS
}  enumQueryType;
//////////////////////////////////////////
//  Enumerated discount type(s)
typedef enum  {
  dtGlobal,  //  0
  dtManual,
  dtAccount,
  dtTender   // 4
}  EnumDiscountType;
//////////////////////////////////////////
//  Enumerated account type(s)
typedef enum  {
    none, clerk, agent, shopper, customer, bridal,
    vatexempt, senior, pwd, beauty, fnbcustomer
}  EnumAccountType;
//////////////////////////////////////////
//  Sales total structure
struct STRUCT_SALESTOTAL  {
    bool   trx_zero_rated;
    bool   trx_discounted;

    bool   trx_discount_pct=0;
    double trx_discount_value=0.00;

    double total_qty = 0.00;
    double amount_due_gross = 0.00;
    double amount_zero_rated = 0.00;
    double amount_due_less_zero_rated = 0.00;
    double amount_discount_item = 0.00;
    double amount_due_less_item_discount = 0.00;
    double amount_global_discount = 0.00;
    double amount_due_less_global_discount = 0.00;
    double amount_due_net = 0.00;
    double amount_due_tax = 0.00;

    double amount_paid = 0.00;
    double amount_change = 0.00;
    double amount_discount_payment = 0.00;

};
//////////////////////////////////////////
//  Detail update structure
typedef struct  {
    std::string quantity;
    std::string item_seq;
    std::string item_code;
    std::string systransnum;
    std::string location_code;
    std::string retail_price;
    std::string tablename;
}  STRUCT_DETAILUPDATE;
//////////////////////////////////////////
//  PO update structure
typedef struct  {
    std::string quantity;
    std::string item_code;
    std::string retail_price;
}  STRUCT_PO_UPDATE;
//////////////////////////////////////////
//  Audit setting(s)
typedef enum  {
    EPA_LOGIN,
    EPA_CASHFUND,
    EPA_TRANSTYPE,
    EPA_TRANSACCT,
    EPA_ACCOUNT,
    EPA_TRANSDISCOUNT,
    EPA_ACCOUNTDELETE,
    EPA_LOGOUT
}  ENUM_POSAUDIT;
/*
static const char* gstr_audit [] = {
    "Login","Cashfund","Transaction type","Transaction account",
    "Account","Transaction discount","Account delete","Logout"
};*/
typedef struct  {
    std::string         key;
    std::string         val;
} STRUCT_ALTJSON;
//////////////////////////////////////////
//  Constant(s)
class cyposrest;
const size_t g_widthReceipt = 40;
//////////////////////////////////////////
//  Global(s)
#define RCPT_REPRINT_SKINNY   "+----------------------------+"
#define RCPT_CUT       "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-"
#define RCPT_REPRINT   "+--------------------------------------+"
#define RCPT_SIGNATURE "*--------------------------------------*"
////////////////////////////////////////
//  Utility function(s)
std::string           refundTransnum (Json::Value jheader);
std::string           legacyTransnum (Json::Value jheader);
//*******************************************************************
//       CLASS:         CYRestCommon
// DESCRIPTION:         Base REST class
//*******************************************************************
class                   CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestCommon ();
  CYRestCommon (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Operation(s)
  virtual
  std::string           cgiform(std::string name);
  ////////////////////////////////////////
  virtual
  std::string           errormsg () { return _error; }
  virtual
  std::string           errorjson (std::string errormsg) {
    return _util->jsonerror("cyrestsales", errormsg);
  }
  ////////////////////////////////////////
  //  Common receipt calculation(s)
  void                  receipt_vat (double& vatTotal,
                                     Json::Value jheader);
  void                  receipt_vatreg (double& vatTotal,
                                        Json::Value jheader);
  void                  receipt_offset (int& left, int& right);
  void                  receipt_pay_totals (Json::Value& jheader,
                                            salesReceiptType receiptType);
  void                  receipt_vat_breakdown (salesReceiptType receiptType,
                                               bool calculate, std::string& netSalesAmount);
  ////////////////////////////////////////
  //  ALT json method(s)
  std::string getAltValue (std::string name,
                           std::vector<STRUCT_ALTJSON*> array);
  std::string getAltName (int idx,
                          std::vector<STRUCT_ALTJSON*> array);

  int safeJsonInt (std::string name, Json::Value jval);
  double safeJsonDouble (std::string name, Json::Value jval);
  std::string safeJsonString (std::string name, Json::Value jval);
  Json::Value safeJsonObject (std::string name, Json::Value jval);
  ////////////////////////////////////////
  //  Sales retrieval method(s)
  virtual
  bool retrieveSalesRecord (Json::Value& jheader,
                            std::string systransnum,
                            salesReceiptType type);
  virtual
  bool retrieveSalesHeader (Json::Value& jheader,
                            std::string systransnum,
                            salesReceiptType type);
  virtual
  bool retrieveSalesTransAccount (Json::Value& jheader);
  virtual
  bool retrieveSalesHeaderAccount (Json::Value& jheader);

  virtual
  bool retrieveSalesDetail (Json::Value& jheader,
                            std::string branch_code,
                            std::string systransnum);

  virtual
  bool retrieveSalesPayment (Json::Value& jheader,
                             std::string systransnum);
  ////////////////////////////////////////
  //  Export method(s)
  virtual
  bool                  assembleSqlInsert (CYDbSql* db,
                                           std::string tableName,
                                           std::string& sqlInsert);
  ////////////////////////////////////////
  //  Loyalty method(s)
  virtual
  bool                  assembleFspUrl (Json::Value& jheader,
                                        Json::Value& jdetail,
                                        Json::Value& jpayment)
  {
      (void)jheader; (void)jdetail; (void)jpayment; return true;
  }
  virtual
  bool                  assembleFspHeader (Json::Value& jheader)
  {
      (void)jheader; return true;
  }
  virtual
  bool                  assembleFspDetail (Json::Value& jdetail)
  {
      (void)jdetail; return true;
  }
  virtual
  bool                  assembleFspTotals (Json::Value& jheader)
  {
      (void)jheader; return true;
  }
  virtual
  bool                  assembleFspPayment (Json::Value& jpayment)
  {
      (void)jpayment; return true;
  }
  virtual
  bool                  assembleFspAccount (Json::Value& jheader)
  {
      (void)jheader; return true;
  }
  virtual
  bool                  assembleReceipt (Json::Value& jheader,
                                         Json::Value& jdetail,
                                         Json::Value& jpayment,
                                         salesReceiptType receiptType)
  {
      (void)jheader; (void)jdetail; (void)jpayment; (void)receiptType; return true;
  }
  //////////////////////////////////////
  //  Database
  SqliteDb*             _liteDb;
  SqliteEnv*            _liteEnv;
  ////////////////////////////////////////
  //  Utility method(s)
  std::string           FMTNumberComma(double num);
  std::string           date_rcc(std::string date);
  std::string           FMTStrDotted(std::string str1,
                                     std::string str2);
  ////////////////////////////////////////
  //  Conversion method(s)
  double              stodsafe(std::string val)  {
      if (val.length() < 1) val = "0";
      cyutility util;
      if (!util.valid_decimal("stodsafe",val.c_str (),1,32,-99999999.99,999999999.99))
          val = "0";
      return stod(val);
  }
  ////////////////////////////////////////
  //  FNB retrieval method(s)
  virtual
  bool retrieveFnbRecord (Json::Value& jheader,
                          cyposrest* rest,
                          std::string table_code,
                          std::string customer_number,
                          std::string register_number);
  ////////////////////////////////////////
  //  FSP url
  STRUCT_FSPURL         _fspurl;
  ////////////////////////////////////////
  //  Retrieved data
  Json::Value           _jdetail;
  Json::Value           _jheader;
  Json::Value           _jpayment;
  salesReceiptType      _receiptType;
  ////////////////////////////////////////
  //  Receipt attribute(s)
  std::string           _receipt;
  std::string           _sqlSales;
  std::string           _exportType;
  std::string           _suspendNum;
  std::string           _resRmaNumber;
  std::string           _txtcopies, _txttype;
  std::string           _txthdr, _txtdet, _txtpay,
                        _txttot, _txtvat, _txtsig,
                        _txtfoot, _txtfootloyalty,
                        _txtfootbeauty, _txtgift, _txtacct,
                        _txtpromoitem, _txtloyalty, _txtbeauty;
  ////////////////////////////////////////
  //  Xread stats
  struct_tax _xread_tax;
  struct_totals _xread_totals;
  std::vector<struct_bank*> _xread_bank;
  std::vector<struct_payment*> _xread_payment;
  std::vector<struct_discount*> _xread_discount;
  ////////////////////////////////////////
  //  Receipt attribute(s)
  bool                  _isPwd,
                        _isCash,
                        _isSen5,
                        _isExtra,
                        _isSkinny,
                        _isSenior,
                        _isEpurse,
                        _isDrawer,
                        _isResRma,
                        _isSigLine,
                        _isSigText,
                        _isGiftWrap,
                        _isGiftPromo,
                        _isInternal,
                        _isZeroRated;
  bool                  _hdrDiscountPct,
                        _noVatNotDiscounted;
  double                _hdrDiscountRate;
  std::string           _hdrDiscountCode,
                        _hdrDiscountDesc;

  ////////////////////////////////////////
  //  Receipt total(s)
  double                _totalItems=0;
  double                _totalAmtDue=0;
  double                _totalNetSales=0;
  double                _totalVatAmount=0;
  double                _totalTrxDiscount=0;
  double                _totalGrossAmount=0;
  double                _totalItemDiscount=0;
  double                _totalPaymentDiscount=0;

  double                _totalNetVat=0;
  double                _totalNetZero=0;
  double                _totalNetExempt=0;

  double                _totalAmtVat=0;
  double                _totalAmtZero=0;
  double                _totalAmtExempt=0;

  double                _totalDiscountZero=0;

  double                _totalAddonAmount=0;
  double                _totalSplitAdjust=0;

  double                _fnbFood=0;
  double                _fnbPayAmount=0;
  double                _fnbBeverage=0;

  double                _totalServiceCharge=0;  

  void                  resetEodTotals ();

protected:
  ////////////////////////////////////////
  //  Object(s)
  cylog*                _log;
  cyini*                _ini;
  cycgi*                _cgi;
  cyutility*            _util;
  ////////////////////////////////////////
  //  Cleanup flags
  bool                  _cleanlog;
  bool                  _cleanini;
  ////////////////////////////////////////
  //  Receipt width
  int                   _rcptWidth;
  ////////////////////////////////////////
  //  Receipt calculation(s)
  bool                mark_detail_trx (std::string trxtype,
                                       std::string systransnum,
                                       std::string item_seq);
  virtual
  bool                  detailCalculate (Json::Value& jdetail,
                                         std::string branch_code);
  virtual
  bool                  detailCalculateSenior (Json::Value& jdetail);
  virtual
  bool                  detailCalculateSeniorWeighted (Json::Value& jdetail);
  virtual
  bool                  detailCalculateRegular (Json::Value& jdetail);
  virtual
  bool                  detailCalculateRegularWeighted (Json::Value& jdetail);
  virtual
  bool                  detailCalculateZeroRated (Json::Value& jdetail);
  virtual
  bool                  detailCalculateZeroRatedWeighted (Json::Value& jdetail);
  virtual
  bool                  paymentCalculate(Json::Value jbank, Json::Value& jpayment);

  ////////////////////////////////////////
  //  RCC specific
  bool                  _isBaMember,
                        _isFspMember;

  ////////////////////////////////////////
  //  Error handling
  std::string           _error;
  virtual
  bool                  seterrormsg (std::string message) {
      _error = message;
      _log->logdebug("cyrestsales",_error.c_str ());
      return false;
  }
  virtual
  salesQueryType        seterrorquery (std::string message)  {
      seterrormsg(message);
      return salesQueryType::SQT_ERROR;
  }
  ////////////////////////////////////////
  //  Offline sales
  bool                  saveOfflineSales (std::string systransnum,
                                          std::string exportType,
                                          std::string sqlSales,
                                          std::string jsonSales);
  //////////////////////////////////////
  //  JSON utility method(s)
  virtual
  salesQueryType        json_select_single (CYDbSql* db,
                                            Json::Value& jval,
                                            std::string sql,
                                            std::string eofmsg="No record(s) found");
  virtual
  salesQueryType        json_select_multiple (CYDbSql* db,
                                              Json::Value& jval,
                                              std::string sql,
                                              std::string eofmsg="No record(s) found");
};
//////////////////////////////////////////
//  Accumulated total(s) structure
typedef struct  {
    std::string         login;
    std::string         last_name;
    std::string         first_name;
    std::string         register_num;
} STRUCT_EOD_USER;
///////////////////////////////////////////
//  Bank list
typedef struct  {
    int                 count;
    double              amount;
    std::string         bankDesc;
    std::string         bankCode;
} STRUCT_EOD_BANK;
///////////////////////////////////////////
//  Payment list
typedef struct  {
    int                 count;
    bool                iscash;
    bool                cashdec;
    double              amount;
    double              declared;
    std::string         tenderDesc;
    std::string         tenderCode;
    std::string         tenderTypeCode;
    std::string         tenderTypeDesc;
} STRUCT_EOD_PAYMENT;
///////////////////////////////////////////
//  Discount list
typedef struct  {
    int                 count;
    double              amount;
    std::string         discountDesc;
    std::string         discountCode;
    std::string         discountTypeCode;
} STRUCT_EOD_DISCOUNT;
//////////////////////////////////////////
//  Sales table struct
typedef struct  {
    bool doubleCheck;
    std::string tableName;
} STRUCT_SALES_TABLES;
///////////////////////////////////////////
//  Void / Cancel list
typedef struct  {
    double              amount;
    std::string         trxnum, ordernum;
} STRUCT_EOD_VOIDCANCEL;
//////////////////////////////////////////
//  FSP total(s)
typedef struct  {
    int                 count;
    double              points;
    double              epurse;
    std::string         last_name;
    std::string         first_name;
    std::string         middle_name;
    std::string         acct_type_code;
    std::string         account_number;
} STRUCT_EOD_FSP;
//////////////////////////////////////////
//  Sales table list
const STRUCT_SALES_TABLES g_sales_tables [] {
  { false, "tg_pos_mobile_detail" },
  { false, "tg_pos_mobile_alias" },
  { false, "tg_pos_email_receipt" },
  { false, "tg_pos_gc_detail" },
  { false, "tg_pos_mobile_modifier" },
  { false, "tg_pos_mobile_header_receipt" },
  { false, "tg_pos_mobile_queue" },
  { false, "tg_pos_mobile_audit" },
  { false, "tg_pos_mobile_gc_payment" },
  { false, "tg_pos_mobile_split" },
  { false, "tg_pos_mobile_detail_split" },
  { false, "tg_pos_mobile_addon_detail" },
  //{ false, "tg_pos_mobile_detail_totals" },
  { false, "tg_pos_mobile_discount_detail" },
  { false, "tg_pos_mobile_fnb" },
  { false, "tg_pos_mobile_lanebust" },
  { true,  "tg_pos_mobile_consign" },
  { true,  "tg_pos_mobile_trxaccount" },
  { false, "tg_pos_discount_summary" },
  { false, "tg_pos_mobile_tax" },
  { false, "tg_pos_mobile_audit" },
  { true,  "tg_pos_mobile_transtype" },
  { true,  "tg_pos_mobile_discount" },
  { true,  "tg_pos_mobile_currency" },
  { false, "tg_pos_mobile_account" },
  { false, "tg_pos_mobile_trxaccount" },
  { false, "tg_pos_mobile_discount_payment" },
  { false, "tg_pos_mobile_bank_payment" },
  { false, "tg_pos_mobile_payment" },
  { false, "tg_pos_journal" },
  //{ false, "tg_pos_mobile_header"  },
  { false, "eof"  },
};
//*******************************************************************
//       CLASS:         CYRestReceipt
// DESCRIPTION:         POS sales object
//*******************************************************************
class                   CYRestReceiptEod : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestReceiptEod ();
  CYRestReceiptEod (cylog* log, cyini* ini, cycgi* cgi,
                    CYRestCommon* receiptType);
  ////////////////////////////////////////
  //  Retrieve relevant information
  bool                  voidTotal ();
  bool                  cancelTotal ();
  bool                  refundTotal ();
  bool                  suspendTotal ();
  bool                  fnbOrderTotal ();

  bool                  offlineSales ();
  bool                  retrieveEodData ();
  bool                  retrieveEodForce ();
  bool                  retrieveFastX ();
  bool                  managerauth (std::string manager,
                                     std::string mgrpass);
  ////////////////////////////////////////
  //  Attribute(s)
  //  receipt object
  CYRestCommon*         receipt;
  salesReceiptType      rcptType;
  //  receipt totals
  double                rcptVat;
  double                rcptVatZero;
  double                rcptVatable;
  double                rcptVatExempt;

  double                rcptNetSales;
  double                rcptGrossSales;
  double                rcptServiceCharge;

  double                totCashFund;
  double                totCashPullout;
  //  retrieval argument(s)
  bool                  argXread;
  std::string           argCashier, argManager, argRegister, argSysdate,
                        argBranch, argShift, argSuspended, argIniReceipt;
  //  earliest / lastest - date & time
  time_t                endDate,   // max date
                        startDate; // earliest date
  //  start / end transaction
  int                   trxStart, trxEnd;
  std::string           endSystransnum,
                        startSystransnum;
  //  accumulated total(s)
  int                   cntCash;    //  cash trx count
  double                totCash;    //  cash trx amount
  int                   cntNonCash; //  non cash trx count
  double                totNonCash; //  non cash trx amount

  int                   xreadCount; //  xread attempts
  int                   zreadCount; //  zread attempts

  int                   fnbCount;   //  FNB order count
  int                   voidCount;  //  void trx count
  double                voidAmount; //  void amount

  int                   refundCount;  //  refund trx count
  double                refundAmount; //  refund amount

  int                   cancelCount;  //  cancel trx count
  double                cancelAmount; //  cancel amount

  int                   suspendRecs;   //  suspended record(s)
  double                suspendCount;  //  no of items suspended
  double                suspendAmount; //  total suspended amount

  int                   trxCount;      //  number of transaction(s)
  int                   nonTrxCount;   //  number of non sales transaction(s)
  double                itemCount;     //  number of items sold
  double                giftWrapCount; //  number of gift wrap items
  //  list of FSP customers
  std::vector<STRUCT_EOD_FSP*> listFspCustomer;
  //  list of cashiers without xread
  std::vector<STRUCT_EOD_USER*>  listUserXread;
  //  list of bank(s)
  std::vector<STRUCT_EOD_BANK*>  listBank;
  //  list of cash fund tender type(s)
  std::vector<STRUCT_EOD_PAYMENT*>  listFund;
  //  list of cash declaration tender type(s)
  std::vector<STRUCT_EOD_PAYMENT*>  listDeclare;
  //  list of cash pullout tender type(s)
  std::vector<STRUCT_EOD_PAYMENT*>  listPullout;
  //  list of payment type(s)
  std::vector<STRUCT_EOD_PAYMENT*>  listPay;
  //  list of cash declaration type(s)
  std::vector<STRUCT_EOD_PAYMENT*>  listCashdec;
  //  list of payment discount(s)
  std::vector<STRUCT_EOD_DISCOUNT*> listPayDiscount;
  //  list of item discount(s)
  std::vector<STRUCT_EOD_DISCOUNT*> listItemDiscount;
  //  list of void transactions
  std::vector<STRUCT_EOD_VOIDCANCEL*> listVoid;
  //  list of cancelled transactions
  std::vector<STRUCT_EOD_VOIDCANCEL*> listCancel;
  ////////////////////////////////////////
  //  JSON alt method(s) / structure(s)
  std::vector<STRUCT_ALTJSON*> alt_settings;
  std::vector<STRUCT_ALTJSON*> alt_settings_branch;
  std::vector<STRUCT_ALTJSON*> alt_settings_sysdate;
  std::vector<STRUCT_ALTJSON*> alt_settings_company;
  std::vector<STRUCT_ALTJSON*> alt_settings_cashier;
  std::vector<STRUCT_ALTJSON*> alt_settings_manager;
  std::vector<STRUCT_ALTJSON*> alt_settings_register;
  std::vector<STRUCT_ALTJSON*> alt_settings_currency;
protected:
  ////////////////////////////////////////
  //  Retrieval method(s)
  bool                  voidLoop ();
  bool                  salesLoop ();
  bool                  refundLoop ();
  bool                  cancelLoop ();
  bool                  suspendLoop ();

  bool                  json_settings ();
  bool                  json_settings_branch ();
  bool                  json_settings_sysdate ();
  bool                  json_settings_company ();
  bool                  json_settings_cashier ();
  bool                  json_settings_manager ();
  bool                  json_settings_register ();
  bool                  json_settings_currency ();
  ////////////////////////////////////////
  //  Internal method(s)
  void                  reset ();
  bool                  cashFund ();
  bool                  canZread ();
  bool                  cashDeclare ();
  bool                  cashPullout ();
  bool                  taxTotals (Json::Value jtax);
  bool                  checkDate (Json::Value jheader);
  bool                  itemTotals (Json::Value jheader,
                                    Json::Value jlistdetail);
  bool                  paymentTotals (Json::Value jpayment);
  ////////////////////////////////////////
  //  FAST internal method(s)
  bool                  listFspFast ();
  bool                  listTaxFast ();
  bool                  canZreadFast ();
  bool                  listBankFast ();
  bool                  listCashdecFast ();
  bool                  listPaymentFast ();
  bool                  listPulloutFast ();
  bool                  listDiscountFast ();
  bool                  listCashfundFast ();
  bool                  listItemCountFast ();

  bool                  voidLoopFast ();
  bool                  salesLoopFast ();
  bool                  refundLoopFast ();
  bool                  cancelLoopFast ();
  bool                  suspendLoopFast ();
};

#endif
