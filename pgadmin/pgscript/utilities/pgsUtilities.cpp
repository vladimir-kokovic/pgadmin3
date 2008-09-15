//////////////////////////////////////////////////////////////////////////
//
// pgScript - PostgreSQL Tools
// RCS-ID:      $Id: pgsUtilities.cpp,v 1.2 2008/08/10 17:45:37 pgunittest Exp $
// Copyright (C) 2002 - 2008, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
//////////////////////////////////////////////////////////////////////////


#include "pgAdmin3.h"
#include "pgscript/utilities/pgsUtilities.h"

wxString pgsUtilities::uniform_line_returns(wxString s)
{
	s.Replace(wxT("\r\n"), wxT("\n"));
	s.Replace(wxT("\r"), wxT("\n"));
	return s;
}

wxString pgsUtilities::escape_quotes(wxString s)
{
	s.Replace(wxT("\\"), wxT("\\\\"));
	s.Replace(wxT("'"), wxT("''"));
	return s;
}
	
wxString pgsUtilities::unescape_quotes(wxString s)
{
	s.Replace(wxT("''"), wxT("'"));
	s.Replace(wxT("\\'"), wxT("'"));
	return s;
}