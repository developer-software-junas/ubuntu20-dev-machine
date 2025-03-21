/*******************************************************************
          FILE:          cyw_html.h
   DESCRIPTION:          HTML class header
 *******************************************************************/
#ifndef __CYWARE_HTML_H
#define __CYWARE_HTML_H
#include <string>
///////////////////////////////////////////
//  INI file header
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "col/column_enum.h"
//////////////////////////////////////////
//  Image path(s)
#define                  g_img_fnb        "/Library/St0rm/2.0.0/web/storm/images/brewery/fnbtable/"
#define                  g_img_mode       "/Library/St0rm/2.0.0/web/storm/images/brewery/posmode/"
#define                  g_img_user       "/Library/St0rm/2.0.0/web/storm/images/brewery/user/"
#define                  g_img_brand      "/Library/St0rm/2.0.0/web/storm/images/brewery/brand/"
#define                  g_img_product    "/Library/St0rm/2.0.0/web/storm/images/products/"
#define                  g_img_division   "/Library/St0rm/2.0.0/web/storm/images/brewery/division/"
#define                  g_img_cardtype   "/Library/St0rm/2.0.0/web/storm/images/brewery/cardtype/"
#define                  g_img_appmenu    "/Library/St0rm/2.0.0/web/storm/images/brewery/appmenu/"
#define                  g_img_event      "/Library/St0rm/2.0.0/web/storm/images/brewery/event/"
#define                  g_img_partner    "/Library/St0rm/2.0.0/web/storm/images/brewery/partner/"
#define                  g_img_promo      "/Library/St0rm/2.0.0/web/storm/images/brewery/promo/"
#define                  g_img_voucher    "/Library/St0rm/2.0.0/web/storm/images/brewery/voucher/"
#define                  g_img_posmode    "/Library/St0rm/2.0.0/web/storm/images/brewery/posmode/"
#define                  g_img_tendertype "/Library/St0rm/2.0.0/web/storm/images/brewery/tendertype/"

#define                  g_img_recipe     "/Library/St0rm/2.0.0/web/storm/images/brewery/fnbitems/"
/***************************************************************************
         CLASS:          cyhtml
   DESCRIPTION:          HTML class definition
 ***************************************************************************/

class                    cyhtml
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cyhtml (cyini* ini,
            cylog* log,
            cycgiform* cgi,
            cyutility* util);
    virtual ~cyhtml ();
    ///////////////////////////////////////
    //  Access method(s)
    std::string          lastkey ();
    void                 set_sessionid (std::string sessionid);
    ///////////////////////////////////////
    //  HTML Method(s)
    virtual std::string  footer ();
    virtual std::string  header (std::string title,
                                 std::string height="100vh");
    virtual std::string  hidden (int offset,
                                 std::string name,
                                 std::string value);

    virtual std::string  link (std::string action,
                               std::string request="");

    virtual std::string  form (int offset,
                               std::string name,
                               std::string request,
                               std::string method,
                               std::string target,
                               std::string style,
                               bool autocomplete,
                               std::string cginame);

    virtual std::string  link_menu (std::string area_code,
                                    std::string table_code,
                                    std::string action,
                                    std::string request="");

    virtual std::string  link_shopify (std::string shopname,
                                       std::string scriptname);
    virtual std::string  redirect (bool clean,
                                   std::string page,
                                   std::string target,
                                   std::string errmsg,
                                   std::string cginame);
    ///////////////////////////////////////
    //  Single frame method(s)
    virtual std::string  link_sf (std::string request="")
        {  return request;  }
    ///////////////////////////////////////
    //  Input method(s)
    virtual std::string  input_select (int offset,
                                       std::string name,
                                       std::string option,
                                       std::string script);

    virtual std::string  input_lbl (int offset,
                                    JSCONST_CASE inputcase,
                                    int maxlen,
                                    bool required,
                                    bool readonly,
                                    std::string name,
                                    std::string value,
                                    std::string type,
                                    std::string label,
                                    std::string prompt,
                                    std::string pattern,
                                    std::string style);

    virtual std::string  input_date (int offset, bool readonly,
                                     bool autofocus, std::string name,
                                     std::string value, std::string label);

    virtual std::string  input_time (int offset, bool readonly,
                                     bool autofocus, std::string name,
                                     std::string value, std::string label);

    virtual std::string  input_edit (int offset, int maxlen, bool readonly,
                                     std::string name,  std::string value,
                                     std::string label, std::string prompt);

    virtual std::string  input_num (int offset,
                                    int maxlen,
                                    double minval,
                                    double maxval,
                                    bool required,
                                    bool readonly,
                                    std::string name,
                                    std::string value,
                                    std::string label,
                                    std::string prompt,
                                    std::string pattern,
                                    std::string style,
                                    std::string step="");

    virtual std::string  input_button (int offset,
                                       std::string type,
                                       std::string name,
                                       std::string desc,
                                       std::string form,
                                       std::string style,
                                       std::string icon,
                                       std::string script);

    virtual std::string  input_checkbox (int offset,
                                         bool readonly,
                                         std::string name,
                                         std::string value,
                                         std::string label);
    ///////////////////////////////////////
    //  Error related method(s)
    virtual std::string  fatal_error (std::string message,
                                      std::string logmessage="",
                                      bool forceheader = false);
    bool                 _hdr;
