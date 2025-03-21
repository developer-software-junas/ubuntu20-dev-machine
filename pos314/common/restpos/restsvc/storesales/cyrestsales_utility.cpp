//*******************************************************************
//        FILE:     cyrestsales_utility.cpp
// DESCRIPTION:     Utility function(s)
//*******************************************************************
#include "cyrestreceipt.h"
using std::string;
/***************************************************************************
      FUNCTION:          legacyTransnum
   DESCRIPTION:          Formattted transaction number
 ***************************************************************************/
string                   legacyTransnum (Json::Value jheader)
{
    char szTmp [256];
    string branch_code="0", transaction_number="0";
    if (jheader.isMember("branch_code"))
        branch_code = jheader["branch_code"].asString ();
    if (jheader.isMember("transaction_number"))
        transaction_number = jheader["transaction_number"].asString();

    long lTrans = atol(transaction_number.c_str ());
    long lBranch = atol(branch_code.c_str ());

    sprintf (szTmp, "%03ld-%09ld", lBranch, lTrans);
    std::stringstream ss;
    ss << szTmp;
    return ss.str ();
}
/***************************************************************************
      FUNCTION:          refundTransnum
   DESCRIPTION:          Formattted transaction number
 ***************************************************************************/
string                   refundTransnum (Json::Value jheader)
{
    char szTmp [256];
    string branch_code="0", register_number="0", transaction_number="0";
    if (jheader.isMember("branch_code"))
        branch_code = jheader["branch_code"].asString ();
    if (jheader.isMember("register_number"))
        register_number = jheader["register_number"].asString();
    if (jheader.isMember("transaction_number"))
        transaction_number = jheader["transaction_number"].asString();

    long lTrans = atol(transaction_number.c_str ());;
    long lBranch = atol(branch_code.c_str ());
    long lRegister = atol(register_number.c_str ());

    sprintf (szTmp, "%02ld-%03ld-%09ld", lBranch, lRegister, lTrans);

    std::stringstream ss;
    ss << szTmp;
    return ss.str ();
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
string CYRestCommon::FMTStrDotted(string str1,string str2)
{
    string res = "";
    int x = strlen(str1.c_str());
    int y = strlen(str2.c_str());
    int w = 40-x-y;

    res = str1.c_str();
    for (int i = 0; i < w; i++)
        res += ".";

    res += str2.c_str();
    res += "\n";

    return res;
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
string CYRestCommon::FMTNumberComma(double num)
{
    char sztmp[255];
    sprintf (sztmp, "%0.02f",num);
    string numComma = _util->fmt_number_comma(sztmp);

    return numComma;
}
//*******************************************************************
//    FUNCTION:     resetEodTotals
// DESCRIPTION:     Reset xread structure(s)
//*******************************************************************
string CYRestCommon::date_rcc(string date)
{
    string frm_date = "",mm,dd,yyyy;
    _util->date_split(date.c_str(),yyyy,mm,dd);
    frm_date = mm;
    frm_date += "/";
    frm_date += dd;
    frm_date += "/";
    frm_date += yyyy;


    return frm_date;
}
