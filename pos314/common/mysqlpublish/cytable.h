/********************************************************************
          FILE:          cytable.h
   DESCRIPTION:          Database table class header
 ********************************************************************/
#ifndef __CYTABLE_H
#define __CYTABLE_H
#include "cypublish.h"
/***************************************************************************
         CLASS:          CYTable
   DESCRIPTION:          Database table class
 ***************************************************************************/
class                    CYTable
{
public:
    //////////////////////////////////////
    //  Entry/exit point
    CYTable (CYDbEnv* env,
             CYDbEnv* schema,
             std::string schemaName,
             std::string tableName);
    virtual ~CYTable ();
    //////////////////////////////////////
    //  Method(s)
    virtual std::string  createTableSql ();
    virtual std::string  createTableIndex ();

    virtual bool         columnInfo (bool withIndex=true);
    virtual bool         createTableFile (std::string fileName,
                                          std::string tableName,
                                          std::string columnList);
    /////////////////////////////////////
    //  Generated SQL
    std::string sqlCreate;
    std::string indexCreate;
//protected:
    //////////////////////////////////////
    //  Attribute(s)
    CYDbEnv              *_env, *_schema;
    tableIndex           _index [g_maxindex];
    tableColumn          _column [g_maxcolumns];
    std::string          _tmp, _error, _dbname, _tablename;
    //////////////////////////////////////
    //  Internal method(s)
    void                 resetColumns ()  {
        for (int i = 0; i < g_maxcolumns; i++)  {
            _column[i].columnName = "";
            _column[i].columnType = "";
            _column[i].nullable = false;

        }
    }
    void                 resetIndex ()  {
        for (int x = 0; x < g_maxindex; x++)  {
            _index[x].indexName = "";
            for (int i = 0; i < g_maxcolumns; i++)  {
                _index[x].indexColumn[i] = "";
            }
        }
    }
public:
    //////////////////////////////////////
    //  Acess methods
    virtual std::string  errormsg ()  { return _error; }
    virtual std::string  tableName ()  { return _tablename; }
    virtual bool seterrormsg (std::string msg)  { _error = msg; return false; }
    virtual void setTableName (std::string val)  { _tablename = val; }

    virtual tableIndex* getIndex (int idx) { return &_index[idx]; }
    virtual tableColumn* getColumn (int idx) { return &_column[idx]; }
};
#endif
