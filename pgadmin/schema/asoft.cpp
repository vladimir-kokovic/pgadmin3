//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2014, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// asoft.cpp - Asoft DB groups - Pionir doo Beograd ERP
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

#include "pgAdmin3.h"
#include "schema/asoft.h"

#include "images/slcluster.pngc"
#include "images/slclusters.pngc"

asoftGroupBaseFactory::asoftGroupBaseFactory(const wxChar *tn, const wxChar *ns, const wxChar *nls, wxImage *img, wxImage *imgSm)
	: pgDatabaseObjFactory(tn, ns, nls, img, imgSm)
{
}

asoftGroupFactory::asoftGroupFactory()
	: asoftGroupBaseFactory(__("Group"), __("New Group..."), __("Create a new Group."), slcluster_png_img, slcluster_png_img)
{
}

pgCollection *asoftGroupFactory::CreateCollection(pgObject *obj)
{
	return new asoftGroupCollection(GetCollectionFactory(), (pgDatabase *)obj);
}

pgCollection *asoftGroupObjFactory::CreateCollection(pgObject *obj)
{
	return new asoftGroupObjCollection(GetCollectionFactory(), (asoftGroup *)obj);
}


asoftGroupFactory asoftgroupfactory;
static pgaCollectionFactory gcf(&asoftgroupfactory, __("Groups"), slclusters_png_img);


asoftGroupCollection::asoftGroupCollection(pgaFactory *factory, pgDatabase *db)
	: pgDatabaseObjCollection(factory, db)
{
}

wxString asoftGroupCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

asoftGroupObjCollection::asoftGroupObjCollection(pgaFactory *factory, asoftGroup *ag)
	: pgCollection(factory)
{
	group = ag;
	server = ag->GetDatabase()->GetServer();
	database = ag->GetDatabase();
	schema = 0;
}

wxString asoftGroupBase::GetSql(ctlTree *browser)
{
	if (sql.IsNull())
	{
		sql = wxT("-- Asoft group: ") + opis + wxT("\n");
	}
	return sql;
}

wxMenu *asoftGroupBase::GetNewMenu()
{
	wxMenu *menu = pgObject::GetNewMenu();
	return menu;
}

asoftGroupBase::asoftGroupBase(pgaFactory &factory, const wxString &newName)
	: pgDatabaseObject(factory, newName)
{
	opis = newName;
}

void asoftGroupBase::ShowTreeDetail(ctlTree *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
	GetDatabase()->GetServer()->iSetLastDatabase(GetDatabase()->GetName());
	GetDatabase()->GetServer()->iSetLastSchema(GetName());

	if (!expandedKids)
	{
		expandedKids = true;
		browser->RemoveDummyChild(this);
		wxLogInfo(wxT("Adding child object to Group %s"), opis.c_str());
		browser->AppendCollection(this, asoftschemafactory);
	}
}

asoftGroup::asoftGroup(const wxString &newName)
	: asoftGroupBase(asoftgroupfactory, newName)
{
}

wxString asoftGroup::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

pgObject *asoftGroup::Refresh(ctlTree *browser, const wxTreeItemId item)
{
	asoftGroup *group = 0;
	pgCollection *coll = browser->GetParentCollection(item);
	if (coll)
	{
		group = (asoftGroup *)asoftgroupfactory.CreateObjects(coll, 0, wxT(" WHERE opis='") + GetOpis() + wxT("'\n"));
	}

	return group;
}



