//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleVatBreakdown
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool                CYReceiptRcc::xreadSaveTax (string systransnum)
{
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_env->errordbenv());
    //////////////////////////////////////
    //  See if a record already exists
    string sql = "select count(*) as recs from tg_pos_xread_tax ";
    sql += " where systransnum = ";
    sql += db->sql_bind(1,systransnum);
    if (!db->sql_result(sql,true))
        return seterrormsg(db->errordb());
    string recs = db->sql_field_value("recs");

    db->sql_reset ();
    if (atoi(recs.c_str ()) > 0)  {
        sql = "delete from tg_pos_xread_tax ";
        sql += " where systransnum = ";
        sql += db->sql_bind(1,systransnum);
        if (!db->sql_only(sql,true))
            return seterrormsg(db->errordb());
    }
    char sztmp[64];
    db->sql_bind_reset ();
    sql  = "insert into tg_pos_xread_tax(systransnum,net_vat,net_zero,net_exempt,";
    sql += "amt_vat,amt_zero,amt_exempt,net_amount,vat_amount,gross_amount,service_charge)";
    sql += " values (";
    sql += db->sql_bind(1,systransnum); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_vat * 100) / 100);
    sql += db->sql_bind(2,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_zero * 100) / 100);
    sql += db->sql_bind(3,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_exempt * 100) / 100);
    sql += db->sql_bind(4,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_vat * 100) / 100);
    sql += db->sql_bind(5,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_zero * 100) / 100);
    sql += db->sql_bind(6,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.amt_exempt * 100) / 100);
    sql += db->sql_bind(7,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.net_sales * 100) / 100);
    sql += db->sql_bind(8,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.vat_amount * 100) / 100);
    sql += db->sql_bind(9,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.gross_sales * 100) / 100);
    sql += db->sql_bind(10,sztmp); sql += ", ";
    sprintf (sztmp,"%.02f",(_xread_tax.service_charge * 100) / 100);
    sql += db->sql_bind(11,sztmp);
    sql += ")";
    if (!db->sql_only(sql,true))
        return seterrormsg(db->errordb());
    if (!db->sql_commit())
        return seterrormsg(db->errordb());

    return true;
}
