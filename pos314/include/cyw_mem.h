/*******************************************************************
          FILE:         cyw_mem.h
   DESCRIPTION:         Memory routines
 *******************************************************************/
#ifndef __CYWARE_MEM__H
#define __CYWARE_MEM__H
#include <stdlib.h>
#include <stddef.h>
//////////////////////////////////////////
//  Standard header file(s)
#define                 CYMEMDEFSIZE  8192
    //////////////////////////////////////
    //  Memory structure
    typedef struct  {
        int             ok;
        char            *memory;
        size_t          pos, size;
    } struct_cymem;
    //////////////////////////////////////
    //  Pagination structure
    typedef struct  {
        char            data[128][4096];
    } struct_cyhdr;
    //////////////////////////////////////
    //  Function prototype(s)
    void                cymem_clean   (struct_cymem* cymem);
    void                cymem_reset   (struct_cymem* cymem);
    void                cymem_cleanse (struct_cymem* cymem);

    void                cymem_init    (struct_cymem* cymem, size_t size);
    void                cymem_grow    (struct_cymem* cymem, size_t size);
    void                cymem_size    (struct_cymem* cymem, size_t size);
    int                 cymem_append  (struct_cymem* cymem,const char* str);
#ifdef _CYMEMCURL
    //////////////////////////////////////
    //  HTTP function(s)
    bool                cymem_https_get (int port, int timeout, const char* host,
                                         const char* args, struct_cymem* mem);
    bool                cymem_https_post(int port, int timeout, const char* host,
                                         const char* args, struct_cymem* mem);
    bool                cymem_http_file (int port,
                                         const char* host,
                                         const char* args,
                                         const char* file,
                                         struct_cymem* mem);
#endif
    //////////////////////////////////////////
    //  Self cleaning class
    class cymem  {
      public:
        cymem (size_t size=512) { cymem_init (&_mem, size); }
        ~cymem () { cymem_clean(&_mem); }
        struct_cymem _mem;
    };
#endif
