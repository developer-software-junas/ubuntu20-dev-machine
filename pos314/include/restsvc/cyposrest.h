/********************************************************************
          FILE:         cyposrest.h
   DESCRIPTION:         REST data for flutter
 ********************************************************************/
#ifndef __CYPOSREST_H
#define __CYPOSREST_H
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
#include "restsvc/storesales/cyrestcommon.h"
//////////////////////////////////////////
//  Classes used
class                   cyerpsales;
class                   cyposaudit;
class                   CGIHandler;
class                   CYRestReceipt;
class                   QByteArray;

const int               g_paperWidth = 40;
const bool              confidential = false;
/********************************************************************
      FUNCTION:         cyposrest
   DESCRIPTION:         POS REST data class
 ********************************************************************/
class                   cyposrest
{
public:
    //////////////////////////////////////
    //  Constructor - LOCAL POS or SERVER
    cyposrest (cyini* ini, cylog* log, cycgi* cgi);
    virtual ~cyposrest ();
#ifdef _CYSTORESERVER
    std::string         rest_publish ();
    std::string         rest_posdevice ();
    std::string         rest_device_check ();
    std::string         rest_cashier_check ();
    std::string         rest_cashier_reset ();
    std::string         rest_online_suspend ();
#else
    std::string         rest_download_status ();
#endif
    //////////////////////////////////////
    //  Method(s)
    std::string         cgiform(std::string name);
    std::string         request(std::string cyrequest);
    //////////////////////////////////////
    //  List(s)
    std::string         rest_translist ();
    std::string         rest_tenderlist ();
    std::string         rest_transaccount ();
    std::string         rest_transtypeacct ();
    std::string         rest_translistfnb ();
    std::string         rest_transaccountlist ();
    //////////////////////////////////////
    //  General service(s)    
    std::string         rest_sales ();
    std::string         rest_sales_record ();
    std::string         rest_logout ();
    std::string         rest_settings ();
    std::string         rest_reprintlu ();
    std::string         rest_auditlog ();
    std::string         rest_postvoid ();
    std::string         rest_postvoidlu ();
    std::string         rest_credentials ();
    std::string         rest_changepassword ();
    std::string         rest_credentials_manager ();

    std::string         rest_refundlu ();
    std::string         rest_refundinfo ();
    std::string         rest_refundsave ();
    std::string         rest_refundclean ();
    std::string         rest_refundreverse ();
    //////////////////////////////////////
    //  Sales cleanup
    bool                rest_sales_done (CYDbSql* db,
                                         std::string& systransnum);
    bool                rest_sales_remote (CYDbSql* db,
                                           std::string systransnum);
    bool                rest_sales_done_redeem (CYDbSql* db,
                                                std::string& systransnum);
    //////////////////////////////////////
    //  Printing method(s)
    std::string         rest_textprint ();
    //////////////////////////////////////
    //  End of day method(s)
    std::string         rest_xaudit ();
    std::string         rest_updateeod ();
    //////////////////////////////////////
    //  Cashier services
    std::string         rest_cashfund ();
    std::string         rest_cashfundlist();
    std::string         rest_cashtakeout ();
    //////////////////////////////////////
    //  Suspend / resume
    std::string         rest_listsales ();
    std::string         rest_listbranch ();
    std::string         rest_listsubclass ();
    std::string         rest_listsalesconsign ();

    std::string         rest_listoptical ();
    std::string         rest_listconsign ();
    std::string         rest_listsuspended ();
    std::string         rest_listsuspendedcashier ();
    std::string         rest_suspend_transaction ();

    std::string         rest_consignrandom ();
    std::string         rest_suspend_consignpos ();
    std::string         rest_suspend_consignsingle ();
    std::string         rest_suspend_consignmultiple ();
    std::string         rest_suspend_concessionaire ();

    std::string         rest_resume_local ();
    std::string         rest_resume_remote ();



    std::string         rest_delete_consign ();
    std::string         rest_resume_consign ();
    std::string         rest_resume_consignadd ();
    std::string         rest_resume_consignpos ();
    std::string         rest_resume_transaction ();
    //////////////////////////////////////
    //  Transaction related
    std::string         rest_trxdefault ();
    std::string         rest_pacvalidate ();
    std::string         rest_deletetrxaccount ();
    std::string         rest_default_transtype ();
    std::string         rest_cancel_transaction ();
    std::string         rest_updatetranstype (std::string trxnum,
                                              std::string keepAccount,
                                              bool islog=true);
    //////////////////////////////////////
    //  Product method(s)
    std::string         rest_listproduct ();
    std::string         rest_deletedetail ();
    std::string         rest_updatedetail (bool isQuantityOverride = false,
                                           STRUCT_DETAILUPDATE* structDetail=nullptr);
    //////////////////////////////////////
    //  Discount method(s)
    std::string         rest_listdiscount ();    
    std::string         rest_update_discount ();

