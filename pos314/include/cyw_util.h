/*******************************************************************
          FILE:         cyw_util.h
   DESCRIPTION:         Utility class header
 *******************************************************************/
#ifndef __CYWARE__UTILITY_H
#define __CYWARE__UTILITY_H
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
//////////////////////////////////////////
//  Class header file(s)
#include "cyw_mem.h"
#include "cyw_log.h"
#include "json/json.h"
//////////////////////////////////////////
//  Flutter SHA256 encoder
std::string CalcHmacSHA256(std::string  msg, std::string decodedKey);
//////////////////////////////////////////
//  SSH copy method
#ifdef _CYSSH
#include <libssh/sftp.h>
int sftp_read_async(ssh_session session, sftp_session sftp,
                    std::string infile, std::string outfile);
#endif
const char g_fmtLong [] = "%ld";
/***************************************************************************
         CLASS:         cyutil
   DESCRIPTION:         Utility class definition
 ***************************************************************************/
class                   cyutility
{
public:
    ///////////////////////////////////////
    //  Constructor/Destructor
    cyutility ();
    virtual ~cyutility ();
    ///////////////////////////////////////
    //  Error related method(s)
    void                errorclear ();
    std::string         errormsg ();
    ///////////////////////////////////////
    //  REST key verification
    bool                parseandverify (std::string strjson,
                                         Json::Value& root);
    //////////////////////////////////////
    //  SSH method(s)
#ifdef _CYSSH
    bool                ssh_login (std::string user, std::string pass,
                                   std::string host, std::string port);
    bool                sftp_send (std::string filename,
                                   std::string user, std::string pass,
                                   std::string host, std::string port,
                                   std::string dirsrc, std::string dirdest);
    bool                sftp_recv (std::string filename,
                                   std::string user, std::string pass,
                                   std::string host, std::string port,
                                   std::string dirsrc, std::string dirdest);
#endif
    ///////////////////////////////////////
    //  Utility Method(s)
    std::string         urldecode(const char* url);
    std::string         urlencode(const char* url);
    std::string         base64encode(std::string value);
    std::string         base64decode(std::string value);
    bool                base64decode(struct_cymem *mem, int len);
    std::string         string_cleanse (const char* str,
                                        bool remove_quote=true);
    std::string         shopify_sha256 (std::string query,
                                        std::string secret);
    std::vector<std::string> string_tokenize(std::string const &in,
                                             const char* sep);
    //////////////////////////////////////
    //  JSON related
    std::string         jsonvalue (const char* name,
                                   const char* buffer,
                                   const char* defvalue=NULL,
                                   bool partial=false);
    std::string         jsonok    (std::string errfunc,
                                   std::string errmsg="");
    std::string         jsonerror (std::string errfunc,
                                   std::string errmsg="");
    std::string         json_error(std::string errfunc);
    //////////////////////////////////////
    //  Json validation
    bool                json_array (int errnum,
                                    Json::Value jval);
    bool                json_string (int errnum,
                                     Json::Value jval,
                                     std::string& value);
    bool                json_object (int errnum,
                                     Json::Value jval);
    //////////////////////////////////////
    //  Date function(s)
    std::string         int_time ();
    std::string         int_timesec ();

    std::string         date_pos ();
    std::string         date_pos_elena ();

    std::string         strtime_t  ();
    std::string         time_eight ();
    std::string         time_eight_colon ();

    std::string         date_eight ();
    std::string         date_sequence ();
    std::string         date_string2time_t (std::string strDate,
                                            std::string strTime);

    std::string         date_age   (const char* dt);
    std::string         date_dash  (const char* dt,
                                    const char* dash="_");
    std::string         date_mysql ();
    std::string         date_mysql (int year, int month, int day, bool start);
    std::string         date_dash_reverse (const char* dt,
                                           const char* dash="_");
    std::string         date_dash_tonum (const char* dt);
    std::string         date_slash_tonum (const char* dt);
    std::string         date_gijgo_tonum (const char* dt);

    bool                int_date (std::string& str, int days);
    bool                int_date_add (std::string& str, int days);
    bool                datetonum (std::string dt, std::string& result);

    bool                date_split (const char* strdate,
                                    std::string& year,
                                    std::string& month,
                                    std::string& day);

    bool                time_split (const char* strtime,
                                    std::string& hour,
                                    std::string& minute,
                                    std::string& second);
    std::string         time_colon (const char* val);
    std::string         time_colon_tonum (const char* val);
    std::string         timestamp  (int timeout, long& lexpire);

