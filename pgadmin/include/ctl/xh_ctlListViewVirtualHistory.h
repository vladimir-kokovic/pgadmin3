//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
//
// Copyright (C) 2002 - 2013, The pgAdmin Development Team
// This software is released under the PostgreSQL Licence
//
// xh_ctlListViewVirtualHistory.h - wxListCtrl handler
//
//////////////////////////////////////////////////////////////////////////


#ifndef _WX_XH_CTLLISTVIEWVIRTUALHISTORY_H_
#define _WX_XH_CTLLISTVIEWVIRTUALHISTORY_H_

#include "wx/xrc/xmlres.h"
#include "wx/listctrl.h"

class wxListViewVirtualHistoryXmlHandler : public wxXmlResourceHandler
{
	DECLARE_DYNAMIC_CLASS(wxListViewVirtualHistoryXmlHandler)
public:
	wxListViewVirtualHistoryXmlHandler();
	virtual wxObject *DoCreateResource();
	virtual bool CanHandle(wxXmlNode *node);
};


#endif // _WX_XH_CTLLISTVIEWVIRTUAL_H_
