/********************************************************************
          FILE:         processbom
   DESCRIPTION:         Process the BOM table(s)
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
      FUNCTION:         processShopifyRefund
   DESCRIPTION:         Process refunded records
 ********************************************************************/
bool                    cysaleshost::processShopifyRefund (string systransnum,
                                                           string location_code,
                                                           std::string oldsystransnum,
                                                           std::string old_location_code)
{
    (void)old_location_code;
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
    Json::Value jorder;
    GRAPH_POSTVOID refund [256];
    GRAPH_POSTVOID original[256];
    //////////////////////////////////////
    //  Retrieve the refunded item(s)
    int idx = 0;
    int ridx = 0;
    int count = 0;

    dbsel->sql_reset();
    double trxTotal = 0;
    string item_code = "";
    string sql  = "select * from tg_pos_mobile_detail where systransnum = ";
    sql += dbsel->sql_bind(1,oldsystransnum);
    if (!dbsel->sql_result(sql,true))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof())  {
        for (int i = 0; i < 256; i++)  {
            refund[i].sel = false;
            refund[i].lineItem = "";
            refund[i].quantity = 0;
            refund[i].amount = 0.00;
            refund[i].variantId = "";
        }
        do  {
            count++;
            char sztax[64], sznet[64];
            string trx_flag = dbsel->sql_field_value("trx_flag");
            string less_tax = dbsel->sql_field_value("less_tax");
            string amount_tax = dbsel->sql_field_value("amount_tax");

            refund[ridx].subtotal = trxTotal;
            item_code = dbsel->sql_field_value("item_code");
            refund[ridx].variantId = dbsel->sql_field_value("item_code");
            string item_seq = dbsel->sql_field_value("item_seq");
            db->sql_reset();
            sql  = "select * from tg_pos_mobile_detail where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            sql += " and item_code = ";
            sql += db->sql_bind(2,refund[ridx].variantId);
            if (!db->sql_result(sql,true))
                return seterrormsg(db->errordb());
            if (!db->eof())  {
                refund[ridx].quantity = _util->stodsafe(db->sql_field_value("quantity"));
            }  else  {
                refund[ridx].quantity = 0;
            }
            int qty = refund[ridx].quantity;
            if (trx_flag == "regular")  {
                //  tax
                amount_tax = dbsel->sql_field_value("amount_tax");
                double taxAmount = _util->stodsafe(_util->cut_decimal(2,less_tax.c_str()));
                double itemAmount = _util->stodsafe(_util->cut_decimal(2,amount_tax.c_str()));
                double t1 = taxAmount / qty;
                sprintf (sztax, "%.02f", t1);
                double d1 = atof(_util->cut_decimal(2, sztax).c_str());
                sprintf (sztax, "%.02f", (d1*qty));
                //  retail less tax
                double i1 = itemAmount / qty;
                sprintf (sznet,"%.02f", i1);
                double d2 = atof(_util->cut_decimal(2, sznet).c_str());
                sprintf (sznet, "%.02f", d2);
                trxTotal = (d1*qty)+(d2*qty);
            }  else  {
                //  tax
                amount_tax = dbsel->sql_field_value("less_discount");
                double taxAmount = _util->stodsafe(_util->cut_decimal(2,less_tax.c_str()));
                double itemAmount = _util->stodsafe(_util->cut_decimal(2,amount_tax.c_str()));

                double t1 = taxAmount / qty;
                sprintf (sztax, "%.02f", t1);
                double d1 = atof(_util->cut_decimal(2, sztax).c_str());
                sprintf (sztax, "%.02f", (d1*qty));
                //  retail less tax
                double i1 = itemAmount / qty;
                sprintf (sznet,"%.02f", i1);
                double d2 = atof(_util->cut_decimal(2, sznet).c_str());
                sprintf (sznet, "%.02f", d2);
                trxTotal = (d1*qty)+(d2*qty);
                trxTotal = (d2*qty);
            }
            ridx++;
            dbsel->sql_next();
        }  while (!dbsel->eof());
    }
    //////////////////////////////////////
    //  Get the order detail(s)
    string transjson, orderjson;
    dbsel->sql_reset();
    string orderInput = " { \"query\" : \""; orderInput += graphvoid;
    sql  = "select trans_json, order_response, total_void_amount from tg_pos_shopify_header where systransnum = ";
    sql += dbsel->sql_bind(1,oldsystransnum);
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
        bool found = false;
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
            original[i].sel = false;
            original[i].lineItem = "";
            original[i].quantity = 0;
            original[i].amount = 0.00;
            original[i].variantId = "";
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
                                original[idx].sel = false;
                                original[idx].lineItem = jline["id"].asString();
                                original[idx].quantity = jline["quantity"].asInt();
                                if (jline.isMember("variant"))  {
                                    Json::Value n06;
                                    n06 = jline["variant"];
                                    if (n06.isMember("id"))  {
                                        //  Some times shopify doesn't return
                                        //  the variant id, so if only one
                                        //  record exists, use that item_code
                                        string tmpVar = n06["id"].asString();
                                        found = true;
                                        if (tmpVar.empty())  {
                                            tmpVar  = "gid://shopify/ProductVariant/";
                                            if (count == 1)
                                                tmpVar += item_code;
                                            else
                                                tmpVar = "00000";
                                        }
                                        original[idx].variantId = n06["id"].asString();
                                    }
                                }  else  {
                                    //  Some times shopify doesn't return
                                    //  the variant id, so if only one
                                    //  record exists, use that item_code
                                    if (count == 1)  {
                                        original[idx].variantId  = "gid://shopify/ProductVariant/";
                                        original[idx].variantId += item_code;
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
        //////////////////////////////////////
        //  Save order details???
        double grandTotal = 0;
        char szqty[64], szamt[64];
        orderInput += "\",\n\"variables\" : {\n";
        orderInput += "  \"input\": {\n";
        orderInput += "    \"orderId\": \""; orderInput += orderId; orderInput += "\",\n";
        orderInput += "    \"note\": \"REFUND (RMA) - point of sale\",\n";
        orderInput += "    \"refundLineItems\": [\n";
        for (int i = 0; i < ridx; i++)  {
            //  quantity
            int z = refund[i].quantity;
            if (z < 0) z = z * -1;
            // item code
            for (int x = 0; x < idx; x++)  {
                string vid, rid = refund[i].variantId;
                if (!original[x].variantId.empty())
                    vid = original[x].variantId.substr(strlen("gid://shopify/ProductVariant/"));
                else  {
                    if (vid.empty())  {
                        //  Some times shopify doesn't return
                        //  the variant id, so if only one
                        //  record exists, use that item_code
                        if (count == 1)  {
                            original[x].variantId  = "gid://shopify/ProductVariant/";
                            original[x].variantId += item_code;
                            vid = original[x].variantId.substr(strlen("gid://shopify/ProductVariant/"));
                        }  else  {
                            vid = "gid://shopify/ProductVariant/00000";
                        }
                    }
                }
                if (vid == rid)  {
                    refund[i].lineItem = original[x].lineItem;
                }
            }
            sprintf (szqty,"%d",z);
            orderInput += "      {\n";
            orderInput += "        \"lineItemId\":\"";  orderInput += refund[i].lineItem; orderInput += "\",\n";
            orderInput += "        \"quantity\": ";  orderInput += szqty; orderInput += "\n";
            orderInput += "      }\n";
            if ((i+1) < idx)
                orderInput += ",";
            orderInput += "\n";
            grandTotal += refund[i].subtotal;
        }
        orderInput += "    ],\n";
        orderInput += "    \"transactions\": [\n";

        sprintf (szamt,"%.02f",grandTotal);
        orderInput += "      {\n";
        orderInput += "        \"orderId\":\"";  orderInput += orderId; orderInput += "\",\n";
        orderInput += "        \"gateway\":\"cash\",\n";
        orderInput += "        \"kind\":\"REFUND\",\n";
        orderInput += "        \"amount\": 0.00\n";
        orderInput += "      }";
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
    for (int x = 0; x < ridx; x++)  {
        char sztmp[64];
        int z = refund[x].quantity;
        if (z < 0) z = z * -1;
        sprintf (sztmp, "%d", z);
        //////////////////////////////////////
        //  Variant description
        dbsel->sql_reset();
        string vid = refund[x].variantId;
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
