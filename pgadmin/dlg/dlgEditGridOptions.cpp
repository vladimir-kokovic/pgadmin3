//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2016, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgEditGridOptions.cpp - Edit Grid Box Options
//
//////////////////////////////////////////////////////////////////////////

// wxWindows headers
#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/settings.h>

#include <wx/generic/gridctrl.h>
#if wxCHECK_VERSION(2, 9, 0)
#include <wx/string.h>
#include <wx/menu.h>
#include <wx/menuitem.h>
#else
#include <wx-2.8/wx/string.h>
#include <wx-2.8/wx/menu.h>
#include <wx-2.8/wx/menuitem.h>
#endif

// App headers
#include "pgAdmin3.h"
#include "utils/pgDefs.h"
#include "frm/frmMain.h"

#include "frm/frmEditGrid.h"
#include "dlg/dlgEditGridOptions.h"
#include "dlg/dlgEditGridOptionsFilter.h"
#include "schema/pgTable.h"
#include "schema/pgView.h"

// Icons
#include "images/sortfilter.pngc"

#define XML_FROM_WXSTRING(s) ((const xmlChar *)(const char *)s.mb_str(wxConvUTF8))
#define WXSTRING_FROM_XML(s) wxString((char *)s, wxConvUTF8)
#define XML_STR(s) ((const xmlChar *)s)

#define nbOptions                   CTRL_NOTEBOOK("nbOptions")
#define btnAsc                      CTRL_BUTTON("btnAsc")
#define btnDesc                     CTRL_BUTTON("btnDesc")
#define btnRemove                   CTRL_BUTTON("wxID_REMOVE")
#define btnValidate                 CTRL_BUTTON("btnValidate")
#define cboColumns                  CTRL_COMBOBOX("cboColumns")
#define lstSortCols                 CTRL_LISTVIEW("lstSortCols")
#define pnlSort                     CTRL_PANEL("pnlSort")
#define pnlFilter                   CTRL_PANEL("pnlFilter")
#define filter                      CTRL_SQLBOX("sqlFilter")
#define btnFind						CTRL_BUTTON("btnFind")

BEGIN_EVENT_TABLE(dlgEditGridOptions, pgDialog)
	EVT_CLOSE(                                      dlgEditGridOptions::OnClose)
	EVT_BUTTON               (wxID_OK,              dlgEditGridOptions::OnOK)
	EVT_BUTTON               (wxID_CANCEL,          dlgEditGridOptions::OnCancel)
	EVT_BUTTON               (wxID_REMOVE,          dlgEditGridOptions::OnRemove)
	EVT_BUTTON               (XRCID("btnAsc"),      dlgEditGridOptions::OnAsc)
	EVT_BUTTON               (XRCID("btnDesc"),     dlgEditGridOptions::OnDesc)
	EVT_BUTTON               (XRCID("btnValidate"), dlgEditGridOptions::OnValidate)
	EVT_COMBOBOX             (XRCID("cboColumns"),  dlgEditGridOptions::OnCboColumnsChange)
	EVT_LIST_ITEM_SELECTED   (XRCID("lstSortCols"), dlgEditGridOptions::OnLstSortColsChange)
	EVT_LIST_ITEM_DESELECTED (XRCID("lstSortCols"), dlgEditGridOptions::OnLstSortColsChange)
	EVT_STC_MODIFIED		 (XRCID("sqlFilter"),   dlgEditGridOptions::OnFilterChange)
	EVT_BUTTON				 (XRCID("btnFind"),		dlgEditGridOptions::OnSearch)
#ifdef __WXMAC__
	EVT_SIZE(                                       dlgEditGridOptions::OnChangeSize)
#endif
END_EVENT_TABLE()

//static void outWinDef(wxWindow *w, wxString& result, wxString& spc)
//{
//	wxWindowList& children = w->GetChildren();
//	for (wxWindowList::iterator i = children.begin(); i != children.end(); ++i)
//	{
//		wxWindow *w1 = (*i);
//		int idint = w1->GetId();
//		wxString idstr;
//		idstr = idstr << idint;
//		wxString name = w1->GetName();
//		wxString classname = w1->GetClassInfo()->GetClassName();
//		result += spc + wxT("name=") + name + wxT(" classname=") + classname + wxT(" id=") + idstr + wxT("\n");
//		wxWindowList& children1 = w1->GetChildren();
//		if (!children1.IsEmpty())
//		{
//			wxString spc1 = spc + wxT("  ");
//			outWinDef(w1, result, spc1);
//		}
//	}
//}

