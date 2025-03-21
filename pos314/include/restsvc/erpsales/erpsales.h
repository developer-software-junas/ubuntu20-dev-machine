/********************************************************************
          FILE:          erpsales.h
   DESCRIPTION:          POS to ERP sales
 ********************************************************************/
#ifndef CYERPSALES_H
#define CYERPSALES_H
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
//////////////////////////////////////////
//  Constants, typedefs and enums
namespace salesinvoice {
enum ENUM_SALES_INVOICE  {
    name, creation, modified, modified_by, owner, docstatus, idx, title, naming_series, customer,
    customer_name, tax_id, company, company_tax_id, posting_date, posting_time, set_posting_time,
    due_date, is_pos, pos_profile, is_consolidated, is_return, return_against, update_outstanding_for_self,
    update_billed_amount_in_sales_order, update_billed_amount_in_delivery_note, is_debit_note, amended_from,
    cost_center, project, currency, conversion_rate, selling_price_list, price_list_currency, plc_conversion_rate,
    ignore_pricing_rule, scan_barcode, update_stock, set_warehouse, set_target_warehouse, total_qty, total_net_weight,
    base_total, base_net_total, total, net_total, tax_category, taxes_and_charges, shipping_rule, incoterm,
    named_place, base_total_taxes_and_charges, total_taxes_and_charges, base_grand_total, base_rounding_adjustment,
    base_rounded_total, base_in_words, grand_total, rounding_adjustment, use_company_roundoff_cost_center, rounded_total,
    in_words, total_advance, outstanding_amount, disable_rounded_total, apply_discount_on, base_discount_amount,
    is_cash_or_non_trade_discount, additional_discount_account, additional_discount_percentage, discount_amount,
    other_charges_calculation, total_billing_hours, total_billing_amount, cash_bank_account, base_paid_amount,
    paid_amount, base_change_amount, change_amount, account_for_change_amount, allocate_advances_automatically,
    only_include_allocated_payments, write_off_amount, base_write_off_amount, write_off_outstanding_amount_automatically,
    write_off_account, write_off_cost_center, redeem_loyalty_points, loyalty_points, loyalty_amount, loyalty_program,
    loyalty_redemption_account, loyalty_redemption_cost_center, customer_address, address_display, contact_person,
    contact_display, contact_mobile, contact_email, territory, shipping_address_name, shipping_address, dispatch_address_name,
    dispatch_address, company_address, company_address_display, ignore_default_payment_terms_template, payment_terms_template,
    tc_name, terms, po_no, po_date, debit_to, party_account_currency, is_opening, unrealized_profit_loss_account,
    against_income_account, sales_partner, amount_eligible_for_commission, commission_rate, total_commission, letter_head,
    group_same_items, select_print_heading, language, subscription, from_date, auto_repeat, to_date, status,
    inter_company_invoice_reference, campaign, represents_company, source, customer_group, is_internal_customer, is_discounted,
    remarks, _user_tags, _comments, _assign, _liked_by, _seen, company_contact_person, si_end
};
}