pgObject *asoftGroupBaseFactory::CreateObjects(pgCollection *coll, ctlTree *browser, const wxString &restriction)
{
	asoftGroup *group = 0;

	pgSet *groups = coll->GetConnection()->ExecuteSet(
	                      wxT("SELECT opis, poslovna_godina, mb_firme, osnovna_grupa_mb, username\n")
	                      wxT("  FROM adefault_posloviadministratora.grupa\n")
	                      wxT("  WHERE aktivan and vazeci\n")
	                      wxT("ORDER BY prioritet DESC,opis"),
			false);

	if (groups)
	{
		while (!groups->Eof())
		{
			const wxString opis = groups->GetVal(wxT("opis"));
			group = new asoftGroup(opis);
			group->iSetPoslovnaGodina((int)groups->GetLong(wxT("poslovna_godina")));
			group->iSetMbFirme(groups->GetVal(wxT("mb_firme")));
			group->iSetOsnovnaGrupaMb(groups->GetBool(wxT("osnovna_grupa_mb")));
			group->iSetOwner(groups->GetVal(wxT("username")));
			group->iSetDatabase(coll->GetDatabase());

			if (browser)
			{
				browser->AppendObject(coll, group);
				groups->MoveNext();
			}
			else
				break;
		}

		delete groups;
	}
	return group;
}



#include "images/namespace.pngc"
#include "images/namespace-sm.pngc"
#include "images/namespaces.pngc"

wxMenu *asoftSchema::GetNewMenu()
{
	wxMenu *menu = pgObject::GetNewMenu();
	return menu;
}

asoftSchemaFactory::asoftSchemaFactory()
	: asoftGroupObjFactory(__("Schema"), __("New Schema..."), __("Create a new Schema."), namespace_png_img, namespace_sm_png_img)
{
	metaType = PGM_SCHEMA;
}

pgCollection *asoftSchemaFactory::CreateCollection(pgObject *obj)
{
	return new asoftSchemaCollection(GetCollectionFactory(), (asoftGroup *)obj);
}

asoftSchemaFactory asoftschemafactory;
static pgaCollectionFactory scf(&asoftschemafactory, __("Schemas"), namespaces_png_img);


pgCollection *asoftSchemaObjFactory::CreateCollection(pgObject *obj)
{
	return new asoftSchemaObjCollection(GetCollectionFactory(), (asoftSchema *)obj);
}

asoftSchemaCollection::asoftSchemaCollection(pgaFactory *factory, asoftGroup *sch)
	: asoftGroupObjCollection(factory, sch)
{
}

wxString asoftSchemaCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

pgObject *asoftSchemaFactory::CreateObjects(pgCollection *collection, ctlTree *browser, const wxString &restriction)
{
	asoftGroupObjCollection *coll = (asoftGroupObjCollection *)collection;
	asoftSchema *asch = 0;

	pgSet *aschemas = coll->GetGroup()->GetDatabase()->ExecuteSet(
			wxT("select distinct schema from adefault_posloviadministratora.grupa_entitet where grupa = '") +
			coll->GetGroup()->Getopis() +
			wxT("' and vazeci and aktivan order by schema"));

	if (aschemas)
	{
		while (!aschemas->Eof())
		{
			asch = new asoftSchema(coll->GetGroup());
			wxString sch = aschemas->GetVal(wxT("schema"));
			asch->iSetschemaName(sch);
			asch->iSetName(sch);
			asch->iSetDatabase(coll->GetDatabase());
			wxString sqlnsp =
					wxT("SELECT nsp.oid\n")
					wxT("  FROM pg_namespace nsp\n")
					wxT("  WHERE nsp.nspname='")
					+ qtIdent(sch) + wxT("'\n");
			OID nspoid = StrToOid(coll->GetGroup()->GetDatabase()->ExecuteScalar(sqlnsp));
			wxString nspoidstr = NumToStr(nspoid) + wxT("::oid");
			asch->schema = (pgSchema *)schemaFactory.CreateObjects(collection, 0, wxT(" WHERE nsp.oid=") + nspoidstr + wxT("\n"));
//			asch->schema->iSetDatabase(coll->GetDatabase());
			if (browser)
			{
				browser->AppendObject(coll, asch);
				aschemas->MoveNext();
			}
			else
				break;
		}

		delete aschemas;
	}
	return asch;
}