    std::string         rest_retailsplitqty ();
    std::string         rest_retailsplitways ();
    std::string         rest_retailsplitreset ();
    std::string         rest_split_discountlu ();

    std::string         rest_updatedetail_price ();
    std::string         rest_updatedetail_quantity ();
    std::string         rest_updatedetail_discount ();
    std::string         rest_updateglobal_discount ();
    std::string         rest_updatesenior_discount ();
    std::string         rest_updatedetail_breakout ();
    std::string         rest_updatedetail_seniorsplit ();
    std::string         rest_updatedetail_trxdiscount ();
    std::string         rest_updatedetail_servicecharge ();
    std::string         rest_updatedetail_quantity_giftwrap ();

    std::string         rest_deletetrxdiscount ();
    std::string         rest_deletedetail_discount ();
    //////////////////////////////////////
    //  Account method(s)
    std::string         rest_addaccount ();
    std::string         rest_guestaccount ();

    std::string         rest_updatecrm ();
    std::string         rest_updateridc ();
    std::string         rest_updateclerk ();
    std::string         rest_updateaccount ();
    std::string         rest_deleteaccount ();
    std::string         rest_chargeaccount ();
    std::string         rest_cashdeclarelu ();
    std::string         rest_updateaccounttrx ();

    std::string         rest_gettaxaccount ();
    std::string         rest_cleartaxaccount ();
    std::string         rest_updatetaxaccount ();

    std::string         rest_listclerk ();
    std::string         rest_listaccount ();
    std::string         rest_listshopper ();
    std::string         rest_listcaacbank ();
    std::string         rest_listfranking ();
    std::string         rest_listaccount_ba ();
    std::string         rest_listaccount_fsp ();
    std::string         rest_listaccount_ycc ();
    std::string         rest_listpayment_fsp ();
    //////////////////////////////////////
    //  Hacked account methods
    std::string         rest_register_reset ();
    std::string         rest_updateaccountlu ();
    //////////////////////////////////////
    //  Payment method(s)
    std::string         rest_gcupdate ();
    std::string         rest_gcbalance ();
    std::string         rest_charge_balance ();

    std::string         rest_finishpayment ();
    std::string         rest_updatepayment ();
    std::string         rest_deletepayment ();
    std::string         rest_deletepaymentgc ();
    //////////////////////////////////////
    //  Receipt journal method(s)
    std::string         rest_auditlu ();
    std::string         rest_journallu ();
    std::string         rest_journalinfo ();
    //////////////////////////////////////
    //  FNB json
    std::string         rest_fnb (std::string branchcode,
                                  std::string areacode,
                                  std::string scetioncode);
    std::string         rest_fnb_table ();
    std::string         rest_fnb_table_lock ();
    std::string         rest_fnb_tableupdate ();
    std::string         rest_fnb_tableresume ();
    std::string         rest_fnb_tablesection ();
    std::string         rest_fnb_tabletransfer();
    std::string         rest_fnb_tableorderadd();
    std::string         rest_fnb_tableorderprint ();
    std::string         rest_fnb_tableorderupdate ();
    std::string         rest_fnb_tableorderdelete ();
    std::string         rest_fnb_tableorderquantity ();
    std::string         rest_fnb_tableorderbreakout ();
    std::string         rest_fnb_tableordertransfer ();
    std::string         rest_fnb_tableordertransferlu ();

    std::string         rest_fnb_check_reopen ();
    std::string         rest_fnb_tablecategoryupdate ();

    std::string         rest_fnb_table_splitqty ();
    std::string         rest_fnb_transtypeupdate ();

    std::string         rest_fnb_suspend ();
    std::string         rest_fnb_suspend_clear ();
    std::string         rest_fnb_suspend_split ();
    std::string         rest_fnb_suspend_table ();
    std::string         rest_fnb_suspend_table_split ();
    std::string         rest_fnb_suspend_table_splitqty ();
    std::string         rest_fnb_resume (std::string customerid="");
    /*
    std::string         rest_fnb_resume_split (std::string server,
                                               std::string tabledesc,
                                               std::string server_first,
                                               std::string server_last);*/

