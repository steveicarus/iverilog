/*
 * Copyright (c) 1999-2015 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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
# include  <typeinfo>
# include  <cstdlib>
# include  <cstring>
# include  <climits>
# include "compiler.h"

# include  "PPackage.h"
# include  "pform.h"
# include  "netlist.h"
# include  "netclass.h"
# include  "netenum.h"
# include  "netparray.h"
# include  "netvector.h"
# include  "discipline.h"
# include  "netmisc.h"
# include  "netdarray.h"
# include  "netstruct.h"
# include  "netscalar.h"
# include  "util.h"
# include  "ivl_assert.h"

bool type_is_vectorable(ivl_variable_type_t type)
{
      switch (type) {
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    return true;
	  default:
	    return false;
      }
}

static ivl_nature_t find_access_function(const pform_name_t&path)
{
      if (path.size() != 1)
	    return 0;
      else
	    return access_function_nature[peek_tail_name(path)];
}

/*
 * Look at the signal to see if there is already a branch that
 * connects the sig to the gnd. If there is, then return it. If not,
 * return 0.
 */
static NetBranch* find_existing_implicit_branch(NetNet*sig, NetNet*gnd)
{
      Nexus*nex = sig->pin(0).nexus();
      for (Link*cur = nex->first_nlink() ; cur ; cur = cur->next_nlink()) {
	    if (cur->is_equal(sig->pin(0)))
		  continue;

	    if (cur->get_pin() != 0)
		  continue;

	    NetBranch*tmp = dynamic_cast<NetBranch*> (cur->get_obj());
	    if (tmp == 0)
		  continue;

	    if (tmp->name())
		  continue;

	    if (tmp->pin(1).is_linked(gnd->pin(0)))
		  return tmp;
      }

      return 0;
}

NetExpr* elaborate_rval_expr(Design*des, NetScope*scope, ivl_type_t lv_net_type,
			     ivl_variable_type_t lv_type, unsigned lv_width,
			     PExpr*expr, bool need_const)
{
      if (debug_elaborate) {
	    cerr << expr->get_fileline() << ": elaborate_rval_expr: "
		 << "expr=" << *expr;
	    if (lv_net_type)
		  cerr << ", lv_net_type=" << *lv_net_type;
	    else
		  cerr << ", lv_net_type=<nil>";

	    cerr << ", lv_type=" << lv_type
		 << ", lv_width=" << lv_width
		 << endl;
      }

      int context_wid = -1;
      switch (lv_type) {
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	      // For these types, use a different elab_and_eval that
	      // uses the lv_net_type. We should eventually transition
	      // all the types to this new form.
	    if (lv_net_type)
		  return elab_and_eval(des, scope, expr, lv_net_type, need_const);
	    break;
	  case IVL_VT_REAL:
	  case IVL_VT_STRING:
	    break;
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
            context_wid = lv_width;
	    break;
	  case IVL_VT_VOID:
	  case IVL_VT_NO_TYPE:
	    ivl_assert(*expr, 0);
	    break;
	  case IVL_VT_CLASS:
	    cerr << expr->get_fileline() << ": sorry: "
		 << "I do not know how to elaborate r-value as IVL_VT_CLASS." << endl;
	    des->errors += 1;
	    return 0;
	    break;
      }

      return elab_and_eval(des, scope, expr, context_wid, need_const,
			   false, lv_type);
}

/*
 * If the mode is UPSIZE, make sure the final expression width is at
 * least integer_width, but return the calculated lossless width to
 * the caller.
 */
unsigned PExpr::fix_width_(width_mode_t mode)
{
      unsigned width = expr_width_;
      if ((mode == UPSIZE) && type_is_vectorable(expr_type_)
          && (width < integer_width))
            expr_width_ = integer_width;

      return width;
}

unsigned PExpr::test_width(Design*des, NetScope*, width_mode_t&)
{
      cerr << get_fileline() << ": internal error: I do not know how to"
	   << " test the width of this expression. " << endl;
      cerr << get_fileline() << ":               : Expression is: " << *this
	   << endl;
      des->errors += 1;
      return 1;
}

NetExpr* PExpr::elaborate_expr(Design*des, NetScope*, ivl_type_t, unsigned) const
{
      cerr << get_fileline() << ": internal error: I do not know how to"
	   << " elaborate (ivl_type_t) this expression. " << endl;
      cerr << get_fileline() << ":               : Expression is: " << *this
	   << endl;
      cerr << get_fileline() << ":               : Expression type: " << typeid(*this).name() << endl;
      des->errors += 1;
      return 0;
}


NetExpr* PExpr::elaborate_expr(Design*des, NetScope*, unsigned, unsigned) const
{
      cerr << get_fileline() << ": internal error: I do not know how to"
	   << " elaborate this expression. " << endl;
      cerr << get_fileline() << ":               : Expression is: " << *this
	   << endl;
      des->errors += 1;
      return 0;
}

/*
 * For now, assume that assignment patterns are for dynamic
 * objects. This is not really true as this expression type, fully
 * supported, can assign to packed arrays and structs, unpacked arrays
 * and dynamic arrays.
 */
unsigned PEAssignPattern::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_  = IVL_VT_DARRAY;
      expr_width_ = 1;
      min_width_  = 1;
      signed_flag_= false;
      return 1;
}

NetExpr*PEAssignPattern::elaborate_expr(Design*des, NetScope*scope,
					ivl_type_t ntype, unsigned flags) const
{
	// Special case: If this is an empty pattern (i.e. '{}) and
	// the expected type is a DARRAY, then convert this to a null
	// handle. Internally, Icarus Verilog uses this to represent
	// nil dynamic arrays.
      if (parms_.size() == 0 && ntype->base_type()==IVL_VT_DARRAY) {
	    NetENull*tmp = new NetENull;
	    tmp->set_line(*this);
	    return tmp;
      }

      if (ntype->base_type()==IVL_VT_DARRAY)
	    return elaborate_expr_darray_(des, scope, ntype, flags);

      cerr << get_fileline() << ": sorry: I don't know how to elaborate "
	   << "assignment_pattern expressions yet." << endl;
      cerr << get_fileline() << ":      : Expression is: " << *this
	   << endl;
      des->errors += 1;
      return 0;
}

NetExpr*PEAssignPattern::elaborate_expr_darray_(Design*des, NetScope*scope,
						ivl_type_t ntype, unsigned flags) const
{
      const netdarray_t*array_type = dynamic_cast<const netdarray_t*> (ntype);
      ivl_assert(*this, array_type);

	// This is an array pattern, so run through the elements of
	// the expression and elaborate each as if they are
	// element_type expressions.
      ivl_type_t elem_type = array_type->element_type();
      vector<NetExpr*> elem_exprs (parms_.size());
      for (size_t idx = 0 ; idx < parms_.size() ; idx += 1) {
	    NetExpr*tmp = parms_[idx]->elaborate_expr(des, scope, elem_type, flags);
	    elem_exprs[idx] = tmp;
      }

      NetEArrayPattern*res = new NetEArrayPattern(array_type, elem_exprs);
      res->set_line(*this);
      return res;
}

NetExpr* PEAssignPattern::elaborate_expr(Design*des, NetScope*, unsigned, unsigned) const
{
      cerr << get_fileline() << ": sorry: I do not know how to"
	   << " elaborate assignment patterns using old method." << endl;
      cerr << get_fileline() << ":      : Expression is: " << *this
	   << endl;
      des->errors += 1;
      ivl_assert(*this, 0);
      return 0;
}

unsigned PEBinary::test_width(Design*des, NetScope*scope, width_mode_t&mode)
{
      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

      unsigned r_width = right_->test_width(des, scope, mode);

      width_mode_t saved_mode = mode;

      unsigned l_width = left_->test_width(des, scope, mode);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEBinary::test_width: "
		 << "op_=" << op_ << ", l_width=" << l_width
		 << ", r_width=" << r_width
		 << ", saved_mode=" << saved_mode << endl;
      }

        // If the width mode changed, retest the right operand, as it
        // may choose a different width if it is in a lossless context.
      if ((mode >= LOSSLESS) && (saved_mode < LOSSLESS))
	    r_width = right_->test_width(des, scope, mode);

      ivl_variable_type_t l_type =  left_->expr_type();
      ivl_variable_type_t r_type = right_->expr_type();

      if (l_type == IVL_VT_REAL || r_type == IVL_VT_REAL)
	    expr_type_ = IVL_VT_REAL;
      else if (l_type == IVL_VT_LOGIC || r_type == IVL_VT_LOGIC)
	    expr_type_ = IVL_VT_LOGIC;
      else
	    expr_type_ = IVL_VT_BOOL;

      if (expr_type_ == IVL_VT_REAL) {
            expr_width_  = 1;
            min_width_   = 1;
            signed_flag_ = true;
      } else {
            expr_width_  = max(l_width, r_width);
            min_width_   = max(left_->min_width(), right_->min_width());
            signed_flag_ = left_->has_sign() && right_->has_sign();

              // If the operands are different types, the expression is
              // forced to unsigned. In this case the lossless width
              // calculation is unreliable and we need to make sure the
              // final expression width is at least integer_width.
            if ((mode == LOSSLESS) && (left_->has_sign() != right_->has_sign()))
                  mode = UPSIZE;

            switch (op_) {
                case '+':
                case '-':
                  if (mode >= EXPAND)
                        expr_width_ += 1;
                  break;

                case '*':
                  if (mode >= EXPAND)
                        expr_width_ = l_width + r_width;
                  break;

                case '%':
                case '/':
                  min_width_ = UINT_MAX; // disable width pruning
                  break;

                case 'l': // <<  Should be handled by PEBShift
                case 'r': // >>  Should be handled by PEBShift
                case 'R': // >>> Should be handled by PEBShift
                case '<': // <   Should be handled by PEBComp
                case '>': // >   Should be handled by PEBComp
                case 'e': // ==  Should be handled by PEBComp
                case 'E': // === Should be handled by PEBComp
                case 'L': // <=  Should be handled by PEBComp
                case 'G': // >=  Should be handled by PEBComp
                case 'n': // !=  Should be handled by PEBComp
                case 'N': // !== Should be handled by PEBComp
                case 'p': // **  should be handled by PEBPower
                  ivl_assert(*this, 0);
                default:
                  break;
            }
      }

      return fix_width_(mode);
}

/*
 * Elaborate binary expressions. This involves elaborating the left
 * and right sides, and creating one of a variety of different NetExpr
 * types.
 */
NetExpr* PEBinary::elaborate_expr(Design*des, NetScope*scope,
				  unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

	// Handle the special case that one of the operands is a real
	// value and the other is a vector type. In that case,
	// elaborate the vectorable argument as self-determined.
        // Propagate the expression type (signed/unsigned) down to
        // any context-determined operands.
      unsigned l_width = expr_wid;
      unsigned r_width = expr_wid;
      if (left_->expr_type()==IVL_VT_REAL
	  && type_is_vectorable(right_->expr_type())) {
	    r_width = right_->expr_width();
      } else {
            right_->cast_signed(signed_flag_);
      }
      if (right_->expr_type()==IVL_VT_REAL
	  && type_is_vectorable(left_->expr_type())) {
	    l_width = left_->expr_width();
      } else {
            left_->cast_signed(signed_flag_);
      }

      NetExpr*lp =  left_->elaborate_expr(des, scope, l_width, flags);
      NetExpr*rp = right_->elaborate_expr(des, scope, r_width, flags);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      return elaborate_expr_base_(des, lp, rp, expr_wid);
}

/*
 * This is the common elaboration of the operator. It presumes that the
 * operands are elaborated as necessary, and all I need to do is make
 * the correct NetEBinary object and connect the parameters.
 */
