/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: dup_expr.cc,v 1.8 2002/08/12 01:34:58 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  <cassert>

NetEScope* NetEScope::dup_expr() const
{
      assert(0);
      return 0;
}

NetESelect* NetESelect::dup_expr() const
{
      return new NetESelect(expr_->dup_expr(), base_->dup_expr(),
			    expr_width());
}

NetESFunc* NetESFunc::dup_expr() const
{
      NetESFunc*tmp = new NetESFunc(name_, expr_width(), nparms());
      assert(tmp);
      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    assert(tmp->parm(idx));
	    tmp->parm(idx, tmp->parm(idx)->dup_expr());
      }

      return tmp;
}

NetESignal* NetESignal::dup_expr() const
{
      NetESignal*tmp = new NetESignal(net_, msi_, lsi_);
      assert(tmp);
      tmp->expr_width(expr_width());
      return tmp;
}

NetETernary* NetETernary::dup_expr() const
{
      NetETernary*tmp = new NetETernary(cond_->dup_expr(),
					true_val_->dup_expr(),
					false_val_->dup_expr());
      return tmp;
}

NetEUnary* NetEUnary::dup_expr() const
{
      NetEUnary*tmp = new NetEUnary(op_, expr_->dup_expr());
      assert(tmp);
      return tmp;
}

/*
 * $Log: dup_expr.cc,v $
 * Revision 1.8  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.7  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.6  2001/11/19 01:54:14  steve
 *  Port close cropping behavior from mcrgb
 *  Move window array reset to libmc.
 *
 * Revision 1.5  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.4  2000/05/07 18:20:07  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.3  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.2  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 */

