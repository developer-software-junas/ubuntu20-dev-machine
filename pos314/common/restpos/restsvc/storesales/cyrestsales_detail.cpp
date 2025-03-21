//*******************************************************************
//        FILE:     cyrestsales_detail.cpp
// DESCRIPTION:     Sales detail record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "cydb/cydbsql.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesDetail
// DESCRIPTION:     Retrieve the sales record
//*******************************************************************
bool CYRestCommon::retrieveSalesDetail(Json::Value& jdetail,
                                        std::string branch_code,
                                        string systransnum)
{
    //////////////////////////////////////
    //  Reset total(s)
    _isGiftWrap = false;

    _totalItems = 0;
    _totalNetSales = 0.00;
    _totalVatAmount = 0.00;
    _totalTrxDiscount = 0.00;
    _totalGrossAmount = 0.00;
    _totalItemDiscount = 0.00;

    _totalNetVat=0.00;
    _totalNetZero=0.00;
    _totalNetExempt=0.00;

    _totalAmtVat=0.00;
    _totalAmtZero=0.00;
    _totalAmtExempt=0.00;

    _totalDiscountZero = 0.00;

    _totalAddonAmount = 0.00;

    _fnbFood = 0;
    _fnbBeverage = 0;
    _totalServiceCharge=0;
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    CYDbSql* db2 = _ini->dbconn ();
    if (nullptr == db2)
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbset = _ini->dbconn ();
    if (nullptr == dbset)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Figure out the sort
    string sql = "select count(*) as recs from tg_pos_mobile_discount_detail where systransnum = ";
    sql += db->sql_bind (1, systransnum);
    sql += " and discount_code IN (select z.discount_cd from pos_discount z where z.discount_rule = '1')";
    if (!db->sql_result (sql,true))
        return seterrormsg(db->errordb());
    string recs = db->sql_field_value("recs");
    bool sortDesc = atoi(recs.c_str()) > 0;
    //////////////////////////////////////
    //  Create the detail record
    jdetail.clear();
    Json::Value jsettings;
    //////////////////////////////////////
    //  Bind and select the detail
    sql  = "select * from tg_pos_mobile_detail where systransnum = ";
    sql += db->sql_bind (1, systransnum);
    if (sortDesc)
        sql += " order by description asc ";
    else
        sql += " order by item_seq desc, consign_reference asc ";

    if (!db->sql_result (sql,true))
        return seterrormsg(db->errordb());
    if (db->eof ())  {
        jdetail = Json::nullValue;
    }  else  {
        int idx = 0;
        string name, val;
        Json::Value jline;
        jline.clear ();

        do  {
            for (int i = 0; i < db->numcols(); i++)  {
                name = db->sql_field_name(i);
                val = db->sql_field_value(name);
                jline[name]=val;
            }
            if (_exportType.length() > 0)  {
                dbset->sql_reset ();
                assembleSqlInsert(db,"tg_pos_mobile_detail",sql);
                _sqlSales += sql;
            }
            string item_seq = jline["item_seq"].asString();
            string item_code = jline["item_code"].asString();
            //////////////////////////////////
            //  Picture
            //////////////////////////////////////
            //  Directory file list
            struct dirent *entry,
                          **namelist;
            string imgdir  = _ini->get_value("PATH","FNBIMAGE");
            imgdir += item_code;
            imgdir += "/";
            //////////////////////////////////////
            //  Get the list of directories
            string picture_file;
            picture_file = "";
            bool found = false;
            int entries = scandir (imgdir.c_str (), &namelist, NULL, alphasort);
            if (entries >= 3)  {
                //////////////////////////////////////
                //  For each item
                string imgurl,
                       sp = "            ";
                int idxpath = 0;
                Json::Value jpath;
                for (int i = 0; i < entries; i++) {
                    entry = namelist [i];
                    //////////////////////////////////
                    //  Type
                    if (entry->d_type == DT_DIR) {
                        jpath[idxpath]["name"] = entry->d_name;
                        ++idxpath;
                    }  else if (entry->d_type == DT_REG)  {
                        //  image url
                        imgurl  = _ini->get_value("PATH","FNBURL");
                        imgurl += item_code;
                        imgurl += "/";
                        imgurl += entry->d_name;
                        picture_file = imgurl;
                        found = true;
                    }
                }
            }
            if (found)
                jline["picture_url"]=picture_file;
            //////////////////////////////////
            //  Product info
            jsettings.clear ();
            dbset->sql_reset ();
            string initype = cgiform("ini");
            sql  = "select a.* ";
            if (initype == "shopify")  {
                sql += ", '' as category_desc ";
            }  else  {
                sql += ", b.description as category_desc ";
            }
            sql += "from   cy_baseitem_hdr a ";
            if (initype != "shopify")  {
                sql += ", cy_category b ";
            }
            sql += "where  a.item_code = '"; sql += item_code; sql += "' ";
            if (initype != "shopify")  {
                sql += " and   a.category_cd = b.category_cd ";
            }
            _log->logmsg("PRODUCT", sql.c_str());
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,sql.c_str ()))
                return false;
            jline["detail_product_header"]=jsettings;
            //////////////////////////////////
            //  Product detail info
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from cy_baseitem_detail where item_code = ";
            sql += dbset->sql_bind(1,item_code);
            sql += " and   location_code = ";
            sql += dbset->sql_bind(2,branch_code);
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve branch product information"))
                return false;
            jline["det_product"]=jsettings;
            //////////////////////////////////
            //  HACK - manager price change
            /*
            Json::Value jhack;
            dbset->sql_reset ();
            sql  = "select * from pos_manager_item where module_number = 503 and item_code = '";
            sql += item_code; sql += "'";

            if (salesQueryType::SQT_SUCCESS == json_select_single(dbset,jhack,sql,"Unable to retrieve branch product information"))  {
                _log->logmsg(sql.c_str (), dbset->sql_field_value("is_manager").c_str ());
                jsettings["wip"]=jhack["is_manager"].asString();
            }  else  {
            _log->logmsg(sql.c_str (), "no recs found");
                jsettings["wip"]="1";
            }
            */
            jsettings["wip"]="1";
            //jline["det_product"]=jsettings;
            //std::stringstream xx;
            //xx << jsettings;
            //_log->logdebug("XX",xx.str().c_str());
            //////////////////////////////////
            //  Tax info
taxinfo001:
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from cy_baseitem_tax where item_code = ";
            sql += dbset->sql_bind(1,item_code);
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve product tax information"))  {
                dbset->sql_reset ();
                sql  = "insert into cy_baseitem_tax (tax_code, item_code) values ('12', ";
                sql += dbset->sql_bind(1,item_code);
                sql += ") ";
                if (!dbset->sql_only (sql,true))
                    return seterrormsg("Unable to fix the product tax information");
                if (!dbset->sql_commit())
                    return seterrormsg("Unable to fix and save the product tax information");
                goto taxinfo001;
            }
            jline["tax_code"]=dbset->sql_field_value("tax_code");
            //////////////////////////////////
            //  TAX rule
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from cy_tax where tax_code = ";
            sql += dbset->sql_bind(1,jline["tax_code"].asString());
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve product tax information"))
                return false;
            jline["det_tax"]=jsettings;
            //////////////////////////////////
            //  UOM
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from cy_uom where uom_code = ";
            sql += dbset->sql_bind(1,jline["uom_code"].asString());
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve UOM information"))  {
                jline["det_uom"]=Json::nullValue;
            }  else  {
                jline["det_uom"]=jsettings;
                string uom_type_code = jsettings["type_code"].asString();
                //////////////////////////////
                //  UOM type
                jsettings.clear ();
                dbset->sql_reset ();
                sql  = "select * from cy_uom_type where type_code = ";
                sql += dbset->sql_bind(1,uom_type_code);
                if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve UOM type information"))
                    jline["detail_uom_type"]=Json::nullValue;
                else
                    jline["detail_uom_type"]=jsettings;
            }
            //////////////////////////////////
            //  Food or beverage
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from tg_recipe_item where recipe_item_code = ";
            sql += dbset->sql_bind(1,item_code);
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve FNB printer information"))  {
                jline["printer_code"]=Json::nullValue;
            }  else  {
                jline["printer_code"]=jsettings["printer_code"].asString();
            }
            //////////////////////////////////
            //  Loyalty points
            Json::Value jpoints;
            jpoints.clear ();
            dbset->sql_reset ();
            sql  = "select * from cy_baseitem_detail_points where item_code = ";
            sql += dbset->sql_bind(1,item_code);
            sql += " and   location_code = ";
            sql += dbset->sql_bind(2,branch_code);
            salesQueryType sqt = json_select_multiple(dbset,jpoints,sql);
            if (sqt == SQT_SUCCESS)
                jline["det_points"]=jpoints;
            else
                jline["det_points"]=Json::nullValue;
            //////////////////////////////////
            //  Split details
            Json::Value jsplit;
            jpoints.clear ();
            dbset->sql_reset ();
            sql  = "select * from tg_pos_mobile_detail_split where systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   item_seq = ";
            sql += dbset->sql_bind(2,item_seq);
            sqt = json_select_single(dbset,jsplit,sql);
            if (sqt == SQT_SUCCESS)
                jline["det_split"]=jsplit;
            else
                jline["det_split"]=Json::nullValue;
            //////////////////////////////////
            //  Discount info
            jsettings.clear ();
            dbset->sql_reset ();
            string override_value = "", discount_code = "";
            sql  = "select * from tg_pos_mobile_discount_detail where systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   item_seq = ";
            sql += dbset->sql_bind(2,item_seq);

            discount_code = "";
            salesQueryType retcode = json_select_single(dbset,jsettings,sql,"Unable to retrieve the item discount information");
            if (retcode == salesQueryType::SQT_NOROWS)  {
                jsettings = Json::nullValue;
                jline["det_discount"]=jsettings;
            } else if (retcode == salesQueryType::SQT_ERROR)  {
                return false;
            } else  {
                if (_exportType.length() > 0)  {
                    assembleSqlInsert(dbset,"tg_pos_mobile_discount_detail",sql);
                    _sqlSales += sql;
                }
                jline["det_discount"]=jsettings;
                override_value = dbset->sql_field_value("discount_value");
                discount_code = jsettings["discount_code"].asString();
            }

            //string discount_type = dbset->sql_field_value("discount_type");
            //string discount_value = dbset->sql_field_value("discount_value");
            //////////////////////////////////////
            //  Detail discount
            string tg_type,
                   discount_type_code;
            tg_type = jsettings["tg_type"].asString();
            jsettings.clear ();
            dbset->sql_reset ();
            discount_type_code = "";
            if (discount_code.length() > 0 && jline["det_discount"] != Json::nullValue)  {
                if (tg_type == "discount")  {
                    sql  = "select * from pos_discount where discount_cd = ";
                    sql += dbset->sql_bind(1,discount_code);
                    salesQueryType retcode = json_select_single(dbset,jsettings,sql,"Unable to retrieve detail discount information");
                    if (retcode != salesQueryType::SQT_SUCCESS)  {
                        jsettings = Json::nullValue;
                    }  else
                        discount_type_code = jsettings["discount_type_cd"].asString();
                }  else if (tg_type == "event")  {
                    sql  = "select       a.*, b.item_code, b.promo_price, d.description, d.event_type ";
                    sql += "from         pos_price a, pos_price_item b, pos_price_location c, pos_price_priority d ";
                    sql += "where        a.event_cd = b.event_cd and a.event_cd = c.event_cd ";
                    sql += "and          a.event_type = d.event_type and c.location_code = ";
                    sql += dbset->sql_bind(1,branch_code);
                    sql += " and b.item_code = ";
                    sql += dbset->sql_bind(2,jline["item_code"].asString());
                    sql += " and         a.start_date <= ";
                    sql += dbset->sql_bind(3,_util->date_eight ());
                    sql += " and         a.end_date >= ";
                    sql += dbset->sql_bind(4,_util->date_eight ());
                    sql += " and         a.start_time <= ";
                    sql += dbset->sql_bind(5,_util->time_eight());
                    sql += " order by    b.item_code asc, d.priority asc, a.start_date desc, a.event_cd desc ";

                    if (!dbset->sql_result(sql,true))
                        return seterrormsg(dbset->errordb());
                    if (!dbset->eof ())  {
                        discount_type_code = "ITEM_AUTO";
                        string adtype = dbset->sql_field_value ("adtype");
                        jsettings["discount_cd"]=dbset->sql_field_value("event_cd");
                        jsettings["discount_type_cd"]=discount_type_code;
                        jsettings["description"]=dbset->sql_field_value("description");
                        jsettings["disctype"]="0";
                        jsettings["readonly"]="1";

                        char sztmp[64];
                        double discount_amount = _util->stodsafe(dbset->sql_field_value("promo_price"));
                        double retail_price = _util->stodsafe(jline["retail_price"].asString());
                        double diff = 0.00;
                        diff = retail_price - discount_amount;
                        sprintf(sztmp, "%.02f", diff);

                        if (diff == 0)
                            jsettings["discvalue"]="0.00";
                        else
                            jsettings["discvalue"]=sztmp;
                        jsettings["end_date"]=dbset->sql_field_value("end_date");
                        jsettings["end_time"]=dbset->sql_field_value("end_time");
                        jsettings["start_date"]=dbset->sql_field_value("start_date");
                        jsettings["start_time"]=dbset->sql_field_value("start_time");
                        jsettings["min_amount"]="0.01";
                        jsettings["max_amount"]="999999999.99";
                        jsettings["require_account"]="0";
                    }  else
                        jsettings = Json::nullValue;
                }
                //  Override???
                string readonly = jsettings["readonly"].asString();
                //if (readonly == "0")  {
                    //jsettings["discvalue"]=override_value;
                //}
                jline["settings_detail_discount"]=jsettings;
                discount_type_code = jsettings["discount_type_cd"].asString();

                jsettings.clear ();
                if (discount_type_code.length()>0)  {
                    dbset->sql_reset ();
                    sql  = "select * from pos_discount_type where discount_type_cd = ";
                    sql += dbset->sql_bind(1,discount_type_code);
                    if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve transaction discount type information"))
                        return false;
                }
                jline["settings_detail_discount_type"]=jsettings;
            }
            //////////////////////////////////////
            //  XITEM
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from tg_pos_mobile_xitem ";
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve  the XITEM setting(s)"))
                return false;
            jline["settings_xitem"]=jsettings;
            //////////////////////////////////////
            //  Modifiers
            Json::Value jmod;
            dbset->sql_reset ();
            sql  = "select * from tg_pos_mobile_modifier  where systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   item_seq = ";
            sql += dbset->sql_bind(2,item_seq);
            if (!dbset->sql_result(sql,true))
                return seterrormsg(db->errordb());
            if (!dbset->eof())  {
                int idx = 0;
                do  {

                    string name, val;
                    Json::Value jline,jval;
                    for (int i = 0; i < dbset->numcols(); i++)  {
                        name = dbset->sql_field_name(i);
                        val = dbset->sql_field_value(name);
                        jline[name]=val;
                    }
                    if (_exportType.length() > 0)  {
                        assembleSqlInsert(dbset,"tg_pos_mobile_modifier",sql);
                        _sqlSales += sql;
                    }
                    jmod[idx] = jline;
                    idx++;
                    dbset->sql_next ();
                } while (!dbset->eof ());
            }  else  {
                jmod = Json::nullValue;
            }
            ///////////////////////////////////////////////////
            //  Retrieve modifier information?
            Json::Value jmodifier, jmoddet, jtmp;
            if (jmod.size() > 0)  {
                //////////////////////////////////
                //  Menu modifier formatting
                int modidx = 0, moddetidx = 0;
                dbset->sql_reset();

                sql  = "select a.* from tg_modifier a where a.modifier_code in ";
                sql += "  (select z.modifier_code from tg_pos_mobile_modifier z ";
                sql += "   where z.systransnum = ";
                sql += dbset->sql_bind(1,systransnum);
                sql += ") ";

                if (!dbset->sql_result (sql,true))
                    return (seterrormsg(dbset->errordb()));
                if (!dbset->eof ())  {
                    modidx = 0;
                    do  {
                        jtmp.clear ();
                        for (int i = 0; i < dbset->numcols(); i++)
                            jtmp[dbset->sql_field_name(i)] =
                                    dbset->sql_field_value (dbset->sql_field_name(i));                        
                        jmodifier[modidx]=jtmp;
                        string modifier_code = dbset->sql_field_value("modifier_code");
                        string modifier_subcode = dbset->sql_field_value("modifier_subcode");
                        //////////////////////////////
                        //  Modifier detail format
                        moddetidx = 0;
                        jmoddet.clear();
                        db2->sql_reset ();
                        string modifier_type = dbset->sql_field_value("modifier_type");
                        if (modifier_type == "1")  {
                            sql  = "select     a.*, a.recipe_item_code as subrecipe_code, ";
                            sql += "           b.description, b.uom_type_code, b.sort_order ";
                            sql += "from       tg_modifier_detail a, tg_recipe_item b ";
                            sql += "where      a.modifier_code = ";
                            sql += db2->sql_bind (1, modifier_code);
                            sql += " and        a.recipe_item_code = b.recipe_item_code ";
                        }  else if (modifier_type == "3")  {
                            sql  = "select     a.*, b.subrecipe_name as description, b.uom_type_code ";
                            sql += "from       tg_modifier_subrecipes a, tg_subrecipe b ";
                            sql += "where      a.modifier_code = ";
                            sql += db2->sql_bind (1, modifier_code);
                            sql += " and        a.subrecipe_code = b.subrecipe_code ";
                        }
                        db2->sql_reset ();
                        sql  = "select a.*, b.item_code as recipe_item_code, b.description ";
                        sql += "from   tg_pos_mobile_modifier a, cy_baseitem_hdr b ";
                        sql += "where  a.modifier_subcode = b.item_code ";
                        sql += " and   a.modifier_code = ";
                        sql += db2->sql_bind(1, modifier_code);
                        sql += " and   a.item_seq = ";
                        sql += db2->sql_bind(2, item_seq);
                        sql += " order by a.addon_amount desc ";
                        if (!db2->sql_result (sql,true))
                            return (seterrormsg(dbset->errordb()));

                        if (!db2->eof ())  {
                            do  {
                                jtmp.clear ();
                                for (int i = 0; i < db2->numcols(); i++)
                                    jtmp[db2->sql_field_name(i)] =
                                            db2->sql_field_value (db2->sql_field_name(i));
                                if (modifier_type == "1")
                                    jtmp["modifier_subcode"]=db2->sql_field_value("recipe_item_code");
                                else
                                    jtmp["modifier_subcode"]=db2->sql_field_value("subrecipe_code");
                                jmoddet[moddetidx]=jtmp;
                                moddetidx++;
                                db2->sql_next ();
                            }  while (!db2->eof ());
                            ///////////////////////////////////////
                            //  Set the proper quantities
                            int y = jmoddet.size();
                            for (int x = 0; x < y; x++)  {
                                Json::Value jmodqty;
                                jmodqty = jmoddet[x];
                                int j = jmod.size();
                                for (int i = 0; i < j; i++)  {
                                    Json::Value jtmpmod = jmod[i];
                                    if (jtmpmod["modifier_code"].asString()
                                        == jmodqty["modifier_code"].asString() &&
                                        jtmpmod["modifier_subcode"].asString()
                                        == jmodqty["modifier_subcode"].asString())  {
                                        jmodqty["quantity"]=jtmpmod["quantity"];
                                        jmoddet[x]=jmodqty;
                                    }
                                }
                            }
                            jmodifier[modidx]["modifier_detail"]=jmoddet;
                        }
                        ///////////////////////////////////////
                        //  Next record
                        modidx++;
                        dbset->sql_next();
                    }  while (!dbset->eof());
                }
            }
            jline["det_modifier"]=jmod;
            jline["info_modifier"]=jmodifier;
            jdetail[idx] = jline;
            ///////////////////////////////////////////////////
            //  Additional calculation(s) are required
            if (!detailCalculate(jdetail[idx],branch_code))
                return false;
            jline.clear ();
            idx++;
            db->sql_next ();
        } while (!db->eof ());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         detailCalculate