    std::string         rest_fnb_resume_bench (std::string customerid="");
    std::string         rest_fnb_resume_split_bench (std::string server,
                                                     std::string tabledesc,
                                                     std::string server_first,
                                                     std::string server_last);

    std::string         rest_fnb_customerlist ();
    std::string         rest_fnb_customertable ();
    std::string         rest_fnb_customerclean ();
    std::string         rest_fnb_customerupdate ();
    std::string         rest_fnb_customerdelete ();
    std::string         rest_fnb_customerreceipt ();

    std::string         rest_fnb_itemsplit ();
    std::string         rest_fnb_ordersplit ();
    std::string         rest_fnb_tablesplit ();
    std::string         rest_fnb_tablesplitqty ();
    std::string         rest_fnb_tablesplitways ();
    std::string         rest_fnb_tablesplitreset ();
    std::string         rest_fnb_tablesplitupdate();
    std::string         rest_fnb_tablesplitapply();
    //  Update all
    std::string         rest_fnb_tablesplitupdateqty();
    //  Update one row
    std::string         rest_fnb_tablesplitqtyupdate();

    std::string         rest_fnb_hintlist ();
    std::string         rest_fnb_lanebust ();

    std::string         rest_fnb_takeoutadd ();
    std::string         rest_fnb_menulookup ();
    std::string         rest_fnb_menutakeout ();
    std::string         rest_fnb_kitchenupdate ();

    std::string         rest_fnb_clear_orders ();
    //////////////////////////////////////
    //  FNB utility method(s)
    bool                rest_fnb_item (Json::Value& jorderitem,
                                       std::string recipe_item_code);
    bool                rest_fnb_trx  (Json::Value& jtrx,
                                       std::string table_code,
                                       std::string customer_number);
    bool                rest_fnb_order (Json::Value& jorder,
                                        std::string table_code,
                                        std::string customer_number);
    bool                rest_fnb_customer (Json::Value& jcustomer,
                                           std::string customer_number);
    //////////////////////////////////////
    //  Sales method(s)
    bool                jsonfmt_sales (enumHeader type,
                                       Json::Value& jsales,
                                       std::string systransnum,
                                       bool update_eod=false);
    bool                jsonfmt_header (Json::Value& root,
                                        enumHeader type,
                                        std::string systransnum);
    bool                jsonfmt_header_transaction (Json::Value& jhdr,
                                                    std::string systransnum);
    bool                jsonfmt_detail (Json::Value& jhdr,
                                        std::string systransnum);
    bool                jsonfmt_payment (Json::Value& jhdr,
                                         std::string systransnum);
    bool                jsonfmt_kitchen (enumHeader type,
                                         std::string systransnum,
                                         std::string recipe_item_code,
                                         std::string& kitchen_receipt);
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
    bool                get_pointsurl (enumHeader type,
                                       std::string systransnum,
                                       std::string& fspurl);
    bool                get_sysdate(std::string location_code,
                                    std::string register_num,
                                    std::string& sysdate);
    bool                eod_sysdate(std::string location_code,
                                    std::string register_num,
                                    std::string& sysdate);