NetExpr* PEBinary::elaborate_expr_base_(Design*des,
					NetExpr*lp, NetExpr*rp,
					unsigned expr_wid) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: elaborate expression "
		 << *this << " expr_width=" << expr_wid << endl;
      }

      NetExpr*tmp;

      switch (op_) {
	  default:
	    tmp = new NetEBinary(op_, lp, rp, expr_wid, signed_flag_);
	    tmp->set_line(*this);
	    break;

	  case 'a':
	  case 'o':
	    cerr << get_fileline() << ": internal error: "
		 << "Elaboration of " << human_readable_op(op_)
		 << " Should have been handled in NetEBLogic::elaborate."
		 << endl;
	    des->errors += 1;
	    return 0;

	  case 'p':
	    cerr << get_fileline() << ": internal error: "
		 << "Elaboration of " << human_readable_op(op_)
		 << " Should have been handled in NetEBPower::elaborate."
		 << endl;
	    des->errors += 1;
	    return 0;

	  case '*':
	    tmp = elaborate_expr_base_mult_(des, lp, rp, expr_wid);
	    break;

	  case '%':
	  case '/':
	    tmp = elaborate_expr_base_div_(des, lp, rp, expr_wid);
	    break;

	  case 'l':
	  case 'r':
	  case 'R':
	    cerr << get_fileline() << ": internal error: "
		 << "Elaboration of " << human_readable_op(op_)
		 << " Should have been handled in NetEBShift::elaborate."
		 << endl;
	    des->errors += 1;
	    return 0;

	  case '^':
	  case '&':
	  case '|':
	  case 'O': // NOR (~|)
	  case 'A': // NAND (~&)
	  case 'X':
	    tmp = elaborate_expr_base_bits_(des, lp, rp, expr_wid);
	    break;

	  case '+':
	  case '-':
	    tmp = new NetEBAdd(op_, lp, rp, expr_wid, signed_flag_);
	    tmp->set_line(*this);
	    break;

	  case 'E': /* === */
	  case 'N': /* !== */
	  case 'e': /* == */
	  case 'n': /* != */
	  case 'L': /* <= */
	  case 'G': /* >= */
	  case '<':
	  case '>':
	    cerr << get_fileline() << ": internal error: "
		 << "Elaboration of " << human_readable_op(op_)
		 << " Should have been handled in NetEBComp::elaborate."
		 << endl;
	    des->errors += 1;
	    return 0;

	  case 'm': // min(l,r)
	  case 'M': // max(l,r)
	    tmp = new NetEBMinMax(op_, lp, rp, expr_wid, signed_flag_);
	    tmp->set_line(*this);
	    break;
      }

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_bits_(Design*des,
					     NetExpr*lp, NetExpr*rp,
					     unsigned expr_wid) const
{
      if (lp->expr_type() == IVL_VT_REAL || rp->expr_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: "
	         << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetEBBits*tmp = new NetEBBits(op_, lp, rp, expr_wid, signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_div_(Design*des,
					    NetExpr*lp, NetExpr*rp,
					    unsigned expr_wid) const
{
	/* The % operator does not support real arguments in
	   baseline Verilog. But we allow it in our extended
	   form of Verilog. */
      if (op_ == '%' && ! gn_icarus_misc_flag) {
	    if (lp->expr_type() == IVL_VT_REAL ||
		rp->expr_type() == IVL_VT_REAL) {
		  cerr << get_fileline() << ": error: Modulus operator "
			"may not have REAL operands." << endl;
		  des->errors += 1;
	    }
      }

      NetEBDiv*tmp = new NetEBDiv(op_, lp, rp, expr_wid, signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_mult_(Design*,
					     NetExpr*lp, NetExpr*rp,
					     unsigned expr_wid) const
{
	// Keep constants on the right side.
      if (dynamic_cast<NetEConst*>(lp)) {
	    NetExpr*tmp = lp;
	    lp = rp;
	    rp = tmp;
      }

	// Handle a few special case multiplies against constants.
      if (NetEConst*rp_const = dynamic_cast<NetEConst*> (rp)) {
	    verinum rp_val = rp_const->value();

	    if (!rp_val.is_defined() && (lp->expr_type() == IVL_VT_LOGIC)) {
		  NetEConst*tmp = make_const_x(expr_wid);
                  tmp->cast_signed(signed_flag_);
                  tmp->set_line(*this);

		  return tmp;
	    }

	    if (rp_val.is_zero() && (lp->expr_type() == IVL_VT_BOOL)) {
		  NetEConst*tmp = make_const_0(expr_wid);
                  tmp->cast_signed(signed_flag_);
                  tmp->set_line(*this);

		  return tmp;
	    }
      }

      NetEBMult*tmp = new NetEBMult(op_, lp, rp, expr_wid, signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

unsigned PEBComp::test_width(Design*des, NetScope*scope, width_mode_t&)
{
      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

	// The width and type of a comparison are fixed and well known.
      expr_type_   = IVL_VT_LOGIC;
      expr_width_  = 1;
      min_width_   = 1;
      signed_flag_ = false;

	// The widths of the operands are semi-self-determined. They
        // affect each other, but not the result.
      width_mode_t mode = SIZED;

      unsigned r_width = right_->test_width(des, scope, mode);

      width_mode_t saved_mode = mode;

      unsigned l_width = left_->test_width(des, scope, mode);

        // If the width mode changed, retest the right operand, as it
        // may choose a different width if it is in a lossless context.
      if ((mode >= LOSSLESS) && (saved_mode < LOSSLESS))
	    r_width = right_->test_width(des, scope, mode);

      ivl_variable_type_t l_type =  left_->expr_type();
      ivl_variable_type_t r_type = right_->expr_type();

      l_width_ = l_width;
      if (type_is_vectorable(l_type) && (r_width > l_width))
	    l_width_ = r_width;

      r_width_ = r_width;
      if (type_is_vectorable(r_type) && (l_width > r_width))
	    r_width_ = l_width;

	// If the expression is lossless and smaller than the integer
	// minimum, then tweak the size up.
	// NOTE: I really would rather try to figure out what it would
	// take to get expand the sub-expressions so that they are
	// exactly the right width to behave just like infinite
	// width. I suspect that adding 1 more is sufficient in all
	// cases, but I'm not certain. Ideas?
      if (mode >= EXPAND) {
            if (type_is_vectorable(l_type) && (l_width_ < integer_width))
	          l_width_ += 1;
            if (type_is_vectorable(r_type) && (r_width_ < integer_width))
	          r_width_ += 1;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: "
		 << "Comparison expression operands are "
		 << l_type << " " << l_width << " bits and "
		 << r_type << " " << r_width << " bits. Resorting to "
		 << l_width_ << " bits and "
		 << r_width_ << " bits." << endl;
      }

      return expr_width_;
}

NetExpr* PEBComp::elaborate_expr(Design*des, NetScope*scope,
				 unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

        // Propagate the comparison type (signed/unsigned) down to
        // the operands.
      if (type_is_vectorable(left_->expr_type()) && !left_->has_sign())
	    right_->cast_signed(false);
      if (type_is_vectorable(right_->expr_type()) && !right_->has_sign())
	    left_->cast_signed(false);

      NetExpr*lp =  left_->elaborate_expr(des, scope, l_width_, flags);
      NetExpr*rp = right_->elaborate_expr(des, scope, r_width_, flags);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      eval_expr(lp, l_width_);
      eval_expr(rp, r_width_);

	// Handle some operand-specific special cases...
      switch (op_) {
	  case 'E': /* === */
	  case 'N': /* !== */
	    if (lp->expr_type() == IVL_VT_REAL ||
		lp->expr_type() == IVL_VT_STRING ||
		rp->expr_type() == IVL_VT_REAL ||
		rp->expr_type() == IVL_VT_STRING) {
		  cerr << get_fileline() << ": error: "
		       << human_readable_op(op_)
		       << " operator may not have REAL or STRING operands."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }
	    break;
	  default:
	    break;
      }

      NetExpr*tmp = new NetEBComp(op_, lp, rp);
      tmp->set_line(*this);

      tmp = pad_to_width(tmp, expr_wid, *this);
      tmp->cast_signed(signed_flag_);

      return tmp;
}

unsigned PEBLogic::test_width(Design*, NetScope*, width_mode_t&)
{
	// The width and type of a logical operation are fixed.
      expr_type_   = IVL_VT_LOGIC;
      expr_width_  = 1;
      min_width_   = 1;
      signed_flag_ = false;

        // The widths of the operands are self determined. We don't need
        // them now, so they can be tested when they are elaborated.

      return expr_width_;
}

NetExpr*PEBLogic::elaborate_expr(Design*des, NetScope*scope,
				 unsigned expr_wid, unsigned flags) const
{
      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

      bool need_const = NEED_CONST & flags;
      NetExpr*lp = elab_and_eval(des, scope,  left_, -1, need_const);
      NetExpr*rp = elab_and_eval(des, scope, right_, -1, need_const);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      lp = condition_reduce(lp);
      rp = condition_reduce(rp);

      NetExpr*tmp = new NetEBLogic(op_, lp, rp);
      tmp->set_line(*this);

      tmp = pad_to_width(tmp, expr_wid, *this);
      tmp->cast_signed(signed_flag_);

      return tmp;
}

unsigned PEBLeftWidth::test_width(Design*des, NetScope*scope, width_mode_t&mode)
{
      ivl_assert(*this, left_);
      ivl_assert(*this, right_);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEBLeftWidth::test_width: "
		 << "op_=" << op_
		 << ", left_=" << *left_
		 << ", right_=" << *right_
		 << ", mode=" << width_mode_name(mode) << endl;
      }

        // The right operand is self determined. Test its type and
        // width for use later. We only need to know its width now
        // if the left operand is unsized and we need to calculate
        // the lossless width.
      width_mode_t r_mode = SIZED;
      unsigned r_width = right_->test_width(des, scope, r_mode);

	// The left operand is what will determine the size of the
	// expression. The l_mode will be converted to UNSIZED if the
	// expression does not have a well-determined size.
      width_mode_t l_mode = SIZED;
      expr_width_  = left_->test_width(des, scope, l_mode);
      expr_type_   = left_->expr_type();
      signed_flag_ = left_->has_sign();

      if (mode==SIZED)
	    mode = l_mode;

	// The left operand width defines the size of the
	// expression. If the expression has a well-defined size, the
	// left_->test_width() above would have set mode==SIZED and we
	// can skip a lot of stuff. But if the mode is an undetermined
	// size, we need to figure out what we really want to keep a
	// lossless value. That's what the following if(...) {...} is
	// all about.
      if ((mode >= EXPAND) && type_is_vectorable(expr_type_)) {

              // We need to make our best guess at the right operand
              // value, to minimize the calculated width. This is
              // particularly important for the power operator...

              // Start off by assuming the maximum value for the
              // type and width of the right operand.
            long r_val = LONG_MAX;
            if (r_width < sizeof(long)*8) {
                  r_val = (1L << r_width) - 1L;
                  if ((op_ == 'p') && right_->has_sign())
                        r_val >>= 1;
            }

              // If the right operand is constant, we can use the
              // actual value.
            NetExpr*rp = right_->elaborate_expr(des, scope, r_width, NO_FLAGS);
            if (rp) {
                  eval_expr(rp, r_width);
            } else {
                  // error recovery
                  PEVoid*tmp = new PEVoid();
                  tmp->set_line(*this);
                  delete right_;
                  right_ = tmp;
            }
            NetEConst*rc = dynamic_cast<NetEConst*> (rp);
	      // Adjust the expression width that can be converter depending
	      // on if the R-value is signed or not.
	    unsigned c_width = sizeof(long)*8;
	    if (! right_->has_sign()) c_width -= 1;
	    if (rc && (r_width <= c_width)) r_val = rc->value().as_long();

	    if (debug_elaborate && rc) {
		  cerr << get_fileline() << ": PEBLeftWidth::test_width: "
		       << "Evaluated rc=" << *rc
		       << ", r_val=" << r_val
		       << ", width_cap=" << width_cap << endl;
	    }

              // Clip to a sensible range to avoid underflow/overflow
              // in the following calculations.
            if (r_val < 0)
                  r_val = 0;
            if ((unsigned long)r_val > width_cap)
                  r_val = width_cap;

              // If the left operand is a simple unsized number, we
              // can calculate the actual width required for the power
              // operator.
            PENumber*lc = dynamic_cast<PENumber*> (left_);

              // Now calculate the lossless width.
            unsigned use_width = expr_width_;
            switch (op_) {
                case 'l': // <<
		  if (l_mode != SIZED)
			use_width += (unsigned)r_val;
                  break;

                case 'r': // >>
                case 'R': // >>>
                    // A logical shift will effectively coerce a signed
                    // operand to unsigned. We have to assume an arithmetic
                    // shift may do the same, as we don't yet know the final
                    // expression type.
                  if ((mode == LOSSLESS) && signed_flag_)
                        mode = UPSIZE;
                  break;

                case 'p': // **
                  if (lc && rc) {
                        verinum result = pow(lc->value(), rc->value());
                        use_width = max(use_width, result.len());
                  } else {
                        if (signed_flag_) use_width -= 1;
                        use_width *= (unsigned)r_val;
                        if (signed_flag_) use_width += 2;
                  }
                  break;

                default:
                  cerr << get_fileline() << ": internal error: "
                       << "Unexpected opcode " << human_readable_op(op_)
                       << " in PEBLeftWidth::test_width." << endl;
                  des->errors += 1;
            }

              // If the right operand is not constant, we could end up
              // grossly overestimating the required width. So in this
              // case, don't expand beyond the width of an integer
              // (which meets the requirements of the standard).
            if ((rc == 0) && (use_width > expr_width_) && (use_width > integer_width))
                  use_width = integer_width;

	    if (use_width >= width_cap) {
		  cerr << get_fileline() << ": warning: "
		       << "Unsized expression (" << *this << ")"
		       << " expanded beyond and was clipped to " << use_width
		       << " bits. Try using sized operands." << endl;
	    }
            expr_width_ = use_width;
      }

      if (op_ == 'l')
            min_width_ = left_->min_width();
      else
            min_width_ = UINT_MAX; // disable width pruning

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEBLeftWidth::test_width: "
		 << "Done calculating expr_width_=" << expr_width_
		 << ", min_width_=" << min_width_
		 << ", mode=" << width_mode_name(mode) << endl;
      }

      return fix_width_(mode);
}

NetExpr*PEBLeftWidth::elaborate_expr(Design*des, NetScope*scope,
				     unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      ivl_assert(*this, left_);

        // The left operand is always context determined, so propagate
        // down the expression type (signed/unsigned).
      left_->cast_signed(signed_flag_);

      unsigned r_width = right_->expr_width();

      NetExpr*lp =  left_->elaborate_expr(des, scope, expr_wid, flags);
      NetExpr*rp = right_->elaborate_expr(des, scope, r_width,  flags);
      if (lp == 0 || rp == 0) {
	    delete lp;
	    delete rp;
	    return 0;
      }

        // For shift operations, the right operand is always treated as
        // unsigned, so coerce it if necessary.
      if ((op_ != 'p') && rp->has_sign()) {
            rp = new NetESelect(rp, 0, rp->expr_width());
            rp->cast_signed(false);
            rp->set_line(*this);
      }

      eval_expr(lp, expr_wid);
      eval_expr(rp, r_width);

      return elaborate_expr_leaf(des, lp, rp, expr_wid);
}

NetExpr*PEBPower::elaborate_expr_leaf(Design*, NetExpr*lp, NetExpr*rp,
				      unsigned expr_wid) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: elaborate expression "
		 << *this << " expr_wid=" << expr_wid << endl;
      }

      NetExpr*tmp = new NetEBPow(op_, lp, rp, expr_wid, signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

NetExpr*PEBShift::elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
				      unsigned expr_wid) const
{
      switch (op_) {
	  case 'l': // <<
	  case 'r': // >>
	  case 'R': // >>>
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected opcode " << human_readable_op(op_)
		 << " in PEBShift::elaborate_expr_leaf." << endl;
	    des->errors += 1;
            return 0;
      }

      if (lp->expr_type() == IVL_VT_REAL || rp->expr_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: "
	         << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
            delete lp;
            delete rp;
	    return 0;
      }

      NetExpr*tmp;

	// If the left expression is constant, then there are some
	// special cases we can work with. If the left expression is
	// not constant, but the right expression is constant, then
	// there are some other interesting cases. But if neither are
	// constant, then there is the general case.

      if (NetEConst*lpc = dynamic_cast<NetEConst*> (lp)) {

	      // Special case: The left expression is zero. If the
	      // shift value contains no 'x' or 'z' bits, the result
	      // is going to be zero.
	    if (lpc->value().is_defined() && lpc->value().is_zero()
		&& (rp->expr_type() == IVL_VT_BOOL)) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Shift of zero always returns zero."
			     << " Elaborate as constant zero." << endl;

		  tmp = make_const_0(expr_wid);
                  tmp->cast_signed(signed_flag_);
                  tmp->set_line(*this);

                  return tmp;
            }

      } else if (NetEConst*rpc = dynamic_cast<NetEConst*> (rp)) {

              // Special case: The shift value contains 'x' or 'z' bits.
              // Elaborate as a constant-x.
            if (!rpc->value().is_defined()) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Shift by undefined value. "
			     << "Elaborate as constant 'x'." << endl;

		  tmp = make_const_x(expr_wid);
                  tmp->cast_signed(signed_flag_);
                  tmp->set_line(*this);

                  delete lp;
                  delete rp;
                  return tmp;
	    }

	    unsigned long shift = rpc->value().as_ulong();

              // Special case: The shift is zero. The result is simply
              // the left operand.
	    if (shift == 0) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Shift by zero. Elaborate as the "
			     << "left hand operand." << endl;

                  delete rp;
                  return lp;
	    }

	      // Special case: the shift is at least the size of the entire
	      // left operand, and the shift is a signed right shift.
              // Elaborate as a replication of the top bit of the left
              // expression.
	    if ((op_=='R' && signed_flag_) && (shift >= expr_wid)) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Value signed-right-shifted " << shift
			     << " beyond width of " << expr_wid
			     << ". Elaborate as replicated top bit." << endl;

		  tmp = new NetEConst(verinum(expr_wid-1));
		  tmp->set_line(*this);
		  tmp = new NetESelect(lp, tmp, 1);
		  tmp->set_line(*this);
		  tmp = pad_to_width(tmp, expr_wid, *this);
		  tmp->cast_signed(true);

                  delete rp;
		  return tmp;
	    }

	      // Special case: The shift is at least the size of the entire
	      // left operand, and the shift is not a signed right shift
              // (which is caught by the previous special case). Elaborate
              // as a constant-0.
	    if (shift >= expr_wid) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Value shifted " << shift
			     << " beyond width of " << expr_wid
			     << ". Elaborate as constant zero." << endl;

		  tmp = make_const_0(expr_wid);
                  tmp->cast_signed(signed_flag_);
		  tmp->set_line(*this);

                  delete lp;
                  delete rp;
		  return tmp;
	    }
      }

	// Fallback, handle the general case.
      tmp = new NetEBShift(op_, lp, rp, expr_wid, signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

unsigned PECallFunction::test_width_sfunc_(Design*des, NetScope*scope,
                                           width_mode_t&mode)
{
      perm_string name = peek_tail_name(path_);

      if (name=="$ivlh_to_unsigned") {
	    ivl_assert(*this, parms_.size() == 2);
	      // The Icarus Verilog specific $ivlh_to_unsigned() system
	      // task takes a second argument which is the output
	      // size. This can be an arbitrary constant function.
	    PExpr*pexpr = parms_[1];
	    if (pexpr == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Missing $ivlh_to_unsigned width." << endl;
		  return 0;
	    }

	    NetExpr*nexpr = elab_and_eval(des, scope, pexpr, -1, true);
	    if (nexpr == 0) {
		  cerr << get_fileline() << ": error: "
		       << "Unable to evaluate " << name
		       << " width argument: " << *pexpr << endl;
		  return 0;
	    }

	    long value = 0;
	    bool rc = eval_as_long(value, nexpr);
	    ivl_assert(*this, rc && value>=0);

	      // The argument type/width is self-determined and doesn't
	      // affect the result type/width.
	    width_mode_t arg_mode = SIZED;
	    parms_[0]->test_width(des, scope, arg_mode);

	    expr_width_ = value;
	    signed_flag_= false;
	    return expr_width_;
      }

      if (name=="$signed" || name=="$unsigned") {
	    PExpr*expr = parms_[0];
	    if (expr == 0)
		  return 0;

              // The argument type/width is self-determined, but affects
              // the result width.
            width_mode_t arg_mode = SIZED;
	    expr_width_  = expr->test_width(des, scope, arg_mode);
	    expr_type_   = expr->expr_type();
            min_width_   = expr->min_width();
            signed_flag_ = (name[1] == 's');

            if ((arg_mode >= EXPAND) && type_is_vectorable(expr_type_)) {
                  if (mode < LOSSLESS)
                        mode = LOSSLESS;
                  if (expr_width_ < integer_width)
                        expr_width_ = integer_width;
            }

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: " << name
		       << " argument width = " << expr_width_ << "." << endl;

            return expr_width_;
      }

      if (name=="$sizeof" || name=="$bits") {
	    PExpr*expr = parms_[0];
	    if (expr == 0)
		  return 0;

	    if (! dynamic_cast<PETypename*>(expr)) {
		    // The argument type/width is self-determined and doesn't
		    // affect the result type/width. Note that if the
		    // argument is a type name (a special case) then
		    // don't bother with this step.
		  width_mode_t arg_mode = SIZED;
		  expr->test_width(des, scope, arg_mode);
	    }

	    expr_type_   = IVL_VT_BOOL;
	    expr_width_  = integer_width;
	    min_width_   = integer_width;
            signed_flag_ = false;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width"
		       << " of " << name << " returns test_width"
		       << " of compiler integer." << endl;

	    return expr_width_;
      }

      if (name=="$is_signed") {
	    PExpr*expr = parms_[0];
	    if (expr == 0)
		  return 0;

              // The argument type/width is self-determined and doesn't
              // affect the result type/width.
            width_mode_t arg_mode = SIZED;
	    expr->test_width(des, scope, arg_mode);

	    expr_type_   = IVL_VT_BOOL;
	    expr_width_  = 1;
	    min_width_   = 1;
            signed_flag_ = false;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width"
		       << " of $is_signed returns test_width"
		       << " of 1." << endl;

	    return expr_width_;
      }

	/* Get the return type of the system function by looking it up
	   in the sfunc_table. */
      const struct sfunc_return_type*sfunc_info = lookup_sys_func(name);

      expr_type_   = sfunc_info->type;
      expr_width_  = sfunc_info->wid;
      min_width_   = expr_width_;
      signed_flag_ = sfunc_info->signed_flag;

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: test_width "
		 << "of system function " << name
		 << " returns wid=" << expr_width_
		 << ", type=" << expr_type_ << "." << endl;

      return expr_width_;
}

unsigned PECallFunction::test_width(Design*des, NetScope*scope,
                                    width_mode_t&mode)
{
      if (peek_tail_name(path_)[0] == '$')
	    return test_width_sfunc_(des, scope, mode);

	// The width of user defined functions depends only on the
	// width of the return value. The arguments are entirely
	// self-determined.
      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	      // If this is an access function, then the width and
	      // type are known by definition.
	    if (find_access_function(path_)) {
		  expr_type_   = IVL_VT_REAL;
		  expr_width_  = 1;
		  min_width_   = 1;
                  signed_flag_ = true;

		  return expr_width_;
	    }

	    if (test_width_method_(des, scope, mode)) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: test_width "
			     << "of method returns width " << expr_width_
			     << ", type=" << expr_type_
			     << "." << endl;
		  return expr_width_;
	    }

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width "
		       << "cannot find definition of " << path_
		       << " in " << scope_path(scope) << "." << endl;
	    return 0;
      }

      NetScope*dscope = def->scope();
      assert(dscope);

      if (NetNet*res = dscope->find_signal(dscope->basename())) {
	    expr_type_   = res->data_type();
	    expr_width_  = res->vector_width();
            min_width_   = expr_width_;
            signed_flag_ = res->get_signed();

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width "
		       << "of function returns width " << expr_width_
		       << ", type=" << expr_type_
		       << "." << endl;

	    return expr_width_;
      }

      ivl_assert(*this, 0);
      return 0;
}

unsigned PECallFunction::test_width_method_(Design*des, NetScope*scope,
					    width_mode_t&)
{
      if (!gn_system_verilog())
	    return 0;

	// This is only useful if the path is at least 2 elements. For
	// example, foo.bar() is a method, bar() is not.
      if (path_.size() < 2)
	    return 0;

      perm_string member_name;
      ivl_type_t  member_type = 0;
      pform_name_t use_path = path_;
      perm_string method_name = peek_tail_name(use_path);
      use_path.pop_back();

      NetNet *net = 0;
      const NetExpr *par;
      NetEvent *eve;
      const NetExpr *ex1, *ex2;

      symbol_search(this, des, scope, use_path,
		    net, par, eve, ex1, ex2);

      const netdarray_t*use_darray = 0;

      if (net != 0)
	    use_darray = net->darray_type();

	// Net is not found, but maybe it is a member of a
	// struct or class. Try to locate net without the member
	// name and test if it is a type that has members.
      if (net == 0 && use_path.size() >= 2) {
	    pform_name_t tmp_path = use_path;
	    member_name = peek_tail_name(tmp_path);
	    tmp_path.pop_back();

	    net = 0;
	    symbol_search(this, des, scope, tmp_path,
			  net, par, eve, ex1, ex2);
	    if (net && net->class_type()) {
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PECallFunction::test_width_method_: "
			     << "Found net=" << tmp_path
			     << ", member_name=" << member_name
			     << ", method_name=" << method_name
			     << endl;
		  }

		  const netclass_t* class_type = net->class_type();
		  int midx = class_type->property_idx_from_name(member_name);
		  if (midx >= 0)
			member_type = class_type->get_prop_type(midx);
		  else
			member_type = 0;
		  use_path = tmp_path;

		  use_darray = dynamic_cast<const netdarray_t*> (member_type);

	    } else {
		  member_name = perm_string();
		  net = 0;
	    }
      }

	// After all, no sign of a net match. Give up.
      if (net == 0)
	    return 0;

	// Look for built in string attributes.
      if (net->data_type()==IVL_VT_STRING) {

	    if (method_name == "len") {
		  expr_type_  = IVL_VT_BOOL;
		  expr_width_ = 32;
		  min_width_  = 32;
		  signed_flag_= true;
		  return expr_width_;
	    }
      }

	// function int size()
      if (use_darray && method_name == "size") {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PECallFunction::test_width_method_: "
		       << "Match darray size() method." << endl;
	    }

	    expr_type_  = IVL_VT_BOOL;
	    expr_width_ = 32;
	    min_width_  = expr_width_;
	    signed_flag_= true;
	    return expr_width_;
      }

      if (use_darray && (method_name == "pop_back" || method_name=="pop_front")) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PECallFunction::test_width_method_: "
		       << "Detected " << method_name << " method"
		       << " of dynamic arrays." << endl;
	    }

	    expr_type_  = use_darray->element_base_type();
	    expr_width_ = use_darray->element_width();
	    min_width_  = expr_width_;
	    signed_flag_= false;

	    return expr_width_;
      }

      if (const netclass_t*class_type = net->class_type()) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PECallFunction::test_width_method_: "
		       << "Try to find method " << method_name
		       << " of class " << class_type->get_name() << endl;
	    }

	    NetScope*func = class_type->method_from_name(method_name);
	    if (func == 0) {
		  return 0;
	    }

	      // Get the function result size be getting the details
	      // from the variable in the function scope that has the
	      // name of the function.
	    if (NetNet*res = func->find_signal(method_name)) {
		  expr_type_ = res->data_type();
		  expr_width_= res->vector_width();
		  min_width_ = expr_width_;
		  signed_flag_ = res->get_signed();
		  return expr_width_;
	    } else {
		  ivl_assert(*this, 0);
	    }
      }

      return 0;
}

NetExpr*PECallFunction::cast_to_width_(NetExpr*expr, unsigned wid) const
{
      if (debug_elaborate) {
            cerr << get_fileline() << ": PECallFunction::cast_to_width_: "
		 << "cast to " << wid
                 << " bits " << (signed_flag_?"signed":"unsigned")
		 << " from expr_width()=" << expr->expr_width() << endl;
      }

        /* If the expression is a const, then replace it with a new
           const. This is a more efficient result. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
            tmp->cast_signed(signed_flag_);
            if (wid != tmp->expr_width()) {
                  tmp = new NetEConst(verinum(tmp->value(), wid));
                  tmp->set_line(*this);
                  delete expr;
            }
            return tmp;
      }

      NetESelect*tmp = new NetESelect(expr, 0, wid);
      tmp->cast_signed(signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

/*
 * Given a call to a system function, generate the proper expression
 * nodes to represent the call in the netlist. Since we don't support
 * size_tf functions, make assumptions about widths based on some
 * known function names.
 */
NetExpr* PECallFunction::elaborate_sfunc_(Design*des, NetScope*scope,
                                          unsigned expr_wid,
                                          unsigned flags) const
{
      perm_string name = peek_tail_name(path_);

	/* Catch the special case that the system function is the
	   $ivl_unsigned function. In this case the second argument is
	   the size of the expression, but should already be accounted
	   for so treat this very much like the $unsigned() function. */
      if (name=="$ivlh_to_unsigned") {
	    ivl_assert(*this, parms_.size()==2);

	    PExpr*expr = parms_[0];
	    ivl_assert(*this, expr);
	    NetExpr*sub = expr->elaborate_expr(des, scope, expr->expr_width(), flags);
	    return cast_to_width_(sub, expr_wid);
      }

	/* Catch the special case that the system function is the $signed
	   function. Its argument will be evaluated as a self-determined
           expression. */
      if (name=="$signed" || name=="$unsigned") {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The " << name
		       << " function takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PECallFunction::elaborate_sfunc_: "
		       << name << " expression is the argument cast to expr_wid=" << expr_wid << endl;
	    }
	    PExpr*expr = parms_[0];
	    NetExpr*sub = expr->elaborate_expr(des, scope, expr_width_, flags);

	    return cast_to_width_(sub, expr_wid);
      }

	/* Interpret the internal $sizeof system function to return
	   the bit width of the sub-expression. The value of the
	   sub-expression is not used, so the expression itself can be
	   deleted. */
      if (name=="$sizeof" || name=="$bits") {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The " << name
		       << " function takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (name=="$sizeof")
		  cerr << get_fileline() << ": warning: $sizeof is deprecated."
		       << " Use $bits() instead." << endl;

	    PExpr*expr = parms_[0];

	    uint64_t use_width = 0;
	    if (PETypename*type_expr = dynamic_cast<PETypename*>(expr)) {
		  ivl_type_t tmp_type = type_expr->get_type()->elaborate_type(des, scope);
		  ivl_assert(*this, tmp_type);
		  use_width = tmp_type->packed_width();
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PECallFunction::elaborate_sfunc_: "
			     << " Packed width of type argument is " << use_width << endl;
		  }

	    } else {
		  use_width = expr->expr_width();
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PECallFunction::elaborate_sfunc_: "
			     << " Width of expression argument is " << use_width << endl;
		  }
	    }

	    verinum val (use_width, integer_width);
	    NetEConst*sub = new NetEConst(val);
	    sub->set_line(*this);

	    return cast_to_width_(sub, expr_wid);
      }

	/* Interpret the internal $is_signed system function to return
	   a single bit flag -- 1 if the expression is signed, 0
	   otherwise. */
      if (name=="$is_signed") {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The " << name
		       << " function takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];

	    verinum val (expr->has_sign()? verinum::V1 : verinum::V0, 1);
	    NetEConst*sub = new NetEConst(val);
	    sub->set_line(*this);

	    return cast_to_width_(sub, expr_wid);
      }

	/* How many parameters are there? The Verilog language allows
	   empty parameters in certain contexts, so the parser will
	   allow things like func(1,,3). It will also cause func() to
	   be interpreted as a single empty parameter.

	   Functions cannot really take empty parameters, but the
	   case ``func()'' is the same as no parameters at all. So
	   catch that special case here. */
      unsigned nparms = parms_.size();
      if ((nparms == 1) && (parms_[0] == 0))
	    nparms = 0;

      NetESFunc*fun = new NetESFunc(name, expr_type_, expr_width_, nparms);
      fun->set_line(*this);

      if (!fun->is_built_in()) {
	    if (scope->need_const_func()) {
		  cerr << get_fileline() << ": error: " << name
		       << " is not a built-in function, so cannot"
		       << " be used in a constant function." << endl;
		  des->errors += 1;
	    }
	    scope->is_const_func(false);
      }

	/* Now run through the expected parameters. If we find that
	   there are missing parameters, print an error message.

	   While we're at it, try to evaluate the function parameter
	   expression as much as possible, and use the reduced
	   expression if one is created. */

      bool need_const = NEED_CONST & flags;

      unsigned parm_errors = 0;
      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    PExpr*expr = parms_[idx];
	    if (expr) {
		  NetExpr*tmp = elab_sys_task_arg(des, scope, name, idx,
                                                  expr, need_const);
                  if (tmp) {
                        fun->parm(idx, tmp);
                  } else {
                        parm_errors += 1;
                        fun->parm(idx, 0);
                  }
	    } else {
		  missing_parms += 1;
		  fun->parm(idx, 0);
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_fileline() << ": error: The function " << name
		 << " has been called with missing/empty parameters." << endl;
	    cerr << get_fileline() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }

      if (missing_parms || parm_errors)
            return 0;

      NetExpr*tmp = pad_to_width(fun, expr_wid, *this);
      tmp->cast_signed(signed_flag_);

      return tmp;
}

NetExpr* PECallFunction::elaborate_access_func_(Design*des, NetScope*scope,
						ivl_nature_t nature,
                                                unsigned expr_wid) const
{
	// An access function must have 1 or 2 arguments.
      ivl_assert(*this, parms_.size()==2 || parms_.size()==1);

      NetBranch*branch = 0;

      if (parms_.size() == 1) {
	    PExpr*arg1 = parms_[0];
	    PEIdent*arg_ident = dynamic_cast<PEIdent*> (arg1);
	    ivl_assert(*this, arg_ident);

	    const pform_name_t&path = arg_ident->path();
	    ivl_assert(*this, path.size()==1);
	    perm_string name = peek_tail_name(path);

	    NetNet*sig = scope->find_signal(name);
	    ivl_assert(*this, sig);

	    ivl_discipline_t dis = sig->get_discipline();
	    ivl_assert(*this, dis);
	    ivl_assert(*this, nature == dis->potential() || nature == dis->flow());

	    NetNet*gnd = des->find_discipline_reference(dis, scope);

	    if ( (branch = find_existing_implicit_branch(sig, gnd)) ) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Re-use implicit branch from "
			     << branch->get_fileline() << endl;
	    } else {
		  branch = new NetBranch(dis);
		  branch->set_line(*this);
		  connect(branch->pin(0), sig->pin(0));
		  connect(branch->pin(1), gnd->pin(0));

		  des->add_branch(branch);
		  join_island(branch);

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Create implicit branch." << endl;

	    }

      } else {
	    ivl_assert(*this, 0);
      }

      NetExpr*tmp = new NetEAccess(branch, nature);
      tmp->set_line(*this);

      tmp = pad_to_width(tmp, expr_wid, *this);
      tmp->cast_signed(signed_flag_);

      return tmp;
}