protected:
    cylog*               _log;
    cyini*               _ini;
    cyutility*           _util;
    cycgiform*           _cgiform;
    std::string          _lastkey,
                         _sessionid;
};


/***************************************************************************
         CLASS:          cyhtmlalt
   DESCRIPTION:          HTML class definition
 ***************************************************************************/
/*
class                    cyhtmlalt : public cyhtml
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cyhtmlalt (cyini* ini,
               cylog* log,
               cycgiform* cgi,
               cyutility* util);
    virtual ~cyhtmlalt ();
    ///////////////////////////////////////
    //  HTML Method(s)
    virtual std::string  footer ();
    virtual std::string  header (std::string title,
                                 std::string height="100vh");
    virtual std::string  hidden (int offset,
                                 std::string name,
                                 std::string value);

    virtual std::string  link (std::string action,
                               std::string request="");

    virtual std::string  form (int offset,
                               std::string name,
                               std::string action,
                               std::string method,
                               std::string target,
                               std::string style,
                               bool autocomplete,
                               std::string cginame);

    virtual std::string  link_shopify (std::string shopname,
                                       std::string scriptname);
    virtual std::string  redirect (bool clean,
                                   std::string page,
                                   std::string target,
                                   std::string errmsg,
                                   std::string cginame);
    ///////////////////////////////////////
    //  Single frame method(s)
    virtual std::string  link_sf (std::string request="");
    ///////////////////////////////////////
    //  Input method(s)
    virtual std::string  input_select (int offset,
                                       std::string name,
                                       std::string option,
                                       std::string script);

    virtual std::string  input_lbl (int offset,
                                    int inputcase,
                                    int maxlen,
                                    bool required,
                                    bool readonly,
                                    std::string name,
                                    std::string value,
                                    std::string type,
                                    std::string label,
                                    std::string prompt,
                                    std::string pattern,
                                    std::string style);

    virtual std::string  input_date (int offset, bool readonly,
                                     bool autofocus, std::string name,
                                     std::string value, std::string label);

    virtual std::string  input_time (int offset, bool readonly,
                                     bool autofocus, std::string name,
                                     std::string value, std::string label);

    virtual std::string  input_edit (int offset, int maxlen, bool readonly,
                                     std::string name,  std::string value,
                                     std::string label, std::string prompt);

    virtual std::string  input_num (int offset,
                                    int maxlen,
                                    double minval,
                                    double maxval,
                                    bool required,
                                    bool readonly,
                                    std::string name,
                                    std::string value,
                                    std::string label,
                                    std::string prompt,
                                    std::string pattern,
                                    std::string style,
                                    std::string step="");

    virtual std::string  input_button (int offset,
                                       std::string type,
                                       std::string name,
                                       std::string desc,
                                       std::string form,
                                       std::string style,
                                       std::string icon,
                                       std::string script);

    virtual std::string  input_checkbox (int offset,
                                         bool readonly,
                                         std::string name,
                                         std::string value,
                                         std::string label);
    ///////////////////////////////////////
    //  Error related method(s)
    virtual std::string  fatal_error (std::string message,
                                      std::string logmessage="",
                                      bool forceheader = false);
};*/
#endif
