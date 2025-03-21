#ifndef CYPOSTRX_H
#define CYPOSTRX_H
/********************************************************************
          FILE:          cypostrx.h
   DESCRIPTION:          POS transaction header
 ********************************************************************/
//////////////////////////////////////////
//  Standard C
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
//////////////////////////////////////////
//  Standard C++
#include <string>
#include <vector>

//#include "json/json.h"
//#include "../common/common.h"
//#include "../common/CYDb.h"



//////////////////////////////////////////
//  User header file(s)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_aes.h"
#include "cyw_util.h"
#include "cydb/cydbsql.h"
#include "cydb/cymysql.h"
//////////////////////////////////////////
//  Global variable(s)
const std::string       g_logfile = "/usr/local/log/salesimport.log";
const std::string       g_inifile = "/usr/local/settings/storerest.ini";
//////////////////////////////////////////
//  Enumerated request type
typedef enum  {
    eit_none,
    eit_sales,
    eit_suspend,
    eit_void,
    eit_cancel
} ENUM_IMPORT_TYPE;

/***************************************************************************
         CLASS:          cypostrx
   DESCRIPTION:          POS transaction class definition
 ***************************************************************************/
class                    cypostrx
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cypostrx ();
    virtual ~cypostrx ();
    ///////////////////////////////////////
    //  Method(s)
    bool                 saveSqlSales ();
    bool                 saveSqlVoid();
    bool                 saveSqlRefund ();
    bool                 parseSales (std::string strdata);
    bool                 removeSalesFile (std::string filename);
    bool                 json_select_multiple (CYDbSql* db,
                                                                 Json::Value& jval,
                                                                 std::string sql);
    bool                 json_select_single (CYDbSql* db,
                                                               Json::Value& jval,
                                                               std::string sql);
    ///////////////////////////////////////
    //  Table method(s)

    ///////////////////////////////////////
    //  Access method(s)
    bool                 seterrormsg (std::string msg);
    std::string          errormsg () { return _errorMessage; }
    std::string          gettranstype() { return _transtype; }

    //////////////////////////////////////
    // Utility to Get Account Discount Info
    std::string         GetAccountDiscountValue (char*  acct_type_code, char* item_code);

    //////////////////////////////////////
    //  Database object(s)
    //MySqlEnv*           _env = nullptr;
    CYDbEnv             *_env;


    ///////////////////////////////
    //cashier register
    bool                pos_cashier_register ( Json::Value jcashier, int nreset = 0 );
    bool                pos_zread(  Json::Value jcashier,Json::Value jcashdec,Json::Value jcashtakeout);


protected:
    //////////////////////////////////////
    //  Utility object(s)
    cyini*              _ini = nullptr;
    cylog*              _log = nullptr;
    cyutility*          _util = nullptr;
    //ENUM_IMPORT_TYPE    _importType = ENUM_IMPORT_TYPE::eit_sales;


    //////////////////////////////////////
    //  Database connection list
    std::vector<CYDbSql*> _dbList;


    //////////////////////////////////////
    //  Attribute(s)
    Json::Value         _jsales;
    std::string         _transtype,
                        _tblprefix,
                        _errorMessage;
    //////////////////////////////////////
    //  Settings json object(s)
    Json::Value         _jsettings, _jcurrency;
    //////////////////////////////////////
    //  Header json object(s)
    Json::Value         _jtrx, _jtrxtype, _jtrxaccount, _jtrxdiscount,
        _jbranch, _jaccount, _jcompany, _jregister, _settingstransdisc;
    //////////////////////////////////////
    //  Detail json object(s)
    Json::Value         _jdetail, _jdetitem, _jdettax,
                        _jdetuom, _jdetdiscount;
    //////////////////////////////////////
    //  Payment json object(s)
    Json::Value         _jpayment, _jpaygc;

    /////////////////////////////////////
    /// accounts
    Json::Value         _jsclerk, _jfsp, _jba;
    /////////////////////////////////////
    /// Casfund json object
    Json::Value         _jcashfund;



    //////////////////////////////////////
    //  Account flag(s)
    std::string         _sclerk, _pshopper;
    std::string         _is_ba, _is_pwd, _is_senior;
    //////////////////////////////////////
    //  Database variable(s)
    std::string         _systransnum, _suspendnum, _company_code, _branch_code,
                        _register_num, _cashier, _cashier_shift,
                        _logical_date, _company_name, _branch_name;

    ////////////////////////////////////
    /// transaction level vars
    double              _transdiscamt;
    //////////////////////////////////////
    //  Sales table method(s)

    bool                cy_daily_account (CYDbSql* db);
    bool                cy_daily_account_discount (CYDbSql* db);

    bool                cy_daily_audit (CYDbSql* db);
    bool                cy_daily_audit_info (CYDbSql* db);
    bool                cy_daily_audit_display (CYDbSql* db);

    bool                cy_daily_currency (CYDbSql* db);
    bool                cy_daily_settings (CYDbSql* db);

    bool                cy_daily_detail (CYDbSql* db);
    bool                cy_daily_payment(CYDbSql* db);
    bool                cy_daily_header (CYDbSql* db, int nvoid=0, int nrefund=0);

    bool                cy_suspend_pac (CYDbSql* db);

    bool                cy_daily_discount (CYDbSql* db); // no global discount data source yet

    bool                cy_daily_xread (CYDbSql* db);


    //////////////////////////////////////
    //  Utility method(s)
    CYDbSql*            dbconnect ();
    bool                parseFileError (Json::Value jsales,
                                        std::string filename);

};
#endif