/*
 * Routine to look for and build enumeration method calls.
 */
static NetExpr* check_for_enum_methods(const LineInfo*li,
                                       Design*des, NetScope*scope,
                                       const netenum_t*netenum,
                                       pform_name_t use_path,
                                       perm_string method_name,
                                       NetExpr*expr,
                                       unsigned rtn_wid,
                                       PExpr*parg, unsigned args)
{
	// The "num()" method returns the number of elements.
      if (method_name == "num") {
	    if (args != 0) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".num() does not "
		          "take an argument." << endl;
		  des->errors += 1;
	    }
	    NetEConst*tmp = make_const_val(netenum->size());
	    tmp->set_line(*li);
	    delete expr; // The elaborated enum variable is not needed.
	    return tmp;
      }

	// The "first()" method returns the first enumeration value.
      if (method_name == "first") {
	    if (args != 0) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".first() does not "
		          "take an argument." << endl;
		  des->errors += 1;
	    }
	    netenum_t::iterator item = netenum->first_name();
	    NetEConstEnum*tmp = new NetEConstEnum(scope, item->first,
	                                          netenum, item->second);
	    tmp->set_line(*li);
	    delete expr; // The elaborated enum variable is not needed.
	    return tmp;
      }

	// The "last()" method returns the first enumeration value.
      if (method_name == "last") {
	    if (args != 0) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".last() does not "
		          "take an argument." << endl;
		  des->errors += 1;
	    }
	    netenum_t::iterator item = netenum->last_name();
	    NetEConstEnum*tmp = new NetEConstEnum(scope, item->first,
	                                          netenum, item->second);
	    tmp->set_line(*li);
	    delete expr; // The elaborated enum variable is not needed.
	    return tmp;
      }

      NetESFunc*sys_expr;

	// Process the method argument if it is available.
      NetExpr* count = 0;
      if (args != 0 && parg) {
	    count = elaborate_rval_expr(des, scope, &netvector_t::atom2u32,
					IVL_VT_BOOL, 32, parg);
	    if (count == 0) {
		  cerr << li->get_fileline() << ": error: unable to elaborate "
		          "enumeration method argument " << use_path << "."
		       << method_name << "(" << parg << ")." << endl;
		  args = 0;
		  des->errors += 1;
	    } else if (NetEEvent*evt = dynamic_cast<NetEEvent*> (count)) {
		  cerr << evt->get_fileline() << ": error: An event '"
		       << evt->event()->name() << "' cannot be an enumeration "
		          "method argument." << endl;
		  args = 0;
		  des->errors += 1;
	    }
      }

	// The "name()" method returns the name of the current
	// enumeration value.
      if (method_name == "name") {
	    if (args != 0) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".name() does not "
		          "take an argument." << endl;
		  des->errors += 1;
	    }
	    sys_expr = new NetESFunc("$ivl_enum_method$name", IVL_VT_STRING,
	                             rtn_wid, 2);
	    sys_expr->parm(0, new NetENetenum(netenum));
	    sys_expr->parm(1, expr);

	      /* The compiler/code generators need to be fixed to support a
	       * string return value. In some contexts we could use the
	       * expression width, but that doesn't always work. */
	    if (rtn_wid == 0) {
		  cerr << li->get_fileline() << ": sorry: Enumeration method "
		          "name() is not currently supported in this context "
		          "(self-determined)." << endl;
		  des->errors += 1;
	    }

	// The "next()" method returns the next enumeration value.
      } else if (method_name == "next") {
	    if (args > 1) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".next() take at "
		          "most one argument." << endl;
		  des->errors += 1;
	    }
	    sys_expr = new NetESFunc("$ivl_enum_method$next", netenum,
	                             2 + (args != 0));
	    sys_expr->parm(0, new NetENetenum(netenum));
	    sys_expr->parm(1, expr);
	    if (args != 0) sys_expr->parm(2, count);

	// The "prev()" method returns the previous enumeration value.
      } else if (method_name == "prev") {
	    if (args > 1) {
		  cerr << li->get_fileline() << ": error: enumeration "
		          "method " << use_path << ".prev() take at "
		          "most one argument." << endl;
		  des->errors += 1;
	    }
	    sys_expr = new NetESFunc("$ivl_enum_method$prev", netenum,
	                             2 + (args != 0));
	    sys_expr->parm(0, new NetENetenum(netenum));
	    sys_expr->parm(1, expr);
	    if (args != 0) sys_expr->parm(2, count);

	// This is an unknown enumeration method.
      } else {
	    cerr << li->get_fileline() << ": error: Unknown enumeration "
	            "method " << use_path << "." << method_name << "()."
	         << endl;
	    des->errors += 1;
	    return expr;
      }

      sys_expr->set_line(*li);

      if (debug_elaborate) {
	    cerr << li->get_fileline() << ": debug: Generate "
	         << sys_expr->name() << "(" << use_path << ")" << endl;
      }

      return sys_expr;
}

/*
 * If the method matches a structure member then return the member otherwise
 * return 0. Also return the offset of the member.
 */
static const netstruct_t::member_t*get_struct_member(const LineInfo*li,
                                                     Design*des, NetScope*,
                                                     NetNet*net,
                                                     perm_string method_name,
                                                     unsigned long&off)
{
      const netstruct_t*type = net->struct_type();
      ivl_assert(*li, type);

      if (! type->packed()) {
	    cerr << li->get_fileline()
	         << ": sorry: unpacked structures not supported here. "
		 << "Method=" << method_name << endl;
	    des->errors += 1;
	    return 0;
      }

      return type->packed_member(method_name, off);
}

bool calculate_part(const LineInfo*li, Design*des, NetScope*scope,
		    const index_component_t&index, long&off, unsigned long&wid)
{
      if (index.sel == index_component_t::SEL_BIT_LAST) {
	    cerr << li->get_fileline() << ": sorry: "
		 << "Last element select expression "
		 << "not supported." << endl;
	    des->errors += 1;
	    return false;
      }

	// Evaluate the last index expression into a constant long.
      NetExpr*texpr = elab_and_eval(des, scope, index.msb, -1, true);
      long msb;
      if (texpr == 0 || !eval_as_long(msb, texpr)) {
	    cerr << li->get_fileline() << ": error: "
		  "Array/part index expressions must be constant here." << endl;
	    des->errors += 1;
	    return false;
      }

      delete texpr;

      long lsb = msb;
      if (index.lsb) {
	    texpr = elab_and_eval(des, scope, index.lsb, -1, true);
	    if (texpr==0 || !eval_as_long(lsb, texpr)) {
		  cerr << li->get_fileline() << ": error: "
			"Array/part index expressions must be constant here." << endl;
		  des->errors += 1;
		  return false;
	    }

	    delete texpr;
      }

      switch (index.sel) {
	  case index_component_t::SEL_BIT:
	    off = msb;
	    wid = 1;
	    return true;

	  case index_component_t::SEL_PART:
	    if (msb >= lsb) {
		  off = lsb;
		  wid = msb - lsb + 1;
	    } else {
		  off = msb;
		  wid = lsb - msb + 1;
	    }
	    return true;

	  case index_component_t::SEL_IDX_UP:
	    wid = lsb;
	    off = msb;
	    break;

	  default:
	    ivl_assert(*li, 0);
	    break;
      }
      return true;
}

/*
 * Test if the tail name (method_name argument) is a member name and
 * the net is a struct. If that turns out to be the case, and the
 * struct is packed, then return a NetExpr that selects the member out
 * of the variable.
 */
static NetExpr* check_for_struct_members(const LineInfo*li,
					 Design*des, NetScope*scope,
					 NetNet*net,
					 const list<index_component_t>&base_index,
					 const name_component_t&comp)
{
      unsigned long off;
      const netstruct_t::member_t*mem = get_struct_member(li, des, 0, net,
                                                          comp.name, off);
      if (mem == 0) return 0;

      ivl_assert(*li, mem->net_type && mem->net_type->packed());

      unsigned use_width = mem->net_type->packed_width();

      if (debug_elaborate) {
	    cerr << li->get_fileline() << ": debug: check_for_struct_members: "
		 << "Found struct member " << mem->name
		 << " At offset " << off
		 << ", member width = " << use_width << endl;
      }

	// The struct member may be a packed array. Process index
	// expression that address the member element.
      if ( ! comp.index.empty() ) {
	    const netvector_t*mem_vec = dynamic_cast<const netvector_t*> (mem->net_type);
	    ivl_assert(*li, mem_vec);

	    const vector<netrange_t>&packed_dims = mem_vec->packed_dims();

	      // Evaluate all but the last index expression, into prefix_indices.
	    list<long>prefix_indices;
	    bool rc = evaluate_index_prefix(des, scope, prefix_indices, comp.index);
	    ivl_assert(*li, rc);

	      // Make sure that index values that select array
	      // elements are in fact like bit selects. The tail may
	      // be part selects only if we are taking the part-select
	      // of the word of an array.
	    ivl_assert(*li, comp.index.size() >= packed_dims.size() || comp.index.back().sel == index_component_t::SEL_BIT);

	      // Evaluate the part/bit select expressions. This may be
	      // a bit select or a part select. In any case, assume
	      // the arguments are constant and generate a part select
	      // of the appropriate width.
	    long poff = 0;
	    unsigned long pwid = 0;
	    rc = calculate_part(li, des, scope, comp.index.back(), poff, pwid);
	    ivl_assert(*li, rc);

	      // Now use the prefix_to_slice function to calculate the
	      // offset and width of the addressed slice of the member.
	    long loff;
	    unsigned long lwid;
	    prefix_to_slice(packed_dims, prefix_indices, poff, loff, lwid);

	    if (debug_elaborate) {
		  cerr << li->get_fileline() << ": debug: check_for_struct_members: "
		       << "Evaluate prefix gives slice loff=" << loff
		       << ", lwid=" << lwid << ", part select pwid=" << pwid << endl;
	    }

	    off += loff;
	    if (comp.index.size() >= packed_dims.size())
		  use_width = pwid;
	    else
		  use_width = lwid;
      }

	// If the base symbol has dimensions, then this is a packed
	// array of structures. Convert an array of indices to a
	// single part select. For example, "net" is a packed array
	// of struct, and "mem" is the struct member. In Verilog it
	// looks something like "net[idx].mem". We've already
	// converted "mem" to an offset into the packed struct, so now
	// we just canonicalize "[idx]" and add the ".mem" offset to
	// get a collapsed index.
      NetExpr*packed_base = 0;
      if(net->packed_dimensions() > 1) {
	    list<index_component_t>tmp_index = base_index;
	    index_component_t member_select;
	    member_select.sel = index_component_t::SEL_BIT;
	    member_select.msb = new PENumber(new verinum(off));
	    tmp_index.push_back(member_select);
	    packed_base = collapse_array_exprs(des, scope, li, net, tmp_index);
	    ivl_assert(*li, packed_base);
	    if (debug_elaborate) {
		  cerr << li->get_fileline() << ": debug: check_for_struct_members: "
		       << "Got collapsed array expr: " << *packed_base << endl;
	    }
      }

      long tmp;
      if (packed_base && eval_as_long(tmp, packed_base)) {
	    off = tmp;
	    delete packed_base;
	    packed_base = 0;
      }

      NetESignal*sig = new NetESignal(net);
      NetExpr  *base = packed_base? packed_base : make_const_val(off);

      if (debug_elaborate) {
	    cerr << li->get_fileline() << ": debug: check_for_struct_members: "
		 << "Convert packed indices/member select into part select: " << *base << endl;
      }

      NetESelect*sel = new NetESelect(sig, base, use_width);
      return sel;
}

static NetExpr* class_static_property_expression(const LineInfo*li,
						 const netclass_t*class_type,
						 perm_string name)
{
      NetNet*sig = class_type->find_static_property(name);
      ivl_assert(*li, sig);
      NetESignal*expr = new NetESignal(sig);
      expr->set_line(*li);
      return expr;
}

static NetExpr* check_for_class_property(const LineInfo*li,
					 Design*des, NetScope*scope,
					 NetNet*net,
					 const name_component_t&comp)
{
      const netclass_t*class_type = net->class_type();
      int pidx = class_type->property_idx_from_name(comp.name);
      if (pidx < 0) {
	    cerr << li->get_fileline() << ": error: "
		 << "Class " << class_type->get_name()
		 << " has no property " << comp.name << "." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << li->get_fileline() << ": check_for_class_property: "
		 << "Property " << comp.name
		 << " of net " << net->name()
		 << ", context scope=" << scope_path(scope)
		 << endl;
      }

      property_qualifier_t qual = class_type->get_prop_qual(pidx);
      if (qual.test_local() && ! class_type->test_scope_is_method(scope)) {
	    cerr << li->get_fileline() << ": error: "
		 << "Local property " << class_type->get_prop_name(pidx)
		 << " is not accessible in this context."
		 << " (scope=" << scope_path(scope) << ")" << endl;
	    des->errors += 1;
      }

      if (qual.test_static()) {
	    perm_string prop_name = lex_strings.make(class_type->get_prop_name(pidx));
	    return class_static_property_expression(li, class_type,
						    prop_name);
      }

      NetEProperty*tmp = new NetEProperty(net, comp.name);
      tmp->set_line(*li);
      return tmp;
}

NetExpr* PECallFunction::elaborate_expr_pkg_(Design*des, NetScope*scope,
					     unsigned expr_wid,
					     unsigned flags) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": PECallFunction::elaborate_expr_pkg_: "
		 << "Elaborate " << path_
		 << " as function in package " << package_->pscope_name()
		 << "." << endl;
      }

	// Find the package that contains this definition, and use the
	// package scope as the search starting point for the function
	// definition.
      NetScope*pscope = des->find_package(package_->pscope_name());
      ivl_assert(*this, pscope);

      NetFuncDef*def = des->find_function(pscope, path_);
      ivl_assert(*this, def);

      NetScope*dscope = def->scope();
      ivl_assert(*this, dscope);

      if (! check_call_matches_definition_(des, dscope))
	    return 0;

      return elaborate_base_(des, scope, dscope, expr_wid, flags);
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope,
					unsigned expr_wid, unsigned flags) const
{
      if (package_)
	    return elaborate_expr_pkg_(des, scope, expr_wid, flags);

      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      if (peek_tail_name(path_)[0] == '$')
	    return elaborate_sfunc_(des, scope, expr_wid, flags);

      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	      // Not a user defined function. Maybe it is an access
	      // function for a nature? If so then elaborate it that
	      // way.
	    ivl_nature_t access_nature = find_access_function(path_);
	    if (access_nature)
		  return elaborate_access_func_(des, scope, access_nature,
                                                expr_wid);

	      // Maybe this is a method attached to a signal? If this
	      // is SystemVerilog then try that possibility.
	    if (gn_system_verilog()) {
		  NetExpr*tmp = elaborate_expr_method_(des, scope, expr_wid);
		  if (tmp) return tmp;
	    }

	      // Nothing was found so report this as an error.
	    cerr << get_fileline() << ": error: No function named `" << path_
	         << "' found in this context (" << scope_path(scope) << ")."
                 << endl;
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, def);
      NetScope*dscope = def->scope();
      ivl_assert(*this, dscope);

       /* In SystemVerilog a method calling another method in the
        * current class needs to be elaborated as a method with an
        * implicit this added.  */
      if (gn_system_verilog() && (path_.size() == 1)) {
           const NetScope *c_scope = scope->get_class_scope();
           if (c_scope && (c_scope == dscope->get_class_scope())) {
                 NetExpr*tmp = elaborate_expr_method_(des, scope, expr_wid,
		                                      true);
                 assert(tmp);
                 return tmp;
           }
      }

      bool need_const = NEED_CONST & flags;

        // It is possible to get here before the called function has been
        // fully elaborated. If this is the case, elaborate it now. This
        // ensures we know whether or not it is a constant function.
      if (dscope->elab_stage() < 3) {
            dscope->need_const_func(need_const || scope->need_const_func());
            const PFunction*pfunc = dscope->func_pform();
            ivl_assert(*this, pfunc);
            pfunc->elaborate(des, dscope);
      }

      if (dscope->parent() != scope->parent() || !dscope->is_const_func()) {
            if (scope->need_const_func()) {
	          cerr << get_fileline() << ": error: A function invoked by "
                          "a constant function must be a constant function "
                          "local to the current module." << endl;
                  des->errors += 1;
            }
            scope->is_const_func(false);
      }

      return elaborate_base_(des, scope, dscope, expr_wid, flags);
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope,
					ivl_type_t type, unsigned flags) const
{
      const netdarray_t*darray = dynamic_cast<const netdarray_t*>(type);
      assert(darray);
      return elaborate_expr(des, scope, darray->element_type()->packed_width(), flags);
}

NetExpr* PECallFunction::elaborate_base_(Design*des, NetScope*scope, NetScope*dscope,
					 unsigned expr_wid, unsigned flags) const
{

      if (! check_call_matches_definition_(des, dscope))
	    return 0;

      NetFuncDef*def = dscope->func_def();

      bool need_const = NEED_CONST & flags;

	// If this is a constant expression, it is possible that we
	// are being elaborated before the function definition. If
	// that's the case, try to elaborate the function as a const
	// function.
      if (need_const && ! def->proc()) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PECallFunction::elaborate_base_: "
		       << "Try to elaborate " << scope_path(dscope)
		       << " as constant function." << endl;
	    }
	    dscope->set_elab_stage(2);
	    dscope->need_const_func(true);
	    const PFunction*pfunc = dscope->func_pform();
	    ivl_assert(*this, pfunc);
	    pfunc->elaborate(des, dscope);
      }

      unsigned parms_count = def->port_count();
      vector<NetExpr*> parms (parms_count);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PECallFunction::elaborate_base_: "
		 << "Expecting " << parms_count
		 << " argument for function " << scope_path(dscope) << "." << endl;
      }

	/* Elaborate the input expressions for the function. This is
	   done in the scope of the function call, and not the scope
	   of the function being called. The scope of the called
	   function is elaborated when the definition is elaborated. */
      unsigned parm_errors = elaborate_arguments_(des, scope,
						  def, need_const,
						  parms, 0);

      if (need_const && !dscope->is_const_func()) {

              // If this is the first time the function has been called in
              // a constant context, force the function to be re-elaborated.
              // This will generate the necessary error messages to allow
              // the user to diagnose the fault.
            if (!dscope->need_const_func()) {
                  dscope->set_elab_stage(2);
                  dscope->need_const_func(true);
                  const PFunction*pfunc = dscope->func_pform();
                  ivl_assert(*this, pfunc);
                  pfunc->elaborate(des, dscope);
            }

            cerr << get_fileline() << ": error: `" << dscope->basename()
                 << "' is not a constant function." << endl;
            des->errors += 1;
            return 0;
      }

      if (parm_errors)
            return 0;

	/* Look for the return value signal for the called
	   function. This return value is a magic signal in the scope
	   of the function, that has the name of the function. The
	   function code assigns to this signal to return a value.

	   dscope, in this case, is the scope of the function, so the
	   return value is the name within that scope. */

      if (NetNet*res = dscope->find_signal(dscope->basename())) {
	    NetESignal*eres = new NetESignal(res);
	    NetEUFunc*func = new NetEUFunc(scope, dscope, eres, parms, need_const);
	    func->set_line(*this);

	    if(res->darray_type())
	        return func;

            NetExpr*tmp = pad_to_width(func, expr_wid, *this);
            tmp->cast_signed(signed_flag_);

	    return tmp;
      }

      cerr << get_fileline() << ": internal error: Unable to locate "
	    "function return value for " << path_
	   << " in " << dscope->basename() << "." << endl;
      des->errors += 1;
      return 0;
}

/*
 * Elaborate the arguments of a function or method. The parms vector
 * is where to place the elaborated expressions, so it an output. The
 * parm_off is where in the parms vector to start writing
 * arguments. This value is normally 0, but is 1 if this is a method
 * so that parms[0] can hold the "this" argument. In this latter case,
 * def->port(0) will be the "this" argument and should be skipped.
 */
