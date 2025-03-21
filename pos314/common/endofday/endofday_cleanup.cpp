/********************************************************************
          FILE:         rest_request.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include <string>
#include <sstream>
using std::string;
using std::stringstream;
#include "cyposrest.h"
/*******************************************************************
      FUNCTION:          offline_sales
   DESCRIPTION:          Generate offline sales
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::cleanupMobileSales(std::string branchCode,
                                                        std::string registerNumber,
                                                        std::string logicalDate,
                                                        std::string cashier)
{
    (void)cashier;
    (void)logicalDate;
    (void)registerNumber;
    char szFunc [] = "cleanupMobileSales";
    //////////////////////////////////////
    //  Database connection
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel){
        _error = _util->jsonerror(szFunc,_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Go through all sales record(s)
    int i, idx = 1;
    std::string sql;
    dbsel->sql_reset ();

    sql  = "select systransnum from tg_pos_mobile_header ";
    sql += "where branch_code = "; sql += dbsel->sql_bind (idx, branchCode); idx++;
    //sql += " and  logical_date = "; sql += dbsel->sql_bind (idx, logicalDate); idx++;
    sql += " and  register_number = "; sql += dbsel->sql_bind (idx, registerNumber); idx++;
    //sql += " and  cashier = "; sql += dbsel->sql_bind (idx, cashier); idx++;


    if (!dbsel->sql_result (sql,true))
        return seterrormsg (dbsel->errordb());
    if (!dbsel->eof ())  {
        //////////////////////////////////
        //  For each mobile record
        do  {
            //////////////////////////////
            //  Retrieve
            i = 0;
            string systransnum = dbsel->sql_field_value("systransnum");
            while (g_sales_tables[i].tableName != "eof")  {
                db->sql_bind_reset ();
                string table = g_sales_tables[i].tableName;
                sql  = "delete from ";
                sql += table;
                sql += " where systransnum = ";
                sql += db->sql_bind (1, systransnum); idx++;
                if (!db->sql_only(sql,true))
                    return seterrormsg(db->errordb());
                i++;
            }
            sql  = "delete from tg_pos_mobile_header ";
            sql += " where systransnum = ";
            sql += db->sql_bind (1, systransnum); idx++;
            if (!db->sql_only(sql,true))
                return seterrormsg(db->errordb());
            if (!db->sql_commit())
                return seterrormsg(db->errordb());
            dbsel->sql_next();
        } while (!dbsel->eof ());
    }
    return true;
}
