//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2012, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// pasteTables.cpp - Copy/Paste table(s) functions
//
//////////////////////////////////////////////////////////////////////////

#include "utils/pgadminScanner.h"

// wxWindows headers
#include <wx/wfstream.h>

// PostgreSQL headers
#include <libpq-fe.h>

#include "utils/pasteTables.h"
#include "schema/pgColumn.h"
#include "schema/pgSequence.h"
#include "schema/pgIndexConstraint.h"
#include "schema/pgForeignKey.h"
#include "schema/pgCheck.h"
#include "schema/pgFunction.h"
#include "ctl/ctlMenuToolbar.h"

#include <errno.h>

#include <wx/arrimpl.cpp>
#include <wx/generic/treectlg.h>
WX_DEFINE_OBJARRAY(pgTableCopyPasteArray);
WX_DEFINE_OBJARRAY(selectedTablesArray);

bool pasteTables::active = false;
struct copy_object_tag *copyTableFactory::copytObject = NULL;
bool copyTableOptionalSelectFactory::available = false;
wxString copyTableOptionalSelectFactory::optsel = wxEmptyString;
selectedTablesArray frmCopyTables::selectedTables;

pasteTables::pasteTables(frmMain *form, struct copy_object_tag *sourceobj, pgObject *targetobj, pasteTablesFactory *factory)
{
	this->mainform = form;
	this->sourceobj = sourceobj;
	this->targetobj = targetobj;
	this->factory = factory;
}

bool pasteTables::tableExists(pgSchema *srcschema, wxString &tablename)
{
	pgSet *set = srcschema->GetDatabase()->ExecuteSet(
		wxT("SELECT tablename\n")
		wxT("  FROM pg_tables\n")
		wxT(" WHERE schemaname=") + srcschema->qtDbString(srcschema->GetName()) +
		wxT(" AND tablename=") + srcschema->qtDbString(tablename));
	if (set)
	{
		long cnt = set->NumRows();
		delete set;
		if (cnt)
			return true;
	}
	return false;
}

pgTable *pasteTables::findTable(frmMain *frm, pgSchema *srcschema, const wxString &tablename)
{
	ctlTree *browser = frm->GetBrowser();
	srcschema->ShowTreeDetail(browser);
	pgCollection *tables = browser->FindCollection(tableFactory, srcschema->GetId());
	if (tables)
	{
		tables->ShowTreeDetail(browser);
		pgObject *obj;
		pgTable *srctable;
		treeObjectIterator colIt(browser, tables);
		while ((obj = colIt.GetNextObject()) != 0)
		{
			obj->ShowTreeDetail(browser);
			srctable = (pgTable *) obj;
			if (srctable->GetName() == tablename)
				return srctable;
		}
	}
	return 0;
}

pgSchema *pasteTables::findSchema(frmMain *frm, pgDatabase *database, const wxString &schemaname)
{
	ctlTree *browser = frm->GetBrowser();
	pgCollection *schemas = browser->FindCollection(schemaFactory, database->GetId());
	if (schemas)
	{
		pgObject *obj;
		pgSchema *srcschema;
		treeObjectIterator colIt(browser, schemas);
		while ((obj = colIt.GetNextObject()) != 0)
		{
			srcschema = (pgSchema *) obj;
			if (srcschema->GetIdentifier() == schemaname)
				return srcschema;
		}
	}
	return 0;
}

pgDatabase *pasteTables::findDB(
	frmMain *frm, const wxString &groupname, const wxString &servername, const wxString &dbname, bool shouldbeconnected)
{
	ctlTree *browser = frm->GetBrowser();
	if (browser->ItemHasChildren(browser->GetRootItem()))
	{
		wxTreeItemIdValue groupcookie;
		wxTreeItemId groupItem = browser->GetFirstChild(browser->GetRootItem(), groupcookie);
		while (groupItem)
		{
			wxCookieType cookie;
			wxTreeItemId serverItem = browser->GetFirstChild(groupItem, cookie);
			while (serverItem)
			{
				pgServer *server = (pgServer *) browser->GetObject(serverItem);
				if (server && server->IsCreatedBy(serverFactory) && server->connection())
				{
					if (server->connection()->IsAlive() && server->GetName() == servername && server->GetGroup() == groupname)
					{
						wxCookieType cookie2;
						wxTreeItemId item = browser->GetFirstChild(serverItem, cookie2);
						while (item)
						{
							pgObject *obj = browser->GetObject(item);
							if (obj && obj->IsCreatedBy(databaseFactory.GetCollectionFactory()))
							{
								wxCookieType cookie3;
								item = browser->GetFirstChild(obj->GetId(), cookie3);
								while (item)
								{
									pgDatabase *db = (pgDatabase *) browser->GetObject(item);
									if (db && db->IsCreatedBy(databaseFactory))
									{
										if (shouldbeconnected)
										{
											pgConn *conn = db->GetConnection();
											if (conn)
											{
												if (!(!conn->IsAlive() && (conn->GetStatus() == PGCONN_BROKEN ||
													conn->GetStatus() == PGCONN_BAD)))
												{
													if (db->GetName() == dbname)
														return db;
												}
											}
										}
										else
										{
											if (db->GetName() == dbname)
												return db;
										}
									}
									item = browser->GetNextChild(obj->GetId(), cookie3);
								}
							}
							item = browser->GetNextChild(serverItem, cookie2);
						}
					}
				}
				serverItem = browser->GetNextChild(groupItem, cookie);
			}
			groupItem = browser->GetNextChild(browser->GetRootItem(), groupcookie);
		}
	}

	return 0;
}

pgServer *pasteTables::findServer(frmMain *frm, const wxString &groupname, const wxString &servername)
{
	ctlTree *browser = frm->GetBrowser();
	if (browser->ItemHasChildren(browser->GetRootItem()))
	{
		wxTreeItemIdValue groupcookie;
		wxTreeItemId groupItem = browser->GetFirstChild(browser->GetRootItem(), groupcookie);
		while (groupItem)
		{
			wxCookieType cookie;
			wxTreeItemId serverItem = browser->GetFirstChild(groupItem, cookie);
			while (serverItem)
			{
				pgServer *server = (pgServer *) browser->GetObject(serverItem);
				if (server && server->IsCreatedBy(serverFactory))
				{
					if (server->GetDescription() == servername && server->GetGroup() == groupname)
					{
						return server;
					}
				}
				serverItem = browser->GetNextChild(groupItem, cookie);
			}
			groupItem = browser->GetNextChild(browser->GetRootItem(), groupcookie);
		}
	}

	return 0;
}

void pasteTables::GetLastResultError(pgConn *conn, PGresult *res, const wxString &msg)
{
	if (res)
	{
		lastResultError.severity = wxString(PQresultErrorField(res, PG_DIAG_SEVERITY), *conn->GetConv());
		lastResultError.sql_state = wxString(PQresultErrorField(res, PG_DIAG_SQLSTATE), *conn->GetConv());
		lastResultError.msg_primary = wxString(PQresultErrorField(res, PG_DIAG_MESSAGE_PRIMARY), *conn->GetConv());
		lastResultError.msg_detail = wxString(PQresultErrorField(res, PG_DIAG_MESSAGE_DETAIL), *conn->GetConv());
		lastResultError.msg_hint = wxString(PQresultErrorField(res, PG_DIAG_MESSAGE_HINT), *conn->GetConv());
		lastResultError.statement_pos = wxString(PQresultErrorField(res, PG_DIAG_STATEMENT_POSITION), *conn->GetConv());
		lastResultError.internal_pos = wxString(PQresultErrorField(res, PG_DIAG_INTERNAL_POSITION), *conn->GetConv());
		lastResultError.internal_query = wxString(PQresultErrorField(res, PG_DIAG_INTERNAL_QUERY), *conn->GetConv());
		lastResultError.context = wxString(PQresultErrorField(res, PG_DIAG_CONTEXT), *conn->GetConv());
		lastResultError.source_file = wxString(PQresultErrorField(res, PG_DIAG_SOURCE_FILE), *conn->GetConv());
		lastResultError.source_line = wxString(PQresultErrorField(res, PG_DIAG_SOURCE_LINE), *conn->GetConv());
		lastResultError.source_function = wxString(PQresultErrorField(res, PG_DIAG_SOURCE_FUNCTION), *conn->GetConv());
	}
	else
	{
		lastResultError.severity = wxEmptyString;
		lastResultError.sql_state = wxEmptyString;
		if (msg.IsEmpty())
			lastResultError.msg_primary = conn->GetLastError();
		else
			lastResultError.msg_primary = msg;
		lastResultError.msg_detail = wxEmptyString;
		lastResultError.msg_hint = wxEmptyString;
		lastResultError.statement_pos = wxEmptyString;
		lastResultError.internal_pos = wxEmptyString;
		lastResultError.internal_query = wxEmptyString;
		lastResultError.context = wxEmptyString;
		lastResultError.source_file = wxEmptyString;
		lastResultError.source_line = wxEmptyString;
		lastResultError.source_function = wxEmptyString;
	}

	wxString errMsg;

	if (lastResultError.severity != wxEmptyString && lastResultError.msg_primary != wxEmptyString)
		errMsg = lastResultError.severity + wxT(": ") + lastResultError.msg_primary;
	else if (lastResultError.msg_primary != wxEmptyString)
		errMsg = lastResultError.msg_primary;

	if (!lastResultError.sql_state.IsEmpty())
	{
		if (!errMsg.EndsWith(wxT("\n")))
			errMsg += wxT("\n");
		errMsg += _("SQL state: ");
		errMsg += lastResultError.sql_state;
	}

	if (!lastResultError.msg_detail.IsEmpty())
	{
		if (!errMsg.EndsWith(wxT("\n")))
			errMsg += wxT("\n");
		errMsg += _("Detail: ");
		errMsg += lastResultError.msg_detail;
	}

	if (!lastResultError.msg_hint.IsEmpty())
	{
		if (!errMsg.EndsWith(wxT("\n")))
			errMsg += wxT("\n");
		errMsg += _("Hint: ");
		errMsg += lastResultError.msg_hint;
	}

	if (!lastResultError.statement_pos.IsEmpty())
	{
		if (!errMsg.EndsWith(wxT("\n")))
			errMsg += wxT("\n");
		errMsg += _("Character: ");
		errMsg += lastResultError.statement_pos;
	}

	if (!lastResultError.context.IsEmpty())
	{
		if (!errMsg.EndsWith(wxT("\n")))
			errMsg += wxT("\n");
		errMsg += _("Context: ");
		errMsg += lastResultError.context;
	}
	lastResultError.formatted_msg = errMsg;
}

/*
 * Functions for handling COPY IN/OUT data transfer->
 *
 * If you want to use COPY TO STDOUT/FROM STDIN in your application,
 * this is the code to steal ;)
 */

/*
 * handleCopyOut
 * receives data as a result of a COPY ... TO STDOUT command
 *
 * conn should be a database connection that you just issued COPY TO on
 * and got back a PGRES_COPY_OUT result.
 * copystream is the file stream for the data to go to.
 *
 * result is true if successful, false if not.
 */
void pasteTables::handleCopyOut(pgConn *conn, wxFile & copystream)
{
	bool OK = true;
	char *buf;
	int ret;
	PGresult *res;
	int counter = 0;

	lastResultError.formatted_msg = wxT("");
	for (;;)
	{
		counter++;
		if (counter % 100)
		{
			if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
				break;
		}
		ret = PQgetCopyData(conn->connection(), &buf, 0);

		if (ret < 0)
			break; /* done or error */

		if (buf)
		{
			int n;
			wxString x = wxString(buf, wxConvUTF8);
			if (!x)
				n = -1;
			else
				n = copystream.Write(x);
			if (n != 1)
			{
				if (OK) /* complain only once, keep reading data */
					lastResultError.formatted_msg.Format(_("could not write COPY data: %s\n"), strerror(errno));
				OK = false;
			}
			PQfreemem(buf);
		}
	}

	if (OK && !copystream.Flush())
	{
		lastResultError.formatted_msg.Format(_("could not write COPY data: %s\n"), strerror(errno));
		OK = false;
	}

	if (ret == -2)
	{
		lastResultError.formatted_msg.Format(_("COPY data transfer failed: %s"), PQerrorMessage(conn->connection()));
		OK = false;
	}

	/* Check command status and return to normal libpq state */
	res = PQgetResult(conn->connection());
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		if (lastResultError.formatted_msg.IsEmpty())
		{
			GetLastResultError(conn, res);
		}
		OK = false;
	}
	PQclear(res);
}

/*
 * handleCopyIn
 * sends data to complete a COPY ... FROM STDIN command
 *
 * conn should be a database connection that you just issued COPY FROM on
 * and got back a PGRES_COPY_IN result.
 * copystream is the file stream to read the data from.
 * isbinary can be set from PQbinaryTuples().
 *
 * result is true if successful, false if not.
 */

