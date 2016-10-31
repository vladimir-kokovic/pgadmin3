//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgAccessMethod.h - Access Method class
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGACCESSMETHOD_H
#define PGACCESSMETHOD_H

#include "pgDatabase.h"

class pgCollection;
class pgAccessMethodFactory : public pgDatabaseObjFactory
{
public:
	pgAccessMethodFactory();
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent);
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
	virtual pgCollection *CreateCollection(pgObject *obj);
};
extern pgAccessMethodFactory accessMethodFactory;

class pgAccessMethod : public pgDatabaseObject
{
public:
	pgAccessMethod(const wxString &newName = wxT(""));

	wxString GetTranslatedMessage(int kindOfMessage) const;
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0);
	bool DropObject(wxFrame *frame, ctlTree *browser, bool cascaded);
	wxString GetSql(ctlTree *browser);
	pgObject *Refresh(ctlTree *browser, const wxTreeItemId item);

	bool CanDropCascaded()
	{
		return true;
	}
	bool HasStats()
	{
		return false;
	}
	bool HasDepends()
	{
		return true;
	}
	bool HasReferences()
	{
		return true;
	}
	bool CanCreate()
	{
		return false;
	}
	bool CanEdit()
	{
		return false;
	}
	wxString GetHandler() const
	{
		return handler;
	}
	void iSetHandler(const wxString &s)
	{
		handler = s;
	}
	void iSetType(const wxString &s)
	{
		type = s;
	}
	wxString GetType() const
	{
		return type;
	}
private:
	wxString handler, type;
};

class pgAccessMethodCollection : public pgDatabaseObjCollection
{
public:
	pgAccessMethodCollection(pgaFactory *factory, pgDatabase *db);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};

#endif
