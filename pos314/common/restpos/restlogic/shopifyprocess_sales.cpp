/********************************************************************
          FILE:         shopifyprocess_sales
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
      FUNCTION:         processShopifySales
   DESCRIPTION:         Process shopify sales records
 ********************************************************************/
bool                    cysaleshost::processShopifySales (string systransnum,
                                                          salesReceiptType type)
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
    //  retrieve sales record(s)
    string sql = "select systransnum from tg_pos_daily_header where systransnum = ";
    sql += db->sql_bind(1, systransnum);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    //////////////////////////////////////
    //  create the shopify object
    cyshopify shop (_ini,_log,_cgi);
    if (!shop.retrieveAppDetails())
        return seterrormsg(shop.errormsg());
    //////////////////////////////////////
    //  create a receipt
    string custid;
    Json::Value jorder;
    Json::Value jheader;
    CYRestCommon *rcpt = new CYReceiptEast (_log,_ini,_cgi);
    if (!db->eof())  {
        do  {
            jheader.clear ();
            string rcptType = _ini->get_value("RECEIPT","TYPE");
            if (!rcpt->retrieveSalesHeader(jheader, systransnum, type))
                return seterrormsg(rcpt->errormsg());

            if (!rcpt->retrieveSalesHeaderAccount(jheader))
                return seterrormsg(rcpt->errormsg());

            Json::Value jdetail;
            jdetail.clear();
            if (!rcpt->retrieveSalesDetail(jdetail,jheader["branch_code"].asString(), systransnum))
                    return seterrormsg(rcpt->errormsg());

            jheader["det_sales"]=jdetail;
            Json::Value jpayment;
            jpayment.clear();
            if (!rcpt->retrieveSalesPayment(jpayment, systransnum))
                return seterrormsg(rcpt->errormsg());
            jheader["pay_sales"]=jpayment;
            rcpt->_jheader = jheader;

            if (!rcpt->assembleFspUrl(jheader, jdetail, jpayment))
                return seterrormsg(rcpt->errormsg());

            stringstream sss;
            sss << jheader;
            //_log->logdebug("JSON",sss.str().c_str());

            rcpt->_txttype = "print";
            rcpt->assembleReceipt(jheader, jdetail, jpayment,SRT_DAILY);

            char sztmp [64];
            jheader["cytotal_items"] = rcpt->_totalItems;

            sprintf(sztmp, "%.02f", (rcpt->_totalNetSales*10000) / 10000);
            jheader["cytotal_net_sales"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalVatAmount*10000) / 10000);
            jheader["cytotal_vat_amount"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalTrxDiscount*10000) / 10000);
            jheader["cytotal_trx_discount"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalGrossAmount*10000) / 10000);
            jheader["cytotal_gross_amount"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalItemDiscount*10000) / 10000);
            jheader["cytotal_item_discount"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalNetVat*10000) / 10000);
            jheader["cytotal_net_vat"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalNetZero*10000) / 10000);
            jheader["cytotal_net_zero"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalNetExempt*10000) / 10000);
            jheader["cytotal_net_exempt"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalAmtVat*10000) / 10000);
            jheader["cytotal_amt_vat"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalAmtZero*10000) / 10000);
            jheader["cytotal_amt_zero"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalAmtExempt*10000) / 10000);
            jheader["cytotal_amt_exempt"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalAddonAmount*10000) / 10000);
            jheader["cytotal_amt_addon"] = atof(sztmp);

            sprintf(sztmp, "%.02f", (rcpt->_totalServiceCharge*10000) / 10000);
            jheader["cytotal_service_charge"] = atof(sztmp);
            //////////////////////////////////////
            //  OVERRIDE THE GENERATED RECEIPT
            rcpt->_receipt  = rcpt->_txthdr;
            rcpt->_receipt += rcpt->_txtdet;
            rcpt->_receipt += rcpt->_txttot;
            rcpt->_receipt += rcpt->_txtpay;
            rcpt->_receipt += rcpt->_txtacct;
            rcpt->_receipt += rcpt->_txtvat;
            rcpt->_receipt += rcpt->_txtsig;
            rcpt->_receipt += rcpt->_txtfoot;
            rcpt->_receipt += rcpt->_txtgift;
            //////////////////////////////////////
            //  Include the separated receipt
            //  in the sales record
            jheader["rcpt_header"]=_util->base64encode(rcpt->_txthdr);

            if (rcpt->_txtdet.length() > 0)
                jheader["rcpt_detail"]=_util->base64encode(rcpt->_txtdet);
            else
                jheader["rcpt_detail"]="";

            if (rcpt->_txttot.length() > 0)
                jheader["rcpt_totals"]=_util->base64encode(rcpt->_txttot);
            else
                jheader["rcpt_totals"]="";

            if (rcpt->_txtpay.length() > 0)
                jheader["rcpt_pay"]=_util->base64encode(rcpt->_txtpay);
            else
                jheader["rcpt_pay"]="";

            if (rcpt->_txtvat.length() > 0)
                jheader["rcpt_vat"]=_util->base64encode(rcpt->_txtvat);
            else
                jheader["rcpt_vat"]="";

            if (rcpt->_txtfoot.length() > 0)
                jheader["rcpt_footer"]=_util->base64encode(rcpt->_txtfoot);
            else
                jheader["rcpt_footer"]="";

            if (rcpt->_txtgift.length() > 0)
                jheader["rcpt_giftwrap"]=_util->base64encode(rcpt->_txtgift);
            else
                jheader["rcpt_giftwrap"]="";
            //////////////////////////////////////
            //  Get primary key variable(s)
            string cashier = jheader["cashier"].asString();
            string shift = jheader["cashier_shift"].asString();
            string location_code = jheader["branch_code"].asString();
            string logical_date = jheader["logical_date"].asString();
            string register_num = jheader["register_number"].asString();
            //////////////////////////////////////
            //  Customer info
            string email;
            string varDesc;
            dbsel->sql_reset();
            sql  = "select * from tg_pos_mobile_trxaccount where systransnum = ";
            sql += dbsel->sql_bind(1,systransnum);
            if (!dbsel->sql_result(sql,true))
                return seterrormsg(dbsel->errordb());
            if (dbsel->eof())
                email = "";
            else  {
                custid = dbsel->sql_field_value("customer_id");
                dbsel->sql_reset();
                sql  = "select * from sc_customer_contact ";
                sql += "where  contact_cd = 'em' ";
                sql += "and    customer_id = ";
                sql += dbsel->sql_bind(1,custid);
                if (!dbsel->sql_result(sql,true))
                    return seterrormsg(dbsel->errordb());
                email = dbsel->sql_field_value("contact_value");
            }
            //////////////////////////////////////
            //  Send the VOID request
            string transInput, orderInput = graphorder;
            orderInput += ",\n\"variables\" : {\n";
            //orderInput += "  \"sendReceipt\": true,\n";
            //orderInput += "  \"sendFulfillmentReceipt\": true,\n";
            //orderInput += "  \"inventoryBehaviour\": \"DECREMENT_IGNORING_POLICY\",\n";
            orderInput += "  \"order\": {\n";
            orderInput += "    \"currency\": \"PHP\",\n";
            orderInput += "    \"fulfillmentStatus\": \"FULFILLED\"";
            if (!email.empty())  {
                orderInput += ",\n    \"email\": \""; orderInput += email; orderInput += "\",\n";
            }  else  {
                orderInput += ",\n";
            }
            orderInput += "    \"fulfillment\": {\n";
            orderInput += "      \"locationId\": \"gid://shopify/Location/"; orderInput += location_code; orderInput += "\",\n";
            orderInput += "      \"shipmentStatus\": \"DELIVERED\"\n";
            orderInput += "    },\n";
            orderInput += "    \"lineItems\": [\n";
            //  assemble item(s)
            string tmp, lineItems = "";
            int y = jdetail.size();
            double trxTotal = 0;
            for (int x = 0; x < y; x++)  {
                char sztmp[64], sznet[64], sztax[64];
                Json::Value line = jdetail[x];
                //  transaction flag
                string trx_flag = line["trx_flag"].asString();
                //  quantity
                int qty = atoi(line["quantity"].asString().c_str());
                sprintf (sztmp, "%d", qty);
                //////////////////////////////////////
                //  Variant description
                dbsel->sql_reset();
                sql  = "select * from cy_baseitem_hdr ";
                sql += "where  item_code = ";
                sql += dbsel->sql_bind(1,line["item_code"].asString());
                if (!dbsel->sql_result(sql,true))
                    return seterrormsg(dbsel->errordb());
                varDesc  = dbsel->sql_field_value("option1"); varDesc += " ";
                varDesc += dbsel->sql_field_value("option2"); varDesc += " ";
                varDesc += dbsel->sql_field_value("option3");

                if (trx_flag == "regular")  {
                    //  tax
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = _util->stodsafe(_util->cut_decimal(2,line["amount_tax"].asString().c_str()));

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

                    lineItems += "      {\n";
                    lineItems += "        \"title\": \""; lineItems += line["description"].asString(); lineItems += "\",\n";
                    lineItems += "        \"variantId\": \"gid://shopify/ProductVariant/"; lineItems += line["item_code"].asString(); lineItems += "\",\n";
                    lineItems += "        \"variantTitle\": \""; lineItems += varDesc; lineItems += "\",\n";
                    lineItems += "        \"productId\": \"gid://shopify/Product/"; lineItems += line["item_code"].asString(); lineItems += "\",\n";
                    //lineItems += "        \"sku\": \""; lineItems += line["trading_sku"].asString(); lineItems += "\",\n";
                    lineItems += "        \"priceSet\": {\n";
                    lineItems += "          \"shopMoney\": {\n";
                    lineItems += "            \"amount\": \""; lineItems += sznet; lineItems += "\",\n";
                    lineItems += "            \"currencyCode\": \"PHP\"\n";
                    lineItems += "          }\n";
                    lineItems += "        },\n";
                    lineItems += "        \"quantity\": "; lineItems += sztmp; lineItems += ",\n";
                    lineItems += "        \"taxLines\": [\n";
                    lineItems += "          {\n";
                    lineItems += "            \"priceSet\": {\n";
                    lineItems += "              \"shopMoney\": {\n";
                    lineItems += "                \"amount\": "; lineItems += sztax; lineItems += ",\n";
                    lineItems += "                \"currencyCode\": \"PHP\"\n";
                    lineItems += "              }\n";
                    lineItems += "            },\n";
                    lineItems += "            \"rate\": 0.12,\n"; //todo::::::lineItems += line["less_tax"].asString(); lineItems += ",\n";
                    lineItems += "            \"title\": \"VAT\"\n"; //todo::::::lineItems += line["less_tax"].asString(); lineItems += ",\n";
                    lineItems += "          }\n";
                    lineItems += "        ]\n";
                    lineItems += "      }";
                }  else  {
                    //  tax
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = rcpt->_totalNetSales;

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

                    lineItems += "      {\n";
                    lineItems += "        \"title\": \""; lineItems += line["description"].asString(); lineItems += "\",\n";
                    lineItems += "        \"productId\": \"gid://shopify/Product/"; lineItems += line["item_code"].asString(); lineItems += "\",\n";
                    lineItems += "        \"sku\": \""; lineItems += line["trading_sku"].asString(); lineItems += "\",\n";
                    lineItems += "        \"priceSet\": {\n";
                    lineItems += "          \"shopMoney\": {\n";
                    lineItems += "            \"amount\": \""; lineItems += sznet; lineItems += "\",\n";
                    lineItems += "            \"currencyCode\": \"PHP\"\n";
                    lineItems += "          }\n";
                    lineItems += "        },\n";
                    lineItems += "        \"quantity\": "; lineItems += sztmp; lineItems += ",\n";
                    lineItems += "        \"taxLines\": [\n";
                    lineItems += "          {\n";
                    lineItems += "            \"priceSet\": {\n";
                    lineItems += "              \"shopMoney\": {\n";
                    lineItems += "                \"amount\": 0,\n";
                    lineItems += "                \"currencyCode\": \"PHP\"\n";
                    lineItems += "              }\n";
                    lineItems += "            },\n";
                    lineItems += "            \"rate\": 0.12,\n"; //todo::::::lineItems += line["less_tax"].asString(); lineItems += ",\n";
                    lineItems += "            \"title\": \"VATEX\"\n"; //todo::::::lineItems += line["less_tax"].asString(); lineItems += ",\n";
                    lineItems += "          }\n";
                    lineItems += "        ]\n";
                    lineItems += "      }";
                }
                if ((x+1) < y)
                    lineItems += ",";
                lineItems += "\n";
            }
            orderInput += lineItems;
            orderInput += "    ],\n";

            orderInput += "    \"transactions\": [\n";
            //  assemble item(s)
            lineItems = "";
            y = jdetail.size();
            double totalVoidAmount = 0;
            for (int x = 0; x < y; x++)  {
                Json::Value line = jdetail[x];
                char sztmp[64], sztax[64], sznet[64];
                //  transaction flag
                string trx_flag = line["trx_flag"].asString();
                //  quantity
                int qty = atoi(line["quantity"].asString().c_str());
                sprintf (sztmp, "%d", qty);
                if (trx_flag == "regular")  {
                    //  tax
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = _util->stodsafe(_util->cut_decimal(2,line["amount_tax"].asString().c_str()));

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
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = rcpt->_totalNetSales;

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
                // paid
                totalVoidAmount += trxTotal;
                /*
                double dpaid = trxTotal;
                sprintf (szpaid,"%.02f", dpaid);
                lineItems += "      {\n";
                lineItems += "        \"kind\": \"SALE\",\n";
                lineItems += "        \"status\": \"SUCCESS\",\n";
                lineItems += "        \"locationId\": \"gid://shopify/Location/"; lineItems += location_code; lineItems += "\",\n";
                lineItems += "        \"amountSet\": {\n";
                lineItems += "          \"shopMoney\": {\n";
                lineItems += "            \"amount\": \""; lineItems += szpaid; lineItems += "\",\n";
                lineItems += "            \"currencyCode\": \"PHP\"\n";
                lineItems += "          }\n";
                lineItems += "        }\n";
                lineItems += "      }";

                if ((x+1) < y)
                    lineItems += ",";
                lineItems += "\n";

                transInput += "      {\n";
                transInput += "        \"kind\": \"SALE\",\n";
                transInput += "        \"status\": \"SUCCESS\",\n";
                transInput += "        \"amount\": \""; transInput += sznet; transInput += "\",\n";
                transInput += "        \"quantity\": "; transInput += sztmp; transInput += ",\n";
                transInput += "        \"variantId\": \"gid://shopify/ProductVariant/"; transInput += line["item_code"].asString(); transInput += "\",\n";
                transInput += "        \"locationId\": \"gid://shopify/Location/"; transInput += location_code; transInput += "\",\n";
                transInput += "        \"amountSet\": {\n";
                transInput += "          \"shopMoney\": {\n";
                transInput += "            \"amount\": \""; transInput += szpaid; transInput += "\",\n";
                transInput += "            \"currencyCode\": \"PHP\"\n";
                transInput += "          }\n";
                transInput += "        }\n";
                transInput += "      }";
            //}
                if ((x+1) < y)
                    transInput += ",";
                transInput += "\n";*/
            }
            char szpaid[64];
            double dpaid = totalVoidAmount;
            sprintf (szpaid,"%.02f", dpaid);
            lineItems += "      {\n";
            lineItems += "        \"kind\": \"SALE\",\n";
            lineItems += "        \"status\": \"SUCCESS\",\n";
            lineItems += "        \"locationId\": \"gid://shopify/Location/"; lineItems += location_code; lineItems += "\",\n";
            lineItems += "        \"amountSet\": {\n";
            lineItems += "          \"shopMoney\": {\n";
            lineItems += "            \"amount\": \""; lineItems += szpaid; lineItems += "\",\n";
            lineItems += "            \"currencyCode\": \"PHP\"\n";
            lineItems += "          }\n";
            lineItems += "        }\n";
            lineItems += "      }\n";
            orderInput += lineItems;
            orderInput += "    ]\n";
            orderInput += "  }\n";
            orderInput += "}\n}\n";

            lineItems  = "[\n";
            lineItems += transInput;
            lineItems += "]\n";
            transInput = lineItems;
            _log->logmsg("GRAPH",orderInput.c_str ());
            ////////////////////////////////////////
            //  Send to shopify
            bool ret = shop.restpost(shop.graphUrl(),orderInput,jorder);
            if (!ret)  {
                return seterrormsg(shop.errormsg());
            }
            ////////////////////////////////////////
            //  Verify the status
            string orderId = "";
            if (jorder.isMember("data"))  {
                Json::Value jtmp1 = jorder["data"];
                if (jtmp1.isMember("orderCreate"))  {
                    Json::Value jtmp2 = jtmp1["orderCreate"];
                    if (jtmp2.isMember("order"))  {
                        Json::Value jtmp3 = jtmp2["order"];
                        if (jtmp3.isMember("id"))  {
                             orderId = jtmp3["id"].asString();
                        }
                    }
                }
            }
            //////////////////////////////
            //  Save the order detail(s)
            stringstream jss;
            jss << jorder;
            string tmpss = jss.str();
            _log->logmsg("RESULT................................",tmpss.c_str());

            dbupd->sql_reset();
            char szvoidamt[64];
            sprintf (szvoidamt,"%.02f",totalVoidAmount);
            sql  = "insert into tg_pos_shopify_header (systransnum, company_code, ";
            sql += "branch_code, register_number, transaction_number, transaction_date, ";
            sql += "transaction_time, logical_date, cashier, transaction_code, cashier_shift, ";
            sql += "order_gid, order_response, inv_response, trans_json, total_void_amount) values (";
            sql += dbupd->sql_bind (1,systransnum); sql += ", ";
            sql += dbupd->sql_bind (2,jheader["company_code"].asString()); sql += ", ";
            sql += dbupd->sql_bind (3,jheader["branch_code"].asString()); sql += ", ";
            sql += dbupd->sql_bind (4,jheader["register_number"].asString()); sql += ", ";
            sql += dbupd->sql_bind (5,jheader["transaction_number"].asString()); sql += ", ";
            sql += dbupd->sql_bind (6,jheader["transaction_date"].asString()); sql += ", ";
            sql += dbupd->sql_bind (7,jheader["transaction_time"].asString()); sql += ", ";
            sql += dbupd->sql_bind (8,jheader["logical_date"].asString()); sql += ", ";
            sql += dbupd->sql_bind (9,jheader["cashier"].asString()); sql += ", ";
            sql += dbupd->sql_bind (10,jheader["transaction_code"].asString()); sql += ", ";
            sql += dbupd->sql_bind (11,jheader["cashier_shift"].asString()); sql += ", ";
            sql += dbupd->sql_bind (12,orderId); sql += ", ";
            sql += dbupd->sql_bind (13,tmpss); sql += ", '', ";
            sql += dbupd->sql_bind (14,transInput); sql += ", ";
            sql += dbupd->sql_bind (15,szvoidamt); sql += ") ";

            if (!dbupd->sql_only(sql,true))
                return seterrormsg(dbupd->errordb());
            //////////////////////////////////////
            //  Save order details
            string invUpdate = graphinv;
            invUpdate += ",\n\"variables\" : {\n";
            //  assemble inventory update
            y = jdetail.size();
            for (int x = 0; x < y; x++)  {
                char sztmp[64], sznet[64], sztax[64];
                Json::Value line = jdetail[x];
                //  transaction flag
                string trx_flag = line["trx_flag"].asString();
                int qty = atoi(line["quantity"].asString().c_str());
                sprintf (sztmp, "%d", qty);
                if (trx_flag == "regular")  {
                    //  tax
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = _util->stodsafe(_util->cut_decimal(2,line["amount_tax"].asString().c_str()));

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
                    double taxAmount = _util->stodsafe(_util->cut_decimal(2,line["less_tax"].asString().c_str()));
                    double itemAmount = rcpt->_totalNetSales;

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
                //////////////////////////////////////
                //  Variant description
                dbsel->sql_reset();
                sql  = "select * from cy_baseitem_hdr ";
                sql += "where  item_code = ";
                sql += dbsel->sql_bind(1,line["item_code"].asString());
                if (!dbsel->sql_result(sql,true))
                    return seterrormsg(dbsel->errordb());
                string invItemCode = dbsel->sql_field_value("inventory_item_code");


                invUpdate += "     \"input\" : {\n";
                invUpdate += "       \"reason\": \"other\",\n";
                invUpdate += "       \"name\": \"available\",\n";
                invUpdate += "       \"changes\": [\n";
                invUpdate += "         {\n";
                invUpdate += "           \"delta\": -"; invUpdate += sztmp; invUpdate += ",\n";
                invUpdate += "           \"inventoryItemId\": \"gid://shopify/InventoryItem/"; invUpdate += invItemCode; invUpdate += "\",\n";
                invUpdate += "           \"locationId\": \"gid://shopify/Location/"; invUpdate += location_code; invUpdate += "\"\n";
                invUpdate += "         }\n";
                invUpdate += "       ]\n";
                invUpdate += "     }";
                //invUpdate += "   }\n";
                if ((x+1) < y)
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
            stringstream iss;
            iss << jorder;
            tmpss = iss.str();
            _log->logmsg("RESULT.......................",tmpss.c_str());

            dbupd->sql_bind_reset();
            sql  = "update tg_pos_shopify_header set inv_response = ";
            sql += dbupd->sql_bind (1,tmpss); sql += " where systransnum = ";
            sql += dbupd->sql_bind (2,systransnum); sql += " ";

            if (!dbupd->sql_only(sql,true))
                return seterrormsg(dbupd->errordb());

            if (!dbupd->sql_commit())
                return seterrormsg(dbupd->errordb());

            db->sql_next();
        }  while (!db->eof());
    }
    return true;
}
