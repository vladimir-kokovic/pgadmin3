//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// pgDatabase.cpp - PostgreSQL Database
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>

// App headers
#include "pgAdmin3.h"
#include "misc.h"
#include "pgDatabase.h"
#include "pgObject.h"
#include "pgServer.h"
#include "pgCollection.h"


pgDatabase::pgDatabase(const wxString& newName)
: pgServerObject(PG_DATABASE, newName)
{
    wxLogInfo(wxT("Creating a pgDatabase object"));

    allowConnections = TRUE;
    connected = FALSE;
	conn = NULL;
}


pgDatabase::~pgDatabase()
{
    wxLogInfo(wxT("Destroying a pgDatabase object"));
	if (conn)
		delete conn;
}


wxMenu *pgDatabase::GetNewMenu()
{
    wxMenu *menu=pgObject::GetNewMenu();
    AppendMenu(menu, PG_CAST);
    AppendMenu(menu, PG_LANGUAGE);
    AppendMenu(menu, PG_SCHEMA);
    return menu;
}
int pgDatabase::Connect()
{
    if (!allowConnections)
        return PGCONN_REFUSED;

    if (connected)
        return conn->GetStatus();
    else
    {
		conn = new pgConn(this->GetServer()->GetName(), this->GetName(), this->GetServer()->GetUsername(), this->GetServer()->GetPassword(), this->GetServer()->GetPort());       
		if (conn->GetStatus() == PGCONN_OK)
        {
            // Now we're connected.
            connected = TRUE;
            pgSet *set=conn->ExecuteSet(wxT(
                "SELECT DEFS.*, description\n"
                "  FROM (SELECT \n"
                "  (SELECT proname FROM pg_proc WHERE proname IN ('pg_get_viewdef', 'pg_get_viewdef2')"
                      " AND pronamespace=11 ORDER BY proname DESC LIMIT 1) AS get_viewdef,\n"
                "  (SELECT proname FROM pg_proc WHERE proname IN ('pg_get_ruledef', 'pg_get_ruledef2')"
                      " AND pronamespace=11 ORDER BY proname DESC LIMIT 1) AS get_ruledef,\n"
                "  (SELECT proname FROM pg_proc WHERE proname IN ('pg_get_expr', 'pg_get_expr2')"
                      " AND pronamespace=11 ORDER BY proname DESC LIMIT 1) AS get_expr,\n"
                " 'nix' as get_ruledef, 'expr' as get_expr\n"
                "       ) AS DEFS\n"
                "  LEFT OUTER JOIN pg_description ON objoid=") + GetOidStr());
            if (set)
            {
                viewdefFunction = set->GetVal(wxT("get_viewdef"));
                ruledefFunction = set->GetVal(wxT("get_ruledef"));
                exprFunction = set->GetVal(wxT("get_expr"));
                iSetComment(set->GetVal(wxT("description")));
                delete set;
            }
            return PGCONN_OK;
        }
        else
        {
			wxLogError(wxT("%s"), conn->GetLastError().c_str());
            return PGCONN_BAD;
        }
    }
}

bool pgDatabase::GetSystemObject() const
{
    if (server) {
        if (this->GetName() == wxT("template0")) return TRUE;
        return (this->GetOid() <= server->GetLastSystemOID());
    } else {
        return FALSE;
    }
}


bool pgDatabase::DropObject(wxFrame *frame, wxTreeCtrl *browser)
{
    if (conn)
    {
        delete conn;
        conn=0;
    }
    bool done=server->ExecuteVoid(wxT("DROP DATABASE ") + GetQuotedIdentifier());
    if (!done)
        Connect();

    return done;
}



wxString pgDatabase::GetSql(wxTreeCtrl *browser)
{
    if (sql.IsEmpty())
    {
        sql = wxT("-- Database: ") + GetQuotedFullIdentifier() + wxT("\n")
            + wxT("CREATE DATABASE ") + GetQuotedIdentifier()
            + wxT("\n  WITH ENCODING = ") + qtString(GetEncoding()) + wxT(";\n");
        wxStringTokenizer vars(GetVariables());
        while (vars.HasMoreTokens())
            sql += wxT("ALTER DATABASE ") + GetQuotedFullIdentifier()
                +  wxT(" SET ") + vars.GetNextToken() + wxT(";\n");
        sql += GetGrant(wxT("CT"))
            +  GetCommentSql();
    }
    return sql;
}



void pgDatabase::ShowTreeDetail(wxTreeCtrl *browser, frmMain *form, wxListCtrl *properties, wxListCtrl *statistics, ctlSQLBox *sqlPane)
{
    if (Connect() == PGCONN_OK)
    {
        // Set the icon if required
        if (browser->GetItemImage(GetId(), wxTreeItemIcon_Normal) != 2)
        {
            browser->SetItemImage(GetId(), PGICON_DATABASE, wxTreeItemIcon_Normal);
		    browser->SetItemImage(GetId(), PGICON_DATABASE, wxTreeItemIcon_Selected);
        }

            // Add child nodes if necessary
        if (browser->GetChildrenCount(GetId(), FALSE) == 0)
        {
            wxLogInfo(wxT("Adding child object to database ") + GetIdentifier());
            pgCollection *collection;

            // Casts
            collection = new pgCollection(PG_CASTS, this);
            AppendBrowserItem(browser, collection);

            // Languages
            collection = new pgCollection(PG_LANGUAGES, this);
            AppendBrowserItem(browser, collection);

            // Schemas
            collection = new pgCollection(PG_SCHEMAS, this);
            AppendBrowserItem(browser, collection);
        }
    }

    GetServer()->iSetLastDatabase(GetName());

    if (properties)
    {
        // Setup listview
        CreateListColumns(properties);
        int pos=0;

        InsertListItem(properties, pos++, wxT("Name"), GetName());
        InsertListItem(properties, pos++, wxT("OID"), NumToStr(GetOid()));
        InsertListItem(properties, pos++, wxT("Owner"), GetOwner());
        InsertListItem(properties, pos++, wxT("ACL"), GetAcl());
        if (!GetPath().IsEmpty())
            InsertListItem(properties, pos++, wxT("Path"), GetPath());
        InsertListItem(properties, pos++, wxT("Encoding"), GetEncoding());
        wxStringTokenizer vars(GetVariables());
        while (vars.HasMoreTokens())
        {
            wxString str=vars.GetNextToken();
            InsertListItem(properties, pos++, str.BeforeFirst('='), str.AfterFirst('='));
        }
        InsertListItem(properties, pos++, wxT("Allow Connections?"), GetAllowConnections());
        InsertListItem(properties, pos++, wxT("Connected?"), GetConnected());
        InsertListItem(properties, pos++, wxT("System Database?"), GetSystemObject());
        InsertListItem(properties, pos++, wxT("Comment"), GetComment());
    }
}



