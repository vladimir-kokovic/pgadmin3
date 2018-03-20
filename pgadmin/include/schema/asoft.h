//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2014, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// asoft.h - Asoft DB groups - Pionir doo Beograd ERP
//
// Asoft DB hierarchy:
// 1.  groups       asoftGroup
// 2     schemas    asoftSchema
// 3.      tables   asoftTable
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ASOFT_H
#define __ASOFT_H

#include "pgDatabase.h"
#include "pgTable.h"
#include "pgSequence.h"

class asoftGroupBaseFactory : public pgDatabaseObjFactory
{
public:
	asoftGroupBaseFactory(const wxChar *tn, const wxChar *ns, const wxChar *nls, wxImage *img, wxImage *imgSm = 0);
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
	{
		return 0;
	}
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
};

class asoftGroupFactory : public asoftGroupBaseFactory
{
public:
	asoftGroupFactory();
	virtual pgCollection *CreateCollection(pgObject *obj);
};

extern asoftGroupFactory asoftgroupfactory;

class asoftGroupBase : public pgDatabaseObject
{
public:
	asoftGroupBase(pgaFactory &factory, const wxString &newName = wxT(""));

	wxString GetOpis() const
	{
		return opis;
	}
	wxString GetQuotedPrefix() const
	{
		return database->GetQuotedSchemaPrefix(GetName());
	}
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0);
	static pgObject *ReadObjects(pgCollection *collection, ctlTree *browser);

	long GetSchemaTyp() const
	{
		return schemaTyp;
	}
	void iSetSchemaTyp(const long l)
	{
		schemaTyp = l;
	}
	bool WantDummyChild()
	{
		return true;
	}

	wxMenu *GetNewMenu();
	wxString GetSql(ctlTree *browser);

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

	wxString opis;
	wxString mb_firme;
	bool osnovna_grupa_mb;
	int poslovna_godina;

private:
	long schemaTyp;
};

class asoftGroup : public asoftGroupBase
{
public:
	asoftGroup(const wxString &newName = wxT(""));
	wxString GetTranslatedMessage(int kindOfMessage) const;
	pgObject *Refresh(ctlTree *browser, const wxTreeItemId item);
	void iSetOpis(const wxString &s)
	{
		opis = s;
	}
	wxString Getopis() const
	{
		return opis;
	}
	void iSetPoslovnaGodina(int i)
	{
		poslovna_godina = i;
	}
	int GetPoslovnaGodina()
	{
		return poslovna_godina;
	}
	void iSetMbFirme(const wxString &s)
	{
		mb_firme = s;
	}
	wxString GetMbFirme() const
	{
		return mb_firme;
	}
	void iSetOsnovnaGrupaMb(bool b)
	{
		osnovna_grupa_mb = b;
	}
	bool GetOsnovnaGrupaMb()
	{
		return osnovna_grupa_mb;
	}
};

/////////////////////////////////////////////////////

class asoftGroupObjFactory : public pgDatabaseObjFactory
{
public:
	asoftGroupObjFactory(const wxChar *tn, const wxChar *ns, const wxChar *nls, wxImage *img, wxImage *imgSm = 0)
		: pgDatabaseObjFactory(tn, ns, nls, img, imgSm) {}
	virtual pgCollection *CreateCollection(pgObject *obj);
};

// Object that lives in a schema
class asoftGroupObject : public pgDatabaseObject
{
public:
	asoftGroupObject(asoftGroup *newGroup, pgaFactory &factory, const wxString &newName = wxEmptyString) : pgDatabaseObject(factory, newName)
	{
		SetGroup(newGroup);
	}
	asoftGroupObject(asoftGroup *newGroup, int newType, const wxString &newName = wxT("")) : pgDatabaseObject(newType, newName)
	{
		SetGroup(newGroup);
	}
	void SetGroup(asoftGroup *newGroup)
	{
		group = newGroup;
	}
	virtual asoftGroup *GetGroup() const
	{
		return group;
	}

protected:
	asoftGroup *group;
};


class asoftGroupCollection : public pgDatabaseObjCollection
{
public:
	asoftGroupCollection(pgaFactory *factory, pgDatabase *db);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};


// collection of asoftGroupObject
class asoftGroupObjCollection : public pgCollection
{
public:
	asoftGroupObjCollection(pgaFactory *factory, asoftGroup *ag);
	virtual bool CanCreate()
	{
		return false;
	}
	virtual asoftGroup *GetGroup() const
	{
		return group;
	}

protected:
	asoftGroup *group;
};




class asoftSchemaFactory : public asoftGroupObjFactory
{
public:
	asoftSchemaFactory();
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
	{
		return 0;
	}
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
	virtual pgCollection *CreateCollection(pgObject *obj);
};
extern asoftSchemaFactory asoftschemafactory;

