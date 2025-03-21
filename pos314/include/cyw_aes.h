/********************************************************************
          FILE:          cyw_aes.h
   DESCRIPTION:          Encryption routine(s)
 ********************************************************************/
#ifndef __CYWARE_AES_H
#define __CYWARE_AES_H
//////////////////////////////////////////
//  Standard header file(s)
#include <string>
//////////////////////////////////////////
//  Encryption key
const std::string        CYHMACKEY      = "pjgabcarcis";
    //////////////////////////////////////////
    //  Encrypt/Decrypt using AES 256
    std::string         decode (const char* in,
                                const char* key=NULL);
    std::string         encode (const char* in,
                                const char* key=NULL);
    std::string         cywencrypt (const char* in);
    //////////////////////////////////////////
    //  Legacy code
    const char* legacyencrypt (const char* szStr,
                               bool unhex=true);
#endif
