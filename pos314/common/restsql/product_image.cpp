/********************************************************************
          FILE:         eod_retrieve.cpp
   DESCRIPTION:         Flutter REST for POS data
 ********************************************************************/
//////////////////////////////////////////
//  Header file(s)
#include <string>
#include <sstream>
#include "cyposrest.h"
using std::string;
using std::stringstream;
/***************************************************************************
      FUNCTION:          eod_retrieve
   DESCRIPTION:          Retrieve EOD totals
 ***************************************************************************/
std::string              cyposrest::sql_product_image()
{
    //////////////////////////////////////
    //  Retrieve argument(s)
    string imgurl = "";
    string imgdir = "";
    string filetype = cgiform("filetype");
    string filereference = cgiform("filereference");
    if (filetype == "recipe" || filetype == "category" || filetype == "subclass")  {
        imgurl = _ini->get_value("PATH","FNBURL");
        imgdir = _ini->get_value("PATH","FNBIMAGE");
    }  else  {
        return _util->json_error("Invalid image type requested for retrieval");
    }
    //////////////////////////////
    //  FNB image directory
    int idx2 = 0;
    string img = "";
    Json::Value jline, jresult;
    struct dirent *entry, **namelist;

    imgdir += filereference;
    imgdir += "/";
    int entries = scandir (imgdir.c_str (), &namelist, NULL, alphasort);
    if (entries >= 3)  {
        //////////////////////////////////////
        //  For each item
        int idxpath = 0;
        Json::Value jpath;
        for (int i = 0; i < entries; i++) {
            jline.clear();
            entry = namelist [i];
            //////////////////////////////////
            //  Type
            if (entry->d_type == DT_DIR) {
                jpath[idxpath]["name"] = entry->d_name;
                ++idxpath;
            }  else if (entry->d_type == DT_REG)  {
                //if (entry->d_name[0] == '.')  {
                    //++idxpath;
                //}  else  {
                    //  image url
                    img  = imgurl;
                    img += filereference;
                    img += "/";
                    img += entry->d_name;

                    jline["image"]=img;
                    jresult[idx2]=jline;
                    idx2++;
                //}
            }
        }
    }
    //////////////////////////////////////
    //  Spit it out
    Json::Value root;
    root["status"]="ok";
    root["numrows"]=idx2;
    root["numcols"]=1;
    root["resultset"]=jresult;

    stringstream ss;
    ss << root;
    return ss.str();
}