    bool                get_transnum (std::string location_code,
                                      std::string register_num,
                                      std::string& transnum);
    bool                get_nontrxnum (std::string location_code,
                                       std::string register_num,
                                       std::string& trxnum,
                                       bool is_commit=true);
    bool                get_voidtrxnum(std::string location_code,
                                       std::string register_num,
                                       std::string& trxnum,
                                       bool is_commit=true);
    bool                get_printcopies(std::string location_code);
    bool                get_registeralias (std::string location_code,
                                           std::string register_num,
                                           std::string& ipad_alias);
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
                                        std::string transaction_code="",
                                        bool ispos = true);
    //////////////////////////////////////
    //  Sales detail service(s)
    bool                new_detail (CYDbSql* db,
                                    std::string systransnum,
                                    std::string xvalue,
                                    std::string& item_seq,
                                    Json::Value jtax, Json::Value jproduct,
                                    Json::Value jdiscountitem, Json::Value jdetail,
                                    std::string table = "tg_pos_mobile_header");
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
    bool                hack_detail_recalc (std::string systransnum,
                                            std::string item_code="", std::string item_seq="");
    bool                hack_detail_discount (double dvat,
                                             double drate,
                                             double& totalPurchase,
                                             double& totalDiscount,
                                             std::string systransnum,
                                             std::string discount_code,
                                             std::string item_code,
                                             std::string item_seq);
    //////////////////////////////////////
    //  Product discount
    bool                product_fnbprice (std::string item_code,
                                          std::string table_code,
                                          std::string& retail_price);

    bool                product_fnbpricetable (std::string table_code);

    bool                product_discount (std::string item_code,
                                          std::string location_code,
                                          std::string retail_price,
                                          std::string transaction_code,
                                          Json::Value& jdiscountitem);
    bool                product_discount_event (std::string item_code,
                                                std::string location_code,
                                                std::string retail_price,
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
    bool                ipad_done (std::string systransnum,
                                   std::string cashier,
                                   std::string cashier_shift,
                                   std::string register_num,
                                   std::string location_code,
                                   std::string& newtransnum);

    bool                ipad_copy (CYDbSql* dbupd,
                                   std::string trxnum,
                                   std::string tableName,
                                   std::string registerNum,
                                   std::string oldTransNum,
                                   std::string sysTransNum);

    bool                xread_copy(CYDbSql* dbupd,
                                   std::string cashier,
                                   std::string registerNum,
                                   std::string logicalDate,
                                   std::string locationCode,
                                   std::string cashierShift,
                                   std::string registerAlias);

    bool                cyupdatedetail (CYDbSql* db,
                                        bool isQuantityOverride,
                                        STRUCT_DETAILUPDATE* structDetail,
                                        std::string table = "tg_pos_mobile_header");

    bool                update_sales_tables (CYDbSql* db,
                                             std::string oldtransnum,
                                             std::string newtransnum);
    bool                cleanup_sales_tables (CYDbSql* db,
                                              std::string systransnum);
    bool                doublecheck_sales_tables (CYDbSql* db,
                                                  std::string table,
                                                  std::string oldtransnum,
                                                  std::string newtransnum);
    //////////////////////////////////////
    //  Default(s)
    bool                default_cash_tender (Json::Value& jcash);
    //////////////////////////////////////
    //  Object access
    cyini               *_ini;
    cylog               *_log;
    cycgi               *_cgi;
    cyutility           *_util = nullptr;  // Useful function(s)

    std::string         _domainName;
    Json::Value         _jdevice, _jsales,
                        _jfnb, _jpayload;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    bool                _postVoid;
    bool                _memberPay;
    Json::Value         _jprtcopies;
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
                                        std::string recipe_item_code,
                                        std::string picture_url,
                                        std::string& retail_price);

