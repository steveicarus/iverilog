/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: net_proc.cc,v 1.1 2000/07/07 04:53:54 steve Exp $"
#endif

# include  "netlist.h"
# include  <assert.h>

NetForever::NetForever(NetProc*p)
: statement_(p)
{
}

NetForever::~NetForever()
{
      delete statement_;
}

NetPDelay::NetPDelay(unsigned long d, NetProc*st)
: delay_(d), expr_(0), statement_(st)
{
}

NetPDelay::NetPDelay(NetExpr*d, NetProc*st)
: delay_(0), expr_(d), statement_(st)
{
}

NetPDelay::~NetPDelay()
{
      if (expr_) delete expr_;
}

unsigned long NetPDelay::delay() const
{
      assert(expr_ == 0);
      return delay_;
}

const NetExpr* NetPDelay::expr() const
{
      return expr_;
}

NetRepeat::NetRepeat(NetExpr*e, NetProc*p)
: expr_(e), statement_(p)
{
}

NetRepeat::~NetRepeat()
{
      delete expr_;
      delete statement_;
}

const NetExpr* NetRepeat::expr() const
{
      return expr_;
}



/*
 * $Log: net_proc.cc,v $
 * Revision 1.1  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 */

