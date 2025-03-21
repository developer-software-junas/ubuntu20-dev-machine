/********************************************************************
          FILE:         rest.h
   DESCRIPTION:         REST data for flutter
 ********************************************************************/
#ifndef __cysaleshost_H
#define __cysaleshost_H
//////////////////////////////////////////
//  CYWare class(es)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cydb/cyw_db.h"
#include "cyrestreceipt.h"
//////////////////////////////////////////
//  Database object(s)
#include "cydb/cydbsql.h"
//////////////////////////////////////////
//  Classes used
class                   CGIHandler;
//////////////////////////////////////////
//  Structs
typedef struct  {
    bool sel;
    int quantity;
    double amount;
    double subtotal;
    std::string lineItem;
    std::string variantId;
} GRAPH_POSTVOID;
//////////////////////////////////////////
//  Order graphql mutation
const std::string graphvoid = " mutation M($input: RefundInput!) { refundCreate(input: $input) { userErrors { field message } refund { id note totalRefundedSet { presentmentMoney { amount } } } } }";
const std::string graphinv = "{ \"query\": \"mutation inventoryAdjustQuantities($input: InventoryAdjustQuantitiesInput!) {        inventoryAdjustQuantities(input: $input) {          userErrors {            field            message          }          inventoryAdjustmentGroup {            createdAt            reason            referenceDocumentUri            changes {              name              delta              location {                id              }              item {                id              }            }            id          }        }      }\"";
const std::string graphorder = "{ \"query\": \"mutation OrderCreate($order: OrderCreateOrderInput!, $options: OrderCreateOptionsInput) {  orderCreate(order: $order, options: $options) {    userErrors {      field      message    }    order {      id      totalTaxSet {        shopMoney {          amount          currencyCode        }      }      lineItems(first: 250) {        nodes {          variant {            id          }          id          title          quantity          taxLines {            title            rate            priceSet {              shopMoney {                amount                currencyCode              }            }          }        }      }    }  }}\"";
/********************************************************************
      FUNCTION:         cysaleshost
   DESCRIPTION:         POS REST data class
 ********************************************************************/
class                   cysaleshost
{
public:
    //////////////////////////////////////
    //  Constructor - LOCAL POS or SERVER
    cysaleshost (cyini* ini, cylog* log, cycgi* cgi);
    virtual ~cysaleshost ();
    //////////////////////////////////////
    //  Method(s)
    CYDbSql*            dbconn ();
    CYDbSql*            schemaconn ();
    CYDbSql*            remoteconn ();
    std::string         cgiform(std::string name);
    //////////////////////////////////////
    //  Check unprocessed sales record(s)
    bool                checkSales ();
    bool                checkShopifySales ();
    bool                remoteResume (std::string systransnum,
                                      CYDbSql* dbLocal, CYDbSql* dbRemote);

    bool                remoteSuspend (bool suspendIt, std::string systransnum);
    bool                remoteSuspendCleanup (std::string register_num,
                                              std::string location_code);
    //////////////////////////////////////
    //  REST key validation
    bool                validatekey (std::string hmac,
                                     std::string request,
                                     std::string session,
                                     std::string timestamp);
    //////////////////////////////////////
    //  Utility function(s)
    std::string         jsonDefaultNumber (std::string val, std::string defValue);
    //////////////////////////////////////
    //  Object access
    cyini               *_ini;
    cylog               *_log;
    cycgi               *_cgi;
    cyutility           *_util = nullptr;  // Useful function(s)

    CYDbSql             *_dbSchema;
    CYDbSql             *_dbRemote;

    Json::Value         _jhdr,
                        _jfsp,
                        _jbeauty;
    std::string         _clerk,
                        _pshopper;
    salesReceiptType    _salesType;
    double              _trxDiscount;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    CGIHandler*         _cgiHandler;
    std::string         _error = "",       // Error string
                        _request = "";     // REST request
    //////////////////////////////////////
    //  Process the sales record
    bool                processSales (std::string systransnum,
                                      salesReceiptType type);

    bool                processShopifySales (std::string systransnum,
                                             salesReceiptType type);
    bool                processShopifyVoid (std::string systransnum,
                                            std::string location_code);
    bool                processShopifyRefund (std::string systransnum,
                                              std::string location_code,
                                              std::string oldsystransnum,
                                              std::string old_location_code);
    //////////////////////////////////////
    //  Process the receipt
    bool                processReceipt (std::string type,
                                        std::string systransnum);
    bool                processReceiptJournal (std::string type,
                                               std::string systransnum);
    //////////////////////////////////////
    //  Import routine(s)
    std::string         tableFilter ();
    std::string         tableFilterCancel ();
    std::string         tableFilterRefund ();
    std::string         tableFilterHeader ();
    std::string         tableFilterSuspend();
    std::string         tableFilterVoid (bool noSalesRecord);

    bool                tableMigrate (std::string tableName,
                                      std::string systransnum,
                                      CYDbSql* dbSource,
                                      CYDbSql* dbDest,
                                      bool withIndex = true);
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
public:
    CYDbEnv* _env;
    CYDbEnv* _schema;
    CYDbEnv* _remote;
    //////////////////////////////////////
    //  Error message access
    std::string         errormsg ();
    std::string         errorjson (std::string errormsg);
    bool                seterrormsg (std::string message);
    enumQueryType       seterrorquery (std::string message)  {
        seterrormsg(message);
        return enumQueryType::EQT_ERROR;
    }
};
#endif
