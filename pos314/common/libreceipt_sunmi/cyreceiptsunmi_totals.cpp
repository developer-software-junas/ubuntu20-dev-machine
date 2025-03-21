//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptDetailTotals
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptSunmi::assembleReceiptTotals (Json::Value& jheader,
                                           salesReceiptType receiptType)
{
     _txttot = "";
     string tmp2 = "", tmp = "";
     int w = 30;
     //////////////////////////////////////
     //  Prep work
     string trxDiscountDesc = "";
     if (_totalTrxDiscount > 0)  {
         Json::Value jtrx = jheader["settings_transaction_discount"];
         trxDiscountDesc = jtrx["description"].asString ();
     }
     //////////////////////////////////////
     //  Detail total(s)
     _txttot += _util->cyAcrossText(w, "=");

     //TOTAL SALES AMOUNT
     if (receiptType != SRT_POSTVOID){
          _totalGrossAmount = (_totalGrossAmount*100) / 100;

          tmp = FMTNumberComma(_totalGrossAmount);
          _txttot += _util->cyLRText(18, "Total Sales Amount", 12, tmp.c_str());

          _totalItemDiscount = (_totalItemDiscount*100) / 100;

         if (_totalItemDiscount != 0)  {
             tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
             tmp = FMTNumberComma(_totalItemDiscount);
             _txttot += _util->cyLRText(18, "Total Item Discount", 12, tmp.c_str());
         }  else
             tmp = FMTNumberComma(_totalGrossAmount);
     } else {
         _totalGrossAmount = (_totalGrossAmount*100) / 100;
         tmp = FMTNumberComma(_totalGrossAmount);
         _txttot += _util->cyLRText(18, "Total Sales Amount", 12, tmp.c_str());


         _totalItemDiscount = (_totalItemDiscount*100) / 100;
         if (_totalItemDiscount > 0)  {
             tmp = FMTNumberComma(_totalGrossAmount-_totalItemDiscount);
             tmp = FMTNumberComma(_totalItemDiscount);
             _txttot += _util->cyLRText(18, "Total Item Discount", 12, tmp.c_str());
         }  else  {
             tmp = FMTNumberComma(_totalGrossAmount);
         }


     }
     //_txttot += _util->cyLRText(23, "Total Sales Amount", 18, tmp.c_str());
     //////////////////////////////////////
     //  Sequence change based on type
     char prefix[2];

     if (receiptType == SRT_REFUND)  {
         strcpy (prefix,"-");
     }  else  {
         strcpy(prefix,"");
     }


     if (_isPwd || _isSenior)  {
         //  Transaction discount
         if (_totalTrxDiscount > 0)  {
             tmp = FMTNumberComma(_totalTrxDiscount);
             _txttot += _util->cyLRText(18,trxDiscountDesc,12,tmp);
         }

         //  Net sales amount
         double netSalesAmount = _totalGrossAmount -
                 (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
         tmp = FMTNumberComma(netSalesAmount);
         _txttot += _util->cyLRText(18,"Net Sales",12,tmp.c_str());
         //  VAT exempt
         if (_totalAmtExempt != 0.00)  {
             tmp = FMTNumberComma(_totalAmtExempt);
             _txttot += _util->cyLRText(18,"VAT 12%",12,tmp.c_str());
         }
         //  Amount due
         _totalAmtDue = _totalGrossAmount -
                 (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
         tmp = FMTNumberComma(_totalAmtDue);
         _txttot += _util->cyLRText(18,"Total Amount",12,tmp.c_str());
     }  else if (_isZeroRated)  {
         //  Transaction discount
         if (_totalTrxDiscount > 0)  {
             tmp = FMTNumberComma(_totalTrxDiscount);
             _txttot += _util->cyLRText(18,trxDiscountDesc,12,tmp.c_str());
         }
         //  Net sales amount
         double netSalesAmount = _totalGrossAmount -
                 (_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
         tmp = FMTNumberComma(netSalesAmount);
         _txttot += _util->cyLRText(18,"Net Sales",12,tmp.c_str());
         //  VAT exempt
         if (_totalAmtZero != 0.00)  {
             tmp = FMTNumberComma(_totalAmtZero);
             _txttot += _util->cyLRText(18,"VAT 12%",12,tmp.c_str());
         }
         //  Amount due
         _totalAmtDue = _totalGrossAmount -
                 (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount+_totalAmtZero);
         tmp = FMTNumberComma(_totalAmtDue);
         _txttot += _util->cyLRText(18,"Total Amount",12,tmp.c_str());
     }  else  {
         //  Transaction discount
         if (_totalTrxDiscount > 0)  {
             tmp = FMTNumberComma(_totalTrxDiscount);
             _txttot += _util->cyLRText(18,trxDiscountDesc,12,tmp.c_str());
         }
         //  Net sales amount
         double netSalesAmount = _totalGrossAmount -
                 (_totalItemDiscount+_totalTrxDiscount+_totalAmtVat);
         tmp = FMTNumberComma(netSalesAmount);
         _txttot += _util->cyLRText(18,"Net Sales",12,tmp.c_str());

         //  VAT exempt
         if (_totalAmtZero > 0)  {
             //  VAT exempt
             tmp = FMTNumberComma(_totalAmtZero);
             _txttot += _util->cyLRText(18,"VAT 12%",12,tmp.c_str());
         }  else  {
             tmp = FMTNumberComma(_totalAmtVat);
             _txttot += _util->cyLRText(18,"VAT 12%",12,tmp.c_str());
         }
         //  Amount due
         _totalAmtDue = _totalGrossAmount -
                 (_totalAmtExempt+_totalItemDiscount+_totalTrxDiscount);
         tmp = FMTNumberComma(_totalAmtDue);
         if (_totalServiceCharge > 0.00)
             _txttot += _util->cyLRText(18,"Amount Due",12,tmp.c_str());
         else
             _txttot += _util->cyLRText(18,"Total Amount",12,tmp.c_str());
     }
     if (_totalServiceCharge > 0.00)  {
         tmp = FMTNumberComma(_totalServiceCharge);
         _txttot += _util->cyLRText(18,"Svc Charge",12,tmp.c_str());
         tmp = FMTNumberComma(_totalAmtDue +_totalServiceCharge);
         _txttot += _util->cyLRText(18,"Total Amount",12,tmp.c_str());
     }
     _receipt += _txttot;

     return true;
}
