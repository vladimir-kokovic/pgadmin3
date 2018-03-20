//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// xh_searchctrl.h - wxSearchCtrl handler
//
//////////////////////////////////////////////////////////////////////////


#ifndef _WX_XH_SEARCHCTRL_H_
#define _WX_XH_SEARCHCTRL_H_

#include "wx/xrc/xmlres.h"
#include "wx/srchctrl.h"

class wxSearchTextCtrlXmlHandler : public wxXmlResourceHandler
{
	DECLARE_DYNAMIC_CLASS(wxSearchTextCtrlXmlHandler)
public:
	wxSearchTextCtrlXmlHandler();
	virtual wxObject *DoCreateResource();
	virtual bool CanHandle(wxXmlNode *node);
};


#endif // _WX_XH_SEARCHCTRL_H_
