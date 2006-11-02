/*
 * Copyright (c) 1999-2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: dup_expr.cc,v 1.18.2.2 2006/11/02 02:13:15 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  <cassert>

NetEBComp* NetEBComp::dup_expr() const
{
      NetEBComp*result = new NetEBComp(op_, left_->dup_expr(),
				       right_->dup_expr());
      result->set_line(*this);
      return result;
}

NetEConst* NetEConst::dup_expr() const
{
      NetEConst*tmp = new NetEConst(value_);
      tmp->set_line(*this);
      return tmp;
}

NetEConstParam* NetEConstParam::dup_expr() const
{
      NetEConstParam*tmp = new NetEConstParam(scope_, name_, value());
      tmp->set_line(*this);
      return tmp;
}

NetECRealParam* NetECRealParam::dup_expr() const
{
      NetECRealParam*tmp = new NetECRealParam(scope_, name_, value());
      tmp->set_line(*this);
      return tmp;
}

NetEEvent* NetEEvent::dup_expr() const
{
      assert(0);
      return 0;
}

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
      NetESFunc*tmp = new NetESFunc(name_, type_, expr_width(), nparms());
      assert(tmp);

      tmp->cast_signed(has_sign());
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
      tmp->set_line(*this);
      return tmp;
}

NetETernary* NetETernary::dup_expr() const
{
      NetETernary*tmp = new NetETernary(cond_->dup_expr(),
					true_val_->dup_expr(),
					false_val_->dup_expr());
      return tmp;
}

NetEUFunc* NetEUFunc::dup_expr() const
{
      NetEUFunc*tmp;
      svector<NetExpr*> tmp_parms (parms_.count());

      for (unsigned idx = 0 ;  idx < tmp_parms.count() ;  idx += 1) {
	    assert(parms_[idx]);
	    tmp_parms[idx] = parms_[idx]->dup_expr();
      }

      tmp = 0;
      if (result_sig_)
	    tmp = new NetEUFunc(func_, result_sig_->dup_expr(), tmp_parms);
      if (result_var_)
	    tmp = new NetEUFunc(func_, result_var_->dup_expr(), tmp_parms);

      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUnary* NetEUnary::dup_expr() const
{
      NetEUnary*tmp = new NetEUnary(op_, expr_->dup_expr());
      assert(tmp);
      return tmp;
}

NetEUReduce* NetEUReduce::dup_expr() const
{
      NetEUReduce*tmp = new NetEUReduce(op_, expr_->dup_expr());
      assert(tmp);
      return tmp;
}

NetEVariable* NetEVariable::dup_expr() const
{
      NetEVariable*tmp = new NetEVariable(var_);
      return tmp;
}

/*
 * $Log: dup_expr.cc,v $
 * Revision 1.18.2.2  2006/11/02 02:13:15  steve
 *  Error message for condit expression not synthesized.
 *
 * Revision 1.18.2.1  2006/06/12 00:16:50  steve
 *  Add support for -Wunused warnings.
 *
 * Revision 1.18  2004/06/17 16:06:18  steve
 *  Help system function signedness survive elaboration.
 *
 * Revision 1.17  2004/05/31 23:34:36  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.16  2003/10/31 02:47:11  steve
 *  NetEUReduce has its own dup_expr method.
 *
 * Revision 1.15  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.14  2003/04/22 04:48:29  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.13  2003/03/15 18:08:43  steve
 *  Comparison operators do have defined width.
 *
 * Revision 1.12  2003/03/15 04:46:28  steve
 *  Better organize the NetESFunc return type guesses.
 *
 * Revision 1.11  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.10  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.9  2002/11/09 00:25:27  steve
 *  Add dup_expr for user defined function calls.
 *
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
 */