void asoftSchema::ShowTreeDetail(ctlTree *browser, frmMain *form, ctlListView *properties, ctlSQLBox *sqlPane)
{
	if (!expandedKids)
	{
		expandedKids = true;
		browser->RemoveDummyChild(this);
		wxLogInfo(wxT("Adding child object to schema %s"), schemaName.c_str());
		browser->AppendCollection(this, asoftsequencefactory);
		browser->AppendCollection(this, asofttablefactory);
	}
}

wxString asoftSchema::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

wxString asoftSchema::GetSql(ctlTree *browser)
{
	if (sql.IsNull())
	{
		sql = wxT("-- Schema: ") + schemaName + wxT("\n\n");
	}
	return sql;
}

#include "images/table.pngc"
#include "images/table-sm.pngc"
#include "images/tables.pngc"
#include "frm/frmMain.h"

asoftTableFactory::asoftTableFactory()
	: asoftSchemaObjFactory(__("Table"), __("New Table..."), __("Create a new Table."), table_png_img, table_sm_png_img)
{
	aschema = 0;
}

pgCollection *asoftTableFactory::CreateCollection(pgObject *obj)
{
	aschema = (asoftSchema *)obj;
	return new asoftTableCollection(GetCollectionFactory(), aschema);
}

asoftTableFactory asofttablefactory;
static pgaCollectionFactory tcf(&asofttablefactory, __("Tables"), tables_png_img);

asoftTableCollection::asoftTableCollection(pgaFactory *factory, asoftSchema *tbl)
	: asoftSchemaObjCollection(factory, tbl)
{
}

wxString asoftTableCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

pgObject *asoftTableFactory::CreateObjects(pgCollection *coll, ctlTree *browser, const wxString &restriction)
{
	asoftSchemaObjCollection *collection = (asoftSchemaObjCollection *)coll;
	asoftTable *atable = 0;
	wxString opis = collection->GetAsoftSchema()->GetGroup()->Getopis();
	wxString schemaName = GetAsoftSchema()->GetSchemaName();
	wxString sql;
	pgSchema *sch1 = (pgSchema *)coll->GetSchema();
	pgSchema *sch2 = (pgSchema *)collection->GetSchema();
	if (!sch1 || !sch2)
	{
		wxString msg = _("Table schema does not exist !\n") + schemaName;
		wxMessageBox(msg);
		return atable;
	}

	sql = wxT("select entitet from adefault_posloviadministratora.grupa_entitet where grupa='") +
			opis +
			wxT("' and schema ='") +
			qtIdent(schemaName) +
			wxT("' and vazeci and aktivan order by entitet");

	pgSet *tables = collection->GetGroup()->GetDatabase()->ExecuteSet(sql);
	if (tables)
	{
		wxString sqlnsp =
			  wxT("SELECT nsp.oid\n")
		      wxT("  FROM pg_namespace nsp\n")
		      wxT("  WHERE nsp.nspname='")
		      + qtIdent(schemaName) + wxT("'\n");
		OID nspoid = StrToOid(collection->GetGroup()->GetDatabase()->ExecuteScalar(sqlnsp));
		wxString nspoidstr = NumToStr(nspoid) + wxT("::oid");
		while (!tables->Eof())
		{
			wxString name = tables->GetVal(wxT("entitet"));
			atable = new asoftTable(GetAsoftSchema(), name);
			atable->table = (pgTable *)tableFactory.CreateObjects(coll, 0,
					wxT("\n   AND rel.relname='") + qtIdent(name) +
					wxT("'\n   AND rel.relnamespace=") + nspoidstr);
			if (!atable->table)
			{
				wxString msg = _("For some reason table cannot be resolved !\n") + qtIdent(schemaName) + wxT(".") + qtIdent(name);
				wxMessageBox(msg);
				tables->MoveNext();
				wxLogInfo(wxT("Asoft error:%s"), msg.c_str());
			}
			else
			{
				if (browser)
				{
					browser->AppendObject(coll, atable->table);
					tables->MoveNext();
				}
				else
					break;
			}
		}
		delete tables;
	}
	return atable;
}

