//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2012, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pasteTables.h - Copy/Paste table(s) functions
//
//////////////////////////////////////////////////////////////////////////

#ifndef PASTETABLES_H
#define	PASTETABLES_H

#include <wx/wx.h>
#include <wx/thread.h>

class pasteTables;
class copyTableFactory;
class copyTableOptionalSelectFactory;
class pasteTablesFactory;

#include "pgAdmin3.h"
#include "frm/frmMain.h"
#include "schema/pgObject.h"
#include "schema/pgSchema.h"
#include "schema/pgTable.h"
#include "schema/pgSequence.h"
#include "schema/pgRule.h"
#include "schema/pgTrigger.h"
#include "schema/pgIndex.h"
#include "ctl/ctlCheckTreeView.h"

// Icons
#include "images/pastetables.pngc"

struct copy_object_tag
{
	wxString groupname;
	wxString servername;
	wxString dbname;
	wxString schemaname;
	wxString tablename;
	pgObject *object;
};

class pasteTables;
class pgTableCopyPaste;
class pgTableCopyPasteArray;

struct transfer_tag
{
	pasteTables *THIS;
	pgDatabase *srcdatabase;
	wxString srcschemaname;
	pgSchema *srcschema;
	pgDatabase *targetdatabase;
	wxString targetschemaname;
	pgSchema *targetschema;
	pgTableCopyPaste *table;
	wxString createsql;
	wxString searchPath;
	pgConn *sourceconn;
	pgConn *targetconn;
	int numberOfCopypasteTables;
	bool pastesuccess;
};

class copyPasteThread : public wxThread
{
private:
	struct transfer_tag *transfer;
public:
	copyPasteThread(struct transfer_tag *transfer1)
	: wxThread(wxTHREAD_JOINABLE), transfer(transfer1)
	{}
	virtual void *Entry();
};

class pgTableCopyPaste
{
public:
	pgTableCopyPaste(pgTable *srctable, pgSchema *targetschema, const wxString &copysuffix,
			pgCollection *columns1, pgCollection *constraints1, pgCollection *tables1, pgCollection *sequences1,
			pgCollection *indexes1, pgCollection *triggers1, pgCollection *rules1);
	virtual ~pgTableCopyPaste();
	virtual wxString GetSql(ctlTree *browser);
	pgTable *Getsrctable() { return srctable; }
	pgSchema *Gettargetschema() { return targetschema; }
	wxString copysuffix;
	wxString srcgroupname;
	wxString srcservername;
	wxString srcdbname;
	wxString srctablename;
	wxString targetgroupname;
	wxString targetservername;
	wxString targetdbname;
private:
	pgTable *srctable;
	pgSchema *targetschema;
	pgCollection *columns;
	pgCollection *constraints;
	pgCollection *tables;
	pgCollection *sequences;
	pgCollection *indexes;
	pgCollection *triggers;
	pgCollection *rules;
	wxString GetSequenceName(const wxString &definition, bool schemaonly = false);
};

class pasteTablesFactory;
class pasteTables
{
public:
	pasteTables(frmMain *form, struct copy_object_tag *sourceobj, pgObject *targetobj, pasteTablesFactory *factory);
	int process();
	int processListOfTables();
	bool pasteNextTable();
	void commitChanges();
	virtual ~pasteTables();
	void copyTable(struct transfer_tag *transfer);
	static pgTable *findTable(frmMain *form, pgSchema *srcschema, const wxString &tablename);
	static pgSchema *findSchema(frmMain *form, pgDatabase *database, const wxString &schemaname);
	static pgDatabase *findDB(frmMain *form, const wxString &groupname, const wxString &servername, const wxString &dbname, bool shouldbeconnected = true);
	static pgServer *findServer(frmMain *form, const wxString &groupname, const wxString &servername);
	frmMain *Getmainform()
	{
		return mainform;
	}
	pasteTablesFactory *GetFactory()
	{
		return factory;
	}
    static void setActive(bool active)
	{
		pasteTables::active = active;
	}
    static bool isActive() {
		return pasteTables::active;
	}
    pgObject *getTargetobj() const {
		return targetobj;
	}
	int copied;
	pgTableCopyPasteArray *tableCopyPasteArray;
	copyPasteThread *thread;
	pgError  lastResultError;
private:
	bool tableExists(pgSchema *srcschema, wxString &tablename);
	void GetLastResultError(pgConn *conn, PGresult *res, const wxString &msg = wxT(""));
	void handleCopyOut(pgConn *conn, wxFile & copystream);
	void handleCopyIn(pgConn *conn, wxFile & copystream, bool isbinary);
	void do_copy(pgConn *conn, wxString & sql, wxFile & copystream);
	void myLogNotice(const wxChar *szFormat, ...);

