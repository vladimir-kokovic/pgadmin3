//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2015, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgQueryHistory.h - Query history
//
//////////////////////////////////////////////////////////////////////////

#ifndef __DLGQUERYHISTORY_H
#define __DLGQUERYHISTORY_H

// wxWindows headers
#include <wx/wx.h>

#include "dlg/dlgClasses.h"
#include "frm/frmQuery.h"

class frmQuery;

////////////////////////////////////////////////////////////////////////////////
// Class declaration
////////////////////////////////////////////////////////////////////////////////

class dlgQueryHistory : public pgDialog
{
public:

	// Construction
	dlgQueryHistory(frmQuery *win);
	~dlgQueryHistory();
	void RestorePosition(int defaultX = -1, int defaultY = -1, int defaultW = -1, int defaultH = -1, int minW = -1, int minH = -1);
	void SavePosition();
	wxString FindItem(const wxString &str, long itemno);

	frmQuery *parentdlg;
	ctlListViewVirtualHistory *lstfilters;

private:

#ifdef __WXMAC__
	void OnChangeSize(wxSizeEvent &ev);
#endif

	void OnCancel(wxCommandEvent &ev);
	void OnClose(wxCloseEvent &ev);
	void OnOK(wxCommandEvent &ev);
	void OnText(wxCommandEvent &event);

	// Macros
	DECLARE_EVENT_TABLE()
};

#endif
