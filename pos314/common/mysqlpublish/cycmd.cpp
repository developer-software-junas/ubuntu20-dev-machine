/********************************************************************
          FILE:         cycmd.cpp
   DESCRIPTION:         Command line class implementation
 ********************************************************************/
//////////////////////////////////////////
//  REST class
#include "cycmd.h"
using std::endl;
using std::string;
using std::stringstream;
/********************************************************************
      FUNCTION:         Constructor
   DESCRIPTION:         Initialize
 ********************************************************************/
CYCommandLine::CYCommandLine (cyini* ini, cylog* log, cyutility* util)
{
    _ini = ini;
    _log = log;
    _error = "";
    _util  = util;
}
/********************************************************************
      FUNCTION:         Destructor
   DESCRIPTION:         Class cleanup
 ********************************************************************/
CYCommandLine::~CYCommandLine ()
{
}
/*******************************************************************
      FUNCTION:          errormsg
   DESCRIPTION:          Get the error message
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
string                   CYCommandLine::errormsg ()
{
    return _error;
}
/*******************************************************************
      FUNCTION:          cleanupFiles
   DESCRIPTION:          Remove work file(s)
     ARGUMENTS:          none
       RETURNS:          the error string
 *******************************************************************/
bool                     CYCommandLine::cleanupFiles()
{
    //////////////////////////////////////
    //  Directory file list
    struct dirent *entry,
                  **namelist;
    string imgdir = "/usr/local/data/publish";
    //////////////////////////////////////
    //  Get the list of directories
    int entries = scandir (imgdir.c_str (), &namelist, NULL, alphasort);
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
                string tmp = "/usr/local/data/publish/";
                tmp += entry->d_name;
                _util->file_delete(tmp.c_str());
            }
        }
    }
    return true;
}
/*******************************************************************
      FUNCTION:          createSchema
   DESCRIPTION:          Generate the table(s) / index(es)
     ARGUMENTS:          none
       RETURNS:          true on succes
 *******************************************************************/
bool                     CYCommandLine::generateSchema (string locationCode,
                                                        string logicalDate)
{
    (void)logicalDate; (void)locationCode;
    //////////////////////////////////////
    //  Publish path
    string path = _ini->get_value("PATH","PUBLISH"); path += "/";
    //////////////////////////////////////
    //  Database settings
    string host = _ini->get_value("default","HOST");
    string user = _ini->get_value("default","USER");
    string name = _ini->get_value("default","NAME");
    string pass = decode (_ini->get_value("default","PASS").c_str ());
    //////////////////////////////////////
    //  Save the password to allow the
    //  MYSQL dump to be unattended
    stringstream ss;
    ss << "[client]" << endl
       << "password=\"" << pass << "\"" << endl
       << "[mysqldump]" << endl
       << "password=\"" << pass << "\"" << endl
       << "user=\"" << user << "\"" << endl;

    string passFile  = path;
           passFile += ".mypass";
    if (!_util->file_create(passFile.c_str (),ss.str()))  {
        _error = _util->errormsg();
        return false;
    }
    //////////////////////////////////////
    //  Output file
    string fileName = path;
    fileName += "createdb.sql";
    //////////////////////////////////////
    //  MYSQL dump
//#ifdef _APPLE_
    //string dumpCmd  = "/opt/homebrew/opt/mysql-client@5.7/bin/mysqldump --no-data -u ";
    //string dumpCmd  = "/opt/homebrew/opt/mysql-client/bin/mysqldump --defaults-extra-file=";
//#else
    string dumpCmd  = "/usr/bin/mysqldump --defaults-extra-file=";
           dumpCmd += passFile; dumpCmd += " --no-data -u ";
//#endif
           dumpCmd += user; dumpCmd += " -h "; dumpCmd += host;
           dumpCmd += " "; dumpCmd += name; dumpCmd += " >> ";
           dumpCmd += fileName;
    _log->logmsg("DUMP",dumpCmd.c_str ());

    int ret = system(dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
    }
    return true;
}
/*******************************************************************
      FUNCTION:          createSchema
   DESCRIPTION:          Generate the table(s) / index(es)
     ARGUMENTS:          none
       RETURNS:          true on succes
 *******************************************************************/
bool                     CYCommandLine::importSchema (string locationCode,
                                                      string logicalDate)
{
    //////////////////////////////////////
    //  Publish path
    string path = _ini->get_value("PATH","PUBLISH"); path += "/";
    //////////////////////////////////////
    //  Database settings
    string host = _ini->get_value("default","HOST");
    string user = _ini->get_value("default","USER");
    string name = _ini->get_value("default","NAME");
    string pass = decode (_ini->get_value("default","PASS").c_str ());
    //////////////////////////////////////
    //  Save the password to allow the
    //  MYSQL dump to be unattended
    stringstream ss;
    ss << "[client]" << endl
       << "password=\"" << pass << "\"" << endl
       << "[mysqldump]" << endl
       << "password=\"" << pass << "\"" << endl
       << "user=\"" << user << "\"" << endl;

    string passFile  = path;
           passFile += ".mypass";
    if (!_util->file_create(passFile.c_str (),ss.str()))  {
        _error = _util->errormsg();
        return false;
    }
    //////////////////////////////////////
    //  Output file
    string fileName = path;
    fileName += locationCode; fileName += ".";
    fileName += logicalDate; fileName += ".sql";
    //////////////////////////////////////
    //  MYSQL dump
//#ifdef _APPLE_
//    string dumpCmd  = "/opt/homebrew/opt/mysql-client/bin/mysql --defaults-extra-file=";
//#else
    string dumpCmd  = "/usr/bin/mysql --defaults-extra-file=";
//#endif
           dumpCmd += passFile; dumpCmd += " --local_infile=1  -u ";
           dumpCmd += user; dumpCmd += " -h localhost";
           dumpCmd += " "; dumpCmd += name; dumpCmd += " < ";
           dumpCmd += fileName;
    _log->logmsg("DUMP",dumpCmd.c_str ());

    int ret = system(dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
    }
    //////////////////////////////////////
    //  Compress the database
    if (!compressMaster(locationCode,logicalDate))
        return false;
    return true;
}
/*******************************************************************
      FUNCTION:          generateSqlite
   DESCRIPTION:          Generate the table(s) / index(es)
     ARGUMENTS:          none
       RETURNS:          true on succes
 *******************************************************************/
