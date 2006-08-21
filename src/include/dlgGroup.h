//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// RCS-ID:      $Id: dlgGroup.h 4875 2006-01-06 21:06:46Z dpage $
// Copyright (C) 2002 - 2006, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// dlgGroup.h - Group property 
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DLG_GROUPPROP
#define __DLG_GROUPPROP

#include "dlgProperty.h"

class pgGroup;

class dlgGroup : public dlgProperty
{
public:
    dlgGroup(pgaFactory *factory, frmMain *frame, pgGroup *node=0);
    wxString GetSql();
    pgObject *CreateObject(pgCollection *collection);
    pgObject *GetObject();

    void CheckChange();
    int Go(bool modal);

private:
    pgGroup *group;

    void OnUserAdd(wxCommandEvent &ev);
    void OnUserRemove(wxCommandEvent &ev);

    wxArrayString usersIn;

    DECLARE_EVENT_TABLE()
};


#endif
