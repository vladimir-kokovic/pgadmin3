//////////////////////////////////////////////////////////////////////////
//
// pgScript - PostgreSQL Tools
// RCS-ID:      $Id: pgsArithmeticException.cpp,v 1.4 2008/08/13 13:24:02 pgunittest Exp $
// Copyright (C) 2002 - 2008, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
//////////////////////////////////////////////////////////////////////////


#include "pgAdmin3.h"
#include "pgscript/exceptions/pgsArithmeticException.h"

pgsArithmeticException::pgsArithmeticException(const wxString & left, const wxString & right) :
	pgsException(), m_left(left), m_right(right)
{
	
}

pgsArithmeticException::~pgsArithmeticException()
{
	
}

const wxString pgsArithmeticException::message() const
{
	return wxString() << wxT("[EXCEPT] Arithmetic Exception - Operation impossible between ")
			<< m_left << wxT(" and ") << m_right;
}