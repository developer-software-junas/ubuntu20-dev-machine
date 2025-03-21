//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptrcc.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYReceiptRcc::assembleReceiptTotals (Json::Value& jheader,
                                          salesReceiptType receiptType)
{
    _txttot = "";
    string tmp2 = "",
           tmp = "";
    int w = g_widthReceipt;
    Json::Value jhdrtrx = jheader["settings_transtype"];
    string trxcode = jhdrtrx["transaction_cd"].asString();
    //////////////////////////////////////
    //  Prep work
    string trxDiscountDesc = "";
    if (_totalTrxDiscount > 0)  {
        Json::Value jtrx = jheader["settings_transaction_discount"];
        trxDiscountDesc = jtrx["description"].asString ();
    }
    //////////////////////////////////////
    //  Normalize
    if (_totalTrxDiscount < 0.00)  _totalTrxDiscount = _totalTrxDiscount * -1;
    if (_totalGrossAmount < 0.00)  _totalGrossAmount = _totalGrossAmount * -1;
    if (_totalItemDiscount < 0.00)  _totalItemDiscount = _totalItemDiscount * -1;
    //////////////////////////////////////
    //  Detail total(s)
    _txttot += _util->cyAcrossText(w, "=");

    //TOTAL SALES AMOUNT
    double totSalesAmt = 0.00;
    if (_totalItemDiscount > 0)
        totSalesAmt = (_totalGrossAmount-_totalItemDiscount);
    else
        totSalesAmt = _totalGrossAmount;
    tmp = FMTNumberComma(totSalesAmt);
    _txttot += _util->cyLRText(23, "Total Sales Amount", 18, tmp.c_str());

    if (_isPwd || _isSenior)  {
        //  Transaction discount, except fine jewelry
        if (_totalTrxDiscount > 0 && trxcode != "44")  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp);
        }

        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtExempt != 0.00)  {
            tmp = FMTNumberComma(_totalAmtExempt);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else if (_isZeroRated)  {
        //  Transaction discount
        if (_totalTrxDiscount > 0 && trxcode != "44")  {
            tmp = FMTNumberComma(_totalTrxDiscount);
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());
        //  VAT exempt
        if (_totalAmtZero != 0.00)  {
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }  else  {
        //  Transaction discount
        if (_totalTrxDiscount > 0 && trxcode != "44")  {
            tmp2 = FMTNumberComma(_totalTrxDiscount);
            if (receiptType == SRT_POSTVOID)
                tmp = "-";
            else
                tmp = "";
            tmp += tmp2;
            _txttot += _util->cyLRText(23,trxDiscountDesc,18,tmp.c_str());
        }
        //  Net sales amount
        double netSalesAmount = _totalGrossAmount -
                (_totalItemDiscount+_totalTrxDiscount+_totalAmtVat);
        tmp = FMTNumberComma(netSalesAmount);
        _txttot += _util->cyLRText(23,"Net Sales Amount",18,tmp.c_str());

        //  VAT exempt
        if (_totalAmtZero > 0)  {
            //  VAT exempt
            tmp = FMTNumberComma(_totalAmtZero);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }  else  {
            tmp = FMTNumberComma(_totalAmtVat);
            _txttot += _util->cyLRText(23,"VAT 12%",18,tmp.c_str());
        }
        //  Amount due
        double amtDue = _totalGrossAmount -
                (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
        tmp = FMTNumberComma(amtDue);
        _txttot += _util->cyLRText(23,"Total Amount Due",18,tmp.c_str());
    }
    _receipt += _txttot;

    return true;
}