pgObject *pgDatabase::Refresh(wxTreeCtrl *browser, const wxTreeItemId item)
{
    pgDatabase *database=0;
    wxTreeItemId parentItem=browser->GetItemParent(item);
    if (parentItem)
    {
        pgObject *obj=(pgObject*)browser->GetItemData(parentItem);
        if (obj->GetType() == PG_DATABASES)
        {
            database = (pgDatabase*)ReadObjects((pgCollection*)obj, 0, wxT(" WHERE db.oid=") + GetOidStr() + wxT("\n"));
            if (database)
            {
                sql=wxT("");
                iSetAcl(database->GetAcl());
                iSetVariables(database->GetVariables());
                iSetComment(conn->ExecuteScalar(wxT("SELECT description FROM pg_description WHERE objoid=") + GetOidStr()));
                delete database;
            }
        }
    }
    return this;
}


pgObject *pgDatabase::ReadObjects(pgCollection *collection, wxTreeCtrl *browser, const wxString &restriction)
{
    pgDatabase *database=0;

    pgSet *databases = collection->GetServer()->ExecuteSet(wxT(
       "SELECT db.oid, datname, datpath, datallowconn, datconfig, datacl, "
              "pg_encoding_to_char(encoding) AS serverencoding, pg_get_userbyid(datdba) AS datowner\n"
       "  FROM pg_database db\n"
       + restriction +
       " ORDER BY datname"));
    
    if (databases)
    {
        while (!databases->Eof())
        {
            database = new pgDatabase(databases->GetVal(wxT("datname")));
            database->iSetServer(collection->GetServer());
            database->iSetOid(databases->GetOid(wxT("oid")));
            database->iSetOwner(databases->GetVal(wxT("datowner")));
            database->iSetAcl(databases->GetVal(wxT("datacl")));
            database->iSetPath(databases->GetVal(wxT("datpath")));
            database->iSetEncoding(databases->GetVal(wxT("serverencoding")));
            wxString str=databases->GetVal(wxT("datconfig"));
            if (!str.IsEmpty())
                database->iSetVariables(str.Mid(1, str.Length()-2));
            database->iSetAllowConnections(databases->GetBool(wxT("datallowconn")));

            // Add the treeview node if required
            if (settings->GetShowSystemObjects() ||!database->GetSystemObject()) 
            {
                if (browser)
                    browser->AppendItem(collection->GetId(), database->GetIdentifier(), PGICON_CLOSEDDATABASE, -1, database);   
                else
                    break;
            }
            else 
				delete database;
	
			databases->MoveNext();
        }
		delete databases;
    }
    return database;
}


void pgDatabase::ShowStatistics(pgCollection *collection, wxListCtrl *statistics)
{
    wxLogInfo(wxT("Displaying statistics for databases on ") + collection->GetServer()->GetIdentifier());

    // Add the statistics view columns
    statistics->ClearAll();
    statistics->InsertColumn(0, wxT("Database"), wxLIST_FORMAT_LEFT, 100);
    statistics->InsertColumn(1, wxT("Backends"), wxLIST_FORMAT_LEFT, 75);
    statistics->InsertColumn(2, wxT("Xact Committed"), wxLIST_FORMAT_LEFT, 100);
    statistics->InsertColumn(3, wxT("Xact Rolled Back"), wxLIST_FORMAT_LEFT, 100);
    statistics->InsertColumn(4, wxT("Blocks Read"), wxLIST_FORMAT_LEFT, 100);
    statistics->InsertColumn(5, wxT("Blocks Hit"), wxLIST_FORMAT_LEFT, 100);

    pgSet *stats = collection->GetServer()->ExecuteSet(wxT("SELECT datname, numbackends, xact_commit, xact_rollback, blks_read, blks_hit FROM pg_stat_database ORDER BY datname"));
    if (stats)
    {
        while (!stats->Eof())
        {
            statistics->InsertItem(stats->CurrentPos() - 1, stats->GetVal(wxT("datname")), PGICON_STATISTICS);
            statistics->SetItem(stats->CurrentPos() - 1, 1, stats->GetVal(wxT("numbackends")));
            statistics->SetItem(stats->CurrentPos() - 1, 2, stats->GetVal(wxT("xact_commit")));
            statistics->SetItem(stats->CurrentPos() - 1, 3, stats->GetVal(wxT("xact_rollback")));
            statistics->SetItem(stats->CurrentPos() - 1, 4, stats->GetVal(wxT("blks_read")));
            statistics->SetItem(stats->CurrentPos() - 1, 5, stats->GetVal(wxT("blks_hit")));
            stats->MoveNext();
        }

	    delete stats;
    }
}