// DESCRIPTION:         Additional detail calculation(s)
//*******************************************************************
bool                    CYRestCommon::detailCalculate(Json::Value& jdetail,
                                                      std::string branch_code)
{
    //////////////////////////////////////
    // Receipt type
    string rcptType = _ini->get_value("RECEIPT","TYPE");
    //////////////////////////////////////
    //  VAT exempt item???
    double vatRate = 0.00;
    bool vatExempt = false;
    //////////////////////////////////////
    //  Check the tax rate
    Json::Value jtax = jdetail["det_tax"];
    string tmp = jtax["tax_value"].asString();
    if (_util->valid_decimal("Tax value", tmp.c_str (), 1, 99, 0, 999999))  {
        vatRate = _util->stodsafe(tmp);
        if (vatRate == 0.00)
            vatExempt = true;
    }  else  {
        return seterrormsg(_util->errormsg());
    }
    //////////////////////////////////////
    //  Check if zero rated
    _isSen5 = false;
    bool vatRegular = false;
    bool trxOverride = (jdetail["override_trx_discount"].asString() == "1");
    Json::Value jdiscountitem = jdetail["settings_detail_discount"];
    std::stringstream xx;
    xx << jdetail["settings_detail_discount"];
    _log->logdebug("XXXXX",xx.str().c_str());
    if (jdiscountitem != Json::nullValue)  {
        jdetail["discount_code"] = jdiscountitem["discount_cd"].asString();
    }
    if (jdetail["discount_code"].asString () == "sen5")  {
        _isSen5 = true;
        jdetail["override_trx_discount"]="1";
        if (!detailCalculateRegular(jdetail))
            return false;
    }  else if (vatRate == 0 && !trxOverride)  {
        if (!detailCalculateZeroRated(jdetail))
            return false;
    }  else if (_isPwd || _isSenior)  {
        if (trxOverride)  {
            vatRegular = true;
            if (!detailCalculateRegular(jdetail))
                return false;
        }  else  {
            std::stringstream xx;
            xx << jdetail;
            _log->logdebug("XXXXX",xx.str().c_str());
            if (!detailCalculateSenior(jdetail))
                return false;
        }
    }  else if (_isZeroRated)  {
        //if (!detailCalculateZeroRated(jdetail))
            //return false;
        if (vatRate == 0 && vatExempt)  {
            if (!detailCalculateSenior(jdetail))
                return false;
        }  else  {
            if (!detailCalculateZeroRated(jdetail))
                return false;
        }
    }  else  {
        vatRegular = true;
        if (!detailCalculateRegular(jdetail))
            return false;
    }
    ////////////////////////////////////
    //  Gift wrap quantity
    double giftWrapQuantity = 0;
    tmp = jdetail["gift_wrap_quantity"].asString();
    if (_util->valid_decimal("Gift wrap quantity", tmp.c_str (), 1, 99, 0, 999999999))  {
        giftWrapQuantity = _util->stodsafe(tmp);
    }
    if (giftWrapQuantity > 0)
        _isGiftWrap = true;
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    //////////////////////////////////
    //  Product info branch
    Json::Value jproduct;
    jproduct.clear ();
    db->sql_reset ();
    sql  = "select * from cy_baseitem_detail where item_code = ";
    sql += db->sql_bind(1,jdetail["item_code"].asString());
    sql += " and   location_code = ";
    sql += db->sql_bind(2,branch_code);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db,jproduct,sql,"Unable to retrieve branch product information"))
        return false;
    db->sql_reset();
    //////////////////////////////////////
    //  Get the calculated total(s)
    string quantity = jdetail["quantity"].asString();
    string amount_discount = jdetail["amount_discount"].asString();
    string less_discount = jdetail["retail_price_adjusted"].asString();
    string service_charge = jproduct["service_charge"].asString();
    string service_charge_vat = jproduct["service_charge_vat"].asString();
    string is_auto_servicecharge = jdetail["is_auto_servicecharge"].asString();


    char szServiceCharge [64];
    strcpy (szServiceCharge, "0.00");
    double dqty = _util->stodsafe(quantity);
    double dretail_price_adjusted = _util->stodsafe(less_discount);
    //  Regular transactions are calculated net of VAT
    if (vatRegular && service_charge_vat == "0")  {
        double dNetVat = _util->stodsafe(jdetail["netVat"].asString());
        if (dqty > 1.00)
            dretail_price_adjusted = dNetVat / _util->stodsafe(quantity);
        else
            dretail_price_adjusted = dNetVat;
        /*jdetail["netVat"]=netVat;
        jdetail["netZero"]=netZero;
        jdetail["netExempt"]=netExempt;

        jdetail["amtVat"]=amtVat;
        jdetail["amtZero"]=amtZero;
        jdetail["amtExempt"]=amtExempt;*/
    }

    if (jdetail.isMember("is_service_charge"))  {
        bool isServiceCharge = (jdetail["is_service_charge"].asString() == "1");
        if (isServiceCharge)  {
            sql  = "update tg_pos_mobile_detail set is_service_charge = 1 ";
            if (is_auto_servicecharge == "1")  {
                double pctService = _util->stodsafe(jdetail["service_charge"].asString());
                sql += ", service_charge = "; sql += jdetail["service_charge"].asString();

                double totalServiceCharge = 0.00;
                double dretailZero = dretail_price_adjusted;
                //if (!vatExempt && _isZeroRated)  {
                    //dretailZero -= _totalAmtExempt;
                //}
                double percentage = (pctService / 100);
                double amount = dretailZero;
                if (dqty > 1.00)  amount = dretailZero * dqty;
                totalServiceCharge = percentage * amount;
                if (dqty < 1.00)
                    totalServiceCharge = ((pctService / 100) * (dretailZero));
                else
                    totalServiceCharge = ((pctService / 100) * (dretailZero * dqty));
                sprintf (szServiceCharge, "%.02f", totalServiceCharge);
                jdetail["total_service_charge"]=szServiceCharge;
                sql += ", total_service_charge = "; sql += jdetail["total_service_charge"].asString();
            }
        } else  {
            sql  = "update tg_pos_mobile_detail set is_service_charge = 0, is_auto_servicecharge = 0 ";
            is_auto_servicecharge = "0";
        }            
        sql += " where item_code = '";
        sql += jdetail["item_code"].asString();
        sql += "' and item_seq = '";
        sql += jdetail["item_seq"].asString();
        sql += "' and systransnum = '";
        sql += jdetail["systransnum"].asString();
        sql += "' ";
        if (!db->sql_only(sql,false))  {
            _error = seterrormsg(db->errordb());
            return false;
        }
    }

    //////////////////////////////////////
    //  Update the calculated total(s)
    //if (false == _isPwd && false == _isSenior)  {
        sql  = "update tg_pos_mobile_detail set less_discount = ";
        sql += jdetail["retail_price_adjusted"].asString();
        sql += ", amount_discount = ";
        sql += jdetail["amount_discount"].asString();
        sql += ", amount_trx = ";
        sql += jdetail["amount_trx"].asString();
        sql += ", amount_tax = ";
        sql += jdetail["amount_tax"].asString();
        sql += ", less_tax = ";
        sql += jdetail["less_tax"].asString();
        sql += " where item_code = '";
        sql += jdetail["item_code"].asString();
        sql += "' and item_seq = '";
        sql += jdetail["item_seq"].asString();
        sql += "' and systransnum = '";
        sql += jdetail["systransnum"].asString();
        sql += "' ";
        if (!db->sql_only(sql,false))  {
            _error = seterrormsg(db->errordb());
            return false;
        }
    //}
    if (!db->sql_commit())  {
        _error = seterrormsg(db->errordb());
        return false;
    }
    return true;
}