    std::string         date_nice  (const char* strdate);
    std::string         date_nicer (const char* strdate);
    //////////////////////////////////////
    //  Vaidation routines
    bool                 valid_email    (const char* name,
                                         const char* email,
                                         unsigned int minlen, unsigned int maxlen);

    bool                 valid_phone    (const char* name,
                                         const char* phone,
                                         unsigned int minlen, unsigned int maxlen);

    bool                 valid_number   (std::string name,
                                         std::string number,
                                         unsigned int minlen, unsigned int maxlen,
                                         long lminval=0L, long lmaxval=999999999L);

    bool                 valid_decimal  (const char* name,
                                         const char* decimal,
                                         unsigned int minlen, unsigned int maxlen,
                                         double dminval=0, double dmaxval=999999999.99);

    bool                 valid_size (const char* name,
                                     const char* alphanum,
                                     unsigned int minlen, unsigned int maxlen);

    bool                 valid_alpha (const char* name,
                                      const char* alpha,
                                      unsigned int minlen, unsigned int maxlen);

    bool                 valid_alphanum (const char* name,
                                         const char* alphanum,
                                         unsigned int minlen,
                                         unsigned int maxlen, const char* extrachars=nullptr);

    bool                 valid_column (const char* name,
                                       const char* alphanum,
                                       unsigned int minlen, unsigned int maxlen);

    bool                 valid_password (const char* name,
                                         const char* passwd,
                                         unsigned int minlen, unsigned int maxlen);

    bool                 valid_generic_field (const char* name, std::string value,
                                              unsigned int minlen, unsigned int maxlen,
                                              bool balpha, bool bnumeric, const char* addlchar);
    //////////////////////////////////////
    //  Safe convenience function(s)
    double               stodsafe (std::string val);
    std::string          safeJson (const Json::Value jval,
                                   const std::string jkey);
    //////////////////////////////////////
    //  Conversion routines
    std::string          booltostring (bool bval);
    std::string          longtostring (long lval);
    std::string          doubletostring (double dval);

    std::string          lowerCase (std::string val);
    std::string          columnLabel (std::string val);
    std::string          lowerCamelCase (std::string val);

    std::string          stringtohex  (const char* src,
                                       size_t len);
    std::string          string_yesno (const char* str);
    std::string          string_upper (const char* str);
    std::string          string_lower (const char* str);
    std::string          string_inclause (const char* str);

    std::string          fmt_number_comma (const char* str);
    std::string          fmt_number  (int zeropad, const char* str);
    std::string          fmt_decimal (int decimal,const char* str);
    std::string          cut_decimal (int decimal,const char* str);
    //////////////////////////////////////
    //  Receipt method(s)
    std::string          cyPreSpace (size_t width, std::string text);
    std::string          cyPostSpace (size_t width, std::string text);
    std::string          cyCenterText (size_t width, std::string str);
    std::string          cyAcrossText (size_t width, std::string text);
    std::string          cyLRText (size_t wLeft, std::string left, size_t wRight, std::string right);
    std::string          cyLRText2(size_t wLeft, std::string left, size_t wRight, std::string right);
    //////////////////////////////////////
    //  File operation(s)
    bool                file_load (const char* filename,
                                   std::string& contents);
    bool                file_create (const char* filename,
                                     std::string contents);
    bool                file_exists (const char* filename);
    bool                file_delete (const char* filename);
    size_t              file_size   (const char* filename,
                                     struct_cymem* mem=nullptr);
    bool                file_dirlist(const char* directory,
                                     std::string& rpclist);
    ///////////////////////////////////////
    //  Random data generation
    virtual char         random_ch (int seed);
    virtual bool         random_number (int seed,
                                        std::string& result,
                                        int length = 1);

    virtual std::string  random_session  (int len);
    virtual std::string  random_password (int len);
    //////////////////////////////////////
    //  Random date generation
    int                  random_min       ();
    int                  random_hour      ();
    int                  random_month     ();
    bool                 isleapyear       (int year);
    int                  random_day       (int year, int month);
    int                  random_year      (int startyear, int endyear);
    void                 random_date      (int startyear, int endyear,
                                           int& yy, int& mm, int& dd,int& hh, int& mn,
                                           std::string& strdate, std::string& strtime);
    virtual std::string  rpc_value (const char* name,
                                    const char* buffer,
                                    const char* defvalue="",
                                    bool partial=false);
    virtual std::string  rpc_value_row (int row,
                                        const char* name,
                                        const char* buffer,
                                        const char* defvalue="",
                                        bool partial=false);
protected:
    ///////////////////////////////////////
    //  Error message string
    std::string         error;
    cylog*              _log;
};
#endif
