//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgCheck.cpp - Check class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>
#include "wx/arrstr.h"

// App headers
#include "pgAdmin3.h"
#include "frm/frmMain.h"
#include "utils/misc.h"
#include "schema/pgPolicy.h"

#include <iostream>

pgPolicy::pgPolicy(pgSchema *newSchema, const wxString &newName)
	: pgSchemaObject(newSchema, policyFactory, newName)
{
}

pgPolicy::~pgPolicy()
{
}


wxString pgPolicy::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;

	switch (kindOfMessage)
	{
		case RETRIEVINGDETAILS:
			message = _("Retrieving details on policy");
			message += wxT(" ") + GetName();
			break;
		case REFRESHINGDETAILS:
			message = _("Refreshing policy");
			message += wxT(" ") + GetName();
			break;
		case DROPTITLE:
			message = _("Drop policy?");
			break;
		case PROPERTIES:
			message = _("Policy properties");
			break;
		case DDLREPORT:
			message = _("Check constraint DDL report");
			message += wxT(" - ") + GetName();
			break;
		case DDL:
			message = _("Check constraint DDL");
			break;
	}

	return message;
}


bool pgPolicy::DropObject(wxFrame *frame, ctlTree *browser, bool cascaded)
{
	wxString sql = wxT("DROP POLICY ") + GetQuotedFullIdentifier() + wxT(" ON ") + GetQuotedSchemaPrefix(GetSchemaName()) + qtIdent(GetTableName());
	return GetDatabase()->ExecuteVoid(sql);
}


wxString pgPolicy::GetSql(ctlTree *browser)
{
	if (sql.IsNull())
	{
		sql = wxT("-- Policy: ") + GetQuotedIdentifier() + wxT("\n\n")
		      + wxT("-- DROP POLICY ") + GetQuotedIdentifier() + wxT(" ON ") + GetQuotedSchemaPrefix(GetSchemaName()) + qtIdent(GetTableName()) + wxT(";\n\n");

		sql += wxT("CREATE POLICY ") + GetIdentifier() + wxT(" ON ") + GetQuotedSchemaPrefix(GetSchemaName()) + qtIdent(GetTableName());
		sql += wxT("\n  FOR ") + GetCommand() + wxT("\n  TO ") + GetRoles();

		if (!GetQual().IsNull())
			sql += wxT("\n  USING (") + GetQual() + wxT(")");
		if (!GetWithCheck().IsNull())
			sql += wxT("\n  WITH CHECK (") + GetWithCheck() + wxT(")");
		sql += ";\n\n";
		if (!GetComment().IsNull())
		{
			sql += wxT("COMMENT ON POLICY ") + GetQuotedIdentifier()
			       + wxT(" ON ") + GetQuotedSchemaPrefix(GetSchemaName()) + qtIdent(GetTableName())
			       + wxT(" IS ") + qtDbString(GetComment()) + wxT(";\n");
		}
	}

	return sql;
}


void pgPolicy::ParseRoles(const wxString &s)
{
	roles = s.SubString(1, s.Length() - 2);
}


void pgPolicy::ShowTreeDetail(ctlTree *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
	if (properties)
	{
		CreateListColumns(properties);

		properties->AppendItem(_("Name"), GetName());
		properties->AppendItem(_("OID"), GetOid());
		properties->AppendItem(_("Roles"), GetRoles());
		properties->AppendItem(_("Defined for"), GetCommand());
		properties->AppendItem(_("Using expression"), GetQual());
		properties->AppendItem(_("Check expression"), GetWithCheck());
		properties->AppendItem(_("Comment"), GetComment());
	}
}


pgObject *pgPolicy::Refresh(ctlTree *browser, const wxTreeItemId item)
{
	pgObject *policy = 0;

	pgCollection *coll = browser->GetParentCollection(item);
	if (coll)
		policy = policyFactory.CreateObjects(coll, 0, wxT("\n   AND p.oid=") + GetOidStr());

	return policy;
}


pgObject *pgPolicyFactory::CreateObjects(pgCollection *coll, ctlTree *browser, const wxString &restriction)
{
	pgSchemaObjCollection *collection = (pgSchemaObjCollection *)coll;
	pgPolicy *policy = 0;

	wxString sql = wxT("SELECT p.oid AS oid, p.polname AS polname, ")
	               wxT(" pp.schemaname AS schemaname, pp.tablename AS tablename, pp.roles AS roles, pp.cmd AS cmd,")
	               wxT(" pp.qual AS qual, pp.with_check AS check, d.description AS description ")
	               wxT("  FROM pg_policy p\n")
	               wxT("  JOIN pg_policies pp ON p.polname = policyname\n")
	               wxT("  LEFT JOIN pg_description d ON p.oid = d.objoid\n")
	               wxT(" WHERE p.polrelid = ") + collection->GetOidStr() +
	               wxT(" ORDER BY p.polname");

	pgSet *policies = collection->GetDatabase()->ExecuteSet(sql);

	if (policies)
	{
		while (!policies->Eof())
		{
			policy = new pgPolicy(collection->GetSchema()->GetSchema(), policies->GetVal(wxT("polname")));

			policy->iSetOid(policies->GetOid(wxT("oid")));
			policy->iSetName(policies->GetVal(wxT("polname")));
			policy->iSetSchemaName(policies->GetVal(wxT("schemaname")));
			policy->iSetTableName(policies->GetVal(wxT("tablename")));
			policy->ParseRoles(policies->GetVal(wxT("roles")));
			policy->iSetCommand(policies->GetVal(wxT("cmd")));
			policy->iSetQual(policies->GetVal(wxT("qual")));
			policy->iSetWithCheck(policies->GetVal(wxT("check")));
			policy->iSetComment(policies->GetVal(wxT("description")));

			if (browser)
			{
				browser->AppendObject(collection, policy);
				policies->MoveNext();
			}
			else
				break;
		}

		delete policies;
	}
	return policy;
}

/////////////////////////////

pgPolicyCollection::pgPolicyCollection(pgaFactory *factory, pgSchema *sch)
	: pgSchemaObjCollection(factory, sch)
{

}

wxString pgPolicyCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;

	switch (kindOfMessage)
	{
		case RETRIEVINGDETAILS:
			message = _("Retrieving details on policies");
			break;
		case REFRESHINGDETAILS:
			message = _("Refreshing policies");
			break;
		case OBJECTSLISTREPORT:
			message = _("Check policies list report");
			break;
	}

	return message;
}

/////////////////////////////

#include "images/constraints.pngc"
#include "images/check.pngc"

pgPolicyFactory::pgPolicyFactory()
	: pgSchemaObjFactory(__("Policy"), __("New Policy..."), __("Create a new Policy."), check_png_img)
{
	metaType = PGM_POLICY;
}

pgCollection *pgPolicyFactory::CreateCollection(pgObject *obj)
{
	return new pgPolicyCollection(GetCollectionFactory(), (pgSchema *)obj);
}


pgPolicyFactory policyFactory;
pgaCollectionFactory policyCollectionFactory(&policyFactory, __("Policies"), constraints_png_img);
