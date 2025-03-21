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
      FUNCTION:         checkSales
   DESCRIPTION:         Check for unprocessed sales record(s)
 ********************************************************************/
bool                    cysaleshost::checkShopifySales ()
{
    //////////////////////////////////////
    //  Main loop db connection
    string sql, systransnum, location_code,
           oldsystransnum, oldlocation_code;
    CYDbSql* db = dbconn ();
    if (nullptr == db)
        return seterrormsg(db->errordb());

    CYDbSql* dbsel = dbconn ();
    if (nullptr == dbsel)
        return seterrormsg(dbsel->errordb());
    _cgi->add_form("ini","shopify");
    //////////////////////////////////////
    //  Upload sales record(s)
    dbsel->sql_reset();
    sql  = "select * from tg_pos_daily_header where is_polled2 <> 888 limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            if (!processShopifySales(systransnum,SRT_DAILY))
                return false;
            db->sql_reset ();
            sql = "update tg_pos_daily_header ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Upload voided record(s)
    dbsel->sql_reset();
    sql  = "select * from tg_pos_void_header where is_polled2 <> 888  limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            location_code = dbsel->sql_field_value("branch_code");
            if (!processShopifyVoid(systransnum,location_code))
                return false;
            db->sql_reset ();
            sql = "update tg_pos_void_header ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    //////////////////////////////////////
    //  Upload refunded record(s)
    dbsel->sql_reset();
    sql  = "select * from tg_pos_refund_header where is_polled2 <> 888  limit 10 ";
    if (!dbsel->sql_result(sql,false))
        return seterrormsg(dbsel->errordb());
    if (!dbsel->eof ())  {
        do  {
            systransnum = dbsel->sql_field_value("systransnum");
            location_code = dbsel->sql_field_value("branch_code");
            oldsystransnum = dbsel->sql_field_value("orig_systransnum");
            oldlocation_code = dbsel->sql_field_value("orig_location_code");
            if (!processShopifyRefund(systransnum,location_code,
                                      oldsystransnum,oldlocation_code))
                return false;
            db->sql_reset ();
            sql = "update tg_pos_refund_header ";
            sql += " set is_polled2 = 888 where systransnum = ";
            sql += db->sql_bind(1,systransnum);
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            //////////////////////////////////////
            //  Save the changes
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    return true;
}
