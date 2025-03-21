/*******************************************************************
   FILE:                cytable.cpp
   DESCRIPTION:         Database table class implementation
 *******************************************************************/
//////////////////////////////////////////
//  Standard headers
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
//////////////////////////////////////////
//  Dereference the namespace
using std::endl;
using std::string;
using std::stringstream;
//////////////////////////////////////////
//  User header file(s)
#include "cytable.h"
#include "cydb/cymysql.h"
/***************************************************************************
         CLASS:         CYTable
   DESCRIPTION:         Constructor
     ARGUMENTS:         none
       RETURNS:         none
 ***************************************************************************/
CYTable::CYTable (CYDbEnv* env, CYDbEnv* schema,
                  std::string dbname, std::string tableName)
{
    _env = env;
    _dbname = dbname;
    _schema = schema;
    _tablename = tableName;
}
/*******************************************************************
   destructor
 *******************************************************************/
CYTable::~CYTable ()
{
}
/***************************************************************************
      FUNCTION:         columnInfo
   DESCRIPTION:         Retrieve the list of column(s)
     ARGUMENTS:         none
       RETURNS:         true on success
 ***************************************************************************/
bool                    CYTable::columnInfo(bool withIndex)
{
    string sql;
    int colidx = 0;

    resetIndex();
    resetColumns();

    MySqlDb db (_schema);

    sql  = "select * from COLUMNS where TABLE_SCHEMA = '";
    sql += _dbname;
    sql += "' and TABLE_NAME = '";
    sql += _tablename;
    sql += "' order by ordinal_position ";

    if (!db.sql_result(sql,false))  {
        seterrormsg (db.errordb());
        return false;
    }
    //////////////////////////////////
    //  Do column(s) exist?
    while (!db.eof ())  {
        _column[colidx].columnKey  = db.sql_field_value("COLUMN_KEY");
        _column[colidx].columnType = db.sql_field_value("DATA_TYPE");
        _column[colidx].columnName = db.sql_field_value("COLUMN_NAME");
        _column[colidx].nullable   = (db.sql_field_value ("IS_NULLABLE") == "YES");
        _column[colidx].columnLen  = db.sql_field_value("CHARACTER_MAXIMUM_LENGTH");
        _column[colidx].columnSeq  = atoi(db.sql_field_value ("ORDINAL_POSITION").c_str ());

        colidx++;
        db.sql_next();
    }
    if (withIndex)  {
        int idx = 0;
        int idxcol = 0;
        MySqlDb dbidx (_schema);
        sql  = "SELECT DISTINCT(INDEX_NAME) as INDEX_NAME FROM INFORMATION_SCHEMA.STATISTICS WHERE TABLE_SCHEMA = '";
        sql += _dbname;
        sql += "' and TABLE_NAME = '";
        sql += _tablename;
        sql += "' ";

        if (!dbidx.sql_result(sql,false))  {
            seterrormsg (dbidx.errordb());
            return false;
        }
        //////////////////////////////////
        //  Do column(s) exist?
        while (!dbidx.eof ())  {
            string index_name = dbidx.sql_field_value("INDEX_NAME");
            _index[idx].indexName = index_name;


            db.sql_reset ();
            sql  = "select table_name, index_name, seq_in_index, ";
            sql += "column_name, non_unique, index_type, comment ";
            sql += "from information_schema.statistics where 1=1 ";
            sql += "and table_schema = '"; sql += _dbname; sql += "' ";
            sql += "and table_name = '"; sql += _tablename; sql += "' ";
            sql += "and index_name = '"; sql += index_name; sql += "' ";
            sql += "order by 1,2,3,4,5,6 ";
            /***
            sql  = "SELECT * FROM INFORMATION_SCHEMA.STATISTICS WHERE TABLE_SCHEMA = '";
            sql += _dbname;
            sql += "' and TABLE_NAME = '";
            sql += _tablename;
            sql += "' and INDEX_NAME = '";
            sql += index_name;
            sql += "' order by SEQ_IN_INDEX ";
            ***/
            if (!db.sql_result(sql,false))  {
                seterrormsg (db.errordb());
                return false;
            }
            //////////////////////////////////
            //  Do column(s) exist?
            idxcol = 0;
            while (!db.eof ())  {
                _index[idx].indexColumn[idxcol]=db.sql_field_value ("COLUMN_NAME");

                idxcol++;
                db.sql_next();
            }

            idx++;
            dbidx.sql_next ();
        }
    }

    return true;
}
/***************************************************************************
         CLASS:         Mysql2Lite
   DESCRIPTION:         Publish the restore file(s)
     ARGUMENTS:         none
       RETURNS:         true on success
 ***************************************************************************/