unsigned PECallFunction::elaborate_arguments_(Design*des, NetScope*scope,
					      NetFuncDef*def, bool need_const,
					      vector<NetExpr*>&parms,
					      unsigned parm_off) const
{
      unsigned parm_errors = 0;
      unsigned missing_parms = 0;

      const unsigned parm_count = parms.size() - parm_off;
      const unsigned actual_count = parms_.size();

	/* The parser can't distinguish between a function call with
	   no arguments and a function call with one empty argument,
	   and always supplies one empty argument. Handle the no
	   argument case here. */
      if ((parm_count == 0) && (actual_count == 1) && (parms_[0] == 0))
	    return 0;

      if (actual_count > parm_count) {
	    cerr << get_fileline() << ": error: "
		 << "Too many arguments (" << actual_count
		 << ", expecting " << parm_count << ")"
		 << " in call to function." << endl;
	    des->errors += 1;
      }

      for (unsigned idx = 0 ; idx < parm_count ; idx += 1) {
	    unsigned pidx = idx + parm_off;
	    PExpr*tmp = (idx < actual_count) ? parms_[idx] : 0;
	    if (tmp) {
		  parms[pidx] = elaborate_rval_expr(des, scope,
						    def->port(pidx)->net_type(),
						    def->port(pidx)->data_type(),
						    (unsigned)def->port(pidx)->vector_width(),
						    tmp, need_const);
		  if (parms[pidx] == 0) {
			parm_errors += 1;
			continue;
		  }

		  if (NetEEvent*evt = dynamic_cast<NetEEvent*> (parms[pidx])) {
			cerr << evt->get_fileline() << ": error: An event '"
			     << evt->event()->name() << "' can not be a user "
			        "function argument." << endl;
			des->errors += 1;
		  }
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug:"
			     << " function " << path_
			     << " arg " << (idx+1)
			     << " argwid=" << parms[pidx]->expr_width()
			     << ": " << *parms[idx] << endl;

	    } else if (def->port_defe(pidx)) {
		  if (! gn_system_verilog()) {
			cerr << get_fileline() << ": internal error: "
			     << "Found (and using) default function argument "
			     << "requires SystemVerilog." << endl;
			des->errors += 1;
		  }
		  parms[pidx] = def->port_defe(pidx);

	    } else {
		  missing_parms += 1;
		  parms[pidx] = 0;
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_fileline() << ": error: The function " << path_
		 << " has been called with missing/empty parameters." << endl;
	    cerr << get_fileline() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    parm_errors += 1;
	    des->errors += 1;
      }

      return parm_errors;
}

NetExpr* PECallFunction::elaborate_expr_method_(Design*des, NetScope*scope,
						unsigned expr_wid,
						bool add_this_flag) const
{
      pform_name_t use_path = path_;
      perm_string method_name = peek_tail_name(use_path);
      use_path.pop_back();

	/* Add the implicit this reference when requested. */
      if (add_this_flag) {
	    assert(use_path.empty());
	    use_path.push_front(name_component_t(perm_string::literal("@")));
      }

	// If there is no object to the left of the method name, then
	// give up on the idea of looking for an object method.
      if (use_path.empty()) return 0;

      NetNet *net = 0;
      const NetExpr *par;
      NetEvent *eve;
      const NetExpr *ex1, *ex2;

      symbol_search(this, des, scope, use_path,
		    net, par, eve, ex1, ex2);

      if (net == 0)
	    return 0;

      if (net->data_type() == IVL_VT_STRING) {

	    if (method_name == "len") {
		  NetESFunc*sys_expr = new NetESFunc("$ivl_string_method$len",
						     IVL_VT_BOOL, 32, 1);
		  sys_expr->parm(0, new NetESignal(net));
		  return sys_expr;
	    }

	    if (method_name == "substr") {
		  NetESFunc*sys_expr = new NetESFunc("$ivl_string_method$substr",
						     IVL_VT_STRING, 1, 3);
		  sys_expr->set_line(*this);

		    // First argument is the source string.
		  sys_expr->parm(0, new NetESignal(net));

		  ivl_assert(*this, parms_.size() == 2);
		  NetExpr*tmp;

		  tmp = elaborate_rval_expr(des, scope, &netvector_t::atom2u32,
					    IVL_VT_BOOL, 32, parms_[0], false);
		  sys_expr->parm(1, tmp);

		  tmp = elaborate_rval_expr(des, scope, &netvector_t::atom2u32,
					    IVL_VT_BOOL, 32, parms_[1], false);
		  sys_expr->parm(2, tmp);

		  return sys_expr;
	    }
      }

      if (const netenum_t*netenum = net->enumeration()) {
	      // We may need the net expression for the
	      // enumeration variable so get it.
	    NetESignal*expr = new NetESignal(net);
	    expr->set_line(*this);
	      // This expression cannot be a select!
	    assert(use_path.back().index.empty());

	    PExpr*tmp = parms_.size() ? parms_[0] : 0;
	    return check_for_enum_methods(this, des, scope,
					  netenum, use_path,
					  method_name, expr,
					  expr_wid, tmp,
					  parms_.size());
      }

      if (net->darray_type()) {

	    if (method_name == "size") {
		  NetESFunc*sys_expr = new NetESFunc("$size",
						     IVL_VT_BOOL, 32, 1);
		  sys_expr->parm(0, new NetESignal(net));
		  sys_expr->set_line(*this);
		  return sys_expr;
	    }

	    if (method_name == "pop_back") {
		  NetESFunc*sys_expr = new NetESFunc("$ivl_darray_method$pop_back",
						     expr_type_,
						     expr_width_, 1);
		  sys_expr->parm(0, new NetESignal(net));
		  sys_expr->set_line(*this);
		  return sys_expr;
	    }

	    if (method_name == "pop_front") {
		  NetESFunc*sys_expr = new NetESFunc("$ivl_darray_method$pop_front",
						     expr_type_,
						     expr_width_, 1);
		  sys_expr->parm(0, new NetESignal(net));
		  sys_expr->set_line(*this);
		  return sys_expr;
	    }

      }

      if (const netclass_t*class_type = net->class_type()) {
	    NetScope*func = class_type->method_from_name(method_name);
	    if (func == 0) {
		  return 0;
	    }

	    NetFuncDef*def = func->func_def();
	    ivl_assert(*this, def);

	    NetNet*res = func->find_signal(func->basename());
	    ivl_assert(*this, res);

	    vector<NetExpr*>parms;

	    NetESignal*ethis = new NetESignal(net);
	    ethis->set_line(*this);
	    parms.push_back(ethis);

	    parms.resize(1 + parms_.size());
	    elaborate_arguments_(des, scope, def, false, parms, 1);

	    NetESignal*eres = new NetESignal(res);
	    NetEUFunc*call = new NetEUFunc(scope, func, eres, parms, false);
	    call->set_line(*this);
	    return call;
      }

      return 0;
}

unsigned PECastSize::test_width(Design*des, NetScope*scope, width_mode_t&)
{
      expr_width_ = size_;

      width_mode_t tmp_mode = PExpr::SIZED;
      base_->test_width(des, scope, tmp_mode);

      return size_;
}

NetExpr* PECastSize::elaborate_expr(Design*des, NetScope*scope,
				    unsigned, unsigned) const
{
      NetExpr*sub = base_->elaborate_expr(des, scope, base_->expr_width(), NO_FLAGS);
      NetESelect*sel = new NetESelect(sub, 0, size_);
      sel->set_line(*this);

      return sel;
}

unsigned PECastType::test_width(Design*des, NetScope*scope, width_mode_t&wid)
{
      ivl_type_t t = target_->elaborate_type(des, scope);
      base_->test_width(des, scope, wid);

      if(const netdarray_t*use_darray = dynamic_cast<const netdarray_t*> (t)) {
	    expr_type_  = use_darray->element_base_type();
	    expr_width_ = use_darray->element_width();
      }

      else if(const netstring_t*use_string = dynamic_cast<const netstring_t*> (t)) {
	    expr_type_  = use_string->base_type();
	    expr_width_ = 8;
      }

      else {
	    expr_type_  = t->base_type();
	    expr_width_ = t->packed_width();
      }

      signed_flag_ = t->get_signed();
      min_width_ = expr_width_;
      return expr_width_;
}

NetExpr* PECastType::elaborate_expr(Design*des, NetScope*scope,
                                    ivl_type_t type, unsigned) const
{
    const netdarray_t*darray = NULL;
    const netvector_t*vector = NULL;

    // Casting array of vectors to dynamic array type
    if((darray = dynamic_cast<const netdarray_t*>(type)) &&
            (vector = dynamic_cast<const netvector_t*>(darray->element_type()))) {
        PExpr::width_mode_t mode = PExpr::SIZED;
        unsigned use_wid = base_->test_width(des, scope, mode);
        NetExpr*base = base_->elaborate_expr(des, scope, use_wid, NO_FLAGS);

        assert(vector->packed_width() > 0);
        assert(base->expr_width() > 0);

        // Find rounded up length that can fit the whole casted array of vectors
        int len = base->expr_width() + vector->packed_width() - 1;
        if(base->expr_width() > vector->packed_width()) {
            len /= vector->packed_width();
        } else {
            len /= base->expr_width();
        }

        // Number of words in the created dynamic array
        NetEConst*len_expr = new NetEConst(verinum(len));
        return new NetENew(type, len_expr, base);
    }

    // Fallback
    return elaborate_expr(des, scope, (unsigned) 0, 0);
}

NetExpr* PECastType::elaborate_expr(Design*des, NetScope*scope,
				    unsigned, unsigned) const
{
      NetExpr*expr = base_->elaborate_expr(des, scope, base_->expr_width(), NO_FLAGS);

      if(dynamic_cast<const real_type_t*>(target_)) {
          return cast_to_real(expr);
      }

      if(const atom2_type_t*atom = dynamic_cast<const atom2_type_t*>(target_)) {
          if(base_->expr_width() > expr_width_) {
              cerr << get_fileline() << ": cast type is not wide enough to store the result." << endl;
              ivl_assert(*this, 0);
          }

          if(base_->has_sign() != atom->signed_flag) {
              cerr << get_fileline() << ": cast type and subject differ in signedness." << endl;
              ivl_assert(*this, 0);
          }

          // That is how you both resize & cast to integers
          return new NetECast('2', expr, expr_width_, expr->has_sign());
      }

      if(const vector_type_t*vec = dynamic_cast<const vector_type_t*>(target_)) {
          switch(vec->base_type) {
              case IVL_VT_BOOL:
                  return cast_to_int2(expr, expr_width_);

              case IVL_VT_LOGIC:
                  return cast_to_int4(expr, expr_width_);

              default:
                  break;  /* Suppress warnings */
          }
      }

      else if(dynamic_cast<const string_type_t*>(target_)) {
          if(base_->expr_type() == IVL_VT_STRING)
            return expr;        // no conversion

          if((base_->expr_type() != IVL_VT_BOOL) &&
                (base_->expr_type() != IVL_VT_LOGIC)) {
              cerr << get_fileline() << ": cannot be casted to string." << endl;
              ivl_assert(*this, false);
          }

          return expr;
      }

      cerr << get_fileline() << ": sorry: I don't know how to cast expression." << endl;
      ivl_assert(*this, false);

      return expr;
}

unsigned PEConcat::test_width(Design*des, NetScope*scope, width_mode_t&)
{
      expr_width_ = 0;
      enum {NO, MAYBE, YES} expr_is_string = MAYBE;
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
	      // Add in the width of this sub-expression.
	    expr_width_ += parms_[idx]->test_width(des, scope, width_modes_[idx]);

	      // If we already know this is not a string, then move on.
	    if (expr_is_string == NO)
		  continue;

	      // If this expression is a string, then the
	      // concatenation is a string until we find a reason to
	      // deny it.
	    if (parms_[idx]->expr_type()==IVL_VT_STRING) {
		  expr_is_string = YES;
		  continue;
	    }

	      // If this is a string literal, then this may yet be a string.
	    if (dynamic_cast<PEString*> (parms_[idx]))
		  continue;

	      // Failed to allow a string result.
	    expr_is_string = NO;
      }

      expr_type_   = (expr_is_string==YES)? IVL_VT_STRING : IVL_VT_LOGIC;
      signed_flag_ = false;

	// If there is a repeat expression, then evaluate the constant
	// value and set the repeat count.
      if (repeat_ && (scope != tested_scope_)) {
	    NetExpr*tmp = elab_and_eval(des, scope, repeat_, -1, true);
	    if (tmp == 0) return 0;

	    if (tmp->expr_type() == IVL_VT_REAL) {
		  cerr << tmp->get_fileline() << ": error: Concatenation "
		       << "repeat expression can not be REAL." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEConst*rep = dynamic_cast<NetEConst*>(tmp);

	    if (rep == 0) {
		  cerr << get_fileline() << ": error: "
			"Concatenation repeat expression is not constant."
		       << endl;
		  cerr << get_fileline() << ":      : The expression is: "
		       << *tmp << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (!rep->value().is_defined()) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be undefined (" << rep->value()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (rep->value().is_negative()) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be negative (" << rep->value().as_long()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

            repeat_count_ = rep->value().as_ulong();

            tested_scope_ = scope;
      }
      expr_width_ *= repeat_count_;
      min_width_   = expr_width_;

      return expr_width_;
}

// Keep track of the concatenation/repeat depth.
static int concat_depth = 0;

NetExpr* PEConcat::elaborate_expr(Design*, NetScope*,
				  ivl_type_t type, unsigned /*flags*/) const
{
      switch (type->base_type()) {
	  case IVL_VT_QUEUE:
	    if (parms_.size() == 0) {
		  NetENull*tmp = new NetENull;
		  tmp->set_line(*this);
		  return tmp;
	    }
	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "I don't know how to elaborate(ivl_type_t)"
		 << " this expression: " << *this << endl;
	    return 0;
      }
}

NetExpr* PEConcat::elaborate_expr(Design*des, NetScope*scope,
				  unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      concat_depth += 1;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate expr=" << *this
		 << ", expr_wid=" << expr_wid << endl;
      }

      if (repeat_count_ == 0 && concat_depth < 2) {
            cerr << get_fileline() << ": error: Concatenation repeat "
                 << "may not be zero in this context." << endl;
            des->errors += 1;
            concat_depth -= 1;
            return 0;
      }

      unsigned wid_sum = 0;
      unsigned parm_cnt = 0;
      unsigned parm_errors = 0;
      svector<NetExpr*> parms(parms_.size());

	/* Elaborate all the parameters and attach them to the concat node. */
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Missing expression "
		       << (idx+1) << " of concatenation list." << endl;
		  des->errors += 1;
		  continue;
	    }

	    assert(parms_[idx]);
            unsigned wid = parms_[idx]->expr_width();
	    NetExpr*ex = parms_[idx]->elaborate_expr(des, scope, wid, flags);
	    if (ex == 0) continue;

	    ex->set_line(*parms_[idx]);

            eval_expr(ex, -1);

	    if (ex->expr_type() == IVL_VT_REAL) {
		  cerr << ex->get_fileline() << ": error: "
		       << "Concatenation operand can not be real: "
		       << *parms_[idx] << endl;
		  des->errors += 1;
                  parm_errors += 1;
		  continue;
	    }

	    if (width_modes_[idx] != SIZED) {
		  cerr << ex->get_fileline() << ": error: "
		       << "Concatenation operand \"" << *parms_[idx]
		       << "\" has indefinite width." << endl;
		  des->errors += 1;
                  parm_errors += 1;
		  continue;
	    }

	      /* We are going to ignore zero width constants. */
	    if ((ex->expr_width() == 0) && dynamic_cast<NetEConst*>(ex)) {
		  parms[idx] = 0;
	    } else {
		  parms[idx] = ex;
		  parm_cnt += 1;
	    }
	    wid_sum += ex->expr_width();
      }
      if (parm_errors) {
	    concat_depth -= 1;
	    return 0;
      }

	/* Make the empty concat expression. */
      NetEConcat*concat = new NetEConcat(parm_cnt, repeat_count_, expr_type_);
      concat->set_line(*this);

	/* Remove any zero width constants. */
      unsigned off = 0;
      for (unsigned idx = 0 ;  idx < parm_cnt ;  idx += 1) {
	    while (parms[off+idx] == 0) off += 1;
	    concat->set(idx, parms[off+idx]);
      }

      if (wid_sum == 0) {
	    cerr << get_fileline() << ": error: Concatenation/replication "
	         << "may not have zero width in this context." << endl;
	    des->errors += 1;
	    concat_depth -= 1;
	    delete concat;
	    return 0;
      }

      NetExpr*tmp = pad_to_width(concat, expr_wid, *this);
      tmp->cast_signed(signed_flag_);

      concat_depth -= 1;
      return tmp;
}

/*
 * Floating point literals are not vectorable. It's not particularly
 * clear what to do about an actual width to return, but whatever the
 * width, it is unsigned.
 *
 * Absent any better idea, we call all real valued results a width of 1.
 */
unsigned PEFNumber::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_   = IVL_VT_REAL;
      expr_width_  = 1;
      min_width_   = 1;
      signed_flag_ = true;

      return expr_width_;
}

NetExpr* PEFNumber::elaborate_expr(Design*, NetScope*, ivl_type_t, unsigned) const
{
      NetECReal*tmp = new NetECReal(*value_);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEFNumber::elaborate_expr(Design*, NetScope*, unsigned, unsigned) const
{
      NetECReal*tmp = new NetECReal(*value_);
      tmp->set_line(*this);
      return tmp;
}

bool PEIdent::calculate_packed_indices_(Design*des, NetScope*scope, NetNet*net,
					list<long>&prefix_indices) const
{
      unsigned dimensions = net->unpacked_dimensions() + net->packed_dimensions();
      switch (net->data_type()) {
	  case IVL_VT_STRING:
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	    dimensions += 1;
	  default:
	    break;
      }
      if (path_.back().index.size() > dimensions) {
	    cerr << get_fileline() << ": error: the number of indices ("
		 << path_.back().index.size()
		 << ") is greater than the number of dimensions ("
		 << dimensions
		 << ")." << endl;
	    des->errors += 1;
	    return false;
      }

      list<index_component_t> index;
      index = path_.back().index;
      ivl_assert(*this, index.size() >= net->unpacked_dimensions());
      for (size_t idx = 0 ; idx < net->unpacked_dimensions() ; idx += 1)
	    index.pop_front();

      return evaluate_index_prefix(des, scope, prefix_indices, index);
}


bool PEIdent::calculate_bits_(Design*des, NetScope*scope,
			      long&msb, bool&defined) const
{
      defined = true;
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.sel == index_component_t::SEL_BIT);
      ivl_assert(*this, index_tail.msb && !index_tail.lsb);

	/* This handles bit selects. In this case, there in one
	   bit select expressions which must be constant. */

      NetExpr*msb_ex = elab_and_eval(des, scope, index_tail.msb, -1, true);
      NetEConst*msb_c = dynamic_cast<NetEConst*>(msb_ex);
      if (msb_c == 0) {
	    cerr << index_tail.msb->get_fileline() << ": error: "
		  "Bit select expressions must be constant."
		 << endl;
	    cerr << index_tail.msb->get_fileline() << ":      : "
                  "This msb expression violates the rule: "
                 << *index_tail.msb << endl;
	    des->errors += 1;
              /* Attempt to recover from error. */
            msb = 0;
      } else {
	    if (! msb_c->value().is_defined())
		  defined = false;
            msb = msb_c->value().as_long();
      }

      delete msb_ex;
      return true;
}

/*
 * Given that the msb_ and lsb_ are part select expressions, this
 * function calculates their values. Note that this method does *not*
 * convert the values to canonical form.
 */
bool PEIdent::calculate_parts_(Design*des, NetScope*scope,
			       long&msb, long&lsb, bool&defined) const
{
      defined = true;
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.sel == index_component_t::SEL_PART);
      ivl_assert(*this, index_tail.msb && index_tail.lsb);

	/* This handles part selects. In this case, there are
	   two bit select expressions, and both must be
	   constant. Evaluate them and pass the results back to
	   the caller. */
      NetExpr*lsb_ex = elab_and_eval(des, scope, index_tail.lsb, -1, true);
      NetEConst*lsb_c = dynamic_cast<NetEConst*>(lsb_ex);
      if (lsb_c == 0) {
	    cerr << index_tail.lsb->get_fileline() << ": error: "
		  "Part select expressions must be constant."
		 << endl;
	    cerr << index_tail.lsb->get_fileline() << ":      : "
		  "This lsb expression violates the rule: "
		 << *index_tail.lsb << endl;
	    des->errors += 1;
              /* Attempt to recover from error. */
            lsb = 0;
      } else {
	    if (! lsb_c->value().is_defined())
		  defined = false;
            lsb = lsb_c->value().as_long();
      }

      NetExpr*msb_ex = elab_and_eval(des, scope, index_tail.msb, -1, true);
      NetEConst*msb_c = dynamic_cast<NetEConst*>(msb_ex);
      if (msb_c == 0) {
	    cerr << index_tail.msb->get_fileline() << ": error: "
		  "Part select expressions must be constant."
		 << endl;
	    cerr << index_tail.msb->get_fileline() << ":      : "
                  "This msb expression violates the rule: "
                 << *index_tail.msb << endl;
	    des->errors += 1;
              /* Attempt to recover from error. */
            msb = lsb;
      } else {
	    if (! msb_c->value().is_defined())
		  defined = false;
            msb = msb_c->value().as_long();
      }

      delete msb_ex;
      delete lsb_ex;
      return true;
}

bool PEIdent::calculate_up_do_width_(Design*des, NetScope*scope,
				     unsigned long&wid) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.lsb && index_tail.msb);

      bool flag = true;

	/* Calculate the width expression (in the lsb_ position)
	   first. If the expression is not constant, error but guess 1
	   so we can keep going and find more errors. */
      NetExpr*wid_ex = elab_and_eval(des, scope, index_tail.lsb, -1, true);
      NetEConst*wid_c = dynamic_cast<NetEConst*>(wid_ex);

      wid = wid_c? wid_c->value().as_ulong() : 0;
      if (wid == 0) {
	    cerr << index_tail.lsb->get_fileline() << ": error: "
		  "Indexed part widths must be constant and greater than zero."
		 << endl;
	    cerr << index_tail.lsb->get_fileline() << ":      : "
		  "This part width expression violates the rule: "
		 << *index_tail.lsb << endl;
	    des->errors += 1;
	    flag = false;
	    wid = 1;
      }
      delete wid_ex;

      return flag;
}

/*
 * When we know that this is an indexed part select (up or down) this
 * method calculates the up/down base, as far at it can be calculated.
 */
NetExpr* PEIdent::calculate_up_do_base_(Design*des, NetScope*scope,
                                        bool need_const) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.lsb != 0);
      ivl_assert(*this, index_tail.msb != 0);

      NetExpr*tmp = elab_and_eval(des, scope, index_tail.msb, -1, need_const);
      return tmp;
}

bool PEIdent::calculate_param_range_(Design*, NetScope*,
				     const NetExpr*par_msb, long&par_msv,
				     const NetExpr*par_lsb, long&par_lsv,
				     long length) const
{
      if (par_msb == 0) {
	      // If the parameter doesn't have an explicit range, then
	      // just return range values of [length-1:0].
	    ivl_assert(*this, par_lsb == 0);
	    par_msv = length-1;
	    par_lsv = 0;
	    return true;
      }

      const NetEConst*tmp = dynamic_cast<const NetEConst*> (par_msb);
      ivl_assert(*this, tmp);

      par_msv = tmp->value().as_long();

      tmp = dynamic_cast<const NetEConst*> (par_lsb);
      ivl_assert(*this, tmp);

      par_lsv = tmp->value().as_long();

      return true;
}

unsigned PEIdent::test_width_method_(Design*des, NetScope*scope, width_mode_t&)
{
      if (!gn_system_verilog())
	    return 0;
      if (path_.size() < 2)
	    return 0;

      pform_name_t use_path = path_;
      perm_string member_name = peek_tail_name(path_);
      use_path.pop_back();

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::test_width_method_: "
		 << "Try to find method=" << member_name
		 << " of signal " << use_path << endl;
      }

      NetNet*net = 0;
      const NetExpr*par = 0;
      NetEvent*eve = 0;
      const NetExpr*ex1 = 0, *ex2 = 0;
      symbol_search(this, des, scope, use_path, net, par, eve, ex1, ex2);
      if (net == 0) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": PEIdent::test_width_method_: "
		       << "Only nets can have methods, so give up here." << endl;
	    return 0;
      }

      if (/*const netdarray_t*dtype =*/ net->darray_type()) {
	    if (member_name == "size") {
		  expr_type_  = IVL_VT_BOOL;
		  expr_width_ = 32;
		  min_width_  = 32;
		  signed_flag_= true;
		  return 32;
	    }
      }

      return 0;
}