dlgEditGridOptions::dlgEditGridOptions(frmEditGrid *win, pgConn *conn, const wxString &rel, ctlSQLEditGrid *grid)
	: winOptions(0)
{
	editGrid = grid;
	connection = conn;
	relation = rel;
	parent = win;
	LoadResource(win, wxT("dlgEditGridOptions"));
	SetFont(settings->GetSystemFont());
	conv = conn->GetConv();

	// Icon
	SetIcon(*sortfilter_png_ico);
	RestorePosition();

	// Accelerator table
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_ALT, (int)'F', XRCID("btnFind"));
	wxAcceleratorTable accel(WXSIZEOF(entries), entries);
	SetAcceleratorTable(accel);

	int cols = grid->GetNumberCols();
	long x;

	for (x = 0; x < cols; x++)
		cboColumns->Append(grid->GetColLabelValue(x).BeforeFirst('\n'));

	// Setup the buttons
	wxCommandEvent nullEvent;
	OnCboColumnsChange(nullEvent);
	wxListEvent nullLstEvent;
	OnLstSortColsChange(nullLstEvent);

	// Setup the list box
	int leftSize = 140, rightSize;
	leftSize = ConvertDialogToPixels(wxPoint(leftSize, 0)).x;
	rightSize = lstSortCols->GetClientSize().GetWidth() - leftSize;
	// This check is to work around a bug in wxGTK that doesn't set
	// appropriately the GetClientSize().
	// Without this workaround, we have an invisible second column.
	if (rightSize < leftSize)
		rightSize = leftSize + 1;
	lstSortCols->InsertColumn(0, _("Column name"), wxLIST_FORMAT_LEFT, leftSize);
	lstSortCols->InsertColumn(1, _("Sort order"), wxLIST_FORMAT_LEFT, rightSize);

	histoFiltersFileName = settings->GetHistoryFile() + wxT("_filters");
	LoadFilters();
	wxString filt = parent->GetFilter().Trim();
	if (filt.length() < 1)
	{
		if (!histoFilters.IsEmpty())
		{
			filt = histoFilters.Item(0).Mid(20);
		}
	}

	// Setup the filter SQL box. This is an XRC 'unknown' control so must
	// be manually created and attache to the XRC global resource.
	filter->SetText(filt);

	// Get the current sort columns, and populate the listbox.
	// The current columns will be parsed char by char to allow us
	// to cope with quoted column names with commas in them (let's hope
	// no one ever does that, but sod's law etc....)
	bool inColumn = true, inQuote = false;
	wxString sortCols = parent->GetSortCols();
	wxString col, dir;
	size_t pos, len = sortCols.Length();
	int itm = 0;

	for (pos = 0; pos < len; pos++)
	{
		if (inColumn)
		{
			if (sortCols.GetChar(pos) == '"') inQuote = !inQuote;
			if (!inQuote && (sortCols.GetChar(pos) == ' ' || sortCols.GetChar(pos) == ','))
				inColumn = false;
			else if (sortCols.GetChar(pos) != '"') col += sortCols.GetChar(pos);
		}
		else
		{
			if (sortCols.GetChar(pos - 1) == ',')
			{
				inColumn = true;
				lstSortCols->InsertItem(itm, col);
				if (dir.GetChar(0) == 'D')
				{
					lstSortCols->SetItem(itm, 1, _("Descending"));
					lstSortCols->SetItemData(itm, 0);
				}
				else
				{
					lstSortCols->SetItem(itm, 1, _("Ascending"));
					lstSortCols->SetItemData(itm, 1);
				}
				col = wxT("");
				dir = wxT("");
				++itm;
			}
			else
			{
				dir += sortCols.GetChar(pos);
			}
		}
	}

	// Insert the last column
	if (col.Length() > 0)
	{
		lstSortCols->InsertItem(itm, col);
		if (dir.GetChar(0) == 'D')
		{
			lstSortCols->SetItem(itm, 1, _("Descending"));
			lstSortCols->SetItemData(itm, 0);
		}
		else
		{
			lstSortCols->SetItem(itm, 1, _("Ascending"));
			lstSortCols->SetItemData(itm, 1);
		}
	}

	// Finally (phew!) remove all columns we're already sorting on from the list.
	long count = lstSortCols->GetItemCount();

	for (x = 0; x < count; x++)
	{
		int idx = cboColumns->FindString(lstSortCols->GetItemText(x));
		if (idx >= 0)
			cboColumns->Delete(idx);
	}

	// Display the appropriate tab. If the EditGrid is not shown, we must be
	// doing a View Filtered Data.
	if (!parent->IsShown())
		nbOptions->DeletePage(0);

