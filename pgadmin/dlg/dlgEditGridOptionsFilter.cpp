//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgEditGridOptionsFilter.cpp - Edit Grid Box Options
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

static dlgEditGridOptionsFilter *THIS;

#define MY_CTRL_TEXT(id) (XRCCTRL(*THIS, id, wxTextCtrl))
#define searchTxt		MY_CTRL_TEXT("searchTxt")
#define lstFilters		CTRL_LISTVIEWVIRTUAL("lstFilters")
#define btnFind			CTRL_BUTTON("wxID_FIND")

BEGIN_EVENT_TABLE(dlgEditGridOptionsFilter, pgDialog)
	EVT_CLOSE(											dlgEditGridOptionsFilter::OnClose)
	EVT_BUTTON					(wxID_OK,				dlgEditGridOptionsFilter::OnOK)
	EVT_BUTTON					(wxID_CANCEL,			dlgEditGridOptionsFilter::OnCancel)
	EVT_TEXT					(XRCID("searchTxt"),	dlgEditGridOptionsFilter::OnText)
#ifdef __WXMAC__
	EVT_SIZE(											dlgEditGridOptionsFilter::OnChangeSize)
#endif
END_EVENT_TABLE()

dlgEditGridOptionsFilter::dlgEditGridOptionsFilter(frmEditGrid *win, dlgEditGridOptions *dlgOptions)
{
	THIS = this;
	this->dlgOptions = dlgOptions;

	LoadResource(win, wxT("dlgEditGridOptionsFilter"));
	SetFont(settings->GetSystemFont());

	RestorePosition();

	// Setup the list box
	lstFilters->SetItemCount(dlgOptions->histoFilters.GetCount());

	searchTxt->SetFocus();
}

dlgEditGridOptionsFilter::~dlgEditGridOptionsFilter()
{
	SavePosition();
}

#ifdef __WXMAC__
void dlgEditGridOptionsFilter::OnChangeSize(wxSizeEvent &ev)
{
	if (lstFilters)
		lstFilters->SetSize(wxDefaultCoord, wxDefaultCoord,
		                     ev.GetSize().GetWidth(), ev.GetSize().GetHeight() - 350);
	if (GetAutoLayout())
	{
		Layout();
	}
}
#endif

void dlgEditGridOptionsFilter::SavePosition()
{
	pgDialog::SavePosition();
	settings->WriteInt(dlgName + wxT("/ListLeftSize"), lstFilters->GetColumnWidth(0));
}

void dlgEditGridOptionsFilter::RestorePosition(int defaultX, int defaultY, int defaultW, int defaultH, int minW, int minH)
{
	pgDialog::RestorePosition(defaultX, defaultY, defaultW, defaultH, minW, minH);
	int leftSize = 140, rightSize;
	settings->Read(dlgName + wxT("/ListLeftSize"), &leftSize, 140);
	rightSize = GetSize().GetWidth() - leftSize - 5;
	if (rightSize < leftSize)
		rightSize = leftSize + 1;
	lstFilters->InsertColumn(0, _("Timestamp"), wxLIST_FORMAT_LEFT, leftSize);
	lstFilters->InsertColumn(1, _("Filters"), wxLIST_FORMAT_LEFT, rightSize);
}

void dlgEditGridOptionsFilter::OnCancel(wxCommandEvent &ev)
{
	SavePosition();
	EndModal(false);
}


void dlgEditGridOptionsFilter::OnClose(wxCloseEvent &ev)
{
	SavePosition();
	EndModal(false);
}

void dlgEditGridOptionsFilter::OnOK(wxCommandEvent &ev)
{
#ifdef __WXGTK__
	if (!btnOK->IsEnabled())
		return;
#endif

	long sel = lstFilters->GetSelection();
	if (sel >=0)
	{
		wxString filt = dlgOptions->FindItem(searchTxt->GetValue(), sel);
		dlgOptions->SaveFiltersOK(filt);
	}

	SavePosition();
	EndModal(true);
}

void dlgEditGridOptionsFilter::OnText(wxCommandEvent &event)
{
	lstFilters->Refresh();
}

wxString ctlListViewVirtual::OnGetItemText(long item, long column) const
{
	if (THIS->dlgOptions->histoFilters.GetCount() < 1)
		return wxEmptyString;
	wxString str;
	if (searchTxt->IsEmpty())
	{
		str = THIS->dlgOptions->histoFilters.Item(item);
		return (column) ? str.Mid(20) : str.Mid(0, 19);
	}
	str = THIS->dlgOptions->FindItem(searchTxt->GetValue(), item);
    return (column) ? str.Mid(20) : str.Mid(0, 19);
}

int ctlListViewVirtual::OnGetItemColumnImage(long item, long column) const
{
    return -1;
}

wxListItemAttr *ctlListViewVirtual::OnGetItemAttr(long item) const
{
    return NULL;
}
