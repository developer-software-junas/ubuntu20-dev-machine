//*******************************************************************
//        FILE:     cyrestmodifier.h
// DESCRIPTION:     Modifier object
//*******************************************************************
#ifndef cy_rest_modifier__h
#define cy_rest_modifier__h
#include <string>
#include <sstream>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <list>
//////////////////////////////////////////
//  Header file(s)
#include "cyw_aes.h"
#include "cyw_log.h"
#include "cyw_mem.h"
#include "cyw_util.h"
#include "json/json.h"
#include "cydb/cyw_db.h"
#include "cyrestcommon.h"
//*******************************************************************
//       CLASS:     CYRestModifier
// DESCRIPTION:     POS sales object
//*******************************************************************
class               CYRestModifier : public CYRestCommon
{
public:
  ////////////////////////////////////////
  //  Constructor / Destructor
  virtual ~CYRestModifier ();
  CYRestModifier (cylog* log, cyini* ini, cycgi* cgi);
  ////////////////////////////////////////
  //  Operation(s)
  bool                  rest_modifier_salesadd (CYDbSql* db,
                                                double& total,
                                                std::string systransnum,
                                                std::string item_seq);
  bool                  rest_modifier_fnbsalesadd (CYDbSql* db,
                                                   double& total,
                                                   std::string systransnum,
                                                   std::string item_seq);
protected:
};
#endif
