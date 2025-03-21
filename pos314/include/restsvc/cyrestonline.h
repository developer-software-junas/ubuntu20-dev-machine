/********************************************************************
          FILE:         restonline.h
   DESCRIPTION:         REST data for flutter
 ********************************************************************/
#ifndef __CYRESTONLINE_H
#define __CYRESTONLINE_H
//////////////////////////////////////////
//  CYWare class(es)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cydb/cyw_db.h"
//////////////////////////////////////////
//  Database object(s)
#include "cydb/cydbsql.h"
//#include "cydb/cysqlite.h"
//////////////////////////////////////////
//  Header type
typedef enum  {
    MOBILE, SUSPEND, CANCEL, POSTVOID, DAILY
}  enumHeader;
//////////////////////////////////////////
//  Login type
typedef enum  {
    CASHIER, MANAGER
}  enumLogin;
//////////////////////////////////////////
//  Query result
typedef enum  {
    ERROR, SUCCESS, NOROWS
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
//  Loyalty transaction structure
struct STRUCT_FSPTRANS {
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
const std::string gstr_audit [] = {
    std::string ("Login"),
    std::string ("Cashfund"),
    std::string ("Transaction type"),
    std::string ("Transaction account"),
    std::string ("Account"),
    std::string ("Transaction discount"),
    std::string ("Account delete"),
    std::string ("Logout")
};
//////////////////////////////////////////
//  Classes used
class                   cyposaudit;
class                   CGIHandler;
/********************************************************************
      FUNCTION:         cyrestnline
   DESCRIPTION:         POS REST data class
 ********************************************************************/
class                   cyrestonline
{
public:
    //////////////////////////////////////
    //  Constructor - LOCAL POS or SERVER
    cyrestonline (cyini* ini, cylog* log, cycgi* cgi);
    virtual ~cyrestonline ();
    std::string         rest_publish ();
    //////////////////////////////////////
    //  Method(s)
    CYDbSql*            dbconn ();
    std::string         cgiform(std::string name);
    std::string         request(std::string cyrequest);
    //////////////////////////////////////
    //  List(s)
    std::string         rest_translist ();
    std::string         rest_tenderlist ();
    std::string         rest_transaccount ();
    std::string         rest_transtypeacct ();
    //////////////////////////////////////
    //  General service(s)    
    std::string         rest_sales ();
    std::string         rest_logout ();
    std::string         rest_settings ();
    std::string         rest_reprintlu ();
    std::string         rest_auditlog ();
    std::string         rest_postvoid ();
    std::string         rest_postvoidlu ();
    std::string         rest_credentials ();
    std::string         rest_credentials_manager ();
    //////////////////////////////////////
    //  Sales cleanup
    bool                rest_sales_done (CYDbSql* db,
                                         std::string systransnum);
    bool                rest_sales_remote (CYDbSql* db,
                                           std::string systransnum);
    //////////////////////////////////////
    //  Printing method(s)
    std::string         rest_textprint ();
    //////////////////////////////////////
    //  End of day method(s)
    std::string         rest_xread ();
    std::string         rest_zread ();
    std::string         rest_xaudit ();
    std::string         rest_updateeod ();
    std::string         rest_xreadsave ();
    std::string         rest_zreadsave ();
    //////////////////////////////////////
    //  Cashier services
    std::string         rest_cashfund ();
    std::string         rest_cashfundlist();
    std::string         rest_cashtakeout ();
    //////////////////////////////////////
    //  Suspend / resume
    std::string         rest_listsuspended ();
    std::string         rest_listsuspendedcashier ();
    std::string         rest_resume_transaction ();
    std::string         rest_suspend_transaction ();
    //////////////////////////////////////
    //  Transaction related
    std::string         rest_pacvalidate ();
    std::string         rest_cancel_transaction ();
    std::string         rest_updatetranstype (std::string trxnum,
                                              bool islog=true);
    //////////////////////////////////////
    //  Product method(s)
    std::string         rest_listproduct ();
    std::string         rest_deletedetail ();
    std::string         rest_updatedetail ();
    //////////////////////////////////////
    //  Discount method(s)
    std::string         rest_listdiscount ();    
    std::string         rest_update_discount ();

    std::string         rest_updatedetail_price ();
    std::string         rest_updatedetail_quantity ();
    std::string         rest_updatedetail_discount ();
    std::string         rest_updateglobal_discount ();
    std::string         rest_updatedetail_quantity_giftwrap ();
    //////////////////////////////////////
    //  Account method(s)
    std::string         rest_addaccount ();
    std::string         rest_listaccount ();
    std::string         rest_guestaccount ();
    std::string         rest_updateaccount ();
    std::string         rest_deleteaccount ();
    std::string         rest_chargeaccount ();
    std::string         rest_updateaccounttrx ();
    //////////////////////////////////////
    //  Hacked account methods
    std::string         rest_updateaccountlu ();
    //////////////////////////////////////
    //  Payment method(s)
    std::string         rest_gcupdate ();
    std::string         rest_updatepayment ();
    std::string         rest_deletepayment ();
    std::string         rest_deletepaymentgc ();
    //////////////////////////////////////
    //  FNB json
    std::string         rest_fnb (std::string branchcode,
                                  std::string areacode,
                                  std::string scetioncode);
    std::string         rest_fnb_table ();
    std::string         rest_fnb_tablesection ();
    std::string         rest_fnb_tabletransfer();
    std::string         rest_fnb_tableorderupdate ();
    std::string         rest_fnb_tableorderdelete ();

    std::string         rest_fnb_tablecategoryupdate ();

    std::string         rest_fnb_suspend ();
    std::string         rest_fnb_suspend_table ();
    std::string         rest_fnb_suspend_table_split ();

    std::string         rest_fnb_customerlist ();
    std::string         rest_fnb_customerupdate ();
    std::string         rest_fnb_customerdelete ();

    std::string         rest_fnb_tablesplit ();
    std::string         rest_fnb_tablesplitways ();
    std::string         rest_fnb_tablesplitreset ();
    std::string         rest_fnb_tablesplitupdate();
    //////////////////////////////////////
    //  FNB utility method(s)
    bool                rest_fnb_item (Json::Value& jorderitem,
                                       std::string recipe_item_code);
    bool                rest_fnb_order (Json::Value& jorder,
                                        std::string table_code,
                                        std::string customer_number);
    bool                rest_fnb_customer (Json::Value& jcustomer,
                                           std::string customer_number);
    //////////////////////////////////////
    //  Sales method(s)
    bool                jsonfmt_sales (enumHeader type,
                                       Json::Value& jsales,
                                       std::string systransnum);
    bool                jsonfmt_header (Json::Value& root,
                                        enumHeader type,
                                        std::string systransnum);
    bool                jsonfmt_header_transaction (Json::Value& jhdr,
                                                    std::string systransnum);
    bool                jsonfmt_detail (Json::Value& jhdr,
                                        std::string systransnum);
    bool                jsonfmt_payment (Json::Value& jhdr,
                                         std::string systransnum);
    //////////////////////////////////////
    //  Manager authorization
    bool                cashierauth (std::string username,
                                      std::string password);
    bool                managerauth (std::string username,
                                      std::string password);
    //////////////////////////////////////
    //  Account method(s)
    bool                account_type (std::string account_type_code,
                                      std::string& account_sql_flag,
                                      Json::Value& jaccount_type);
    //////////////////////////////////////
    //  Sales header service(s)
    bool                get_xvalue (std::string& xvalue);
    bool                get_points (CYDbSql* dbupd,
                                    std::string systransnum);
    bool                get_sysdate(std::string location_code,
                                    std::string& sysdate);
    bool                eod_sysdate(std::string location_code,
                                    std::string& sysdate);
    //bool                get_transnum (std::string& transnum);
    bool                get_transnum (std::string location_code,
                                      std::string register_num,
                                      std::string& transnum);
    bool                get_nontrxnum (std::string location_code, std::string sysdate,
                                       std::string cashier, std::string register_num,
                                       std::string shift, std::string& trxnum);
    bool                get_cashier_shift (std::string location_code,
                                           std::string sysdate, std::string cashier,
                                           std::string register_num, std::string& shift);

    bool                new_header (CYDbSql* db,
                                    std::string& systransnum,
                                    std::string company_code,
                                    std::string branch_code,
                                    std::string register_num,
                                    std::string cashier,
                                    std::string transaction_code="");
    bool                new_fnbheader  (CYDbSql* db, char* szidx,
                                        std::string& systransnum,
                                        std::string company_code,
                                        std::string branch_code,
                                        std::string register_num,
                                        std::string cashier,
                                        std::string transaction_code="");
    //////////////////////////////////////
    //  Sales detail service(s)
    bool                new_detail (CYDbSql* db,
                                    std::string systransnum,
                                    std::string xvalue,
                                    std::string& item_seq,
                                    Json::Value jtax, Json::Value jproduct,
                                    Json::Value jdiscountitem, Json::Value jdetail);
    bool                product_tax (std::string item_code,
                                     std::string quantity,
                                     std::string retail_price,
                                     Json::Value& jtax);
    bool                product_detail (std::string item_code,
                                        std::string location_code,
                                        Json::Value& jproduct);
    bool                product_beauty (std::string item_code,
                                        std::string location_code,
                                        Json::Value& jbeauty);
    //////////////////////////////////////
    //  Product discount
    bool                product_discount (std::string item_code,
                                          std::string transaction_code,
                                          std::string location_code,
                                          std::string retail_price,
                                          Json::Value& jdiscountitem);
    bool                product_discount_event (std::string item_code,
                                                std::string location_code,
                                                Json::Value& jdiscountitem);
    bool                product_discount_group (std::string item_code,
                                                std::string location_code,
                                                std::string retail_price,
                                                Json::Value& jdiscountitem);
    //////////////////////////////////////
    //  REST key validation
    bool                validatekey (std::string hmac,
                                     std::string request,
                                     std::string session,
                                     std::string timestamp);
    //////////////////////////////////////
    //  Sales utility method(s)
    bool                update_sales_tables (CYDbSql* db,
                                             std::string oldtransnum,
                                             std::string newtransnum);
    bool                cleanup_sales_tables (CYDbSql* db,
                                              std::string systransnum);
    //////////////////////////////////////
    //  Database connection list
    std::vector<CYDbSql*> _dbList;
    //////////////////////////////////////
    //  Object access
    cyini               *_ini;
    cylog               *_log;
    cycgi               *_cgi;
    cyutility           *_util = nullptr;  // Useful function(s)

    CYDbEnv             *_env;
    Json::Value         _jdevice, _jsales, _jfnb;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    CGIHandler*         _cgiHandler;
    cyposaudit*         _audit;
    std::string         _error = "",       // Error string
                        _request = "";     // REST request
    //////////////////////////////////////
    //  Internal key validation
    std::string         digikey  (std::string  queryargs);
    bool                parsekey (std::string  queryargs,
                                  std::string& digikey,
                                  std::string& cleanargs);
    void                appendkey(std::string& queryargs);
    //////////////////////////////////////
    //  JSON utility method(s)
    enumQueryType       json_select_single (CYDbSql* db,
                                            Json::Value& jval,
                                            std::string sql);
    enumQueryType       json_select_multiple (CYDbSql* db,
                                              Json::Value& jval,
                                              std::string sql);
    //////////////////////////////////////
    //  FNB utility function(s)
    bool                anonymousorder (CYDbSql* db,
                                        std::string table_code,
                                        std::string& customer_number);
    bool                migratefnbitem (std::string location_code,
                                        std::string recipe_item_code);
public:
    //////////////////////////////////////
    //  Error message access
    std::string         errormsg ();
    std::string         errorjson (std::string errormsg);
    bool                seterrormsg (std::string message);
    enumQueryType       seterrorquery (std::string message)  {
        seterrormsg(message);
        return enumQueryType::ERROR;
    }
    //////////////////////////////////////
    //  Temporary variable(s)
    bool                salesDone;
    std::string         tmpString;
    //////////////////////////////////////
    //  Sales totals
    STRUCT_FSPTRANS     _fspTrans;
    STRUCT_SALESTOTAL   _salesTotal;
};
/********************************************************************
      FUNCTION:         cyposaudit
   DESCRIPTION:         POS audit data class
 ********************************************************************/
class                   cyposaudit
{
public:
    //////////////////////////////////////
    //  Constructor
    cyposaudit (cyposrest* rest);
    virtual ~cyposaudit ();
    //////////////////////////////////////
    //  Audit method(s)
    bool                logsql(std::string sql);
    bool                login (CYDbSql* db,
                               std::string cashier,
                               std::string register_num,
                               std::string location_code);

    bool                auditlog(CYDbSql* db,
                                 std::string action,
                                 std::string cashier,
                                 std::string register_num,
                                 std::string location_code,
                                 std::string audit_info,
                                 std::string systransnum,
                                 std::string cashier_shift="");

protected:
    cyposrest*          _rest;

    bool                new_xread (CYDbSql* db, int shift,
                                   std::string cashier,
                                   std::string logical_date,
                                   std::string register_num,
                                   std::string location_code);
};
#endif
