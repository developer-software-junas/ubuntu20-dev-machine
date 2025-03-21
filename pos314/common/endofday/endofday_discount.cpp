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
      FUNCTION:          assembleEodDiscount
   DESCRIPTION:          EOD discount portion
     ARGUMENTS:          none
       RETURNS:          true on success
 *******************************************************************/
bool                     cyposrest::assembleEodDiscount ()
{
    _txtdiscount = "";
    int totalCount = 0;
    double totalAmount = 0;
    string tmp, left, right;
    //////////////////////////////////////
    //  Display the discount list
    size_t  w = g_paperWidth;
    _txtdiscount += _util->cyAcrossText(w, "-");
    _txtdiscount += _util->cyCenterText(w,"Discount");
    _txtdiscount += _util->cyAcrossText(w, "-");

    int j = _eod->listPayDiscount.size ();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_DISCOUNT* ptr = _eod->listPayDiscount.at(i);
        totalCount += ptr->count;
        totalAmount += ptr->amount;

        left  = "Discount";
        left += " (Count)";
        right = "Amount";
        _txtdiscount += _util->cyLRText(24,left,17,right);

        left  = " ";
        left += ptr->discountDesc;
        left += " (";
        char szCount [16];
        sprintf (szCount, "%d", ptr->count);
        left += szCount;
        left += ")";
        right = _eod->FMTNumberComma(ptr->amount);
        _txtdiscount += _util->cyLRText(24,left,17,right);
    }

    j = _eod->listItemDiscount.size ();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_DISCOUNT* ptr = _eod->listItemDiscount.at(i);
        totalCount += ptr->count;
        totalAmount += ptr->amount;

        left  = "Discount";
        left += " (Count)";
        right = "Amount";
        _txtdiscount += _util->cyLRText(24,left,17,right);

        left  = " ";
        left += ptr->discountDesc;
        left += " (";
        char szCount [16];
        sprintf (szCount, "%d", ptr->count);
        left += szCount;
        left += ")";
        right = _eod->FMTNumberComma(ptr->amount);
        _txtdiscount += _util->cyLRText(24,left,17,right);
    }

    left  = "Total Discount (";
    char szCount [16];
    sprintf (szCount, "%d", totalCount);
    left += szCount;
    left += ")";
    right = _eod->FMTNumberComma(totalAmount);
    _txtdiscount += _util->cyLRText(24,left,17,right);
    _discountTotal = totalAmount;
    _receipt += _txtdiscount;
    //////////////////////////////////////
    //  Database connection
    _txtdiscount = "";
    char sztmp[32];
    double grandTotal = 0.00;
    string line, ordernum, sysnum, systotal, middle;
    bool xread_order_list = false;
    CYDbSql* db = _ini->dbconn ();
    if (!db->ok())
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Include order(s)
    int idx = 1;
    db->sql_bind_reset ();
    string sql = "select * from pos_settings where location_code =  ";
    sql += db->sql_bind(idx,_eod->argBranch); idx++;
    if (db->sql_result(sql,true))  {
        if (!db->eof ())  {
            string xtmp = db->sql_field_value("xread_order_list");
            xread_order_list = xtmp == "1";
        }
    }
    if (xread_order_list)  {
        string cashier = _eod->getAltValue("login",_eod->alt_settings_cashier);
        string register_num = _eod->getAltValue("register_num",_eod->alt_settings_register);
        string sysdate = _eod->getAltValue("transaction_date",_eod->alt_settings_sysdate);

        idx = 1;
        db->sql_bind_reset ();
        sql = "select transaction_number, pac_number as order_number, ridc_number as total_amount ";
        sql += "from tg_pos_daily_header ";
        sql += "where register_number = ";
        sql += db->sql_bind(idx,register_num); idx++;
        sql += " and  logical_date = ";
        sql += db->sql_bind(idx,sysdate); idx++;
        if (_eod->argXread)  {
            sql += " and cashier = ";
            sql += db->sql_bind(idx,cashier); idx++;
        }
        sql += " order by order_number asc, transaction_number asc ";
        if (!db->sql_result(sql,true))
            return true;

        if (!db->eof())  {
            //////////////////////////////////////
            //  Service charge
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyCenterText(w,"ORDER AND PAYMENT INFO");
            _txtdiscount += _util->cyAcrossText(w, "-");
            line  = _util->cyPostSpace(10,"Order#");
            line += _util->cyPostSpace(10,"OR#");
            line += _util->cyPreSpace(20,"Total");
            line += "\n";
            _txtdiscount += line;
            do  {
                sysnum = db->sql_field_value("transaction_number");
                ordernum = db->sql_field_value("order_number");
                systotal = db->sql_field_value("total_amount");
                grandTotal += _util->stodsafe(systotal);

                if (ordernum.length() > 0)  {
                    long lnum = atol(sysnum.c_str());
                    sprintf(sztmp,"%010ld", lnum);

                    line  = _util->cyPostSpace(10,ordernum);
                    line += _util->cyPostSpace(10,sztmp);
                    line += _util->cyPreSpace(20,systotal);
                    line += "\n";
                    _txtdiscount += line;
                }
                db->sql_next();
            }  while (!db->eof());
            sprintf (sztmp, "%.02f", grandTotal);
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyLRText(13, "Grand Total", 28, sztmp);
        }
        //////////////////////////////////////
        //  Cancelled transaction(s)
        grandTotal = 0.00;
        if (_eod->listCancel.size() > 0)  {
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyCenterText(w,"CANCELLED TRANSACTION(S)");
            _txtdiscount += _util->cyAcrossText(w, "-");
        }
        for (size_t x = 0; x < _eod->listCancel.size(); x++)  {

            line  = _util->cyPostSpace(10,"Order#");
            string rcptType = _ini->get_value("RECEIPT","TYPE");
            if (rcptType == "elena")  {
                line += _util->cyPostSpace(18,"Chit#");
            }  else  {
                line += _util->cyPostSpace(18,"Ref#");
            }
            line += _util->cyPreSpace(12,"Total");
            line += "\n";
            _txtdiscount += line;

                sysnum = _eod->listCancel[x]->trxnum;
                ordernum = _eod->listCancel[x]->ordernum;
                sprintf(sztmp,"%.02f", _eod->listCancel[x]->amount);
                grandTotal += _eod->listCancel[x]->amount;

                line  = _util->cyPostSpace(10,ordernum);
                line += _util->cyPostSpace(18,sysnum);
                line += _util->cyPreSpace(12,sztmp);
                    line += "\n";
                    _txtdiscount += line;


        }
        if (_eod->listCancel.size() > 0)  {
            sprintf (sztmp, "%.02f", grandTotal);
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyLRText(14, "Cancel Total", 27, sztmp);
        }
        //////////////////////////////////////
        //  Voided transaction(s)
        grandTotal = 0.00;
        if (_eod->listVoid.size() > 0)  {
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyCenterText(w,"VOID TRANSACTION(S)");
            _txtdiscount += _util->cyAcrossText(w, "-");
        }
        for (size_t x = 0; x < _eod->listVoid.size(); x++)  {

            line  = _util->cyPostSpace(10,"Order#");
            string rcptType = _ini->get_value("RECEIPT","TYPE");
            if (rcptType == "elena")  {
                line += _util->cyPostSpace(18,"Chit#");
            }  else  {
                line += _util->cyPostSpace(18,"Ref#");
            }
            line += _util->cyPreSpace(12,"Total");
            line += "\n";
            _txtdiscount += line;

            sysnum = _eod->listVoid[x]->trxnum;
            ordernum = _eod->listVoid[x]->ordernum;
            sprintf(sztmp,"%.02f", _eod->listVoid[x]->amount);
            grandTotal += _eod->listVoid[x]->amount;

            line  = _util->cyPostSpace(10,ordernum);
            line += _util->cyPostSpace(18,sysnum);
            line += _util->cyPreSpace(12,sztmp);
                    line += "\n";
                    _txtdiscount += line;


        }
        if (_eod->listVoid.size() > 0)  {
            sprintf (sztmp, "%.02f", grandTotal);
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyLRText(14, "Void Total", 27, sztmp);
        }
        //////////////////////////////////
        //  Transaction summary
        db->sql_reset ();
        grandTotal = 0.00;
        sql = "select description, count(*) as count ";
        sql += "from tg_pos_mobile_transtype where systransnum IN ";
        sql += "(select z.systransnum from tg_pos_daily_header z ";
        sql += "where z.register_number = ";
        sql += register_num;
        sql += " and  z.logical_date = ";
        sql += sysdate;
        if (_eod->argXread)  {
            sql += " and z.cashier = '";
            sql += cashier; sql += "' ";
        }
        sql += ") group by description ";
        if (!db->sql_result(sql,false))
            return true;
        //_log->logmsg("TRXTOIT", sql.c_str());
        _txtdiscount += _util->cyAcrossText(w, "-");
        _txtdiscount += _util->cyCenterText(w,"TRANSACTION SUMMARY");
        _txtdiscount += _util->cyAcrossText(w, "-");
        if (!db->eof())  {
            //////////////////////////////////////
            //  Service charge

            line  = _util->cyPostSpace(22,"Transaction Type");
            line += _util->cyPreSpace(18,"Count");
            line += "\n";
            _txtdiscount += line;
            do  {
                ordernum = db->sql_field_value("description");
                systotal = db->sql_field_value("count");
                grandTotal += _util->stodsafe(systotal);

                if (ordernum.length() > 0)  {
                    line  = _util->cyPostSpace(22,ordernum);
                    line += _util->cyPreSpace(18,systotal);
                    line += "\n";
                    _txtdiscount += line;
                }
                db->sql_next();
            }  while (!db->eof());
            sprintf (sztmp, "%.02f", grandTotal);
            _txtdiscount += _util->cyAcrossText(w, "-");
            _txtdiscount += _util->cyLRText(13, "Total Count", 28, sztmp);
        }
    }    
    _receipt += _txtdiscount;
    return true;
}
