/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: net_variable.cc,v 1.1 2003/01/28 16:23:27 steve Exp $"
#endif

# include "config.h"
# include "netlist.h"

NetVariable::NetVariable(const char*name)
{
      name_ = strdup(name);
      scope_ = 0;
      snext_ = 0;
}

NetVariable::~NetVariable()
{
      free(name_);
}

const char*NetVariable::basename() const
{
      return name_;
}

NetScope* NetVariable::scope()
{
      return scope_;
}

const NetScope* NetVariable::scope() const
{
      return scope_;
}

NetEVariable::NetEVariable(NetVariable*v)
: var_(v)
{
}

NetEVariable::~NetEVariable()
{
}

NetExpr::TYPE NetEVariable::expr_type() const
{
      return NetExpr::ET_REAL;
}

const NetVariable* NetEVariable::variable() const
{
      return var_;
}

/*
 * $Log: net_variable.cc,v $
 * Revision 1.1  2003/01/28 16:23:27  steve
 *  Add missing net_variable.cc to CVS.
 *
 */
