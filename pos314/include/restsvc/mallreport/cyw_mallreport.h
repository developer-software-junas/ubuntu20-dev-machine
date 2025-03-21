#ifndef CYW_MALLREPORT_H
#define CYW_MALLREPORT_H
/********************************************************************
          FILE:          cyw_mallreport.h
   DESCRIPTION:          MALL report base class
 ********************************************************************/
#include <vector>
#include <sstream>
#include <sys/types.h>

#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_aes.h"
#include "cyw_util.h"
#include "cyw_util.h"

#include "cydb/cyw_db.h"
#include "cydb/cydbsql.h"
#include "storesales/cyrestreceipt.h"

using std::string;
using std::vector;
using std::stringstream;
typedef enum  {
    MT_AYALA, MT_MEGAWORLD,MT_SMSIA,MT_ROBINSON
}  mallType;
//*******************************************************************
//       CLASS:         cymallreport
// DESCRIPTION:         Mallreport base class
//*******************************************************************
class                   cymallreport : public CYRestCommon
{
public:
    //////////////////////////////////////
    //  Constructor / Destructor
    virtual ~cymallreport ();
    cymallreport (cylog* log, cyini* ini, cycgi* cgi);
    //////////////////////////////////////
    //  Interface
    virtual bool        insertReport ()=0;
    virtual bool        generateReport (Json::Value jsales)=0;
protected:

};
//*******************************************************************
//       CLASS:         CYMallreport
// DESCRIPTION:         Ayala mallreport sales object
//*******************************************************************
class                   cymallayala : public cymallreport
{
public:
    //////////////////////////////////////
    //  Constructor / Destructor
    virtual ~cymallayala ();
    cymallayala (cylog* log, cyini* ini, cycgi* cgi);
    //////////////////////////////////////
    //  Interface
    virtual bool        insertReport ();
    virtual bool        generateReport (Json::Value jsales);
protected:
    ////////////////////////////////////////
    //  Ayala variables
    typedef struct {
        double qty=0.00;
        string item_code;
        double price=0.00;
        double ldisc=0.00;
        double item_seq=0.00;
    } struct_trx_items;

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
        std::vector<struct_trx_items*> listItem;
    } struct_transactions;

    /////////////////////////////////////
    std::vector<struct_transactions*> _transList;
};
//*******************************************************************
//       CLASS:         cymallmegaworld
// DESCRIPTION:         Megaworld mallreport sales object
//*******************************************************************
class                   cymallmegaworld : public cymallreport
{
public:
    //////////////////////////////////////
    //  Constructor / Destructor
    virtual ~cymallmegaworld ();
    cymallmegaworld (cylog* log, cyini* ini, cycgi* cgi);
    //////////////////////////////////////
    //  Interface
    virtual bool        insertReport ();
    virtual bool        generateReport (Json::Value jsales);
protected:
    //////////////////////////////////////
    // Megaworld Variables
    typedef struct {
        string systransnum;
        string transaction_date;
        string transaction_no;
        int terminal_no = 0;
        int trans_time = 0;
        double gross_sales = 0.00;
        double vatable_sls = 0.00;
        double nonvat_sls = 0.00;
        double vatexempt_sls = 0.00;
        double senior_disc = 0.00;
        double other_disc = 0.00;
        double refund_amt = 0.00;
        double vat_amt = 0.00;
        double service_charge = 0.00;
        double net_sales_amt = 0.00;
        double cash_sls = 0.00;
        double credit_sls = 0.00;
        double gc_sls = 0.00;
        double void_amt = 0.00;
        double customer_cnt = 0.00;
        double control_no = 0.00;
        int transaction_cnt = 0;
        string sales_type;
    } struct_megaworld_daily;
    /////////////////////////////////////
    std::vector<struct_megaworld_daily*> _megawordList;
};


//*******************************************************************
//       CLASS:         CYMallreport
// DESCRIPTION:         SM SIA mallreport sales object
//*******************************************************************
class                   cymallsmsia : public cymallreport
{
public:
    //////////////////////////////////////
    //  Constructor / Destructor
    virtual ~cymallsmsia ();
    cymallsmsia (cylog* log, cyini* ini, cycgi* cgi);
    //////////////////////////////////////
    //  Interface
    virtual bool        insertReport ();
    virtual bool        generateReport (Json::Value jsales);
protected:
    ////////////////////////////////////////
    //  Ayala variables
    typedef struct {
        std::string systransnum = "";
        int    bill_num	 = 0;
        std::string item_id = "";
        std::string item_name = "";
        std::string item_parent_category = "";
        std::string item_category = "";
        std::string item_sub_category = "";
        double item_quantity = 0.00;
        double transaction_item_price = 0.00;
        double menu_item_price = 0.00;
        std::string discount_code = "";
        double discount_amount = 0.00;
        std::string modifier_1_name = "";
        double modifier_1_quantity = 0.00;
        std::string modifier_2_name = "";
        double modifier_2_quantity = 0.00;
        int    is_void = 0;
        double void_amount = 0.00;
        int    refund = 0;
        double refund_amount = 0.00;
    } struct_trx_items;

