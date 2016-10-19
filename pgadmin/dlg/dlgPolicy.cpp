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
#include "dlg/dlgPolicy.h"

#define txtWhere        CTRL_TEXT("txtWhere")
#define chkNoInherit    CTRL_CHECKBOX("chkNoInherit")
#define chkDontValidate CTRL_CHECKBOX("chkDontValidate")


BEGIN_EVENT_TABLE(dlgPolicy, dlgProperty)
	EVT_TEXT(XRCID("txtWhere"),                 dlgProperty::OnChange)
	EVT_CHECKBOX(XRCID("chkDontValidate"),      dlgPolicy::OnChangeValidate)
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
}

void dlgPolicy::CheckChange()
{
	bool enable = true;
	if (policy)
	{
//		enable = txtName->GetValue() != policy->GetName() || txtComment->GetValue() != policy->GetComment();
//		if (connection->BackendMinimumVersion(9, 2) && !policy->GetValid() && !chkDontValidate->GetValue())
//			enable = true;
		EnableOK(enable);
	}
	else
	{
		// We don't allow changing the comment if the dialog is launched from dlgTable or dlgDomain
		// so we check IsModal()
		txtComment->Enable(!GetName().IsEmpty() && !IsModal() && object->GetTypeName().Upper() == wxT("TABLE"));
		CheckValid(enable, !txtWhere->GetValue().IsEmpty(), _("Please specify condition."));
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

	pgObject *obj = policyFactory.CreateObjects(collection, 0, wxT(
	                    "\n   AND conname=") + qtDbString(name) + wxT(
	                    "\n   AND relnamespace=") + object->GetSchema()->GetOidStr());
	return obj;
}


int dlgPolicy::Go(bool modal)
{
	if (policy)
	{
		// edit mode
		txtName->Enable(connection->BackendMinimumVersion(9, 2));
		txtComment->Enable(object->GetTypeName().Upper() == wxT("TABLE"));

//		txtWhere->SetValue(policy->GetDefinition());
		txtWhere->Disable();

//		if (connection->BackendMinimumVersion(9, 2))
//		{
//			chkNoInherit->SetValue(check->GetNoInherit());
//			chkDontValidate->SetValue(!check->GetValid());
//		}
//		else
//			chkDontValidate->SetValue(true);
//		chkDontValidate->Enable(connection->BackendMinimumVersion(9, 2) && !check->GetDefinition().IsEmpty() && !check->GetValid());
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
		chkDontValidate->Enable(connection->BackendMinimumVersion(9, 2));
	}

//	chkNoInherit->Enable(connection->BackendMinimumVersion(9, 2) && !check);

	return dlgProperty::Go(modal);
}


void dlgPolicy::OnChangeValidate(wxCommandEvent &ev)
{
	CheckChange();
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


wxString dlgPolicy::GetDefinition()
{
	wxString sql;

	sql = wxT("(") + txtWhere->GetValue() + wxT(")");

	if (chkDontValidate->GetValue())
		sql += wxT(" NOT VALID");

	return sql;
}