const std::string field_sales_invoice [] = {
    "name", "creation", "modified", "modified_by", "owner", "docstatus", "idx", "title", "naming_series", "customer",
    "customer_name", "tax_id", "company", "company_tax_id", "posting_date", "posting_time", "set_posting_time",
    "due_date", "is_pos", "pos_profile", "is_consolidated", "is_return", "return_against", "update_outstanding_for_self",
    "update_billed_amount_in_sales_order", "update_billed_amount_in_delivery_note", "is_debit_note", "amended_from",
    "cost_center", "project", "currency", "conversion_rate", "selling_price_list", "price_list_currency", "plc_conversion_rate",
    "ignore_pricing_rule", "scan_barcode", "update_stock", "set_warehouse", "set_target_warehouse", "total_qty", "total_net_weight",
    "base_total", "base_net_total", "total", "net_total", "tax_category", "taxes_and_charges", "shipping_rule", "incoterm",
    "named_place", "base_total_taxes_and_charges", "total_taxes_and_charges", "base_grand_total", "base_rounding_adjustment",
    "base_rounded_total", "base_in_words", "grand_total", "rounding_adjustment", "use_company_roundoff_cost_center", "rounded_total",
    "in_words", "total_advance", "outstanding_amount", "disable_rounded_total", "apply_discount_on", "base_discount_amount",
    "is_cash_or_non_trade_discount", "additional_discount_account", "additional_discount_percentage", "discount_amount",
    "other_charges_calculation", "total_billing_hours", "total_billing_amount", "cash_bank_account", "base_paid_amount",
    "paid_amount", "base_change_amount", "change_amount", "account_for_change_amount", "allocate_advances_automatically",
    "only_include_allocated_payments", "write_off_amount", "base_write_off_amount", "write_off_outstanding_amount_automatically",
    "write_off_account", "write_off_cost_center", "redeem_loyalty_points", "loyalty_points", "loyalty_amount", "loyalty_program",
    "loyalty_redemption_account", "loyalty_redemption_cost_center", "customer_address", "address_display", "contact_person",
    "contact_display", "contact_mobile", "contact_email", "territory", "shipping_address_name", "shipping_address", "dispatch_address_name",
    "dispatch_address", "company_address", "company_address_display", "ignore_default_payment_terms_template", "payment_terms_template",
    "tc_name", "terms", "po_no", "po_date", "debit_to", "party_account_currency", "is_opening", "unrealized_profit_loss_account",
    "against_income_account", "sales_partner", "amount_eligible_for_commission", "commission_rate", "total_commission", "letter_head",
    "group_same_items", "select_print_heading", "language", "subscription", "from_date", "auto_repeat", "to_date", "status",
    "inter_company_invoice_reference", "campaign", "represents_company", "source", "customer_group", "is_internal_customer", "is_discounted",
    "remarks", "_user_tags", "_comments", "_assign", "_liked_by", "_seen", "company_contact_person", "si_end"
};
namespace salesitem  {
enum ENUM_SALES_ITEM  {
    name, creation, modified, modified_by, owner, docstatus, idx,
    barcode, has_item_scanned, item_code, item_name, customer_item_code, description, item_group, brand, image, qty, stock_uom, uom,
    conversion_factor, stock_qty, price_list_rate, base_price_list_rate, margin_type, margin_rate_or_amount, rate_with_margin,
    discount_percentage, discount_amount, base_rate_with_margin, rate, amount, item_tax_template, base_rate, base_amount, pricing_rules,
    stock_uom_rate, is_free_item, grant_commission, net_rate, net_amount, base_net_rate, base_net_amount, delivered_by_supplier,
    income_account, is_fixed_asset, asset, finance_book, expense_account, discount_account, deferred_revenue_account, service_stop_date,
    enable_deferred_revenue, service_start_date, service_end_date, weight_per_unit, total_weight, weight_uom, warehouse, target_warehouse,
    quality_inspection, serial_and_batch_bundle, use_serial_batch_fields, allow_zero_valuation_rate, incoming_rate, item_tax_rate,
    actual_batch_qty, serial_no, batch_no, actual_qty, company_total_stock, sales_order, so_detail, sales_invoice_item, delivery_note,
    dn_detail, delivered_qty, purchase_order, purchase_order_item, cost_center, project, page_break, parent, parentfield, parenttype, si_end
};
}
const std::string field_sales_item [] = {
    "name", "creation", "modified", "modified_by", "owner", "docstatus", "idx",
    "barcode", "has_item_scanned", "item_code", "item_name", "customer_item_code", "description", "item_group", "brand", "image", "qty", "stock_uom", "uom",
    "conversion_factor", "stock_qty", "price_list_rate", "base_price_list_rate", "margin_type", "margin_rate_or_amount", "rate_with_margin",
    "discount_percentage", "discount_amount", "base_rate_with_margin", "rate", "amount", "item_tax_template", "base_rate", "base_amount", "pricing_rules",
    "stock_uom_rate", "is_free_item", "grant_commission", "net_rate", "net_amount", "base_net_rate", "base_net_amount", "delivered_by_supplier",
    "income_account", "is_fixed_asset", "asset", "finance_book", "expense_account", "discount_account", "deferred_revenue_account", "service_stop_date",
    "enable_deferred_revenue", "service_start_date", "service_end_date", "weight_per_unit", "total_weight", "weight_uom", "warehouse", "target_warehouse",
    "quality_inspection", "serial_and_batch_bundle", "use_serial_batch_fields", "allow_zero_valuation_rate", "incoming_rate", "item_tax_rate",
    "actual_batch_qty", "serial_no", "batch_no", "actual_qty", "company_total_stock", "sales_order", "so_detail", "sales_invoice_item", "delivery_note",
    "dn_detail", "delivered_qty", "purchase_order", "purchase_order_item", "cost_center", "project", "page_break", "parent", "parentfield", "parenttype", "si_end"
};
namespace salestax {
enum ENUM_SALES_TAX  {
    name, creation, modified, modified_by, owner, docstatus, idx,
    charge_type, row_id, account_head, description, included_in_print_rate, included_in_paid_amount, cost_center,
    rate, account_currency, tax_amount, total, tax_amount_after_discount_amount, base_tax_amount, base_total,
    base_tax_amount_after_discount_amount, item_wise_tax_detail, dont_recompute_tax, parent, parentfield, parenttype,
    si_end
};
}
const std::string field_sales_tax [] = {
    "name", "creation", "modified", "modified_by", "owner", "docstatus", "idx",
    "charge_type", "row_id", "account_head", "description", "included_in_print_rate", "included_in_paid_amount", "cost_center",
    "rate", "account_currency", "tax_amount", "total", "tax_amount_after_discount_amount", "base_tax_amount", "base_total",
    "base_tax_amount_after_discount_amount", "item_wise_tax_detail", "dont_recompute_tax", "parent", "parentfield", "parenttype",
    "si_end"
};
namespace salespay {
enum ENUM_SALES_PAY  {
    name, creation, modified, modified_by, owner, docstatus, idx,
    mode_of_payment, amount, reference_no, account, type,
    base_amount, parent, parentfield, parenttype,
    si_end
};
}
const std::string field_sales_pay [] = {
    "name", "creation", "modified", "modified_by", "owner", "docstatus", "idx",
    "mode_of_payment", "amount", "reference_no", "account", "type",
    "base_amount", "parent", "parentfield", "parenttype",
    "si_end"
};
namespace erpcustomer  {
enum ENUM_ERPCUSTOMER  {
    name,creation,modified,modified_by,owner,docstatus,idx,naming_series,
    salutation,customer_name,customer_type,customer_group,gender,default_currency,
    is_internal_customer,represents_company,customer_pos_id,mobile_no,email_id,
    payment_terms,is_frozen,disabled,custom_last_name,custom_first_name,custom_middle_name,
    custom_company_name,custom_card_number,custom_systransnum,card_number,application_type,
    active,total_sales,total_points,points_balance,card_type,application_date,card_type_code,
    total_visits,total_redeemed,custom_birthday, si_end
};
}
const std::string field_erpcustomer [] = {
    "name","creation","modified","modified_by","owner","docstatus","idx","naming_series",
    "salutation","customer_name","customer_type","customer_group","gender","default_currency",
    "is_internal_customer","represents_company","customer_pos_id","mobile_no","email_id",
    "payment_terms","is_frozen","disabled","custom_last_name","custom_first_name","custom_middle_name",
    "custom_company_name","custom_card_number","custom_systransnum","card_number","application_type",
    "active","total_sales","total_points","points_balance","card_type","application_date","card_type_code",
    "total_visits","total_redeemed","custom_birthday","si_end"
};

