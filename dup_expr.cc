/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  "netlist.h"
# include  <cassert>
# include  <cstdlib>
# include  "ivl_assert.h"

using namespace std;

NetEAccess* NetEAccess::dup_expr() const
{
      NetEAccess*tmp = new NetEAccess(branch_, nature_);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEArrayPattern*NetEArrayPattern::dup_expr() const
{
      vector<NetExpr*>tmp (items_.size());
      for (size_t idx = 0 ; idx < tmp.size() ; idx += 1)
	    tmp[idx] = items_[idx]->dup_expr();

      NetEArrayPattern*res = new NetEArrayPattern(net_type(), tmp);
      res->set_line(*this);
      return res;
}

NetEBinary* NetEBinary::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetEBAdd* NetEBAdd::dup_expr() const
{
      NetEBAdd*tmp = new NetEBAdd(op_, left_->dup_expr(), right_->dup_expr(),
                                  expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBBits* NetEBBits::dup_expr() const
{
      NetEBBits*tmp = new NetEBBits(op_, left_->dup_expr(), right_->dup_expr(),
                                    expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBComp* NetEBComp::dup_expr() const
{
      NetEBComp*tmp = new NetEBComp(op_, left_->dup_expr(), right_->dup_expr());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBDiv* NetEBDiv::dup_expr() const
{
      NetEBDiv*tmp = new NetEBDiv(op_, left_->dup_expr(), right_->dup_expr(),
                                  expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBLogic* NetEBLogic::dup_expr() const
{
      NetEBLogic*tmp = new NetEBLogic(op_, left_->dup_expr(), right_->dup_expr());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBMult* NetEBMult::dup_expr() const
{
      NetEBMult*tmp = new NetEBMult(op_, left_->dup_expr(), right_->dup_expr(),
                                    expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBPow* NetEBPow::dup_expr() const
{
      NetEBPow*tmp = new NetEBPow(op_, left_->dup_expr(), right_->dup_expr(),
                                  expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEBShift* NetEBShift::dup_expr() const
{
      NetEBShift*tmp = new NetEBShift(op_, left_->dup_expr(), right_->dup_expr(),
                                      expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConcat* NetEConcat::dup_expr() const
{
      NetEConcat*dup = new NetEConcat(parms_.size(), repeat_, expr_type_);
      ivl_assert(*this, dup);
      dup->set_line(*this);
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
	    if (parms_[idx]) {
		  NetExpr*tmp = parms_[idx]->dup_expr();
                  ivl_assert(*this, tmp);
		  dup->parms_[idx] = tmp;
	    }

      dup->expr_width(expr_width());

      return dup;
}

NetEConst* NetEConst::dup_expr() const
{
      NetEConst*tmp = new NetEConst(value_);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConstEnum* NetEConstEnum::dup_expr() const
{
      NetEConstEnum*tmp = new NetEConstEnum(name_, enumeration(), value());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEConstParam* NetEConstParam::dup_expr() const
{
      NetEConstParam*tmp = new NetEConstParam(scope_, name_, value());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetECReal* NetECReal::dup_expr() const
{
      NetECReal*tmp = new NetECReal(value_);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetECRealParam* NetECRealParam::dup_expr() const
{
      NetECRealParam*tmp = new NetECRealParam(scope_, name_, value());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEEvent* NetEEvent::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetELast* NetELast::dup_expr() const
{
      NetELast*tmp = new NetELast(sig_);
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetENetenum* NetENetenum::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetENew* NetENew::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetENull* NetENull::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetEProperty* NetEProperty::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetEScope* NetEScope::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetESelect* NetESelect::dup_expr() const
{
      NetESelect*tmp = new NetESelect(expr_->dup_expr(),
			              base_? base_->dup_expr() : 0,
			              expr_width(), sel_type_);
      ivl_assert(*this, tmp);
      tmp->cast_signed(has_sign());
      tmp->set_line(*this);
      return tmp;
}

NetESFunc* NetESFunc::dup_expr() const
{
      NetESFunc*tmp = new NetESFunc(name_, type_, expr_width(), nparms(), is_overridden_);
      ivl_assert(*this, tmp);

      tmp->cast_signed(has_sign());
      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    ivl_assert(*this, parm(idx));
	    tmp->parm(idx, parm(idx)->dup_expr());
      }

      tmp->set_line(*this);
      return tmp;
}

NetEShallowCopy* NetEShallowCopy::dup_expr() const
{
      ivl_assert(*this, 0);
      return 0;
}

NetESignal* NetESignal::dup_expr() const
{
      NetESignal*tmp = new NetESignal(net_, word_);
      ivl_assert(*this, tmp);
      tmp->expr_width(expr_width());
      tmp->cast_signed(has_sign());
      tmp->set_line(*this);
      return tmp;
}

NetETernary* NetETernary::dup_expr() const
{
      NetETernary*tmp = new NetETernary(cond_->dup_expr(),
					true_val_->dup_expr(),
					false_val_->dup_expr(),
                                        expr_width(),
                                        has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUFunc* NetEUFunc::dup_expr() const
{
      NetEUFunc*tmp;
      vector<NetExpr*> tmp_parms (parms_.size());

      for (unsigned idx = 0 ;  idx < tmp_parms.size() ;  idx += 1) {
	    ivl_assert(*this, parms_[idx]);
	    tmp_parms[idx] = parms_[idx]->dup_expr();
      }

      tmp = new NetEUFunc(scope_, func_, result_sig_->dup_expr(), tmp_parms,
                          need_const_);

      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUBits* NetEUBits::dup_expr() const
{
      NetEUBits*tmp = new NetEUBits(op_, expr_->dup_expr(), expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUnary* NetEUnary::dup_expr() const
{
      NetEUnary*tmp = new NetEUnary(op_, expr_->dup_expr(), expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetEUReduce* NetEUReduce::dup_expr() const
{
      NetEUReduce*tmp = new NetEUReduce(op_, expr_->dup_expr());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}

NetECast* NetECast::dup_expr() const
{
      NetECast*tmp = new NetECast(op_, expr_->dup_expr(), expr_width(), has_sign());
      ivl_assert(*this, tmp);
      tmp->set_line(*this);
      return tmp;
}
