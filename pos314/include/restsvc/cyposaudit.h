/********************************************************************
          FILE:         cyposaudit.h
   DESCRIPTION:         REST data for flutter
 ********************************************************************/
#ifndef __CYPOSAUDIT_H
#define __CYPOSAUDIT_H
//////////////////////////////////////////
//  CYWare class(es)
#include "cyw_ini.h"
#include "cyw_log.h"
#include "cyw_cgi.h"
#include "cyw_util.h"
#include "cydb/cyw_db.h"
//////////////////////////////////////////
//  Database object(s)
#include "cydb/cydbsql.h"
#include "cyposrest.h"
/********************************************************************
      FUNCTION:         cyposaudit
   DESCRIPTION:         POS audit data class
 ********************************************************************/
class                   cyposaudit
{
public:
    //////////////////////////////////////
    //  Constructor
    cyposaudit (cyposrest* rest);
    //  *** use for printing only, no rest pointer
    cyposaudit (cylog* log, cyini* ini, cycgi* cgi, cyutility* cyutil);
    virtual ~cyposaudit ();
    //////////////////////////////////////
    //  Log FNB actions
    bool                auditlog_fnb (std::string action,
                                      std::string cashier,
                                      std::string register_num,
                                      std::string location_code,
                                      std::string table_code,
                                      std::string order_number,
                                      std::string customer_number,
                                      std::string audit_info);

    bool                auditlog_pos (std::string action,
                                      std::string cashier,
                                      std::string register_num,
                                      std::string location_code,
                                      std::string systransnum,
                                      std::string audit_info);
    //////////////////////////////////////
    //  Log using the systransnum
    /*
    bool                auditlog_trxnum (enumHeader hdrType,
                                         std::string action,
                                         std::string systransnum,
                                         std::string audit_info);*/
    //////////////////////////////////////
    //  Log credentials
    /*
    bool                auditlog_credentials (std::string action,
                                              std::string cashier,
                                              std::string serialno);*/
    //////////////////////////////////////
    //  Audit method(s)
    /*
    bool                auditlog(std::string action,
                                 std::string cashier,
                                 std::string register_num,
                                 std::string location_code,
                                 std::string audit_info,
                                 std::string systransnum,
                                 std::string cashier_shift="");*/

    std::string         _auditInfo;
protected:
    cylog*              _log;
    cyini*              _ini;
    cycgi*              _cgi;
    cyutility*          _util;
    cyposrest*          _rest;
};
#endif
