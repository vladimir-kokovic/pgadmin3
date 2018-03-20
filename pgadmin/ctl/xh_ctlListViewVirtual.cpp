//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// XH_CTLLISTVIEWVIRTUAL.cpp - wxListCtrl handler
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

#include "wx/wx.h"
#include "ctl/xh_ctlListViewVirtual.h"


IMPLEMENT_DYNAMIC_CLASS(wxListViewVirtualXmlHandler, wxXmlResourceHandler)

wxListViewVirtualXmlHandler::wxListViewVirtualXmlHandler()
	: wxXmlResourceHandler()
{
}


wxObject *wxListViewVirtualXmlHandler::DoCreateResource()
{
	ctlListViewVirtual *win = new ctlListViewVirtual(m_parentAsWindow,
	                GetID(),
	                GetPosition(),
					GetSize(),
	                GetStyle() | wxLC_REPORT | wxLC_VIRTUAL);

	SetupWindow(win);

	return win;
}

bool wxListViewVirtualXmlHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("ctlListViewVirtual"));
}
