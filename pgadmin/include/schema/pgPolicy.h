//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgSchema.h PostgreSQL Policy
//
//////////////////////////////////////////////////////////////////////////

#ifndef PGPOLICY_H
#define PGPOLICY_H

// App headers
#include "pgTable.h"


class pgPolicyFactory : public pgSchemaObjFactory
{
public:
	pgPolicyFactory();
	virtual pgCollection *CreateCollection(pgObject *obj);
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent);
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
};
extern pgPolicyFactory policyFactory;


class pgPolicy : public pgSchemaObject
{
public:
	pgPolicy(pgSchema *newSchema, const wxString &newName = wxT(""));
	~pgPolicy();

	wxString GetTranslatedMessage(int kindOfMessage) const;
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0);
	pgObject *Refresh(ctlTree *browser, const wxTreeItemId item);
	bool DropObject(wxFrame *frame, ctlTree *browser, bool cascaded);
	wxString GetSql(ctlTree *browser);
	void ParseRoles(const wxString &s);
	wxString GetRoles() const;
	wxArrayString &GetRolesArray()
	{
		return roles;
	}
	wxString GetCommand() const
	{
		return command;
	}
	void iSetCommand(const wxString &s)
	{
		command = s;
	}
	wxString GetUsingExpr() const
	{
		return usingExpr;
	}
	void iSetUsingExpr(const wxString &s)
	{
		usingExpr = s;
	}
	wxString GetCheckExpr() const
	{
		return checkExpr;
	}
	void iSetCheckExpr(const wxString &s)
	{
		checkExpr = s;
	}
	wxString GetTableName() const
	{
		return tablename;
	}
	void iSetTableName(const wxString &s)
	{
		tablename = s;
	}
	wxString GetSchemaName() const
	{
		return schemaname;
	}
	void iSetSchemaName(const wxString &s)
	{
		schemaname = s;
	}
private:
	wxString command, usingExpr, checkExpr, tablename, schemaname;
	wxArrayString roles;
};

class pgPolicyCollection : public pgSchemaObjCollection
{
public:
	pgPolicyCollection(pgaFactory *factory, pgSchema *sch);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};
extern pgaCollectionFactory policyCollectionFactory;


#endif // PGPOLICY_H