unsigned PEIdent::test_width(Design*des, NetScope*scope, width_mode_t&mode)
{
      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*ex1, *ex2;

      NetScope*use_scope = scope;
      if (package_) {
	    use_scope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, use_scope);
      }

      if (unsigned tmp = test_width_method_(des, scope, mode)) {
	    return tmp;
      }

      NetScope*found_in = symbol_search(this, des, use_scope, path_,
					net, par, eve,
                                        ex1, ex2);

	// If there is a part/bit select expression, then process it
	// here. This constrains the results no matter what kind the
	// name is.

      const name_component_t&name_tail = path_.back();
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty()) {
	    const index_component_t&index_tail = name_tail.index.back();
	      // Skip full array word net selects.
	    if (!net || (name_tail.index.size() > net->unpacked_dimensions())) {
		  use_sel = index_tail.sel;
	    }
      }

      unsigned use_width = UINT_MAX;
      switch (use_sel) {
	  case index_component_t::SEL_NONE:
	    break;
	  case index_component_t::SEL_PART:
	      { long msb, lsb;
		bool parts_defined;
		calculate_parts_(des, scope, msb, lsb, parts_defined);
		if (parts_defined)
		      use_width = 1 + ((msb>lsb)? (msb-lsb) : (lsb-msb));
		else
		      use_width = UINT_MAX;
		break;
	      }
	  case index_component_t::SEL_IDX_UP:
	  case index_component_t::SEL_IDX_DO:
	      { unsigned long tmp = 0;
		calculate_up_do_width_(des, scope, tmp);
		use_width = tmp;
		break;
	      }
	  case index_component_t::SEL_BIT:
	      { ivl_assert(*this, !name_tail.index.empty());
		const index_component_t&index_tail = name_tail.index.back();
		ivl_assert(*this, index_tail.msb);
	      }
		// If we have a net in hand, then we can predict what the
		// slice width will be. If not, then assume it will be a
		// simple bit select. If the net only has a single dimension
		// then this is still a simple bit select.
	      if ((net == 0) || (net->packed_dimensions() <= 1))
		    use_width = 1;
	      break;
	  case index_component_t::SEL_BIT_LAST:
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::test_width: "
		       << "Queue/Darray last index ($)" << endl;
	    }
	    break;
	  default:
	    ivl_assert(*this, 0);
      }

      if (const netdarray_t*darray = net? net->darray_type() : 0) {
	    switch (use_sel) {
		case index_component_t::SEL_BIT:
		case index_component_t::SEL_BIT_LAST:
		  expr_type_   = darray->element_base_type();
		  expr_width_  = darray->element_width();
		  min_width_   = expr_width_;
		  signed_flag_ = net->get_signed();
		  break;
		default:
		  expr_type_   = net->data_type();
		  expr_width_  = net->vector_width();
		  min_width_   = expr_width_;
		  signed_flag_ = net->get_signed();
		  break;
	    }
	    return expr_width_;
      }

      if (use_width != UINT_MAX) {
	    expr_type_   = IVL_VT_LOGIC; // Assume bit/parts selects are logic
	    expr_width_  = use_width;
	    min_width_   = use_width;
            signed_flag_ = false;

	    return expr_width_;
      }

	// The width of a signal expression is the width of the signal.
      if (net != 0) {
	    size_t use_depth = name_tail.index.size();
	      // Account for unpacked dimensions by assuming that the
	      // unpacked dimensions are consumed first, so subtract
	      // the unpacked dimensions from the dimension depth
	      // useable for making the slice.
	    if (use_depth >= net->unpacked_dimensions()) {
		  use_depth -= net->unpacked_dimensions();

	    } else {
		    // In this case, we have a slice of an unpacked
		    // array. This likely handled as an array instead
		    // of a slice. Hmm...
		  use_depth = 0;
	    }

	    expr_type_   = net->data_type();
	    expr_width_  = net->slice_width(use_depth);
	    min_width_   = expr_width_;
	    signed_flag_ = net->get_signed();
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::test_width: "
		       << net->name() << " is a net, "
		       << "type=" << expr_type_
		       << ", width=" << expr_width_
		       << ", signed_=" << (signed_flag_?"true":"false")
		       << ", use_depth=" << use_depth
		       << ", packed_dimensions=" << net->packed_dimensions()
		       << ", unpacked_dimensions=" << net->unpacked_dimensions()
		       << endl;
	    }
	    return expr_width_;
      }

	// The width of an enumeration literal is the width of the
	// enumeration base.
      if (const NetEConstEnum*par_enum = dynamic_cast<const NetEConstEnum*> (par)) {
	    const netenum_t*use_enum = par_enum->enumeration();
	    ivl_assert(*this, use_enum != 0);

	    expr_type_   = use_enum->base_type();
	    expr_width_  = use_enum->packed_width();
	    min_width_   = expr_width_;
	    signed_flag_ = par_enum->has_sign();

	    return expr_width_;
      }

	// The width of a parameter is the width of the parameter value
        // (as evaluated earlier).
      if (par != 0) {
	    expr_type_   = par->expr_type();
	    expr_width_  = par->expr_width();
            min_width_   = expr_width_;
            signed_flag_ = par->has_sign();

            if (!par->has_width() && (mode < LOSSLESS))
                  mode = LOSSLESS;

	    return expr_width_;
      }

      if (path_.size() == 1
	  && scope->genvar_tmp.str()
	  && strcmp(peek_tail_name(path_), scope->genvar_tmp) == 0) {
	    verinum val (scope->genvar_tmp_val);
            expr_type_   = IVL_VT_BOOL;
            expr_width_  = val.len();
            min_width_   = expr_width_;
            signed_flag_ = true;

            if (gn_strict_expr_width_flag) {
                  expr_width_ = integer_width;
                  mode = UNSIZED;
            } else if (mode < LOSSLESS) {
                  mode = LOSSLESS;
            }

            return expr_width_;
      }

	// If this is SystemVerilog then maybe this is a structure element.
      if (gn_system_verilog() && found_in==0 && path_.size() >= 2) {
	    pform_name_t use_path = path_;
	    perm_string method_name = peek_tail_name(use_path);
	    use_path.pop_back();

	    ivl_assert(*this, net == 0);
	    symbol_search(this, des, scope, use_path, net, par, eve, ex1, ex2);

	      // Check to see if we have a net and if so is it a structure?
	    if (net != 0) {
		    // If this net is a struct, the method name may be
		    // a struct member. If it is, then we know the
		    // width of this identifier my knowing the width
		    // of the member. We don't even need to know
		    // anything about positions in containing arrays.
		  if (net->struct_type() != 0) {

			if (debug_elaborate) {
			      cerr << get_fileline() << ": debug: PEIdent::test_width: "
				   << "Net " << use_path << " is a struct, "
				   << "checking width of member " << method_name << endl;
			}

			const netstruct_t::member_t*mem;
			unsigned long unused;
			mem = get_struct_member(this, des, scope, net,
			                        method_name, unused);
			if (mem) {
			      expr_type_   = mem->data_type();
			      expr_width_  = mem->net_type->packed_width();
			      min_width_   = expr_width_;
			      signed_flag_ = mem->get_signed();
			      return expr_width_;
			}
		  }

		  if (const netclass_t*class_type = net->class_type()) {
			int pidx = class_type->property_idx_from_name(method_name);
			if (pidx >= 0) {
			      ivl_type_t ptype = class_type->get_prop_type(pidx);
			      expr_type_   = ptype->base_type();
			      expr_width_  = ptype->packed_width();
			      min_width_   = expr_width_;
			      signed_flag_ = ptype->get_signed();
			      return expr_width_;
			}
		  }
	    }
      }

	// Not a net, and not a parameter? Give up on the type, but
	// set the width to 0.
      expr_type_   = IVL_VT_NO_TYPE;
      expr_width_  = 0;
      min_width_   = 0;
      signed_flag_ = false;

      return expr_width_;
}


NetExpr* PEIdent::elaborate_expr(Design*des, NetScope*scope,
				 ivl_type_t ntype, unsigned flags) const
{
      bool need_const = NEED_CONST & flags;

      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;
      const NetExpr*ex1, *ex2;

      NetScope*use_scope = scope;
      if (package_) {
	    use_scope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, use_scope);
      }

      if (NetExpr* tmp = elaborate_expr_class_member_(des, scope, 0, flags)) {
	    return tmp;
      }

      /* NetScope*found_in = */ symbol_search(this, des, use_scope, path_,
					      net, par, eve,
					      ex1, ex2);

      if (net == 0 && gn_system_verilog() && path_.size() >= 2) {
	    pform_name_t use_path = path_;
	    name_component_t member_comp = use_path.back();
	    use_path.pop_back();

	    ivl_assert(*this, net == 0);
	    symbol_search(this, des, use_scope, use_path, net, par, eve, ex1, ex2);

	    if (net == 0) {
		    // Nope, no struct/class with member.

	    } else if (net->struct_type() != 0) {
		  return check_for_struct_members(this, des, use_scope,
						  net, use_path.back().index,
						  member_comp);

	    } else if (net->class_type()!=0) {
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PEIdent::elaborate_expr: "
			     << "Ident " << use_path
			     << " look for property " << member_comp << endl;
		  }

		  return check_for_class_property(this, des, scope,
						  net, member_comp);
	    }
      }

      if (net == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "Expecting idents with ntype to be signals." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (! ntype->type_compatible(net->net_type())) {
	    cerr << get_fileline() << ": internal_error: "
		 << "net type doesn't match context type." << endl;

	    cerr << get_fileline() << ":               : "
		 << "net type=";
	    if (net->net_type())
		  net->net_type()->debug_dump(cerr);
	    else
		  cerr << "<nil>";
	    cerr << endl;

	    cerr << get_fileline() << ":               : "
		 << "context type=";
	    ivl_assert(*this, ntype);
	    ntype->debug_dump(cerr);
	    cerr << endl;
      }
      ivl_assert(*this, ntype->type_compatible(net->net_type()));

      const name_component_t&use_comp = path_.back();

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_expr: "
		 << "Typed ident " << net->name()
		 << " with " << use_comp.index.size() << " indices"
		 << " and " << net->unpacked_dimensions() << " expected."
		 << endl;
      }

      if (net->unpacked_dimensions() != use_comp.index.size()) {
	    cerr << get_fileline() << ": sorry: "
		 << "Net " << net->name()
		 << " expects " << net->unpacked_dimensions()
		 << ", but got " << use_comp.index.size() << "."
		 << endl;
	    des->errors += 1;

	    NetESignal*tmp = new NetESignal(net);
	    tmp->set_line(*this);
	    return tmp;
      }

      if (net->unpacked_dimensions() == 0) {
	    NetESignal*tmp = new NetESignal(net);
	    tmp->set_line(*this);
	    return tmp;
      }

	// Convert a set of index expressions to a single expression
	// that addresses the canonical element.
      list<NetExpr*>unpacked_indices;
      list<long> unpacked_indices_const;
      indices_flags idx_flags;
      indices_to_expressions(des, scope, this,
			     use_comp.index, net->unpacked_dimensions(),
			     need_const,
			     idx_flags,
			     unpacked_indices,
			     unpacked_indices_const);

      NetExpr*canon_index = 0;

      if (idx_flags.invalid) {
	      // Nothing to do

      } else if (idx_flags.undefined) {
	    cerr << get_fileline() << ": warning: "
		 << "returning 'bx for undefined array access "
		 << net->name() << as_indices(unpacked_indices)
		 << "." << endl;

      } else if (idx_flags.variable) {
	    ivl_assert(*this, unpacked_indices.size() == net->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(net, unpacked_indices);

      } else {
	    ivl_assert(*this, unpacked_indices_const.size() == net->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(net, unpacked_indices_const);
      }

      ivl_assert(*this, canon_index);
      NetESignal*tmp = new NetESignal(net, canon_index);
      tmp->set_line(*this);

      return tmp;
}

/*
 * Guess that the path_ is the name of a member of a containing class,
 * and see how that works. If it turns out that the current scope is
 * not a method, or the name is not in the parent class, then
 * fail. Otherwise, return a NetEProperty.
 */
NetExpr* PEIdent::elaborate_expr_class_member_(Design*des, NetScope*scope,
					       unsigned, unsigned) const
{
      if (!gn_system_verilog())
	    return 0;
      if (scope->parent() == 0)
	    return 0;
      if (path_.size() != 1)
	    return 0;

      const netclass_t*class_type = find_class_containing_scope(*this, scope);
      if (class_type == 0)
	    return 0;

      const name_component_t&name_comp = path_.back();

      perm_string member_name = name_comp.name;
      int pidx = class_type->property_idx_from_name(member_name);
      if (pidx < 0)
	    return 0;

      NetScope*scope_method = find_method_containing_scope(*this, scope);
      ivl_assert(*this, scope_method);

      NetNet*this_net = scope_method->find_signal(perm_string::literal("@"));
      if (this_net == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "Unable to find 'this' port of " << scope_path(scope_method)
		 << "." << endl;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_expr_class_member: "
		 << "Found member " << member_name
		 << " is a member of class " << class_type->get_name()
		 << ", context scope=" << scope_path(scope)
		 << ", type=" << *class_type->get_prop_type(pidx)
		 << ", so making a NetEProperty." << endl;
      }

      property_qualifier_t qual = class_type->get_prop_qual(pidx);
      if (qual.test_local() && ! class_type->test_scope_is_method(scope)) {
	    cerr << get_fileline() << ": error: "
		 << "Local property " << class_type->get_prop_name(pidx)
		 << " is not accessible in this context."
		 << " (scope=" << scope_path(scope) << ")" << endl;
	    des->errors += 1;
      }

      if (qual.test_static()) {
	    return class_static_property_expression(this, class_type, member_name);
      }

      NetExpr*canon_index = 0;
      ivl_type_t tmp_type = class_type->get_prop_type(pidx);
      if (const netuarray_t*tmp_ua = dynamic_cast<const netuarray_t*>(tmp_type)) {

	    const std::vector<netrange_t>&dims = tmp_ua->static_dimensions();

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_expr_class_member_: "
		       << "Property " << class_type->get_prop_name(pidx)
		       << " has " << dims.size() << " dimensions, "
		       << " got " << name_comp.index.size() << " indices." << endl;
	    }

	    if (dims.size() != name_comp.index.size()) {
		  cerr << get_fileline() << ": error: "
		       << "Got " << name_comp.index.size() << " indices, "
		       << "expecting " << dims.size()
		       << " to index the property " << class_type->get_prop_name(pidx) << "." << endl;
		  des->errors += 1;

	    } else {

		  canon_index = make_canonical_index(des, scope, this,
						     name_comp.index, tmp_ua, false);
	    }
      }

      if (debug_elaborate && canon_index) {
	    cerr << get_fileline() << ": PEIdent::elaborate_expr_class_member_: "
		 << "Property " << class_type->get_prop_name(pidx)
		 << " canonical index: " << *canon_index << endl;
      }

      NetEProperty*tmp = new NetEProperty(this_net, member_name, canon_index);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEIdent::elaborate_expr_method_(Design*des, NetScope*scope,
					 unsigned, unsigned) const
{
      if (!gn_system_verilog())
	    return 0;
      if (path_.size() < 2)
	    return 0;

      pform_name_t use_path = path_;
      perm_string member_name = peek_tail_name(path_);
      use_path.pop_back();

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_expr_method_: "
		 << "Try to find method=" << member_name
		 << " of signal " << use_path << endl;
      }

      NetNet*net = 0;
      const NetExpr*par = 0;
      NetEvent*eve = 0;
      const NetExpr*ex1 = 0, *ex2 = 0;
      symbol_search(this, des, scope, use_path, net, par, eve, ex1, ex2);
      if (net == 0) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": PEIdent::elaborate_expr_method_: "
		       << "Only nets can have methods, so give up here." << endl;
	    return 0;
      }

      if (net->darray_type()) {
	    if (member_name == "size") {
		  NetESFunc*fun = new NetESFunc("$size", IVL_VT_BOOL, 32, 1);
		  fun->set_line(*this);

		  NetESignal*arg = new NetESignal(net);
		  arg->set_line(*net);

		  fun->parm(0, arg);
		  return fun;
	    }

	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PEIdent::elaborate_expr_method_: "
		 << "Give up trying to find method " << member_name
		 << " of " << path_ << "." << endl;
      }

      return 0;
}

/*
 * Elaborate an identifier in an expression. The identifier can be a
 * parameter name, a signal name or a memory name. It can also be a
 * scope name (Return a NetEScope) but only certain callers can use
 * scope names. However, we still support it here.
 *
 * Function names are not handled here, they are detected by the
 * parser and are elaborated by PECallFunction.
 *
 * The signal name may be escaped, but that affects nothing here.
 */
NetExpr* PEIdent::elaborate_expr(Design*des, NetScope*scope,
				 unsigned expr_wid, unsigned flags) const
{
      assert(scope);

      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*ex1, *ex2;

	// Special case: Detect the special situation that this name
	// is the name of a variable in the class, and this is a class
	// method. We sense that this might be the case by noting that
	// the parent scope of where we are working is a
	// NetScope::CLASS, the path_ is a single component, and the
	// name is a property of the class. If that turns out to be
	// the case, then handle this specially.
      if (NetExpr*tmp = elaborate_expr_class_member_(des, scope, expr_wid, flags)) {
	    return tmp;
      }

      if (path_.size() > 1) {
            if (NEED_CONST & flags) {
                  cerr << get_fileline() << ": error: A hierarchical reference"
                          " (`" << path_ << "') is not allowed in a constant"
                          " expression." << endl;
                  des->errors += 1;
                  return 0;
            }
            if (scope->need_const_func()) {
                  cerr << get_fileline() << ": error: A hierarchical reference"
                          " (`" << path_ << "') is not allowed in a constant"
                          " function." << endl;
                  des->errors += 1;
                  return 0;
            }
            scope->is_const_func(false);
      }

      if (debug_elaborate)
	    cerr << get_fileline() << ": PEIdent::elaborate_expr: path_=" << path_ << endl;

      NetScope*use_scope = scope;
      if (package_) {
	    use_scope = des->find_package(package_->pscope_name());
	    ivl_assert(*this, use_scope);
      }

	// Special case: Detect the special situation that the name is
	// a method of an object (including built-in methods) that has
	// no arguments. For example, "foo.size" is the call to the
	// size() method if foo is an array type.
      if (NetExpr*tmp = elaborate_expr_method_(des, scope, expr_wid, flags)) {
	    return tmp;
      }

      NetScope*found_in = symbol_search(this, des, use_scope, path_,
					net, par, eve,
					ex1, ex2);

	// If the identifier name is a parameter name, then return
	// the parameter value.
      if (par != 0) {
	    NetExpr*tmp = elaborate_expr_param_(des, scope, par, found_in,
                                                ex1, ex2, expr_wid, flags);

            if (!tmp) return 0;

            tmp = pad_to_width(tmp, expr_wid, *this);
            tmp->cast_signed(signed_flag_);

            return tmp;
      }

	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (net != 0) {
            if (NEED_CONST & flags) {
                  cerr << get_fileline() << ": error: A reference to a wire "
                          "or reg (`" << path_ << "') is not allowed in "
                          "a constant expression." << endl;
	          des->errors += 1;
                  return 0;
            }
            if (net->scope()->type() == NetScope::MODULE) {
                  if (scope->need_const_func()) {
                        cerr << get_fileline() << ": error: A reference to a "
                                "non-local wire or reg (`" << path_ << "') is "
                                "not allowed in a constant function." << endl;
                        des->errors += 1;
                        return 0;
                  }
                  scope->is_const_func(false);
            }

	    NetExpr*tmp = elaborate_expr_net(des, scope, net, found_in,
                                             expr_wid, flags);

            if (!tmp) return 0;

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_expr: "
		       << "Expression as net. expr_wid=" << expr_wid
		       << ", tmp->expr_width()=" << tmp->expr_width()
		       << ", tmp=" << *tmp << endl;
	    }

            tmp = pad_to_width(tmp, expr_wid, *this);
            tmp->cast_signed(signed_flag_);

            return tmp;
      }

	// If the identifier is a named event
        // then create a NetEEvent node to handle it.
      if (eve != 0) {
            if (NEED_CONST & flags) {
                  cerr << get_fileline() << ": error: A reference to a named "
                          "event (`" << path_ << "') is not allowed in a "
                          "constant expression." << endl;
	          des->errors += 1;
                  return 0;
            }
            if (eve->scope() != scope) {
                  if (scope->need_const_func()) {
                        cerr << get_fileline() << ": error: A reference to a "
                                "non-local named event (`" << path_ << "') is "
                                "not allowed in a constant function." << endl;
                        des->errors += 1;
                        return 0;
                  }
                  scope->is_const_func(false);
            }

	    NetEEvent*tmp = new NetEEvent(eve);
	    tmp->set_line(*this);
	    return tmp;
      }

	// Hmm... maybe this is a genvar? This is only possible while
	// processing generate blocks, but then the genvar_tmp will be
	// set in the scope.
      if (path_.size() == 1
	  && scope->genvar_tmp.str()
	  && strcmp(peek_tail_name(path_), scope->genvar_tmp) == 0) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: " << path_
		       << " is genvar with value " << scope->genvar_tmp_val
		       << "." << endl;
	    verinum val (scope->genvar_tmp_val, expr_wid);
	    val.has_sign(true);
	    NetEConst*tmp = new NetEConst(val);
	    tmp->set_line(*this);
	    return tmp;
      }

	// Maybe this is a method attached to an enumeration name? If
	// this is SystemVerilog, then test to see if the name is
	// really a method attached to an object.
      if (gn_system_verilog() && found_in==0 && path_.size() >= 2) {
	    pform_name_t use_path = path_;
	    name_component_t member_comp = use_path.back();
	    use_path.pop_back();

	    if (debug_elaborate)
		  cerr << get_fileline() << ": PEIdent::elaborate_expr: "
		       << "Look for base_path " << use_path
		       << " for member " << member_comp << "." << endl;

	    ivl_assert(*this, net == 0);
	    symbol_search(this, des, use_scope, use_path, net, par, eve, ex1, ex2);

	      // Check to see if we have a net and if so is it an
	      // enumeration? If so then check to see if this is an
	      // enumeration method call.
	    if (net != 0) {
		    // If this net is actually an enum, the method may
		    // be an enumeration method.
		  if (const netenum_t*netenum = net->enumeration()) {
			  // We may need the net expression for the
			  // enumeration variable so get it.
			NetESignal*expr = new NetESignal(net);
			expr->set_line(*this);
			  // This expression cannot be a select!
			assert(use_path.back().index.empty());

			return check_for_enum_methods(this, des, use_scope,
			                              netenum,
			                              use_path, member_comp.name,
			                              expr, expr_wid, NULL, 0);
		  }

		    // If this net is a struct, the method name may be
		    // a struct member.
		  if (net->struct_type() != 0) {
			if (debug_elaborate) {
			      cerr << get_fileline() << ": debug: "
				   << "PEIdent::elaborate_expr: "
				   << "Ident " << use_path
				   << " is a struct."
				   << " Expecting " << net->packed_dims().size()
				   << "-1 dimensions, "
				   << "got " << use_path.back().index.size() << "." << endl;
			}

			NetExpr*tmp = check_for_struct_members(this, des, use_scope,
							       net, use_path.back().index,
							       member_comp);
			if (!tmp) return 0;

			tmp = pad_to_width(tmp, expr_wid, *this);
			tmp->cast_signed(signed_flag_);
			return tmp;
		  }

		  if (net->class_type() != 0) {
			if (debug_elaborate) {
			      cerr << get_fileline() << ": PEIdent::elaborate_expr: "
				   << "Ident " << use_path
				   << " look for property " << member_comp << endl;
			}

			return check_for_class_property(this, des, use_scope,
							net, member_comp);
		  }
	    }
      }

	// At this point we've exhausted all the possibilities that
	// are not scopes. If this is not a system task argument, then
	// it cannot be a scope name, so give up.

      if ( !(SYS_TASK_ARG & flags) ) {
	      // I cannot interpret this identifier. Error message.
            cerr << get_fileline() << ": error: Unable to bind "
                 << (NEED_CONST & flags ? "parameter" : "wire/reg/memory")
                 << " `" << path_ << "' in `" << scope_path(scope) << "'"
                 << endl;
            if (scope->need_const_func()) {
                  cerr << get_fileline() << ":      : `" << scope->basename()
                       << "' is being used as a constant function, so may "
                          "only reference local variables." << endl;
            }
	    des->errors += 1;
	    return 0;
      }

	// Finally, if this is a scope name, then return that. Look
	// first to see if this is a name of a local scope. Failing
	// that, search globally for a hierarchical name.
      if ((path_.size() == 1)) {
	    hname_t use_name ( peek_tail_name(path_) );
	    if (NetScope*nsc = scope->child(use_name)) {
		  NetEScope*tmp = new NetEScope(nsc);
		  tmp->set_line(*this);

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: Found scope "
			     << use_name << " in scope " << scope->basename()
			     << endl;

		  return tmp;
	    }
      }

      list<hname_t> spath = eval_scope_path(des, scope, path_);

      ivl_assert(*this, spath.size() == path_.size());

	// Try full hierarchical scope name.
      if (NetScope*nsc = des->find_scope(spath)) {
	    NetEScope*tmp = new NetEScope(nsc);
	    tmp->set_line(*this);

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Found scope "
		       << nsc->basename()
		       << " path=" << path_ << endl;

	    if ( !(SYS_TASK_ARG & flags) ) {
		  cerr << get_fileline() << ": error: Scope name "
		       << nsc->basename() << " not allowed here." << endl;
		  des->errors += 1;
	    }

	    return tmp;
      }

	// Try relative scope name.
      if (NetScope*nsc = des->find_scope(scope, spath)) {
	    NetEScope*tmp = new NetEScope(nsc);
	    tmp->set_line(*this);

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Found scope "
		       << nsc->basename() << " in " << scope_path(scope) << endl;

	    return tmp;
      }

	// I cannot interpret this identifier. Error message.
      cerr << get_fileline() << ": error: Unable to bind wire/reg/memory "
	    "`" << path_ << "' in `" << scope_path(scope) << "'" << endl;
      des->errors += 1;
      return 0;
}

