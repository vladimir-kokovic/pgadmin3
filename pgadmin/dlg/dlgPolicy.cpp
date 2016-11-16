//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgPolicy.cpp - PostgreSQL Policy Property
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "utils/misc.h"
#include "frm/frmMain.h"
#include "schema/pgObject.h"
#include "schema/pgPolicy.h"
#include "schema/pgUser.h"
#include "schema/pgGroup.h"
#include "dlg/dlgPolicy.h"

#define txtName          CTRL_TEXT("txtName")
#define ctrlTable        CTRL_TEXT("ctrlTable")
#define lbRoles          CTRL_LISTBOX("lbRoles")
#define btnAdd           CTRL_BUTTON("btnAdd")
#define btnRemove        CTRL_BUTTON("btnRemove")
#define cbRole           CTRL_CHOICE("cbRole")
#define cbCommand        CTRL_CHOICE("cbCommand")
#define ctrlUsingExpr    CTRL_TEXT("ctrlUsingExpr")
#define ctrlCheckExpr    CTRL_TEXT("ctrlCheckExpr")

//Possible types of policy's commands
#define CMD_ALL          wxT("ALL")
#define CMD_SELECT       wxT("SELECT")
#define CMD_INSERT       wxT("INSERT")
#define CMD_UPDATE       wxT("UPDATE")
#define CMD_DELETE       wxT("DELETE")

//public role
static wxString publicRole = wxT("public");

BEGIN_EVENT_TABLE(dlgPolicy, dlgProperty)
	EVT_BUTTON(XRCID("btnAdd"),               dlgPolicy::OnAddRole)
	EVT_BUTTON(XRCID("btnRemove"),            dlgPolicy::OnDelRole)
END_EVENT_TABLE();


dlgProperty *pgPolicyFactory::CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
{
	return new dlgPolicy(this, frame, (pgPolicy *)node, parent);
}


dlgPolicy::dlgPolicy(pgaFactory *f, frmMain *frame, pgPolicy *node, pgObject *parentNode)
	: dlgProperty(f, frame, wxT("dlgPolicy"))
{
	policy = node;
	table = (pgTable *) parentNode;
	if (table)
		connection = table->GetConnection();

	//Users list view initialization
	SetRolesToCtrl();

	//Command box initialization
	cbCommand->Append(CMD_ALL);
	cbCommand->Append(CMD_SELECT);
	cbCommand->Append(CMD_INSERT);
	cbCommand->Append(CMD_UPDATE);
	cbCommand->Append(CMD_DELETE);
}

void dlgPolicy::CheckChange()
{
	if (policy)
		EnableOK(!GetSql().IsEmpty());
	else
	{
		bool enable = true;

		CheckValid(enable, !GetName().IsEmpty(), _("Please specify name."));
		EnableOK(enable);
	}
}


pgObject *dlgPolicy::GetObject()
{
	return policy;
}


pgObject *dlgPolicy::CreateObject(pgCollection *collection)
{
	wxString name = GetName();

	if (name.IsEmpty())
		return 0;

	return policyFactory.CreateObjects(collection, 0);
}


int dlgPolicy::Go(bool modal)
{
	ctrlTable->SetValue(table->GetQuotedFullIdentifier());
	if (policy)
	{
		ctrlCheckExpr->SetValue(policy->GetCheckExpr());
		ctrlUsingExpr->SetValue(policy->GetUsingExpr());

		cbCommand->SetSelection(cbCommand->FindString(policy->GetCommand()));

		//Add users to list view
		roles = policy->GetRolesArray();
		for (unsigned int i = 0; i < roles.GetCount(); i++)
		{
			wxString role = roles[i];
			lbRoles->Append(role);
			if (role == publicRole)
			{
				cbRole->Disable();
				btnAdd->Disable();
			}
		}
		cbCommand->Disable();
	}
	else
	{
		// create mode
		if (!table)
		{
			cbClusterSet->Disable();
			cbClusterSet = 0;
		}
	}

	return dlgProperty::Go(modal);
}


