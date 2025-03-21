/*****************************************************************************
          FILE:         column_array_base.h
   DESCRIPTION:         Column array header file
 *****************************************************************************/
#ifndef _CYCOLUMN_ARRAY_BASE
#define _CYCOLUMN_ARRAY_BASE
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "cyw_mem.h"
#include "cyw_cgi.h"
#include "column.h"
#include "cyw_colbind.h"
#include "cydb/cydbsql.h"
///////////////////////////////////////////
//  Classes used
class                    CYColumn;
class                    cycgiform;
class                    CYColumnAlpha;
///////////////////////////////////////////
//  Enumerated type(s)
typedef enum  {
    CY_ALL,
    CY_KEYONLY,
    CY_PASSIVE
}  enum_validcol;
typedef enum  {
    CY_OK,
    CY_EOF,
    CY_RESULT
}  enum_cyret;
/***************************************************************************
         CLASS:          CYColumnArray
   DESCRIPTION:          Column array class definition
 ***************************************************************************/
class                    CYColumnArrayBase
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnArrayBase();
    virtual ~CYColumnArrayBase();
    //////////////////////////////////////
    //  Access methods
    virtual int          size  ();
    virtual void         reset ();
    virtual void         cleanup ();
    virtual void         set_name  (const char* name);

    virtual std::string  name ();
    virtual std::string  dump ();
    virtual std::string  bindargs ();
    //virtual std::string  bindchar (int idx);
    virtual std::string  errormsg ()  { return m_error; }
    //////////////////////////////////////
    //  Initialization
    virtual bool         create ();
    //////////////////////////////////////
    //  Column method(s)
    virtual void         add_column (CYColumn* col);
    virtual CYColumn*    get_column (int idx);
    virtual CYColumn*    get_column (const char* field);
    ///////////////////////////////////////////////////////////////////////////
    //  Access method(s)
    virtual std::string  schema_lib () { return m_schema_lib; }
    virtual void         set_schema_lib (std::string value);

    virtual std::string  schema_req () { return m_schema_req; }
    virtual void         set_schema_req (std::string value);
    virtual void         set_schema_table (std::string value);

    virtual std::string  schema_name ();
    virtual std::string  schema_table ();
    virtual std::string  schema_title ();
    virtual std::string  schema_detail ();
    virtual std::string  schema_welcome ();
    virtual std::string  schema_category ();
    //////////////////////////////////////
    //  Argument mapping
    bool                 mapdb_nocheck (CYDbSql* db);
    bool                 mapdb_arguments (CYDbSql* db);
    bool                 mapweb_arguments (cycgi* cgi);

    ///////////////////////////////////////////////////////////////////////////
    //  Schema
    virtual bool         load_schema_buffer (const char* rpcbuffer);
    ///////////////////////////////////////////////////////////////////////////
    //  Utility
    virtual bool         json_string (std::string& query);
    virtual bool         query_string (std::string& query);
    ///////////////////////////////////////////////////////////////////////////
    //  Mapping
    virtual bool         maprpcbuffer (const char* rpcbuf,
                                       int row, bool validate=true);
    ///////////////////////////////////////////////////////////////////
    //////////////////////////////////////
    //  MYSQL import optimization
    bool                 mysql_insert_optimize_start (bool append,
                                                      std::string cmdfile,
                                                      std::string datafile,
                                                      std::string livefile,
                                                      std::string schema="");
    bool                 mysql_insert_optimize_data (std::string datafile);
    //////////////////////////////////////
    //  SQLITE import optimization
    bool                 lite_insert_optimize_start (bool append,
                                                     std::string cmdfile,
                                                     std::string datafile,
                                                     std::string livefile,
                                                     std::string schema="");
    bool                 lite_insert_optimize_data (std::string datafile);
    //////////////////////////////////////
    //  Validation routine(s)
    virtual bool         valid_key (std::string column_name,
                                   std::string value);
    virtual bool         valid_flag (std::string column_name,
                                    std::string value);
    virtual bool         valid_time (std::string column_name,
                                    std::string value);
    virtual bool         valid_date (std::string column_name,
                                    std::string value);
    virtual bool         valid_email (std::string column_name,
                                     std::string value);
    virtual bool         valid_number (std::string column_name,
                                      std::string value);
    virtual bool         valid_decimal (std::string column_name,
                                       std::string value);
    virtual bool         valid_alphanum (std::string column_name,
                                        std::string value);
    ///////////////////////////////////////
    //  Hack - for JS scripts
    std::string          m_curpage,
                         m_listrows,
                         m_inisection;
    ///////////////////////////////////////////////////////////////////////////
    //  SQL generation
    virtual std::string  rpcargs    ();
    virtual int          arg_count  ();
    virtual int          add_count  ();
    virtual int          del_count  ();
    virtual int          upd_count  ();

    virtual bool         sql_count  (std::string& result,
                                     CYColumnBind* bind,
                                     bool is_setup=false);
    virtual bool         sql_exists (std::string& result,
                                     CYColumnBind* bind);
    virtual bool         sql_select (std::string& result,
                                     CYColumnBind* bind,
                                     bool is_setup=false);
    virtual bool         sql_insert (std::string& result,
                                     CYColumnBind* bind,
                                     std::string table="",
                                     bool bliteral = false);
    virtual bool         sql_delete (std::string& result,
                                     CYColumnBind* bind);
    virtual bool         sql_update (std::string& result,
                                     CYColumnBind* bind);
    virtual bool         sql_updateall (std::string& result,
                                        CYColumnBind* bind);
protected:
    ///////////////////////////////////////
    //  Attribute(s)
    bool                 is_setup;
    std::string          m_error,
                         m_bind_dump,
                         m_schema_table,
                         m_schema_title,
                         m_schema_welcome,
                         m_schema_req,
                         m_schema_lib,
                         m_schema_name,
                         m_schema_detail,
                         m_schema_rpcadd,
                         m_schema_rpcupd,
                         m_schema_rpcdel,
                         m_schema_loadtxt,
                         m_schema_category;
    ///////////////////////////////////////
    //  Column data
    CYColumn*            m_column[CYBINDVARMAX];
    ///////////////////////////////////////
    //  Dummy column
    CYColumnAlpha        m_col_dummy;
    ///////////////////////////////////////
    //  Utility function(s)
    void                 cleanup_bindargs (CYColumnBind* bind);
    std::string          cleanup_string (const char* str);
};
#endif
