//////////////////////////////////////////////////////////////////////////
//
// pgAdmin III - PostgreSQL Tools
// Copyright (C) 2002 - 2003, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
// dlgSchema.h - Schemaproperty 
//
//////////////////////////////////////////////////////////////////////////


#ifndef __DLG_SCHEMAPROP
#define __DLG_SCHEMAPROP

#include "dlgProperty.h"

class pgSchema;

class dlgSchema : public dlgSecurityProperty
{
public:
    dlgSchema(frmMain *frame, pgSchema *db);
    int Go(bool modal);

    wxString GetSql();
    pgObject *CreateObject(pgCollection *collection);
    pgObject *GetObject();

private:
    pgSchema *schema;
    void OnChange(wxNotifyEvent &ev);

    DECLARE_EVENT_TABLE();
};


#endif