/********************************************************************
         CLASS:         cyerpdetail
   DESCRIPTION:         POS to ERP sales class
 ********************************************************************/
class                   cylinedetail
{
public:
    cylinedetail () {}
    virtual ~cylinedetail () {}
    std::string line [salesitem::ENUM_SALES_ITEM::si_end];
};
/********************************************************************
         CLASS:         cyerpdetail
   DESCRIPTION:         POS to ERP sales class
 ********************************************************************/
class                   cylinetax
{
public:
    cylinetax () {}
    virtual ~cylinetax () {}
    std::string tax [salestax::ENUM_SALES_TAX::si_end];
};
/********************************************************************
         CLASS:         cyerppayment
   DESCRIPTION:         POS to ERP sales class
 ********************************************************************/
class                   cylinepayment
{
public:
    cylinepayment () {}
    virtual ~cylinepayment () {}
    std::string line [salespay::ENUM_SALES_PAY::si_end];
};
/********************************************************************
         CLASS:         cyerpsales
   DESCRIPTION:         POS to ERP sales class
 ********************************************************************/
class                   cyerpsales
{
public:
    //////////////////////////////////////
    //  Constructor / destructor
    cyerpsales (cyini* ini, cylog* log, cycgi* cgi);
    virtual ~cyerpsales ();
    //////////////////////////////////////
    //  Initialize
    bool                startup ();
    //////////////////////////////////////
    //  Sales method(s)
    bool                jsonfmt_sales (enumHeader type,
                                       Json::Value& jsales,
                                       std::string systransnum,
                                       bool _memberPay,
                                       std::string _extReceipt,
                                       bool update_eod=false);
    bool                jsonfmt_header (Json::Value& root,
                                        enumHeader type,
                                        std::string systransnum);
    bool                jsonfmt_header_transaction (Json::Value& jhdr,
                                                    std::string systransnum);
    bool                jsonfmt_detail (Json::Value& jhdr,
                                        std::string systransnum);
    bool                jsonfmt_payment (Json::Value& jhdr,
                                         std::string systransnum);
    bool                jsonfmt_kitchen (enumHeader type,
                                         std::string systransnum,
                                         std::string recipe_item_code,
                                         std::string& kitchen_receipt);
    //////////////////////////////////////
    //  Batch method(s)
    bool                batch ();
    bool                batch_sales ();
    bool                batch_sales_void ();
    bool                batch_sales_refund ();
    bool                batch_sales_refund_submit (std::string systransnum);
    //////////////////////////////////////
    //  Sales invoice function(s)
    bool                salestohost(std::string systransnum);
    bool                cancel_trx (std::string systransnum);
    bool                online_sales (enumHeader   type,
                                      Json::Value& jheader,
                                      std::string  systransnum);
    bool                export_header (enumHeader type,
                                       Json::Value jheader,
                                       std::string systransnum);
    bool                export_detail (enumHeader type,
                                       Json::Value jheader,
                                       std::string systransnum);
    bool                export_detail_tax (MySqlDb* taxdb,
                                           MySqlDb* detdb,
                                           std::string systransnum);
    bool                export_detail_svccharge (MySqlDb* taxdb,
                                                 std::string systransnum);