/* read chunk size for COPY IN - size is not critical */
#define COPYBUFSIZ 8192

void pasteTables::handleCopyIn(pgConn *conn, wxFile & copystream, bool isbinary)
{
	bool OK = true;
	char buf[COPYBUFSIZ];
	PGresult *res;
	int counter = 0;

	if (isbinary)
	{
		for (;;)
		{
			counter++;
			if (counter % 100)
			{
				if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
					break;
			}
			int buflen = copystream.Read(buf, 1);
			if (buflen <= 0)
				break;
			if (PQputCopyData(conn->connection(), buf, buflen) <= 0)
			{
				OK = false;
				break;
			}
		}
	}
	else
	{
		wxFileInputStream input(copystream);
		wxTextInputStream textfile(input);

		while (input.CanRead()) /* for each bufferload in line ... */
		{
			counter++;
			if (counter % 100)
			{
				if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
					break;
			}
			wxString buf1 = textfile.ReadLine() + wxT("\n");
			int buflen = buf1.Length();
			if (buf1 == wxT("\n"))
			{
				break;
			}
			const wxCharBuffer wc = buf1.ToUTF8();
			if (!wc) {
				OK = false;
				break;
			}
			const char *tmp = wc.data();
			int lenc = strlen(tmp);
			if (PQputCopyData(conn->connection(), tmp, lenc) <= 0)
			{
				OK = false;
				break;
			}
		}
	}

	/* Terminate data transfer */
	wxString errmsg = _("Aborted because of read failure or data could not be converted to the server encoding.");
	if (PQputCopyEnd(conn->connection(), OK ? NULL : (const char *)errmsg.ToAscii()) <= 0)
		OK = false;

	/* Check command status and return to normal libpq state */
	res = PQgetResult(conn->connection());
	if (PQresultStatus(res) != PGRES_COMMAND_OK)
	{
		GetLastResultError(conn, res);
		OK = false;
	}
	PQclear(res);
}

/*
 * Execute a \copy command (frontend copy). We have to open a file, then
 * submit a COPY query to the backend and either feed it data from the
 * file or route its response into the file.
 */
void pasteTables::do_copy(pgConn *conn, wxString & sql, wxFile & copystream)
{
	PGresult *result;
	struct stat st;

	result = conn->ExecuteOptionalResult(sql);
	switch (PQresultStatus(result))
	{
	case PGRES_COPY_OUT:
		handleCopyOut(conn, copystream);
		break;
	case PGRES_COPY_IN:
		handleCopyIn(conn, copystream, PQbinaryTuples(result));
		break;
	case PGRES_NONFATAL_ERROR:
	case PGRES_FATAL_ERROR:
	case PGRES_BAD_RESPONSE:
		lastResultError.formatted_msg.Format(_("copy error: %s\n%s\n"), PQerrorMessage(conn->connection()), sql.c_str());
		break;
	default:
		lastResultError.formatted_msg.Format(
			_("copy error: unexpected response (%d)\n%s\n"), PQresultStatus(result), sql.c_str());
		break;
	}

	PQclear(result);

	/*
	 * Make sure we have pumped libpq dry of results; else it may still be in
	 * ASYNC_BUSY state, leading to false readings in, eg, get_prompt().
	 */
	while ((result = PQgetResult(conn->connection())) != NULL)
	{
		lastResultError.formatted_msg.Format(_("copy: unexpected response (%d)\n"), PQresultStatus(result));
		/* if still in COPY IN state, try to get out of it */
		if (PQresultStatus(result) == PGRES_COPY_IN)
			PQputCopyEnd(conn->connection(), (const char *) _("trying to exit copy mode"));
		PQclear(result);
	}
}

void pasteTables::myLogNotice(const wxChar *szFormat, ...)
{
	wxLogLevel savedlogLevel = sysLogger::logLevel;
	sysLogger::logLevel = LOG_NOTICE;
	va_list argptr;
	va_start(argptr, szFormat);
#if wxCHECK_VERSION(2, 9, 0)
	wxChar s_szBuf[8192];
	if (sysLogger::logLevel >= LOG_NOTICE)
	{
		wxVsnprintf(s_szBuf, WXSIZEOF(s_szBuf), szFormat, argptr);
		wxLog::OnLog(wxLOG_Notice, s_szBuf, time(NULL));
	}
#else
	wxVLogNotice(szFormat, argptr);
#endif
	va_end(argptr);
	sysLogger::logLevel = savedlogLevel;
}

void pasteTables::copyTable(struct transfer_tag *transfer)
{
	lastResultError.formatted_msg = wxT("");
	bool rc;

	if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
		return;
#if wxCHECK_VERSION(2, 9, 0)
	myLogNotice(wxT("CopyPaste=\"\n%s\""), (const char *)(transfer->createsql.c_str()));
#else
	myLogNotice(wxT("CopyPaste=\"\n%s\""), transfer->createsql.c_str());
#endif
	rc = transfer->targetconn->ExecuteVoid(transfer->createsql, false);
	if (!rc)
	{
		lastResultError = transfer->targetconn->GetLastResultError();
		lastResultError.formatted_msg = pastemsg + wxT("\n") + lastResultError.formatted_msg;
		return;
	}
	if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
		return;
	if (!transfer->searchPath.IsEmpty())
	{
		rc = transfer->targetconn->ExecuteVoid(wxT("SET search_path=") + transfer->searchPath, false);
		if (!rc)
		{
			lastResultError = transfer->targetconn->GetLastResultError();
			lastResultError.formatted_msg = pastemsg + wxT("\n") + lastResultError.formatted_msg;
			return;
		}
	}

	if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
		return;
	if (transfer->sourceconn->GetDbname() == transfer->targetconn->GetDbname() &&
			transfer->sourceconn->GetHost() == transfer->targetconn->GetHost() &&
			transfer->sourceconn->GetPort() == transfer->targetconn->GetPort())
	{
		//same DB server
		wxString tablenamenew = qtIdent(transfer->table->srctablename);
		bool addsuffix = (!transfer->table->copysuffix.IsEmpty());
		if (addsuffix)
		{
			if (tablenamenew.StartsWith(wxT("\"")))
			{
				tablenamenew = tablenamenew.Mid(1, tablenamenew.length() - 2) + transfer->table->copysuffix;
				tablenamenew = wxT("\"") + tablenamenew + wxT("\"");
			}
			else
			{
				tablenamenew += transfer->table->copysuffix;
				tablenamenew = qtIdent(tablenamenew);
			}
		}
		wxString copysql;
		if (copyTableOptionalSelectFactory::available)
		{
			copysql =
				wxT("\nINSERT INTO ") +
				qtIdent(transfer->targetschemaname) + wxT(".") + tablenamenew +
				wxT(" (SELECT * FROM ") +
				qtIdent(transfer->srcschemaname) + wxT(".") + qtIdent(transfer->table->srctablename) +
				((copyTableOptionalSelectFactory::optsel.IsEmpty()) ?
				wxT("") : (wxT(" WHERE ") + copyTableOptionalSelectFactory::optsel)) +
				wxT(")\n\n");
		}
		else
			copysql =
				wxT("\nINSERT INTO ") +
				qtIdent(transfer->targetschemaname) + wxT(".") + tablenamenew +
				wxT(" (SELECT * FROM ") +
				qtIdent(transfer->srcschemaname) + wxT(".") + qtIdent(transfer->table->srctablename) +
				wxT(")\n\n");
		rc = transfer->targetconn->ExecuteVoid(copysql, false);
		if (!rc)
		{
			lastResultError = transfer->targetconn->GetLastResultError();
			lastResultError.formatted_msg = pastemsg + wxT("\n") + lastResultError.formatted_msg;
			return;
		}
		if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
			return;
	}
	else
	{
		//different DB servers
		wxString tmpFilename;
		wxFile tmpFile;
		tmpFilename = wxFileName::CreateTempFileName(wxT("copytable"));
		tmpFile.Open(tmpFilename.c_str(), wxFile::write);
		if (!tmpFile.IsOpened())
		{
			lastResultError.formatted_msg = _("Can't create temporary file: ") + tmpFilename;
			return;
		}
		if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
			return;
		wxString copysql;
		if (copyTableOptionalSelectFactory::available)
		{
			copysql =
				wxT("COPY (SELECT * FROM ") +
				qtIdent(transfer->srcschemaname) + wxT(".") + qtIdent(transfer->table->srctablename) +
				((copyTableOptionalSelectFactory::optsel.IsEmpty()) ?
				wxT("") : (wxT(" WHERE ") + copyTableOptionalSelectFactory::optsel)) +
				wxT(") TO STDOUT");
		}
		else
			copysql =
				wxT("COPY ")
				+ qtIdent(transfer->srcschemaname) + wxT(".") + qtIdent(transfer->table->srctablename)
				+ wxT(" TO STDOUT");
		do_copy(transfer->sourceconn, copysql, tmpFile);
		if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
			return;
		if (lastResultError.formatted_msg.IsEmpty())
		{
			tmpFile.Close();
			tmpFile.Open(tmpFilename.c_str(), wxFile::read);
			if (!tmpFile.IsOpened())
			{
				lastResultError.formatted_msg = _("Can't open temporary file: ") + tmpFilename;
				wxRemoveFile(tmpFilename);
				return;
			}
			if (transfer->THIS->thread && transfer->THIS->thread->TestDestroy())
			{
				wxRemoveFile(tmpFilename);
				return;
			}
			copysql =
					wxT("COPY ")
					+ qtIdent(transfer->targetschemaname) + wxT(".") +
					qtIdent(transfer->table->srctablename + transfer->table->copysuffix)
					+ wxT(" FROM STDIN");
			do_copy(transfer->targetconn, copysql, tmpFile);
		}
		tmpFile.Close();
		wxRemoveFile(tmpFilename);
	}
	copied++;
}

void *copyPasteThread::Entry()
{
	if (transfer)
	{
		if (transfer->THIS)
		{
			transfer->THIS->copyTable(transfer);
			if (!TestDestroy())
			{
				wxCommandEvent event(EVT_THREAD_COPYPASTE_UPDATE_GUI);
				event.SetClientData(transfer);
				wxPostEvent(winMain->GetEventHandler(), event);
			}
		}
	}
	return (NULL);
}