	frmMain  *mainform;
	struct copy_object_tag *sourceobj;
	pgObject *targetobj;
	wxString pastemsg;
	struct transfer_tag *transfer;
	pasteTablesFactory *factory;
	static bool active;
};

class pgSequenceCopyPaste
{
public:
	pgSequenceCopyPaste(pgSequence *srcseq, pgSchema *targetschema, const wxString &copysuffix);
	virtual ~pgSequenceCopyPaste();
	virtual wxString GetSql(ctlTree *browser);
private:
	pgSequence *srcseq;
	pgSchema *targetschema;
	wxString copysuffix;
};

class pgIndexCopyPaste
{
public:
	pgIndexCopyPaste(pgIndex *srcindex, pgSchema *targetschema, const wxString &copysuffix);
	virtual ~pgIndexCopyPaste();
	virtual wxString GetSql(ctlTree *browser);
	wxString GetCreate();
private:
	pgIndex *srcindex;
	pgSchema *targetschema;
	wxString copysuffix;
};

class pgTriggerCopyPaste
{
public:
	pgTriggerCopyPaste(pgTrigger *srctrigger, pgSchema *targetschema, const wxString &copysuffix);
	virtual ~pgTriggerCopyPaste();
	virtual wxString GetSql(ctlTree *browser);
private:
	pgTrigger *srctrigger;
	pgSchema *targetschema;
	wxString copysuffix;
};

class pgRuleCopyPaste
{
public:
	pgRuleCopyPaste(pgRule *srcrule, pgSchema *targetschema, const wxString &copysuffix);
	virtual ~pgRuleCopyPaste();
	virtual wxString GetSql(ctlTree *browser);
private:
	pgRule *srcrule;
	pgSchema *targetschema;
	wxString copysuffix;
};

WX_DECLARE_OBJARRAY(pgTableCopyPaste, pgTableCopyPasteArray);

WX_DECLARE_STRING_HASH_MAP(wxString, MyHash);

WX_DECLARE_OBJARRAY(struct copy_object_tag, selectedTablesArray);

class frmCopyTables : public ExecutionDialog
{
public:
	frmCopyTables(frmMain *form, pgObject *_object);
	~frmCopyTables();
	wxString GetSql();
	bool treeDetails(const wxString &servername, const wxString &dbname, const wxString &schemaname);
	MyHash hashgroupname, hashservername, hashservernamename;
	static selectedTablesArray selectedTables;
private:
	wxString GetHelpPage() const;
	void OnOK(wxCommandEvent &ev);
	void addDatabases(pgServer *server, ctlTree *browser, wxTreeItemId &servernode);
	void addSchemaTables(pgDatabase *db, ctlTree *browser, wxTreeItemId &dbnode);
	void addTables(pgSchema *srcschema, ctlTree *browser, wxTreeItemId &schemanode);
	pgObject *object;

	DECLARE_EVENT_TABLE()
};

class copyTableFactory : public contextActionFactory
{
public:
	copyTableFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar);
	wxWindow *StartDialog(frmMain *form, pgObject *obj);
	bool CheckEnable(pgObject *obj);
	static struct copy_object_tag *copytObject;
};

class copyTableOptionalSelectFactory : public contextActionFactory
{
public:
	copyTableOptionalSelectFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar);
	wxWindow *StartDialog(frmMain *form, pgObject *obj);
	bool CheckEnable(pgObject *obj);
	static bool available;
	static wxString optsel;
};

class copyTablesListFactory : public contextActionFactory
{
public:
	copyTablesListFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar);
	wxWindow *StartDialog(frmMain *form, pgObject *obj);
	bool CheckEnable(pgObject *obj);
};

class pasteTablesFactory : public contextActionFactory
{
public:
	pasteTablesFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar);
	wxWindow *StartDialog(frmMain *form, pgObject *obj);
	bool CheckEnable(pgObject *obj);
	ctlMenuToolbar *GetToolbar()
	{
		return toolbar;
	}
private:
	ctlMenuToolbar *toolbar;
};
#endif	/* PASTETABLES_H */