//	if (true) {
//		wxString result;
//		wxString spc = wxT("");
//		outWinDef(this, result, spc);
//		wxPrintf(wxT("%s"), result.c_str());
//	}

//	enum
//	{
//		 ID_SEARCHMENU = wxID_HIGHEST,
//	};
//	wxMenu *menu = new wxMenu;
//	wxMenuItem* menuItem = menu->Append(wxID_ANY, _T("Recent Searches"), wxT(""), wxITEM_NORMAL);
//    menuItem->Enable(false);
//	for (int i = 0; i < histoFilters.GetCount(); i++)
//	{
//		menu->Append(ID_SEARCHMENU + i, histoFilters.Item(i));
//	}
//	Connect(ID_SEARCHMENU, ID_SEARCHMENU + histoFilters.GetCount(), wxEVT_COMMAND_MENU_SELECTED,
//			wxCommandEventHandler(dlgEditGridOptions::OnMenuSelect));
//	searchText->SetMenu(menu);

	int cnt = nbOptions->GetPageCount();
	nbOptions->SetSelection(cnt - 1);
	btnValidate->Disable();
	filter->SetFocus();
}

dlgEditGridOptions::~dlgEditGridOptions()
{
	SavePosition();
	if (winOptions)
	{
		delete winOptions;
		winOptions = 0;
	}
}

// Enable/disable the validation button
void dlgEditGridOptions::OnFilterChange(wxStyledTextEvent &ev)
{
	btnValidate->Enable(!filter->GetText().Trim().IsEmpty());
}