static verinum param_part_select_bits(const verinum&par_val, long wid,
				     long lsv)
{
      verinum result (verinum::Vx, wid, true);

      for (long idx = 0 ; idx < wid ; idx += 1) {
	    long off = idx + lsv;
	    if (off < 0)
		  continue;
	    else if (off < (long)par_val.len())
		  result.set(idx, par_val.get(off));
	    else if (par_val.is_string()) // Pad strings with nulls.
		  result.set(idx, verinum::V0);
	    else if (par_val.has_len()) // Pad sized parameters with X
		  continue;
	    else // Unsized parameters are "infinite" width.
		  result.set(idx, sign_bit(par_val));
      }

	// If the input is a string, and the part select is working on
	// byte boundaries, then make the result into a string.
      if (par_val.is_string() && (labs(lsv)%8 == 0) && (wid%8 == 0))
	    return result.as_string();

      return result;
}

NetExpr* PEIdent::elaborate_expr_param_bit_(Design*des, NetScope*scope,
					    const NetExpr*par,
					    NetScope*found_in,
					    const NetExpr*par_msb,
					    const NetExpr*par_lsb,
                                            bool need_const) const
{
      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);

      long par_msv, par_lsv;
      if(! calculate_param_range_(des, scope, par_msb, par_msv,
                                  par_lsb, par_lsv,
                                  par_ex->value().len())) return 0;

      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());
      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb);
      ivl_assert(*this, !index_tail.lsb);

      NetExpr*sel = elab_and_eval(des, scope, index_tail.msb, -1, need_const);
      if (sel == 0) return 0;

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Calculate bit select "
		 << "[" << *sel << "] from range "
		 << "[" << par_msv << ":" << par_lsv << "]." << endl;

      perm_string name = peek_tail_name(path_);

	// Handle the special case that the selection is constant. In this
	// case, just precalculate the entire constant result.
      if (NetEConst*sel_c = dynamic_cast<NetEConst*> (sel)) {
	      // Special case: If the bit select is constant and not fully
	      // defined, then we know that the result must be 1'bx.
	    if (! sel_c->value().is_defined()) {
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: "
			        "Constant undefined bit select ["
			     << sel_c->value() << "] for parameter '"
			     << name << "'." << endl;
			cerr << get_fileline() << ":        : "
			        "Replacing select with a constant 1'bx."
			     << endl;
		  }
		  NetEConst*res = make_const_x(1);
		  res->set_line(*this);
		  return res;
	    }
	      // Calculate the canonical index value.
	    long sel_v = sel_c->value().as_long();
	    if (par_msv >= par_lsv) sel_v -= par_lsv;
	    else sel_v = par_lsv - sel_v;

	      // Select a bit from the parameter.
	    verinum par_v = par_ex->value();
	    verinum::V rtn = verinum::Vx;

	      // A constant in range select.
	    if ((sel_v >= 0) && ((unsigned long) sel_v < par_v.len())) {
		  rtn = par_v[sel_v];
	      // An unsized after select.
	    } else if ((sel_v >= 0) && (! par_v.has_len())) {
		  if (par_v.has_sign()) rtn = par_v[par_v.len()-1];
		  else rtn = verinum::V0;
	    } else if (warn_ob_select) {
		  cerr << get_fileline() << ": warning: "
		          "Constant bit select [" << sel_c->value().as_long()
		       << "] is ";
		  if (sel_v < 0) cerr << "before ";
		  else cerr << "after ";
		  cerr << name << "[";
		  if (par_v.has_len()) cerr << par_msv;
		  else cerr << "<inf>";
		  cerr << ":" << par_lsv << "]." << endl;
		  cerr << get_fileline() << ":        : "
		          "Replacing select with a constant 1'bx." << endl;
	    }
	    NetEConst*res = new NetEConst(verinum(rtn, 1));
	    res->set_line(*this);
	    return res;
      }

      sel = normalize_variable_base(sel, par_msv, par_lsv, 1, true);

	/* Create a parameter reference for the variable select. */
      NetEConstParam*ptmp = new NetEConstParam(found_in, name, par_ex->value());
      NetScope::param_ref_t pref = found_in->find_parameter(name);
      ptmp->set_line((*pref).second);

      NetExpr*tmp = new NetESelect(ptmp, sel, 1);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEIdent::elaborate_expr_param_part_(Design*des, NetScope*scope,
					     const NetExpr*par,
					     NetScope*,
					     const NetExpr*par_msb,
					     const NetExpr*par_lsb,
                                             unsigned expr_wid) const
{
      long msv, lsv;
      bool parts_defined_flag;
      bool flag = calculate_parts_(des, scope, msv, lsv, parts_defined_flag);
      if (!flag)
	    return 0;

      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);


      long par_msv, par_lsv;
      if (! calculate_param_range_(des, scope, par_msb, par_msv,
                                   par_lsb, par_lsv,
                                   par_ex->value().len())) return 0;

      if (! parts_defined_flag) {
	    if (warn_ob_select) {
		  const index_component_t&psel = path_.back().index.back();
		  perm_string name = peek_tail_name(path_);
		  cerr << get_fileline() << ": warning: "
		          "Undefined part select [" << *(psel.msb) << ":"
		       << *(psel.lsb) << "] for parameter '" << name
		       << "'." << endl;
		  cerr << get_fileline() << ":        : "
		          "Replacing select with a constant 'bx." << endl;
	    }

	    verinum val(verinum::Vx, expr_wid, true);
	    NetEConst*tmp = new NetEConst(val);
	    tmp->set_line(*this);
	    return tmp;
      }

	// Notice that the par_msv is not used in this function other
	// than for this test. It is used to tell the direction that
	// the bits are numbers, so that we can make sure the
	// direction matches the part select direction. After that,
	// we only need the par_lsv.
      if ((msv>lsv && par_msv<par_lsv) || (msv<lsv && par_msv>=par_lsv)) {
	    perm_string name = peek_tail_name(path_);
	    cerr << get_fileline() << ": error: Part select " << name
		 << "[" << msv << ":" << lsv << "] is out of order." << endl;
	    des->errors += 1;
	    return 0;
      }

      long wid = 1 + labs(msv-lsv);

	// Watch out for reversed bit numbering. We're making
	// the part select from LSB to MSB.
      long base;
      if (par_msv < par_lsv) {
	    base = par_lsv - lsv;
      } else {
	    base = lsv - par_lsv;
      }

      if (warn_ob_select) {
	    if (base < 0) {
		  perm_string name = peek_tail_name(path_);
		  cerr << get_fileline() << ": warning: Part select "
		       << "[" << msv << ":" << lsv << "] is selecting "
		          "before the parameter " << name << "[";
		  if (par_ex->value().has_len()) cerr << par_msv;
		  else cerr << "<inf>";
		  cerr << ":" << par_lsv << "]." << endl;
		  cerr << get_fileline() << ":        : Replacing "
		          "the out of bound bits with 'bx." << endl;
	    }
	    if (par_ex->value().has_len() &&
                (base+wid > (long)par->expr_width())) {
		  perm_string name = peek_tail_name(path_);
		  cerr << get_fileline() << ": warning: Part select "
		       << name << "[" << msv << ":" << lsv << "] is selecting "
		          "after the parameter " << name << "[" << par_msv
		       << ":" << par_lsv << "]." << endl;
		  cerr << get_fileline() << ":        : Replacing "
		          "the out of bound bits with 'bx." << endl;
	    }
      }

      verinum result = param_part_select_bits(par_ex->value(), wid, base);
      NetEConst*result_ex = new NetEConst(result);
      result_ex->set_line(*this);

      return result_ex;
}

static void warn_param_ob(long par_msv, long par_lsv, bool defined,
                          long par_base, unsigned long wid, long pwid,
                          const LineInfo *info, perm_string name, bool up)
{
      long par_max;

      if (defined) {
	    if (par_msv < par_lsv) par_max = par_lsv-par_msv;
	     else par_max = par_msv-par_lsv;
      } else {
	    if (pwid < 0) par_max = integer_width;
	    else par_max = pwid;
      }

	/* Is this a select before the start of the parameter? */
      if (par_base < 0) {
	    cerr << info->get_fileline() << ": warning: " << name << "["
	         << par_base;
	    if (up) cerr << "+:";
	    else cerr << "-:";
	    cerr << wid << "] is selecting before vector." << endl;
      }

	/* Is this a select after the end of the parameter? */
      if (par_base + (long)wid - 1 > par_max) {
	    cerr << info->get_fileline() << ": warning: " << name << "["
	         << par_base << "+:" << wid << "] is selecting after vector."
	         << endl;
      }
}

NetExpr* PEIdent::elaborate_expr_param_idx_up_(Design*des, NetScope*scope,
					       const NetExpr*par,
					       NetScope*found_in,
					       const NetExpr*par_msb,
					       const NetExpr*par_lsb,
                                               bool need_const) const
{
      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);

      long par_msv, par_lsv;
      if(! calculate_param_range_(des, scope, par_msb, par_msv,
                                  par_lsb, par_lsv,
                                  par_ex->value().len())) return 0;

      NetExpr*base = calculate_up_do_base_(des, scope, need_const);
      if (base == 0) return 0;

	// Use the part select width already calculated by test_width().
      unsigned long wid = min_width_;

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Calculate part select "
		 << "[" << *base << "+:" << wid << "] from range "
		 << "[" << par_msv << ":" << par_lsv << "]." << endl;

      perm_string name = peek_tail_name(path_);

	// Handle the special case that the base is constant. In this
	// case, just precalculate the entire constant result.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    if (! base_c->value().is_defined()) {
		  NetEConst *ex;
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: " << name
			     << "['bx+:" << wid
			     << "] is always outside vector." << endl;
		  }
		  return ex;
	    }
	    long lsv = base_c->value().as_long();
	    long par_base = par_lsv;

	      // Watch out for reversed bit numbering. We're making
	      // the part select from LSB to MSB.
	    if (par_msv < par_lsv) {
		  par_base = lsv;
		  lsv = par_lsv - wid + 1;
	    }

	    if (warn_ob_select) {
                  bool defined = true;
		    // Check to see if the parameter has a defined range.
                  if (par_msb == 0) {
			assert(par_lsb == 0);
			defined = false;
                  }
		    // Get the parameter values width.
                  long pwid = -1;
                  if (par_ex->has_width()) pwid = par_ex->expr_width()-1;
                  warn_param_ob(par_msv, par_lsv, defined, lsv-par_base, wid,
                                pwid, this, name, true);
	    }
	    verinum result = param_part_select_bits(par_ex->value(), wid,
						    lsv-par_base);
	    NetEConst*result_ex = new NetEConst(result);
	    result_ex->set_line(*this);
	    return result_ex;
      }

      base = normalize_variable_base(base, par_msv, par_lsv, wid, true);

	/* Create a parameter reference for the variable select. */
      NetEConstParam*ptmp = new NetEConstParam(found_in, name, par_ex->value());
      NetScope::param_ref_t pref = found_in->find_parameter(name);
      ptmp->set_line((*pref).second);

      NetExpr*tmp = new NetESelect(ptmp, base, wid, IVL_SEL_IDX_UP);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEIdent::elaborate_expr_param_idx_do_(Design*des, NetScope*scope,
					       const NetExpr*par,
					       NetScope*found_in,
					       const NetExpr*par_msb,
					       const NetExpr*par_lsb,
                                               bool need_const) const
{
      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);

      long par_msv, par_lsv;
      if(! calculate_param_range_(des, scope, par_msb, par_msv,
                                  par_lsb, par_lsv,
                                  par_ex->value().len())) return 0;

      NetExpr*base = calculate_up_do_base_(des, scope, need_const);
      if (base == 0) return 0;

	// Use the part select width already calculated by test_width().
      unsigned long wid = min_width_;

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Calculate part select "
		 << "[" << *base << "-:" << wid << "] from range "
		 << "[" << par_msv << ":" << par_lsv << "]." << endl;

      perm_string name = peek_tail_name(path_);

	// Handle the special case that the base is constant. In this
	// case, just precalculate the entire constant result.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    if (! base_c->value().is_defined()) {
		  NetEConst *ex;
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: " << name
			     << "['bx-:" << wid
			     << "] is always outside vector." << endl;
		  }
		  return ex;
	    }
	    long lsv = base_c->value().as_long();
	    long par_base = par_lsv + wid - 1;

	      // Watch out for reversed bit numbering. We're making
	      // the part select from LSB to MSB.
	    if (par_msv < par_lsv) {
		  par_base = lsv;
		  lsv = par_lsv;
	    }

	    if (warn_ob_select) {
                  bool defined = true;
		    // Check to see if the parameter has a defined range.
                  if (par_msb == 0) {
			assert(par_lsb == 0);
			defined = false;
                  }
		    // Get the parameter values width.
                  long pwid = -1;
                  if (par_ex->has_width()) pwid = par_ex->expr_width()-1;
                  warn_param_ob(par_msv, par_lsv, defined, lsv-par_base, wid,
                                pwid, this, name, false);
	    }

	    verinum result = param_part_select_bits(par_ex->value(), wid,
						    lsv-par_base);
	    NetEConst*result_ex = new NetEConst(result);
	    result_ex->set_line(*this);
	    return result_ex;
      }

      base = normalize_variable_base(base, par_msv, par_lsv, wid, false);

	/* Create a parameter reference for the variable select. */
      NetEConstParam*ptmp = new NetEConstParam(found_in, name, par_ex->value());
      NetScope::param_ref_t pref = found_in->find_parameter(name);
      ptmp->set_line((*pref).second);

      NetExpr*tmp = new NetESelect(ptmp, base, wid, IVL_SEL_IDX_DOWN);
      tmp->set_line(*this);
      return tmp;
}

/*
 * Handle the case that the identifier is a parameter reference. The
 * parameter expression has already been located for us (as the par
 * argument) so we just need to process the sub-expression.
 */
