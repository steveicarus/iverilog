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
#ident "$Id: net_assign.cc,v 1.2 2000/09/02 23:40:13 steve Exp $"
#endif

# include  "netlist.h"

/*
 * NetAssign
 */

NetAssign_::NetAssign_(const string&n, unsigned w)
: NetNode(n, w), bmux_(0)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::OUTPUT);
	    pin(idx).set_name("P", idx);
      }

}

NetAssign_::~NetAssign_()
{
      if (bmux_) delete bmux_;
}

void NetAssign_::set_bmux(NetExpr*r)
{
      assert(bmux_ == 0);
      bmux_ = r;
}

const NetExpr* NetAssign_::bmux() const
{
      return bmux_;
}

NetAssignBase::NetAssignBase(NetAssign_*lv, NetExpr*rv)
: lval_(lv), rval_(rv)
{
}

NetAssignBase::~NetAssignBase()
{
      if (rval_) delete rval_;
      if (lval_) delete lval_;
}

NetExpr* NetAssignBase::rval()
{
      return rval_;
}

const NetExpr* NetAssignBase::rval() const
{
      return rval_;
}

void NetAssignBase::set_rval(NetExpr*r)
{
      if (rval_) delete rval_;
      rval_ = r;
}

NetAssign_* NetAssignBase::l_val(unsigned idx)
{
      assert(idx == 0);
      return lval_;
}

const NetAssign_* NetAssignBase::l_val(unsigned idx) const
{
      assert(idx == 0);
      return lval_;
}


NetAssign::NetAssign(NetAssign_*lv, NetExpr*rv)
: NetAssignBase(lv, rv)
{
}

NetAssign::~NetAssign()
{
}

NetAssignNB::NetAssignNB(NetAssign_*lv, NetExpr*rv)
: NetAssignBase(lv, rv)
{
}

NetAssignNB::~NetAssignNB()
{
}

/*
 * $Log: net_assign.cc,v $
 * Revision 1.2  2000/09/02 23:40:13  steve
 *  Pull NetAssign_ creation out of constructors.
 *
 * Revision 1.1  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 */

