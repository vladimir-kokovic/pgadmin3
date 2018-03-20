//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// xh_searchctrl.cpp - wxSearchCtrl handler
//
//////////////////////////////////////////////////////////////////////////

#include "pgAdmin3.h"

#include "wx/wx.h"
#include "ctl/xh_searchctrl.h"


IMPLEMENT_DYNAMIC_CLASS(wxSearchTextCtrlXmlHandler, wxXmlResourceHandler)

wxSearchTextCtrlXmlHandler::wxSearchTextCtrlXmlHandler()
	: wxXmlResourceHandler()
{
}


wxObject *wxSearchTextCtrlXmlHandler::DoCreateResource()
{
	XRC_MAKE_INSTANCE(instance, wxSearchCtrl);

	instance->Create(m_parentAsWindow,
	                 GetID(), GetName(),
	                 GetPosition(), GetSize(),
	                 GetStyle());

	SetupWindow(instance);

	return instance;
}

bool wxSearchTextCtrlXmlHandler::CanHandle(wxXmlNode *node)
{
	return IsOfClass(node, wxT("wxSearchCtrl"));
}
