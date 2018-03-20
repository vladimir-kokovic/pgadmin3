//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2014, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// dlgEditGridOptionsFilter.h - Edit grid options
//
//////////////////////////////////////////////////////////////////////////

#ifndef __DLGEDITGRIDOPTIONSFILTER_H
#define __DLGEDITGRIDOPTIONSFILTER_H

// wxWindows headers
#include <wx/wx.h>

#include "dlg/dlgClasses.h"

class frmEditGrid;

////////////////////////////////////////////////////////////////////////////////
// Class declaration
////////////////////////////////////////////////////////////////////////////////

class dlgEditGridOptionsFilter : public pgDialog
{
public:

	// Construction
	dlgEditGridOptionsFilter(frmEditGrid *win, dlgEditGridOptions *dlgOptions);
	~dlgEditGridOptionsFilter();
	void RestorePosition(int defaultX = -1, int defaultY = -1, int defaultW = -1, int defaultH = -1, int minW = -1, int minH = -1);
	void SavePosition();

	dlgEditGridOptions *dlgOptions;

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
