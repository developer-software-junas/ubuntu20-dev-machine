/*****************************************************************************
          FILE:         column_array.h
   DESCRIPTION:         Column array header file
 *****************************************************************************/
#ifndef _CYCOLUMN_ARRAY_CGI
#define _CYCOLUMN_ARRAY_CGI
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "cyw_js.h"
#include "cydb/cyw_db.h"
#include "cyw_cgi.h"
#include "col/column_array_base.h"
/***************************************************************************
         CLASS:          CYColumnArray
   DESCRIPTION:          Column array class definition
 ***************************************************************************/
class                    CYColumnArray : public CYColumnArrayBase
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnArray (cyjs* js);
    virtual ~CYColumnArray ();
    //////////////////////////////////////
    //  REST method(s)
    /*
    virtual bool         rest_select (std::string& result,
                                      CYColumnBind* bind,
                                      bool is_setup=false);
    */
    //////////////////////////////////////
    //  HTTP method(s)
    std::string          syncget (std::string url);
    ///////////////////////////////////////
    //  CGI dependent
    virtual void         form (cycgiform* cgifrm,
                               const char* column,
                               const char* query=NULL);

    virtual void         formall (cycgiform* cgifrm,
                                  const char* suffix=NULL);
    virtual bool         mapweb (cycgiform* cgifrm,
                                 enum_validcol type,
                                 const char* suffix=NULL);
    virtual bool         mapandvalidate (cycgiform* cgifrm,
                                         const char* column,
                                         const char* query=NULL);

    virtual bool         mapsearchfields   (cycgiform* cgifrm,
                                            const char* suffix=NULL);
    virtual bool         mapandvalidatekey (cycgiform* cgifrm,
                                            const char* suffix=NULL);
    virtual bool         mapandvalidateall (cycgiform* cgifrm,
                                            const char* suffix=NULL);
    //////////////////////////////////////
    //  REST database operation(s)
    virtual std::string  restread ();
    virtual std::string  restcreate ();
    virtual std::string  restupdate ();
    virtual std::string  restdelete ();
protected:
    cycgi*               _cgi;
    cywaredb*            _cydb;
    cyutility*           _util;
    std::string          _dbdefault;
};
#endif
