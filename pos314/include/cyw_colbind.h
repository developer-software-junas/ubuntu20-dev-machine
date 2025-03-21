/*******************************************************************
   FILE:                 cyw_colbind.h
   DESCRIPTION:          Column bind header
 *******************************************************************/
#ifndef __CYCOLBIND_H
#define __CYCOLBIND_H
//////////////////////////////////////////
//  Standard headers
#include <string.h>
#include <string>
#include "cyw_mem.h"
//////////////////////////////////////////
//  Maxiumum bind variable count
const int CYBINDVARMAX = 512;
/***************************************************************************
         CLASS:          CYColumnBind
   DESCRIPTION:          Bind column list class definition
 ***************************************************************************/
class                    CYColumnBind
{
public:
    //////////////////////////////////////
    //  Constructor/Destructor
    CYColumnBind ();
    virtual ~CYColumnBind ();
    ///////////////////////////////////////
    //  Bind variables
    int                  m_count;
    unsigned long        m_length;
    std::string          m_bindchar;
    struct_cymem         m_value [4096];
};
#endif
