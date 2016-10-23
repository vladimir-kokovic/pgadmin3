//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgPolicy.h - Policy property
//
//////////////////////////////////////////////////////////////////////////

#ifndef __DLG_POLICYPROP
#define __DLG_POLICYPROP

#include "dlg/dlgProperty.h"

class pgPolicy;
class pgObject;

class dlgPolicy : public dlgProperty
{
public:
	dlgPolicy(pgaFactory *factory, frmMain *frame, pgPolicy *node = 0, pgObject *parentNode = 0);

	void CheckChange();
	wxString GetSql();
	pgObject *CreateObject(pgCollection *collection);
	pgObject *GetObject();
	wxString GetHelpPage() const
	{
		return wxT("pg/sql-altertable");
	}

	int Go(bool modal);

private:
	pgPolicy *policy;
	pgTable *table;
	wxArrayString roles;

	wxString GetQuotedFullIdentifier() const;
	void SetRolesToCtrl();
	wxString GetRoles() const;
	bool CompareRoles() const;

	void OnAddRole(wxCommandEvent &ev);
	void OnDelRole(wxCommandEvent &ev);

	DECLARE_EVENT_TABLE()
};

#endif