class asoftSchema : public asoftGroupObject
{
public:
	asoftSchema(asoftGroup *newSchema, const wxString &newName = wxT("")) : asoftGroupObject(newSchema, asoftschemafactory, newName)
	{
		schema = 0;
	}
	asoftSchema(asoftGroup *newSchema, pgaFactory &newFactory, const wxString &newName = wxT("")) : asoftGroupObject(newSchema, newFactory, newName)
	{
		schema = 0;
	}
	~asoftSchema()
	{
	}
	wxString GetSql(ctlTree *browser);
	wxString GetTranslatedMessage(int kindOfMessage) const;
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0);
	bool WantDummyChild()
	{
		return true;
	}
	bool HasStats()
	{
		return true;
	}
	bool HasDepends()
	{
		return true;
	}
	bool HasReferences()
	{
		return true;
	}
	void iSetschemaName(const wxString &s)
	{
		schemaName = s;
		schema = new pgSchema(s);
	}
	wxString GetSchemaName() const
	{
		return schemaName;
	}
	pgSchema *GetSchema() const
	{
		return schema;
	}
	virtual wxMenu *GetNewMenu();

	wxString schemaName;
	pgSchema *schema;
};


class asoftSchemaObject : public asoftGroupObject
{
public:
	asoftSchemaObject(asoftSchema *newSchema, pgaFactory &factory, const wxString &newName = wxT(""))
		: asoftGroupObject(newSchema->GetGroup(), factory, newName)
	{
		aschema = newSchema;
	}
	asoftSchema *GetAsoftSchema() const
	{
		return aschema;
	}
	OID GetTableOid() const
	{
		return aschema->GetOid();
	}
	wxString GetTableOidStr() const
	{
		return NumToStr(aschema->GetOid()) + wxT("::oid");
	}

protected:
	asoftSchema *aschema;
};


class asoftSchemaCollection : public asoftGroupObjCollection
{
public:
	asoftSchemaCollection(pgaFactory *factory, asoftGroup *sch);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};

class asoftSchemaObjCollection : public asoftGroupObjCollection
{
public:
	asoftSchemaObjCollection(pgaFactory *factory, asoftSchema *aschema)
		: asoftGroupObjCollection(factory, aschema->GetGroup())
	{
		asoftschema = aschema;
		server = asoftschema->GetGroup()->GetDatabase()->GetServer();
		database = asoftschema->GetGroup()->GetDatabase();
		schema = asoftschema->GetSchema();
	}
	asoftSchema *GetAsoftSchema() const
	{
		return asoftschema;
	}
	bool CanCreate()
	{
		return false;
	}

protected:
	asoftSchema *asoftschema;
};

class asoftSchemaObjFactory : public asoftGroupObjFactory
{
public:
	asoftSchemaObjFactory(const wxChar *tn, const wxChar *ns, const wxChar *nls, wxImage *img, wxImage *imgSm = 0)
		: asoftGroupObjFactory(tn, ns, nls, img, imgSm) {}
	virtual pgCollection *CreateCollection(pgObject *obj);
};








class asoftTableFactory : public asoftSchemaObjFactory
{
public:
	asoftTableFactory();
	asoftSchema *GetAsoftSchema() const
	{
		return aschema;
	}
	pgSchema *GetSchema() const
	{
		return schema;
	}
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
	{
		return 0;
	}
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
	virtual pgCollection *CreateCollection(pgObject *obj);

protected:
	asoftSchema *aschema;
	pgSchema *schema;
};
extern asoftTableFactory asofttablefactory;

class asoftTable : public asoftSchemaObject
{
public:
	asoftTable(asoftSchema *newTable, const wxString &newName = wxT(""));
	~asoftTable();
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0)
	{
	}
	wxString GetTranslatedMessage(int kindOfMessage) const;

	pgTable *table;
};

class asoftTableCollection : public asoftSchemaObjCollection
{
public:
	asoftTableCollection(pgaFactory *factory, asoftSchema *tbl);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};






class asoftSequenceFactory : public asoftSchemaObjFactory
{
public:
	asoftSequenceFactory();
	asoftSchema *GetAsoftSchema() const
	{
		return aschema;
	}
	pgSchema *GetSchema() const
	{
		return schema;
	}
	virtual dlgProperty *CreateDialog(frmMain *frame, pgObject *node, pgObject *parent)
	{
		return 0;
	}
	virtual pgObject *CreateObjects(pgCollection *obj, ctlTree *browser, const wxString &restr = wxEmptyString);
	virtual pgCollection *CreateCollection(pgObject *obj);

protected:
	asoftSchema *aschema;
	pgSchema *schema;
};
extern asoftSequenceFactory asoftsequencefactory;

class asoftSequence : public asoftSchemaObject
{
public:
	asoftSequence(asoftSchema *newTable, const wxString &newName = wxT(""));
	~asoftSequence();
	void ShowTreeDetail(ctlTree *browser, frmMain *form = 0, ctlListView *properties = 0, ctlSQLBox *sqlPane = 0)
	{
	}
	wxString GetTranslatedMessage(int kindOfMessage) const;

	pgSequence *sequence;
};

class asoftSequenceCollection : public asoftSchemaObjCollection
{
public:
	asoftSequenceCollection(pgaFactory *factory, asoftSchema *tbl);
	wxString GetTranslatedMessage(int kindOfMessage) const;
};

#endif
