/*******************************************************************
          FILE:          cyw_js.h
   DESCRIPTION:          JS engine class header
 *******************************************************************/
#ifndef __CYWARE_JS_H
#define __CYWARE_JS_H
#include <string>
#include <sstream>
#include <pthread.h>
///////////////////////////////////////////
//  Header file(s)
#include "cydb/cyw_db.h"
#include "cyw_aes.h"
#include "cyw_mem.h"
#include "cyw_log.h"
#include "cyw_ini.h"
#include "cyw_cgi.h"
#include "cyw_html.h"
#include "cyw_util.h"
//#include "shopify/cyw_shopify.h"
//#include "shopify/cyw_global_app.h"
//////////////////////////////////////////
//  Classes used
class                    cysession;
class                    cyw_shopify;
class                    CYColumnArray;
//////////////////////////////////////////
//  Constant(s)
const int                CYMAXLIB       = 64;
//const std::string        CYHMACKEY      = "pjgabcarcis";
const std::string        CYCGIPATH      = "/brewery/";
const std::string        CYLIBPATH      = "/Library/St0rm/2.0.0/";
///////////////////////////////////////////
//  Shared library function prototype
typedef bool             (*cyjslib)(void*, const char*);
/***************************************************************************
         CLASS:          cyjs
   DESCRIPTION:          JS engine class definition
 ***************************************************************************/
class                    cyjs
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    cyjs (int argc, char** argv, cyini* ini);
    cyjs (int argc, char** argv,
          std::string inifile,
          std::string logfile);
    virtual ~cyjs ();
    //////////////////////////////////////
    //  Overrideable method(s)
    virtual bool         cgimain  (std::string& cyw_request,
                                   bool nosession = false);
    //////////////////////////////////////
    //  Method(s)
    bool                 validatekey (bool bsession=true);
    std::string          digikey   (std::string  queryargs);
    void                 appendkey (std::string& queryargs);
    bool                 parsekey  (std::string  queryargs,
                                    std::string& digikey,
                                    std::string& cleanargs);
    //////////////////////////////////////
    //  Session attribute(s)
    std::string          _sessionip;
    //////////////////////////////////////
    //  Aggregate objects
    cyini*               _ini;
    cylog*               _log;
    cycgi*               _cgi;
    cyhtml*              _html;
    cywaredb*            _cydb;
    cyutility*           _util;
    cysession*           _session;
    cyw_shopify*         _shopify;
    ///////////////////////////////////////
    //  Access method(s)
    bool                 isdebug   ();
    std::string          errormsg  ();
    std::string          exeonly  (std::string exearg="");
    virtual bool         seterrormsg (std::string errormsg);
public:
    //////////////////////////////////////
    //  Public global attribute(s)
    std::string          _cginame,
                         _dbschema,
                         _dbdefault,
                         _cgi_universal,
                         _session_cookie;
protected:
    bool                 _alt,
                         _isdebug,
                         _is_ownini;
    std::string          _exename,
                         _webargs,

                         _appname,
                         _appconfig,
                         _appversion,
                         _apikey,
                         _apipasswd,
                         _apisecret;
    std::stringstream    _error;
    ///////////////////////////////////////
    //  Schema list
    CYColumnArray*       _colarr [CYMAXLIB];
    ///////////////////////////////////////
    //  Utility function(s)
    bool                 debugmode (int argc, char** argv);
};
/***************************************************************************
         CLASS:          cysession
   DESCRIPTION:          Session class
 ***************************************************************************/
class                    cysession
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cysession (cyjs* js, std::string dbdefault);
    virtual ~cysession ();
    ///////////////////////////////////////
    //  Method(s)
    bool                 session_new  (std::string username,
                                       std::string password);
    bool                 session_check (std::string sessionid);
    bool                 session_parse (std::string sessionid);
public:
    //////////////////////////////////////
    //  Utility function(s)
    std::string          get_expire ();
    std::string          session_error ();
    std::string          session_cookie ();
    std::string          session_timeout ();
    std::string          session_id (std::string username);
    void                 session_db (std::string dbdefault);
    bool                 session_info (std::string username="");
    //////////////////////////////////////
    //  User information
    std::string          _entity_code, _company_code, _branch_code,
                         _entity_name, _company_name, _branch_name,
                         _group_name, _branch_type_code, _first_name,
                         _group_code, _middle_name, _last_name,
                         _username, _cookie, _sessionid;
    bool                 _is_cashier, _is_manager,
                         _is_clerk, _is_treasury, _is_shopper;
    int                  _expire_date, _retry_count, _user_locked,
                         _min_pass_len, _passwd_history;
    //////////////////////////////////////
    //  Shopify security
    bool                 _approve_refunds, _approve_revoke,
                         _create_voucher, _view_orders, _same_branch_only;
protected:
    cyjs*                _js;         //  JS object
    std::string          _error,      //  Error message string
                         //_sessionid,  //  Session ID
                         _dbdefault;  //  INI database settings section
};
#endif
