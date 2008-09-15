//////////////////////////////////////////////////////////////////////////
//
// pgScript - PostgreSQL Tools
// RCS-ID:      $Id: pgsOver.cpp,v 1.2 2008/08/10 17:45:36 pgunittest Exp $
// Copyright (C) 2002 - 2008, The pgAdmin Development Team
// This software is released under the Artistic Licence
//
//////////////////////////////////////////////////////////////////////////


#include "pgAdmin3.h"
#include "pgscript/expressions/pgsOver.h"

#include "pgscript/objects/pgsVariable.h"

pgsOver::pgsOver(const pgsExpression * left, const pgsExpression * right) :
	pgsOperation(left, right)
{
	
}

pgsOver::~pgsOver()
{
	
}

pgsExpression * pgsOver::clone() const
{
	return pnew pgsOver(*this);
}

pgsOver::pgsOver(const pgsOver & that) :
	pgsOperation(that)
{

}

pgsOver & pgsOver::operator =(const pgsOver & that)
{
	if (this != &that)
	{
		pgsOperation::operator=(that);
	}
	return (*this);
}

wxString pgsOver::value() const
{
	return wxString() << m_left->value() << wxT(" / ") << m_right->value();
}

pgsOperand pgsOver::eval(pgsVarMap & vars) const
{
	// Evaluate operands
	pgsOperand left(m_left->eval(vars));
	pgsOperand right(m_right->eval(vars));
	
	// Return the result
	return (*left / *right);
}