void dlgEditGridOptions::OnRemove(wxCommandEvent &ev)
{
	long itm = -1;
	itm = lstSortCols->GetNextItem(itm, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	cboColumns->Append(lstSortCols->GetItemText(itm));
	lstSortCols->DeleteItem(itm);
	if (lstSortCols->GetItemCount() > 0)
	{
		if (lstSortCols->GetItemCount() < itm + 1)
			lstSortCols->SetItemState(lstSortCols->GetItemCount() - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		else
			lstSortCols->SetItemState(itm, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
	wxListEvent nullLstEvent;
	OnLstSortColsChange(nullLstEvent);
}


void dlgEditGridOptions::OnAsc(wxCommandEvent &ev)
{
	long itm = lstSortCols->GetItemCount();
	lstSortCols->InsertItem(itm, cboColumns->GetValue());
	lstSortCols->SetItem(itm, 1, _("Ascending"));
	lstSortCols->SetItemData(itm, 1);
	cboColumns->Delete(cboColumns->GetCurrentSelection());

	// Setup the buttons
	OnCboColumnsChange(ev);
	wxListEvent nullLstEvent;
	OnLstSortColsChange(nullLstEvent);
}

void dlgEditGridOptions::OnDesc(wxCommandEvent &ev)
{
	long itm = lstSortCols->GetItemCount();
	lstSortCols->InsertItem(itm, cboColumns->GetValue());
	lstSortCols->SetItem(itm, 1, _("Descending"));
	lstSortCols->SetItemData(itm, 0);
	cboColumns->Delete(cboColumns->GetCurrentSelection());

	// Setup the buttons
	OnCboColumnsChange(ev);
	wxListEvent nullLstEvent;
	OnLstSortColsChange(nullLstEvent);
}

#ifdef __WXMAC__
void dlgEditGridOptions::OnChangeSize(wxSizeEvent &ev)
{
	if (lstSortCols)
		lstSortCols->SetSize(wxDefaultCoord, wxDefaultCoord,
		                     ev.GetSize().GetWidth(), ev.GetSize().GetHeight() - 350);
	if (GetAutoLayout())
	{
		Layout();
	}
}
#endif

void dlgEditGridOptions::OnValidate(wxCommandEvent &ev)
{
	if (Validate())
		wxMessageBox(_("Filter string syntax validates OK!"), _("Syntax Validation"), wxICON_INFORMATION | wxOK);
}

void dlgEditGridOptions::OnCboColumnsChange(wxCommandEvent &ev)
{
	// Set the command buttons appropriately
	if (cboColumns->GetCurrentSelection() == wxNOT_FOUND)
	{
		btnAsc->Enable(false);
		btnDesc->Enable(false);
	}
	else
	{
		btnAsc->Enable(true);
		btnDesc->Enable(true);
	}
}

void dlgEditGridOptions::OnLstSortColsChange(wxListEvent &ev)
{
	// Set the command buttons appropriately
	if (lstSortCols->GetSelectedItemCount() == 0)
		btnRemove->Enable(false);
	else
		btnRemove->Enable(true);
}

void dlgEditGridOptions::OnCancel(wxCommandEvent &ev)
{
	EndModal(false);
}


void dlgEditGridOptions::OnClose(wxCloseEvent &ev)
{
	EndModal(false);
}

void dlgEditGridOptions::OnOK(wxCommandEvent &ev)
{
#ifdef __WXGTK__
	if (!btnOK->IsEnabled())
		return;
#endif
	// Check the filter syntax
	if (!Validate()) return;

	if (nbOptions->GetPageCount() > 1)
	{
		wxString sortCols;
		long x, count = lstSortCols->GetItemCount();

		for (x = 0; x < count; x++)
		{
			sortCols += qtIdent(lstSortCols->GetItemText(x));
			if (lstSortCols->GetItemData(x) == 0)
				sortCols += wxT(" DESC");
			else
				sortCols += wxT(" ASC");
			sortCols += wxT(", ");
		}

		if (sortCols.Length() > 2)
		{
			sortCols.RemoveLast();
			sortCols.RemoveLast();
		}

		parent->SetSortCols(sortCols);
	}

	wxString filt = filter->GetText().Trim();
	SaveFiltersOK(filt);
	parent->SetFilter(filt);
	SavePosition();

	EndModal(true);
}

void dlgEditGridOptions::SaveFiltersOK(wxString &filt)
{
	bool found;
	wxString strtoday;
	wxDateTime today = wxDateTime::Now();
	strtoday << today.FormatISODate() << wxT("/") << today.FormatISOTime() << wxT("/");
	wxString str = filt;
	if (str[0] < '0' || str[0] > '9')
	{
		str = strtoday + str;
	}
	for (int index = 0; index < histoFilters.GetCount(); index++)
	{
		found = histoFilters.Item(index).Mid(20) == str.Mid(20);
		if (found)
		{
			histoFilters.RemoveAt(index);
			break;
		}
	}
	histoFilters.Insert(str, 0);
	SaveFilters();

	filter->SetText(str.Mid(20));
}

bool dlgEditGridOptions::Validate()
{
	winMain->StartMsg(_("Validating filter string"));
	filter->MarkerDeleteAll(0);
	if (!filter->GetText().Trim().Length())
	{
		winMain->EndMsg();
		return true;
	}

	wxString sql = wxT("EXPLAIN SELECT * FROM ") + relation + wxT(" WHERE ");
	int queryOffset = sql.Length();
	sql += filter->GetText();

	PGresult *qryRes;
	qryRes = connection->vkPQexec(connection->connection(), sql.mb_str(*conv));
	int res = PQresultStatus(qryRes);

	// Check for errors
	if (res == PGRES_TUPLES_OK ||
	        res == PGRES_COMMAND_OK)
	{
		// No errors, all OK!
		winMain->EndMsg();
		return true;
	}

	// Figure out where the error is
	wxString errMsg = connection->GetLastError();

	wxString atChar = wxT(" at character ");
	int chp = errMsg.Find(atChar);

	if (chp > 0)
	{
		int selStart = filter->GetSelectionStart(), selEnd = filter->GetSelectionEnd();
		if (selStart == selEnd)
			selStart = 0;

		long errPos = 0;
		errMsg.Mid(chp + atChar.Length()).ToLong(&errPos);
		errPos -= queryOffset;  // do not count EXPLAIN or similar
		wxLogError(wxT("%s"), _("ERROR: Syntax error at character %d!"), errPos);

		int line = 0, maxLine = filter->GetLineCount();
		while (line < maxLine && filter->GetLineEndPosition(line) < errPos + selStart + 1)
			line++;
		if (line < maxLine)
		{
			filter->MarkerAdd(line, 0);
			filter->EnsureVisible(line);
		}
	}
	else
		wxLogError(wxT("%s"), errMsg.BeforeFirst('\n').c_str());

	// Cleanup
	PQclear(qryRes);
	winMain->EndMsg();
	return false;
}

void dlgEditGridOptions::LoadFilters()
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *key;
	int cnt = 0;

	if (!wxFile::Access(histoFiltersFileName, wxFile::read))
		return;

	doc = xmlParseFile((const char *)histoFiltersFileName.mb_str(wxConvUTF8));
	if (doc == NULL)
	{
		wxMessageBox(_("Failed to load the filters history file!"));
		::wxRemoveFile(settings->GetHistoryFile());
		return;
	}

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL)
	{
		xmlFreeDoc(doc);
		return;
	}

	if (xmlStrcmp(cur->name, (const xmlChar *) "histofilters"))
	{
		wxMessageBox(_("Failed to load the filters history file!"));
		xmlFreeDoc(doc);
		::wxRemoveFile(histoFiltersFileName);
		return;
	}

	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"histofilter")))
		{
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			if (key)
			{
				if (WXSTRING_FROM_XML(key) != wxT(""))
				{
					wxString filt = WXSTRING_FROM_XML(key);
					histoFilters.Add(filt);
				}
				xmlFree(key);
			}
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
}


