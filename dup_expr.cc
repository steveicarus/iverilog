/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "netlist.h"
# include  <cassert>
# include  <cstdlib>
# include  "ivl_assert.h"

NetEAccess* NetEAccess::dup_expr() const
{
      NetEAccess*tmp = new NetEAccess(branch_, nature_);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBComp* NetEBComp::dup_expr() const
{
      NetEBComp*tmp = new NetEBComp(op_, left_->dup_expr(),
				    right_->dup_expr());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConst* NetEConst::dup_expr() const
{
      NetEConst*tmp = new NetEConst(value_);
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConstParam* NetEConstParam::dup_expr() const
{
      NetEConstParam*tmp = new NetEConstParam(scope_, name_, value());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetECRealParam* NetECRealParam::dup_expr() const
{
      NetECRealParam*tmp = new NetECRealParam(scope_, name_, value());
      assert(tmp);
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
      NetESelect*tmp = new NetESelect(expr_->dup_expr(),
			              base_? base_->dup_expr() : 0,
			              expr_width());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetESFunc* NetESFunc::dup_expr() const
{
      NetESFunc*tmp = new NetESFunc(name_, type_, expr_width(), nparms());
      assert(tmp);

      tmp->cast_signed(has_sign());
      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    assert(parm(idx));
	    tmp->parm(idx, parm(idx)->dup_expr());
      }

      tmp->set_line(*this);
      return tmp;
}

NetESignal* NetESignal::dup_expr() const
{
      NetESignal*tmp = new NetESignal(net_, word_);
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
      assert(tmp);
      tmp->set_line(*this);
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

      tmp = new NetEUFunc(scope_, func_, result_sig_->dup_expr(), tmp_parms);

      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUBits* NetEUBits::dup_expr() const
{
      NetEUBits*tmp = new NetEUBits(op_, expr_->dup_expr());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUnary* NetEUnary::dup_expr() const
{
      NetEUnary*tmp = new NetEUnary(op_, expr_->dup_expr());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUReduce* NetEUReduce::dup_expr() const
{
      NetEUReduce*tmp = new NetEUReduce(op_, expr_->dup_expr());
      assert(tmp);
      tmp->set_line(*this);
      return tmp;
}
