/*****************************************************************************
          FILE:         column.h
   DESCRIPTION:         Column header file
           LOG:         09-27-14 initial creation
 *****************************************************************************/
#ifndef __CYWARE_CYCOLUMN
#define __CYWARE_CYCOLUMN
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include "col/column_enum.h"
/***************************************************************************
         CLASS:         CYColumn
   DESCRIPTION:         Column data class
 ***************************************************************************/
class                   CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumn ();
    virtual ~CYColumn ();
    //////////////////////////////////////
    //  Operator(s)
    void                operator=(const CYColumn& col);
    bool                isreplica(const CYColumn& col);
    //////////////////////////////////////
    //  Column reset
    void                clear ();
    //////////////////////////////////////
    //  Pure virtual function(s)
    virtual std::string get_value    (bool defval=false)=0;
    virtual std::string get_defvalue ()=0;

    virtual bool        validate     (std::string value)=0;
    virtual bool        set_value    (std::string value,
                                      bool validation=true)=0;
    virtual bool        set_defvalue (std::string defvalue)=0;
    ///////////////////////////////////////
    //  SQL method(s)
    std::string         sql_insert ();
    ///////////////////////////////////////
    //  HTML input
    virtual std::string input_hidden(int offset,
                                     const char* suffix=NULL);

    virtual std::string input_label (int offset,
                                     bool required,
                                     bool readonly,
                                     bool autofocus,
                                     const char* type,
                                     const char* prompt,
                                     const char* suffix=NULL);

    virtual std::string input_display (int offset,
                                       bool required,
                                       bool readonly,
                                       bool autofocus,
                                       const char* type,
                                       const char* prompt,
                                       const char* suffix=NULL);

    virtual std::string input_nolabel(int offset,
                                      bool required,
                                      bool readonly,
                                      bool autofocus,
                                      const char* type,
                                      const char* prompt,
                                      const char* suffix=NULL);

    virtual std::string input_mobile(int offset,
                                     bool required,
                                     bool readonly,
                                     bool autofocus,
                                     const char* type,
                                     const char* prompt,
                                     const char* suffix=NULL);

    virtual std::string input_number (int offset,
                                      bool required,
                                      bool readonly,
                                      bool autofocus,
                                      const char* step,
                                      const char* prompt,
                                      const char* suffix=NULL);

    virtual std::string input_matrix (int offset,
                                      bool required,
                                      bool readonly,
                                      bool autofocus,
                                      const char* step,
                                      const char* prompt,
                                      const char* suffix=NULL);

    virtual std::string input_select (int offset,
                                      bool required,
                                      bool readonly,
                                      bool autofocus,
                                      std::string& sel,
                                      const char* rpcbuf,
                                      const char* suffix=NULL,
                                      const char* drop_field=NULL,
                                      const char* drop_script=NULL);

    virtual std::string rpc_value (const char* name,
                                   const char* buffer,
                                   const char* defvalue,
                                   bool partial);

    virtual std::string rpc_value_row (int row,
                                       const char* name,
                                       const char* buffer,
                                       const char* defvalue,
                                       bool partial);

    int                  _col_seq,
                         _col_case,
                         _col_type,
                         _col_minlen,
                         _col_maxlen,
                         _col_decimal,
                         _col_zeropad;

    long double          _col_minval,
                         _col_maxval;

    bool                 _col_key,
                         _col_sort,
                         _col_insert,
                         _col_select,
                         _col_update,
                         _col_display,
                         _col_select_where,
                         _col_update_where,
                         _col_delete_where,
                         _col_display_search;

    std::string          _schema_name,
                         _column_name,
                         _col_label,
                         _col_defvalue,
                         _col_dropsql,
                         _col_drop_field,
                         _col_drop_script,
                         _col_searchvalue,
                         _col_display_css,
                         _col_display_pattern;

    int                  _case;
    ENUM_COLTYPE         _type;

    std::string          _sql,
                         _temp,
                         _value;
    std::stringstream    _error;
};



/***************************************************************************
         CLASS:         CYColumnAlpha
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnAlpha : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnAlpha ();
    virtual ~CYColumnAlpha ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnAlphaKey
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnAlphaKey : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnAlphaKey (int maxlen = 10);
    virtual ~CYColumnAlphaKey ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnNumber
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnNumber : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnNumber ();
    virtual ~CYColumnNumber ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnDate
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnDate : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnDate ();
    virtual ~CYColumnDate ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
protected:
    int                 year, month, day;
};


/***************************************************************************
         CLASS:         CYColumnTime
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnTime : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnTime ();
    virtual ~CYColumnTime ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
protected:
    int                 hour, minute;
};


/***************************************************************************
         CLASS:         CYColumnYesNo
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnYesNo : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnYesNo ();
    virtual ~CYColumnYesNo ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};

/***************************************************************************
         CLASS:         CYColumnDropParent
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnDropParent : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnDropParent ();
    virtual ~CYColumnDropParent ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnDefault
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnDefault : public CYColumn
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnDefault ();
    virtual ~CYColumnDefault ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnEmail
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnEmail : public CYColumnAlpha
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnEmail ();
    virtual ~CYColumnEmail ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnMobile
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnMobile : public CYColumnAlpha
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnMobile ();
    virtual ~CYColumnMobile ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnColor
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnColor : public CYColumnAlpha
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnColor ();
    virtual ~CYColumnColor ();

    //////////////////////////////////////
    //  Override(s)
    virtual std::string get_value    (bool defval=false);
    virtual std::string get_defvalue ();

    virtual bool        validate     (std::string value);
    virtual bool        set_value    (std::string value,
                                      bool validation=true);
    virtual bool        set_defvalue (std::string defvalue);
};


/***************************************************************************
         CLASS:         CYColumnSequence
   DESCRIPTION:         Column data class
 ***************************************************************************/

class                   CYColumnSequence : public CYColumnAlpha
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnSequence ();
    virtual ~CYColumnSequence ();
};
#endif
