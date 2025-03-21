/********************************************************************
          FILE:         cypublish.cpp
   DESCRIPTION:         Publish implementation
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cypublish.h"

#include <string>
#include <sstream>
#include "cyw_aes.h"
using std::string;
using std::stringstream;
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
bool                     CYPublish::processSales ()
{
    //////////////////////////////////////
    //  Directory file list
    struct dirent *entry, **namelist;
    string salesdir = _ini->get_value("PATH","SALES");
    //////////////////////////////////////
    //  Get the list of directories
    int entries = scandir (salesdir.c_str (), &namelist, NULL, alphasort);
    if (entries >= 3)  {
        //////////////////////////////////////
        //  For each item
        int idxpath = 0;
        Json::Value jpath;
        for (int i = 0; i < entries; i++) {
            entry = namelist [i];
            //////////////////////////////////
            //  Type
            if (entry->d_type == DT_DIR) {
                jpath[idxpath]["name"] = entry->d_name;
                ++idxpath;
            }  else if (entry->d_type == DT_REG)  {
                //////////////////////////////
                //  Unzip the file
                string cmd = "/usr/bin/unzip -d ";
                cmd += salesdir;
                cmd += " ";
                cmd += salesdir;
                cmd += "/";
                cmd += entry->d_name;
                system (cmd.c_str ());
                publishLog ("file unzipped",cmd.c_str ());
                //////////////////////////////////////
                //  sales file
                string salesfile = salesdir;
                salesfile += "/"; salesfile += "sales.sqlite";
                if (_util->file_exists(salesfile.c_str()))  {
                    string destdir = salesdir;
                    string zipFile = salesdir;
                    zipFile += "/"; zipFile += entry->d_name;
                    if (importSales(salesfile))  {
                        destdir += "/done";
                    }  else  {
                        destdir += "/error";
                    }
                    //  Move the original zip file
                    cmd = "mv "; cmd += zipFile; cmd += " "; cmd += destdir;
                    system (cmd.c_str ());
                    publishLog ("file processed and moved",cmd.c_str ());
                    //  Remove the salesfile
                    _util->file_delete(salesfile.c_str());
                }
            }
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
bool                     CYPublish::importSales (string salesfile)
{
    //////////////////////////////////////
    //  connect
    SqliteEnv* env = nullptr;
    env = new SqliteEnv;
    if (!env->connect ("localhost","sa","admin",salesfile,0))  {
        return false;
    }
    //////////////////////////////////////
    //  Connect to the database
    string sql;
    SqliteDb db (env);
    SqliteDb db2(env);
    CYDbSql*  dbmaster = _ini->dbconnini();

    string type="";
    string is_polled="";
    string systransnum = "";

    sql = "select * from tg_pos_offline_sales  ";
    if (!db.sql_result(sql,false))
        return false;
    if (!db.eof ())  {
        do  {
            type = db.sql_field_value("type");
            sql = db.sql_field_value("sqldata");
            is_polled = db.sql_field_value("is_polled");
            systransnum = db.sql_field_value("systransnum");
            if (is_polled != "888")  {
                if (dbmaster->sql_only(sql,false))
                    return false;
                if (!dbmaster->sql_commit())
                    return false;

                sql  = "update tg_pos_offline_sales set is_polled = 888 ";
                sql += "where  systransnum = '"; sql += systransnum; sql += "' ";
                sql += " and   type = '"; sql += type; sql += "' ";
                if (!db2.sql_only(sql,false))
                    return false;
                if (!db2.sql_commit())
                    return false;
            }
            db.sql_next();
        } while (!db.eof());
    }
    return true;
}
