/********************************************************************
          FILE:         rest.h
   DESCRIPTION:         REST data for flutter
 ********************************************************************/
#ifndef __cyreststore_H
#define __cyreststore_H
//////////////////////////////////////////
//  CYWare class(es)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cyrestcommon.h"
//////////////////////////////////////////
//  Classes used
class                   CYDbEnv;
class                   CYDbSql;
class                   cyposaudit;
class                   CGIHandler;
/********************************************************************
      FUNCTION:         cyreststore
   DESCRIPTION:         POS REST data class
 ********************************************************************/
class                   cyreststore
{
public:
    //////////////////////////////////////
    //  Constructor - LOCAL POS or SERVER
    cyreststore (cyini* ini, cylog* log, cycgi* cgi);
    virtual ~cyreststore ();
    //////////////////////////////////////
    //  Method(s)
    bool                prequel(std::string req);
    std::string         cgiform(std::string name);
    std::string         request(std::string cyrequest);
    //////////////////////////////////////
    //  SQL request
    std::string         rest_update  ();
    std::string         rest_retrieve ();
    //////////////////////////////////////
    //  Loyalty request
    bool                rest_loyalty_item_populate ();
    //////////////////////////////////////
    //  Product request
    bool                rest_product_save ();
    std::string         rest_product_image ();
    bool                rest_product_detail_check ();
    //////////////////////////////////////
    //  REST key validation
    bool                validatekey (std::string hmac,
                                     std::string request,
                                     std::string session,
                                     std::string timestamp);
    //////////////////////////////////////
    //  Object access
    cyini               *_ini;
    cylog               *_log;
    cycgi               *_cgi;
    cyutility           *_util = nullptr;  // Useful function(s)

    Json::Value         _jdevice, _jsales,
                        _jfnb, _jpayload;
protected:
    //////////////////////////////////////
    //  Attribute(s)
    CGIHandler*         _cgiHandler;
    cyposaudit*         _audit;
    std::string         _error = "",       // Error string
                        _request = "";     // REST request
    //////////////////////////////////////
    //  Internal key validation
    std::string         digikey  (std::string  queryargs);
    bool                parsekey (std::string  queryargs,
                                  std::string& digikey,
                                  std::string& cleanargs);
    void                appendkey(std::string& queryargs);
    //////////////////////////////////////
    //  JSON utility method(s)
    enumQueryType       json_select_single (CYDbSql* db,
                                            Json::Value& jval,
                                            std::string sql);
    enumQueryType       json_select_multiple (CYDbSql* db,
                                              Json::Value& jval,
                                              std::string sql);
public:
    //////////////////////////////////////
    //  Error message access
    std::string         errormsg ();
    std::string         errorjson (std::string errormsg);
    bool                seterrormsg (std::string message);
    enumQueryType       seterrorquery (std::string message)  {
        seterrormsg(message);
        return enumQueryType::EQT_ERROR;
    }
};
#endif