    bool                export_item_senior (enumHeader type,
                                            Json::Value& jline,
                                            cylinedetail* detail,
                                            std::string systransnum);
    bool                export_item_regular (enumHeader type,
                                             Json::Value& jline,
                                             cylinedetail* detail,
                                             std::string systransnum);
    bool                export_item_diplomat (enumHeader type,
                                              Json::Value& jline,
                                              cylinedetail* detail,
                                              std::string systransnum);

    bool                export_detail_senior_tax (MySqlDb* taxdb,
                                                  MySqlDb* detdb,
                                                  std::string systransnum);

    bool                export_payment (enumHeader type,
                                        Json::Value jheader,
                                        std::string systransnum);
    //////////////////////////////////////
    //  Service method(s)
    bool                credit_limit (std::string party,
                                      std::string company,
                                      double &debit, double &credit,
                                      double &creditlimit, double &outstanding_balance);
    //////////////////////////////////////
    //  REST functions
    bool                restput    (std::string url,
                                    std::string postdata,
                                    Json::Value &jresult,
                                    std::string &rawResult);
    bool                restpost   (std::string url,
                                    std::string postdata,
                                    Json::Value &jresult,
                                    std::string &rawResult);
    bool                parseAndVerify (std::string queryResult,
                                        Json::Value& jresult);
    //////////////////////////////////////
    //  Error message
    std::string         errormsg () { return _error; }
protected:
    //////////////////////////////////////
    //  Database environment
    MySqlEnv*           _pos;
    MySqlEnv*           _erp;
    //////////////////////////////////////
    //  Field list
    bool                        _terms,
    _isPwd,
    _isSen5,
    _isSenior,
    _isDiplomat;
    double                      _svcCharge;
    double                      _trxDiscount,
    _lineDiscount;
    std::vector<cylinetax*>     _invTax;
    std::vector<cylinetax*>     _invSvc;
    std::vector<cylinedetail*>  _invDetail;
    std::vector<cylinepayment*> _invPayment;
    std::string                 _invHdrValue [salesinvoice::ENUM_SALES_INVOICE::si_end];
    std::string                 _customerInfo[erpcustomer::ENUM_ERPCUSTOMER::si_end];
    //////////////////////////////////////
    //  Attribute(s)
    cyini*              _ini;
    cylog*              _log;
    cycgi*              _cgi;
    cyutility*          _util;
    std::string         _error, _token;
    //////////////////////////////////////
    //  Default value(s)
    Json::Value         _jerpbranch, _jposprofile;
    std::string         _sqlhdr, _sqldet, _sqlpay, _branch_code, _branch_name,
    _abbr, _company_code, _company_name, _company_tax_id, _currency;
    //////////////////////////////////////
    //  POS default(s)
    string              _price_list_currency, _cost_center, _tax_category,
    _pos_profile, _set_warehouse, _taxes_and_charges,
    _selling_price_list, _set_target_warehouse,
    _account_for_change_amount, _write_off_account,
    _write_off_cost_center, _debit_to, _expense_account,
    _party_account_currency, _against_income_account;
    //////////////////////////////////////
    //  Cleanup dynamic allocation
    void                cleanup_tax ();
    void                cleanup_svc ();
    void                cleanup_sales ();
    void                cleanup_detail ();
    void                cleanup_payment ();
    void                cleanup_customer();
    //////////////////////////////////////
    //  DB method(s)
    bool                customer_exists (std::string account_number);
    bool                clear_sales_tables (std::string systransnum);
    std::string         get_customer_id (std::string account_number);
    //////////////////////////////////////
    //  Error method(s)
    bool                seterrormsg (std::string err)  {
        _error = err;
        return false;
    }
};

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
    //////////////////////////////////////
    // Mallreport functions Ayala
    bool                   mallreportAyala (Json::Value& jsales,mallSalesType type);
    bool                   mallreportAyalaInsert ();
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
    //////////////////////////////////////
    // Mallreport functions Megaworld
    bool                   mallreportMegaworld (Json::Value& jsales,mallSalesType type);
    bool                   mallreportMegaworldInsert ();
    //////////////////////////////////////
protected:

};
#endif