    typedef struct {
        std::string systransnum = "";
        int bill_num = 0;
        std::string business_day = "";
        std::string check_open = "";
        std::string check_close = "";
        std::string sales_type = "";
        std::string transaction_type = "";
        int is_void = 0;
        double void_amount = 0.00;
        int refund = 0;
        double refund_amount = 0.00;
        int guest_count = 0;
        int guest_count_senior = 0;
        int guest_count_pwd = 0;
        double gross_sales_amount = 0.00;
        double net_salesamount = 0.00;
        double total_tax = 0.00;
        double other_local_tax = 0.00;
        double total_service_charge = 0.00;
        double total_tip = 0.00;
        double total_discount = 0.00;
         double less_tax_amount = 0.00;
        double tax_exempt_sales = 0.00;
        std::string regular_other_discount = "";
        double regular_other_discount_amnt = 0.00;
        double employee_discount_amount = 0.00;
        double senior_citizen_discount_amt = 0.00;
        double vip_discount_amt = 0.00;
        double pwd_discount_amt = 0.00;
        double NCAMOV_disc_amount = 0.00;
        double SMAC_disc_amount = 0.00;
        std::string online_deals_discount_name = "";
        double online_deals_discount_amount = 0.00;
        std::string discount_field_1_name = "";
        std::string discount_field_2_name = "";
        std::string discount_field_3_name = "";
        std::string discount_field_4_name = "";
        std::string discount_field_5_name = "";
        std::string discount_field_6_name = "";
        double discount_field_1_amt = 0.00;
        double discount_field_2_amt = 0.00;
        double discount_field_3_amt = 0.00;
        double discount_field_4_amt = 0.00;
        double discount_field_5_amt = 0.00;
        double discount_field_6_amt = 0.00;
        std::string payment_type_1 = "";
        double payment_amount_1 = 0.00;
        std::string payment_type_2 = "";
        double payment_amount_2 = 0.00;
        std::string payment_type_3 = "";
        double payment_amount_3 = 0.00;
        double total_cash_sales_amt = 0.00;
        double total_gift_cheque_amt = 0.00;
        double total_debit_card__amount = 0.00;
        double total_ewallet_amt = 0.00;
        double total_other_tender_amount = 0.00;
        double total_mastercard_amount = 0.00;
        double total_visa_amount = 0.00;
        double total_american_exp_amt = 0.00;
        double total_diners_amt = 0.00;
        double total_jcb_amt = 0.00;
        std::string terminal_number  = "";
        std::string serial_number = "";
        double total_other_credit_amt = 0.00;
    } struct_transactions;

    /////////////////////////////////////
    std::vector<struct_transactions*> _transList;
    std::vector<struct_trx_items*> _itemList;
};

//*******************************************************************
//       CLASS:         CYMallreport
// DESCRIPTION:         Ayala mallreport sales object
//*******************************************************************
class                   cymallrobinson : public cymallreport
{
public:
    //////////////////////////////////////
    //  Constructor / Destructor
    virtual ~cymallrobinson ();
    cymallrobinson (cylog* log, cyini* ini, cycgi* cgi);
    //////////////////////////////////////
    //  Interface
    virtual bool        insertReport ();
    virtual bool        generateReport (Json::Value jsales);
protected:

    typedef struct {
        std::string systransnum;
        std::string transaction_no;
        std::string tenant_id;
        std::string pos_terminal_no;
        double gross_sales = 0.00;
        double total_tax_vat = 0.00;
        double tot_amt_void = 0.00;
        int no_of_void  = 0;
        double tot_amt_disc = 0.00;
        int no_of_disc = 0;
        double tot_amt_refund = 0.00;
        int no_of_refund = 0;
        double other_negative_adj  = 0.00;
        int no_of_negative_adj  = 0;
        double tot_service_chrg = 0.00;
        int prev_eod_ctr = 0;
        double prev_acc_grand_tot  = 0;
        int current_eod_ctr = 0;
        double current_acc_grand_tot = 0.00;
        string sls_trans_date;
        double novelty = 0.00;
        double misc = 0.00;
        double local_tax = 0.00;
        double tot_credit_sls = 0.00;
        double tot_credit_vat = 0.00;
        double tot_nonvat_sls = 0.00;
        double pharma_sls = 0.00;
        double non_pharma_sls = 0.00;
        double pwd_discount = 0.00;
        double gross_sls_fix_rate = 0.00;
        double tot_reprinted_trx = 0.00;
        int no_of_reprint = 0;
    } struct_transactions;

    /////////////////////////////////////
    std::vector<struct_transactions*> _transList;
};
#endif // CYW_MALLREPORT_H


