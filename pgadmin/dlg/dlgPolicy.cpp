//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgPolicy.cpp - PostgreSQL Operator Property
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

BEGIN_EVENT_TABLE(dlgPolicy, dlgProperty)
	EVT_LISTBOX(XRCID("lbRoles"),             dlgPolicy::OnRoleSelChange)
	EVT_BUTTON(XRCID("btnAdd"),               dlgPolicy::OnAddRole)
	EVT_BUTTON(XRCID("btnRemove"),            dlgPolicy::OnDelRole)
	EVT_TEXT(XRCID("cbRole"),                 dlgPolicy::OnRoleChange)
END_EVENT_TABLE();


dlgProperty *pgPolicyFactory::CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
{
	return new dlgPolicy(this, frame, (pgPolicy *)node, parent);
}


dlgPolicy::dlgPolicy(pgaFactory *f, frmMain *frame, pgPolicy *node, pgObject *parentNode)
	: dlgProperty(f, frame, wxT("dlgPolicy"))
{
	policy = node;
	object = parentNode;
	if (object)
		connection = object->GetConnection();

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
	bool enable = true;

	if (policy)
	{
		EnableOK(!GetSql().IsEmpty());
	}
	else
	{
		CheckValid(enable, !GetName().IsEmpty(), _("Please specify name."));
	}
	EnableOK(enable);
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

	pgObject *obj = policyFactory.CreateObjects(collection, 0, wxT(
	                    "\n   AND conname=") + qtDbString(name) + wxT(
	                    "\n   AND relnamespace=") + object->GetSchema()->GetOidStr());
	return obj;
}


int dlgPolicy::Go(bool modal)
{
	if (policy)
	{
		ctrlTable->SetValue(policy->GetTableName());

		ctrlCheckExpr->SetValue(policy->GetCheckExpr());
		ctrlUsingExpr->SetValue(policy->GetUsingExpr());

		cbCommand->SetSelection(cbCommand->FindString(policy->GetCommand()));

		//Add users to list view
		for (unsigned int i = 0; i < policy->GetRolesArray().GetCount(); i++)
		{
			wxString username = policy->GetRolesArray()[i];
			lbRoles->Append(username);
		}
	}
	else
	{
		// create mode
		txtComment->Disable();
		if (!object)
		{
			cbClusterSet->Disable();
			cbClusterSet = 0;
		}
		ctrlTable->SetValue(((pgTable *) object)->GetName());
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


void dlgPolicy::OnChangeValidate(wxCommandEvent &ev)
{
	CheckChange();
}


void dlgPolicy::OnRoleSelChange(wxCommandEvent &ev)
{
	std::cerr << "OnRoleSelChange" << std::endl;
}


void dlgPolicy::OnAddRole(wxCommandEvent &ev)
{
	std::cerr << "OnAddRole" << std::endl;
}


void dlgPolicy::OnDelRole(wxCommandEvent &ev)
{
	std::cerr << "OnDelRole" << std::endl;
}


void dlgPolicy::OnRoleChange(wxCommandEvent &ev)
{
	std::cerr << "OnRoleChange" << std::endl;
}


wxString dlgPolicy::GetSql()
{
	wxString sql;
	wxString name = GetName();

	if (!policy)
	{
//		sql = wxT("ALTER ") + object->GetTypeName().Upper() + wxT(" ") + object->GetQuotedFullIdentifier()
//		      + wxT("\n  ADD");
//		if (name.Length() > 0)
//		{
//			sql += wxT(" CONSTRAINT ") + qtIdent(name) + wxT("\n ");
//		}
//		sql += wxT(" CHECK ");
//		sql += GetDefinition();
//		if (connection->BackendMinimumVersion(9, 2) && chkNoInherit->GetValue())
//		{
//			sql += wxT(" NO INHERIT");
//		}
//		sql += wxT(";\n");
	}
	else
	{
//		if (check->GetName() != name)
//		{
//			sql = wxT("ALTER ") + object->GetTypeName().Upper() + wxT(" ") + object->GetQuotedFullIdentifier()
//			      + wxT("\n  RENAME CONSTRAINT ") + qtIdent(check->GetName())
//			      + wxT(" TO ") + qtIdent(name) + wxT(";\n");
//		}
//		if (connection->BackendMinimumVersion(9, 2) && !check->GetValid() && !chkDontValidate->GetValue())
//		{
//			sql += wxT("ALTER ") + object->GetTypeName().Upper() + wxT(" ") + object->GetQuotedFullIdentifier()
//			       + wxT("\n  VALIDATE CONSTRAINT ") + qtIdent(name) + wxT(";\n");
//		}
	}

//	if (!name.IsEmpty())
//		AppendComment(sql, wxT("CONSTRAINT ") + qtIdent(name)
//		              + wxT(" ON ") + object->GetQuotedFullIdentifier(), check);
	return sql;
}