public:
    //////////////////////////////////////
    //  Current print copy
    int                 printCopy = 1;
    //////////////////////////////////////
    //  Service(s)
    std::string         rest_xread();
    std::string         rest_zread();
    std::string         rest_zread_poll();
    std::string         rest_xread_save();
    std::string         rest_zread_save();
    std::string         rest_zread_check();
    std::string         rest_zread_force();
    std::string         rest_xread_cashdec();
    //////////////////////////////////////
    //  POLL file related
    bool                cy_daily_xread (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_detail (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_payment (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_account (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_currency (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_settings (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_trxdiscount (CYDbSql* db, Json::Value jheader);
    bool                cy_daily_header (CYDbSql* db, Json::Value jheader,
                                         int nvoid, int nrefund);

    double              _transdiscamt = 0.00;
    Json::Value         _jfsp = Json::nullValue;
    std::string         _sclerk = "", _pshopper = "",
                        _is_pwd="0", _is_senior="0", _is_ba="0";
    //////////////////////////////////////
    //  Alternative end of day
    bool                exportOfflineSales (std::string branchCode,
                                            std::string registerNumber,
                                            std::string logicalDate);
    bool                importOfflineSales (std::string branchCode,
                                            std::string registerNumber,
                                            std::string logicalDate);
    //////////////////////////////////////
    //  EOD receipt method(s)
    bool                assembleEodHeader ();
    bool                assembleEodPayment ();
    bool                assembleEodCash ();
    bool                assembleEodDiscount ();
    bool                assembleEodVat ();
    bool                assembleEodFsp ();
    bool                assembleEodBank ();
    bool                assembleEodCashfund ();
    bool                assembleEodCashDeclare ();
    bool                assembleEodCashier ();
    bool                assembleEodFooter ();
    bool                cleanupSuspended (std::string branchCode,
                                          std::string registerNumber,
                                          std::string logicalDate,
                                          std::string cashier);
    bool                cleanupMobileSales (std::string branchCode,
                                            std::string registerNumber,
                                            std::string logicalDate,
                                            std::string cashier);
    //////////////////////////////////////
    //  EOD Attribute(s)
    CYRestReceiptEod    *_eod;
    double              _drawer = 0.00;
    double              _discountTotal = 0.00;
    std::string         _receipt, _command = "";
    std::string         _txthdr, _txtpay, _txtcash, _txtdiscount,
                        _txtvat, _txtbank, _txtfund, _txtdeclare,
                        _txtcashier, _txtfooter;

    STRUCT_FSPURL       _fspurl;
    //////////////////////////////////////
    //  PRINT method(s) - FSP
    CYRestCommon*       _rcptType;

    std::string         rest_fspaddpoints ();
    std::string         httpget(std::string url);

    bool                assembleImage (std::string imageFile);
    bool                updateJournal (std::string type,
                                       std::string cytimestamp,
                                       std::string systransnum,
                                       std::string receiptdata);
    //////////////////////////////////////
    //  PRINT method(s) - RECEIPT
    std::string         rest_bridal ();
    std::string         rest_receipt ();
    std::string         rest_receipt_eod ();
    std::string         rest_receipt_raw ();
    std::string         rest_receipt_debug ();
    std::string         rest_receipt_refund ();
    std::string         rest_receipt_reprint();
    std::string         rest_receipt_cashfund();
    std::string         rest_receipt_validate();
    std::string         rest_receipt_kitchen ();
    std::string         rest_receipt_network ();
    std::string         rest_receipt_postvoid ();
    std::string         rest_receipt_formatted ();
    std::string         rest_validate (int position, std::string data);
    //////////////////////////////////////
    //  PRINTER Device specific
    std::string         rest_drawer ();
    std::string         rest_drawer_open ();
    std::string         deviceLocal (size_t length,
                                     std::string receipt,
                                     std::string type,
                                     std::string reference);
    std::string         deviceLocal (size_t length,
                                     unsigned char* buffer,
                                     std::string type,
                                     std::string reference);
    std::string         deviceNetwork (std::string buffer,
                                       std::string reference,
                                       std::string printurl="");
    bool                deviceOverride (std::string& printer_url);

    bool                deviceCut ();
    bool                deviceKick ();
    bool                deviceQrPrint (std::string qrcode);
    bool                deviceImage (std::string imageFile);
    bool                deviceImageCommand (std::string imageFile);
    bool                deviceSiemensImage (std::string qrdata);

    bool                deviceNetworkQr  (std::string buffer,
                                          std::string printurl="");
    bool                deviceNetworkRaw (std::string buffer,
                                          std::string printurl="");
    bool                deviceNetworkText (std::string data,
                                           std::string printurl);
    bool                deviceNetworkImage (std::string imageFile,
                                            std::string printurl="");
    bool                signatureCheck (std::string referenceNumber);
    //////////////////////////////////////
    //  REST SQL method(s)
    //////////////////////////////////////
    //  SQL request
    std::string         sql_update  ();
    std::string         sql_retrieve ();
    //////////////////////////////////////
    //  SQL Loyalty request
    bool                sql_prequel(std::string req);
    bool                sql_loyalty_item_populate ();
    //////////////////////////////////////
    //  SQL Product request
    bool                sql_product_save ();
    bool                sql_product_detail_check ();

    std::string         sql_class_image ();
    std::string         sql_product_image ();
    //////////////////////////////////////
    //  HTTP rest
    bool                resthttpget (std::string url, std::string& json);
    bool                resthttpparse (std::string json, std::string& errormsg);
    //////////////////////////////////////
    //  Error message access
    std::string         errormsg ();
    std::string         errorjson (std::string errormsg);
    bool                seterrormsg (std::string message);
    enumQueryType       seterrorquery (std::string message)  {
        seterrormsg(message);
        return enumQueryType::EQT_ERROR;
    }
    //////////////////////////////////////
    //  ERP sales
    cyerpsales*         _erpSales;
    //////////////////////////////////////
    //  Temporary variable(s)
    bool                salesDone;
    std::string         _extLoyalty;
    salesReceiptType    _receiptType;
    //////////////////////////////////////
    //  Sales totals
    STRUCT_FSPURL       _fspTrans;
    STRUCT_SALESTOTAL   _salesTotal;
};
#endif