NetExpr* PEIdent::elaborate_expr_param_(Design*des,
					NetScope*scope,
					const NetExpr*par,
					NetScope*found_in,
					const NetExpr*par_msb,
					const NetExpr*par_lsb,
					unsigned expr_wid, unsigned flags) const
{
      bool need_const = NEED_CONST & flags;

      if (need_const && !(ANNOTATABLE & flags)) {
            perm_string name = peek_tail_name(path_);
            if (found_in->make_parameter_unannotatable(name)) {
                  cerr << get_fileline() << ": warning: specparam '" << name
                       << "' is being used in a constant expression." << endl;
                  cerr << get_fileline() << ":        : This will prevent it "
                          "being annotated at run time." << endl;
            }
      }

      const name_component_t&name_tail = path_.back();
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

      if (par->expr_type() == IVL_VT_REAL &&
          use_sel != index_component_t::SEL_NONE) {
	    perm_string name = peek_tail_name(path_);
	    cerr << get_fileline() << ": error: "
	         << "can not select part of real parameter: " << name << endl;
	    des->errors += 1;
	    return 0;
      }

      ivl_assert(*this, use_sel != index_component_t::SEL_BIT_LAST);

      if (use_sel == index_component_t::SEL_BIT)
	    return elaborate_expr_param_bit_(des, scope, par, found_in,
					     par_msb, par_lsb, need_const);

      if (use_sel == index_component_t::SEL_PART)
	    return elaborate_expr_param_part_(des, scope, par, found_in,
					      par_msb, par_lsb, expr_wid);

      if (use_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_param_idx_up_(des, scope, par, found_in,
						par_msb, par_lsb, need_const);

      if (use_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_param_idx_do_(des, scope, par, found_in,
						par_msb, par_lsb, need_const);

      NetExpr*tmp = 0;

      const NetEConstEnum*etmp = dynamic_cast<const NetEConstEnum*>(par);
      if (etmp) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Elaborate parameter <" << path_
		       << "> as enumeration constant." << *etmp << endl;
	    tmp = etmp->dup_expr();
            tmp = pad_to_width(tmp, expr_wid, *this);

      } else {
	    perm_string name = peek_tail_name(path_);

	      /* No bit or part select. Make the constant into a
		 NetEConstParam or NetECRealParam as appropriate. */
	    const NetEConst*ctmp = dynamic_cast<const NetEConst*>(par);
	    if (ctmp) {
                  verinum cvalue = ctmp->value();
                  if (cvalue.has_len())
			cvalue.has_sign(signed_flag_);
                  cvalue = cast_to_width(cvalue, expr_wid);
		  tmp = new NetEConstParam(found_in, name, cvalue);
		  tmp->cast_signed(signed_flag_);
		  tmp->set_line(*par);

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Elaborate parameter <" << name
			     << "> as constant " << *tmp << endl;
	    }

	    const NetECReal*rtmp = dynamic_cast<const NetECReal*>(par);
	    if (rtmp) {
		  tmp = new NetECRealParam(found_in, name, rtmp->value());
		  tmp->set_line(*par);

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Elaborate parameter <" << name
			     << "> as constant " << *tmp << endl;
	    }
	      /* The numeric parameter value needs to have the file and line
	       * information for the actual parameter not the expression. */
	    assert(tmp);
	    NetScope::param_ref_t pref = found_in->find_parameter(name);
	    tmp->set_line((*pref).second);
      }

      return tmp;
}

/*
 * Handle word selects of vector arrays.
 */
NetExpr* PEIdent::elaborate_expr_net_word_(Design*des, NetScope*scope,
					   NetNet*net, NetScope*found_in,
                                           unsigned expr_wid,
					   unsigned flags) const
{
      bool need_const = NEED_CONST & flags;

      const name_component_t&name_tail = path_.back();

	// Special case: This is the entire array, and we are a direct
	// argument of a system task.
      if (name_tail.index.empty() && (SYS_TASK_ARG & flags)) {
	    NetESignal*res = new NetESignal(net, 0);
	    res->set_line(*this);
	    return res;
      }

      if (name_tail.index.empty()) {
	    cerr << get_fileline() << ": error: Array " << path()
		 << " needs an array index here." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Make sure there are enough indices to address an array element.
      if (name_tail.index.size() < net->unpacked_dimensions()) {
	    cerr << get_fileline() << ": error: Array " << path()
		 << " needs " << net->unpacked_dimensions() << " indices,"
		 << " but got only " << name_tail.index.size() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Evaluate all the index expressions into an
	// "unpacked_indices" array.
      list<NetExpr*>unpacked_indices;
      list<long> unpacked_indices_const;
      indices_flags idx_flags;
      indices_to_expressions(des, scope, this,
			     name_tail.index, net->unpacked_dimensions(),
			     need_const,
			     idx_flags,
			     unpacked_indices,
			     unpacked_indices_const);

      NetExpr*canon_index = 0;
      if (idx_flags.invalid) {
	    // Nothing to do.

      } else if (idx_flags.undefined) {
	    cerr << get_fileline() << ": warning: "
		 << "returning 'bx for undefined array access "
		 << net->name() << as_indices(unpacked_indices)
		 << "." << endl;

      } else if (idx_flags.variable) {
	    ivl_assert(*this, unpacked_indices.size() == net->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(net, unpacked_indices);

      } else {
	    ivl_assert(*this, unpacked_indices_const.size() == net->unpacked_dimensions());
	    canon_index = normalize_variable_unpacked(net, unpacked_indices_const);

	    if (canon_index == 0) {
		  cerr << get_fileline() << ": warning: "
		       << "returning 'bx for out of bounds array access "
		       << net->name() << as_indices(unpacked_indices_const)
		       << "." << endl;
	    }
      }

      if (canon_index == 0) {
	    NetEConst*xxx = make_const_x(net->vector_width());
	    xxx->set_line(*this);
	    return xxx;
      }
      canon_index->set_line(*this);

      NetESignal*res = new NetESignal(net, canon_index);
      res->set_line(*this);

	// Detect that the word has a bit/part select as well.

      index_component_t::ctype_t word_sel = index_component_t::SEL_NONE;
      if (name_tail.index.size() > net->unpacked_dimensions())
	    word_sel = name_tail.index.back().sel;

      if (net->get_scalar() &&
          word_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (res->expr_type() == IVL_VT_REAL) cerr << "real";
	    else cerr << "scalar";
	    cerr << " array word: " << net->name()
		 << as_indices(unpacked_indices) << endl;
	    des->errors += 1;
	    delete res;
	    return 0;
      }

      if (word_sel == index_component_t::SEL_PART)
	    return elaborate_expr_net_part_(des, scope, res, found_in,
                                            expr_wid);

      if (word_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_net_idx_up_(des, scope, res, found_in,
                                              need_const);

      if (word_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_net_idx_do_(des, scope, res, found_in,
                                              need_const);

      if (word_sel == index_component_t::SEL_BIT)
	    return elaborate_expr_net_bit_(des, scope, res, found_in,
                                           need_const);

      ivl_assert(*this, word_sel == index_component_t::SEL_NONE);

      return res;
}

/*
 * Handle part selects of NetNet identifiers.
 */
NetExpr* PEIdent::elaborate_expr_net_part_(Design*des, NetScope*scope,
				           NetESignal*net, NetScope*,
                                           unsigned expr_wid) const
{
      list<long> prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, net->sig(), prefix_indices);
      if (!rc)
	    return 0;

      long msv, lsv;
      bool parts_defined_flag;
      bool flag = calculate_parts_(des, scope, msv, lsv, parts_defined_flag);
      if (!flag)
	    return 0;

	/* The indices of part selects are signed integers, so allow
	   negative values. However, the width that they represent is
	   unsigned. Remember that any order is possible,
	   i.e., [1:0], [-4:6], etc. */
      unsigned long wid = 1 + labs(msv-lsv);
	/* But wait... if the part select expressions are not fully
	   defined, then fall back on the tested width. */
      if (!parts_defined_flag) {
	    if (warn_ob_select) {
		  const index_component_t&psel = path_.back().index.back();
		  cerr << get_fileline() << ": warning: "
		          "Undefined part select [" << *(psel.msb) << ":"
		       << *(psel.lsb) << "] for ";
		  if (net->word_index()) cerr << "array word";
		  else cerr << "vector";
		  cerr << " '" << net->name();
		  if (net->word_index()) cerr << "[]";
		  cerr << "'." << endl;
		  cerr << get_fileline() << ":        : "
		          "Replacing select with a constant 'bx." << endl;
	    }

	    NetEConst*tmp = new NetEConst(verinum(verinum::Vx, expr_wid, true));
	    tmp->set_line(*this);
	    return tmp;
      }
      long sb_lsb, sb_msb;
      if (prefix_indices.size()+1 < net->sig()->packed_dims().size()) {
	      // Here we have a slice that doesn't have enough indices
	      // to get to a single slice. For example:
	      //    wire [9:0][5:1] foo
	      //      ... foo[4:3] ...
	      // Make this work by finding the indexed slices and
	      // creating a generated slice that spans the whole
	      // range.
	    long loff, moff;
	    unsigned long lwid, mwid;
	    bool lrc;
	    lrc = net->sig()->sb_to_slice(prefix_indices, lsv, loff, lwid);
	    ivl_assert(*this, lrc);
	    lrc = net->sig()->sb_to_slice(prefix_indices, msv, moff, mwid);
	    ivl_assert(*this, lrc);
	    ivl_assert(*this, lwid == mwid);

	    if (moff > loff) {
		  sb_lsb = loff;
		  sb_msb = moff + mwid - 1;
	    } else {
		  sb_lsb = moff;
		  sb_msb = loff + lwid - 1;
	    }
	    wid = sb_msb - sb_lsb + 1;
      } else {
	      // This case, the prefix indices are enough to index
	      // down to a single bit/slice.
	    ivl_assert(*this, prefix_indices.size()+1 == net->sig()->packed_dims().size());
	    sb_lsb = net->sig()->sb_to_idx(prefix_indices, lsv);
	    sb_msb = net->sig()->sb_to_idx(prefix_indices, msv);
      }

      if (sb_msb < sb_lsb) {
	    cerr << get_fileline() << ": error: part select " << net->name();
	    if (net->word_index()) cerr << "[]";
	    cerr << "[" << msv << ":" << lsv << "] is out of order." << endl;
	    des->errors += 1;
	      //delete lsn;
	      //delete msn;
	    return net;
      }

      if (warn_ob_select) {
	    if ((sb_lsb >= (signed) net->vector_width()) ||
	        (sb_msb >= (signed) net->vector_width())) {
		  cerr << get_fileline() << ": warning: "
		          "Part select " << "[" << msv << ":" << lsv
		       << "] is selecting after the ";
		  if (net->word_index()) cerr << "array word ";
		  else cerr << "vector ";
		  cerr << net->name();
		  if (net->word_index()) cerr << "[]";
		  cerr << "[" << net->msi() << ":" << net->lsi() << "]."
		       << endl;
		  cerr << get_fileline() << ":        : "
		       << "Replacing the out of bound bits with 'bx." << endl;
	    }
	    if ((sb_msb < 0) || (sb_lsb < 0)) {
		  cerr << get_fileline() << ": warning: "
		          "Part select " << "[" << msv << ":" << lsv
		       << "] is selecting before the ";
		  if (net->word_index()) cerr << "array word ";
		  else cerr << "vector ";
		  cerr << net->name();
		  if (net->word_index()) cerr << "[]";
		  cerr << "[" << net->msi() << ":" << net->lsi() << "]."
		       << endl;
		  cerr << get_fileline() << ":        : "
		          "Replacing the out of bound bits with 'bx." << endl;
	    }
      }

	// If the part select covers exactly the entire
	// vector, then do not bother with it. Return the
	// signal itself, casting to unsigned if necessary.
      if (sb_lsb == 0 && wid == net->vector_width()) {
	    net->cast_signed(false);
	    return net;
      }

	// If the part select covers NONE of the vector, then return a
	// constant X.

      if ((sb_lsb >= (signed) net->vector_width()) || (sb_msb < 0)) {
	    NetEConst*tmp = make_const_x(wid);
	    tmp->set_line(*this);
	    return tmp;
      }

      NetExpr*ex = new NetEConst(verinum(sb_lsb));
      NetESelect*ss = new NetESelect(net, ex, wid);
      ss->set_line(*this);
      return ss;
}

/*
 * Part select indexed up, i.e. net[<m> +: <l>]
 */
NetExpr* PEIdent::elaborate_expr_net_idx_up_(Design*des, NetScope*scope,
				             NetESignal*net, NetScope*,
                                             bool need_const) const
{
      list<long>prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, net->sig(), prefix_indices);
      if (!rc)
	    return 0;

      NetExpr*base = calculate_up_do_base_(des, scope, need_const);

	// Use the part select width already calculated by test_width().
      unsigned long wid = min_width_;

	// Handle the special case that the base is constant as
	// well. In this case it can be converted to a conventional
	// part select.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    NetExpr*ex;
	    if (base_c->value().is_defined()) {
		  long lsv = base_c->value().as_long();
		  long offset = 0;
		    // Get the signal range.
		  const vector<netrange_t>&packed = net->sig()->packed_dims();
		  ivl_assert(*this, packed.size() == prefix_indices.size()+1);

		    // We want the last range, which is where we work.
		  const netrange_t&rng = packed.back();
		  if (rng.get_msb() < rng.get_lsb()) {
			offset = -wid + 1;
		  }

		  long rel_base = net->sig()->sb_to_idx(prefix_indices, lsv);

		    // If the part select covers exactly the entire
		    // vector, then do not bother with it. Return the
		    // signal itself.
		  if (rel_base == 0 && wid == net->vector_width()) {
			delete base;
			net->cast_signed(false);
			return net;
		  }

		    // Otherwise, make a part select that covers the right
		    // range.
		  ex = new NetEConst(verinum(rel_base + offset));
		  if (warn_ob_select) {
			if (rel_base < 0) {
			      cerr << get_fileline() << ": warning: "
			           << net->name();
			      if (net->word_index()) cerr << "[]";
			      cerr << "[" << lsv << "+:" << wid
			           << "] is selecting before vector." << endl;
			}
			if (rel_base + wid > net->vector_width()) {
			      cerr << get_fileline() << ": warning: "
			           << net->name();
			      if (net->word_index()) cerr << "[]";
			      cerr << "[" << lsv << "+:" << wid
			           << "] is selecting after vector." << endl;
			}
		  }
	    } else {
		    // Return 'bx for an undefined base.
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  delete base;
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: " << net->name();
			if (net->word_index()) cerr << "[]";
			cerr << "['bx+:" << wid
			     << "] is always outside vector." << endl;
		  }
		  return ex;
	    }
	    NetESelect*ss = new NetESelect(net, ex, wid);
	    ss->set_line(*this);

	    delete base;
	    return ss;
      }


      ivl_assert(*this, prefix_indices.size()+1 == net->sig()->packed_dims().size());

	// Convert the non-constant part select index expression into
	// an expression that returns a canonical base.
      base = normalize_variable_part_base(prefix_indices, base, net->sig(), wid, true);

      NetESelect*ss = new NetESelect(net, base, wid, IVL_SEL_IDX_UP);
      ss->set_line(*this);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate part "
		 << "select base="<< *base << ", wid="<< wid << endl;
      }

      return ss;
}

/*
 * Part select indexed down, i.e. net[<m> -: <l>]
 */
NetExpr* PEIdent::elaborate_expr_net_idx_do_(Design*des, NetScope*scope,
					     NetESignal*net, NetScope*,
                                             bool need_const) const
{
      list<long>prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, net->sig(), prefix_indices);
      if (!rc)
	    return 0;

      NetExpr*base = calculate_up_do_base_(des, scope, need_const);

	// Use the part select width already calculated by test_width().
      unsigned long wid = min_width_;

	// Handle the special case that the base is constant as
	// well. In this case it can be converted to a conventional
	// part select.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    NetExpr*ex;
	    if (base_c->value().is_defined()) {
		  long lsv = base_c->value().as_long();

		    // If the part select covers exactly the entire
		    // vector, then do not bother with it. Return the
		    // signal itself.
		  if (net->sig()->sb_to_idx(prefix_indices,lsv) == (signed) (wid-1) &&
		      wid == net->vector_width()) {
			delete base;
			net->cast_signed(false);
			return net;
		  }

		  long offset = 0;
		  if (net->msi() > net->lsi()) {
			offset = -wid + 1;
		  }
		    // Otherwise, make a part select that covers the right
		    // range.
		  ex = new NetEConst(verinum(net->sig()->sb_to_idx(prefix_indices,lsv) + offset));
		  if (warn_ob_select) {
			long rel_base = net->sig()->sb_to_idx(prefix_indices,lsv) + offset;
			if (rel_base < 0) {
			      cerr << get_fileline() << ": warning: "
			           << net->name();
			      if (net->word_index()) cerr << "[]";
			      cerr << "[" << lsv << "+:" << wid
			           << "] is selecting before vector." << endl;
			}
			if (rel_base + wid > net->vector_width()) {
			      cerr << get_fileline() << ": warning: "
			           << net->name();
			      if (net->word_index()) cerr << "[]";
			      cerr << "[" << lsv << "-:" << wid
			           << "] is selecting after vector." << endl;
			}
		  }
	    } else {
		    // Return 'bx for an undefined base.
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  delete base;
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: " << net->name();
			if (net->word_index()) cerr << "[]";
			cerr << "['bx-:" << wid
			     << "] is always outside vector." << endl;
		  }
		  return ex;
	    }
	    NetESelect*ss = new NetESelect(net, ex, wid);
	    ss->set_line(*this);

	    delete base;
	    return ss;
      }

      base = normalize_variable_base(base, net->msi(), net->lsi(), wid, false);

      NetESelect*ss = new NetESelect(net, base, wid, IVL_SEL_IDX_DOWN);
      ss->set_line(*this);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate part "
		 << "select base="<< *base << ", wid="<< wid << endl;
      }

      return ss;
}

NetExpr* PEIdent::elaborate_expr_net_bit_(Design*des, NetScope*scope,
				          NetESignal*net, NetScope*,
                                          bool need_const) const
{
      list<long>prefix_indices;
      bool rc = calculate_packed_indices_(des, scope, net->sig(), prefix_indices);
      if (!rc)
	    return 0;

      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

      NetExpr*mux = elab_and_eval(des, scope, index_tail.msb, -1, need_const);

      if (const netdarray_t*darray = net->sig()->darray_type()) {
	      // Special case: This is a select of a dynamic
	      // array. Generate a NetESelect and attach it to
	      // the NetESignal. This should be interpreted as
	      // an array word select downstream.
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: "
		       << "Bit select of a dynamic array becomes NetESelect." << endl;
	    }
	    NetESelect*res = new NetESelect(net, mux, darray->element_width());
	    res->set_line(*net);
	    return res;
      }

	// If the bit select is constant, then treat it similar
	// to the part select, so that I save the effort of
	// making a mux part in the netlist.
      if (NetEConst*msc = dynamic_cast<NetEConst*> (mux)) {

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_expr_net_bit_: "
		       << "mux is constant=" << *msc
		       << ", packed_dims()=" << net->sig()->packed_dims()
		       << ", packed_dims().size()=" << net->sig()->packed_dims().size()
		       << ", prefix_indices.size()=" << prefix_indices.size()
		       << endl;
	    }

	      // Special case: The bit select expression is constant
	      // x/z. The result of the expression is 1'bx.
	    if (! msc->value().is_defined()) {
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: "
			        "Constant bit select [" << msc->value()
			      << "] is undefined for ";
			if (net->word_index()) cerr << "array word";
			else cerr << "vector";
			cerr << " '" << net->name();
			if (net->word_index()) cerr << "[]";
			cerr  << "'." << endl;
			cerr << get_fileline() << ":        : "
			     << "Replacing select with a constant 1'bx."
			     << endl;
		  }

		    // FIXME: Should I be using slice_width() here?
		  NetEConst*tmp = make_const_x(1);
		  tmp->set_line(*this);
		  delete mux;
		  return tmp;
	    }

	    long msv = msc->value().as_long();

	    const vector<netrange_t>& sig_packed = net->sig()->packed_dims();
	    if (prefix_indices.size()+2 <= sig_packed.size()) {
		    // Special case: this is a slice of a multi-dimensional
		    // packed array. For example:
		    //   reg [3:0][7:0] x;
		    //   ... x[2] ...
		    // This shows up as the prefix_indices being too short
		    // for the packed dimensions of the vector. What we do
		    // here is convert to a "slice" of the vector.
		  unsigned long lwid;
		  long idx;
		  rc = net->sig()->sb_to_slice(prefix_indices, msv, idx, lwid);
		  ivl_assert(*this, rc);

		    // Make an expression out of the index
		  NetEConst*idx_c = new NetEConst(verinum(idx));
		  idx_c->set_line(*net);

		  NetESelect*res = new NetESelect(net, idx_c, lwid);
		  res->set_line(*net);
		  return res;
	    }

	    if (net->sig()->data_type()==IVL_VT_STRING && (msv < 0)) {
		    // Special case: This is a constant bit select of
		    // a string, and the index is < 0. For example:
		    //   string foo;
		    //   ... foo[-1] ...
		    // This is known to be 8'h00.
		  NetEConst*tmp = make_const_0(8);
		  tmp->set_line(*this);
		  delete mux;
		  return tmp;
	    }

	    if (net->sig()->data_type()==IVL_VT_STRING) {
		    // Special case: This is a select of a string
		    // variable. Generate a NetESelect and attach it
		    // to the NetESignal. This should be interpreted
		    // as a character select downstream.
		  if (debug_elaborate) {
			cerr << get_fileline() << ": debug: "
			     << "Bit select of string becomes NetESelect." << endl;
		  }
		  NetESelect*res = new NetESelect(net, mux, 8);
		  res->set_line(*net);
		  return res;
	    }

	    long idx = net->sig()->sb_to_idx(prefix_indices,msv);

	    if (idx >= (long)net->vector_width() || idx < 0) {
		    /* The bit select is out of range of the
		       vector. This is legal, but returns a
		       constant 1'bx value. */
		  if (warn_ob_select) {
			cerr << get_fileline() << ": warning: "
			        "Constant bit select [" << msv
			      << "] is ";
			if (idx < 0) cerr << "before ";
			else cerr << "after ";
			if (net->word_index()) cerr << "array word ";
			else cerr << "vector ";
			cerr << net->name();
			if (net->word_index()) cerr << "[]";
			cerr  << net->sig()->packed_dims() << "." << endl;
			cerr << get_fileline() << ":        : "
			     << "Replacing select with a constant 1'bx."
			     << endl;
		  }

		  NetEConst*tmp = make_const_x(1);
		  tmp->set_line(*this);

		  delete mux;
		  return tmp;
	    }

	      // If the vector is only one bit, we are done. The
	      // bit select will return the scalar itself.
	    if (net->vector_width() == 1)
		  return net;

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PEIdent::elaborate_expr_net_bit_: "
		       << "Make bit select idx=" << idx
		       << endl;
	    }

	      // Make an expression out of the index
	    NetEConst*idx_c = new NetEConst(verinum(idx));
	    idx_c->set_line(*net);

	      // Make a bit select with the canonical index
	    NetESelect*res = new NetESelect(net, idx_c, 1);
	    res->set_line(*net);

	    return res;
      }

      const vector<netrange_t>& sig_packed = net->sig()->packed_dims();
      if (prefix_indices.size()+2 <= sig_packed.size()) {
	      // Special case: this is a slice of a multi-dimensional
	      // packed array. For example:
	      //   reg [3:0][7:0] x;
	      //   x[2] = ...
	      // This shows up as the prefix_indices being too short
	      // for the packed dimensions of the vector. What we do
	      // here is convert to a "slice" of the vector.
	    unsigned long lwid;
	    mux = normalize_variable_slice_base(prefix_indices, mux,
						net->sig(), lwid);
	    mux->set_line(*net);

	      // Make a PART select with the canonical index
	    NetESelect*res = new NetESelect(net, mux, lwid);
	    res->set_line(*net);

	    return res;
      }

      if (net->sig()->data_type() == IVL_VT_STRING) {
	      // Special case: This is a select of a string.
	      // This should be interpreted as a byte select.
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: "
		       << "Bit select of a string becomes NetESelect." << endl;
	    }
	    NetESelect*res = new NetESelect(net, mux, 8);
	    res->set_line(*net);
	    return res;
      }

	// Non-constant bit select? punt and make a subsignal
	// device to mux the bit in the net. This is a fairly
	// complicated task because we need to generate
	// expressions to convert calculated bit select
	// values to canonical values that are used internally.
      mux = normalize_variable_bit_base(prefix_indices, mux, net->sig());

      NetESelect*ss = new NetESelect(net, mux, 1);
      ss->set_line(*this);
      return ss;
}

NetExpr* PEIdent::elaborate_expr_net_bit_last_(Design*, NetScope*,
					       NetESignal*net,
					       NetScope* /* found_in */,
					       bool need_const) const
{
      if (need_const) {
	    cerr << get_fileline() << ": error: "
		 << "Expression with \"[$]\" is not constant." << endl;
	    return 0;
      }

      unsigned use_width = 1;
      if (const netdarray_t*darray = net->sig()->darray_type()) {
	    use_width = darray->element_width();
      }

      NetELast*mux = new NetELast(net->sig());
      mux->set_line(*this);
      NetESelect*ss = new NetESelect(net, mux, use_width);
      ss->set_line(*this);
      return ss;
}

NetExpr* PEIdent::elaborate_expr_net(Design*des, NetScope*scope,
				     NetNet*net, NetScope*found_in,
                                     unsigned expr_wid,
				     unsigned flags) const
{
      if (net->unpacked_dimensions() > 0)
	    return elaborate_expr_net_word_(des, scope, net, found_in,
                                            expr_wid, flags);

      bool need_const = NEED_CONST & flags;

      NetESignal*node = new NetESignal(net);
      node->set_line(*this);

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (! path_.back().index.empty())
	    use_sel = path_.back().index.back().sel;

      if (net->get_scalar() &&
          use_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (node->expr_type() == IVL_VT_REAL) cerr << "real: ";
	    else cerr << "scalar: ";
	    cerr << net->name() << endl;
	    des->errors += 1;
	    return 0;
      }

      list<long> prefix_indices;
      bool rc = evaluate_index_prefix(des, scope, prefix_indices, path_.back().index);
      if (!rc) return 0;

	// If this is a part select of a signal, then make a new
	// temporary signal that is connected to just the
	// selected bits. The lsb_ and msb_ expressions are from
	// the foo[msb:lsb] expression in the original.
      if (use_sel == index_component_t::SEL_PART)
	    return elaborate_expr_net_part_(des, scope, node, found_in,
                                            expr_wid);

      if (use_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_net_idx_up_(des, scope, node, found_in,
                                              need_const);

      if (use_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_net_idx_do_(des, scope, node, found_in,
                                              need_const);

      if (use_sel == index_component_t::SEL_BIT)
	    return elaborate_expr_net_bit_(des, scope, node, found_in,
                                           need_const);

      if (use_sel == index_component_t::SEL_BIT_LAST)
	    return elaborate_expr_net_bit_last_(des, scope, node, found_in,
						need_const);

	// It's not anything else, so this must be a simple identifier
	// expression with no part or bit select. Return the signal
	// itself as the expression.
      assert(use_sel == index_component_t::SEL_NONE);

      return node;
}

unsigned PENewArray::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_  = IVL_VT_DARRAY;
      expr_width_ = 1;
      min_width_  = 1;
      signed_flag_= false;
      return 1;
}

NetExpr* PENewArray::elaborate_expr(Design*des, NetScope*scope,
				    ivl_type_t ntype, unsigned flags) const
{
	// Elaborate the size expression.
      width_mode_t mode = LOSSLESS;
      unsigned use_wid = size_->test_width(des, scope, mode);
      NetExpr*size = size_->elaborate_expr(des, scope, use_wid, flags);
      NetExpr*init_val = 0;

      if (dynamic_cast<PEAssignPattern*> (init_)) {
	      // Special case: the initial value expression is an
	      // array_pattern. Elaborate the expression like the
	      // r-value to an assignment to array.
	    init_val = init_->elaborate_expr(des, scope, ntype, flags);

      } else if (init_) {
	      // Regular case: The initial value is an
	      // expression. Elaborate the expression as an element
	      // type. The run-time will assign this value to each element.
	    const netarray_t*array_type = dynamic_cast<const netarray_t*> (ntype);
	    ivl_type_t elem_type = array_type->element_type();

	    init_val = init_->elaborate_expr(des, scope, elem_type, flags);
      }

      NetENew*tmp = new NetENew(ntype, size, init_val);
      tmp->set_line(*this);

      return tmp;
}

/*
 * This method should never actually be called.
 */
NetExpr* PENewArray::elaborate_expr(Design*, NetScope*, unsigned, unsigned) const
{
      ivl_assert(*this, 0);
      return 0;
}

unsigned PENewClass::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_  = IVL_VT_CLASS;
      expr_width_ = 1;
      min_width_  = 1;
      signed_flag_= false;
      return 1;
}

/*
 * This elaborates the constructor for a class. This arranges for the
 * call of class constructor, if present, and also
 * initializers in front of an explicit constructor.
 *
 * The derived argument is the type of the class derived from the
 * current one. This is used to get chained constructor arguments, if necessary.
 */
