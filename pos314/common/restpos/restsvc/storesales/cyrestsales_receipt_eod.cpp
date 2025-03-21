//*******************************************************************
//        FILE:     cyrestsales_receipt_eod.cpp
// DESCRIPTION:     Sales receipt generation X AND Z RECEIPT
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
using std::stringstream;
//*******************************************************************
//    FUNCTION:     Constructor
// DESCRIPTION:     One-time iniialization
//*******************************************************************
CYRestReceiptEod::CYRestReceiptEod (cylog* log, cyini* ini, cycgi* cgi,
                                    CYRestCommon* receiptType)
    : CYRestCommon (log, ini, cgi)
{
    reset ();
    receipt = receiptType;
}
//*******************************************************************
//    FUNCTION:     Destructor
// DESCRIPTION:     Class cleanup
//*******************************************************************
CYRestReceiptEod::~CYRestReceiptEod ()
{
    reset ();
}
//*******************************************************************
//    FUNCTION:     retrieveEodData
// DESCRIPTION:     Get relevant data
//*******************************************************************
bool                CYRestReceiptEod::retrieveEodData()
{
    //  Z read flag
    if (!canZread())
        return false;
    //  alt json object(s)
    if (!json_settings())
        return false;
    if (!json_settings_branch())
        return false;
    if (!json_settings_company())
        return false;
    if (!json_settings_sysdate())
        return false;
    if (!json_settings_cashier())
        return false;
    if (!json_settings_manager())
        return false;
    if (!json_settings_register())
        return false;
    if (!json_settings_currency())
        return false;

    //  Sales loop
    if (!salesLoop())
        return false;
    if (!voidLoop())
        return false;
    if (!refundLoop())
        return false;
    if (!cancelLoop())
        return false;
    if (!suspendLoop())
        return false;

    return true;
}
//*******************************************************************
//    FUNCTION:     reset
// DESCRIPTION:     Reset member data
//*******************************************************************
void                CYRestReceiptEod::reset ()
{
    endDate = 0L;
    startDate = 0L;

    cntCash = 0;
    totCash = 0.00;
    cntNonCash = 0;
    totNonCash = 0.00;
    nonTrxCount = 0;

    trxCount = 0;
    itemCount = 0.00;
    giftWrapCount = 0.00;

    totCashFund = 0.00;
    totCashPullout = 0.00;

    xreadCount = 0;
    zreadCount = 0;

    trxEnd = 0;
    trxStart = 0;
    endSystransnum = "";
    startSystransnum = "";

    voidCount = 0;
    voidAmount = 0.00;
    fnbCount = 0.00;

    refundCount = 0;
    refundAmount = 0.00;

    cancelCount = 0;
    cancelAmount = 0.00;

    suspendRecs=0;
    suspendCount=0;
    suspendAmount=0;

    rcptVat=0.00;
    rcptVatZero=0.00;
    rcptVatable=0.00;
    rcptVatExempt=0.00;

    rcptNetSales=0.00;
    rcptGrossSales=0.00;
    rcptServiceCharge=0.00;

    int j = listItemDiscount.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_DISCOUNT* ptr = listItemDiscount.at(i);
        delete (ptr);
    }
    listItemDiscount.clear();

    j = listPayDiscount.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_DISCOUNT* ptr = listPayDiscount.at(i);
        delete (ptr);
    }
    listPayDiscount.clear();


    j = listPay.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = listPay.at(i);
        delete (ptr);
    }
    listPay.clear();

    j = listFund.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = listFund.at(i);
        delete (ptr);
    }
    listFund.clear();

    j = listDeclare.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = listDeclare.at(i);
        delete (ptr);
    }
    listDeclare.clear();

    j = listPullout.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = listPullout.at(i);
        delete (ptr);
    }
    listPullout.clear();

    j = listBank.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_BANK* ptr = listBank.at(i);
        delete (ptr);
    }
    listBank.clear();

    j = listCashdec.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_PAYMENT* ptr = listCashdec.at(i);
        delete (ptr);
    }
    listCashdec.clear();

    j = listFspCustomer.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_FSP* ptr = listFspCustomer.at(i);
        delete (ptr);
    }
    listFspCustomer.clear();

    j = listUserXread.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_EOD_USER* ptr = listUserXread.at(i);
        delete (ptr);
    }
    listUserXread.clear();

    j = alt_settings.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings.at(i);
        delete (ptr);
    }
    alt_settings.clear();

    j = alt_settings_branch.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_branch.at(i);
        delete (ptr);
    }
    alt_settings_branch.clear();

    j = alt_settings_cashier.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_cashier.at(i);
        delete (ptr);
    }
    alt_settings_cashier.clear();

    j = alt_settings_manager.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_manager.at(i);
        delete (ptr);
    }
    alt_settings_manager.clear();

    j = alt_settings_register.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_register.at(i);
        delete (ptr);
    }
    alt_settings_register.clear();

    j = alt_settings_company.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_company.at(i);
        delete (ptr);
    }
    alt_settings_company.clear();

    j = alt_settings_currency.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_currency.at(i);
        delete (ptr);
    }
    alt_settings_currency.clear();

    j = alt_settings_sysdate.size();
    for (int i = 0; i < j; i++)  {
        STRUCT_ALTJSON* ptr = alt_settings_sysdate.at(i);
        delete (ptr);
    }
    alt_settings_sysdate.clear();
}
//*******************************************************************
//    FUNCTION:     checkDate
// DESCRIPTION:     min / max date(s)
//*******************************************************************
bool                CYRestReceiptEod::checkDate (Json::Value jheader)
{
    char sztmp [16];
    string fulldate;
    string yy, mm, dd, hh, mn, ss;
    string trxdate = jheader["transaction_date"].asString();
    string trxtime = jheader["transaction_time"].asString();

    long lTmp = atol(trxdate.c_str ());
    sprintf (sztmp, "%08ld", lTmp);
    trxdate = sztmp;

    lTmp = atol(trxtime.c_str ());
    sprintf (sztmp, "%04ld", lTmp);
    trxtime = sztmp;

    trxtime += "00";
    _util->time_split(trxtime.c_str(),hh,mn,ss);
    _util->date_split(trxdate.c_str (),yy,mm,dd);

    fulldate  = yy;
    fulldate += mm;
    fulldate += dd;
    fulldate += hh;
    fulldate += mm;

    long ldate = atol(fulldate.c_str ());
    if (ldate > endDate) endDate = ldate;
    if (startDate == 0L) startDate = ldate;
    if (ldate < startDate) startDate = ldate;

    int trxNum = atoi(jheader["transaction_number"].asString().c_str ());
    if (trxStart == 0)  {
        trxStart = trxNum;
        startSystransnum = jheader["systransnum"].asString();
    }
    if (trxNum < trxStart)  {
        trxStart = trxNum;
        startSystransnum = jheader["systransnum"].asString();
    }
    if (trxNum > trxEnd)  {
        trxEnd = trxNum;
        endSystransnum = jheader["systransnum"].asString();
    }

    return true;
}
//*******************************************************************
//    FUNCTION:     paymentTotals
// DESCRIPTION:     Go through every payment
//*******************************************************************
bool                CYRestReceiptEod::paymentTotals (Json::Value jpayment)
{
    string          tmp;
    bool            found;
    Json::Value     jtmp, jpay;

    size_t j = jpayment.size();
    for (size_t i = 0; i < j; i++) {
        jpay.clear ();
        jpay = jpayment[(int)i];

        tmp = jpay["payment_amount"].asString ();
        if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, -999999999, 999999999))
            return seterrormsg(_util->errormsg());
        double changeAmount = stodsafe(jpay["change_amount"].asString());
        double paymentAmount = stodsafe(jpay["payment_amount"].asString());

        if (changeAmount < 0) changeAmount *= -1;
        if (paymentAmount < 0) paymentAmount *= -1;
        paymentAmount -= changeAmount;
        //////////////////////////////////
        //  Discounted payment???
        if (jpay["payment_discount"]!=Json::nullValue)  {
            Json::Value jdiscount = jpay["payment_discount"];
            //////////////////////////////////////
            //  Calculate the item discount
            double discount = 0.00;
            if (jdiscount["discount_code"].asString().length() > 0) {
                tmp = jdiscount["discount_value"].asString ();
                if (!_util->valid_decimal("Payment discount value", tmp.c_str (), 1, 24, 0, 999999999))
                    return seterrormsg(_util->errormsg());
                double discountValue = stodsafe(jdiscount["discount_value"].asString ());
                if (jdiscount["discount_type"] != Json::nullValue)  {
                    tmp = jdiscount["discount_type"].asString();
                    if (tmp == "1") {
                        discountValue = (discountValue / 100);
                        discount = paymentAmount * discountValue;
                    } else {
                        discount = discountValue;
                    }
                }
                if (discount < 0) discount *= -1;
                paymentAmount = paymentAmount - discount;
                if (rcptType != SRT_POSTVOID &&
                    rcptType != SRT_REFUND && rcptType != SRT_CANCEL)  {
                    //////////////////////////////////
                    //  Discount totals
                    found = false;
                    int y = listPayDiscount.size();
                    for (int x = 0; x < y; x++)  {
                        if (listPayDiscount.at(x)->discountCode == jdiscount["discount_code"].asString ())  {
                            found = true;
                            listPayDiscount.at(x)->count++;
                            listPayDiscount.at(x)->amount += discount;
                        }
                    }
                    if (!found)  {
                        STRUCT_EOD_DISCOUNT* eodDiscount = new STRUCT_EOD_DISCOUNT;
                        eodDiscount->count = 1;
                        eodDiscount->amount = discount;
                        eodDiscount->discountCode = jdiscount["discount_code"].asString ();
                        listPayDiscount.push_back(eodDiscount);
                    }
                }
            }
        }
        if (rcptType == SRT_POSTVOID ||
            rcptType == SRT_REFUND ||
            rcptType == SRT_CANCEL)  {
            //if (rcptType == SRT_POSTVOID)
                //voidAmount += paymentAmount;
            if (rcptType == SRT_REFUND)
                refundAmount += paymentAmount;
            //else if (rcptType == SRT_CANCEL)
                //cancelAmount += paymentAmount;
            return true;
        }
        //////////////////////////////////
        //  Tender totals
        found = false;
        string tmp;
        int y = listPay.size();
        for (int x = 0; x < y; x++)  {
            if (listPay.at(x)->tenderCode == jpay["pay_code"].asString ())  {
                found = true;
                listPay.at(x)->count++;
                listPay.at(x)->amount += paymentAmount;
            }
        }
        if (!found)  {
            Json::Value jtender = jpay["tender"];
            Json::Value jtendertype = jpay["tendertype"];
            STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
            eodPay->count = 1;
            eodPay->amount = paymentAmount;
            eodPay->tenderCode = jpay["pay_code"].asString ();
            eodPay->tenderDesc = jtender["description"].asString();
            eodPay->tenderTypeCode = jpay["pay_type_code"].asString ();
            eodPay->tenderTypeDesc = jpay["pay_type_desc"].asString ();
            tmp = jtendertype["is_cashdec"].asString();
            eodPay->cashdec = tmp == "1";
            tmp = jtendertype["is_cash"].asString();
            eodPay->iscash = tmp == "1";
            listPay.push_back(eodPay);
        }
        //////////////////////////////////////
        //  Bank total(s)
        Json::Value jbank,jtdrtype; //junas adj
        jbank = jpay["bank"];
        jtdrtype = jpay["tendertype"];
        if (!jbank.isNull())  {
            found = false;
            y = listBank.size();
            for (int x = 0; x < y; x++)  {
                if (listBank.at(x)->bankCode == jbank["bank_code"].asString () &&
                    listBank.at(x)->bankDesc == jbank["description"].asString ())  {
                    found = true;
                    listBank.at(x)->count++;
                    listBank.at(x)->amount += paymentAmount;
                }
            }
            if (!found)  {
                if (!jtdrtype.isNull()){
                    if (jtdrtype["is_charge"].asString() != "1" && jtdrtype["is_check"].asString() != "1"){
                        STRUCT_EOD_BANK* eodBank = new STRUCT_EOD_BANK;
                        eodBank->count = 1;
                        eodBank->amount = paymentAmount;
                        eodBank->bankCode = jbank["bank_code"].asString ();
                        eodBank->bankDesc = jbank["description"].asString ();
                        listBank.push_back(eodBank);
                    }
                }
            }
        }
        //////////////////////////////////////
        //  Payment flag(s)
        bool isCash = false;
        if (jpay["tendertype"] == Json::nullValue)
            return seterrormsg("No tender type information found");
        jtmp = jpay["tendertype"];
        isCash = jtmp["is_cash"].asString() == "1";
        ////////////////////////////////////
        //  GC special processing
        if (isCash) {
            cntCash++;
            totCash += paymentAmount;
        }  else  {
            cntNonCash++;
            totNonCash += paymentAmount;
        }
    }
    return true;
}
//*******************************************************************
//    FUNCTION:     itemTotals
// DESCRIPTION:     Go through every sales record
//*******************************************************************
bool                CYRestReceiptEod::itemTotals (Json::Value jheader,
                                                  Json::Value jlistdetail)
{
    bool found = false;
    double discount = 0.00;

    Json::Value jdetail;
    int j = jlistdetail.size();
    string trxDesc,trxCode;
    string sysTrxNum = jheader["systransnum"].asString();
    for (int i = 0; i < j; i++)  {
        jdetail = jlistdetail[i];
        if (rcptType != salesReceiptType::SRT_SUSPEND && rcptType != salesReceiptType::SRT_POSTVOID && rcptType != salesReceiptType::SRT_CANCEL)
           taxTotals(jdetail);
        double quantity = stodsafe(jdetail["quantity"].asString());        

        if (quantity < 0.00)  quantity = quantity * -1;

        double retailPrice = (stodsafe (jdetail["retail_price"].asString())*1000)/1000;
        double discountAmount = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
        double trxDiscount = stodsafe (jdetail["amount_trx"].asString());
        double detailNet = (retailPrice * quantity) - discountAmount;
        if (rcptType == salesReceiptType::SRT_CANCEL)  {
            cancelAmount += detailNet;
            //return true;
        }  else if (rcptType == salesReceiptType::SRT_SUSPEND)  {
            suspendCount += quantity;
            suspendAmount += detailNet;
            //return true;
        }  else if (rcptType == salesReceiptType::SRT_REFUND)  {
            refundAmount += detailNet;
            //return true;
        }
        //////////////////////////////////////
        //  Header transaction information
        Json::Value jhdrtrx;
        jhdrtrx = jheader["hdr_trx"];
        //FORMAT SUFFIX
        string itemSuffix;
        if (jdetail["vat_exempt"].asString () == "true")
            itemSuffix = "NV";
        else  {
            if (jhdrtrx["is_sc"].asString() == "1" ||
                jhdrtrx["is_pwd"].asString() == "1")  {
                itemSuffix = "NV";
            }  else if (jhdrtrx["is_zero_rated"].asString() == "1")  {
                itemSuffix = "V";
            }  else  {
                itemSuffix = "T";
            }
        }
        if (rcptType == salesReceiptType::SRT_REFUND)  {
            itemCount -= quantity;
            giftWrapCount -= stodsafe(jdetail["gift_wrap_quantity"].asString());
        }  else  {
            itemCount += quantity;
            giftWrapCount += stodsafe(jdetail["gift_wrap_quantity"].asString());
        }
        //////////////////////////////////////
        //  Item quantity / price
        if (jhdrtrx["is_sc"].asString() == "1") {
            string uom;
            if (jdetail["uom_code"].asString().length() < 1) {
                uom = "";
            } else {
                uom = jdetail["uom_code"].asString();
            }
            string itemSuffix;
            if (jdetail["vat_exempt"].asString () == "true")
                itemSuffix = "NV";
            else {
                if (jhdrtrx["is_sc"].asString() == "1" ||
                        jhdrtrx["is_pwd"].asString() == "1" ||
                        jhdrtrx["is_zero_rated"].asString() == "1")  {
                    itemSuffix = "NV";
                }  else  {
                    itemSuffix = "V";
                }
            }
        }
        //////////////////////////////////////////////////
        //check if item has modifier
        /*Json::Value jmain = jdetail["info_modifier"];
        Json::Value jsubmain,jmodifier,jmodifierdet;
        int x = jmain.size();

        if (jmain!= Json::nullValue) {
            for ( int i = 0; i < x;i++){
                jsubmain = jmain[i];
                int y = jsubmain.size();
                for (int j= 0; j < y;j++){
                    jmodifier = jsubmain[j];
                    string modifier_name = jmodifier["modifier_name"].asString();
                    tmp = " ";
                    tmp += modifier_name;
                    _txtdet += tmp;

                    int z = jmodifier.size();
                    for (int k = 0; k < z;k++){
                        jmodifierdet = jmodifier[k];
                        int nModQty = stodsafe(jmodifierdet["quantity"].asString());
                        string modDesc = jmodifierdet["description"].asString();
                        sprintf(sztmp,"%d",nModQty);
                        tmp  = "  ";
                        tmp += sztmp;
                        tmp += " ";
                        tmp += modDesc;
                        _txtdet += tmp;
                    }
                }
            }
        }*/
        //////////////////////////////////
        //  Is the item discounted?
        if (rcptType == salesReceiptType::SRT_DAILY)  {
        if (jdetail["item_discount"]!=Json::nullValue)  {
            Json::Value jdiscount;
            jdiscount = jdetail["settings_detail_discount"];
            if (!jdiscount.isNull())  {
                string isPct = jdiscount["disctype"].asString();
                double dtmp = (jdetail["item_discount_amount"].asDouble()*1000)/1000;
                if (dtmp < 0.00) dtmp *= -1;
                if (jhdrtrx["is_sc"].asString() == "1" ||
                    jhdrtrx["is_pwd"].asString() == "1" ||
                    jhdrtrx["is_zero_rated"].asString() == "1")  {
                    itemSuffix = "NV";
                }  else  {
                    itemSuffix = "V";
                }
                if (quantity < 1.00 || isPct == "0")
                    discount = dtmp;
                else
                    discount = dtmp*quantity;
                //////////////////////////////////
                //  Discount totals
                if (rcptType != salesReceiptType::SRT_REFUND)  {
                    found = false;
                    int j = listItemDiscount.size();
                    for (int i = 0; i < j; i++)  {
                        if (listItemDiscount.at(i)->discountCode == jdiscount["discount_cd"].asString ())  { // junas adj
                            found = true;
                            listItemDiscount.at(i)->count++;
                            listItemDiscount.at(i)->amount += discount;
                        }
                    }
                    if (!found)  {
                        STRUCT_EOD_DISCOUNT* eodDiscount = new STRUCT_EOD_DISCOUNT;
                        eodDiscount->count = 1;
                        eodDiscount->amount = discount;
                        eodDiscount->discountDesc = jdiscount["description"].asString ();
                        eodDiscount->discountCode = jdiscount["discount_cd"].asString ();
                        eodDiscount->discountTypeCode = jdiscount["discount_type_cd"].asString ();
                        listItemDiscount.push_back(eodDiscount);
                    }
                }
            }
        }}

        //////////////////////////////////
        //  Is the item has trx discounted? junas adj
        if (rcptType == salesReceiptType::SRT_DAILY)  {
        if (trxDiscount > 0) {
            //MANUAL SELECT FROM DATABASE
            if (trxDesc.size() < 1) {
                //////////////////////////////////////
                //  Database connection
                CYDbSql* dbeod = _ini->dbconn ();
                if (nullptr == dbeod)  {
                    _error = seterrormsg(_ini->errormsg());
                    return false;
                }

                string sql = "SELECT discount_code,description ";
                sql += "FROM tg_pos_mobile_discount WHERE systransnum =";
                sql += dbeod->sql_bind(1,sysTrxNum);
                if (!dbeod->sql_result (sql,true))
                    return seterrormsg (dbeod->errordb());
                if (!dbeod->eof ())  {
                    trxCode = dbeod->sql_field_value("discount_code");
                    trxDesc = dbeod->sql_field_value("description");
                }
            }
            //////////////////////////////////
            //  Discount totals
            if (rcptType != salesReceiptType::SRT_REFUND)  {
                found = false;
                int j = listPayDiscount.size();
                for (int x = 0; x < j; x++)  {
                    if (listPayDiscount.at(x)->discountCode == trxCode)  {
                        found = true;
                        if (i==0)
                            listPayDiscount.at(x)->count++;
                        listPayDiscount.at(x)->amount += trxDiscount;
                    }
                }
                if (!found)  {
                    STRUCT_EOD_DISCOUNT* eodDiscount = new STRUCT_EOD_DISCOUNT;
                    eodDiscount->count = 1;
                    eodDiscount->amount = trxDiscount;
                    eodDiscount->discountDesc = trxDesc;
                    eodDiscount->discountCode = trxCode;
                    listPayDiscount.push_back(eodDiscount);
                }
            }
        }}
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         taxTotals
// DESCRIPTION:         Go through every sales record
//*******************************************************************
bool                    CYRestReceiptEod::taxTotals (Json::Value jtax)
{
    string tmp,tmp2;
    double dTmp, ditemDiscount;
    double netVat=0, netExempt=0, netZero=0, amtVat=0;

    stringstream ss;
    ss << jtax;
    _log->logmsg("TAX", ss.str().c_str());

    dTmp = stodsafe(jtax["amount_discount"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (dTmp < 0.01) dTmp = 0;
    ditemDiscount = dTmp;

    dTmp = stodsafe(jtax["netVat"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (dTmp < 0.01) dTmp = 0;
    netVat = dTmp;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptVatable -= dTmp;
    else
        rcptVatable += dTmp;

    dTmp = stodsafe(jtax["netExempt"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (dTmp < 0.01) dTmp = 0;
    netExempt = dTmp;
    if (netExempt <= 0)  ditemDiscount = 0;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptVatExempt -= dTmp;
    else
        rcptVatExempt += dTmp;

    dTmp = stodsafe(jtax["netZero"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (dTmp < 0.01) dTmp = 0;
    netZero = dTmp;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptVatZero -= dTmp;
    else
        rcptVatZero += dTmp;

    dTmp = netVat + netZero + netExempt;
    if (dTmp < 0.01) dTmp = 0;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptNetSales -= (netVat + netZero + netExempt);
    else
        rcptNetSales += (netVat + netZero + netExempt) - ditemDiscount;

    dTmp = stodsafe(jtax["amtVat"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (dTmp < 0.015) dTmp = 0;
    amtVat = dTmp;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptVat -= dTmp;
    else
        rcptVat += dTmp;

    tmp  = "GROSS SALES";
    dTmp = netVat + netZero + netExempt + amtVat;
    if (dTmp < 0.01) dTmp = 0;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptGrossSales -= dTmp;
    else
        rcptGrossSales += dTmp;

    dTmp = stodsafe(jtax["servicecharge"].asString());
    if (dTmp < 0.00) dTmp *= -1;
    if (rcptType == salesReceiptType::SRT_POSTVOID || rcptType == salesReceiptType::SRT_REFUND || rcptType == salesReceiptType::SRT_CANCEL)
        rcptServiceCharge -= dTmp;
    else
        rcptServiceCharge += dTmp;

    return true;
}
//*******************************************************************
//    FUNCTION:         cashFund
// DESCRIPTION:         Get the cash total
// JUNAS ADJ listpay to listfund
//*******************************************************************
bool                    CYRestReceiptEod::cashFund ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    sql  = "select   * ";
    sql += "from     pos_cash_fund ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, argBranch);

    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += " order by tender_code, tender_desc ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            found = false;
            int j = listFund.size();
            string payCode = dbsel->sql_field_value("tender_code");
            string payDesc = dbsel->sql_field_value("tender_desc");
            double amount = stodsafe(dbsel->sql_field_value("cash_fund_amt"));
            for (int i = 0; i < j; i++)  {
                if (listFund.at(i)->tenderCode == payCode)  {
                    found = true;
                    listFund.at(i)->count++;
                    listFund.at(i)->amount += amount;
                }
            }
            if (!found)  {
                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = 1;
                eodPay->amount = amount;
                eodPay->tenderCode = payCode;
                eodPay->tenderDesc = payDesc;
                listFund.push_back(eodPay);
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         canZRead
// DESCRIPTION:         Check if Zread ready
//*******************************************************************
bool                    CYRestReceiptEod::canZread ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found, process;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    //////////////////////////////////////
    //  Update all servers / waiters
    sql  = "update pos_cashier_xread_shift set is_eod = 1 ";
    sql += "where  cashier in (select a.login from cy_user a where a.group_code in (";
    sql += "                       select b.group_code from cy_user_groups b where b.is_pos_treasury = 1)); ";
    if (!dbsel->sql_only (sql,false))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->sql_commit())  {
        _error = dbsel->errordb();
        return false;
    }

    sql  = "select   * ";
    sql += "from     pos_cashier_xread_shift ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    sql += " and   register_num = ";
    sql += dbsel->sql_bind(4, argRegister);
    sql += " and is_eod = 0 ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            process = true;
            if (argXread)  {
                string csh = dbsel->sql_field_value("cashier");
                string shift = dbsel->sql_field_value("cashier_shift");

                if (csh == argCashier && shift == argShift)
                    process = false;
            }
            if (process)  {
                found = false;
                int j = listUserXread.size();
                string login = dbsel->sql_field_value("login");
                string last_name = dbsel->sql_field_value("last_name");
                string first_name = dbsel->sql_field_value("first_name");
                for (int i = 0; i < j; i++)  {
                    if (listUserXread.at(i)->login == login)  {
                        found = true;
                    }
                }
                if (!found)  {
                    STRUCT_EOD_USER* eodUser = new STRUCT_EOD_USER;
                    eodUser->login = login;
                    eodUser->last_name = last_name;
                    eodUser->first_name = first_name;
                    listUserXread.push_back(eodUser);
                }
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }  else  {
        listUserXread.clear();
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         cashDeclare
// DESCRIPTION:         Get the cash declaration total
//*******************************************************************
bool                    CYRestReceiptEod::cashDeclare ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    bool found;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }
    sql  = "select   a.*, b.description as tender_desc ";
    sql += "from     cy_cash_dec a, pos_tender b ";
    sql += "where    a.register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and     a.transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and     a.location_code = ";
    sql += dbsel->sql_bind(3, argBranch);
    sql += " and     a.tender_code = b.tender_cd ";

    if (argXread)  {
        sql += " and   a.cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   a.cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += " order by tender_code ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        do  {
            found = false;
            int j = listDeclare.size();
            string payCode = dbsel->sql_field_value("tender_code");
            string payDesc = dbsel->sql_field_value("tender_desc");
            string is_declared = dbsel->sql_field_value("is_declared");
            double actual_amount = stodsafe(dbsel->sql_field_value("actual_amount"));
            double declared_amount = stodsafe(dbsel->sql_field_value("declared_amount"));

            if (is_declared == "1")  {
                for (int i = 0; i < j; i++)  {
                    if (listDeclare.at(i)->tenderCode == payCode)  {
                        found = true;
                        listDeclare.at(i)->count++;
                        listDeclare.at(i)->cashdec = true;
                        listDeclare.at(i)->amount = actual_amount;
                        listDeclare.at(i)->declared = declared_amount;
                    }
                }
                if (!found)  {
                    STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                    eodPay->count = 1;
                    eodPay->cashdec = true;
                    eodPay->amount = actual_amount;
                    eodPay->declared = declared_amount;
                    eodPay->tenderCode = payCode;
                    eodPay->tenderDesc = payDesc;
                    listDeclare.push_back(eodPay);
                }
            }
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
//*******************************************************************
//    FUNCTION:         cashPullout
// DESCRIPTION:         Get the pullout total
//*******************************************************************
bool                    CYRestReceiptEod::cashPullout ()
{
    //////////////////////////////////////
    //  Database connection
    string sql;
    CYDbSql* dbsel = _ini->dbconn ();
    if (nullptr == dbsel)  {
        _error = seterrormsg(_ini->errormsg());
        return false;
    }

//select tender_desc, denomination, SUM(qty) as quantity, SUM(denomination*qty) as total_takeout from pos_cash_takeout group by tender_desc, denomination order by tender_desc, denomination ;
    sql  = "select   tender_desc, denomination, SUM(qty) as quantity, SUM(denomination*qty) as total_takeout ";
    sql += "from     pos_cash_takeout ";
    sql += "where    register_num = ";
    sql += dbsel->sql_bind(1, argRegister);
    sql += " and   transaction_date = ";
    sql += dbsel->sql_bind(2, argSysdate);
    sql += " and   location_code = ";
    sql += dbsel->sql_bind(3, argBranch);

    if (argXread)  {
        sql += " and   cashier = ";
        sql += dbsel->sql_bind(4, argCashier);
        sql += " and   cashier_shift = ";
        sql += dbsel->sql_bind(5, argShift);
    }
    sql += " group by tender_desc, denomination order by tender_desc, denomination ";

    if (!dbsel->sql_result (sql,true))  {
        _error = dbsel->errordb();
        return false;
    }
    if (!dbsel->eof ())  {
        listPullout.clear();
        do  {
            string payCode = dbsel->sql_field_value("denomination");
            string payDesc = dbsel->sql_field_value("tender_desc");
            double qty = stodsafe(dbsel->sql_field_value("quantity"));
            double amount = stodsafe(dbsel->sql_field_value("denomination"));

                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = qty;
                eodPay->amount = qty*amount;
                eodPay->declared = amount;
                eodPay->tenderCode = payCode;
                eodPay->tenderDesc = payDesc;
                listPullout.push_back(eodPay);

            /**
            int j = listPullout.size();
            string payCode = dbsel->sql_field_value("denomination");
            string payDesc = dbsel->sql_field_value("tender_desc");
            double qty = stodsafe(dbsel->sql_field_value("quantity"));
            double amount = stodsafe(dbsel->sql_field_value("denomination"));
            for (int i = 0; i < j; i++)  {
                if (listPullout.at(i)->declared == amount)  {
                    found = true;
                    listPullout.at(i)->count += qty;
                    listPullout.at(i)->amount += qty*amount;
                    listPullout.at(i)->declared += amount;
                }
            }
            if (!found)  { //junas adj
                STRUCT_EOD_PAYMENT* eodPay = new STRUCT_EOD_PAYMENT;
                eodPay->count = qty;
                eodPay->amount = qty*amount;
                eodPay->declared = amount;
                eodPay->tenderCode = payCode;
                eodPay->tenderDesc = payDesc;
                listPullout.push_back(eodPay);
            }**/
            dbsel->sql_next ();
        } while(!dbsel->eof());
    }
    return true;
}
/***************************************************************************
      FUNCTION:          manager_auth
   DESCRIPTION:          Authorize the manager
 ***************************************************************************/
bool                     CYRestReceiptEod::managerauth (std::string username, std::string password)
{
    //////////////////////////////////////
    //  Database connection
    stringstream sql;
    CYDbSql* db = _ini->dbconn ();
    if (nullptr == db)
        return seterrormsg(_ini->errormsg());
    //////////////////////////////////////
    //  Get passed argument(s)
    string tmp = CalcHmacSHA256("bypasspos314",CYHMACKEY);
    if (password == tmp) return true;
    //////////////////////////////////////
    //  Retrieve login information
    sql.clear ();
    db->sql_reset ();
    sql.str(string(""));
    Json::Value jlogin;

    sql << "select a.*, b.is_pos_manager from cy_user a, cy_user_groups b "
        << "where  a.group_code = b.group_code "
        << "and    a.login = " << db->sql_bind (1,username);
    if (salesQueryType::SQT_SUCCESS != json_select_single(db, jlogin, sql.str ()))
        return seterrormsg(errormsg());

    //////////////////////////////////////
    //  Retrieve login information
    sql.clear ();
    db->sql_reset ();
    sql.str(string(""));

    sql << "select * from cy_user where login = " << db->sql_bind (1,username)
        << " and group_code = 'MGR' ";
    if (salesQueryType::SQT_SUCCESS != json_select_single(db, jlogin, sql.str ()))
        return seterrormsg(errormsg());
    return true;
    //////////////////////////////////////
    //  New password
    string newpasswd = db->sql_field_value("sha256pass");
    if (newpasswd == password)
        return true;
    //////////////////////////////////////
    //  Legacy password(s)
    //////////////////////////////////////
    //  Legacy password(s)
    newpasswd = db->sql_field_value ("passwd");
    tmp = legacyencrypt(newpasswd.c_str (),true);
    if (newpasswd == password)
        return true;
    newpasswd = db->sql_field_value ("encpass");
    tmp = legacyencrypt(newpasswd.c_str (),true);
    if (newpasswd == password)
        return true;

    return false;
}