int pasteTables::process()
{
	if (!sourceobj || !targetobj)
	{
		return 0;
	}

	pgSchema *targetschema = (pgSchema *) targetobj;

	pgDatabase *db = pasteTables::findDB(mainform, sourceobj->groupname, sourceobj->servername, sourceobj->dbname);
	if (!db)
		return 0;
	pgSchema *srcschema = pasteTables::findSchema(mainform, db, sourceobj->schemaname);
	if (!srcschema)
		return 0;
	pgTable *table = pasteTables::findTable(mainform, srcschema, sourceobj->tablename);

	pgConn *sourceconn;
	pgConn *targetconn = targetobj->GetConnection();

	if (table)
	{
		wxString msg;
		if (copyTableOptionalSelectFactory::available)
			msg = wxString::Format(_("\nwith condition WHERE %s "), copyTableOptionalSelectFactory::optsel.c_str());
		if (wxMessageBox(
			_("Paste source table ?\n") +
			table->GetSchema()->GetDatabase()->GetQuotedIdentifier() + wxT(".") + table->GetSchema()->GetQuotedIdentifier() +
			wxT(".") + table->GetQuotedIdentifier() + msg + wxT("\n") +
			wxT(" into schema\n") + targetschema->GetDatabase()->GetQuotedIdentifier() + wxT(".") +
			targetschema->GetQuotedIdentifier(), _("Paste table"), wxYES_NO) != wxYES)
		{
			return 0;
		}
		srcschema = table->GetSchema();
		sourceconn = table->GetConnection();
	}
	else
	{
		if (wxMessageBox(
			_("Paste schema tables ?\n") +
			srcschema->GetDatabase()->GetQuotedIdentifier() + wxT(".") + srcschema->GetQuotedIdentifier() + wxT("\n") +
			wxT(" into schema\n") +
			targetschema->GetDatabase()->GetQuotedIdentifier() + wxT(".") + targetschema->GetQuotedIdentifier(),
			_("Paste schema tables"), wxYES_NO) != wxYES)
		{
			return 0;
		}
		sourceconn = srcschema->GetConnection();
	}

	if (!sourceconn || !targetconn)
	{
		wxMessageBox(
			_("Both source and target schema connections should be established before paste table(s) operation !"));
		return 0;
	}

	static wxString copysuffix = wxT("_copy_1");
	wxString tmpcopysuffix;
	if (srcschema->GetId() == targetschema->GetId())
	{
		copysuffix = wxGetTextFromUser(
			_("Target schema is the same as source schema.\n"
			"Enter suffix name for all the new table objects"),
			_("New Suffix Name"),
			copysuffix);
		if (copysuffix.IsEmpty())
		{
			copysuffix = wxT("_copy_1");
			return 0;
		}
		tmpcopysuffix = copysuffix;
	}

	ctlTree *browser = mainform->GetBrowser();
	srcschema->ShowTreeDetail(browser);
	targetschema->ShowTreeDetail(browser);
	tableCopyPasteArray = new pgTableCopyPasteArray();
	pgTableCopyPaste *tablenew;
	pgCollection *tables = browser->FindCollection(tableFactory, srcschema->GetId());
	pgCollection *sequences = browser->FindCollection(sequenceFactory, srcschema->GetId());

	if (table)
	{
		table->ShowTreeDetail(browser);
		pgCollection *constraints = browser->FindCollection(primaryKeyFactory, table->GetId());
		if (constraints)
		{
			constraints->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, constraints);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *indexes = browser->FindCollection(indexFactory, table->GetId());
		if (indexes)
		{
			indexes->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, indexes);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *triggers = browser->FindCollection(triggerFactory, table->GetId());
		if (triggers)
		{
			triggers->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, triggers);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *rules = browser->FindCollection(ruleFactory, table->GetId());
		if (rules)
		{
			rules->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, rules);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *columns = browser->FindCollection(columnFactory, table->GetId());
		if (columns)
			columns->ShowTreeDetail(browser);
		tablenew = new pgTableCopyPaste(table, targetschema, tmpcopysuffix, columns, constraints, tables, sequences,
			indexes, triggers, rules);
		tablenew->srcgroupname = table->GetServer()->GetGroup();
		tablenew->srcservername = table->GetServer()->GetName();
		tablenew->srcdbname = table->GetDatabase()->GetName();
		tablenew->srctablename = table->GetName();
		tablenew->targetgroupname = targetschema->GetServer()->GetGroup();
		tablenew->targetservername = targetschema->GetServer()->GetName();
		tablenew->targetdbname = targetschema->GetDatabase()->GetName();
		tableCopyPasteArray->Add(tablenew);
	}
	else
	{
		if (tables)
		{
			pgObject *obj;
			pgTable *srctable;
			treeObjectIterator colIt(browser, tables);
			while ((obj = colIt.GetNextObject()) != 0)
			{
				srctable = (pgTable *) obj;
				srctable->ShowTreeDetail(browser);
				pgCollection *constraints = browser->FindCollection(primaryKeyFactory, srctable->GetId());
				if (constraints)
				{
					constraints->ShowTreeDetail(browser);
					treeObjectIterator consIt(browser, constraints);
					pgObject *data;
					while ((data = consIt.GetNextObject()) != 0)
					{
						data->ShowTreeDetail(browser);
					}
				}
				pgCollection *indexes = browser->FindCollection(indexFactory, srctable->GetId());
				if (indexes)
				{
					indexes->ShowTreeDetail(browser);
					treeObjectIterator consIt(browser, indexes);
					pgObject *data;
					while ((data = consIt.GetNextObject()) != 0)
					{
						data->ShowTreeDetail(browser);
					}
				}
				pgCollection *triggers = browser->FindCollection(triggerFactory, srctable->GetId());
				if (triggers)
				{
					triggers->ShowTreeDetail(browser);
					treeObjectIterator consIt(browser, triggers);
					pgObject *data;
					while ((data = consIt.GetNextObject()) != 0)
					{
						data->ShowTreeDetail(browser);
					}
				}
				pgCollection *rules = browser->FindCollection(ruleFactory, srctable->GetId());
				if (rules)
				{
					rules->ShowTreeDetail(browser);
					treeObjectIterator consIt(browser, rules);
					pgObject *data;
					while ((data = consIt.GetNextObject()) != 0)
					{
						data->ShowTreeDetail(browser);
					}
				}
				pgCollection *columns = browser->FindCollection(columnFactory, srctable->GetId());
				if (columns)
					columns->ShowTreeDetail(browser);
				tablenew = new pgTableCopyPaste(srctable, targetschema, tmpcopysuffix, columns, constraints, tables,
					sequences, indexes, triggers, rules);
				tablenew->srcgroupname = srctable->GetServer()->GetGroup();
				tablenew->srcservername = srctable->GetServer()->GetName();
				tablenew->srcdbname = srctable->GetDatabase()->GetName();
				tablenew->srctablename = srctable->GetName();
				tablenew->targetgroupname = targetschema->GetServer()->GetGroup();
				tablenew->targetservername = targetschema->GetServer()->GetName();
				tablenew->targetdbname = targetschema->GetDatabase()->GetName();
				tableCopyPasteArray->Add(tablenew);
			}
		}
	}

	transfer = new struct transfer_tag();

	transfer->THIS = this;
	transfer->THIS->thread = NULL;
	transfer->targetdatabase = targetschema->GetDatabase();
	transfer->targetschemaname = targetschema->GetIdentifier();
	transfer->numberOfCopypasteTables = tableCopyPasteArray->Count();
	copied = 0;
	pasteNextTable();
	return tableCopyPasteArray->Count();
}

int pasteTables::processListOfTables()
{
	if (frmCopyTables::selectedTables.IsEmpty() || !targetobj)
	{
		return 0;
	}

	pgSchema *targetschema = (pgSchema *) targetobj;
	pgDatabase *db;
	pgSchema *srcschema;
	pgTable *table;
	pgConn *targetconn = targetobj->GetConnection();

	if (wxMessageBox(
		_("Paste tables from the selection list ?\n into schema\n") +
		targetschema->GetDatabase()->GetQuotedIdentifier() + wxT(".") + targetschema->GetQuotedIdentifier(),
		_("Paste tables from the selection list"), wxYES_NO) != wxYES)
	{
		return 0;
	}

	if (!targetconn)
	{
		wxMessageBox(
			_("Target schema connection should be established before paste table(s) operation !"));
		return 0;
	}

	wxString copysuffix;

	ctlTree *browser = mainform->GetBrowser();
	targetschema->ShowTreeDetail(browser);
	tableCopyPasteArray = new pgTableCopyPasteArray();
	pgTableCopyPaste *tablenew;
	struct copy_object_tag item;

	for (unsigned int i = 0; i < frmCopyTables::selectedTables.Count(); i++)
	{
		item = frmCopyTables::selectedTables.Item(i);
		db = pasteTables::findDB(mainform, item.groupname, item.servername, item.dbname);
		if (!db)
			continue;
		srcschema = pasteTables::findSchema(mainform, db, item.schemaname);
		if (!srcschema)
			continue;
		table = pasteTables::findTable(mainform, srcschema, item.tablename);
		if (!table)
			continue;
		table->ShowTreeDetail(browser);
		pgCollection *constraints = browser->FindCollection(primaryKeyFactory, table->GetId());
		if (constraints)
		{
			constraints->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, constraints);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *indexes = browser->FindCollection(indexFactory, table->GetId());
		if (indexes)
		{
			indexes->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, indexes);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *triggers = browser->FindCollection(triggerFactory, table->GetId());
		if (triggers)
		{
			triggers->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, triggers);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *rules = browser->FindCollection(ruleFactory, table->GetId());
		if (rules)
		{
			rules->ShowTreeDetail(browser);
			treeObjectIterator consIt(browser, rules);
			pgObject *data;
			while ((data = consIt.GetNextObject()) != 0)
			{
				data->ShowTreeDetail(browser);
			}
		}
		pgCollection *tables = browser->FindCollection(tableFactory, table->GetSchema()->GetId());
		pgCollection *sequences = browser->FindCollection(sequenceFactory, table->GetSchema()->GetId());
		pgCollection *columns = browser->FindCollection(columnFactory, table->GetId());
		if (columns)
			columns->ShowTreeDetail(browser);
		tablenew = new pgTableCopyPaste(table, targetschema, copysuffix, columns, constraints, tables, sequences,
			indexes, triggers, rules);
		tablenew->srcgroupname = table->GetServer()->GetGroup();
		tablenew->srcservername = table->GetServer()->GetName();
		tablenew->srcdbname = table->GetDatabase()->GetName();
		tablenew->srctablename = table->GetName();
		tablenew->targetgroupname = targetschema->GetServer()->GetGroup();
		tablenew->targetservername = targetschema->GetServer()->GetName();
		tablenew->targetdbname = targetschema->GetDatabase()->GetName();
		tableCopyPasteArray->Add(tablenew);
	}

	transfer = new struct transfer_tag();

	transfer->THIS = this;
	transfer->THIS->thread = NULL;
	transfer->targetdatabase = targetschema->GetDatabase();
	transfer->targetschemaname = targetschema->GetIdentifier();
	transfer->numberOfCopypasteTables = tableCopyPasteArray->Count();

	copied = 0;
	pasteNextTable();
	return tableCopyPasteArray->Count();
}

bool pasteTables::pasteNextTable()
{
	int i = transfer->numberOfCopypasteTables - 1;
	ctlTree *browser = mainform->GetBrowser();
	bool err = false;
	pgTableCopyPaste &item = tableCopyPasteArray->Item(i);
	transfer->srcdatabase = item.Getsrctable()->GetDatabase();
	transfer->srcschemaname = item.Getsrctable()->GetSchema()->GetIdentifier();

	pgDatabase *dbt = pasteTables::findDB(mainform, item.srcgroupname, item.srcservername, item.srcdbname);
	if (dbt)
	{
		if (dbt->GetConnected())
		{
			pgSchema *schema = pasteTables::findSchema(mainform, dbt, transfer->srcschemaname);
			if (schema)
			{
				pgTable *table = pasteTables::findTable(mainform, schema, item.srctablename);
				if (table == 0)
					err = true;
			}
			else
				err = true;
		}
		else
			err = true;
	}
	else
		err = true;

	if (!err)
	{
		pgDatabase *dbt = pasteTables::findDB(mainform, item.targetgroupname, item.targetservername, item.targetdbname);
		if (dbt)
		{
			if (dbt->GetConnected())
			{
				pgSchema *schema = pasteTables::findSchema(mainform, dbt, transfer->targetschemaname);
				if (!schema)
					err = true;
			}
			else
				err = true;
		}
		else
			err = true;
	}

	if (err)
	{
		lastResultError.formatted_msg = pastemsg + wxT("\n") +
			_("Source/Target schema disappeared\nProbably DB server stopped or schema deleted !");
		wxMessageBox(lastResultError.formatted_msg);
		if (transfer->THIS->thread)
		{
			transfer->THIS->thread->Delete();
			transfer->THIS->thread = NULL;
		}
		while (transfer->THIS->tableCopyPasteArray->Count() > 0)
		{
			transfer->THIS->tableCopyPasteArray->RemoveAt(0);
		}
//		OVO NE RADI !!!
//		transfer->THIS->tableCopyPasteArray->Empty();
		delete transfer->THIS->tableCopyPasteArray;
		transfer->THIS->GetFactory()->GetToolbar()->SetToolShortHelp(
			transfer->THIS->GetFactory()->GetId(), transfer->THIS->lastResultError.formatted_msg);
		delete transfer;
		mainform->SetCopypasteobject(NULL);
		pasteTables::setActive(false);
		return false;
	}

	transfer->table = &item;
	transfer->srcdatabase = item.Getsrctable()->GetDatabase();
	transfer->srcschemaname = item.Getsrctable()->GetSchema()->GetIdentifier();
	transfer->pastesuccess = true;
	transfer->THIS->thread = NULL;
	wxString targetdatabasename = transfer->targetdatabase->GetQuotedIdentifier();
	wxString srcdatabasename = transfer->srcdatabase->GetQuotedIdentifier();

	pgConn *targetconn = targetobj->GetConnection();
	pgConn *newtargetconn;
	pgConn *newsourceconn = item.Getsrctable()->GetConnection()->Duplicate();
	if (newsourceconn->GetDbname() == targetconn->GetDbname() &&
		newsourceconn->GetHost() == targetconn->GetHost() &&
		newsourceconn->GetPort() == targetconn->GetPort())
	{
		//same DB server
		newtargetconn = newsourceconn;
	}
	else
	{
		newtargetconn = targetconn->Duplicate();
	}

	transfer->sourceconn = newsourceconn;
	transfer->targetconn = newtargetconn;

	wxString msg;
	if (copyTableOptionalSelectFactory::available)
		msg = wxString::Format(_(" with condition WHERE %s "), copyTableOptionalSelectFactory::optsel.c_str());
	wxString pastemsg1 = _("Copy table:") +
		srcdatabasename + wxT(".") + qtIdent(transfer->srcschemaname) + wxT(".") +
		item.srctablename + msg +
		_(" INTO:") + targetdatabasename + wxT(".") + qtIdent(transfer->targetschemaname);
	if (copyTableOptionalSelectFactory::available)
		msg = wxString::Format(_("\nwith condition WHERE %s"), copyTableOptionalSelectFactory::optsel.c_str());
	mainform->GetStatusBar()->SetStatusText(pastemsg1, 1);
	pastemsg = _("copy table:\n") +
		srcdatabasename + wxT(".") + qtIdent(transfer->srcschemaname) + wxT(".") +
		item.srctablename + msg +
		_("\nINTO:\n") + targetdatabasename + wxT(".") + qtIdent(transfer->targetschemaname);

	factory->GetToolbar()->SetToolShortHelp(factory->GetId(), pastemsg);

	thread = new copyPasteThread(transfer);
	wxThreadError rc = thread->Create();

	if (rc != wxTHREAD_NO_ERROR)
	{
		lastResultError.formatted_msg.Format(_("Create thread error: rc=%d\n"), rc);
	}
	else
	{
		transfer->srcschema = findSchema(mainform, transfer->srcdatabase, transfer->srcschemaname);
		if (!transfer->srcschema)
		{
			lastResultError.formatted_msg = pastemsg + wxT("\n") + _("Source schema disappeared");
			err = true;
		}
		if (!err)
		{
			transfer->targetschema = findSchema(mainform, transfer->targetdatabase, transfer->targetschemaname);
			if (!transfer->targetschema)
			{
				lastResultError.formatted_msg = pastemsg + wxT("\n") + _("Target schema disappeared");
				err = true;
			}
		}

		if (!err /*&& transfer->srcschema->GetId() != transfer->targetschema->GetId()*/)
		{
			static wxString copysuffix = wxT("_1");
			wxString tmpcopysuffix;
			err = true;
			wxString tablename = item.Getsrctable()->GetName() + item.copysuffix;
			while (err)
			{
				err = tableExists(transfer->targetschema, tablename);
				if (err)
				{
					wxString msg = wxString::Format(
						_("Target schema %s.%s\n"
						"Already contains table %s\n"
						"Enter suffix name for the new table objects"),
						transfer->targetschema->GetDatabase()->GetName().c_str(), transfer->targetschema->GetName().c_str(),
						tablename.c_str());
					copysuffix = wxGetTextFromUser(
						msg,
						_("New Suffix Name"),
						copysuffix);
					if (copysuffix.IsEmpty())
					{
						pastemsg1 += _(" Canceled by user");
						mainform->GetStatusBar()->SetStatusText(pastemsg1, 1);
						copysuffix = wxT("_1");

						//skip this table
						transfer->THIS->thread = NULL;
						transfer->numberOfCopypasteTables--;
						wxCommandEvent event(EVT_THREAD_COPYPASTE_UPDATE_GUI);
						event.SetClientData(transfer);
						wxPostEvent(winMain->GetEventHandler(), event);

						return true;
					}
					tmpcopysuffix = copysuffix;
					tablename = item.Getsrctable()->GetName() + item.copysuffix + copysuffix;
				}
			}
			if (!err)
				item.copysuffix += tmpcopysuffix;
		}
		if (!err)
		{
			bool rc = transfer->targetconn->ExecuteVoid(wxT("BEGIN"));
			if (!rc)
			{
				lastResultError = transfer->targetconn->GetLastResultError();
				lastResultError.formatted_msg = pastemsg + wxT("\n") + lastResultError.formatted_msg;
			}
			else
			{
				transfer->createsql = transfer->table->GetSql(browser);
				transfer->searchPath = transfer->targetconn->ExecuteScalar(wxT("SHOW search_path"));
				transfer->createsql = wxT("SET search_path=") + transfer->targetschema->GetQuotedIdentifier() +
					wxT(";\n") + transfer->createsql;
				thread->Run();
				return true;
			}
		}
	}
	commitChanges();
	return true;
}

