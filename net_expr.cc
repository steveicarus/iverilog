/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: net_expr.cc,v 1.2 2002/01/29 22:36:31 steve Exp $"
#endif

# include  "config.h"
# include  "netlist.h"

NetESelect::NetESelect(NetExpr*exp, NetExpr*base, unsigned wid)
: expr_(exp), base_(base)
{
      expr_width(wid);
}

NetESelect::~NetESelect()
{
      delete expr_;
      delete base_;
}

const NetExpr*NetESelect::sub_expr() const
{
      return expr_;
}

const NetExpr*NetESelect::select() const
{
      return base_;
}

bool NetESelect::has_width() const
{
      return true;
}

bool NetESelect::set_width(unsigned w)
{
      if (expr_width() == 1)
	    return true;
      else
	    return false;
}

/*
 * $Log: net_expr.cc,v $
 * Revision 1.2  2002/01/29 22:36:31  steve
 *  include config.h to eliminate warnings.
 *
 * Revision 1.1  2002/01/28 01:39:45  steve
 *  Add ne_expr.cc
 *
 */

