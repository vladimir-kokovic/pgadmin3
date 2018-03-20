//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2015, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgQueryHistory.cpp - frmQuery queries
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

#include "frm/frmQuery.h"
#include "dlg/dlgQueryHistory.h"

static dlgQueryHistory *THIS;

#define MY_CTRL_TEXT(id) (XRCCTRL(*THIS, id, wxTextCtrl))
#define searchTxt		MY_CTRL_TEXT("searchTxt")
#define lstFilters		CTRL_LISTVIEWVIRTUALHISTORY("lstFilters")
#define btnFind			CTRL_BUTTON("wxID_FIND")
#define filter          CTRL_SQLBOX("sqlFilter")

#define XML_FROM_WXSTRING(s) ((const xmlChar *)(const char *)s.mb_str(wxConvUTF8))
#define WXSTRING_FROM_XML(s) wxString((char *)s, wxConvUTF8)
#define XML_STR(s) ((const xmlChar *)s)

BEGIN_EVENT_TABLE(dlgQueryHistory, pgDialog)
    EVT_LIST_ITEM_SELECTED      (XRCID("lstFilters"), ctlListViewVirtualHistory::OnListItemSelection)
	EVT_CLOSE(										            dlgQueryHistory::OnClose)
	EVT_BUTTON					(wxID_OK,			            dlgQueryHistory::OnOK)
	EVT_BUTTON					(wxID_CANCEL,		            dlgQueryHistory::OnCancel)
	EVT_TEXT					(XRCID("searchTxt"),            dlgQueryHistory::OnText)
#ifdef __WXMAC__
	EVT_SIZE(											dlgQueryHistory::OnChangeSize)
#endif
END_EVENT_TABLE()

dlgQueryHistory::dlgQueryHistory(frmQuery *win)
{
	THIS = this;
	parentdlg = win;

	LoadResource(win, wxT("dlgQueryHistory"));
	SetFont(settings->GetSystemFont());

	// Setup the list box
	lstfilters = lstFilters;
	lstfilters->SetItemCount(THIS->parentdlg->GetHistoQueries()->GetCount());
	lstfilters->SetParent((void *)parentdlg);

	RestorePosition();
	searchTxt->SetFocus();
}

dlgQueryHistory::~dlgQueryHistory()
{
	SavePosition();
}

#ifdef __WXMAC__
void dlgQueryHistory::OnChangeSize(wxSizeEvent &ev)
{
	if (lstfilters)
		lstfilters->SetSize(wxDefaultCoord, wxDefaultCoord,
		                     ev.GetSize().GetWidth(), ev.GetSize().GetHeight() - 350);
	if (GetAutoLayout())
	{
		Layout();
	}
}
#endif

void dlgQueryHistory::SavePosition()
{
	pgDialog::SavePosition();
	settings->WriteInt(dlgName + wxT("/ListLeftSize"), lstfilters->GetColumnWidth(0));
}

void dlgQueryHistory::RestorePosition(int defaultX, int defaultY, int defaultW, int defaultH, int minW, int minH)
{
	pgDialog::RestorePosition(defaultX, defaultY, defaultW, defaultH, minW, minH);
	int leftSize = 140, rightSize;
	settings->Read(dlgName + wxT("/ListLeftSize"), &leftSize, 140);
	lstfilters->InsertColumn(0, _("Queries"), wxLIST_FORMAT_LEFT, leftSize);
}

void dlgQueryHistory::OnCancel(wxCommandEvent &ev)
{
	SavePosition();
	EndModal(false);
}


void dlgQueryHistory::OnClose(wxCloseEvent &ev)
{
	SavePosition();
	EndModal(false);
}

void dlgQueryHistory::OnOK(wxCommandEvent &ev)
{
#ifdef __WXGTK__
	if (!btnOK->IsEnabled())
		return;
#endif

	wxString filt;
	long sel = lstfilters->GetSelection();
	if (sel >= 0)
	{
		filt = FindItem(searchTxt->GetValue(), sel);
	}
	ctlSQLBox *sqlQuery = parentdlg->GetQueryText();
	sqlQuery->SetText(filt);
	sqlQuery->Colourise(0, filt.Length());
	sqlQuery->EmptyUndoBuffer();
	wxSafeYield(); // needed to process sqlQuery modify event

	SavePosition();
	EndModal(true);
}

void dlgQueryHistory::OnText(wxCommandEvent &event)
{
	lstfilters->Refresh();
}

wxString ctlListViewVirtualHistory::OnGetItemText(long itemx, long column) const
{
	int cnt = THIS->parentdlg->GetHistoQueries()->GetCount();
	if (cnt < 1)
	{
		return wxEmptyString;
	}
	long rev = cnt - itemx - 1;
	info->m_itemId = itemx;
	wxString str;
	if (searchTxt->IsEmpty())
	{
		str = THIS->parentdlg->GetHistoQueries()->Item(rev);
		info->m_text = str;
		return str;
	}
	str = THIS->FindItem(searchTxt->GetValue(), itemx);
	info->m_text = str;
    return str;
}

int ctlListViewVirtualHistory::OnGetItemColumnImage(long item, long column) const
{
    return -1;
}

wxListItemAttr *ctlListViewVirtualHistory::OnGetItemAttr(long item) const {
	if (item % 2)
	{

		if (info && info->m_itemId == item && !info->m_text.IsEmpty());
		{
			wxString str = info->m_text;
			if (!searchTxt->IsEmpty())
			{
				str = THIS->FindItem(searchTxt->GetValue(), item);
			}
			if (!str.IsEmpty())
			{
				wxColour cbg(0xC0, 0xFF, 0xC0);
				static wxListItemAttr s_attr(wxNullColour, cbg, wxNullFont);
				return &s_attr;
			}
		}
	}
	return NULL;
}

wxString dlgQueryHistory::FindItem(const wxString &str, long itemno) {
	wxString str1 = str.Lower();
	long index = parentdlg->GetHistoQueries()->GetCount() - 1;
	long cnt = 0;
	bool found = false;
	while (index >= 0)
	{
		wxString str2 = THIS->parentdlg->GetHistoQueries()->Item(index).Lower();
		found = str2.Contains(str1);
		if (found)
		{
			if (cnt == itemno)
			{
				return THIS->parentdlg->GetHistoQueries()->Item(index);
			}
			cnt++;
		}
		index--;
	}
	return wxEmptyString;
}

void ctlListViewVirtualHistory::OnListItemSelection(wxListEvent& event)
{
	long item = event.GetIndex();
	if (item > -1)
	{
		ctlListViewVirtualHistory *lstfilters = lstFilters;
		wxString toolTip = lstFilters->GetItemText(item);
		if (lstfilters->parentfrm)
		{
			frmQuery *frmptr = (frmQuery *)lstfilters->parentfrm;
			ctlSQLBox *sqlQuery = frmptr->GetQueryText();
			sqlQuery->SetText(toolTip);
			sqlQuery->Colourise(0, toolTip.Length());
			sqlQuery->EmptyUndoBuffer();
			wxSafeYield(); // needed to process sqlQuery modify event
		}
	}
}