void pasteTables::commitChanges()
{
	wxString msg;
	if (lastResultError.formatted_msg != wxT("TABLE EXISTS"))
	{
		if (lastResultError.formatted_msg.IsEmpty())
		{
			if (transfer->targetconn->IsAlive())
			{
				transfer->targetconn->ExecuteVoid(wxT("COMMIT"));
				lastResultError = transfer->targetconn->GetLastResultError();
				if (!lastResultError.formatted_msg.IsEmpty())
				{
					lastResultError.formatted_msg = pastemsg + wxT("\n") + lastResultError.formatted_msg;
					msg = lastResultError.formatted_msg;
				}
			}
			else
				lastResultError.formatted_msg = _("Server closed the connection unexpectedly");
		}
		else
		{
			if (transfer->targetconn->IsAlive())
			{
				transfer->targetconn->ExecuteVoid(wxT("ROLLBACK"));
			}
			msg = lastResultError.formatted_msg;
		}
	}
	if (!lastResultError.formatted_msg.IsEmpty())
	{
		transfer->pastesuccess = false;
		wxMessageBox(msg,
			_("Cannot paste table:") +
			transfer->targetdatabase->GetQuotedIdentifier() + wxT(".") + transfer->targetschemaname +
			wxT(".") + transfer->table->Getsrctable()->GetQuotedIdentifier(),
			wxOK | wxICON_ERROR, winMain);
	}
}

pasteTables::~pasteTables()
{
}

pgTableCopyPaste::pgTableCopyPaste(pgTable *srctable, pgSchema *targetschema, const wxString &copysuffix1,
	pgCollection *columns1, pgCollection *constraints1, pgCollection *tables1, pgCollection *sequences1,
	pgCollection *indexes1, pgCollection *triggers1, pgCollection *rules1)
{
	this->srctable = srctable;
	this->targetschema = targetschema;
	this->copysuffix = copysuffix1;
	this->columns = columns1;
	this->constraints = constraints1;
	this->tables = tables1;
	this->sequences = sequences1;
	this->indexes = indexes1;
	this->triggers = triggers1;
	this->rules = rules1;
}

wxString pgTableCopyPaste::GetSequenceName(const wxString &definition, bool schemaonly)
{
	//Supported formats:
	//1. nextval('"""".id_gk_vrsta_naloga'::regclass)
	//2. nextval(('ps_nalog_id_seq'::text)::regclass)

	//Schema and sequence name combinations:
	//1. "schema.name"."seq.name"
	//2. "schema.name".seq_name
	//3  schema_name."seq.name"
	//4  schema_name.seq_name
	//5  "seq.name"
	//6  seq_name

	wxString defval = definition;
	wxString schemaname, seqname;
	int pos = wxString(wxT("nextval(")).length();
	if (wxString(defval[pos]) == wxT("("))
	{
		pos++; //skip left brace
	}
	pos++; //skip starting apostrophe
	if (wxString(defval[pos]) == wxT("\""))
	{
		//start parsing after the first "
		wxString newdefval = defval.Mid(pos + 1);
		//eliminate Ambiguous ". from the definition field
		newdefval.Replace(wxT("\"\""), wxT("  "), true);

		int subseq = newdefval.Find(wxT("\"."));
		if (subseq == wxNOT_FOUND)
		{
			//case 5 quoted sequence name without schema name
			subseq = newdefval.rfind(wxT("\"'"));
			if (subseq == wxNOT_FOUND)
				return wxEmptyString;
			schemaname = wxT("public");
			seqname = defval.Mid(pos, subseq + 2);
		}
		else
		{
			//quoted schema name
			schemaname = defval.Mid(pos + 1, subseq);
			schemaname.Replace(wxT("\"\""), wxT("\""), true);
			schemaname = wxT("\"") + schemaname + wxT("\"");
			subseq += 3;
			int subseq1 = newdefval.Mid(subseq + 2).rfind(wxT("'"));
			if (subseq1 == wxNOT_FOUND)
				return wxEmptyString;
			//case 1, 2 quoted schema name
			seqname = defval.Mid(pos + subseq, subseq1 + 3);
		}
		if (wxString(seqname[0]) == wxT("\""))
		{
			seqname = seqname.Mid(1, seqname.length() - 2);
			seqname.Replace(wxT("\"\""), wxT("\""), true);
			seqname.Replace(wxT("''"), wxT("'"), true);
			seqname = wxT("\"") + seqname + wxT("\"");
		}
	}
	else
	{
		//case 3, 4, 5, 6 not quoted schema name
		wxString newdefval = defval.Mid(pos);
		int subseq = newdefval.Find(wxT("."));
		if (subseq == wxNOT_FOUND)
		{
			//case 5, 6 sequence name without schema name
			subseq = newdefval.rfind(wxT("'"));
			if (subseq == wxNOT_FOUND)
				return wxEmptyString;
			schemaname = wxT("public");
			seqname = defval.Mid(pos, subseq);
		}
		else
		{
			//case 3, 4 not quoted schema name
			schemaname = defval.Mid(pos, subseq);
			subseq += 1;
			int subseq1 = newdefval.Mid(subseq + 1).rfind(wxT("'"));
			if (subseq1 == wxNOT_FOUND)
				return wxEmptyString;
			seqname = defval.Mid(pos + subseq, subseq1 + 1);
		}
		if (wxString(seqname[0]) == wxT("\""))
		{
			seqname = seqname.Mid(1, seqname.length() - 2);
			seqname.Replace(wxT("\"\""), wxT("\""), true);
			seqname.Replace(wxT("''"), wxT("'"), true);
			seqname = wxT("\"") + seqname + wxT("\"");
		}
	}

	return (schemaonly) ? schemaname : seqname;
}