asoftTable::asoftTable(asoftSchema *newTable, const wxString &newName)
	: asoftSchemaObject(newTable, asofttablefactory, newName)
{
	table = 0;
}

asoftTable::~asoftTable()
{
}

wxString asoftTable::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}





#include "images/sequence.pngc"
#include "images/sequences.pngc"

asoftSequenceFactory::asoftSequenceFactory()
	: asoftSchemaObjFactory(__("Sequence"), __("New Sequence..."), __("Create a new Sequence."), sequence_png_img)
{
	aschema = 0;
}

asoftSequenceCollection::asoftSequenceCollection(pgaFactory *factory, asoftSchema *tbl)
	: asoftSchemaObjCollection(factory, tbl)
{
}

wxString asoftSequenceCollection::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}

pgCollection *asoftSequenceFactory::CreateCollection(pgObject *obj)
{
	aschema = (asoftSchema *)obj;
	return new asoftSequenceCollection(GetCollectionFactory(), aschema);
}

asoftSequenceFactory asoftsequencefactory;
static pgaCollectionFactory cfs(&asoftsequencefactory, __("Sequences"), sequences_png_img);

pgObject *asoftSequenceFactory::CreateObjects(pgCollection *coll, ctlTree *browser, const wxString &restriction)
{
	asoftSchemaObjCollection *collection = (asoftSchemaObjCollection *)coll;
	asoftSequence *asequence = 0;
	wxString opis = collection->GetAsoftSchema()->GetGroup()->Getopis();
	wxString schemaName = GetAsoftSchema()->GetSchemaName();
	wxString sql;
	pgSchema *sch1 = (pgSchema *)coll->GetSchema();
	pgSchema *sch2 = (pgSchema *)collection->GetSchema();
	if (!sch1 || !sch2)
	{
		wxString msg = _("Sequence schema does not exist !\n") + schemaName;
		wxMessageBox(msg);
		return asequence;
	}

	wxString sqlnsp =
			wxT("SELECT nsp.oid\n")
			wxT("  FROM pg_namespace nsp\n")
			wxT("  WHERE nsp.nspname='")
			+ qtIdent(schemaName) + wxT("'\n");
	OID nspoid = StrToOid(collection->GetGroup()->GetDatabase()->ExecuteScalar(sqlnsp));
	wxString nspoidstr = NumToStr(nspoid) + wxT("::oid");

	sql = wxT("SELECT relname")
			wxT("\n  FROM pg_class cl\n")
			wxT(" WHERE relkind = 'S' AND relnamespace  = ") + nspoidstr + wxT("\n")
			wxT(" ORDER BY relname");

	pgSet *sequences = collection->GetGroup()->GetDatabase()->ExecuteSet(sql);
	if (sequences)
	{
		while (!sequences->Eof())
		{
			wxString name = sequences->GetVal(wxT("relname"));
			asequence = new asoftSequence(GetAsoftSchema(), name);
			asequence->sequence = (pgSequence *)sequenceFactory.CreateObjects(coll, 0,
					wxT("\n   AND relname='") + qtIdent(name) + wxT("'"));
			if (!asequence->sequence)
			{
				wxString msg = _("For some reason sequence cannot be resolved !\n") + qtIdent(schemaName) + wxT(".") + qtIdent(name);
				wxMessageBox(msg);
				sequences->MoveNext();
				wxLogInfo(wxT("Asoft error:%s"), msg.c_str());
			}
			else
			{
				if (browser)
				{
					browser->AppendObject(coll, asequence->sequence);
					sequences->MoveNext();
				}
				else
					break;
			}
		}
		delete sequences;
	}
	return asequence;
}

asoftSequence::asoftSequence(asoftSchema *newTable, const wxString &newName)
	: asoftSchemaObject(newTable, asoftsequencefactory, newName)
{
	sequence = 0;
}

asoftSequence::~asoftSequence()
{
}

wxString asoftSequence::GetTranslatedMessage(int kindOfMessage) const
{
	wxString message = wxEmptyString;
	return message;
}
