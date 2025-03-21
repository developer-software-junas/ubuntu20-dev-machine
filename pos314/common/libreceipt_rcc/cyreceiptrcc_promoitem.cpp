//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptPromoItem
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::assembleReceiptPromoItem (Json::Value& jheader,
                                                            Json::Value& jdetail)
{
    _txtpromoitem = "";
    //////////////////////////////////////
    //  Database connection
    string sql="";
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());

    CYDbSql* dbsel = _ini->dbconn ();
    if (!dbsel->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Retrieve all active promotions
    dbsel->sql_reset();
    salesQueryType sqt;
    Json::Value jhdrtrx, jgiftpromo;
    jhdrtrx = jheader["hdr_trx"];

    sql  = "select   * ";
    sql += "from     pos_giftitem_promo a, pos_giftitem_promo_trans b ";
    sql += "where    a.gift_code = b.gift_code ";
    sql += " and     a.start_date <= ";
    sql += dbsel->sql_bind (1, _util->date_eight());
    sql += " and     a.end_date >= ";
    sql += dbsel->sql_bind (2, _util->date_eight());
    sql += " and     b.transaction_cd = ";
    sql += dbsel->sql_bind (3, jhdrtrx["transaction_code"].asString());

    sqt = json_select_multiple(dbsel, jgiftpromo, sql);
    if (sqt == salesQueryType::SQT_ERROR)
        return false;
    else if (sqt == salesQueryType::SQT_NOROWS)
        return true;

    //////////////////////////////////////
    //  For each qualified promotion
    string tmp;
    double promoTotal = 0.00;
    Json::Value jpromo, jitem,
                jcategory;
    int j = jgiftpromo.size();
    for (int i = 0; i < j; i++)  {
        jpromo = jgiftpromo[i];
        double quantity, retailPrice;
        //////////////////////////////////
        //  Check if item(s) qualify
        int y = jdetail.size();
        for (int x = 0; x < y; x++)  {
            jitem = jdetail[x];
            tmp = jitem["quantity"].asString();
            if (tmp .length() < 1) tmp = "0.00";
            quantity = stodsafe(tmp);

            tmp = jitem["retail_price"].asString();
            if (tmp .length() < 1) tmp = "0.00";
            retailPrice = stodsafe(tmp);

            dbsel->sql_reset();
            sql  = "select count(*) as recs ";
            sql += "from   pos_giftitem_promo_sku ";
            sql += "where  gift_code = ";
            sql +=  dbsel->sql_bind (1, jpromo["gift_code"].asString());
            sql += " and   item_code = ";
            sql += dbsel->sql_bind (2, jitem["item_code"].asString());

            if (!dbsel->sql_result(sql,true))
                return seterrormsg(dbsel->errordb());

            if (atoi(dbsel->sql_field_value("recs").c_str()))  {
                promoTotal += quantity * retailPrice;
            }  else  {
                //////////////////////////
                //  qualify by hierarchy
                dbsel->sql_reset();
                sql  = "select * ";
                sql += " from  pos_giftitem_promo_dept ";
                sql += " where gift_code = ";
                sql += dbsel->sql_bind(1, jpromo["gift_code"].asString());

                if (!dbsel->sql_result(sql,true))
                    return seterrormsg(dbsel->errordb());

                bool bexit = false;
                string itemCategory = "";
                string promoCategory = "";
                if (!dbsel->eof ())  {
                    do  {
                        jcategory.clear ();
                        jcategory = jitem["detail_product_header"];

                        itemCategory  = jcategory["category_cd"].asString();
                        if (dbsel->sql_field_value ("subcat_cd").length() > 0)
                            itemCategory += jcategory["subcat_cd"].asString();
                        if (dbsel->sql_field_value ("class_cd").length() > 0)
                            itemCategory += jcategory["class_cd"].asString();
                        if (dbsel->sql_field_value ("subclass_cd").length() > 0)
                            itemCategory += jcategory["subclass_cd"].asString();

                        promoCategory  = dbsel->sql_field_value ("category_cd");
                        promoCategory += dbsel->sql_field_value ("subcat_cd");
                        promoCategory += dbsel->sql_field_value ("class_cd");
                        promoCategory += dbsel->sql_field_value ("subclass_cd");

                        dbsel->sql_next ();
                        bexit = dbsel->eof ();
                        if (promoCategory == itemCategory)  {
                            promoTotal += quantity * retailPrice;
                            bexit = true;
                        }
                    } while (!bexit);
                }
            }
        }
        //////////////////////////////////
        //  Minimum promo amount
        tmp = jpromo["min_amount"].asString();
        if (tmp .length() < 1) tmp = "0.00";
        double promoMin = stodsafe(tmp);
        /////////////////////////////////
        Json::Value jbranch, jsetting, jcompany, jtrxtype;
        if (promoTotal >= promoMin)  {
           ///////////////////////////////
           // Promo Satisfied
            _isGiftPromo = true;
           _txtpromoitem += "\n\n\n";
           ///////////////////////////////
           //  No fractional portion
           long freeItem = (long) promoTotal / promoMin;
           string barCode = _util->fmt_number(3, jheader["register_number"].asString().c_str());
           barCode += _util->date_sequence().c_str();
           ///////////////////////////////
           //  Extended receipt
           size_t  w = g_widthReceipt;
           jbranch = jheader["hdr_branch"];
           jsetting = jheader["hdr_settings"];
           jcompany = jheader["hdr_company"];
           jtrxtype = jhdrtrx["trx_type"];

           _txtpromoitem += _util->cyCenterText(w, jcompany["description"].asString());
           _txtpromoitem += _util->cyCenterText(w, jsetting["trade_name"].asString());
           _txtpromoitem += _util->cyCenterText(w, jbranch["description"].asString());
           _txtpromoitem += _util->cyCenterText(w, RCPT_REPRINT);
           _txtpromoitem += "\n\n";

           if(jtrxtype["is_postvoid"].asString() == "1" )
               _txtpromoitem += _util->cyCenterText(w, "*** V O I D ***");

           dbsel->sql_reset();
           sql  = "select * from  pos_giftitem_promo_rcpt ";
           sql += "where  gift_code = ";
           sql += dbsel->sql_bind(1, jpromo["gift_code"].asString());
           sql += " order by seq_num ";

           if (!dbsel->sql_result(sql,true))
               return seterrormsg(dbsel->errordb());

           while (!dbsel->eof ())  {
               string line   = dbsel->sql_field_value("rcpt_line");
               string center = dbsel->sql_field_value("is_center");

               size_t found = line.find("<QTY>");
               if (found != std::string::npos)
                    line.replace (found, 5, _util->longtostring(freeItem));

               if (center == "1") {
                   _txtpromoitem += _util->cyCenterText(w, line);
               }  else  {
                   _txtpromoitem += line;
               }
               dbsel->sql_next();
           }
           _txtpromoitem += "\n\n";
           _txtpromoitem += _util->cyLRText(w/2, "TOTAL PURCHASE. ",
                                            w/2, _util->fmt_decimal(2, _util->doubletostring(promoTotal).c_str()).c_str());
           string left, right = "TERM NO. ";
           right += jheader["register_number"].asString();

           string trans_time,trans_date,dd,mm,yyyy;
           trans_date = jheader["transaction_date"].asString();
           trans_time = jheader["transaction_time"].asString();
           _util->date_split(trans_date.c_str(),yyyy,mm,dd);

           left  = mm; left += "/"; left += dd;
           left += "/"; left += yyyy; left += "-";
           left += _util->time_colon(trans_time.c_str());
           _txtpromoitem += _util->cyLRText(w/2, left.c_str(), w/2, right.c_str());


           left  = "TRANS NO. ";
           left += jheader["transaction_number"].asString();

           Json::Value jcashier;
           jcashier = jheader["settings_cashier"];
           right  = jcashier["last_name"].asString();
           right += ",";
           right += jcashier["first_name"].asString();

           _txtpromoitem += _util->cyLRText(w/2, left.c_str() ,w/2, right.c_str());
           _txtpromoitem += "\n";


           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += _util->cyCenterText(w, barCode.c_str());
           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += "\n";
           _txtpromoitem += _util->cyAcrossText(w, "_");
           _txtpromoitem += _util->cyCenterText(w, "Customer Signature over Printed Name");
           _txtpromoitem += "\n\n\n";
           _txtpromoitem += RCPT_CUT;

           db->sql_reset ();
           sql  = "insert into pos_giftitem_promo_redeem (gift_code, ";
           sql += "item_code, bar_code, total_purchase, is_redeem ) values('";
           sql += jpromo["gift_code"].asString();
           sql += "', '', '";
           sql += barCode;
           sql += "',";
           sql += _util->doubletostring(promoTotal);
           sql += ",0)";

            if(!db->sql_only(sql,false)){
                return seterrormsg(db->errordb());
            }

            if(!db->sql_commit()){
                return seterrormsg(db->errordb());
            }
        }
        promoTotal = 0.00;
    }
    _receipt += _txtpromoitem;
    return true;
}

