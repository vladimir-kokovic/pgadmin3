//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgColumn.h PostgreSQL Column
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGColumn_H
#define PGColumn_H

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "pgObject.h"
#include "pgServer.h"
#include "pgDatabase.h"

class pgCollection;

class pgColumn : public pgSchemaObject
{
public:
    pgColumn(pgSchema *newSchema, const wxString& newName = wxString(""));
    ~pgColumn();

    int GetIcon() { return PGICON_COLUMN; }
    void ShowTreeDetail(wxTreeCtrl *browser, frmMain *form=0, wxListCtrl *properties=0, wxListCtrl *statistics=0, ctlSQLBox *sqlPane=0);
    static pgObject *ReadObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction=wxT(""));

    wxString GetDefinition();

    wxString GetVarTypename() const { return varTypename; }
    void iSetVarTypename(const wxString& s) { varTypename=s; }
    wxString GetQuotedTypename() const { return quotedTypename; }
    void iSetQuotedTypename(const wxString& s) { quotedTypename=s; }
    wxString GetDefault() const { return defaultVal; }
    void iSetDefault(const wxString& s) { defaultVal=s; }
    long GetColNumber() const { return colNumber; }
    void iSetColNumber(const long l) { colNumber=l; }
    long GetLength() const { return length; }
    void iSetLength(const long l) { length=l; }
    long GetPrecision() const { return precision; }
    void iSetPrecision(const long l) { precision=l; }
    long GetStatistics() const { return statistics; }
    void iSetStatistics(const long l) { statistics=l; }
    bool GetIsPK() const {return isPK; }
    bool GetIsFK() const {return isFK; }
    bool GetNotNull() const {return notNull; }
    void iSetNotNull(const bool b) {notNull=b; }
    bool GetIsArray() const {return isArray; }
    void iSetIsArray(const bool b) {isArray=b; }
    long GetTyplen() const { return typlen; }
    void iSetTyplen(const long l) { typlen=l; }
    long GetTypmod() const { return typmod; }
    void iSetTypmod(const long l) { typmod=l; }
    wxString GetQuotedFullTable() const { return quotedFullTable; }
    void iSetQuotedFullTable(const wxString &s) { quotedFullTable=s; }
    wxString GetStorage() const {return storage; }
    void iSetStorage(const wxString& s) { storage=s; }
    long GetInheritedCount() const { return inheritedCount; }
    void iSetInheritedCount(const long l) { inheritedCount=l; }

    bool CanCreate() { return true; }
    bool CanEdit() { return true; }
    bool CanDrop() { return true; }
    bool DropObject(wxFrame *frame, wxTreeCtrl *browser);
    bool GetSystemObject() const { return colNumber < 0; }
    wxString GetSql(wxTreeCtrl *browser);

    pgObject *Refresh(wxTreeCtrl *browser, const wxTreeItemId item);

private:
    wxString varTypename, quotedTypename, defaultVal, quotedFullTable, storage;
    long colNumber, length, precision, statistics;
    long typlen, typmod, inheritedCount;
    bool isPK, isFK, notNull, isArray;
};

#endif
