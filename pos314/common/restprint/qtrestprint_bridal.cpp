/********************************************************************
          FILE:         cyposrest_bridal.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cyposrest.h"

#include <string>
#include <sstream>
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:          rest_bridal
   DESCRIPTION:          Get bridal information
 ***************************************************************************/
std::string              cyposrest::rest_bridal ()
{
    char szfunc [] = "RestBridal";
    //////////////////////////////////////
    //  JSON object(s)
    Json::Value          root;
    ///////////////////////
    int arridx = 0;
    Json::Value jline, jitem, jcust;
    string cnum, fname, lname, mname, rgcusttype;
    //////////////////////////////////////
    //  Retrieve argument(s)
    cnum       = _cgi->form("registry_number");
    fname      = _cgi->form("first_name");
    lname      = _cgi->form("last_name");
    mname      = _cgi->form("mid_name");
    rgcusttype = _cgi->form("custtype");
    //////////////////////////////////////
    //  Validate
    if(cnum.length() <= 0)
        return _util->jsonerror(szfunc,"The registry number is a required entry");
    //  Test only HACK
    jcust["m_szCustId"]="8888";
    jcust["m_szCustType"]="GT";
    jcust["m_szRegId"]=cnum;
    jcust["m_szFirst"]="Jean";
    jcust["m_szMiddle"]="Paul";
    jcust["m_szLast"]="Sartre";
    jcust["m_nBirthDate"]="07051947";
    jcust["m_nStartDate"]="01012024";
    jcust["m_nEndDate"]="03152024";
    jcust["m_szAcctType"]="BR";
    jcust["m_szFirst2"]="Simone";
    jcust["m_szLast2"]="De Beauvoir";
    arridx = 0;
    jitem = Json::nullValue;
    //////////////////////////////////////
    //  Assemble all JSON object(s)
    root["status"]  ="ok";
    root["customer"]=jcust;
    root["itemrows"]=arridx;
    root["items"]=jitem;
    //////////////////////////////////////
    //  Spit it out...
    stringstream ss;
    ss << root;

    return ss.str ();



    //////////////////////////////////////
    //  Registry URL
    string registry_url =    _ini->get_value("REGISTRY","URL");
    if (registry_url.length() < 1)
        registry_url = "http://129.37.2.67/cgi-bin/registry.cgi";

    string registrypost;


    registrypost = "?state=state_search";

    registrypost += "&cnum=";
    registrypost += cnum;

    registrypost += "&fname=";
    registrypost += fname;

    registrypost += "&mname=";
    registrypost += mname;

    registrypost += "&lname=";
    registrypost += lname;
    registrypost += "&rgcusttype=";
    registrypost += rgcusttype;




    //Uncomment for test response
    //ret_curl = "<cyware>0|100309|GT|043157|SAMPLE||REGISTRATION|20230109|20220713|20230123|BR|.|.|2229685;1;0:2659070;1;0";
    registry_url += registrypost;

    string tp = httpget(registry_url);

    if(tp.length() <= 0 )
        return _util->jsonerror(szfunc,"No Response From Registry Server, Pls check Connection....");

    tp.replace(0,8,"");
    tp.replace( tp.length()-9,9,"");


    int j=0;
    string delimiter = "|";
    string item_delimiter = ";";
    size_t pos = 0;
    string token, reg_items, reg_return ;


    if((pos = tp.find(delimiter)) != std::string::npos){

        token = tp.substr(0, pos);


        if( token == "0"){

            tp += delimiter;
             //parse comma delimited return
            while ((pos = tp.find(delimiter)) != std::string::npos) {
                token = tp.substr(0, pos);

                if (j == 0)
                    reg_return = token;
                else if (j == 1)
                    jcust["m_szCustId"]=token;
                else if (j == 2)
                    jcust["m_szCustType"]=token;
                else if (j == 3)
                    jcust["m_szRegId"]=token;
                else if (j == 4)
                    jcust["m_szFirst"]=token;
                else if (j == 5)
                    jcust["m_szMiddle"]=token;
                else if (j == 6)
                    jcust["m_szLast"]=token;
                else if (j == 7)
                    jcust["m_nBirthDate"]=token;
                else if (j == 8)
                    jcust["m_nStartDate"]=token;
                else if (j == 9)
                    jcust["m_nEndDate"]=token;
                else if (j == 10)
                    jcust["m_szAcctType"]=token;
                else if (j == 11)
                    jcust["m_szFirst2"]=token;
                else if (j == 12)
                    jcust["m_szLast2"]=token;
                else if (j == 13)
                    reg_items = token;

                j++;
                tp.erase(0, pos + delimiter.length());
            }

            /////////////////////////////////
            //parse registry items
            tp = reg_items;
            tp += ":";
            j=0;
            delimiter = ":";
            //pos = 0;

            string itemtok, item, qty, qty_sold;
            size_t itempos;

            if(tp.find(":") != std::string::npos && tp.find(";") != std::string::npos){

                ///////////////
                //loop on items
                while ((pos = tp.find(delimiter)) != std::string::npos) {
                    itemtok = tp.substr(0, pos);


                    if(itemtok.find(";") != std::string::npos){

                       itemtok += item_delimiter;

                        while ((itempos = itemtok.find(item_delimiter)) != std::string::npos) {
                           token = itemtok.substr(0, itempos);

                           if (j == 0)
                                item = token;
                           else if (j == 1)
                                qty= token;
                           else if (j == 2)
                                qty_sold= token;

                           j++;
                           itemtok.erase(0, itempos + item_delimiter.length());
                        }

                        ///////////////////////////////////////////
                        // add to item json array
                        jline["item"]=item;
                        jline["qty"]=qty;
                        jline["qty_sold"]=qty_sold;
                        jitem[arridx]=jline;
                        arridx++;


                    }

                    tp.erase(0, pos + delimiter.length());
                }




            }// if(tp.find(":") !=
            /////////////////////////////////////////






        }



    }  else  {
        //////////////////////////////////
        //  Parse fail
        return _util->jsonerror("Parsing failed", tp);
    }

    //////////////////////////////////////
    //  Assemble all JSON object(s)
    root["status"]  ="ok";
    root["customer"]=jcust;
    root["itemrows"]=arridx;
    root["items"]=jitem;
    //////////////////////////////////////
    //  Spit it out...
    //stringstream ss;
    ss << root;

    return ss.str ();
}