bool                     CYCommandLine::generateSqlite(std::string locationCode,
                                                       std::string logicalDate)
{
    //////////////////////////////////////
    //  Publish path
    string path = _ini->get_value("PATH","PUBLISH"); path += "/";
    //////////////////////////////////////
    //  Output file
    string outfile, fileName = path;
    fileName += locationCode; fileName += "."; fileName += logicalDate;
    outfile = fileName;  outfile += ".qtdb";
    fileName += ".sqlite";

    _util->file_delete(outfile.c_str ());
    string tmp = outfile; tmp += ".gz";
    //////////////////////////////////////
    //  SQLITE
    string dumpCmd  = "/usr/bin/sqlite3 "; dumpCmd += outfile;
    dumpCmd += " < "; dumpCmd += fileName;
    _log->logmsg("DUMP",dumpCmd.c_str ());

    int ret = system(dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
    }
    //////////////////////////////////////
    //  Compress the database
    dumpCmd  = "/usr/bin/gzip -f ";
    dumpCmd += outfile;

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    string destDir = _ini->get_value("PATH","DOCROOT");
    dumpCmd  = destDir;
    dumpCmd += locationCode; dumpCmd += ".";
    dumpCmd += logicalDate; dumpCmd += ".qtdb.gz";
    _util->file_delete(dumpCmd.c_str ());

    dumpCmd  = "mv ";
    dumpCmd += outfile;
    dumpCmd += ".gz ";
    dumpCmd += destDir;

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    dumpCmd  = "rm ";
    dumpCmd += fileName;
    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    dumpCmd  = "rm ";
    dumpCmd += path;
    dumpCmd += "*.sql";
    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }
    return true;
}
/*******************************************************************
      FUNCTION:          compressMaster
   DESCRIPTION:          Compress the masterfile
     ARGUMENTS:          none
       RETURNS:          true on succes
 *******************************************************************/
bool                     CYCommandLine::compressMaster(std::string locationCode,
                                                       std::string logicalDate)
{
    //////////////////////////////////////
    //  Go to the export directory
    string dumpCmd = "";
    string path = _ini->get_value("PATH","PUBLISH");
    path += "/";
    dumpCmd  = "cd ";
    dumpCmd += path;
    int ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }
    //////////////////////////////////////
    //  Compress generated files
    string tmp, fileName = path;
    fileName += locationCode; fileName += ".";
    fileName += logicalDate; fileName += ".zip";

    dumpCmd  = "rm -f ";
    dumpCmd += fileName;
    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    dumpCmd  = "/usr/bin/zip -j ";
    dumpCmd += fileName;
    dumpCmd += " ";
    dumpCmd += path;
    dumpCmd += "*.sql";

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    string destDir = _ini->get_value("PATH","DOCROOT");
    dumpCmd  = destDir;
    dumpCmd += locationCode; dumpCmd += ".";
    dumpCmd += logicalDate; dumpCmd += ".zip";
    _util->file_delete(dumpCmd.c_str ());

    dumpCmd  = "mv ";
    dumpCmd += fileName;
    dumpCmd += " ";
    dumpCmd += destDir;

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    return true;
}
/*******************************************************************
      FUNCTION:          compressAndPublish
   DESCRIPTION:          Compress and publish the file
     ARGUMENTS:          none
       RETURNS:          true on succes
 *******************************************************************/
bool                     CYCommandLine::compressAndPublish(std::string locationCode,
                                                           std::string logicalDate)
{
    (void)logicalDate;
    (void)locationCode;
    //////////////////////////////////////
    //  Go to the export directory
    string dumpCmd = "";
    string path = _ini->get_value("PATH","PUBLISH");
    path += "/";
    dumpCmd  = "cd ";
    dumpCmd += path;
    int ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }
    return true;
    //////////////////////////////////////
    //  Compress generated files
    string publishPath = _ini->get_value("PATH","PUBLISH");
    string publishPass = "/.mypass";

    string tmp, fileName = path;
    fileName += locationCode; fileName += ".";
    fileName += logicalDate; fileName += ".tar.gz";

    dumpCmd  = "/usr/bin/tar -czf ";
    dumpCmd += fileName;
    dumpCmd += " *.sql ";

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }
    //////////////////////////////////////
    //  Cleanup temporary files
    dumpCmd  = "rm -f ";
    dumpCmd += path;
    dumpCmd += "*.sql ";
    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }
    //////////////////////////////////////
    //  Publish to the web
    /*
    dumpCmd  = "cp -r ";
    dumpCmd += fileName;
    dumpCmd += " ";
    dumpCmd += _ini->get_value("PATH","DOCROOT");

    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }

    dumpCmd  = "rm ";
    dumpCmd += fileName;
    ret = system(dumpCmd.c_str ());
    _log->logmsg("CMDLINE",dumpCmd.c_str ());
    if (ret < 0)  {
        _error = "Unable to execute: ";
        _error += dumpCmd;
        _log->logmsg("ERROR",_error.c_str ());
        return false;
    }*/
    return true;
}
