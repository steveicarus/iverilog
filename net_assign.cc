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
#ifdef HAVE_CVS_IDENT
#ident "$Id: net_assign.cc,v 1.17 2004/02/18 17:11:56 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"

/*
 * NetAssign
 */

unsigned count_lval_width(const NetAssign_*idx)
{
      unsigned wid = 0;
      while (idx) {
	    wid += idx->lwidth();
	    idx = idx->more;
      }
      return wid;
}

NetAssign_::NetAssign_(NetNet*s)
: sig_(s), mem_(0), var_(0), bmux_(0)
{
      loff_ = 0;
      lwid_ = sig_->pin_count();
      sig_->incr_lref();
      more = 0;
}

NetAssign_::NetAssign_(NetMemory*s)
: sig_(0), mem_(s), var_(0), bmux_(0)
{
      loff_ = 0;
      lwid_ = mem_->width();
      more = 0;
}

NetAssign_::NetAssign_(NetVariable*s)
: sig_(0), mem_(0), var_(s), bmux_(0)
{
      loff_ = 0;
      lwid_ = 0;
      more = 0;
}

NetAssign_::~NetAssign_()
{
      if (sig_) {
	    sig_->decr_lref();
	    if (sig_->peek_lref() == 0)
		  sig_->type(NetNet::WIRE);
      }

      assert( more == 0 );
      if (bmux_) delete bmux_;
}

void NetAssign_::set_bmux(NetExpr*r)
{
      assert(bmux_ == 0);
      bmux_ = r;
}

NetExpr* NetAssign_::bmux()
{
      return bmux_;
}

const NetExpr* NetAssign_::bmux() const
{
      return bmux_;
}

unsigned NetAssign_::lwidth() const
{
      if (mem_)  return lwid_;
      else if (bmux_) return 1;
      else return lwid_;
}

perm_string NetAssign_::name() const
{
      if (sig_) {
	    return sig_->name();
      } else if (mem_) {
	    return mem_->name();
      } else {
	    return perm_string::literal("");
      }
}

NetNet* NetAssign_::sig() const
{
      return sig_;
}

NetMemory* NetAssign_::mem() const
{
      return mem_;
}

NetVariable* NetAssign_::var() const
{
      return var_;
}


void NetAssign_::set_part(unsigned lo, unsigned lw)
{
      loff_ = lo;
      lwid_ = lw;
      if (sig_) {
	    assert(sig_->pin_count() >= (lo + lw));
      } else {
	    assert(mem_);
	    assert(lwid_ == mem_->width());
      }
}

unsigned NetAssign_::get_loff() const
{
      return loff_;
}

NetAssignBase::NetAssignBase(NetAssign_*lv, NetExpr*rv)
: lval_(lv), rval_(rv), delay_(0)
{
}

NetAssignBase::~NetAssignBase()
{
      if (rval_) delete rval_;
      while (lval_) {
	    NetAssign_*tmp = lval_;
	    lval_ = tmp->more;
	    tmp->more = 0;
	    delete tmp;
      }
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
      NetAssign_*cur = lval_;
      while (idx > 0) {
	    if (cur == 0)
		  return cur;

	    cur = cur->more;
	    idx -= 1;
      }

      assert(idx == 0);
      return cur;
}

const NetAssign_* NetAssignBase::l_val(unsigned idx) const
{
      const NetAssign_*cur = lval_;
      while (idx > 0) {
	    if (cur == 0)
		  return cur;

	    cur = cur->more;
	    idx -= 1;
      }

      assert(idx == 0);
      return cur;
}

unsigned NetAssignBase::l_val_count() const
{
      const NetAssign_*cur = lval_;
      unsigned cnt = 0;
      while (cur) {
	    cnt += 1;
	    cur = cur->more;
      }

      return cnt;
}

unsigned NetAssignBase::lwidth() const
{
      unsigned sum = 0;
      for (NetAssign_*cur = lval_ ;  cur ;  cur = cur->more)
	    sum += cur->lwidth();
      return sum;
}

void NetAssignBase::set_delay(NetExpr*expr)
{
      delay_ = expr;
}

const NetExpr* NetAssignBase::get_delay() const
{
      return delay_;
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
 * Revision 1.17  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.16  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.15  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.14  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.13  2002/07/02 03:02:57  steve
 *  Change the signal to a net when assignments go away.
 *
 * Revision 1.12  2002/06/08 23:42:46  steve
 *  Add NetRamDq synthsesis from memory l-values.
 *
 * Revision 1.11  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.10  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.9  2002/04/21 22:31:02  steve
 *  Redo handling of assignment internal delays.
 *  Leave it possible for them to be calculated
 *  at run time.
 *
 * Revision 1.8  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.7  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.6  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.5  2000/09/20 02:53:15  steve
 *  Correctly measure comples l-values of assignments.
 *
 * Revision 1.4  2000/09/10 02:18:16  steve
 *  elaborate complex l-values
 *
 * Revision 1.3  2000/09/07 00:06:53  steve
 *  encapsulate access to the l-value expected width.
 *
 * Revision 1.2  2000/09/02 23:40:13  steve
 *  Pull NetAssign_ creation out of constructors.
 *
 * Revision 1.1  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 */