wxString pgTableCopyPaste::GetSql(ctlTree *browser)
{
	wxString colDetails, conDetails;
	wxString prevComment;
	wxString cols_sql;
	wxString sql;
	wxString sqlcreate;
	wxString sqlsequence = wxT("\n");
	wxString columnPrivileges;
	wxString newtablename;
	wxString qtfullident;
	wxString qtfullidentsrc = srctable->GetQuotedFullIdentifier();
	bool addsuffix = (!copysuffix.IsEmpty());

	if (addsuffix)
	{
		newtablename = srctable->GetIdentifier() + copysuffix;
		if (!tables)
			return sql;
		pgObject *obj;
		pgTable *srctable;
		bool fnd = false;
		treeObjectIterator colIt(browser, tables);
		while ((obj = colIt.GetNextObject()) != 0)
		{
			srctable = (pgTable *) obj;
			if (srctable->GetIdentifier() == newtablename)
			{
				fnd = true;
				break;
			}
		}
		if (!fnd)
		{
			qtfullident = targetschema->GetQuotedPrefix() + qtIdent(newtablename);
		}
	}
	else
	{
		qtfullident = targetschema->GetQuotedPrefix() + srctable->GetQuotedIdentifier();
	}

	sqlcreate = wxT("-- Table: ") + qtfullident + wxT("\n\n")
			+ wxT("-- DROP TABLE ") + qtfullident + wxT(";");
	sql = wxT("\n\nCREATE ");
	if (srctable->GetUnlogged())
		sql += wxT("UNLOGGED ");
	sql += wxT("TABLE ") + qtfullident;

	// of type (9.0 material)
	if (srctable->GetOfTypeOid() > 0)
		sql += wxT("\nOF ") + qtIdent(srctable->GetOfType());

	// Get a count of the constraints.
	int consCount = 0;
	if (constraints)
		consCount = browser->GetChildrenCount(constraints->GetId());

	// Get the columns
	if (columns)
	{
		treeObjectIterator colIt1(browser, columns);
		treeObjectIterator colIt2(browser, columns);

		int lastRealCol = 0;
		int currentCol = 0;
		pgColumn *column;

		// Iterate the columns to find the last 'real' one
		while ((column = (pgColumn *) colIt1.GetNextObject()) != 0)
		{
			currentCol++;

			if (column->GetInheritedCount() == 0)
				lastRealCol = currentCol;
		}

		// Now build the actual column list
		int colCount = 0;
		while ((column = (pgColumn *) colIt2.GetNextObject()) != 0)
		{
			if (column->GetColNumber() > 0)
			{
				if (colCount)
				{
					// Only add a comma if this isn't the last 'real' column, or if there are constraints
					if (colCount != lastRealCol || consCount)
						cols_sql += wxT(",");
					if (!prevComment.IsEmpty())
						cols_sql += wxT(" -- ") + firstLineOnly(prevComment);

					cols_sql += wxT("\n");
				}

				if (column->GetInheritedCount() > 0)
				{
					if (!column->GetIsLocal())
					{
						cols_sql += wxString::Format(wxT("-- %s "), _("Inherited"))
								+ wxT("from table ") + column->GetInheritedTableName() + wxT(":");
					}
				}

				if (srctable->GetOfTypeOid() > 0)
				{
					if (column->GetDefinition().Length() == 0)
					{
						cols_sql += wxString::Format(wxT("-- %s "), _("Inherited")) +
							wxT("from type ") + srctable->GetOfType() + wxT(": ") +
							column->GetQuotedIdentifier();
					}
					else
					{
						cols_sql += wxT("  ") + column->GetQuotedIdentifier() + wxT(" WITH OPTIONS ") +
							column->GetDefinition();
					}
				}
				else
				{
					wxString coldef = column->GetDefault();
					wxString colseq = column->GetSerialSequence();
					if (/*colseq.IsEmpty() && */coldef.StartsWith(wxT("nextval(")))
					{
						wxString newseqname, origseqname;
						bool fnd = false;
						wxString seqname = GetSequenceName(coldef, false);
						if (!seqname.IsEmpty())
						{
							bool addsuffix = (!copysuffix.IsEmpty());
							if (addsuffix)
							{
								if (!sequences)
									return sql;
								pgObject *obj;
								pgSequence *srcsequence = 0, *tmpsequence;
								bool fnd = false;
								newseqname = origseqname = seqname;
								if (seqname.StartsWith(wxT("\"")))
								{
									newseqname = seqname.Mid(1, seqname.length() - 2);
									origseqname = seqname.Mid(1, seqname.length() - 2);
								}
								newseqname += copysuffix;
								treeObjectIterator colIt(browser, sequences);
								while ((obj = colIt.GetNextObject()) != 0)
								{
									tmpsequence = (pgSequence *) obj;
									if (tmpsequence->GetIdentifier() == newseqname)
									{
										fnd = true;
										break;
									}
									if (tmpsequence->GetIdentifier() == origseqname)
									{
										srcsequence = tmpsequence;
									}
								}
								if (fnd)
									return sql;
								pgSequenceCopyPaste seqcp(srcsequence, targetschema, copysuffix);
								sqlsequence += seqcp.GetSql(browser);
								wxString schseqname = GetSequenceName(coldef, true);
								if (schseqname.StartsWith(wxT("\"")))
								{
									schseqname = schseqname.Mid(1, schseqname.length() - 2);
								}
								wxString coldef = column->GetDefinition();
								newseqname = qtIdent(newseqname);
								wxString qtfullidentseq = targetschema->GetQuotedPrefix() + newseqname;
								qtfullidentseq.Replace(wxT("'"), wxT("''"));
								origseqname.Replace(wxT("'"), wxT("''"));
								if (colseq.IsEmpty())
								{
									int noc = coldef.Replace(qtIdent(schseqname) + wxT(".") + qtIdent(origseqname),
										qtfullidentseq, true);
									if (noc == 0 && schseqname == wxT("public"))
									{
										coldef.Replace(qtIdent(origseqname), qtfullidentseq, true);
									}
								}
								else
								{
									wxString x;
									int pos;
									if (coldef.StartsWith(wxT("serial")))
									{
										pos = 7;
										x = wxT("integer");
									}
									else
									{
										pos = 10;
										x = wxT("bigint");
									}
									coldef = x + wxT(" default nextval('") + qtfullidentseq + wxT("'::regclass) ") +
										coldef.Mid(pos);
								}
								cols_sql += wxT("  ") + column->GetQuotedIdentifier() + wxT(" ") + coldef;
							}
							else
							{
								if (!tables)
									return sql;
								pgObject *obj;
								pgSequence *srcsequence;
								wxString seqname1 = seqname;
								if (seqname1.StartsWith(wxT("\"")))
								{
									seqname1 = seqname1.Mid(1, seqname1.length() - 2);
								}
								treeObjectIterator colIt(browser, sequences);
								while ((obj = colIt.GetNextObject()) != 0)
								{
									srcsequence = (pgSequence *) obj;
									if (srcsequence->GetIdentifier() == seqname1)
									{
										fnd = true;
										break;
									}
								}
								if (!fnd)
									return sql;
								pgSequenceCopyPaste seqcp(srcsequence, targetschema, copysuffix);
								sqlsequence += seqcp.GetSql(browser);
								wxString schseqname = GetSequenceName(coldef, true);
								if (schseqname.StartsWith(wxT("\"")))
								{
									schseqname = schseqname.Mid(1, schseqname.length() - 2);
								}
								wxString coldef = column->GetDefinition();
								wxString qtfullidentseq = targetschema->GetQuotedPrefix() + qtIdent(seqname1);
								qtfullidentseq.Replace(wxT("'"), wxT("''"));
								seqname1.Replace(wxT("'"), wxT("''"));
								if (colseq.IsEmpty())
								{
									coldef.Replace(qtIdent(schseqname) + wxT(".") + qtIdent(seqname1), qtfullidentseq, true);
								}
								else
								{
									wxString x;
									int pos;
									if (coldef.StartsWith(wxT("serial")))
									{
										pos = 7;
										x = wxT("integer");
									}
									else
									{
										pos = 10;
										x = wxT("bigint");
									}
									coldef = x + wxT(" default nextval('") + qtfullidentseq + wxT("'::regclass) ") +
										coldef.Mid(pos);
								}
								cols_sql += wxT("  ") + column->GetQuotedIdentifier() + wxT(" ") + coldef;
							}
						}
					}
					else
						cols_sql += wxT("  ") + column->GetQuotedIdentifier() + wxT(" ") + column->GetDefinition();
				}

				prevComment = column->GetComment();

				// Whilst we are looping round the columns, grab their comments as well.
				wxString comment = column->GetCommentSql();
				comment.Replace(qtfullidentsrc, qtfullident, false);
				colDetails += comment;
				if (colDetails.Length() > 0)
					if (colDetails.Last() != '\n')
						colDetails += wxT("\n");
				colDetails += column->GetStorageSql();
				if (colDetails.Length() > 0)
					if (colDetails.Last() != '\n')
						colDetails += wxT("\n");
				colDetails += column->GetAttstattargetSql();
				if (colDetails.Length() > 0)
					if (colDetails.Last() != '\n')
						colDetails += wxT("\n");

				colCount++;
				columnPrivileges += column->GetPrivileges();
			}
		}
	}

	// Now iterate the constraints
	if (constraints)
	{
		wxString constraintname;
		treeObjectIterator consIt(browser, constraints);

		pgObject *data;

		while ((data = consIt.GetNextObject()) != 0)
		{
			cols_sql += wxT(",");

			if (!prevComment.IsEmpty())
				cols_sql += wxT(" -- ") + firstLineOnly(prevComment);

			constraintname = data->GetQuotedIdentifier();
			if (addsuffix)
			{
				if (constraintname.StartsWith(wxT("\"")))
				{
					constraintname = constraintname.Mid(1, constraintname.length() - 2) + copysuffix;
					constraintname = wxT("\"") + constraintname + wxT("\"");
				}
				else
				{
					constraintname += copysuffix;
					constraintname = qtIdent(constraintname);
				}
			}
			cols_sql += wxT("\n  CONSTRAINT ") + constraintname +
				wxT(" ") + data->GetTypeName().Upper() +
				wxT(" ");

			prevComment = data->GetComment();
			if (!data->GetComment().IsEmpty())
				conDetails += wxT("COMMENT ON CONSTRAINT ") + constraintname +
					wxT(" ON ") + srctable->GetQuotedFullIdentifier() +
					wxT(" IS ") + srctable->qtDbString(data->GetComment()) + wxT(";\n");

			switch (data->GetMetaType())
			{
			case PGM_PRIMARYKEY:
			case PGM_UNIQUE:
			case PGM_EXCLUDE:
				cols_sql += ((pgIndexConstraint *) data)->GetDefinition();
				break;
			case PGM_FOREIGNKEY:
				cols_sql += ((pgForeignKey *) data)->GetDefinition();
				break;
			case PGM_CHECK:
				cols_sql += wxT("(") + ((pgCheck *) data)->GetDefinition() + wxT(")");
				break;
			}
		}
	}
	if (!prevComment.IsEmpty())
		cols_sql += wxT(" -- ") + firstLineOnly(prevComment);

	sql += wxT("\n(\n") + cols_sql + wxT("\n)");

	if (srctable->GetInheritedTableCount())
	{
		sql += wxT("\nINHERITS (") + srctable->GetQuotedInheritedTables() + wxT(")");
	}

	if (srctable->GetConnection()->BackendMinimumVersion(8, 2))
	{
		sql += wxT("\nWITH (");
		if (srctable->GetFillFactor().Length() > 0)
			sql += wxT("\n  FILLFACTOR=") + srctable->GetFillFactor() + wxT(", ");
		if (srctable->GetAppendOnly().Length() > 0)
			sql += wxT("APPENDONLY=") + srctable->GetAppendOnly() + wxT(", ");
		if (srctable->GetCompressLevel().Length() > 0)
			sql += wxT("COMPRESSLEVEL=") + srctable->GetCompressLevel() + wxT(", ");
		if (srctable->GetOrientation().Length() > 0)
			sql += wxT("ORIENTATION=") + srctable->GetOrientation() + wxT(", ");
		if (srctable->GetCompressType().Length() > 0)
			sql += wxT("COMPRESSTYPE=") + srctable->GetCompressType() + wxT(", ");
		if (srctable->GetBlocksize().Length() > 0)
			sql += wxT("BLOCKSIZE=") + srctable->GetBlocksize() + wxT(", ");
		if (srctable->GetChecksum().Length() > 0)
			sql += wxT("CHECKSUM=") + srctable->GetChecksum() + wxT(", ");
		if (srctable->GetHasOids())
			sql += wxT("\n  OIDS=TRUE");
		else
			sql += wxT("\n  OIDS=FALSE");
		if (srctable->GetConnection()->BackendMinimumVersion(8, 4))
		{
			if (srctable->GetCustomAutoVacuumEnabled())
			{
				if (srctable->GetAutoVacuumEnabled() == 1)
					sql += wxT(",\n  autovacuum_enabled=true");
				else if (srctable->GetCustomAutoVacuumEnabled() == 0)
					sql += wxT(",\n  autovacuum_enabled=false");
				if (!srctable->GetAutoVacuumVacuumThreshold().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_vacuum_threshold=") + srctable->GetAutoVacuumVacuumThreshold();
				}
				if (!srctable->GetAutoVacuumVacuumScaleFactor().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_vacuum_scale_factor=") + srctable->GetAutoVacuumVacuumScaleFactor();
				}
				if (!srctable->GetAutoVacuumAnalyzeThreshold().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_analyze_threshold=") + srctable->GetAutoVacuumAnalyzeThreshold();
				}
				if (!srctable->GetAutoVacuumAnalyzeScaleFactor().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_analyze_scale_factor=") + srctable->GetAutoVacuumAnalyzeScaleFactor();
				}
				if (!srctable->GetAutoVacuumVacuumCostDelay().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_vacuum_cost_delay=") + srctable->GetAutoVacuumVacuumCostDelay();
				}
				if (!srctable->GetAutoVacuumVacuumCostLimit().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_vacuum_cost_limit=") + srctable->GetAutoVacuumVacuumCostLimit();
				}
				if (!srctable->GetAutoVacuumFreezeMinAge().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_freeze_min_age=") + srctable->GetAutoVacuumFreezeMinAge();
				}
				if (!srctable->GetAutoVacuumFreezeMaxAge().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_freeze_max_age=") + srctable->GetAutoVacuumFreezeMaxAge();
				}
				if (!srctable->GetAutoVacuumFreezeTableAge().IsEmpty())
				{
					sql += wxT(",\n  autovacuum_freeze_table_age=") + srctable->GetAutoVacuumFreezeTableAge();
				}
			}
			if (srctable->GetHasToastTable() && srctable->GetToastCustomAutoVacuumEnabled())
			{
				if (srctable->GetToastAutoVacuumEnabled() == 1)
					sql += wxT(",\n  toast.autovacuum_enabled=true");
				else if (srctable->GetToastAutoVacuumEnabled() == 0)
					sql += wxT(",\n  toast.autovacuum_enabled=false");
				if (!srctable->GetToastAutoVacuumVacuumThreshold().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_vacuum_threshold=") + srctable->GetToastAutoVacuumVacuumThreshold();
				}
				if (!srctable->GetToastAutoVacuumVacuumScaleFactor().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_vacuum_scale_factor=") +
						srctable->GetToastAutoVacuumVacuumScaleFactor();
				}
				if (!srctable->GetToastAutoVacuumVacuumCostDelay().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_vacuum_cost_delay=") + srctable->GetToastAutoVacuumVacuumCostDelay();
				}
				if (!srctable->GetToastAutoVacuumVacuumCostLimit().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_vacuum_cost_limit=") + srctable->GetToastAutoVacuumVacuumCostLimit();
				}
				if (!srctable->GetToastAutoVacuumFreezeMinAge().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_freeze_min_age=") + srctable->GetToastAutoVacuumFreezeMinAge();
				}
				if (!srctable->GetToastAutoVacuumFreezeMaxAge().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_freeze_max_age=") + srctable->GetToastAutoVacuumFreezeMaxAge();
				}
				if (!srctable->GetToastAutoVacuumFreezeTableAge().IsEmpty())
				{
					sql += wxT(",\n  toast.autovacuum_freeze_table_age=") + srctable->GetToastAutoVacuumFreezeTableAge();
				}
			}
		}
		sql += wxT("\n)");
	}
	else
	{
		if (srctable->GetHasOids())
			sql += wxT("\nWITH OIDS");
		else
			sql += wxT("\nWITHOUT OIDS");
	}

	if (srctable->GetConnection()->BackendMinimumVersion(8, 0) &&
			srctable->GetTablespace() != srctable->GetDatabase()->GetDefaultTablespace())
		sql += wxT("\nTABLESPACE ") + qtIdent(srctable->GetTablespace());

	if (srctable->GetConnection()->GetIsGreenplum())
	{
		// Add Greenplum DISTRIBUTED BY
		if (srctable->GetDistributionIsRandom())
		{
			sql += wxT("\nDISTRIBUTED RANDOMLY");
		}
		else if (srctable->GetDistributionColNumbers().Length() == 0)
		{
			// catalog table or other non-distributed table
		}
		else
		{
			// convert list of columns numbers to column names
			wxStringTokenizer collist(srctable->GetDistributionColNumbers(), wxT(","));
			wxString cn;
			wxString distributionColumns;
			while (collist.HasMoreTokens())
			{
				cn = collist.GetNextToken();
				pgSet *set = srctable->ExecuteSet(
					wxT("SELECT attname\n")
					wxT("  FROM pg_attribute\n")
					wxT(" WHERE attrelid=") + srctable->GetOidStr() + wxT(" AND attnum IN (") + cn + wxT(")"));
				if (set)
				{
					if (!distributionColumns.IsNull())
					{
						distributionColumns += wxT(", ");
					}
					distributionColumns += qtIdent(set->GetVal(0));
					delete set;
				}
			}

			sql += wxT("\nDISTRIBUTED BY (");
			sql += distributionColumns;

			sql += wxT(")");
		}

		if (srctable->GetIsPartitioned())
			if (srctable->GetConnection()->BackendMinimumVersion(8, 2, 9) && srctable->GetConnection()->GetIsGreenplum())
				if (srctable->GetPartitionDef().Length() == 0)
				{
					wxString query = wxT("SELECT pg_get_partition_def(");
					query += srctable->GetOidStr();
					query += wxT(", true) ");
					wxString partition_def = srctable->GetDatabase()->ExecuteScalar(query);
					srctable->iSetPartitionDef(partition_def);
					// pg_get_partition_def() doesn't work on partitions
					if (srctable->GetPartitionDef().Length() == 0)
						srctable->iSetPartitionDef(wxT("-- This partition has subpartitions"));
				}
		if (srctable->GetPartitionDef().Length() > 0)
			sql += wxT("\n") + srctable->GetPartitionDef() + wxT("\n");
	}


	sql += wxT(";\n") + srctable->GetOwnerSql(7, 3, wxT("TABLE ") + qtfullident);

	if (srctable->GetConnection()->BackendMinimumVersion(8, 2))
		sql += srctable->GetGrant(wxT("arwdxt"), wxT("TABLE ") + qtfullident);
	else
		sql += srctable->GetGrant(wxT("arwdRxt"), wxT("TABLE ") + qtfullident);

	wxString comment = srctable->GetCommentSql();
	comment.Replace(qtfullidentsrc, qtfullident);
	sql += comment;

	// Column/constraint comments
	if (!colDetails.IsEmpty())
		sql += colDetails + wxT("\n");

	if (!conDetails.IsEmpty())
		sql += conDetails + wxT("\n");

	if (!columnPrivileges.IsEmpty())
	{
		sql += columnPrivileges + wxT("\n");
	}

	wxString tmp;
	if (indexes)
	{
		wxString tmp1;
		tmp1 += wxT("\n");
		treeObjectIterator idxIt(browser, indexes);
		pgIndex *obj;
		while ((obj = (pgIndex *) idxIt.GetNextObject()) != 0)
		{
			pgIndexCopyPaste indexcp(obj, targetschema, copysuffix);
			tmp1 += indexcp.GetSql(browser) + wxT("\n");
		}
		if (tmp1 != wxT("\n"))
			tmp += tmp1;
	}
	if (rules)
	{
		wxString tmp1;
		tmp1 += wxT("\n");
		treeObjectIterator idxIt(browser, rules);
		pgRule *obj;
		while ((obj = (pgRule *) idxIt.GetNextObject()) != 0)
		{
			pgRuleCopyPaste rulecp(obj, targetschema, copysuffix);
			tmp1 += rulecp.GetSql(browser) + wxT("\n");
		}
		if (tmp1 != wxT("\n"))
			tmp += tmp1;
	}
	if (triggers)
	{
		wxString tmp1;
		tmp1 += wxT("\n");
		treeObjectIterator idxIt(browser, triggers);
		pgTrigger *obj;
		while ((obj = (pgTrigger *) idxIt.GetNextObject()) != 0)
		{
			pgTriggerCopyPaste triggercp(obj, targetschema, copysuffix);
			tmp1 += triggercp.GetSql(browser) + wxT("\n");
		}
		if (tmp1 != wxT("\n"))
			tmp += tmp1;
	}

	return sqlcreate + sqlsequence + sql + tmp;
}

