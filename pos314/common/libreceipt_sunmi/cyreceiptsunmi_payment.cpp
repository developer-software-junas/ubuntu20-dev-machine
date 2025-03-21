//*******************************************************************
//        FILE:     cyrestsales_receipt.cpp
// DESCRIPTION:     Sales receipt generation
//*******************************************************************
#include "cyreceiptsunmi.h"
using std::string;
//*******************************************************************
//    FUNCTION:     assembleReceiptPayment
// DESCRIPTION:     Create a receipt
//*******************************************************************
bool CYRestReceiptSunmi::assembleReceiptPayment (Json::Value& jheader,
                                            Json::Value& jpayment,
                                            salesReceiptType receiptType)
{
     string tmp;
     char sztmp[32];
     bool isGc = false;
     bool isCash = false;
     //bool isDebit = false;
     bool isCharge = false;
     bool isCreditCard = false;
     bool isMobilePay = false;



     Json::Value jtmp, jpay;
     double totalChangeAmount = 0.00;
     double totalPaymentAmount = 0.00;
     size_t j = jpayment.size();
     for (size_t i = 0; i < j; i++) {
         jpay.clear ();
         jpay = jpayment[(int)i];

         tmp = jpay["payment_amount"].asString ();
         if (!_util->valid_decimal("Payment amount", tmp.c_str (), 1, 24, -999999999, 999999999))
             return seterrormsg(_util->errormsg());

         double changeAmount = stodsafe(jpay["change_amount"].asString());
         double paymentAmount = stodsafe(jpay["payment_amount"].asString());
         paymentAmount -= changeAmount;
         totalChangeAmount += changeAmount;
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
                 paymentAmount = paymentAmount - discount;
             }
         }
         totalPaymentAmount += paymentAmount;
         //////////////////////////////////////
         //  Special payment information
         isGc = false;
         isCash = false;
         //isDebit = false;
         isCharge = false;
         isCreditCard = false;

         if (jpay["tendertype"] == Json::nullValue)
             return seterrormsg("No tender type information found");
         jtmp = jpay["tendertype"];
         isGc = jtmp["is_gc"].asString() == "1";
         isCash = jtmp["is_cash"].asString() == "1";
         isCharge = jtmp["is_charge"].asString() == "1";
         //isDebit = jtmp["is_debit_card"].asString() == "1";
         isCreditCard = jtmp["is_credit_card"].asString() == "1";
         isMobilePay = jtmp["is_social_distancing"].asString() == "1";
         ////////////////////////////////////
         //  GC special processing
         if (isGc) {
             if (jpay["gc"] == Json::nullValue)
                 return seterrormsg("Unable to process the GC payment information");

             Json::Value gc, jgc = jpay["gc"];
             ////////////////////////////
             //  GC payment(s)
             if (isGc) {
                 sprintf (sztmp, "%.02f", paymentAmount);
                 _txtpay += _util->cyLRText(18, jpay["description"].asString(), 12, sztmp);
                 int j = jgc.size();
                 for (int i = 0; i < j; i++) {
                   gc.clear ();
                   gc = jgc[i];
                   _txtpay += gc["description"].asString();
                   //_txtpay += "\n";
                   _txtpay += _util->cyLRText(18, "  GC SERIES", 12, gc["gcnum"].asString());

                   double dtmp = stodsafe(gc["amount_paid"].asString());
                   sprintf (sztmp, "%.02f", dtmp);
                   _txtpay += _util->cyLRText(18, "  OLD BALANCE", 12, sztmp);
                   dtmp = stodsafe(gc["remaining_balance"].asString());
                   sprintf (sztmp, "%.02f", dtmp);
                   _txtpay += _util->cyLRText(18, "  NEW BALANCE", 12, sztmp);

                   if (dtmp > 0.00)  {
                       Json::Value jgcrefund = jpay["gcrefund"];

                   }
                 }
             }
         }  else  {
             string right = "0.00";
             if (receiptType == SRT_REFUND || receiptType == SRT_POSTVOID)  {
                 sprintf (sztmp, "%.02f", paymentAmount);
             } else  {
                 if (isCash)
                     sprintf (sztmp, "%.02f", (paymentAmount+changeAmount));
                 else
                     sprintf (sztmp, "%.02f", paymentAmount);
             }
             right = FMTNumberComma(atof(sztmp));

             string left = "";
             left += jpay["description"].asString();
             left += "(";
             left += jpay["pay_code"].asString();
             left += ")";
             _txtpay += _util->cyLRText(18, left, 12, right);

             if (receiptType == SRT_REFUND)  {
                 left  = "    RES NO.  : ";
                 left += jpay["approval_code"].asString ();
                 left += "\n";
                 _txtpay += left;

                 left  = "    NAME     : ";
                 left += jheader["first_name"].asString ();
                 left += " ";
                 left += jheader["last_name"].asString ();
                 left += "\n";
                 _txtpay += left;
             }
         }
         ////////////////////////////////////
         //  Credit card
         bool bextra = false;
         string headerText, lineDesc = "";
         if (isCreditCard) {
             bextra = true;
             int idxTerms = 0;
             lineDesc = "  TERMS ";
             tmp = jpay["approval_terms"].asString();
             if (!_util->valid_number("Payment terms", tmp, 1, 2))
                 jpay["approval_terms"]="0";
             idxTerms = atoi(tmp.c_str ());
             switch (idxTerms) {
               case 0:
                 headerText += "Straight";
                 break;
               case 1:
                 headerText += "3 months";
                 break;
               case 2:
                 headerText += "6 months";
                 break;
               case 3:
                 headerText += "9 months";
                 break;
               case 4:
                 headerText += "12 months";
                 break;
               case 5:
                 headerText += "18 months";
                 break;
               case 6:
                 headerText += "24 months";
                 break;
               case 7:
                 headerText += "36 months";
                 break;
             }
             headerText += " ";
             if (isCreditCard) {
               _txtpay += _util->cyLRText(18, lineDesc, 12, headerText);
             }
             //lineDesc  = "  ACQUIRER";
             //receipt += cyLRText(23, lineDesc, 19, "${pay.getApprovalMerchant()} ");
             lineDesc = "  APPROVAL CODE ";
             _txtpay += _util->cyLRText(18, lineDesc, 12, jpay["approval_code"].asString());
         }
         ////////////////////////////////////
         //  Charge card
         lineDesc = "";
         if (isCharge) {
             bextra = true;            bextra = true;
             lineDesc = "  ACCOUNT   ";
             lineDesc += jpay["account_number"].asString();
             lineDesc += "\n";
             _txtpay += lineDesc;

             lineDesc = "  CUSTOMER  ";
             lineDesc += jpay["first_name"].asString();
             lineDesc += " ";
             lineDesc += jpay["last_name"].asString();
             lineDesc += "\n";
             _txtpay += lineDesc;
         }
         if (isMobilePay) {
             int idxTerms = 0;
             bextra = true;
             lineDesc = "  TERMS ";
             tmp = jpay["approval_terms"].asString();
             if (!_util->valid_number("Payment terms", tmp, 1, 2))
                 return seterrormsg(_util->errormsg());
             idxTerms = atoi(tmp.c_str ());
             switch (idxTerms) {
               case 0:
                 headerText += "Straight";
                 break;
               case 1:
                 headerText += "3 months";
                 break;
               case 2:
                 headerText += "6 months";
                 break;
               case 3:
                 headerText += "9 months";
                 break;
               case 4:
                 headerText += "12 months";
                 break;
               case 5:
                 headerText += "18 months";
                 break;
               case 6:
                 headerText += "24 months";
                 break;
               case 7:
                 headerText += "36 months";
                 break;
             }
             _txtpay += _util->cyLRText(17, lineDesc, 13, headerText);

             lineDesc  = "  ACQUIRER";
             _txtpay += _util->cyLRText(17, lineDesc, 13, "eGHL");

             lineDesc  = "  CARD HOLDER";
             _txtpay += _util->cyLRText(17, lineDesc, 13, jpay["first_name"].asString());

             lineDesc = "  APPROVAL CODE ";
             _txtpay += _util->cyLRText(17, lineDesc, 13, jpay["approval_code"].asString());
         }
         if (!bextra)  {
             lineDesc  = "    REFERENCE: ";
             lineDesc += jpay["approval_code"].asString();
             if (jpay["approval_code"].asString().length() > 0 && receiptType != SRT_REFUND)  {
                 _txtpay += lineDesc;
                 _txtpay += "\n";
             }

         }
     }
     jheader["cytotal_payment"]=totalPaymentAmount;
     //////////////////////////////////////
     //  Net sales amount
     /*
     double dtmp;
     dtmp = _totalGrossAmount;
     dtmp = _totalItemDiscount;
     dtmp = _totalTrxDiscount;
     dtmp = _totalAmtExempt;
     dtmp = _totalAmtZero;*/
     double netSalesAmount = _totalGrossAmount -
             (_totalItemDiscount+_totalTrxDiscount+_totalAmtExempt+_totalAmtZero);
     //////////////////////////////////////
     //  Calculate the change
     double absNet = netSalesAmount;
     if (absNet < 0)
         absNet = netSalesAmount * -1;
     //////////////////////////////////////
     //  GC has no change
     if (isGc && totalChangeAmount > 0)
         totalChangeAmount = 0.00;
     sprintf (sztmp, "%.02f", (totalChangeAmount*1000)/1000);
     totalChangeAmount = atof(sztmp);
     jheader["cytotal_change"]=totalChangeAmount;
     //////////////////////////////////////
     //  Display the change
     if (receiptType != salesReceiptType::SRT_REFUND)  {
         if (totalChangeAmount > 0) {
             if (receiptType == SRT_POSTVOID)
                 sprintf (sztmp, "-%.02f", (totalChangeAmount*1000)/1000);
             _txtpay += _util->cyLRText(18, "Change", 12, sztmp);
         }  else  {
             if (totalChangeAmount != 0)  {
                 double dtmp = totalChangeAmount * -1;
                 sprintf (sztmp, "%.02f", dtmp);
                 //_txtpay += _util->cyLRText(23, "Remaining balance", 18, sztmp);
             }
         }
     }

     int w = 30;
     _txtpay += _util->cyCenterText(w, " ");
     sprintf (sztmp, "%.03f-item(s)", _totalItems);
     _txtpay += _util->cyCenterText(w, sztmp);
     _txtpay += _util->cyCenterText(w, " ");
     if (receiptType == salesReceiptType::SRT_SUSPEND)  {
         _receipt += _txtpay;
         return true;
     }


     Json::Value jcashier;
     if (jheader["settings_cashier"] == Json::nullValue)
         return seterrormsg("Unable to get the cashier details");
     jcashier = jheader["settings_cashier"];

     size_t len = jcashier["first_name"].asString().length();
     len += jcashier["last_name"].asString().length();
     len += 14;
     len += jcashier["login"].asString().length();

     if ((int)len > w)  {
         _txtpay += "Cashier  : ";
         _txtpay += jcashier["first_name"].asString();
         _txtpay += " ";
         _txtpay += jcashier["last_name"].asString();

         _txtpay += "\n           [";
         _txtpay += jcashier["login"].asString();
         _txtpay += "]\n";
     }  else  {
         _txtpay += "Cashier  : ";
         _txtpay += jcashier["first_name"].asString();
         _txtpay += " ";
         _txtpay += jcashier["last_name"].asString();
         _txtpay += "[";
         _txtpay += jcashier["login"].asString();
         _txtpay += "]\n";
     }
     if (receiptType == SRT_REFUND)  {
         _txtpay += "Txn No.  : ";
         _txtpay += jheader["systransnum"].asString();
         _txtpay += "\n";

         _txtpay += "Bus Date : ";
         _txtpay += jheader["logical_date"].asString();
         _txtpay += "\n";
/*
         _txtpay += "Orig Pos#: ";
         _txtpay += jheader["orig_register_number"].asString();
         _txtpay += "\n";

         _txtpay += "Orig Date: ";
         _txtpay += jheader["orig_transaction_date"].asString();
         _txtpay += "\n";

         _txtpay += "Orig Inv : ";
         _txtpay += jheader["orig_systransnum"].asString();
         _txtpay += "\n";*/
     }  else  {
         //CONSTRUCT DATE IN RCC FORMAT
         string tmp2, trans_time,trans_date,dd,mm,yyyy;
         trans_date = jheader["transaction_date"].asString();
         trans_time = jheader["transaction_time"].asString();
         _util->date_split(trans_date.c_str(),yyyy,mm,dd);
         tmp2  = "Date-time: ";
         tmp2 += mm;
         tmp2 += "/";
         tmp2 += dd;
         tmp2 += "/";
         tmp2 += yyyy;
         tmp2 += "-";
         tmp2 += _util->time_colon(trans_time.c_str());

         _txtpay += tmp2.c_str();
         _txtpay += "\n";

         _txtpay += "SYSDATE  : ";
         _txtpay += _util->date_nice(jheader["logical_date"].asString().c_str ());
         _txtpay += "\n\n";
     }

     _receipt += _txtpay;
     return true;
}
