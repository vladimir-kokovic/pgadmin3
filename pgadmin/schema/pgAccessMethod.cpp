//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pgAccessMethod.cpp - Access Method class
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "utils/misc.h"
#include "schema/pgAccessMethod.h"


pgAccessMethod::pgAccessMethod(const wxString &newName)
	: pgDatabaseObject(accessMethodFactory, newName)
{
}

wxString pgAccessMethod::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;

	switch (kindOfMessage)
	{
		case RETRIEVINGDETAILS:
			message = _("Retrieving details on access method");
			message += wxT(" ") + GetName();
			break;
		case REFRESHINGDETAILS:
			message = _("Refreshing access method");
			message += wxT(" ") + GetName();
			break;
		case DROPINCLUDINGDEPS:
			message = wxString::Format(_("Are you sure you wish to drop access method \"%s\" including all objects that depend on it?"),
			                           GetFullIdentifier().c_str());
			break;
		case DROPEXCLUDINGDEPS:
			message = wxString::Format(_("Are you sure you wish to drop access method \"%s\"?"),
			                           GetFullIdentifier().c_str());
			break;
		case DROPCASCADETITLE:
			message = _("Drop access method cascaded?");
			break;
		case DROPTITLE:
			message = _("Drop access method?");
			break;
		case PROPERTIESREPORT:
			message = _("Access method properties report");
			message += wxT(" - ") + GetName();
			break;
		case PROPERTIES:
			message = _("Access method properties");
			break;
		case DDLREPORT:
			message = _("Access method DDL report");
			message += wxT(" - ") + GetName();
			break;
		case DDL:
			message = _("Access method DDL");
			break;
		case DEPENDENCIESREPORT:
			message = _("Access method dependencies report");
			message += wxT(" - ") + GetName();
			break;
		case DEPENDENCIES:
			message = _("Access method dependencies");
			break;
		case DEPENDENTSREPORT:
			message = _("Access method dependents report");
			message += wxT(" - ") + GetName();
			break;
		case DEPENDENTS:
			message = _("Access method dependents");
			break;
	}

	return message;
}

bool pgAccessMethod::DropObject(wxFrame *frame, ctlTree *browser, bool cascaded)
{
	wxString sql = wxT("DROP ACCESS METHOD ") + GetQuotedFullIdentifier();
	if (cascaded)
		sql += wxT(" CASCADE");
	return GetDatabase()->ExecuteVoid(sql);
}

wxString pgAccessMethod::GetSql(ctlTree *browser)
{
	if (sql.IsNull())
	{
		sql = wxT("-- Access Method: ") + GetQuotedFullIdentifier() + wxT("\n\n")
		      + wxT("-- DROP ACESS METHOD ") + GetQuotedFullIdentifier() + wxT(";")
		      + wxT("\n\nCREATE ACCESS METHOD ") + GetName();

		sql += wxT("\n  TYPE ") + GetType();
		sql += wxT("\n  HANDLER ") + GetHandler() + wxT(";\n\n");

		sql += GetCommentSql();
	}
	return sql;
}

void pgAccessMethod::ShowTreeDetail(ctlTree *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
	if (properties)
	{
		CreateListColumns(properties);

		properties->AppendItem(_("Name"), GetName());
		properties->AppendItem(_("OID"), GetOid());
		properties->AppendItem(_("Handler"), GetHandler());
		properties->AppendItem(_("Type"), GetType());
		properties->AppendItem(_("Comment"), GetComment());
	}
}

pgObject *pgAccessMethod::Refresh(ctlTree *browser, const wxTreeItemId item)
{
	pgObject *accessMethod = 0;
	pgCollection *coll = browser->GetParentCollection(item);
	if (coll)
		accessMethod = accessMethodFactory.CreateObjects(coll, 0, wxT("\n   AND lan.oid=") + GetOidStr());

	return accessMethod;
}

pgObject *pgAccessMethodFactory::CreateObjects(pgCollection *collection, ctlTree *browser, const wxString &restriction)
{
	pgAccessMethod *accessMethod = 0;

	wxString sql = wxT("SELECT am.oid AS oid, am.amname AS amname, am.amhandler AS amhandler, ")
	               wxT("CASE am.amtype WHEN 'i' THEN 'INDEX' ELSE NULL END AS amtype, d.description AS comment ")
	               wxT("  FROM pg_am am\n")
	               wxT("  LEFT JOIN pg_description d ON am.oid = d.objoid\n AND d.classoid = 'pg_am'::regclass ")
	               wxT("ORDER BY am.amname");

	pgSet *accessMethods = collection->GetDatabase()->ExecuteSet(sql);

	if (accessMethods)
	{
		while (!accessMethods->Eof())
		{
			accessMethod = new pgAccessMethod(accessMethods->GetVal(wxT("amname")));
			accessMethod->iSetDatabase(collection->GetDatabase());
			accessMethod->iSetOid(accessMethods->GetOid(wxT("oid")));
			accessMethod->iSetHandler(accessMethods->GetVal(wxT("amhandler")));
			accessMethod->iSetType(accessMethods->GetVal(wxT("amtype")));
			accessMethod->iSetComment(accessMethods->GetVal(wxT("comment")));
			if (browser)
			{
				browser->AppendObject(collection, accessMethod);
				accessMethods->MoveNext();
			}
			else
				break;
		}
		delete accessMethods;
	}
	return accessMethod;
}

/////////////////////////////

pgAccessMethodCollection::pgAccessMethodCollection(pgaFactory *factory, pgDatabase *db)
	: pgDatabaseObjCollection(factory, db)
{
}

wxString pgAccessMethodCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;

	switch (kindOfMessage)
	{
		case RETRIEVINGDETAILS:
			message = _("Retrieving details on access methods");
			break;
		case REFRESHINGDETAILS:
			message = _("Refreshing access methods");
			break;
		case OBJECTSLISTREPORT:
			message = _("Access methods list report");
			break;
	}

	return message;
}

///////////////////////////////////////////////////

#include "images/access-method.pngc"
#include "images/access-methods.pngc"

pgAccessMethodFactory::pgAccessMethodFactory()
	: pgDatabaseObjFactory(__("Access Method"), __("New Access Method..."), __("Create a new Access Method."), access_method_png_img, access_methods_png_img)
{
	metaType = PGM_ACCESSMETHOD;
}

dlgProperty *pgAccessMethodFactory::CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
{
	return 0; // not implemented
}

pgCollection *pgAccessMethodFactory::CreateCollection(pgObject *obj)
{
	return new pgAccessMethodCollection(GetCollectionFactory(), (pgDatabase *)obj);
}

pgAccessMethodFactory accessMethodFactory;
static pgaCollectionFactory cf(&accessMethodFactory, __("Access Methods"), access_methods_png_img);