pgTableCopyPaste::~pgTableCopyPaste()
{
}

pgSequenceCopyPaste::pgSequenceCopyPaste(pgSequence *srcseq, pgSchema *targetschema, const wxString &copysuffix1)
{
	this->srcseq = srcseq;
	this->targetschema = targetschema;
	this->copysuffix = copysuffix1;
}

wxString pgSequenceCopyPaste::GetSql(ctlTree *browser)
{
	wxString sql = wxEmptyString;
	wxString qtfullident;
	wxString newtablename;
	bool addsuffix = (!copysuffix.IsEmpty());
	if (addsuffix)
	{
		newtablename = srcseq->GetIdentifier() + copysuffix;
		qtfullident = targetschema->GetQuotedPrefix() + qtIdent(newtablename);
	}
	else
		qtfullident = targetschema->GetQuotedPrefix() + srcseq->GetQuotedIdentifier();

	wxString qtfullidentsrc = srcseq->GetQuotedFullIdentifier();

	srcseq->UpdateValues();
	wxLongLong startval = srcseq->GetLastValue();
	if (startval < srcseq->GetMinValue())
		startval = srcseq->GetMinValue();
	sql = wxT("-- Sequence: ") + qtfullident + wxT("\n\n")
			+ wxT("-- DROP SEQUENCE ") + qtfullident + wxT(";")
			+ wxT("\n\nCREATE SEQUENCE ") + qtfullident
			+ wxT("\n  INCREMENT ") + srcseq->GetIncrement().ToString()
			+ wxT("\n  MINVALUE ") + srcseq->GetMinValue().ToString()
			+ wxT("\n  MAXVALUE ") + srcseq->GetMaxValue().ToString()
			+ wxT("\n  START ") + startval.ToString()
			+ wxT("\n  CACHE ") + srcseq->GetCacheValue().ToString();
	if (srcseq->GetCycled())
		sql += wxT("\n  CYCLE");
	sql += wxT(";\n") + srcseq->GetOwnerSql(7, 3, wxT("TABLE ") + qtfullident);

	if (!srcseq->GetConnection()->BackendMinimumVersion(8, 2))
		sql += srcseq->GetGrant(wxT("arwdRxt"), wxT("TABLE ") + qtfullident);
	else
		sql += srcseq->GetGrant(wxT("rwU"), wxT("TABLE ") + qtfullident);

	wxString comment = srcseq->GetCommentSql();
	comment.Replace(qtfullidentsrc, qtfullident);
	sql += comment;

	return sql;
}

pgSequenceCopyPaste::~pgSequenceCopyPaste()
{
}

pgIndexCopyPaste::pgIndexCopyPaste(pgIndex *srcindex, pgSchema *targetschema, const wxString &copysuffix1)
{
	this->srcindex = srcindex;
	this->targetschema = targetschema;
	this->copysuffix = copysuffix1;
}

wxString pgIndexCopyPaste::GetCreate()
{
	wxString str;
	wxString qtname, qtfullname, qtfullident;
	wxString newname = srcindex->GetName();
	wxString newtablename = srcindex->GetIdxTable();
	bool addsuffix = (!copysuffix.IsEmpty());
	if (addsuffix)
	{
		qtfullident = targetschema->GetQuotedPrefix() + qtIdent(newtablename + copysuffix);
	}
	else
	{
		qtfullident = targetschema->GetQuotedPrefix() + qtIdent(newtablename);
	}
	qtname = qtIdent(newname + copysuffix);
	qtfullname = targetschema->GetQuotedPrefix() + qtname;

	str = wxT("-- Index: ") + qtfullname + wxT("\n\n") +
		wxT("-- DROP INDEX ") + qtfullname + wxT(";\n\n");

	str += wxT("CREATE ");
	if (srcindex->GetIsUnique())
		str += wxT("UNIQUE ");
	str += wxT("INDEX ");
	str += qtname +
		wxT("\n  ON ") + qtfullident +
		wxT("\n  USING ") + srcindex->GetIndexType() +
		wxT("\n  (");
	if (srcindex->GetProcName().IsNull())
		str += srcindex->GetQuotedColumns();
	else
	{
		str += srcindex->GetQuotedSchemaPrefix(srcindex->GetProcNamespace()) + qtIdent(srcindex->GetProcName()) + wxT("(") +
			srcindex->GetQuotedColumns() + wxT(")");
		if (!srcindex->GetOperatorClasses().IsNull())
			str += wxT(" ") + srcindex->GetOperatorClasses();
	}

	str += wxT(")");

	if (srcindex->GetConnection()->BackendMinimumVersion(8, 2) && srcindex->GetFillFactor().Length() > 0)
		str += wxT("\n  WITH (FILLFACTOR=") + srcindex->GetFillFactor() + wxT(")");

	if (srcindex->GetConnection()->BackendMinimumVersion(8, 0) &&
			srcindex->GetTablespace() != srcindex->GetDatabase()->GetDefaultTablespace())
		str += wxT("\nTABLESPACE ") + qtIdent(srcindex->GetTablespace());

	AppendIfFilled(str, wxT("\n  WHERE "), srcindex->GetConstraint());

	str += wxT(";\n");

	if (srcindex->GetConnection()->BackendMinimumVersion(7, 5))
		if (srcindex->GetIsClustered())
			str += wxT("ALTER TABLE ") +
				qtfullident +
				wxT(" CLUSTER ON ") + qtIdent(srcindex->GetName())
			+ wxT(";\n");

	wxString comment = srcindex->GetCommentSql();
	comment.Replace(srcindex->GetQuotedFullIdentifier(), qtfullident);
	str += comment;

	return str;
}

wxString pgIndexCopyPaste::GetSql(ctlTree *browser)
{
	return GetCreate();
}

pgIndexCopyPaste::~pgIndexCopyPaste()
{
}

pgRuleCopyPaste::pgRuleCopyPaste(pgRule *srcrule, pgSchema *targetschema, const wxString &copysuffix1)
{
	this->srcrule = srcrule;
	this->targetschema = targetschema;
	this->copysuffix = copysuffix1;
}

wxString pgRuleCopyPaste::GetSql(ctlTree *browser)
{
	int majorVersion = srcrule->GetConnection()->GetMajorVersion();
	int minorVersion = srcrule->GetConnection()->GetMinorVersion();
	int serverversion = majorVersion * 10 + minorVersion;
	wxString definition = srcrule->GetFormattedDefinition();
	wxString newdefinition = definition;
	wxString newname = srcrule->GetName();
	wxString qtname = qtIdent(newname + copysuffix);
	wxString newtablename = srcrule->GetQuotedFullTable();
	int pos = qualifiedIdentifierDotPosCpp(newtablename.mb_str(*srcrule->GetConnection()->GetConv()));
	if (pos != -1)
	{
		newtablename = newtablename.Mid(pos + 1);
	}

	wxString qtfullname, qtfullident;
	bool addsuffix = (!copysuffix.IsEmpty());
	if (addsuffix)
	{
		if (newtablename.StartsWith(wxT("\"")))
		{
			newtablename = newtablename.Mid(1, newtablename.length() - 2) + copysuffix;
			newtablename = wxT("\"") + newtablename + wxT("\"");
			qtfullident = newtablename;
		}
		else
		{
			newtablename = qtIdent(newtablename + copysuffix);
			qtfullident = newtablename;
		}
	}
	else
		qtfullident = newtablename;
	qtfullname = targetschema->GetQuotedPrefix() + qtname;
	qtfullident = targetschema->GetQuotedPrefix() + newtablename;
	struct myScannerNode *head = scanSqlCpp(definition.mb_str(*srcrule->GetConnection()->GetConv()));
	if (head)
	{
		char *rule = parseRuleCpp(
			head, qtname.ToAscii(), targetschema->GetIdentifier().ToAscii(), newtablename.ToAscii());
		if (rule)
		{
			newdefinition = wxString(rule, *targetschema->GetConnection()->GetConv());
			free(rule);
		}
		destroylistCpp(head);
	}

	wxString sql =
		wxT("-- Rule: ") + qtfullname + wxT(" ON ") + qtfullident + wxT("\n\n")
		+ wxT("-- DROP RULE ") + qtfullname + wxT(" ON ") + qtfullident + wxT(";\n\n")
		+ newdefinition
		+ wxT("\n");

	if (!srcrule->GetEnabled())
	{
		sql += wxT("ALTER TABLE ") + qtfullident + wxT(" ")
			+ wxT("DISABLE RULE ") + qtname + wxT(";\n");
	}

	if (!srcrule->GetComment().IsEmpty())
	{
		wxString comment = wxT("COMMENT ON RULE ") + qtname + wxT(" ON ") + qtfullident
			+ wxT(" IS ") + srcrule->qtDbString(srcrule->GetComment()) + wxT(";\n");
		sql += comment;
	}
	return sql;
}