void dlgPolicy::SetRolesToCtrl()
{
	//Add default roles to combobox
	cbRole->Append(wxT("public"));
	cbRole->Append(wxT("SESSION_USER"));
	cbRole->Append(wxT("CURRENT_USER"));

	wxString sql = wxT("SELECT rolname FROM pg_roles ORDER BY rolname");
	pgSet *users = connection->ExecuteSet(sql, true);

	//Add users
	if (users)
	{
		while (!users->Eof())
		{
			cbRole->Append(users->GetVal(wxT("rolname")));
			users->MoveNext();
		}
		delete users;
	}
}


wxString dlgPolicy::GetRoles() const
{
	wxString result = wxEmptyString;

	for (unsigned int i = 0; i < lbRoles->GetCount(); i++)
	{
		result += lbRoles->GetString(i);
		if (i + 1 != lbRoles->GetCount())
			result += wxT(", ");
	}

	return result;
}


bool dlgPolicy::CompareRoles() const
{
	wxArrayString policyRolesArray = policy->GetRolesArray();
	if (roles.GetCount() != policyRolesArray.GetCount())
		return false;

	for (unsigned int i = 0; i < policyRolesArray.GetCount(); i++)
	{
		if (roles.Index(policyRolesArray[i]) == wxNOT_FOUND)
			return false;
	}
	return true;
}


void dlgPolicy::OnAddRole(wxCommandEvent &ev)
{
	int pos = cbRole->GetCurrentSelection();
	wxString role = cbRole->GetString(pos);
	if (role.IsEmpty())
		return;

	//If we modify already exists policy,
	//check, that role exists in view
	if (roles.Index(role) != wxNOT_FOUND)
		return;

	lbRoles->Append(role);
	roles.push_back(role);
	if (role == publicRole)
	{
		cbRole->Disable();
		btnAdd->Disable();
	}
	CheckChange();
}


void dlgPolicy::OnDelRole(wxCommandEvent &ev)
{
	int pos = lbRoles->GetSelection();
	wxString role = lbRoles->GetString(pos);

	if (pos >= 0)
	{
		roles.Remove(role);
		lbRoles->Delete(pos);
	}
	if (role == publicRole)
	{
		cbRole->Enable();
		btnAdd->Enable();
	}
	CheckChange();
}


wxString dlgPolicy::GetSql()
{
	wxString sql = wxEmptyString;
	wxString name = GetName();

	if (!policy)
	{
		int commandPos = cbCommand->GetSelection();
		sql = wxT("CREATE POLICY ") + GetName() + wxT(" ON ") + table->GetQuotedFullIdentifier();
		if (commandPos != wxNOT_FOUND)
			sql += wxT("\n  FOR ") + cbCommand->GetString(commandPos);
		//Add roles
		if (lbRoles->GetCount())
			sql += wxT("\n  TO ") + GetRoles();
		if (!ctrlUsingExpr->GetValue().IsNull())
			sql += wxT("\n  USING (") + ctrlUsingExpr->GetValue() + wxT(")");
		if (!ctrlCheckExpr->GetValue().IsNull())
			sql += wxT("\n  WITH CHECK (") + ctrlCheckExpr->GetValue() + wxT(")");
		sql += wxT(";\n\n");
	}
	else
	{
		//edit mode
		if (policy->GetName() != name)
		{
			sql = wxT("ALTER POLICY ") + policy->GetName() + wxT(" ON ") + table->GetQuotedFullIdentifier();
			sql += wxT(" RENAME TO ") + name + wxT(";\n\n");
		}
		else if (!CompareRoles() ||
				 (ctrlUsingExpr->GetValue() != policy->GetUsingExpr()) ||
				 (ctrlCheckExpr->GetValue() != policy->GetCheckExpr()))
		{
			sql = wxT("ALTER POLICY ") + policy->GetName() + wxT(" ON ") + table->GetQuotedFullIdentifier();
			if (!CompareRoles() && !GetRoles().IsNull())
				sql += wxT("\n  TO ") + GetRoles();
			if (ctrlUsingExpr->GetValue() != policy->GetUsingExpr())
				sql += wxT("\n  USING(") + ctrlUsingExpr->GetValue() + wxT(")");
			if (ctrlCheckExpr->GetValue() != policy->GetCheckExpr())
				sql += wxT("\n  WITH CHECK(") + ctrlCheckExpr->GetValue() + wxT(")");
			sql += wxT(";\n\n");
		}
	}
	AppendComment(sql, wxT("POLICY ") + GetName() + wxT(" ON ") + table->GetQuotedFullIdentifier(), policy);

	return sql;
}
