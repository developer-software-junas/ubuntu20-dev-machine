/*****************************************************************************
          FILE:         column_enum.h
   DESCRIPTION:         Column enumerated type(s)
 *****************************************************************************/
#ifndef __ENUMCOLUMN
#define __ENUMCOLUMN
#include <stdio.h>
#include <stdlib.h>
//////////////////////////////////////////
//  Typedefs, enums and constants
///////////////////////////////////////////
//  Constant(s)
typedef enum
{
  js_uppercase,
  js_lowercase,
  js_nocase,
  js_propercase,
  js_unknowncase
}  JSCONST_CASE;
/*
typedef enum  {
    js_uppercase,
    js_lowercase,
    js_nocase,
    js_unknown
}  ENUM_COLCASE;
///////////////////////////////////////////
//  Constant(s)
typedef enum
{ js_uppercase,
  js_lowercase,
  js_nocase,
  js_propercase,
  js_unknowncase
}  ENUM_COLCASE;*/
typedef enum  {
    COLTYPE_ALPHA,
    COLTYPE_NUMERIC,
    COLTYPE_DECIMAL,
    COLTYPE_PASSWD,
    COLTYPE_YESNO,
    COLTYPE_DEFAULT,
    COLTYPE_DATE,
    COLTYPE_TIME,
    COLTYPE_DROPALPHA,
    COLTYPE_DROPNUMERIC,
    COLTYPE_DROPPARENT,
    COLTYPE_EMAIL,
    COLTYPE_MOBILE,
    COLTYPE_SESSION,
    COLTYPE_COLOR,
    COLTYPE_SEQUENCE,
    COLTYPE_UNKNOWN
}  ENUM_COLTYPE;

typedef enum  {
    COLSEARCH_PLAIN,
    COLSEARCH_REQUIRED,
    COLSEARCH_HIDDEN,
    COLSEARCH_UNKNOWN
}  ENUM_COLSEARCH;
#endif