pgRuleCopyPaste::~pgRuleCopyPaste()
{
}

pgTriggerCopyPaste::pgTriggerCopyPaste(pgTrigger *srctrigger, pgSchema *targetschema, const wxString &copysuffix1)
{
	this->srctrigger = srctrigger;
	this->targetschema = targetschema;
	this->copysuffix = copysuffix1;
}

wxString pgTriggerCopyPaste::GetSql(ctlTree *browser)
{
	int majorVersion = srctrigger->GetConnection()->GetMajorVersion();
	int minorVersion = srctrigger->GetConnection()->GetMinorVersion();
	int serverversion = majorVersion * 10 + minorVersion;
	wxString newtablename = srctrigger->GetQuotedFullTable();
	int pos = qualifiedIdentifierDotPosCpp(newtablename.mb_str(*srctrigger->GetConnection()->GetConv()));
	if (pos != -1)
	{
		newtablename = newtablename.Mid(pos + 1);
	}
	wxString qtname, qtfullname, qtfullident;
	wxString newname = srctrigger->GetName();
	bool addsuffix = (!copysuffix.IsEmpty());
	if (addsuffix)
	{
		if (newtablename.StartsWith(wxT("\"")))
		{
			newtablename = newtablename.Mid(1, newtablename.length() - 2) + copysuffix;
			newtablename = wxT("\"") + newtablename + wxT("\"");
			qtfullident = newtablename;
		}
		else
		{
			newtablename = qtIdent(newtablename + copysuffix);
			qtfullident = newtablename;
		}
	}
	else
		qtfullident = newtablename;
	qtname = qtIdent(newname + copysuffix);
	qtfullname = targetschema->GetQuotedPrefix() + qtname;
	qtfullident = targetschema->GetQuotedPrefix() + newtablename;

	wxString sql = wxT("-- Trigger: ") + qtfullname + wxT(" on ") + qtfullident + wxT("\n\n")
		+ wxT("-- DROP TRIGGER ") + qtfullname + wxT(" ON ") + qtfullident + wxT(";\n\n");

	if (srctrigger->GetLanguage() == wxT("edbspl"))
		sql += wxT("CREATE OR REPLACE TRIGGER ");
	else if (targetschema->GetConnection()->BackendMinimumVersion(8, 2) && srctrigger->GetIsConstraint())
		sql += wxT("CREATE CONSTRAINT TRIGGER ");
	else
		sql += wxT("CREATE TRIGGER ");

	sql += qtname + wxT("\n  ")
		+ srctrigger->GetFireWhen()
		+ wxT(" ") + srctrigger->GetEvent();

	sql += wxT("\n  ON ") + qtfullident;
	if (srctrigger->GetDeferrable())
	{
		sql += wxT("\n  DEFERRABLE INITIALLY ");
		if (srctrigger->GetDeferred())
			sql += wxT("DEFERRED");
		else
			sql += wxT("IMMEDIATE");
	}
	sql += wxT("\n  FOR EACH ") + srctrigger->GetForEach();

	if (targetschema->GetConnection()->BackendMinimumVersion(8, 5) && !srctrigger->GetWhen().IsEmpty())
		sql += wxT("\n  WHEN (") + srctrigger->GetWhen() + wxT(")");

	if (srctrigger->GetLanguage() == wxT("edbspl"))
	{
		sql += wxT("\n") + srctrigger->GetSource();
		if (!sql.Trim().EndsWith(wxT(";")))
			sql = sql.Trim() + wxT(";");
		sql += wxT("\n");
	}
	else
	{
		sql += wxT("\n  EXECUTE PROCEDURE ") + srctrigger->GetTriggerFunction()->GetQuotedFullIdentifier()
			+ wxT("(") + srctrigger->GetArguments() + wxT(")")
			+ wxT(";\n");
	}

	if (!srctrigger->GetEnabled())
	{
		sql += wxT("ALTER TABLE ") + qtfullident + wxT(" ")
			+ wxT("DISABLE TRIGGER ") + qtname + wxT(";\n");
	}

	if (!srctrigger->GetComment().IsEmpty())
	{
		wxString comment = wxT("COMMENT ON TRIGGER ") + qtname
			+ wxT(" ON ") + qtfullident
			+ wxT(" IS ") + srctrigger->qtDbString(srctrigger->GetComment()) + wxT(";\n");
		sql += comment;
	}

	return sql;
}

pgTriggerCopyPaste::~pgTriggerCopyPaste()
{
}

copyTableFactory::copyTableFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar) : contextActionFactory(list)
{
	mnu->Append(id, _("&Copy table(s)..."), _("Store table(s) reference for later paste"));
}

wxWindow *copyTableFactory::StartDialog(frmMain *form, pgObject *obj)
{
	if (!pasteTables::isActive())
	{
		copyTableOptionalSelectFactory::available = false;
		while (frmCopyTables::selectedTables.Count() > 0)
		{
			frmCopyTables::selectedTables.RemoveAt(0);
		}
//		OVO NE RADI !!!
//		frmCopyTables::selectedTables.empty();
//
		if (copytObject)
		{
			delete copytObject;
			copytObject = NULL;
		}
		pgObject *copyobj = obj;
		pgSchema *schema = NULL;
		pgTable *table = NULL;
		if (copyobj)
		{
			pgDatabase *db = copyobj->GetDatabase();
			if (db)
			{
				if (db->GetConnected())
				{
					table = dynamic_cast<pgTable *> (copyobj);
					if (table == 0)
					{
						schema = dynamic_cast<pgSchema *> (copyobj);
						if (schema == 0)
						{
							copyobj = NULL;
						}
					}
				}
				else
					copyobj = NULL;
			}
			else
				copyobj = NULL;
		}
		if (copyobj)
		{
			copytObject = new copy_object_tag;
			copytObject->groupname = copyobj->GetServer()->GetGroup();
			copytObject->servername = copyobj->GetServer()->GetName();
			copytObject->dbname = copyobj->GetDatabase()->GetName();
			copytObject->schemaname = (schema) ? schema->GetName() : wxT("");
			if (table)
			{
				copytObject->tablename = table->GetName();
				copytObject->schemaname = table->GetSchema()->GetName();
			}
			else
				copytObject->tablename = wxT("");
			copytObject->object = obj;
			form->GetMenuFactories()->CheckMenu(obj, form->GetMenuBar(), (ctlMenuToolbar *) form->GetToolBar());
		}
	}
	return 0;
}

bool copyTableFactory::CheckEnable(pgObject *obj)
{
	if (!obj || pasteTables::isActive())
		return false;

	pgConn *connection = obj->GetConnection();
	if (connection)
	{
		//tree item 'Tables' has GetMetaType() == PGM_TABLE !
		pgTable *table = (obj->GetMetaType() == PGM_TABLE) ? dynamic_cast<pgTable *> (obj) : 0;
		if (table)
			return true;
		//tree item 'Schemas' has GetMetaType() == PGM_SCHEMA !
		pgSchema *schema = (obj->GetMetaType() == PGM_SCHEMA) ? dynamic_cast<pgSchema *> (obj) : 0;
		if (schema)
			return true;
	}
	return false;
}

copyTableOptionalSelectFactory::copyTableOptionalSelectFactory(
	menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar) : contextActionFactory(list)
{
	mnu->Append(id,
			_("&Copy table with optional select"),
			_("Store table reference for later paste with optional select"));
}

wxWindow *copyTableOptionalSelectFactory::StartDialog(frmMain *form, pgObject *obj)
{
	if (!pasteTables::isActive())
	{
		available = false;
		while (frmCopyTables::selectedTables.Count() > 0)
		{
			frmCopyTables::selectedTables.RemoveAt(0);
		}
//		OVO NE RADI !!!
//		frmCopyTables::selectedTables.empty();
//
		if (copyTableFactory::copytObject)
		{
			delete copyTableFactory::copytObject;
			copyTableFactory::copytObject = NULL;
		}
		pgObject *copyobj = obj;
		pgTable *table;
		if (copyobj)
		{
			pgDatabase *db = copyobj->GetDatabase();
			if (db)
			{
				if (db->GetConnected())
				{
					table = dynamic_cast<pgTable *> (copyobj);
					if (table == 0)
						copyobj = NULL;
				}
				else
					copyobj = NULL;
			}
			else
				copyobj = NULL;
		}
		if (copyobj)
		{
			wxString msg = wxString::Format(
					_("Enter SELECT WHERE clause(empty string means SELECT without WHERE clause)\n"
					"Generated SELECT will be: SELECT * FROM %s.%s WHERE ..."),
					table->GetSchema()->GetQuotedIdentifier().c_str(), table->GetQuotedIdentifier().c_str());
			optsel = wxGetTextFromUser(msg, _("SELECT WHERE clause"), optsel);
			if (!optsel.IsEmpty())
				available = true;
			copyTableFactory::copytObject = new copy_object_tag;
			copyTableFactory::copytObject->groupname = copyobj->GetServer()->GetGroup();
			copyTableFactory::copytObject->servername = copyobj->GetServer()->GetName();
			copyTableFactory::copytObject->dbname = copyobj->GetDatabase()->GetName();
			copyTableFactory::copytObject->tablename = table->GetName();
			copyTableFactory::copytObject->schemaname = table->GetSchema()->GetName();
			copyTableFactory::copytObject->object = obj;
			form->GetMenuFactories()->CheckMenu(obj, form->GetMenuBar(), (ctlMenuToolbar *) form->GetToolBar());
		}
	}
	return 0;
}

bool copyTableOptionalSelectFactory::CheckEnable(pgObject *obj)
{
	if (!obj || pasteTables::isActive())
		return false;

	pgConn *connection = obj->GetConnection();
	if (connection)
	{
		//tree item 'Tables' has GetMetaType() == PGM_TABLE !
		pgTable *table = (obj->GetMetaType() == PGM_TABLE) ? dynamic_cast<pgTable *> (obj) : 0;
		if (table)
			return true;
	}
	return false;
}

pasteTablesFactory::pasteTablesFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar) :
	contextActionFactory(list)
{
	mnu->Append(id, _("&Paste table(s)..."), _("Paste table(s)"));
	this->toolbar = toolbar;
	toolbar->AddTool(id, _("Paste table(s)"), *pastetables_png_bmp, _("Paste table(s) into schema."), wxITEM_NORMAL);
}

wxWindow *pasteTablesFactory::StartDialog(frmMain *form, pgObject *obj)
{
	struct copy_object_tag *copyobj = copyTableFactory::copytObject;
	pgDatabase *targetdatabase = ((pgSchema *) obj)->GetDatabase();
	if (copyobj)
	{
		pgDatabase *db = pasteTables::findDB(form, copyobj->groupname, copyobj->servername, copyobj->dbname);
		if (db)
		{
			if (db->GetConnected())
			{
				pgSchema *schema = pasteTables::findSchema(form, db, copyobj->schemaname);
				if (schema && !copyobj->tablename.IsEmpty())
				{
					pgTable *table = pasteTables::findTable(form, schema, copyobj->tablename);
					if (table == 0)
					{
						copyobj = NULL;
					}
				}
			}
			else
				copyobj = NULL;
		}
		else
			copyobj = NULL;
	}
	if ((copyobj || !frmCopyTables::selectedTables.IsEmpty()) && !pasteTables::isActive())
	{
		pasteTables::setActive(true);
		pasteTables *copypasteobject = new pasteTables(winMain, copyobj, obj, this);
		int numtables;
		if (copyobj)
			numtables = copypasteobject->process();
		else
			numtables = copypasteobject->processListOfTables();
		if (numtables == 0)
		{
			delete copypasteobject;
			winMain->SetCopypasteobject(NULL);
			pasteTables::setActive(false);
		}
		else
		{
			form->SetCopypasteobject(copypasteobject);
		}
		form->GetMenuFactories()->CheckMenu((pgSchema *) obj, form->GetMenuBar(), (ctlMenuToolbar *) form->GetToolBar());
	}
	return 0;
}

bool pasteTablesFactory::CheckEnable(pgObject *obj)
{
	if (!obj)
		return false;

	if (obj->GetConnection() && (copyTableFactory::copytObject) || !frmCopyTables::selectedTables.IsEmpty())
	{
		//tree item 'Schemas' has GetMetaType() == PGM_SCHEMA !
		pgSchema *schema = (obj->GetMetaType() == PGM_SCHEMA) ? (pgSchema *) obj : 0;
//		pgSchema *schema = (obj->GetMetaType() == PGM_SCHEMA) ? dynamic_cast<pgSchema *> (obj) : 0;
		if (schema)
			return true;
	}
	return false;
}