void dlgEditGridOptions::SaveFilters()
{
	size_t i;
	xmlTextWriterPtr writer;

	writer = xmlNewTextWriterFilename((const char *)histoFiltersFileName.mb_str(wxConvUTF8), 0);
	if (!writer)
	{
		wxMessageBox(_("Failed to write to filters history file!"));
		return;
	}
	xmlTextWriterSetIndent(writer, 1);

	if ((xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL) < 0) ||
	        (xmlTextWriterStartElement(writer, XML_STR("histofilters")) < 0))
	{
		wxMessageBox(_("Failed to write to filters history file!"));
		xmlFreeTextWriter(writer);
		return;
	}

	wxDateTime today = wxDateTime::Now();
	wxString strtoday;
	strtoday << today.FormatISODate() << wxT("/") << today.FormatISOTime() << wxT("/");
	for (i = 0; i < histoFilters.GetCount(); i++)
	{
		wxString str = histoFilters.Item(i);
		if (str[0] < '0' || str[0] > '9')
		{
			str = strtoday + str;
		}
		xmlTextWriterStartElement(writer, XML_STR("histofilter"));
		xmlTextWriterWriteString(writer, XML_FROM_WXSTRING(str));
		xmlTextWriterEndElement(writer);
	}

	if (xmlTextWriterEndDocument(writer) < 0)
	{
		wxMessageBox(_("Failed to write to filters history file!"));
	}

	xmlFreeTextWriter(writer);
}

wxString dlgEditGridOptions::getHistoFiltersFileName()
{
	return histoFiltersFileName;
}

wxString dlgEditGridOptions::FindItem(const wxString &str, long itemno) {
	wxString str1 = str.Lower();
	unsigned int index = 0;
	long cnt = 0;
	bool found = false;
	while (index < histoFilters.GetCount())
	{
		wxString str2 = histoFilters.Item(index).Lower();
		found = str2.Contains(str1);
		if (found)
		{
			if (cnt == itemno)
			{
				return histoFilters.Item(index);
			}
			cnt++;
		}
		index++;
	}
	return wxEmptyString;
}

void dlgEditGridOptions::OnSearch(wxCommandEvent &ev)
{
	if (winOptions)
		delete winOptions;
	winOptions = new dlgEditGridOptionsFilter(parent, this);
	winOptions->ShowModal();
}
