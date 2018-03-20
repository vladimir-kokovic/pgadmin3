//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// XH_CTLLISTVIEWVIRTUALHISTORY.cpp - wxListCtrl handler
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

#include "wx/wx.h"
#include "ctl/xh_ctlListViewVirtualHistory.h"


IMPLEMENT_DYNAMIC_CLASS(wxListViewVirtualHistoryXmlHandler, wxXmlResourceHandler)

wxListViewVirtualHistoryXmlHandler::wxListViewVirtualHistoryXmlHandler()
	: wxXmlResourceHandler()
{
}


wxObject *wxListViewVirtualHistoryXmlHandler::DoCreateResource()
{
	ctlListViewVirtualHistory *win = new ctlListViewVirtualHistory(m_parentAsWindow,
	                GetID(),
	                GetPosition(),
					GetSize(),
	                GetStyle() | wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL);

	SetupWindow(win);

	return win;
}

bool wxListViewVirtualHistoryXmlHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("ctlListViewVirtualHistory"));
}