copyTablesListFactory::copyTablesListFactory(menuFactoryList *list, wxMenu *mnu, ctlMenuToolbar *toolbar) :
	contextActionFactory(list)
{
	mnu->Append(id, _("&Copy tables from the list..."), _("Store tables reference for later paste"));
}

wxWindow *copyTablesListFactory::StartDialog(frmMain *form, pgObject *obj)
{
	if (!pasteTables::isActive())
	{
		if (copyTableFactory::copytObject)
		{
			delete copyTableFactory::copytObject;
			copyTableFactory::copytObject = NULL;
		}
		while (frmCopyTables::selectedTables.Count() > 0)
		{
			frmCopyTables::selectedTables.RemoveAt(0);
		}
//		OVO NE RADI !!!
//		frmCopyTables::selectedTables.empty();
//
		frmCopyTables *frm = new frmCopyTables(form, obj);
		form->GetMenuFactories()->CheckMenu(obj, form->GetMenuBar(), (ctlMenuToolbar *) form->GetToolBar());
	}
	return 0;
}

bool copyTablesListFactory::CheckEnable(pgObject *obj)
{
	if (pasteTables::isActive())
		return false;
	if (!obj)
		return false;
	return obj->CanBackup();
}

#define ctvTablesCopy CTRL_CHECKTREEVIEW("ctvTablesCopy")

BEGIN_EVENT_TABLE(frmCopyTables, ExecutionDialog)
EVT_BUTTON(wxID_OK, frmCopyTables::OnOK)
EVT_CLOSE(ExecutionDialog::OnClose)
END_EVENT_TABLE()

frmCopyTables::frmCopyTables(frmMain *form, pgObject *_object) : ExecutionDialog(form, _object)
{
	object = _object;
	LoadResource(form, wxT("frmCopyTables"));
	SetFont(settings->GetSystemFont());
	ctvTablesCopy->SetFont(settings->GetSystemFont());
	ctvTablesCopy->SetCopytables(this);
	RestorePosition();
	SetIcon(*pastetables_png_ico);

	ctlTree *browser = form->GetBrowser();
	if (browser->ItemHasChildren(browser->GetRootItem()))
	{
		wxTreeItemId rootnode, groupnode, servernode, dbnode;
		rootnode = ctvTablesCopy->AddRoot(_("Server Groups"), 2);
		wxTreeItemIdValue groupcookie;
		wxTreeItemId groupitem = browser->GetFirstChild(browser->GetRootItem(), groupcookie);
		while (groupitem)
		{
			groupnode = ctvTablesCopy->AppendItem(rootnode, browser->GetItemText(groupitem), 2);
			wxCookieType cookie;
			wxTreeItemId serverItem = browser->GetFirstChild(groupitem, cookie);
			while (serverItem)
			{
				pgServer *server = (pgServer *) browser->GetObject(serverItem);
				if (server && server->IsCreatedBy(serverFactory))
				{
					hashgroupname[server->GetFullName()] = server->GetGroup();
					hashservername[server->GetFullName()] = server->GetDescription();
					hashservernamename[server->GetFullName()] = server->GetName();
					servernode = ctvTablesCopy->AppendItem(groupnode, server->GetFullName(), 2);
					if (server->connection() && server->connection()->IsAlive())
					{
						addDatabases(server, browser, servernode);
					}
				}
				serverItem = browser->GetNextChild(groupitem, cookie);
			}
			groupitem = browser->GetNextChild(browser->GetRootItem(), groupcookie);
		}
		ctvTablesCopy->Expand(rootnode);
	}

	Show(true);
}

void frmCopyTables::OnOK(wxCommandEvent &ev)
{
	SavePosition();
	wxTreeItemId groupnode;
	wxTreeItemIdValue groupcookie;
	wxTreeItemIdValue childData;
	wxTreeItemId rootnode = ctvTablesCopy->GetRootItem();
	wxTreeItemId childserver;
	struct copy_object_tag *selection;
	wxString groupname, servername, dbname, schemaname, tablename;
	groupnode = ctvTablesCopy->GetFirstChild(rootnode, groupcookie);
	while (groupnode.IsOk())
	{
		groupname = ctvTablesCopy->GetItemText(groupnode);
		int subseq = groupname.rfind(wxT("("));
		if (subseq != wxNOT_FOUND)
			groupname = groupname.Mid(0, subseq);
		childserver = ctvTablesCopy->GetFirstChild(groupnode, childData);
		while (childserver.IsOk())
		{
			servername = ctvTablesCopy->GetItemText(childserver);
			wxTreeItemIdValue childData1;
			wxTreeItemId childdb = ctvTablesCopy->GetFirstChild(childserver, childData1);
			while (childdb.IsOk())
			{
				dbname = ctvTablesCopy->GetItemText(childdb);
				wxTreeItemIdValue childData2;
				wxTreeItemId childschema = ctvTablesCopy->GetFirstChild(childdb, childData2);
				while (childschema.IsOk())
				{
					schemaname = ctvTablesCopy->GetItemText(childschema);
					wxTreeItemIdValue childData3;
					wxTreeItemId childtable = ctvTablesCopy->GetFirstChild(childschema, childData3);
					while (childtable.IsOk())
					{
						if (ctvTablesCopy->IsChecked(childtable))
						{
							tablename = ctvTablesCopy->GetItemText(childtable);
							selection = new struct copy_object_tag;
							selection->groupname = hashgroupname[servername];
							selection->servername = hashservernamename[servername];
							selection->dbname = dbname;
							selection->schemaname = schemaname;
							selection->tablename = tablename;
							selectedTables.Add(selection);
						}
						childtable = ctvTablesCopy->GetNextChild(childschema, childData3);
					}
					childschema = ctvTablesCopy->GetNextChild(childdb, childData2);
				}
				childdb = ctvTablesCopy->GetNextChild(childserver, childData1);
			}
			childserver = ctvTablesCopy->GetNextChild(groupnode, childData);
		}
		groupnode = ctvTablesCopy->GetNextChild(rootnode, groupcookie);
	}
	Destroy();
	mainForm->GetMenuFactories()->CheckMenu(object, mainForm->GetMenuBar(), (ctlMenuToolbar *) mainForm->GetToolBar());
}

wxString frmCopyTables::GetHelpPage() const
{
	return wxEmptyString;
}

wxString frmCopyTables::GetSql()
{
	return wxEmptyString;
}

frmCopyTables::~frmCopyTables()
{
	SavePosition();
	ctvTablesCopy->SetCopytables(NULL);
}

bool frmCopyTables::treeDetails(const wxString &fullservername, const wxString &dbname, const wxString &schemaname)
{
	wxString groupname = hashgroupname[fullservername];
	wxString servername = hashservername[fullservername];
	wxString servernamename = hashservernamename[fullservername];
	pgDatabase *db;
	pgSchema *srcschema;
	pgTable *table;
	wxTreeItemId childserver, childdb, childschema;

	pgServer *server = pasteTables::findServer(mainForm, groupname, servername);
	if (!server)
		return false;
	if (!server->connection())
	{
		server = mainForm->ConnectToServer(servername, true);
		if (!server)
			return false;
	}
	if (dbname.IsEmpty())
	{
		wxTreeItemIdValue groupcookie;
		wxTreeItemId groupitem;
		wxTreeItemIdValue childData1;
		groupitem = ctvTablesCopy->GetFirstChild(ctvTablesCopy->GetRootItem(), groupcookie);
		while (groupitem)
		{
			wxTreeItemId servernode = ctvTablesCopy->GetFirstChild(groupitem, childData1);
			wxString n1 = ctvTablesCopy->GetItemText(servernode);
			while (servernode)
			{
				if (ctvTablesCopy->GetItemText(servernode) == server->GetFullName() && groupname == server->GetGroup())
				{
					ctvTablesCopy->DeleteChildren(servernode);
					addDatabases(server, mainForm->GetBrowser(), servernode);
					return true;
				}
				servernode = ctvTablesCopy->GetNextChild(groupitem, childData1);
			}
			groupitem = ctvTablesCopy->GetNextChild(ctvTablesCopy->GetRootItem(), groupcookie);
		}
		return false;
	}

	db = pasteTables::findDB(mainForm, groupname, servernamename, dbname);
	if (db)
	{
		pgConn *conn = db->GetConnection();
		if (!conn ||
			(conn && (!(!conn->IsAlive() && (conn->GetStatus() == PGCONN_BROKEN || conn->GetStatus() == PGCONN_BAD)))))
		{
			db->ShowTreeDetail(mainForm->GetBrowser());
			wxTreeItemIdValue groupcookie, childData;
			wxTreeItemId rootnode = ctvTablesCopy->GetRootItem();
			wxTreeItemId groupitem = ctvTablesCopy->GetFirstChild(rootnode, groupcookie);
			while (groupitem)
			{
				childserver = ctvTablesCopy->GetFirstChild(groupitem, childData);
				while (childserver)
				{
					wxString txt = ctvTablesCopy->GetItemText(childserver);
					if (txt == fullservername)
					{
						wxTreeItemIdValue childData1;
						childdb = ctvTablesCopy->GetFirstChild(childserver, childData1);
						while (childdb)
						{
							txt = ctvTablesCopy->GetItemText(childdb);
							if (txt == db->GetName())
							{
								if (schemaname.IsEmpty())
								{
									ctvTablesCopy->DeleteChildren(childdb);
									addSchemaTables(db, mainForm->GetBrowser(), childdb);
									return true;
								}
								else
								{
									wxTreeItemIdValue childData2;
									childschema = ctvTablesCopy->GetFirstChild(childdb, childData2);
									while (childschema)
									{
										txt = ctvTablesCopy->GetItemText(childschema);
										if (txt == schemaname)
										{
											ctvTablesCopy->DeleteChildren(childschema);
											pgSchema *srcschema = pasteTables::findSchema(mainForm, db, schemaname);
											if (srcschema)
											{
												srcschema->ShowTreeDetail(mainForm->GetBrowser());
												addTables(srcschema, mainForm->GetBrowser(), childschema);
												return true;
											}
										}
										childschema = ctvTablesCopy->GetNextChild(childdb, childData2);
									}
								}
							}
							childdb = ctvTablesCopy->GetNextChild(childserver, childData1);
						}
					}
					childserver = ctvTablesCopy->GetNextChild(groupitem, childData);
				}
				groupitem = ctvTablesCopy->GetNextChild(rootnode, groupcookie);
			}
			return true;
		}
	}

	return false;
}

void frmCopyTables::addDatabases(pgServer *server, ctlTree *browser, wxTreeItemId &servernode)
{
	pgCollection *databases = browser->FindCollection(databaseFactory, server->GetId());
	if (databases)
	{
		pgObject *obj;
		pgDatabase *srcdb;
		treeObjectIterator dbIt(browser, databases);
		while ((obj = dbIt.GetNextObject()) != 0)
		{
			srcdb = (pgDatabase *) obj;
			wxTreeItemId dbnode = ctvTablesCopy->AppendItem(servernode, srcdb->GetName(), 0);
			ctvTablesCopy->SetItemTextColour(dbnode, wxColour(102, 0, 51));
			addSchemaTables(srcdb, browser, dbnode);
		}
	}
}

void frmCopyTables::addSchemaTables(pgDatabase *db, ctlTree *browser, wxTreeItemId &dbnode)
{
	pgCollection *schemas = browser->FindCollection(schemaFactory, db->GetId());
	if (schemas)
	{
		pgObject *obj;
		pgSchema *srcschema;
		treeObjectIterator schIt(browser, schemas);
		while ((obj = schIt.GetNextObject()) != 0)
		{
			srcschema = (pgSchema *) obj;
			wxTreeItemId schemanode = ctvTablesCopy->AppendItem(dbnode, srcschema->GetName(), 0);
			ctvTablesCopy->SetItemTextColour(schemanode, wxColour(0, 0, 0xFF));
			addTables(srcschema, browser, schemanode);
		}
	}
}

void frmCopyTables::addTables(pgSchema *srcschema, ctlTree *browser, wxTreeItemId &schemanode)
{
	pgCollection *tables = browser->FindCollection(tableFactory, srcschema->GetId());
	if (tables)
	{
		pgObject *obj;
		pgTable *srctable;
		treeObjectIterator tabIt(browser, tables);
		while ((obj = tabIt.GetNextObject()) != 0)
		{
			srctable = (pgTable *) obj;
			wxTreeItemId tablenode = ctvTablesCopy->AppendItem(schemanode, srctable->GetName(), 0);
			ctvTablesCopy->SetItemTextColour(tablenode, wxColour(26, 94, 31));
		}
	}
}