NetExpr* PENewClass::elaborate_expr_constructor_(Design*des, NetScope*scope,
						 const netclass_t*ctype,
						 NetExpr*obj, unsigned /*flags*/) const
{
      ivl_assert(*this, ctype);

	// If there is an initializer function, then pass the object
	// through that function first. Note that the initializer
	// function has no arguments other than the object itself.
      if (NetScope*new1_scope = ctype->method_from_name(perm_string::literal("new@"))) {
	    NetFuncDef*def1 = new1_scope->func_def();
	    ivl_assert(*this, def1);
	    ivl_assert(*this, def1->port_count()==1);
	    vector<NetExpr*> parms1 (1);
	    parms1[0] = obj;

	      // The return value of the initializer is the "this"
	      // variable, instead of the "new&" scope name.
	    NetNet*res1 = new1_scope->find_signal(perm_string::literal("@"));
	    ivl_assert(*this, res1);

	    NetESignal*eres = new NetESignal(res1);
	    NetEUFunc*tmp = new NetEUFunc(scope, new1_scope, eres, parms1, true);
	    tmp->set_line(*this);
	    obj = tmp;
      }


      NetScope*new_scope = ctype->method_from_name(perm_string::literal("new"));
      if (new_scope == 0) {
	      // No constructor.
	    if (parms_.size() > 0) {
		  cerr << get_fileline() << ": error: "
		       << "Class " << ctype->get_name()
		       << " has no constructor, but you passed " << parms_.size()
		       << " arguments to the new operator." << endl;
		  des->errors += 1;
	    }
	    return obj;
      }


      NetFuncDef*def = new_scope->func_def();
      if (def == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "Scope " << scope_path(new_scope)
		 << " is missing constructor definition." << endl;
	    des->errors += 1;
      }
      ivl_assert(*this, def);

	// Are there too many arguments passed to the function. If so,
	// generate an error message. The case of too few arguments
	// will be handled below, when we run out of arguments.
      if ((parms_.size()+1) > def->port_count()) {
	    cerr << get_fileline() << ": error: Parm count mismatch"
		 << " passing " << parms_.size() << " arguments "
		 << " to constructor expecting " << (def->port_count()-1)
		 << " arguments." << endl;
	    des->errors += 1;
      }

      vector<NetExpr*> parms (def->port_count());
      parms[0] = obj;

      int missing_parms = 0;
      int parm_errors = 0;
      for (size_t idx = 1 ; idx < parms.size() ; idx += 1) {
	      // While there are default arguments, check them.
	    if (idx <= parms_.size() && parms_[idx-1]) {
		  PExpr*tmp = parms_[idx-1];
		  parms[idx] = elaborate_rval_expr(des, scope,
						   def->port(idx)->net_type(),
						   def->port(idx)->data_type(),
						   def->port(idx)->vector_width(),
						   tmp, false);
		  if (parms[idx] == 0)
			parm_errors += 1;

		  continue;
	    }

	      // Ran out of explicit arguments. Is there a default
	      // argument we can use?
	    if (NetExpr*tmp = def->port_defe(idx)) {
		  parms[idx] = tmp;
		  continue;
	    }

	      // If we run out of passed expressions, and there is no
	      // default value for this port, then we will need to
	      // report an error that we are missing parameters.
	    missing_parms += 1;
	    parms[idx] = 0;
      }

      if (missing_parms > 0) {
	    cerr << get_fileline() << ": error: The " << scope_path(new_scope)
		 << " constructor call is missing arguments." << endl;
	    parm_errors += 1;
	    des->errors += 1;
      }

	// The return value for the constructor is actually the "this"
	// variable, instead of the "new" scope name.
      NetNet*res = new_scope->find_signal(perm_string::literal("@"));
      ivl_assert(*this, res);

      NetESignal*eres = new NetESignal(res);
      NetEUFunc*con = new NetEUFunc(scope, new_scope, eres, parms, true);
      con->set_line(*this);

      return con;
}

NetExpr* PENewClass::elaborate_expr(Design*des, NetScope*scope,
				    ivl_type_t ntype, unsigned flags) const
{
      NetExpr*obj = new NetENew(ntype);
      obj->set_line(*this);

	// Find the constructor for the class. If there is no
	// constructor then the result of this expression is the
	// allocation alone.
      const netclass_t*ctype = dynamic_cast<const netclass_t*> (ntype);

      obj = elaborate_expr_constructor_(des, scope, ctype, obj, flags);
      return obj;
}

unsigned PENewCopy::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_  = IVL_VT_CLASS;
      expr_width_ = 1;
      min_width_  = 1;
      signed_flag_= false;
      return 1;
}

NetExpr* PENewCopy::elaborate_expr(Design*des, NetScope*scope, ivl_type_t obj_type, unsigned) const
{
      NetExpr*copy_arg = src_->elaborate_expr(des, scope, obj_type, 0);
      if (copy_arg == 0)
	    return 0;

      NetENew*obj_new = new NetENew(obj_type);
      obj_new->set_line(*this);

      NetEShallowCopy*copy = new NetEShallowCopy(obj_new, copy_arg);
      copy->set_line(*this);

      return copy;
}

/*
 * A "null" expression represents class objects/handles. This brings
 * up a ton of special cases, but we handle it here by setting the
 * expr_type_ and expr_width_ to fixed values.
 */
unsigned PENull::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_   = IVL_VT_CLASS;
      expr_width_  = 1;
      min_width_   = 1;
      signed_flag_ = false;
      return expr_width_;
}

NetExpr* PENull::elaborate_expr(Design*, NetScope*, ivl_type_t, unsigned) const
{
      NetENull*tmp = new NetENull;
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PENull::elaborate_expr(Design*, NetScope*, unsigned, unsigned) const
{
      NetENull*tmp = new NetENull;
      tmp->set_line(*this);
      return tmp;
}

unsigned PENumber::test_width(Design*, NetScope*, width_mode_t&mode)
{
      expr_type_   = IVL_VT_LOGIC;
      expr_width_  = value_->len();
      min_width_   = expr_width_;
      signed_flag_ = value_->has_sign();

      if (!value_->has_len() && !value_->is_single()) {
            if (gn_strict_expr_width_flag) {
                  expr_width_ = integer_width;
                  mode = UNSIZED;
            } else if (mode < LOSSLESS) {
		  if (expr_width_ < integer_width) {
			expr_width_ = integer_width;
			if (mode < UNSIZED)
			      mode = UNSIZED;
		  } else {
			mode = LOSSLESS;
		  }
            }
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PENumber::test_width: "
		 << "Value=" << *value_
		 << ", width=" << expr_width_
		 << ", output mode=" << width_mode_name(mode) << endl;
      }

      return expr_width_;
}

NetExpr* PENumber::elaborate_expr(Design*des, NetScope*, ivl_type_t ntype, unsigned) const
{
      const netvector_t*use_type = dynamic_cast<const netvector_t*> (ntype);
      if (use_type == 0) {
	    cerr << get_fileline() << ": internal error: "
		 << "I don't know how cast numbers to this type."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

	// Special case: If the context type is REAL, then cast the
	// vector value to a real and return a NetECReal.
      if (ntype->base_type() == IVL_VT_REAL) {
	    verireal val (value_->as_long());
	    NetECReal*tmp = new NetECReal(val);
	    tmp->set_line(*this);
	    return tmp;
      }

      verinum use_val = value();
      use_val .has_sign( use_type->get_signed() );
      use_val = cast_to_width(use_val, use_type->packed_width());

      NetEConst*tmp = new NetEConst(use_val);
      tmp->set_line(*this);

      return tmp;
}

NetEConst* PENumber::elaborate_expr(Design*, NetScope*,
				    unsigned expr_wid, unsigned) const
{
      assert(value_);
      verinum val = *value_;
      if (val.has_len())
            val.has_sign(signed_flag_);
      val = cast_to_width(val, expr_wid);
      NetEConst*tmp = new NetEConst(val);
      tmp->cast_signed(signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

unsigned PEString::test_width(Design*, NetScope*, width_mode_t&)
{
      expr_type_   = IVL_VT_BOOL;
      expr_width_  = text_? verinum(text_).len() : 0;
      min_width_   = expr_width_;
      signed_flag_ = false;

      return expr_width_;
}

NetEConst* PEString::elaborate_expr(Design*, NetScope*, ivl_type_t, unsigned)const
{
      verinum val(value());
      NetEConst*tmp = new NetEConst(val);
      tmp->cast_signed(signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

NetEConst* PEString::elaborate_expr(Design*, NetScope*,
				    unsigned expr_wid, unsigned) const
{
      verinum val(value());
      val = pad_to_width(val, expr_wid);
      NetEConst*tmp = new NetEConst(val);
      tmp->cast_signed(signed_flag_);
      tmp->set_line(*this);

      return tmp;
}

unsigned PETernary::test_width(Design*des, NetScope*scope, width_mode_t&mode)
{
	// The condition of the ternary is self-determined, so
	// we will test its width when we elaborate it.

        // Test the width of the true and false clauses.
      unsigned tru_width = tru_->test_width(des, scope, mode);

      width_mode_t saved_mode = mode;

      unsigned fal_width = fal_->test_width(des, scope, mode);

        // If the width mode changed, retest the true clause, as it
        // may choose a different width if it is in a lossless context.
      if ((mode >= LOSSLESS) && (saved_mode < LOSSLESS)) {
	    tru_width = tru_->test_width(des, scope, mode);
      }

	// If either of the alternatives is IVL_VT_REAL, then the
	// expression as a whole is IVL_VT_REAL. Otherwise, if either
	// of the alternatives is IVL_VT_LOGIC, then the expression as
	// a whole is IVL_VT_LOGIC. The fallback assumes that the
	// types are the same and we take that.
      ivl_variable_type_t tru_type = tru_->expr_type();
      ivl_variable_type_t fal_type = fal_->expr_type();

      if (tru_type == IVL_VT_REAL || fal_type == IVL_VT_REAL) {
	    expr_type_ = IVL_VT_REAL;
      } else if (tru_type == IVL_VT_LOGIC || fal_type == IVL_VT_LOGIC) {
	    expr_type_ = IVL_VT_LOGIC;
      } else {
	    ivl_assert(*this, tru_type == fal_type);
	    expr_type_ = tru_type;
      }
      if (expr_type_ == IVL_VT_REAL) {
	    expr_width_  = 1;
            min_width_   = 1;
            signed_flag_ = true;
      } else {
	    expr_width_  = max(tru_width, fal_width);
            min_width_   = max(tru_->min_width(), fal_->min_width());
            signed_flag_ = tru_->has_sign() && fal_->has_sign();

              // If the alternatives are different types, the expression
              // is forced to unsigned. In this case the lossless width
              // calculation is unreliable and we need to make sure the
              // final expression width is at least integer_width.
            if ((mode == LOSSLESS) && (tru_->has_sign() != fal_->has_sign()))
                  mode = UPSIZE;
      }

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: "
		 << "Ternary expression type=" << expr_type_
		 << ", width=" << expr_width_
		 << " (tru_type=" << tru_type
		 << ", fal_type=" << fal_type << ")" << endl;

      return fix_width_(mode);
}

bool NetETernary::test_operand_compat(ivl_variable_type_t l,
				      ivl_variable_type_t r)
{
      if (l == IVL_VT_LOGIC && r == IVL_VT_BOOL)
	    return true;
      if (l == IVL_VT_BOOL && r == IVL_VT_LOGIC)
	    return true;

      if (l == IVL_VT_REAL && (r == IVL_VT_LOGIC || r == IVL_VT_BOOL))
	    return true;
      if (r == IVL_VT_REAL && (l == IVL_VT_LOGIC || l == IVL_VT_BOOL))
	    return true;

      if (l == r)
	    return true;

      return false;
}

/*
 * Elaborate the Ternary operator. I know that the expressions were
 * parsed so I can presume that they exist, and call elaboration
 * methods. If any elaboration fails, then give up and return 0.
 */
NetExpr*PETernary::elaborate_expr(Design*des, NetScope*scope,
				  unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag

      ivl_assert(*this, expr_);
      ivl_assert(*this, tru_);
      ivl_assert(*this, fal_);

	// Elaborate and evaluate the condition expression. Note that
	// it is always self-determined.
      NetExpr*con = elab_and_eval(des, scope, expr_, -1, NEED_CONST & flags);
      if (con == 0)
	    return 0;

	/* Make sure the condition expression reduces to a single bit. */
      con = condition_reduce(con);

	// Verilog doesn't say that we must do short circuit
	// evaluation of ternary expressions, but it doesn't disallow
	// it. The disadvantage of doing this is that semantic errors
	// in the unused clause will be missed, but people don't seem
	// to mind, and do appreciate the optimization available here.
      if (NetEConst*tmp = dynamic_cast<NetEConst*> (con)) {
	    verinum cval = tmp->value();
	    ivl_assert(*this, cval.len()==1);

	      // Condition is constant TRUE, so we only need the true clause.
	    if (cval.get(0) == verinum::V1) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: Short-circuit "
			        "elaborate TRUE clause of ternary."
			     << endl;

		  return elab_and_eval_alternative_(des, scope, tru_,
                                                    expr_wid, flags, true);
	    }

	      // Condition is constant FALSE, so we only need the
	      // false clause.
	    if (cval.get(0) == verinum::V0) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: Short-circuit "
			        "elaborate FALSE clause of ternary."
			<< endl;

		  return elab_and_eval_alternative_(des, scope, fal_,
                                                    expr_wid, flags, true);
	    }

	      // X and Z conditions need to blend both results, so we
	      // can't short-circuit.
      }

      NetExpr*tru = elab_and_eval_alternative_(des, scope, tru_,
					       expr_wid, flags, false);
      if (tru == 0) {
	    delete con;
	    return 0;
      }

      NetExpr*fal = elab_and_eval_alternative_(des, scope, fal_,
					       expr_wid, flags, false);
      if (fal == 0) {
	    delete con;
	    delete tru;
	    return 0;
      }

      if (! NetETernary::test_operand_compat(tru->expr_type(), fal->expr_type())) {
	    cerr << get_fileline() << ": error: Data types "
		 << tru->expr_type() << " and "
		 << fal->expr_type() << " of ternary"
		 << " do not match." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetETernary*res = new NetETernary(con, tru, fal, expr_wid, signed_flag_);
      res->set_line(*this);
      return res;
}

/*
 * When elaborating the true or false alternative expression of a
 * ternary, take into account the overall expression type. If the type
 * is not vectorable, then the alternative expression is evaluated as
 * self-determined.
 */
NetExpr* PETernary::elab_and_eval_alternative_(Design*des, NetScope*scope,
					       PExpr*expr, unsigned expr_wid,
                                               unsigned flags, bool short_cct) const
{
      int context_wid = expr_wid;
      if (type_is_vectorable(expr->expr_type()) && !type_is_vectorable(expr_type_)) {
	    expr_wid = expr->expr_width();
            context_wid = -1;
      } else {
            expr->cast_signed(signed_flag_);
      }
      NetExpr*tmp = expr->elaborate_expr(des, scope, expr_wid, flags);
      if (tmp == 0) return 0;

      if (short_cct && (expr_type_ == IVL_VT_REAL)
          && (expr->expr_type() != IVL_VT_REAL))
	    tmp = cast_to_real(tmp);

      eval_expr(tmp, context_wid);

      return tmp;
}

/*
 * A typename expression is only legal in very narrow cases. This is
 * just a placeholder.
 */
unsigned PETypename::test_width(Design*des, NetScope*, width_mode_t&)
{
      cerr << get_fileline() << ": error: "
	   << "Type names are not valid expressions here." << endl;
      des->errors += 1;

      expr_type_   = IVL_VT_NO_TYPE;
      expr_width_  = 1;
      min_width_   = 1;
      signed_flag_ = false;
      return expr_width_;
}

NetExpr*PETypename::elaborate_expr(Design*des, NetScope*,
				   ivl_type_t, unsigned) const
{
      cerr << get_fileline() << ": error: Type name not a valid expression here." << endl;
      des->errors += 1;
      return 0;
}

unsigned PEUnary::test_width(Design*des, NetScope*scope, width_mode_t&mode)
{
      switch (op_) {
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	  case '!':
	    {
		  width_mode_t sub_mode = SIZED;
		  unsigned sub_width = expr_->test_width(des, scope, sub_mode);

		  expr_type_   = expr_->expr_type();
	          expr_width_  = 1;
	          min_width_   = 1;
                  signed_flag_ = false;

                  if ((op_ == '!') && (expr_type_ != IVL_VT_BOOL))
                        expr_type_ = IVL_VT_LOGIC;

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Test width of sub-expression of " << op_
			     << " returns " << sub_width << "." << endl;

	    }
            return expr_width_;
      }

      expr_width_  = expr_->test_width(des, scope, mode);
      expr_type_   = expr_->expr_type();
      min_width_   = expr_->min_width();
      signed_flag_ = expr_->has_sign();

      return fix_width_(mode);
}


NetExpr* PEUnary::elaborate_expr(Design*des, NetScope*scope,
				 unsigned expr_wid, unsigned flags) const
{
      flags &= ~SYS_TASK_ARG; // don't propagate the SYS_TASK_ARG flag
      ivl_variable_type_t t;

      unsigned sub_width = expr_wid;
      switch (op_) {
            // Reduction operators and ! always have a self determined width.
	  case '!':
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    sub_width = expr_->expr_width();
	    break;

            // Other operators have context determined operands, so propagate
            // the expression type (signed/unsigned) down to the operands.
	  default:
            expr_->cast_signed(signed_flag_);
	    break;
      }
      NetExpr*ip = expr_->elaborate_expr(des, scope, sub_width, flags);
      if (ip == 0) return 0;

      ivl_assert(*expr_, expr_type_ != IVL_VT_NO_TYPE);

      NetExpr*tmp;
      switch (op_) {
	  case 'i':
	  case 'I':
	  case 'D':
	  case 'd':
		t = ip->expr_type();
		if (expr_wid != expr_->expr_width()) {
			/*
			 * TODO: Need to modify draw_unary_expr() to support
			 * increment/decrement operations on slice of vector.
			 */
			cerr << get_fileline() << ": sorry: "
				<< human_readable_op(op_, true)
				<< " operation is not yet supported on "
				<< "vector slice." << endl;
			des->errors += 1;
			return 0;
		} else if (t == IVL_VT_LOGIC || t == IVL_VT_BOOL ||
				t == IVL_VT_REAL) {

			if (dynamic_cast<NetEConst *> (ip) ||
				dynamic_cast<NetECReal*> (ip)) {
				/*
				 * invalid operand: operand is a constant
				 * or real number
				 */
				cerr << get_fileline() << ": error: "
					<< "inappropriate use of "
					<< human_readable_op(op_, true)
					<< " operator." << endl;
				des->errors += 1;
				return 0;
			}

			/*
			 * **** Valid use of operator ***
			 * For REAL variables draw_unary_real() is invoked during
			 * evaluation and for LOGIC/BOOLEAN draw_unary_expr()
			 * is called for evaluation.
			 */
			tmp = new NetEUnary(op_, ip, expr_wid, signed_flag_);
			tmp->set_line(*this);
		} else {
			cerr << get_fileline() << ": error: "
				<< "inappropriate use of "
				<< human_readable_op(op_, true)
				<< " operator." << endl;
			des->errors += 1;
			return 0;
		}
		break;

	  default:
	    tmp = new NetEUnary(op_, ip, expr_wid, signed_flag_);
	    tmp->set_line(*this);
	    break;

	  case '-':
	    if (NetEConst*ipc = dynamic_cast<NetEConst*>(ip)) {

		  verinum val = - ipc->value();
		  tmp = new NetEConst(val);
		  tmp->cast_signed(signed_flag_);
		  tmp->set_line(*this);
		  delete ip;

	    } else if (NetECReal*ipr = dynamic_cast<NetECReal*>(ip)) {

		    /* When taking the - of a real, fold this into the
		       constant value. */
		  verireal val = - ipr->value();
		  tmp = new NetECReal(val);
		  tmp->set_line(*this);
		  delete ip;

	    } else {
		  tmp = new NetEUnary(op_, ip, expr_wid, signed_flag_);
		  tmp->set_line(*this);
	    }
	    break;

	  case '+':
	    tmp = ip;
	    break;

	  case '!': // Logical NOT
	      /* If the operand to unary ! is a constant, then I can
		 evaluate this expression here and return a logical
		 constant in its place. */
	    if (NetEConst*ipc = dynamic_cast<NetEConst*>(ip)) {
		  verinum val = ipc->value();
		  unsigned v1 = 0;
		  unsigned vx = 0;
		  for (unsigned idx = 0 ;  idx < val.len() ;  idx += 1)
			switch (val[idx]) {
			    case verinum::V0:
			      break;
			    case verinum::V1:
			      v1 += 1;
			      break;
			    default:
			      vx += 1;
			      break;
			}

		  verinum::V res;
		  if (v1 > 0)
			res = verinum::V0;
		  else if (vx > 0)
			res = verinum::Vx;
		  else
			res = verinum::V1;

		  verinum vres (res, 1, true);
		  tmp = new NetEConst(vres);
		  tmp->set_line(*this);
		  delete ip;
	    } else if (NetECReal*ipr = dynamic_cast<NetECReal*>(ip)) {
		  verinum::V res;
		  if (ipr->value().as_double() == 0.0) res = verinum::V1;
		  else res = verinum::V0;
		  verinum vres (res, 1, true);
		  tmp = new NetEConst(vres);
		  tmp->set_line(*this);
		  delete ip;
	    } else {
		  if (ip->expr_type() == IVL_VT_REAL) {
			tmp = new NetEBComp('e', ip,
			                    new NetECReal(verireal(0.0)));
		  } else {
			tmp = new NetEUReduce(op_, ip);
		  }
		  tmp->set_line(*this);
	    }
            tmp = pad_to_width(tmp, expr_wid, *this);
	    break;

	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    if (ip->expr_type() == IVL_VT_REAL) {
		  cerr << get_fileline() << ": error: "
		       << human_readable_op(op_, true)
		       << " operator may not have a REAL operand." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    tmp = new NetEUReduce(op_, ip);
	    tmp->set_line(*this);
            tmp = pad_to_width(tmp, expr_wid, *this);
	    break;

	  case '~':
	    tmp = elaborate_expr_bits_(ip, expr_wid);
	    break;
      }

      return tmp;
}

NetExpr* PEUnary::elaborate_expr_bits_(NetExpr*operand, unsigned expr_wid) const
{
	// Handle the special case that the operand is a
	// constant. Simply calculate the constant results of the
	// expression and return that.
      if (NetEConst*ctmp = dynamic_cast<NetEConst*> (operand)) {
	    verinum value = ctmp->value();

	      // The only operand that I know can get here is the
	      // unary not (~).
	    ivl_assert(*this, op_ == '~');
	    value = ~value;

	    ctmp = new NetEConst(value);
	    ctmp->set_line(*this);
	    delete operand;
	    return ctmp;
      }

      NetEUBits*tmp = new NetEUBits(op_, operand, expr_wid, signed_flag_);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEVoid::elaborate_expr(Design*, NetScope*, unsigned, unsigned) const
{
      return 0;
}

NetNet* Design::find_discipline_reference(ivl_discipline_t dis, NetScope*scope)
{
      NetNet*gnd = discipline_references_[dis->name()];

      if (gnd) return gnd;

      string name = string(dis->name()) + "$gnd";
      netvector_t*gnd_vec = new netvector_t(IVL_VT_REAL,0,0);
      gnd = new NetNet(scope, lex_strings.make(name), NetNet::WIRE, gnd_vec);
      gnd->set_discipline(dis);
      discipline_references_[dis->name()] = gnd;

      if (debug_elaborate)
	    cerr << gnd->get_fileline() << ": debug: "
		 << "Create an implicit reference terminal"
		 << " for discipline=" << dis->name()
		 << " in scope=" << scope_path(scope) << endl;

      return gnd;
}
