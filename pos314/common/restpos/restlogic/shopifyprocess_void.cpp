/********************************************************************
          FILE:         shopifyprocess_void
   DESCRIPTION:         Process sales record(s)
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <cmath>
#include <string>
#include <sstream>
#include "cyw_aes.h"
#include "cysaleshost.h"
#include "cyrestreceipt.h"
#include "cyshopify.h"
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         processShopifyVoid
   DESCRIPTION:         Process voided records
 ********************************************************************/
bool                    cysaleshost::processShopifyVoid (string systransnum,
                                                         string location_code)
{
    //////////////////////////////////////
    //  database connection
    CYDbSql* db = dbconn();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbsel = dbconn();
    if (nullptr == dbsel)
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbupd = dbconn();
    if (nullptr == dbupd)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  create the shopify object
    cyshopify shop (_ini,_log,_cgi);
    if (!shop.retrieveAppDetails())
        return seterrormsg(shop.errormsg());
    //////////////////////////////////////
    //  create a receipt
    int idx = 0;
    Json::Value jorder;
    GRAPH_POSTVOID pv[256];
    //////////////////////////////////////
    //  Get the order detail(s)
    string transjson, orderjson;
    dbsel->sql_reset();
    string sql, orderInput = " { \"query\" : \""; orderInput += graphvoid;
    sql  = "select trans_json, order_response, total_void_amount from tg_pos_shopify_header where systransnum = ";
    sql += dbsel->sql_bind(1,systransnum);
    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof())  {
        transjson = dbsel->sql_field_value("trans_json");
        orderjson = dbsel->sql_field_value("order_response");
        string totalvoid = dbsel->sql_field_value("total_void_amount");
        //////////////////////////////
        //  Parse the order response
        Json::Value jorder;
        JSONCPP_STRING err;
        Json::CharReaderBuilder builder;
        const std::string rawJson = orderjson;
        const auto rawJsonLength = static_cast<int>(rawJson.length());
        const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

        jorder.clear();
        if (!reader->parse(rawJson.c_str(),
                           rawJson.c_str() + rawJsonLength,
                           &jorder, &err)) {
            _error = "Unable to parse the JSON reply";
            return false;
        }
        _log->logmsg("JSONVOIDORDER",orderjson.c_str());
        //////////////////////////////
        //  JSON check
        string orderId;
        for (int i = 0; i < 256; i++)  {
            pv->lineItem = "";
            pv->quantity = -1;
        }
        if (jorder.isMember("data"))  {
            Json::Value n01;
            n01 = jorder["data"];
            if (n01.isMember("orderCreate"))  {
                Json::Value n02;
                n02 = n01["orderCreate"];
                if (n02.isMember("order"))  {
                    Json::Value n03, n04;
                    n03 = n02["order"];
                    orderId = n03["id"].asString();
                    n04 = n03["lineItems"];
                    if (n04.isMember("nodes"))  {
                        Json::Value n05;
                        n05 = n04["nodes"];
                        int y = n05.size ();
                        Json::Value jline;
                        for (int x = 0; x < y; x++)  {
                            jline.clear ();
                            jline = n05[x];
                            if (jline.isMember("id") && jline.isMember("quantity"))  {
                                pv[idx].lineItem = jline["id"].asString();
                                pv[idx].quantity = jline["quantity"].asInt();
                                if (jline.isMember("variant"))  {
                                    Json::Value n06;
                                    n06 = jline["variant"];
                                    if (n06.isMember("id"))  {
                                        pv[idx].variantId = n06["id"].asString();
                                    }
                                }
                                idx++;
                            }
                        }
                    }
                }
            }
        }
        //////////////////////////////
        //  Parse the order response
        Json::Value jtrans;
        const std::string trxJson = transjson;
        const auto trxJsonLength = static_cast<int>(trxJson.length());
        const std::unique_ptr<Json::CharReader> trxreader(builder.newCharReader());

        jtrans.clear();
        if (!trxreader->parse(trxJson.c_str(),
                              trxJson.c_str() + trxJsonLength,
                              &jtrans, &err)) {
            _error = "Unable to parse the JSON reply";
            return false;
        }
        string variantId;
        _log->logmsg("JSONVOID",transjson.c_str());
        int j = jtrans.size();
        for (int i = 0; i < j; i++)  {
            Json::Value jtrx;
            jtrx = jtrans[i];
            variantId = jtrx["variantId"].asString ();
            for (int x = 0; x < idx; x++)  {
                if (pv[x].variantId == variantId)  {
                    pv[x].amount = 0.00;

                    //if (jtrx.isMember("amount"))  {
                        //pv[x].amount = _util->stodsafe(jtrx["amount"].asString());
                    //}
                    if (jtrx.isMember("amountSet"))  {
                        Json::Value v1 = jtrx["amountSet"];
                        if (v1.isMember("shopMoney"))  {
                            Json::Value v2 = v1["shopMoney"];
                            if (v2.isMember("amount"))  {
                                pv[x].amount = _util->stodsafe(v2["amount"].asString());
                            }
                        }

                    }
                }
            }
        }
        //////////////////////////////////////
        //  Save order details???
        char szqty[64];
        orderInput += "\",\n\"variables\" : {\n";
        orderInput += "  \"input\": {\n";
        orderInput += "    \"orderId\": \""; orderInput += orderId; orderInput += "\",\n";
        orderInput += "    \"note\": \"POSTVOID - point of sale\",\n";
        orderInput += "    \"refundLineItems\": [\n";
        for (int i = 0; i < idx; i++)  {
            sprintf (szqty,"%d",pv[i].quantity);
            orderInput += "      {\n";
            orderInput += "        \"lineItemId\":\"";  orderInput += pv[i].lineItem; orderInput += "\",\n";
            orderInput += "        \"quantity\": ";  orderInput += szqty; orderInput += "\n";
            orderInput += "      }\n";
            if ((i+1) < idx)
                orderInput += ",";
            orderInput += "\n";
        }
        orderInput += "    ],\n";
        orderInput += "    \"transactions\": [\n";

        orderInput += "      {\n";
        orderInput += "        \"orderId\":\"";  orderInput += orderId; orderInput += "\",\n";
        //orderInput += "        \"status\":\"success\",\n";
        orderInput += "        \"gateway\":\"cash\",\n";
        orderInput += "        \"kind\":\"REFUND\",\n";
        orderInput += "        \"amount\":\"";  orderInput += totalvoid; orderInput += "\"\n";
        orderInput += "      }";
/***
        for (int i = 0; i < idx; i++)  {
            char szamt [64];
            sprintf (szamt, "%.02f", pv[i].amount);
            orderInput += "      {\n";
            orderInput += "        \"orderId\":\"";  orderInput += orderId; orderInput += "\",\n";
            //orderInput += "        \"status\":\"success\",\n";
            orderInput += "        \"gateway\":\"cash\",\n";
            orderInput += "        \"kind\":\"REFUND\",\n";
            orderInput += "        \"amount\":\"";  orderInput += szamt; orderInput += "\"\n";
            orderInput += "      }";
            if ((i+1) < idx)
                orderInput += ",";
            orderInput += "\n";
        }
***/
        orderInput += "    ]\n";
        orderInput += "  }\n";
        orderInput += "}\n";
    }  else  {
        return true;
    }
    orderInput += "}";/*"status": "success",
        "message": "Bogus Gateway: Forced success",*/
    ////////////////////////////////////////
    //  Dump the request
    _log->logmsg("REQUEST",orderInput.c_str());
    ////////////////////////////////////////
    //  Send to shopify
    bool ret = shop.restpost(shop.graphUrl(),orderInput,jorder);
    if (!ret)  {
        return seterrormsg(shop.errormsg());
    }
    string tmp;
    stringstream ss;
    ss << jorder;
    tmp = ss.str();
    _log->logmsg("RESULT................................",tmp.c_str());





    //////////////////////////////////////
    //  Save order details
    string invUpdate = graphinv;
    invUpdate += ",\n\"variables\" : {\n";
    //  assemble inventory update
    for (int x = 0; x < idx; x++)  {
        char sztmp[64];
        sprintf (sztmp, "%d", pv[x].quantity);
        //////////////////////////////////////
        //  Variant description
        dbsel->sql_reset();
        string vid = pv[x].variantId.substr(strlen("gid://shopify/ProductVariant/"));
        //"gid://shopify/ProductVariant/36226558754976"
        sql  = "select * from cy_baseitem_hdr ";
        sql += "where  item_code = ";
        sql += dbsel->sql_bind(1,vid);
        if (!dbsel->sql_result(sql,true))
            return seterrormsg(dbsel->errordb());
        string invItemCode = dbsel->sql_field_value("inventory_item_code");


        invUpdate += "     \"input\" : {\n";
        invUpdate += "       \"reason\": \"other\",\n";
        invUpdate += "       \"name\": \"available\",\n";
        invUpdate += "       \"changes\": [\n";
        invUpdate += "         {\n";
        invUpdate += "           \"delta\": "; invUpdate += sztmp; invUpdate += ",\n";
        invUpdate += "           \"inventoryItemId\": \"gid://shopify/InventoryItem/"; invUpdate += invItemCode; invUpdate += "\",\n";
        invUpdate += "           \"locationId\": \"gid://shopify/Location/"; invUpdate += location_code; invUpdate += "\"\n";
        invUpdate += "         }\n";
        invUpdate += "       ]\n";
        invUpdate += "     }";
        //invUpdate += "   }\n";
        if ((x+1) < idx)
            invUpdate += ",";
        invUpdate += "\n";
    }
    invUpdate += "  }\n";
    invUpdate += "}\n";
    _log->logmsg("GRAPH",invUpdate.c_str ());
    //  Send to shopify
    ret = shop.restpost(shop.graphUrl(),invUpdate,jorder);
    if (!ret)  {
        return seterrormsg(shop.errormsg());
    }
    return true;
}
