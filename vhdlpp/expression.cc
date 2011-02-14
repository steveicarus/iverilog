/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include "expression.h"

Expression::Expression()
{
}

Expression::~Expression()
{
}

bool Expression::evaluate(int64_t&) const
{
      return false;
}

ExpInteger::ExpInteger(int64_t val)
: value_(val)
{
}

ExpInteger::~ExpInteger()
{
}

bool ExpInteger::evaluate(int64_t&val) const
{
      val = value_;
      return true;
}

ExpLogical::ExpLogical(ExpLogical::fun_t ty, Expression*op1, Expression*op2)
: fun_(ty), operand1_(op1), operand2_(op2)
{
}

ExpLogical::~ExpLogical()
{
      delete operand1_;
      delete operand2_;
}

ExpName::ExpName(perm_string nn)
: name_(nn)
{
}

ExpName::~ExpName()
{
}
