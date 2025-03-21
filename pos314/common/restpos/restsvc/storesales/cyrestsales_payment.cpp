//*******************************************************************
//        FILE:     cyrestsales_payment.cpp
// DESCRIPTION:     Sales detail record retrieval
//*******************************************************************
#include "cyrestcommon.h"
#include "cydb/cydbsql.h"
#include "cydb/cymysql.h"
using std::string;
//*******************************************************************
//    FUNCTION:     retrieveSalesPayment
// DESCRIPTION:     Retrieve the sales record
//*******************************************************************
bool CYRestCommon::retrieveSalesPayment(Json::Value& jpayment,
                                         string systransnum)
{
    _totalPaymentDiscount = 0.00;
    //////////////////////////////////////
    //  Database connection
    MySqlDb mdb (_ini->_env);
    CYDbSql* db = &mdb;
    MySqlDb mdbset (_ini->_env);
    CYDbSql* dbset = &mdbset;
    /*CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    CYDbSql* dbset = _ini->dbconn ();
    if (nullptr == dbset)
        return seterrormsg(_ini->errormsg());*/
    //////////////////////////////////////
    //  Create the detail record
    jpayment.clear();
    Json::Value jsettings;
    //////////////////////////////////////
    //  Bind and select the detail
    db->sql_reset();
    string sql  = "select * from tg_pos_mobile_payment where systransnum = ";
    sql += db->sql_bind (1, systransnum);
    sql += " order by pay_seq ";

    if (!db->sql_result (sql,true))
        return seterrormsg(db->errordb());
    if (db->eof ())  {
        jpayment = Json::nullValue;
    }  else  {
        int idx = 0;
        string name, val;
        Json::Value jline;
        jline.clear ();

        do  {
            string pay_seq = db->sql_field_value("pay_seq");
            string pay_code = db->sql_field_value("pay_code");
            string pay_type_code = db->sql_field_value("pay_type_code");
            _log->logmsg ("PAYTYPECODEFROMPAYMENT",pay_type_code.c_str ());
            for (int i = 0; i < db->numcols(); i++)  {
                name = db->sql_field_name(i);
                val = db->sql_field_value(name);
                jline[name]=val;
            }

            //////////////////////////////////
            //  Tender
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from pos_tender where tender_cd = ";
            sql += dbset->sql_bind(1,pay_code);
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve tender information"))
                return false;
            jline["tender"]=jsettings;
            //////////////////////////////////
            //  Tender type
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from pos_tender_type where tender_type_cd = ";
            sql += dbset->sql_bind(1,pay_type_code);
            if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve tender type information"))
                return false;
            jline["tendertype"]=jsettings;            
            //////////////////////////////////
            //  Discount info
            jsettings.clear ();
            dbset->sql_reset ();
            string discount_code;
            sql  = "select * from tg_pos_mobile_discount_payment where systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   pay_seq = ";
            sql += dbset->sql_bind(2,pay_seq);


            discount_code = "";
            salesQueryType retcode = json_select_single(dbset,jsettings,sql,"Unable to retrieve payment discount information");
            if (retcode == salesQueryType::SQT_NOROWS)
                jsettings = Json::nullValue;
            else if (retcode == salesQueryType::SQT_ERROR)
                return false;
            else  {
                discount_code = jsettings["discount_code"].asString();
            }
            jline["payment_discount"]=jsettings;
            //////////////////////////////////
            //  GC check the detail first
            dbset->sql_reset ();
            sql  = "select * from tg_pos_gc_detail where source_systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   source_pay_seq = ";
            sql += dbset->sql_bind(2,pay_seq);
            if (!dbset->sql_result (sql,true))  {
                seterrormsg(dbset->errordb());
                return false;
            }

            Json::Value jgc;
            jgc.clear ();
            if (!dbset->eof())  {
                string gc_pay_seq = pay_seq;//dbset->sql_field_value("pay_seq");
                string gc_systransnum = systransnum;//dbset->sql_field_value("systransnum");
                //////////////////////////////////
                //  GC info
                dbset->sql_reset ();
                sql  = "select * from tg_pos_mobile_gc_payment where systransnum = ";
                sql += dbset->sql_bind(1,gc_systransnum);
                sql += " and   pay_seq = ";
                sql += dbset->sql_bind(2,gc_pay_seq);

                retcode = json_select_multiple(dbset,jgc,sql,"Unable to retrieve GC payment information");
                if (retcode == salesQueryType::SQT_NOROWS)
                    jgc = Json::nullValue;
                else if (retcode == salesQueryType::SQT_ERROR)
                    return false;
                jline["gc"]=jgc;
            }  else  {
                jgc = Json::nullValue;
            }

            //////////////////////////////////
            //  GC balance info
            Json::Value jgcrefund;
            jsettings.clear ();
            dbset->sql_reset ();
            sql  = "select * from tg_pos_refund where systransnum = ";
            sql += dbset->sql_bind(1,systransnum);
            sql += " and   pay_seq = ";
            sql += dbset->sql_bind(2,pay_seq);

            retcode = json_select_multiple(dbset,jgc,sql,"Unable to retrieve GC balance information");
            if (retcode == salesQueryType::SQT_NOROWS)
                jgcrefund = Json::nullValue;
            else if (retcode == salesQueryType::SQT_ERROR)
                return false;
            jline["gcrefund"]=jgcrefund;
            //////////////////////////////////////
            //  Detail discount
            string discount_type_code;
            jsettings.clear ();
            dbset->sql_reset ();

            discount_type_code = "";
            if (discount_code.length() > 0)  {
                sql  = "select * from pos_discount where discount_cd = ";
                sql += dbset->sql_bind(1,discount_code);
                salesQueryType retcode = json_select_single(dbset,jsettings,sql,"Unable to retrieve detail discount information");
                if (retcode != salesQueryType::SQT_SUCCESS)
                    jsettings = Json::nullValue;
                else
                    discount_type_code = jsettings["discount_type_cd"].asString();
            }
            jline["settings_payment_discount"]=jsettings;

            jsettings.clear ();
            if (discount_type_code.length()>0)  {
                dbset->sql_reset ();
                sql  = "select * from pos_discount_type where discount_type_cd = ";
                sql += dbset->sql_bind(1,discount_type_code);
                if (salesQueryType::SQT_SUCCESS != json_select_single(dbset,jsettings,sql,"Unable to retrieve transaction discount type information"))
                    return false;
            }  else
                jsettings=Json::nullValue;
            jline["settings_payment_discount_type"]=jsettings;

            //////////////////////////////////
            //  Bank information
            dbset->sql_reset ();
            Json::Value jbank;

            sql = "select * from tg_pos_mobile_bank_payment ";
            sql += " where systransnum = ";
            sql += dbset->sql_bind (1,jline["systransnum"].asString());
            sql += " and pay_seq = ";
            sql += dbset->sql_bind (2,jline["pay_seq"].asString());

            if (!dbset->sql_result (sql, true))
                return seterrormsg(dbset->errordb ().c_str ());
            if (dbset->eof ())  {
                jbank = Json::nullValue;
            }  else  {
                jbank.clear();
                for (int i = 0; i < dbset->numcols(); i++)  {
                    name = dbset->sql_field_name(i);
                    val = dbset->sql_field_value(name);
                    jbank[name]=val;
                }
            }
            jline["bank"]=jbank;
            jpayment[idx] = jline;
            if (!paymentCalculate(jbank, jpayment[idx]))
                return false;

            idx++;
            db->sql_next ();
        } while (!db->eof ());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         paymentCalculate
// DESCRIPTION:         Calculate the payment
//*******************************************************************
bool                    CYRestCommon::paymentCalculate(Json::Value jbank, Json::Value& jpayment)
{
    //////////////////////////////////////
    //  Quantity / price
    string change = jpayment["change_amount"].asString();
    string payamt = jpayment["payment_amount"].asString();
    string credit_balance = jpayment["credit_balance"].asString ();
    if (!_util->valid_decimal("Payment", payamt.c_str (), 1, 99, -999999999, 999999999))
        return seterrormsg(_util->errormsg());
    double change_amount =  _util->stodsafe(change);
    double payment_amount = _util->stodsafe(payamt);
    Json::Value tendertype = jpayment["tendertype"];
    //////////////////////////////////////
    //  Calculate the payment discount???
    double discount=0.00;
    bool found = false;
    if (jpayment["settings_payment_discount"] != Json::nullValue)  {
        double discountValue = 0.00;
        Json::Value jdiscount = jpayment["settings_detail_discount"];
        bool discountPercentage = jdiscount["disctype"].asString() == "1";
        //  Validate the discount value
        string tmp = jdiscount["discvalue"].asString();
        if (_util->valid_decimal("Discount value", tmp.c_str (), 1, 99, 0, 999999999))  {
            discountValue = _util->stodsafe(tmp);
        }  else  {
            return seterrormsg(_util->errormsg());
        }
        //  Calculate the discount
        discount = 0.00;
        if (discountPercentage)  {
            discountValue = (discountValue / 100);
            discount = payment_amount * discountValue;
        }  else  {
            discount = discountValue;
        }
        //  Update totals
        _totalPaymentDiscount += discount;
        //////////////////////////////
        //  EOD details
        struct_discount* strDiscount = new struct_discount;

        strDiscount->type = DISC_PAYMENT;
        strDiscount->amount = discount;
        strDiscount->discount_desc = jdiscount["description"].asString();
        strDiscount->discount_code = jdiscount["discount_cd"].asString();
        strDiscount->discount_type_code = jdiscount["discount_type_cd"].asString();

        found = false;
        size_t j = _xread_discount.size();
        for (size_t i = 0; i < j; i++)  {
            struct_discount* disc = _xread_discount.at(i);
            if (disc->discount_code == strDiscount->discount_code &&
                disc->discount_type_code == strDiscount->discount_type_code)  {
                found = true;
                disc->amount += discount;
            }
        }
        if (!found)
            _xread_discount.push_back(strDiscount);
        else
            delete (strDiscount);        
    }
    //////////////////////////////
    //  EOD details
    if (jbank != Json::nullValue)  {
        struct_bank* strBank = new struct_bank;

        strBank->bank_code = jbank["bank_code"].asString();
        strBank->bank_desc = jbank["description"].asString();
        strBank->amount = payment_amount - discount;

        found = false;
        size_t j = _xread_bank.size();
        for (size_t i = 0; i < j; i++)  {
            struct_bank* bank = _xread_bank.at(i);
            if (bank->bank_code == strBank->bank_code)  {
                found = true;
                bank->amount += (payment_amount - discount);
            }
        }
        if (!found)
            _xread_bank.push_back(strBank);
        else
            delete (strBank);
    }
    //////////////////////////////
    //  EOD details
    struct_payment* strPay = new struct_payment;

    strPay->cash = tendertype["is_cash"].asString() == "1";
    strPay->amount = payment_amount - change_amount;
    strPay->tender_code = jpayment["pay_code"].asString();
    strPay->tender_desc = jpayment["description"].asString();
    strPay->tender_type_code = jpayment["pay_type_code"].asString();
    strPay->tender_type_desc = jpayment["pay_type_desc"].asString();

    found = false;
    size_t j = _xread_payment.size();
    for (size_t i = 0; i < j; i++)  {
        struct_payment* pay = _xread_payment.at(i);
        if (pay->tender_type_code == strPay->tender_type_code &&
            pay->tender_code == strPay->tender_code)  {
            found = true;
            pay->amount += (payment_amount - discount);
        }
    }
    if (!found)
        _xread_payment.push_back(strPay);
    else
        delete (strPay);
    return true;
}
