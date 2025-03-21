//*******************************************************************
//        FILE:     CYRestMallreport.h
// DESCRIPTION:     POS sales objectt
//*******************************************************************
#ifndef cy_rest_sales__h
#define cy_rest_sales__h
#include <string>
#include <sstream>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <list>
//////////////////////////////////////////
//  Header file(s)
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_log.h"
#include "cyw_util.h"
#include "json/json.h"
#include "cydb/cydbsql.h"
#include "cydb/cysqlite.h"
//////////////////////////////////////////
#include "restsvc/storesales/cyrestcommon.h"
//////////////////////////////////////////


//*******************************************************************
//       CLASS:         CYRestMallreport
// DESCRIPTION:         Mallreport sales object
//*******************************************************************
typedef enum  {
    MST_DAILY, MST_POSTVOID
}  mallSalesType;
class                   CYRestMallreport : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestMallreport ();
  CYRestMallreport (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  mallreport variables
      typedef struct {
          std::string systransnum;
          std::string trn_date;
          std::string trn_time;
          std::string transaction_no;
          double gross_sls =0.00;
          double vat_amnt=0.00;
          double vatable_sls=0.00;
          double nonvat_sls=0.00;
          double vatexempt_sls=0.00;
          double vatexempt_amnt=0.00;
          double local_tax=0.00;
          double pwd_disc=0.00;
          double snrcit_disc=0.00;
          double emplo_disc=0.00;
          double ayala_disc=0.00;
          double store_disc=0.00;
          double other_disc=0.00;
          double refund_amt=0.00;
          double schrge_amt=0.00;
          double other_schr=0.00;
          double cash_sls=0.00;
          double card_sls=0.00;
          double epay_sls=0.00;
          double dcard_sls=0.00;
          double othersl_sls=0.00;
          double check_sls=0.00;
          double gc_sls=0.00;
          double mastercard_sls=0.00;
          double visa_sls=0.00;
          double amex_sls=0.00;
          double diners_sls=0.00;
          double jcb_sls=0.00;
          double gcash_sls=0.00;
          double paymaya_sls=0.00;
          double alipay_sls=0.00;
          double wechat_sls=0.00;
          double grab_sls=0.00;
          double foodpanda_sls=0.00;
          double masterdebit_sls=0.00;
          double visadebit_sls=0.00;
          double paypal_sls=0.00;
          double online_sls=0.00;
          double open_sales=0.00;
          double open_sales_2=0.00;
          double open_sales_3=0.00;
          double open_sales_4=0.00;
          double open_sales_5=0.00;
          double open_sales_6=0.00;
          double open_sales_7=0.00;
          double open_sales_8=0.00;
          double open_sales_9=0.00;
          double open_sales_10=0.00;
          double open_sales_11=0.00;
          double gc_excess=0.00;
          std::string mobile_no;
          int no_cust=0;
          std::string trn_type;
          std::string sls_flag;
          double vat_pct=0.00;
          double qty_sld=0.00;
      } struct_transactions;

      typedef struct {
          std::string register_number = "";
          int strans = 0;
          int etrans = 0;
          double gross_sls = 0;
          double vat_amnt = 0;
          double vatable_sls = 0;
          double nonvat_sls = 0;
          double vatexempt_sls = 0;
          double vatexempt_amnt = 0;
          double old_grntot = 0;
          double new_grntot = 0;
          double local_tax = 0;
          double void_amnt = 0;
          double no_void = 0;
          double discounts = 0;
          int no_disc = 0;
          double refund_amt = 0;
          int no_refund = 0;
          double snrcit_disc = 0;
          int no_snrcit = 0;
          double pwd_disc = 0;
          int no_pwd = 0;
          double emplo_disc = 0;
          int no_emplo = 0;
          double ayala_disc = 0;
          int no_ayala = 0;
          double store_disc = 0;
          int no_store = 0;
          double other_disc = 0;
          int no_other_disc = 0;
          double schrge_amt = 0;
          int other_schr = 0;
          double cash_sls = 0;
          double card_sls = 0;
          double epay_sls = 0;
          double dcard_sls = 0;
          double other_sls = 0;
          double check_sls = 0;
          double gc_sls = 0;
          double mastercard_sls = 0;
          double visa_sls = 0;
          double amex_sls = 0;
          double diners_sls = 0;
          double jcb_sls = 0;
          double gcash_sls = 0;
          double paymaya_sls = 0;
          double alipay_sls = 0;
          double wechat_sls = 0;
          double grab_sls = 0;
          double foodpanda_sls = 0;
          double masterdebit_sls = 0;
          double visadebit_sls = 0;
          double paypal_sls = 0;
          double online_sls = 0;
          double open_sales = 0;
          double open_sales_2 = 0;
          double open_sales_3 = 0;
          double open_sales_4 = 0;
          double open_sales_5 = 0;
          double open_sales_6 = 0;
          double open_sales_7 = 0;
          double open_sales_8 = 0;
          double open_sales_9 = 0;
          double open_sales_10 = 0;
          double open_sales_11 = 0;
          double gc_excess = 0;
          int no_vatexemt = 0;
          int no_schrge = 0;
          int no_other_sur = 0;
          int no_cash = 0;
          int no_card = 0;
          int no_epay = 0;
          int no_dcard_sls = 0;
          int no_other_sls = 0;
          int no_check = 0;
          int no_gc = 0;
          int no_mastercard_sls = 0;
          int no_visa_sls = 0;
          int no_amex_sls = 0;
          int no_diners_sls = 0;
          int no_jcb_sls = 0;
          int no_gcash_sls = 0;
          int no_paymaya_sls = 0;
          int no_alipay_sls = 0;
          int no_wechat_sls = 0;
          int no_grab_sls = 0;
          int no_foodpanda_sls = 0;
          int no_masterdebit_sls = 0;
          int no_visadebit_sls = 0;
          int no_paypal_sls = 0;
          int no_online_sls = 0;
          int no_open_sales = 0;
          int no_open_sales_2 = 0;
          int no_open_sales_3 = 0;
          int no_open_sales_4 = 0;
          int no_open_sales_5 = 0;
          int no_open_sales_6 = 0;
          int no_open_sales_7 = 0;
          int no_open_sales_8 = 0;
          int no_open_sales_9 = 0;
          int no_open_sales_10 = 0;
          int no_open_sales_11 = 0;
          int no_nosale = 0;
          int no_cust = 0;
          int no_trn = 0;
          double prev_eodctr = 0;
          double eodctr = 0;
      } struct_daily;
  /////////////////////////////////////
  std::vector<struct_transactions*> _transList;
  //////////////////////////////////////
  // Mallreport functions
  bool                   mallreportAyala (Json::Value& jsales,mallSalesType type);
  bool                   mallreportAyalaInsert ();
  //////////////////////////////////////
protected:

};
#endif