string                  CYTable::createTableSql ()
{
    //string sql;
    //string hdr;
    stringstream ss;
    //sql  = "select ";
    ss << endl << endl << "CREATE TABLE " << _tablename << "(" << endl;

    int idx = 0;
    bool done = false;
    while (!done)  {
        if (_column[idx].columnName.length() < 1)
            done = true;
        else  {
            //////////////////////////////
            //  Spacing
            string strSpace = "";
            int len = _column[idx].columnName.length();
            int space = 48 - len;
            if (space <= 0) space = 4;
            strSpace = "";
            for (int i = 0; i < space; i++)
                strSpace += " ";
           // hdr += _column[idx].columnName;
           // sql += _column[idx].columnName;
            ss << "    " << _column[idx].columnName << strSpace;
            //////////////////////////////
            //  For each column
            _column[idx].numeric = true;
            if (_column[idx].columnType == "char" ||
                _column[idx].columnType == "text" ||
                _column[idx].columnType == "varchar")  {
                _tmp = _column[idx].columnLen;
                _column[idx].numeric = false;
                if (_tmp.length() < 1)
                    _tmp = "512";
                ss << "varchar(" << _tmp << ")";
            }  else if (_column[idx].columnType == "float" ||
                        _column[idx].columnType == "double" ||
                        _column[idx].columnType == "decimal")  {
                ss << "numeric(16,2)";
            }  else if (_column[idx].columnType == "int" ||
                        _column[idx].columnType == "date" ||
                        _column[idx].columnType == "time" ||
                        _column[idx].columnType == "bigint" ||
                        _column[idx].columnType == "tinyint")  {
                ss << "integer";
            }  else if (_column[idx].columnType == "date")  {
                ss << "integer";
            }  else  {
                _tmp = _column[idx].columnLen;
                _column[idx].numeric = false;
                if (_tmp.length() < 1)
                    _tmp = "4096";
                ss << "text(" << _tmp << ")";
            }
            //////////////////////////////
            //  Comma or parentheses
            if ((idx+1) < g_maxcolumns)  {
                if (_column[(idx+1)].columnName.length() < 1)  {
                    ss << ");" << endl;
                    //hdr += "\n";
                   // sql += " from ";
                   // sql += _tablename;
                    //////////////////////////////////////
                    //  Do NOT IMPORT legacy sales table
                    //  information
                    if (0 == strncmp (_tablename.c_str (), "cy_daily", strlen("cy_daily")))  {
                       // sql += " limit 1 ";
                    }  else  {
                        //sql += " limit 10000 ";
                    }
                    //////////////////////
                    //  Data file
                    //createTableFile(sql, hdr, _tablename);
                }  else  {
                    //hdr += "\t";
                    //sql += ",";
                    ss << "," << endl;
                }
            }  else  {
                done = true;
            }
            idx++;
        }
    }
    //////////////////////////////////////
    //
    _tmp = ss.str ();
    sqlCreate = _tmp;
    return _tmp;
}
/***************************************************************************
         CLASS:         Mysql2Lite
   DESCRIPTION:         Publish the index file(s)
     ARGUMENTS:         none
       RETURNS:         true on success
 ***************************************************************************/
bool                    CYTable::createTableFile (std::string fileName,
                                                  std::string tableName,
                                                  std::string columnList)
{
    //////////////////////////////////////
    //  Create the output file
    string tmp, add, ins;
    FILE* f = fopen (fileName.c_str (),"w");
    if (NULL == f)  {
        tmp  = "Unable to create the file: ";
        tmp += fileName;
        return seterrormsg(tmp);
    }
    //////////////////////////////////////
    //  Insert statement
    add  = "insert into ";
    add += tableName;
    add += "(";
    add += columnList;

    MySqlDb db (_env);
    db.sql_reset ();
    string sql  = "select "; sql += columnList; sql += " from "; sql += tableName;
    //  Filter promotion(s)
    cyutility util;
    if (tableName == "pos_price")  {
        sql  = "select "; sql += columnList; sql += " from "; sql += tableName;
        sql += " where end_date >= ";
        sql += util.date_eight();
    }  else if (tableName == "pos_price_item")  {
        sql  = "select a.* from pos_price_item a, pos_price b ";
        sql += "where  a.event_cd = b.event_cd and b.end_date >= ";
        sql += util.date_eight();
    }

    if (!db.sql_result(sql,false))
        return seterrormsg (db.errordb());

    string val, name;
    while (!db.eof ())  {
        for (int i = 0; i < db.numcols(); i++)  {
            name = db.sql_field_name(i);
            val = db.sql_field_value(name);
            if (0 != i)  {
                ins += ",";
            }
            ins += "\"";
            ins += val;
            ins += "\"";
        }
        ins += "\n";
        fwrite (ins.c_str (),ins.size(),1,f);
        ins = "";
        db.sql_next ();
    }
    fclose(f);
    return true;
}
/***************************************************************************
         CLASS:         Mysql2Lite
   DESCRIPTION:         Publish the index file(s)
     ARGUMENTS:         none
       RETURNS:         true on success
 ***************************************************************************/
string                  CYTable::createTableIndex ()
{
    int idx = 0;
    int colidx = 0;
    bool done = false;
    bool coldone = false;
    string ss = "";

    while (!done)  {
        if (_index[idx].indexName.length() < 1)
            done = true;
        else  {
            colidx = 0;
            coldone = false;

            if (_index[idx].indexName != "PRIMARY")  {
                ss += "\n";
                ss += "CREATE INDEX ";
                ss += _index[idx].indexName;
                ss += " on ";
                ss += _tablename;
                ss += "(";

                while (!coldone)  {
                    if (_index[idx].indexColumn[colidx].length () < 1)
                        done = true;
                    else  {
                        ss += _index[idx].indexColumn[colidx];
                    }
                    //////////////////////////////
                    //  Comma or parentheses
                    if ((colidx+1) < g_maxcolumns)  {
                        if (_index[idx].indexColumn[(colidx+1)].length () < 1)  {
                            ss += ");";
                            coldone = true;
                        }  else
                            ss += ",";
                    }  else  {
                        coldone = true;
                    }
                    colidx++;
                }
            }
        }
        idx++;
    }
    _tmp = ss;
    indexCreate = _tmp;
    return _tmp;
}
