//////////////////////////////////////////////////////////////////////////
//
// pgScript - PostgreSQL Tools
// RCS-ID:      $Id: pgsOr.h,v 1.2 2008/08/10 17:45:37 pgunittest Exp $
// Copyright (C) 2002 - 2008, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
//////////////////////////////////////////////////////////////////////////


#ifndef PGSOR_H_
#define PGSOR_H_

#include "pgscript/pgScript.h"
#include "pgscript/expressions/pgsOperation.h"

class pgsOr : public pgsOperation
{
	
public:

	pgsOr(const pgsExpression * left, const pgsExpression * right);

	virtual ~pgsOr();

	virtual pgsExpression * clone() const;

	pgsOr(const pgsOr & that);

	pgsOr & operator =(const pgsOr & that);

	virtual wxString value() const;
	
	virtual pgsOperand eval(pgsVarMap & vars) const;
	
};

#endif /*PGSOR_H_*/