//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgCheck.h PostgreSQL Check
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGCheck_H
#define PGCheck_H

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "pgObject.h"
#include "pgServer.h"
#include "pgDatabase.h"

class pgCollection;

class pgCheck : public pgSchemaObject
{
public:
    pgCheck(pgSchema *newSchema, const wxString& newName = wxString(""));
    ~pgCheck();
    void SetSchema(pgSchema *newSchema) { schema = newSchema; }
    pgSchema *GetSchema() const {return schema; }
    int GetIcon() { return PGICON_CHECK; }

    void ShowTreeDetail(wxTreeCtrl *browser, frmMain *form=0, wxListCtrl *properties=0, wxListCtrl *statistics=0, ctlSQLBox *sqlPane=0);
    static pgObject *ReadObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction=wxT(""));

    wxString GetFkTable() const { return fkTable; }
    void iSetFkTable(const wxString& s) { fkTable=s; }
    wxString GetFkSchema() const { return fkSchema; }
    void iSetFkSchema(const wxString& s) { fkSchema=s; }
    wxString GetDefinition() const { return definition; }
    void iSetDefinition(const wxString& s) { definition=s; }
    bool GetDeferrable() const { return deferrable; }
    void iSetDeferrable(const bool b) { deferrable=b; }
    bool GetDeferred() const { return deferred; }
    void iSetDeferred(const bool b) { deferred=b; }

    bool CanDrop() { return true; }
    bool CanCreate() { return true; }
    bool CanEdit() { return true; }
    bool DropObject(wxFrame *frame, wxTreeCtrl *browser);
    wxString GetConstraint();
    wxString GetSql(wxTreeCtrl *browser);
    pgObject *Refresh(wxTreeCtrl *browser, const wxTreeItemId item);

private:
    wxString definition, fkTable, fkSchema;
    bool deferrable, deferred;
};

#endif
