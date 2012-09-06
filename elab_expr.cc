/*
 * Copyright (c) 1999-2011 Stephen Williams (steve@icarus.com)
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
# include  <typeinfo>
# include  <cstdlib>
# include  <cstring>
# include  <climits>
# include "compiler.h"

# include  "pform.h"
# include  "netlist.h"
# include  "discipline.h"
# include  "netmisc.h"
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

NetExpr* elaborate_rval_expr(Design*des, NetScope*scope,
			     ivl_variable_type_t data_type_lv, int expr_wid_lv,
			     PExpr*expr)
{
      bool unsized_flag     = type_is_vectorable(data_type_lv)? false : true;
      unsigned use_lval_wid = type_is_vectorable(data_type_lv)? expr_wid_lv : 0;
      unsigned use_min_wid  = expr_wid_lv;

	/* Find out what the r-value width is going to be. We
	guess it will be the l-value width, but it may turn
	out to be something else based on self-determined
	widths inside. */
      ivl_variable_type_t rval_type = IVL_VT_NO_TYPE;
      int expr_wid = expr->test_width(des, scope, use_min_wid, use_lval_wid, rval_type, unsized_flag);

      if (debug_elaborate) {
	    cerr << expr->get_fileline() << ": debug: r-value tested "
		 << "type=" << rval_type
		 << ", width=" << expr_wid
		 << ", min=" << use_min_wid
		 << ", unsized_flag=" << (unsized_flag?"true":"false") << endl;
      }

      switch (data_type_lv) {
	  case IVL_VT_REAL:
	    unsized_flag = true;
	    expr_wid = -2;
	    expr_wid_lv = -1;
	    break;
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    break;
	  case IVL_VT_VOID:
	  case IVL_VT_NO_TYPE:
	    ivl_assert(*expr, 0);
	    expr_wid = -2;
	    expr_wid_lv = -1;
	    break;
      }

      NetExpr*result = elab_and_eval(des, scope, expr, expr_wid, expr_wid_lv);
      return result;
}

/*
 * The default behavior for the test_width method is to just return the
 * minimum width that is passed in.
 */
unsigned PExpr::test_width(Design*des, NetScope*scope,
			   unsigned min, unsigned lval,
			   ivl_variable_type_t&, bool&)
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: test_width defaults to "
		 << min << ", ignoring unsized_flag. typeid="
		 << typeid(*this).name() << endl;
      }
      return min;
}

NetExpr* PExpr::elaborate_expr(Design*des, NetScope*, int, bool) const
{
      cerr << get_fileline() << ": internal error: I do not know how to elaborate"
	   << " expression. " << endl;
      cerr << get_fileline() << ":               : Expression is: " << *this
	   << endl;
      des->errors += 1;
      return 0;
}

unsigned PEBinary::test_width(Design*des, NetScope*scope,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&expr_type__,
			      bool&unsized_flag)
{
      ivl_variable_type_t expr_type_left = IVL_VT_NO_TYPE;
      ivl_variable_type_t expr_type_right= IVL_VT_NO_TYPE;

      bool flag = unsized_flag;

      bool flag_left = flag;
      unsigned wid_left = left_->test_width(des,scope, min, 0, expr_type_left, flag_left);

      bool flag_right = flag;
      unsigned wid_right = right_->test_width(des,scope, min, 0, expr_type_right, flag_right);

      if (flag_right && !flag) {
	    unsized_flag = true;
	    wid_left = left_->test_width(des, scope, min, 0, expr_type_left, unsized_flag);
      }

      if (flag_left && !flag) {
	    unsized_flag = true;
	    wid_right = right_->test_width(des, scope, min, 0, expr_type_right, unsized_flag);
      }

      if (expr_type_left == IVL_VT_REAL || expr_type_right == IVL_VT_REAL)
	    expr_type_ = IVL_VT_REAL;
      else if (expr_type_left==IVL_VT_LOGIC || expr_type_right==IVL_VT_LOGIC)
	    expr_type_ = IVL_VT_LOGIC;
      else
	    expr_type_ = IVL_VT_BOOL;

      switch (op_) {
	  case '+':
	  case '-':
	    if (wid_left > min)
		  min = wid_left;
	    if (wid_right > min)
		  min = wid_right;
	    if (unsized_flag && type_is_vectorable(expr_type_))
		  min += 1;
	    break;

	  case '*':
	    if (unsized_flag && type_is_vectorable(expr_type_)) {
		  unsigned use_wid = wid_left + wid_right;
                  if (use_wid > integer_width)
                        use_wid = integer_width;
                  if (use_wid > min)
                        min = use_wid;
	    }
	    if (wid_left > min)
		  min = wid_left;
	    if (wid_right > min)
		  min = wid_right;
	    break;

	  case 'l': // <<  Should be handled by PEBShift
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
	    if (wid_left > min)
		  min = wid_left;
	    if (wid_right > min)
		  min = wid_right;
	    break;
      }

      if (type_is_vectorable(expr_type_))
	    expr_width_ = min;
      else
	    expr_width_ = 1;

      expr_type__ = expr_type_;
      return expr_width_;
}

/*
 * Elaborate binary expressions. This involves elaborating the left
 * and right sides, and creating one of a variety of different NetExpr
 * types.
 */
NetExpr* PEBinary::elaborate_expr(Design*des, NetScope*scope,
				     int expr_wid, bool) const
{
      assert(left_);
      assert(right_);

      NetExpr*lp = left_->elaborate_expr(des, scope, expr_wid, false);
      NetExpr*rp = right_->elaborate_expr(des, scope, expr_wid, false);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

	// Handle the special case that one of the operands is a real
	// value and the other is a vector type. In that case,
	// re-elaborate the vectorable argument as self-determined
	// lossless.
      if (lp->expr_type()==IVL_VT_REAL
	  && type_is_vectorable(rp->expr_type())
	  && expr_wid != -2) {
	    delete rp;
	    rp = right_->elaborate_expr(des, scope, -2, false);
      }

      if (rp->expr_type()==IVL_VT_REAL
	  && type_is_vectorable(lp->expr_type())
	  && expr_wid != -2) {
	    delete lp;
	    lp = left_->elaborate_expr(des, scope, -2, false);
      }

      NetExpr*tmp = elaborate_eval_expr_base_(des, lp, rp, expr_wid);
      return tmp;
}

void suppress_binary_operand_sign_if_needed(NetExpr*lp, NetExpr*rp)
{
	// If an argument is a non-vector type, then this type
	// suppression does not apply.
      if (! type_is_vectorable(lp->expr_type()))
	    return;
      if (! type_is_vectorable(rp->expr_type()))
	    return;

	// If either operand is unsigned, then treat the whole
	// expression as unsigned. This test needs to be done here
	// instead of in *_expr_base_ because it needs to be done
	// ahead of any subexpression evaluation (because they need to
	// know their signedness to evaluate) and because there are
	// exceptions to this rule.
      if (! lp->has_sign())
	    rp->cast_signed(false);
      if (! rp->has_sign())
	    lp->cast_signed(false);
}

NetExpr* PEBinary::elaborate_eval_expr_base_(Design*des,
						NetExpr*lp,
						NetExpr*rp,
						int expr_wid) const
{
	/* If either expression can be evaluated ahead of time, then
	   do so. This can prove helpful later. */
      eval_expr(lp, expr_wid);
      eval_expr(rp, expr_wid);

      return elaborate_expr_base_(des, lp, rp, expr_wid);
}

/*
 * This is common elaboration of the operator. It presumes that the
 * operands are elaborated as necessary, and all I need to do is make
 * the correct NetEBinary object and connect the parameters.
 */
NetExpr* PEBinary::elaborate_expr_base_(Design*des,
					NetExpr*lp, NetExpr*rp,
					int expr_wid, bool is_pexpr) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: elaborate expression "
		 << *this << " expr_wid=" << expr_wid << endl;
      }

      NetExpr*tmp;

      switch (op_) {
	  default:
	    tmp = new NetEBinary(op_, lp, rp);
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
	    tmp = new NetEBPow(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '*':
	    tmp = elaborate_expr_base_mult_(des, lp, rp, expr_wid, is_pexpr);
	    break;

	  case '%':
	  case '/':
	    tmp = elaborate_expr_base_div_(des, lp, rp, expr_wid, is_pexpr);
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
	    tmp = elaborate_expr_base_add_(des, lp, rp, expr_wid, is_pexpr);
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
	    tmp = new NetEBMinMax(op_, lp, rp);
	    tmp->set_line(*this);
	    break;
      }

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_bits_(Design*des,
					     NetExpr*lp, NetExpr*rp,
					     int expr_wid) const
{
      if (lp->expr_type() == IVL_VT_REAL || rp->expr_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: "
	         << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

	// If either of the arguments is unsigned, then process both
	// of them as unsigned. This only impacts the padding that is
	// done to get the operands to the expr_wid.
      if (! lp->has_sign()) rp->cast_signed(false);
      if (! rp->has_sign()) lp->cast_signed(false);

      if (expr_wid > 0) {
	    if (type_is_vectorable(lp->expr_type()))
		  lp = pad_to_width(lp, expr_wid, *this);
	    if (type_is_vectorable(rp->expr_type()))
		  rp = pad_to_width(rp, expr_wid, *this);
      }

      NetEBBits*tmp = new NetEBBits(op_, lp, rp);
      tmp->set_line(*this);

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_div_(Design*des,
					    NetExpr*lp, NetExpr*rp,
					    int expr_wid, bool is_pexpr) const
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

	// If either of the arguments is unsigned, then process both
	// of them as unsigned. This only impacts the padding that is
	// done to get the operands to the expr_wid.
      if (! lp->has_sign()) rp->cast_signed(false);
      if (! rp->has_sign()) lp->cast_signed(false);

	/* The original elaboration of the left and right expressions
	   already tried to elaborate to the expr_wid. If the
	   expressions are not that width by now, then they need to be
	   padded. The divide expression operands must be the width
	   of the output. */
      if (expr_wid > 0) {
	    lp = pad_to_width(lp, expr_wid, *this);
	    rp = pad_to_width(rp, expr_wid, *this);
      }

      NetEBDiv*tmp = new NetEBDiv(op_, lp, rp);
      tmp->set_line(*this);

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_lshift_(Design*des,
					       NetExpr*lp, NetExpr*rp,
					       int expr_wid) const
{
      if (lp->expr_type() == IVL_VT_REAL || rp->expr_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: "
	         << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*tmp;

      long use_wid = lp->expr_width();
      if (expr_wid > 0)
	    use_wid = expr_wid;

      if (use_wid == 0) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Oops, left expression width is not known, "
		       << "so expression width is not known. Punt." << endl;
	    tmp = new NetEBShift(op_, lp, rp);
	    tmp->set_line(*this);
	    return tmp;
      }

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

		  tmp = make_const_0(use_wid);
		  tmp->set_line(*this);
		  return tmp;
	    }

	    if (NetEConst*rpc = dynamic_cast<NetEConst*> (rp)) {
		    // Handle the super-special case that both
		    // operands are constants. Precalculate the
		    // entire value here.
                  if (!rpc->value().is_defined()) {
		        tmp = make_const_x(use_wid);
		        tmp->set_line(*this);
		        return tmp;
                  }
		  verinum lpval = lpc->value();
		  unsigned shift = rpc->value().as_ulong();
		  verinum result = lpc->value() << shift;
		    // If the l-value has explicit size, or
		    // there is a context determined size, use that.
		  if (lpval.has_len() || expr_wid > 0) {
			int use_len = lpval.len();
			if (expr_wid > 0 && expr_wid > use_len)
			      use_len = expr_wid;
			result = verinum(result, use_len);
		  }

		  tmp = new NetEConst(result);
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Precalculate " << *lpc << " << " << shift
			     << " to constant " << *tmp
			     << " (expr_wid=" << expr_wid << ")" << endl;

	    } else {
		    // Handle the special case that the left
		    // operand is constant. If it is unsized, we
		    // may have to expand it to an integer width.
		  verinum lpval = lpc->value();
		  if (lpval.len() < integer_width && !lpval.has_len()) {
			lpval = verinum(lpval, integer_width);
			lpc = new NetEConst(lpval);
			lpc->set_line(*lp);
		  }

		  tmp = new NetEBShift(op_, lpc, rp);
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Adjust " << *this
			     << " to this " << *tmp
			     << " to allow for integer widths." << endl;
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
                  tmp->set_line(*this);
                  return tmp;
            }

	    long shift = rpc->value().as_long();
	    use_wid = lp->expr_width();
	    if (expr_wid > 0)
		  use_wid = expr_wid;

	    if (shift >= use_wid || (-shift) >= (long)lp->expr_width()) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Value left-shifted " << shift
			     << " beyond width of " << use_wid
			     << ". Elaborate as constant zero." << endl;

		  tmp = make_const_0(use_wid);

	    } else {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Left shift expression by constant "
			     << shift << " bits. (use_wid=" << use_wid << ")" << endl;
		  lp = pad_to_width(lp, use_wid, *this);
		  tmp = new NetEBShift(op_, lp, rp);
	    }

      } else {
	      // Left side is not constant, so handle it the
	      // default way.
	    if (expr_wid >= 0)
		  lp = pad_to_width(lp, expr_wid, *this);

            rp = new NetESelect(rp, 0, rp->expr_width());
            rp->cast_signed(false);
            rp->set_line(*this);

	    tmp = new NetEBShift(op_, lp, rp);
      }

      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_rshift_(Design*des,
					       NetExpr*lp, NetExpr*rp,
					       int expr_wid) const
{
      if (lp->expr_type() == IVL_VT_REAL || rp->expr_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: "
	         << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetExpr*tmp;

      long use_wid = lp->expr_width();
      if (expr_wid > 0)
	    use_wid = expr_wid;

      if (use_wid == 0) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Oops, left expression width is not known, "
		       << "so expression width is not known. Punt." << endl;
	    tmp = new NetEBShift(op_, lp, rp);
	    tmp->set_line(*this);
	    return tmp;
      }

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

		  tmp = make_const_0(use_wid);
		  tmp->set_line(*this);
		  return tmp;
	    }
      }

      if (NetEConst*rpc = dynamic_cast<NetEConst*> (rp)) {
              // Special case: The shift value contains 'x' or 'z' bits.
              // Elaborate as a constant-x.
            if (!rpc->value().is_defined()) {

                  if (debug_elaborate)
                        cerr << get_fileline() << ": debug: "
                             << "Shift by undefined value. "
                             << "Elaborate as constant 'x'." << endl;

                  tmp = make_const_x(expr_wid);
                  tmp->set_line(*this);
                  return tmp;
            }

	    unsigned long shift = rpc->value().as_ulong();

	      // Special case: The shift is the size of the entire
	      // left operand, and the shift is unsigned. Elaborate as
	      // a constant-0.
	    if ((op_=='r' || (lp->has_sign()==false)) &&
		shift >= lp->expr_width()) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Value right-shifted " << shift
			     << " beyond width of " << lp->expr_width()
			     << ". Elaborate as constant zero." << endl;

		  tmp = make_const_0(use_wid);
		  tmp->set_line(*this);
		  return tmp;

	    }

	      // Special case: the shift is the size of the entire
	      // left operand, and the shift is signed. Elaborate as a
	      // replication of the top bit of the left expression.
	      //
	      // The above test assures us that op_ == 'R' && the left
	      // argument is signed when the shift is greater than the
	      // expression width.
	    if (shift >= lp->expr_width()) {

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Value signed-right-shifted " << shift
			     << " beyond width of " << lp->expr_width()
			     << ". Elaborate as replicated top bit." << endl;

		  ivl_assert(*this, lp->expr_width() > 0);
		  ivl_assert(*this, use_wid > 0);

		  tmp = new NetEConst(verinum(lp->expr_width()-1));
		  tmp->set_line(*this);
		  tmp = new NetESelect(lp, tmp, 1);
		  tmp->cast_signed(true);
		  tmp = pad_to_width(tmp, use_wid, *this);
		  return tmp;
	    }
      }

	// Fallback, handle the general case.
      if (expr_wid > 0)
	    lp = pad_to_width(lp, expr_wid, *this);

      rp = new NetESelect(rp, 0, rp->expr_width());
      rp->cast_signed(false);
      rp->set_line(*this);

      tmp = new NetEBShift(op_, lp, rp);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_mult_(Design*des,
					     NetExpr*lp, NetExpr*rp,
					     int expr_wid, bool is_pexpr) const
{
	// First, Make sure that signed arguments are padded to the
	// width of the output. This is necessary for 2s complement
	// multiplication to come out right.
      if (expr_wid > 0) {
	    if (lp->has_sign() && lp->expr_type() != IVL_VT_REAL)
		  lp = pad_to_width(lp, expr_wid, *this);
	    if (rp->has_sign() && rp->expr_type() != IVL_VT_REAL)
		  rp = pad_to_width(rp, expr_wid, *this);
      }

	// Keep constants on the right side.
      if (dynamic_cast<NetEConst*>(lp)) {
	    NetExpr*tmp = lp;
	    lp = rp;
	    rp = tmp;
      }

	// Handle a few special case multiplies against constants.
      if (NetEConst*rp_const = dynamic_cast<NetEConst*> (rp)) {
	    verinum rp_val = rp_const->value();

	    int use_wid = expr_wid;
	    if (use_wid < 0)
		  use_wid = max(rp->expr_width(), lp->expr_width());

	    if (! rp_val.is_defined()) {
		  NetEConst*tmp = make_const_x(use_wid);
		  return tmp;
	    }

	    if (rp_val.is_zero()) {
		  NetEConst*tmp = make_const_0(use_wid);
		  return tmp;
	    }
      }

	// If this expression is unsigned, then make sure the
	// arguments are unsigned so that the padding below doesn't
	// cause any sign extension to happen.
      if (! is_pexpr)
	    suppress_binary_operand_sign_if_needed(lp, rp);


	// Multiply will guess a width that is the sum of the
	// widths of the operand. If that sum is too small, then
	// pad one of the arguments enough that the sum is the
	// desired width.
      if (expr_wid > (long)(lp->expr_width() + rp->expr_width()))
	    lp = pad_to_width(lp, expr_wid - rp->expr_width(), *this);

      NetEBMult*tmp = new NetEBMult(op_, lp, rp);
      tmp->set_line(*this);

      if (expr_wid > 0 && type_is_vectorable(tmp->expr_type()))
	    tmp->set_width(expr_wid, false);

      return tmp;
}

NetExpr* PEBinary::elaborate_expr_base_add_(Design*des,
					    NetExpr*lp, NetExpr*rp,
					    int expr_wid, bool is_pexpr) const
{
      NetExpr*tmp;
      bool use_lossless_flag = expr_wid == -2;

	// If this expression is not vectorable, then do NOT pass the
	// lossless flag to the NetEBAdd constructor. For non-
	// vectorable, lossless is implicit.
      if (! type_is_vectorable(lp->expr_type()))
	    use_lossless_flag = false;
      if (! type_is_vectorable(rp->expr_type()))
	    use_lossless_flag = false;

	// If the expression is unsigned, then force the operands to
	// unsigned so that the set_width below doesn't cause them to
	// be sign-extended.
      if (! is_pexpr)
	    suppress_binary_operand_sign_if_needed(lp, rp);

      tmp = new NetEBAdd(op_, lp, rp, use_lossless_flag);
      if (expr_wid > 0 && type_is_vectorable(tmp->expr_type()))
	    tmp->set_width(expr_wid);

      tmp->set_line(*this);
      return tmp;
}

unsigned PEBComp::test_width(Design*des, NetScope*scope, unsigned, unsigned,
			     ivl_variable_type_t&my_expr_type,
			     bool&)
{
	// The width and type of a comparison operator is fixed and
	// well known. Set them now.
      expr_type_ = IVL_VT_LOGIC;
      my_expr_type = expr_type_;
      expr_width_ = 1;

	// The widths of operands are self-determined, but need to be
	// figured out.
      bool unsized_flag = false;
      ivl_variable_type_t left_type = IVL_VT_NO_TYPE;
      unsigned left_width = left_->test_width(des, scope, 0, 0, left_type, unsized_flag);
      bool left_unsized_flag = unsized_flag;
      ivl_variable_type_t right_type = IVL_VT_NO_TYPE;
      unsigned right_width = right_->test_width(des, scope, 0, 0, right_type, unsized_flag);

      if (left_unsized_flag != unsized_flag) {
	    left_width = left_->test_width(des, scope, 0, 0, left_type, unsized_flag);
      }

      int try_wid_l = left_width;
      if (type_is_vectorable(left_type) && (right_width > left_width))
	    try_wid_l = right_width;

      int try_wid_r = right_width;
      if (type_is_vectorable(right_type) && (left_width > right_width))
	    try_wid_r = left_width;

	// If the expression is unsized and smaller then the integer
	// minimum, then tweak the size up.
	// NOTE: I really would rather try to figure out what it would
	// take to get expand the sub-expressions so that they are
	// exactly the right width to behave just like infinite
	// width. I suspect that adding 1 more is sufficient in all
	// cases, but I'm not certain. Ideas?
      if (type_is_vectorable(left_type) && unsized_flag && try_wid_l<(int)integer_width)
	    try_wid_l += 1;
      if (type_is_vectorable(right_type) && unsized_flag && try_wid_r<(int)integer_width)
	    try_wid_r += 1;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: "
		 << "Comparison expression operands are "
		 << left_width << " bits and "
		 << right_width << " bits. Resorting to "
		 << try_wid_l << " bits and "
		 << try_wid_r << " bits." << endl;
      }

      left_width_ = try_wid_l;
      right_width_ = try_wid_r;

      return 1;
}

NetExpr* PEBComp::elaborate_expr(Design*des, NetScope*scope,
				 int expr_width_dummy, bool sys_task_arg) const
{
      assert(left_);
      assert(right_);

      NetExpr*lp = left_->elaborate_expr(des, scope, left_width_, false);
      NetExpr*rp = right_->elaborate_expr(des, scope, right_width_, false);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      suppress_binary_operand_sign_if_needed(lp, rp);

	// The arguments of a compare need to have matching widths, so
	// pad the width here. This matters because if the arguments
	// are signed, then this padding will do sign extension.
      if (type_is_vectorable(lp->expr_type()))
	    lp = pad_to_width(lp, left_width_, *this);
      if (type_is_vectorable(rp->expr_type()))
	    rp = pad_to_width(rp, right_width_, *this);

      eval_expr(lp, left_width_);
      eval_expr(rp, right_width_);

	// Handle some operand-specific special cases...
      switch (op_) {
	  case 'E': /* === */
	  case 'N': /* !== */
	    if (lp->expr_type() == IVL_VT_REAL ||
		rp->expr_type() == IVL_VT_REAL) {
		  cerr << get_fileline() << ": error: "
		       << human_readable_op(op_)
		       << " operator may not have REAL operands." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    break;
	  default:
	    break;
      }

      NetEBComp*tmp = new NetEBComp(op_, lp, rp);
      tmp->set_line(*this);
      bool flag = tmp->set_width(1);
      if (flag == false) {
	    cerr << get_fileline() << ": internal error: "
		  "expression bit width of comparison != 1." << endl;
	    des->errors += 1;
      }

      return tmp;
}

unsigned PEBLogic::test_width(Design*des, NetScope*scope,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&expr_type_out,
			      bool&unsized_flag)
{
      expr_type_ = IVL_VT_LOGIC;
      expr_width_ = 1;
      expr_type_out = expr_type_;
      return expr_width_;
}

NetExpr*PEBLogic::elaborate_expr(Design*des, NetScope*scope,
				 int expr_width_dummy, bool sys_task_arg) const
{
      assert(left_);
      assert(right_);

	// The left and right expressions are self-determined and
	// independent. Run their test_width methods independently. We
	// don't need the widths here, but we do need the expressions
	// to calculate their self-determined width and type.

      bool left_flag = false;
      ivl_variable_type_t left_type = IVL_VT_NO_TYPE;
      left_->test_width(des, scope, 0, 0, left_type, left_flag);

      bool right_flag = false;
      ivl_variable_type_t right_type = IVL_VT_NO_TYPE;
      right_->test_width(des, scope, 0, 0, right_type, right_flag);

      NetExpr*lp = elab_and_eval(des, scope, left_, -1);
      NetExpr*rp = elab_and_eval(des, scope, right_, -1);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      lp = condition_reduce(lp);
      rp = condition_reduce(rp);

      NetEBLogic*tmp = new NetEBLogic(op_, lp, rp);
      tmp->set_line(*this);
      tmp->set_width(1);

      return tmp;
}

unsigned PEBLeftWidth::test_width(Design*des, NetScope*scope,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&expr_type__,
			      bool&unsized_flag)
{
      unsigned wid_left = left_->test_width(des,scope,min, lval, expr_type__, unsized_flag);

	// The right expression is self-determined and has no impact
	// on the expression size that is generated.

      if (wid_left < min)
	    wid_left = min;
      if (wid_left < lval)
	    wid_left = lval;

      if (unsized_flag
	  && type_is_vectorable(expr_type__)
	  && wid_left > 0
	  && wid_left < integer_width) {
	    wid_left = integer_width;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Test width of unsized " << human_readable_op(op_)
		       << " is padded to compiler integer width=" << wid_left
		       << endl;
      }

      expr_type_ = expr_type__;
      expr_width_ = wid_left;

	// Run a test-width on the shift amount so that its types are
	// worked out for elaboration later on. We don't need the
	// value now.
      ivl_variable_type_t rtype = IVL_VT_NO_TYPE;
      bool rflag = false;
      unsigned wid_right = right_->test_width(des, scope, 0, 0, rtype, rflag);
      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: "
		 << "Test width of exponent of " << op_ << " expression "
		 << "returns wid=" << wid_right << ", type=" << rtype
		 << ", flag=" << rflag << endl;

      return expr_width_;
}

NetExpr*PEBLeftWidth::elaborate_expr(Design*des, NetScope*scope,
				     int expr_wid, bool sys_task_arg) const
{
      assert(left_);
      assert(right_);

      NetExpr*lp = left_->elaborate_expr(des, scope, expr_wid, false);
      if (expr_wid > 0 && lp->expr_width() < (unsigned)expr_wid) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Pad left operand of " << human_readable_op(op_)
		       << " to " << expr_wid << "." << endl;
	    lp = pad_to_width(lp, expr_wid, *this);
      }

      NetExpr*rp = right_->elaborate_expr(des, scope, -1, false);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      eval_expr(lp);
      eval_expr(rp);

      return elaborate_expr_leaf(des, lp, rp, expr_wid);
}

NetExpr*PEBPower::elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
				      int expr_wid) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: elaborate expression "
		 << *this << " expr_wid=" << expr_wid << endl;
      }

      NetExpr*tmp = new NetEBPow(op_, lp, rp);
      tmp->set_line(*this);

      return tmp;
}

NetExpr*PEBShift::elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
				      int expr_wid) const
{
      NetExpr*tmp = 0;

      switch (op_) {
	  case 'l':
	    tmp = elaborate_expr_base_lshift_(des, lp, rp, expr_wid);
	    break;

	  case 'r': // >>
	  case 'R': // >>>
	    tmp = elaborate_expr_base_rshift_(des, lp, rp, expr_wid);
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected opcode " << human_readable_op(op_)
		 << " in PEBShift::elaborate_expr_leaf." << endl;
	    des->errors += 1;
      }

      return tmp;
}

unsigned PECallFunction::test_width_sfunc_(Design*des, NetScope*scope,
					   unsigned min, unsigned lval,
					   ivl_variable_type_t&expr_type__,
					   bool&unsized_flag)
{
      perm_string name = peek_tail_name(path_);

      if (name=="$signed"|| name=="$unsigned") {
	    PExpr*expr = parms_[0];
	    if (expr == 0)
		  return 0;

              // The argument width is self-determined.
	    expr_width_ = expr->test_width(des, scope, 0, 0, expr_type__, unsized_flag);
	    expr_type_ = expr_type__;

              // The result width is context dependent.
            if (expr_width_ > min)
                   min = expr_width_;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: $signed/$unsigned"
		       << " argument width = " << expr_width_
		       << ", result width = " << min << "." << endl;

            return min;
      }

	// Run through the arguments of the system function and make
	// sure their widths/types are calculated. They are all self-
	// determined.
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
	    PExpr*expr = parms_[idx];
	    ivl_variable_type_t sub_type = IVL_VT_NO_TYPE;
	    bool flag = false;
	    unsigned wid = expr->test_width(des,scope,0,0,sub_type,flag);
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width"
		       << " of " << name << " argument " << idx+1
		       << " returns type=" << sub_type
		       << ", wid=" << wid << endl;
      }

      if (name=="$sizeof" || name=="$bits") {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width"
		       << " of $sizeof/$bits returns test_width"
		       << " of compiler integer." << endl;

	    expr_type_ = IVL_VT_BOOL;
	    expr_width_= integer_width;

	    expr_type__ = expr_type_;
	    return expr_width_;
      }

      if (name=="$is_signed") {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width"
		       << " of $is_signed returns test_width"
		       << " of 1." << endl;

	    expr_type_  = IVL_VT_BOOL;
	    expr_width_ = 1;
	    expr_type__  = expr_type_;
	    return expr_width_;
      }

	/* Get the return type of the system function by looking it up
	   in the sfunc_table. */
      const struct sfunc_return_type*sfunc_info
	    = lookup_sys_func(peek_tail_name(path_));

      expr_type_  = sfunc_info->type;
      expr_width_ = sfunc_info->wid;

      expr_type__ = expr_type_;

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: test_width "
		 << "of system function " << name
		 << " returns wid=" << expr_width_
		 << ", type=" << expr_type_ << "." << endl;

      return expr_width_;
}

unsigned PECallFunction::test_width(Design*des, NetScope*scope,
				    unsigned min, unsigned lval,
				    ivl_variable_type_t&expr_type__,
				    bool&unsized_flag)
{
      if (peek_tail_name(path_)[0] == '$')
	    return test_width_sfunc_(des, scope, min, lval, expr_type__, unsized_flag);

	// The width of user defined functions depends only on the
	// width of the return value. The arguments are entirely
	// self-determined.
      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	      // If this is an access function, then the width and
	      // type are known by definition.
	    if (find_access_function(path_)) {
		  expr_type_ = IVL_VT_REAL;
		  expr_width_ = 1;
		  expr_type__ = expr_type_;
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
	    expr_type_ = res->data_type();
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: test_width "
		       << "of function returns width " << res->vector_width()
		       << ", type=" << expr_type_
		       << "." << endl;

	    if (! type_is_vectorable(expr_type__))
		  unsized_flag = true;

	    expr_width_ = res->vector_width();
	    expr_type__ = expr_type_;
	    return expr_width_;
      }

      ivl_assert(*this, 0);
      return 0;
}

NetExpr*PECallFunction::cast_to_width_(NetExpr*expr, int wid, bool signed_flag) const
{
        /* If the expression is a const, then replace it with a new
           const. This is a more efficient result. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
	      /* If this is an unsized signed constant then we may need
	       * to extend it up to integer_width. */
	    if (! tmp->has_width() && tmp->has_sign() && (wid > 0)) {
		  unsigned pwidth = tmp->expr_width();
		  if ((unsigned)wid > integer_width) {
			if (integer_width > pwidth) pwidth = integer_width;
		  } else {
			if ((unsigned)wid > pwidth) pwidth = wid;
		  }
		  tmp = dynamic_cast<NetEConst*>(pad_to_width(tmp, pwidth,
		                                              *expr));
		  assert(tmp);
	    }
            tmp->cast_signed(signed_flag);
            if (wid > (int)(tmp->expr_width())) {
                  verinum oval = pad_to_width(tmp->value(), wid);
                  tmp = new NetEConst(oval);
                  tmp->set_line(*this);
                  delete expr;
            }
            return tmp;
      }

      if (wid < 0)
            wid = expr->expr_width();

      if (debug_elaborate)
            cerr << get_fileline() << ": debug: cast to " << wid
                 << " bits" << endl;

      NetESelect*tmp = new NetESelect(expr, 0, wid);
      tmp->set_line(*this);
      tmp->cast_signed(signed_flag);
      return tmp;
}

/*
 * Given a call to a system function, generate the proper expression
 * nodes to represent the call in the netlist. Since we don't support
 * size_tf functions, make assumptions about widths based on some
 * known function names.
 */
NetExpr* PECallFunction::elaborate_sfunc_(Design*des, NetScope*scope, int expr_wid) const
{

	/* Catch the special case that the system function is the $signed
	   function. Its argument will be evaluated as a self-determined
           expression. */
      if (strcmp(peek_tail_name(path_), "$signed") == 0) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $signed() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetExpr*sub = expr->elaborate_expr(des, scope, expr_width_, true);

	    return cast_to_width_(sub, expr_wid, true);
      }
        /* As above, for the $unsigned function. */
      if (strcmp(peek_tail_name(path_), "$unsigned") == 0) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $unsigned() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetExpr*sub = expr->elaborate_expr(des, scope, expr_width_, true);

	    return cast_to_width_(sub, expr_wid, false);
      }

	/* Interpret the internal $sizeof system function to return
	   the bit width of the sub-expression. The value of the
	   sub-expression is not used, so the expression itself can be
	   deleted. */
      if ((strcmp(peek_tail_name(path_), "$sizeof") == 0)
	  || (strcmp(peek_tail_name(path_), "$bits") == 0)) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $bits() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (strcmp(peek_tail_name(path_), "$sizeof") == 0)
		  cerr << get_fileline() << ": warning: $sizeof is deprecated."
		       << " Use $bits() instead." << endl;

	    PExpr*expr = parms_[0];
	    ivl_assert(*this, expr);

	      /* Elaborate the sub-expression to get its
		 self-determined width, and save that width. Then
		 delete the expression because we don't really want
		 the expression itself. */
	    long sub_expr_width = 0;
	    if (NetExpr*tmp = expr->elaborate_expr(des, scope, -1, true)) {
		  sub_expr_width = tmp->expr_width();
		  delete tmp;
	    }

	    verinum val ( (uint64_t)sub_expr_width, 8*sizeof(unsigned));
	    NetEConst*sub = new NetEConst(val);
	    sub->set_line(*this);

	    return sub;
      }

	/* Interpret the internal $is_signed system function to return
	   a single bit flag -- 1 if the expression is signed, 0
	   otherwise. The subexpression is elaborated but not
	   evaluated. */
      if (strcmp(peek_tail_name(path_), "$is_signed") == 0) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $is_signed() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetExpr*sub = expr->elaborate_expr(des, scope, -1, true);

	    verinum val (sub->has_sign()? verinum::V1 : verinum::V0, 1);
	    delete sub;

	    sub = new NetEConst(val);
	    sub->set_line(*this);

	    return sub;
      }

	/* Get the return type of the system function by looking it up
	   in the sfunc_table. */
      const struct sfunc_return_type*sfunc_info
	    = lookup_sys_func(peek_tail_name(path_));

      ivl_variable_type_t sfunc_type = sfunc_info->type;
      unsigned wid = sfunc_info->wid;


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

      NetESFunc*fun = new NetESFunc(peek_tail_name(path_), sfunc_type,
				    wid, nparms);
      fun->set_line(*this);
      if (sfunc_info->signed_flag)
	    fun->cast_signed(true);

	/* Now run through the expected parameters. If we find that
	   there are missing parameters, print an error message.

	   While we're at it, try to evaluate the function parameter
	   expression as much as possible, and use the reduced
	   expression if one is created. */

      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < nparms ;  idx += 1) {
	    PExpr*expr = parms_[idx];
	    if (expr) {
		  NetExpr*tmp1 = expr->elaborate_expr(des, scope, -1, true);
		  eval_expr(tmp1);
		  fun->parm(idx, tmp1);

	    } else {
		  missing_parms += 1;
		  fun->parm(idx, 0);
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_fileline() << ": error: The function "
		 << peek_tail_name(path_)
		 << " has been called with empty parameters." << endl;
	    cerr << get_fileline() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }

      return fun;
}

NetExpr* PECallFunction::elaborate_access_func_(Design*des, NetScope*scope,
						ivl_nature_t nature) const
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

      NetEAccess*tmp = new NetEAccess(branch, nature);
      tmp->set_line(*this);

      return tmp;
}

NetExpr* PECallFunction::elaborate_expr(Design*des, NetScope*scope,
					int expr_wid, bool) const
{
      if (peek_tail_name(path_)[0] == '$')
	    return elaborate_sfunc_(des, scope, expr_wid);

      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	      // Not a user defined function. Maybe it is an access
	      // function for a nature? If so then elaborate it that
	      // way.
	    ivl_nature_t access_nature = find_access_function(path_);
	    if (access_nature)
		  return elaborate_access_func_(des, scope, access_nature);

	      // We do not currently support constant user function and
	      // this is where things fail because of that, though we
	      // don't know for sure so we need to display both messages.
	    if (need_constant_expr) {
		  cerr << get_fileline() << ": sorry: constant user "
		          "functions are not currently supported: "
		       << path_ << "()." << endl << "    or" << endl;
	    }
	    cerr << get_fileline() << ": error: No function " << path_
	         << " in this context (" << scope_path(scope) << ")." << endl;
	    des->errors += 1;
	    return 0;
      }
      ivl_assert(*this, def);

      NetScope*dscope = def->scope();
      ivl_assert(*this, dscope);

      if (! check_call_matches_definition_(des, dscope))
	    return 0;

      unsigned parms_count = parms_.size();
      if ((parms_count == 1) && (parms_[0] == 0))
	    parms_count = 0;



      svector<NetExpr*> parms (parms_count);

	/* Elaborate the input expressions for the function. This is
	   done in the scope of the function call, and not the scope
	   of the function being called. The scope of the called
	   function is elaborated when the definition is elaborated. */

      unsigned missing_parms = 0;
      for (unsigned idx = 0 ;  idx < parms.count() ;  idx += 1) {
	    PExpr*tmp = parms_[idx];
	    if (tmp) {
		  parms[idx] = elaborate_rval_expr(des, scope,
						   def->port(idx)->data_type(),
						   def->port(idx)->vector_width(),
						   tmp);
		  if (NetEEvent*evt = dynamic_cast<NetEEvent*> (parms[idx])) {
			cerr << evt->get_fileline() << ": error: An event '"
			     << evt->event()->name() << "' can not be a user "
			        "function argument." << endl;
			des->errors += 1;
		  }
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug:"
			     << " function " << path_
			     << " arg " << (idx+1)
			     << " argwid=" << parms[idx]->expr_width()
			     << ": " << *parms[idx] << endl;

	    } else {
		  missing_parms += 1;
		  parms[idx] = 0;
	    }
      }

      if (missing_parms > 0) {
	    cerr << get_fileline() << ": error: The function " << path_
		 << " has been called with empty parameters." << endl;
	    cerr << get_fileline() << ":      : Verilog doesn't allow "
		 << "passing empty parameters to functions." << endl;
	    des->errors += 1;
      }


	/* Look for the return value signal for the called
	   function. This return value is a magic signal in the scope
	   of the function, that has the name of the function. The
	   function code assigns to this signal to return a value.

	   dscope, in this case, is the scope of the function, so the
	   return value is the name within that scope. */

      if (NetNet*res = dscope->find_signal(dscope->basename())) {
	    NetESignal*eres = new NetESignal(res);
	    NetEUFunc*func = new NetEUFunc(scope, dscope, eres, parms);
	    func->set_line(*this);
	    func->cast_signed(res->get_signed());
	    return func;
      }

      cerr << get_fileline() << ": internal error: Unable to locate "
	    "function return value for " << path_
	   << " in " << dscope->basename() << "." << endl;
      des->errors += 1;
      return 0;
}

unsigned PEConcat::test_width(Design*des, NetScope*scope,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&expr_type__,
			      bool&unsized_flag)
{
      expr_type_ = IVL_VT_LOGIC;

      unsigned count_width = 0;
      for (unsigned idx = 0 ; idx < parms_.count() ; idx += 1) {
	    tested_widths_[idx] = parms_[idx]->test_width(des, scope, 0, 0, expr_type__, unsized_flag);
	    count_width += tested_widths_[idx];
      }

      if (repeat_) {
	    unsigned repeat_count = 1;

	      // The repeat expression is self-determined and
	      // its own type.
	    ivl_variable_type_t tmp_type = IVL_VT_NO_TYPE;
	    bool tmp_flag = false;
	    repeat_->test_width(des, scope, 0, 0, tmp_type, tmp_flag);

	      // Try to evaluate the repeat expression now, so
	      // that we can give the caller an accurate
	      // expression width.
	    NetExpr*tmp = elab_and_eval(des, scope, repeat_, -1);
	    if (NetEConst*tmp_c = dynamic_cast<NetEConst*> (tmp)) {
		  repeat_count = tmp_c->value().as_ulong();

	    } else {
		    // Gack! Can't evaluate expression yet!
		    // Unfortunately, it is possible that this
		    // expression may turn out to be constant later in
		    // elaboration, so we can't really get away with
		    // reporting an error.
		  repeat_count = 1;
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "CONCAT MISSING TEST_WIDTH WHEN REPEAT IS PRESENT!"
			     << endl;
	    }
	    count_width *= repeat_count;
      }

      expr_type__ = expr_type_;
      unsized_flag = false;
      return count_width;
}

// Keep track of the concatenation/repeat depth.
static int concat_depth = 0;

NetExpr* PEConcat::elaborate_expr(Design*des, NetScope*scope,
				  int expr_wid, bool) const
{
      concat_depth += 1;
      NetExpr* repeat = 0;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate expr=" << *this
		 << ", expr_wid=" << expr_wid << endl;
      }

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count. */
      if (repeat_) {
	    need_constant_expr = true;
	    NetExpr*tmp = elab_and_eval(des, scope, repeat_, -1);
	    need_constant_expr = false;
	    assert(tmp);

	    if (tmp->expr_type() == IVL_VT_REAL) {
		  cerr << tmp->get_fileline() << ": error: Concatenation "
		       << "repeat expression can not be REAL." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEConst*rep = dynamic_cast<NetEConst*>(tmp);

	    if (rep == 0) {
		  cerr << get_fileline() << ": error: "
			"Concatenation repeat expression cannot be evaluated."
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
		  concat_depth -= 1;
		  return 0;
	    }

	    if (rep->value().is_negative()) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be negative (" << rep->value().as_long()
		       << ")." << endl;
		  des->errors += 1;
		  concat_depth -= 1;
		  return 0;
	    }

	    if (rep->value().is_zero() && concat_depth < 2) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be zero in this context." << endl;
		  des->errors += 1;
		  concat_depth -= 1;
		  return 0;
	    }

	    repeat = rep;
      }

      unsigned wid_sum = 0;
      unsigned parm_cnt = 0;
      svector<NetExpr*> parms(parms_.count());

	/* Elaborate all the parameters and attach them to the concat node. */
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Missing expression "
		       << (idx+1) << " of concatenation list." << endl;
		  des->errors += 1;
		  continue;
	    }

	    assert(parms_[idx]);
	    NetExpr*ex = elab_and_eval(des, scope, parms_[idx],
				       tested_widths_[idx], 0);
	    if (ex == 0) continue;

	    ex->set_line(*parms_[idx]);

	    if (ex->expr_type() == IVL_VT_REAL) {
		  cerr << ex->get_fileline() << ": error: "
		       << "Concatenation operand can not be real: "
		       << *parms_[idx] << endl;
		  des->errors += 1;
		  continue;
	    }

	    if (! ex->has_width()) {
		  cerr << ex->get_fileline() << ": error: "
		       << "Concatenation operand \"" << *parms_[idx]
		       << "\" has indefinite width." << endl;
		  des->errors += 1;
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

	/* Make the empty concat expression. */
      NetEConcat*tmp = new NetEConcat(parm_cnt, repeat);
      tmp->set_line(*this);

	/* Remove any zero width constants. */
      unsigned off = 0;
      for (unsigned idx = 0 ;  idx < parm_cnt ;  idx += 1) {
	    while (parms[off+idx] == 0) off += 1;
	    tmp->set(idx, parms[off+idx]);
      }

      tmp->set_width(wid_sum * tmp->repeat());

      if (wid_sum == 0 && concat_depth < 2) {
	    cerr << get_fileline() << ": error: Concatenation may not "
	         << "have zero width in this context." << endl;
	    des->errors += 1;
	    concat_depth -= 1;
	    delete tmp;
	    return 0;
      }

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
unsigned PEFNumber::test_width(Design*des, NetScope*scope,
			       unsigned min, unsigned lval,
			       ivl_variable_type_t&expr_type__,
			       bool&unsized_flag)
{
      expr_type_  = IVL_VT_REAL;
      expr_width_ = 1;
      unsized_flag = false;

      expr_type__ = expr_type_;
      return 1;
}

NetExpr* PEFNumber::elaborate_expr(Design*des, NetScope*scope, int, bool) const
{
      NetECReal*tmp = new NetECReal(*value_);
      tmp->set_line(*this);
      tmp->set_width(1U, false);
      return tmp;
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

      ivl_variable_type_t tmp_type = IVL_VT_NO_TYPE;
      bool tmp_flag = false;
      int msb_wid = index_tail.msb->test_width(des, scope, 0, 0, tmp_type, tmp_flag);

      tmp_type = IVL_VT_NO_TYPE;
      tmp_flag = false;
      int lsb_wid = index_tail.lsb->test_width(des, scope, 0, 0, tmp_type, tmp_flag);

	/* This handles part selects. In this case, there are
	   two bit select expressions, and both must be
	   constant. Evaluate them and pass the results back to
	   the caller. */
      need_constant_expr = true;
      NetExpr*lsb_ex = elab_and_eval(des, scope, index_tail.lsb, lsb_wid);
      need_constant_expr = false;
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

      need_constant_expr = true;
      NetExpr*msb_ex = elab_and_eval(des, scope, index_tail.msb, msb_wid);
      need_constant_expr = false;
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
      need_constant_expr = true;
      NetExpr*wid_ex = elab_and_eval(des, scope, index_tail.lsb, -1);
      need_constant_expr = false;
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
NetExpr* PEIdent::calculate_up_do_base_(Design*des, NetScope*scope) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.lsb != 0);
      ivl_assert(*this, index_tail.msb != 0);

      probe_expr_width(des, scope, index_tail.msb);
      NetExpr*tmp = elab_and_eval(des, scope, index_tail.msb, -1);
      return tmp;
}

bool PEIdent::calculate_param_range_(Design*des, NetScope*scope,
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

static void probe_index_expr_width(Design*des, NetScope*scope,
				   const name_component_t&name)
{
      for (list<index_component_t>::const_iterator cur = name.index.begin()
		 ; cur != name.index.end() ; cur ++) {

	    if (cur->msb)
		  probe_expr_width(des, scope, cur->msb);
	    if (cur->lsb)
		  probe_expr_width(des, scope, cur->lsb);
      }
}

unsigned PEIdent::test_width(Design*des, NetScope*scope,
			     unsigned min, unsigned lval,
			     ivl_variable_type_t&expr_type__,
			     bool&unsized_flag)
{
      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*ex1, *ex2;

      symbol_search(0, des, scope, path_, net, par, eve, ex1, ex2);

	// If there is a part/bit select expression, then process it
	// here. This constrains the results no matter what kind the
	// name is.

      const name_component_t&name_tail = path_.back();
      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty()) {
	    probe_index_expr_width(des, scope, name_tail);
	    const index_component_t&index_tail = name_tail.index.back();
	      // Skip full array word net selects.
	    if (!net || (name_tail.index.size() > net->array_dimensions())) {
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
	      use_width = 1;
	      break;
	  default:
	    ivl_assert(*this, 0);
      }

      if (use_width != UINT_MAX) {
	    expr_type_ = IVL_VT_LOGIC; // Assume bit/parts selects are logic
	    expr_width_ = max(use_width, min);
	    expr_type__ = expr_type_;
	    return expr_width_;
      }

	// The width of a signal expression is the width of the signal.
      if (net != 0) {
	    expr_type_ = net->data_type();
	    expr_width_= max(net->vector_width(), (unsigned long)min);
	    expr_type__ = expr_type_;
	    return expr_width_;
      }

	// The width of a parameter name is the width of the range for
	// the parameter name, if a range is declared. Otherwise, the
	// width is undefined.
      if (par != 0) {
	    expr_type_ = par->expr_type();
	    expr_type__ = expr_type_;
	    if (ex1) {
		  ivl_assert(*this, ex2);
		  const NetEConst*ex1_const = dynamic_cast<const NetEConst*> (ex1);
		  const NetEConst*ex2_const = dynamic_cast<const NetEConst*> (ex2);
		  ivl_assert(*this, ex1_const && ex2_const);

		  long msb = ex1_const->value().as_long();
		  long lsb = ex2_const->value().as_long();
		  if (msb >= lsb)
			expr_width_ = msb - lsb + 1;
		  else
			expr_width_ = lsb - msb + 1;
		  return expr_width_;
	    }

	      // This is a parameter. If it is sized (meaning it was
	      // declared with range expressions) then the range
	      // expressions would have been caught above. So if we
	      // got there there we know this is an unsized constant.
	    expr_width_ = par->expr_width();
	    unsized_flag = true;
	    return expr_width_;
      }

	// Not a net, and not a parameter? Give up on the type, but
	// set the width that we collected.
      expr_type_ = IVL_VT_NO_TYPE;
      expr_width_ = min;

      expr_type__ = expr_type_;
      return min;
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
				 int expr_wid, bool sys_task_arg) const
{
      assert(scope);

      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*ex1, *ex2;

      NetScope*found_in = symbol_search(this, des, scope, path_,
					net, par, eve,
					ex1, ex2);

	// If the identifier name is a parameter name, then return
	// a reference to the parameter expression.
      if (par != 0)
	    return elaborate_expr_param_(des, scope, par, found_in, ex1, ex2, expr_wid);


	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (net != 0)
	    return elaborate_expr_net(des, scope, net, found_in, sys_task_arg);

	// If the identifier is a named event.
	// is a variable reference.
      if (eve != 0) {
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
	    verinum val (scope->genvar_tmp_val);
	    NetEConst*tmp = new NetEConst(val);
	    tmp->set_line(*this);
	    return tmp;
      }

	// A specparam? Look up the name to see if it is a
	// specparam. If we find it, then turn it into a NetEConst
	// value and return that.
      map<perm_string,NetScope::spec_val_t>::const_iterator specp;
      perm_string key = peek_tail_name(path_);
      if (path_.size() == 1 &&
          ((specp = scope->specparams.find(key)) != scope->specparams.end())) {
	    NetScope::spec_val_t value = (*specp).second;
	    NetExpr*tmp = 0;
	    switch (value.type) {
	      case IVL_VT_BOOL:
		  tmp = new NetEConst(verinum(value.integer));
		  break;
	      case IVL_VT_REAL:
		  tmp = new NetECReal(verireal(value.real_val));
		  break;
	      default:
		  break;
	    }
	    assert(tmp);
	    tmp->set_line(*this);

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: " << path_
		       << " is a specparam" << endl;
	    return tmp;
      }

      if (error_implicit==false
	  && sys_task_arg==false
	  && path_.size()==1
	  && scope->default_nettype() != NetNet::NONE) {
	    NetNet::Type nettype = scope->default_nettype();
	    net = new NetNet(scope, peek_tail_name(path_), nettype, 1);
	    net->data_type(IVL_VT_LOGIC);
	    net->set_line(*this);
	    if (warn_implicit) {
		  cerr << get_fileline() << ": warning: implicit "
			"definition of wire " << scope_path(scope)
		       << "." << peek_tail_name(path_) << "." << endl;
	    }
	    return elaborate_expr_net(des, scope, net, scope, sys_task_arg);
      }

	// At this point we've exhausted all the possibilities that
	// are not scopes. If this is not a system task argument, then
	// it cannot be a scope name, so give up.

      if (! sys_task_arg) {
	      // I cannot interpret this identifier. Error message.
	    cerr << get_fileline() << ": error: Unable to bind wire/reg/memory "
		  "`" << path_ << "' in `" << scope_path(scope) << "'" << endl;
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

	    if (! sys_task_arg) {
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

NetExpr* PEIdent::elaborate_expr_param_part_(Design*des, NetScope*scope,
					     const NetExpr*par,
					     NetScope*found_in,
					     const NetExpr*par_msb,
					     const NetExpr*par_lsb) const
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

	    NetEConst*tmp = new NetEConst(verinum(verinum::Vx, 1, false));
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
					       const NetExpr*par_lsb) const
{
      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);

      long par_msv, par_lsv;
      if(! calculate_param_range_(des, scope, par_msb, par_msv,
                                  par_lsb, par_lsv,
                                  par_ex->value().len())) return 0;

      NetExpr*base = calculate_up_do_base_(des, scope);
      if (base == 0) return 0;

      unsigned long wid = 0;
      calculate_up_do_width_(des, scope, wid);

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Calculate part select "
		 << "[" << *base << "+:" << wid << "] from range "
		 << "[" << par_msv << ":" << par_lsv << "]." << endl;

	// Handle the special case that the base is constant. In this
	// case, just precalculate the entire constant result.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    if (! base_c->value().is_defined()) {
		  NetEConst *ex;
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  if (warn_ob_select) {
			perm_string name = peek_tail_name(path_);
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
                  perm_string name = peek_tail_name(path_);
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

      NetExpr*tmp = par->dup_expr();
      tmp = new NetESelect(tmp, base, wid);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEIdent::elaborate_expr_param_idx_do_(Design*des, NetScope*scope,
					       const NetExpr*par,
					       NetScope*found_in,
					       const NetExpr*par_msb,
					       const NetExpr*par_lsb) const
{
      const NetEConst*par_ex = dynamic_cast<const NetEConst*> (par);
      ivl_assert(*this, par_ex);

      long par_msv, par_lsv;
      if(! calculate_param_range_(des, scope, par_msb, par_msv,
                                  par_lsb, par_lsv,
                                  par_ex->value().len())) return 0;

      NetExpr*base = calculate_up_do_base_(des, scope);
      if (base == 0) return 0;

      unsigned long wid = 0;
      calculate_up_do_width_(des, scope, wid);

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: Calculate part select "
		 << "[" << *base << "-:" << wid << "] from range "
		 << "[" << par_msv << ":" << par_lsv << "]." << endl;

	// Handle the special case that the base is constant. In this
	// case, just precalculate the entire constant result.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    if (! base_c->value().is_defined()) {
		  NetEConst *ex;
		  ex = new NetEConst(verinum(verinum::Vx, wid, true));
		  ex->set_line(*this);
		  if (warn_ob_select) {
			perm_string name = peek_tail_name(path_);
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
                  perm_string name = peek_tail_name(path_);
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

      NetExpr*tmp = par->dup_expr();
      tmp = new NetESelect(tmp, base, wid);
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
					int expr_wid) const
{
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

	// NOTE TO SELF: This is the way I want to see this code
	// structured. This closely follows the structure of the
	// elaborate_expr_net_ code, which splits all the various
	// selects to different methods.
      if (use_sel == index_component_t::SEL_PART)
	    return elaborate_expr_param_part_(des, scope, par, found_in,
					      par_msb, par_lsb);

      if (use_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_param_idx_up_(des, scope, par, found_in,
						par_msb, par_lsb);

      if (use_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_param_idx_do_(des, scope, par, found_in,
						par_msb, par_lsb);

	// NOTE TO SELF (continued): The code below should be
	// rewritten in the above format, as I get to it.

      NetExpr*tmp = par->dup_expr();

      if (use_sel == index_component_t::SEL_BIT) {
	    ivl_assert(*this, !name_tail.index.empty());
	    const index_component_t&index_tail = name_tail.index.back();
	    ivl_assert(*this, index_tail.msb);
	    ivl_assert(*this, !index_tail.lsb);

	    const NetEConst*par_me =dynamic_cast<const NetEConst*>(par_msb);
	    const NetEConst*par_le =dynamic_cast<const NetEConst*>(par_lsb);

	    ivl_assert(*this, par_me || !par_msb);
	    ivl_assert(*this, par_le || !par_lsb);
	    ivl_assert(*this, par_me || !par_le);

	      /* Handle the case where a parameter has a bit
		 select attached to it. Generate a NetESelect
		 object to select the bit as desired. */
	    NetExpr*mtmp = index_tail.msb->elaborate_expr(des, scope, -1,false);
	    eval_expr(mtmp);

	      /* Let's first try to get constant values for both
		 the parameter and the bit select. If they are
		 both constant, then evaluate the bit select and
		 return instead a single-bit constant. */

	    NetEConst*le = dynamic_cast<NetEConst*>(tmp);
	    NetEConst*re = dynamic_cast<NetEConst*>(mtmp);
	    if (le && re) {

		    // Special case: If the bit select is constant and
		    // not fully defined, then we know that the result
		    // must be 1'bx.
		  if (! re->value().is_defined()) {
			if (warn_ob_select) {
			      perm_string name = peek_tail_name(path_);
			      cerr << get_fileline() << ": warning: "
				      "Constant undefined bit select ["
				   << re->value() << "] for parameter '"
				   << name << "'." << endl;
			      cerr << get_fileline() << ":        : "
				      "Replacing select with a constant "
				      "1'bx." << endl;
			}
			NetEConst*res = make_const_x(1);
			res->set_line(*this);
			return res;
		  }

		    /* Argument and bit select are constant. Calculate
		       the final result. */
		  verinum lv = le->value();
		  verinum rv = re->value();
		  verinum::V rb = verinum::Vx;

		  long par_mv = lv.len()-1;
		  long par_lv = 0;
		  if (par_me) {
			par_mv = par_me->value().as_long();
			par_lv = par_le->value().as_long();
		  }
		    /* Convert the index to canonical bit address. */
		  long ridx = rv.as_long();
		  if (par_mv >= par_lv) {
			ridx -= par_lv;
		  } else {
			ridx = par_lv - ridx;
		  }

		  if ((ridx >= 0) && ((unsigned long) ridx < lv.len())) {
			rb = lv[ridx];

		  } else if ((ridx >= 0) && (!lv.has_len())) {
			if (lv.has_sign())
			      rb = lv[lv.len()-1];
			else
			      rb = verinum::V0;
		  } else {
			if (warn_ob_select) {
			      perm_string name = peek_tail_name(path_);
			      cerr << get_fileline() << ": warning: "
			              "Constant bit select [" << rv.as_long()
			           << "] is ";
			      if (ridx < 0) cerr << "before ";
			      else cerr << "after ";
			      cerr << name << "[";
			      if (lv.has_len()) cerr << par_mv;
			      else cerr << "<inf>";
			      cerr << ":" << par_lv << "]." << endl;
			      cerr << get_fileline() << ":        : "
			              "Replacing select with a constant "
			              "1'bx." << endl;
			}
		  }


		  NetEConst*re2 = new NetEConst(verinum(rb, 1));
		  delete tmp;
		  delete mtmp;
		  tmp = re2;

	    } else {

		  if (par_me) {
			mtmp = normalize_variable_base(mtmp,
			             par_me->value().as_long(),
			             par_le->value().as_long(),
			             1, true);
		  }

		    /* The value is constant, but the bit select
		       expression is not. Elaborate a NetESelect to
		       evaluate the select at run-time. */

		  NetESelect*stmp = new NetESelect(tmp, mtmp, 1);
		  stmp->set_line(*this);
		  tmp = stmp;
	    }

      } else {
	      /* No bit or part select. Make the constant into a
		 NetEConstParam if possible. */
	    NetEConst*ctmp = dynamic_cast<NetEConst*>(tmp);
	    if (ctmp != 0) {
		  perm_string name = peek_tail_name(path_);
		  NetEConstParam*ptmp
			= new NetEConstParam(found_in, name, ctmp->value());

		  if (expr_wid > 0)
			ptmp->set_width((unsigned)expr_wid);

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Elaborate parameter <" << name
			     << "> as constant " << *ptmp << endl;
		  delete tmp;
		  tmp = ptmp;
	    }

	    NetECReal*rtmp = dynamic_cast<NetECReal*>(tmp);
	    if (rtmp != 0) {
		  perm_string name = peek_tail_name(path_);
		  NetECRealParam*ptmp
			= new NetECRealParam(found_in, name, rtmp->value());

		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "Elaborate parameter <" << name
			     << "> as constant " << *ptmp << endl;
		  delete tmp;
		  tmp = ptmp;
	    }
      }

      tmp->set_line(*this);
      return tmp;
}

/*
 * Handle word selects of vector arrays.
 */
NetExpr* PEIdent::elaborate_expr_net_word_(Design*des, NetScope*scope,
					   NetNet*net, NetScope*found_in,
					   bool sys_task_arg) const
{
      const name_component_t&name_tail = path_.back();

      if (name_tail.index.empty() && !sys_task_arg) {
	    cerr << get_fileline() << ": error: Array " << path()
		 << " Needs an array index here." << endl;
	    des->errors += 1;
	    return 0;
      }

      index_component_t index_front;
      if (! name_tail.index.empty()) {
	    index_front = name_tail.index.front();
	    ivl_assert(*this, index_front.sel != index_component_t::SEL_NONE);
	    if (index_front.sel != index_component_t::SEL_BIT) {
		  cerr << get_fileline() << ": error: Array " << path_
		       << " cannot be indexed by a range." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    ivl_assert(*this, index_front.msb);
	    ivl_assert(*this, !index_front.lsb);
      }

      NetExpr*word_index = index_front.sel == index_component_t::SEL_NONE
	    ? 0
	    : elab_and_eval(des, scope, index_front.msb, -1);
      if (word_index == 0 && !sys_task_arg)
	    return 0;

      if (NetEConst*word_addr = dynamic_cast<NetEConst*>(word_index)) {
	    long addr = word_addr->value().as_long();

	      // Special case: The index is out of range, so the value
	      // of this expression is a 'bx vector the width of a word.
	    if (!net->array_index_is_valid(addr)) {
		  cerr << get_fileline() << ": warning: returning 'bx for out "
		          "of bounds array access " << net->name()
		       << "[" << addr << "]." << endl;
		  NetEConst*resx = make_const_x(net->vector_width());
		  resx->set_line(*this);
		  delete word_index;
		  return resx;
	    }

	      // Recalculate the constant address with the adjusted base.
	    unsigned use_addr = net->array_index_to_address(addr);
	    if (addr < 0 || use_addr != (unsigned long)addr) {
		  verinum val ( (uint64_t)use_addr, 8*sizeof(use_addr));
		  NetEConst*tmp = new NetEConst(val);
		  tmp->set_line(*this);
		  delete word_index;
		  word_index = tmp;
	    }

      } else if (word_index) {
              // If there is a non-zero base to the memory, then build an
              // expression to calculate the canonical address.
            if (long base = net->array_first()) {

                  word_index = normalize_variable_array_base(
		                     word_index, base, net->array_count());
                  eval_expr(word_index);
            }
      }

      NetESignal*res = new NetESignal(net, word_index);
      res->set_line(*this);

	// Detect that the word has a bit/part select as well.

      index_component_t::ctype_t word_sel = index_component_t::SEL_NONE;
      if (name_tail.index.size() > 1)
	    word_sel = name_tail.index.back().sel;

      if (net->get_scalar() &&
          word_sel != index_component_t::SEL_NONE) {
	    cerr << get_fileline() << ": error: can not select part of ";
	    if (res->expr_type() == IVL_VT_REAL) cerr << "real";
	    else cerr << "scalar";
	    cerr << " array word: " << net->name()
	         <<"[" << *word_index << "]" << endl;
	    des->errors += 1;
	    delete res;
	    return 0;
      }

      if (word_sel == index_component_t::SEL_PART)
	    return elaborate_expr_net_part_(des, scope, res, found_in);

      if (word_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_net_idx_up_(des, scope, res, found_in);

      if (word_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_net_idx_do_(des, scope, res, found_in);

      if (word_sel == index_component_t::SEL_BIT)
	    return elaborate_expr_net_bit_(des, scope, res, found_in);

      ivl_assert(*this, word_sel == index_component_t::SEL_NONE);

      return res;
}

/*
 * Handle part selects of NetNet identifiers.
 */
NetExpr* PEIdent::elaborate_expr_net_part_(Design*des, NetScope*scope,
				      NetESignal*net, NetScope*found_in) const
{
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

	    NetEConst*tmp = new NetEConst(verinum(verinum::Vx, 1, false));
	    tmp->set_line(*this);
	    return tmp;
      }

      long sb_lsb = net->sig()->sb_to_idx(lsv);
      long sb_msb = net->sig()->sb_to_idx(msv);

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
				      NetESignal*net, NetScope*found_in) const
{
      NetExpr*base = calculate_up_do_base_(des, scope);

      unsigned long wid = 0;
      calculate_up_do_width_(des, scope, wid);


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
		  if (net->sig()->sb_to_idx(lsv) == 0 &&
		      wid == net->vector_width()) {
			delete base;
			net->cast_signed(false);
			return net;
		  }

		  long offset = 0;
		  if (net->msi() < net->lsi()) {
			offset = -wid + 1;
		  }
		    // Otherwise, make a part select that covers the right
		    // range.
		  ex = new NetEConst(verinum(net->sig()->sb_to_idx(lsv) +
		                             offset));
		  if (warn_ob_select) {
			long rel_base = net->sig()->sb_to_idx(lsv) + offset;
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

      base = normalize_variable_base(base, net->msi(), net->lsi(), wid, true);

      NetESelect*ss = new NetESelect(net, base, wid);
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
					   NetESignal*net, NetScope*found_in)const
{
      NetExpr*base = calculate_up_do_base_(des, scope);

      unsigned long wid = 0;
      calculate_up_do_width_(des, scope, wid);

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
		  if (net->sig()->sb_to_idx(lsv) == (signed) (wid-1) &&
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
		  ex = new NetEConst(verinum(net->sig()->sb_to_idx(lsv) +
		                             offset));
		  if (warn_ob_select) {
			long rel_base = net->sig()->sb_to_idx(lsv) + offset;
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

      NetESelect*ss = new NetESelect(net, base, wid);
      ss->set_line(*this);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate part "
		 << "select base="<< *base << ", wid="<< wid << endl;
      }

      return ss;
}

NetExpr* PEIdent::elaborate_expr_net_bit_(Design*des, NetScope*scope,
				      NetESignal*net, NetScope*found_in) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

      NetExpr*ex = elab_and_eval(des, scope, index_tail.msb, -1);

	// If the bit select is constant, then treat it similar
	// to the part select, so that I save the effort of
	// making a mux part in the netlist.
      if (NetEConst*msc = dynamic_cast<NetEConst*> (ex)) {

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

		  NetEConst*tmp = make_const_x(1);
		  tmp->set_line(*this);
		  delete ex;
		  return tmp;
	    }

	    long msv = msc->value().as_long();
	    long idx = net->sig()->sb_to_idx(msv);

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
			cerr  << "[" << net->sig()->msb() << ":"
			      << net->sig()->lsb() << "]." << endl;
			cerr << get_fileline() << ":        : "
			     << "Replacing select with a constant 1'bx."
			     << endl;
		  }

		  NetEConst*tmp = make_const_x(1);
		  tmp->set_line(*this);

		  delete ex;
		  return tmp;
	    }

	      // If the vector is only one bit, we are done. The
	      // bit select will return the scalar itself.
	    if (net->vector_width() == 1)
		  return net;

	      // Make an expression out of the index
	    NetEConst*idx_c = new NetEConst(verinum(idx));
	    idx_c->set_line(*net);

	      // Make a bit select with the canonical index
	    NetESelect*res = new NetESelect(net, idx_c, 1);
	    res->set_line(*net);

	    return res;
      }

	// Non-constant bit select? punt and make a subsignal
	// device to mux the bit in the net. This is a fairly
	// complicated task because we need to generate
	// expressions to convert calculated bit select
	// values to canonical values that are used internally.
      ex = normalize_variable_base(ex, net->sig()->msb(), net->sig()->lsb(),
                                   1, true);

      NetESelect*ss = new NetESelect(net, ex, 1);
      ss->set_line(*this);
      return ss;
}

NetExpr* PEIdent::elaborate_expr_net(Design*des, NetScope*scope,
				     NetNet*net, NetScope*found_in,
				     bool sys_task_arg) const
{
      if (net->array_dimensions() > 0)
	    return elaborate_expr_net_word_(des, scope, net, found_in, sys_task_arg);

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

	// If this is a part select of a signal, then make a new
	// temporary signal that is connected to just the
	// selected bits. The lsb_ and msb_ expressions are from
	// the foo[msb:lsb] expression in the original.
      if (use_sel == index_component_t::SEL_PART)
	    return elaborate_expr_net_part_(des, scope, node, found_in);

      if (use_sel == index_component_t::SEL_IDX_UP)
	    return elaborate_expr_net_idx_up_(des, scope, node, found_in);

      if (use_sel == index_component_t::SEL_IDX_DO)
	    return elaborate_expr_net_idx_do_(des, scope, node, found_in);

      if (use_sel == index_component_t::SEL_BIT)
	    return elaborate_expr_net_bit_(des, scope, node, found_in);

	// It's not anything else, so this must be a simple identifier
	// expression with no part or bit select. Return the signal
	// itself as the expression.
      assert(use_sel == index_component_t::SEL_NONE);

      return node;
}

unsigned PENumber::test_width(Design*, NetScope*,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&use_expr_type,
			      bool&unsized_flag)
{
      expr_type_ = IVL_VT_LOGIC;
      unsigned use_wid = value_->len();
      if (min > use_wid)
	    use_wid = min;

      if (! value_->has_len())
	    unsized_flag = true;

      if (lval > 0 && lval < use_wid)
	    use_wid = lval;

      use_expr_type = expr_type_;
      expr_width_ = use_wid;
      return use_wid;
}

NetEConst* PENumber::elaborate_expr(Design*des, NetScope*,
				    int expr_width__, bool) const
{
      assert(value_);
      verinum tvalue = *value_;

	// If the expr_width is >0, then the context is requesting a
	// specific size (for example this is part of the r-values of
	// an assignment) so we pad to the desired width and ignore
	// the self-determined size.
      if (expr_width__ > 0) {
	    tvalue = pad_to_width(tvalue, expr_width__);
	    if (tvalue.len() > (unsigned)expr_width__) {
		  verinum tmp (tvalue, expr_width__);
		  tmp.has_sign(tvalue.has_sign());
		  tvalue = tmp;
	    }
      }

      NetEConst*tmp = new NetEConst(tvalue);
      tmp->set_line(*this);
      return tmp;
}

unsigned PEString::test_width(Design*des, NetScope*scope,
			      unsigned min, unsigned lval,
			      ivl_variable_type_t&expr_type__,
			      bool&unsized_flag)
{
      expr_type_ = IVL_VT_BOOL;
      expr_width_ = text_? 8*strlen(text_) : 0;
      if (min > expr_width_)
	    expr_width_ = min;

      expr_type__ = expr_type_;
      return expr_width_;
}

NetEConst* PEString::elaborate_expr(Design*des, NetScope*,
				    int expr_width_dummy, bool) const
{
      NetEConst*tmp = new NetEConst(value());
      tmp->set_line(*this);
      return tmp;
}

unsigned PETernary::test_width(Design*des, NetScope*scope,
			       unsigned min, unsigned lval,
			       ivl_variable_type_t&use_expr_type,
			       bool&flag)
{
	// The condition of the ternary is self-determined, but we
	// test its width to force its type to be calculated.
      ivl_variable_type_t con_type = IVL_VT_NO_TYPE;
      bool con_flag = false;
      expr_->test_width(des, scope, 0, 0, con_type, con_flag);

      ivl_variable_type_t tru_type = IVL_VT_NO_TYPE;
      unsigned tru_wid = tru_->test_width(des, scope, min, lval, tru_type,flag);

      bool initial_flag = flag;
      ivl_variable_type_t fal_type = IVL_VT_NO_TYPE;
      unsigned fal_wid = fal_->test_width(des, scope, min, lval, fal_type,flag);

	// If the false clause is unsized, then try again with the
	// true clause, because it might choose a different width if
	// it is in an unsized context.
      if (initial_flag == false && flag == true) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "False clause is unsized, so retest width of true clause."
		       << endl;
	    tru_wid = tru_->test_width(des, scope, max(min,fal_wid), lval, tru_type, flag);
      }

	// If either of the alternatives is IVL_VT_REAL, then the
	// expression as a whole is IVL_VT_REAL. Otherwise, if either
	// of the alternatives is IVL_VT_LOGIC, then the expression as
	// a whole is IVL_VT_LOGIC. The fallback assumes that the
	// types are the same and we take that.
      if (tru_type == IVL_VT_REAL || fal_type == IVL_VT_REAL) {
	    expr_type_ = IVL_VT_REAL;
	    expr_width_ = 1;
      } else if (tru_type == IVL_VT_LOGIC || fal_type == IVL_VT_LOGIC) {
	    expr_type_ = IVL_VT_LOGIC;
	    expr_width_ = max(tru_wid,fal_wid);
      } else {
	    ivl_assert(*this, tru_type == fal_type);
	    expr_type_ = tru_type;
	    expr_width_ = max(tru_wid,fal_wid);
      }

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: "
		 << "Ternary expression type=" << expr_type_
		 << ", width=" << expr_width_
		 << ", unsized_flag=" << flag
		 << " (tru_type=" << tru_type
		 << ", fal_type=" << fal_type << ")" << endl;

      use_expr_type = expr_type_;
      return expr_width_;
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
				      int expr_wid, bool) const
{
      assert(expr_);
      assert(tru_);
      assert(fal_);

      int use_wid = expr_wid >= 0? expr_wid : 0;

      if (expr_wid < 0) {
	    use_wid = expr_width();
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Self-sized ternary chooses wid="<< use_wid
		       << ", type=" << expr_type()
		       << ",  expr=" << *this
		       << endl;

	    ivl_assert(*this, use_wid > 0);
      }

	// Elaborate and evaluate the condition expression. Note that
	// it is always self-determined.
      NetExpr*con = elab_and_eval(des, scope, expr_, -1);
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
		  if (use_wid <= 0) {
			cerr << get_fileline() << ": internal error: "
			     << "Unexpected use_wid=" << use_wid
			     << " processing short-circuit TRUE clause"
			     << " of expression: " << *this << endl;
		  }
		  ivl_assert(*this, use_wid > 0);
		  NetExpr*texpr = elab_and_eval_alternative_(des, scope, tru_,
		                                             use_wid);
		  return pad_to_width(texpr, use_wid, *this);
	    }

	      // Condition is constant FALSE, so we only need the
	      // false clause.
	    if (cval.get(0) == verinum::V0) {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: Short-circuit "
			        "elaborate FALSE clause of ternary."
			<< endl;
		  if (use_wid <= 0) {
			cerr << get_fileline() << ": internal error: "
			     << "Unexpected use_wid=" << use_wid
			     << " processing short-circuit FALSE clause"
			     << " of expression: " << *this << endl;
		  }
		  ivl_assert(*this, use_wid > 0);
		  NetExpr*texpr = elab_and_eval_alternative_(des, scope, fal_,
		                                             use_wid);
		  return pad_to_width(texpr, use_wid, *this);
	    }

	      // X and Z conditions need to blend both results, so we
	      // can't short-circuit.
      }

      NetExpr*tru = elab_and_eval_alternative_(des, scope, tru_, use_wid);
      if (tru == 0) {
	    delete con;
	    return 0;
      }

      NetExpr*fal = elab_and_eval_alternative_(des, scope, fal_, use_wid);
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

      suppress_binary_operand_sign_if_needed(tru, fal);

	/* Whatever the width we choose for the ternary operator, we
	need to make sure the operands match. */
      tru = pad_to_width(tru, use_wid, *this);
      fal = pad_to_width(fal, use_wid, *this);

      NetETernary*res = new NetETernary(con, tru, fal);
      res->cast_signed(tru->has_sign() && fal->has_sign());
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
					       PExpr*expr, int use_wid) const
{
      if (type_is_vectorable(expr->expr_type()) && !type_is_vectorable(expr_type_)) {
	    return elab_and_eval(des, scope, expr, -1);
      }

      return elab_and_eval(des, scope, expr, use_wid);
}

unsigned PEUnary::test_width(Design*des, NetScope*scope,
			     unsigned min, unsigned lval,
			     ivl_variable_type_t&expr_type__,
			     bool&unsized_flag)
{
      switch (op_) {
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	      {
		    ivl_variable_type_t sub_type = IVL_VT_NO_TYPE;
		    bool flag = false;
		    expr_->test_width(des, scope, 0, 0, sub_type, flag);
		    expr_type_ = sub_type;
	      }
	      expr_width_ = 1;

	      expr_type__ = expr_type_;
	      return expr_width_;

	  case '!':
	      {
		    ivl_variable_type_t sub_type = IVL_VT_NO_TYPE;
		    bool flag = false;
		    expr_->test_width(des, scope, 0, 0, sub_type, flag);
		    expr_type_ = (sub_type==IVL_VT_BOOL)? IVL_VT_BOOL : IVL_VT_LOGIC;
	      }
		// Logical ! always returns a single-bit LOGIC or BOOL value.
	      expr_width_ = 1;
	      expr_type__ = expr_type_;
	      return expr_width_;
      }

      unsigned test_wid = expr_->test_width(des, scope, min, lval, expr_type__, unsized_flag);
      switch (op_) {
	      // For these operators, the act of padding to the
	      // minimum width can have an important impact on the
	      // calculation. So don't let the tested width be less
	      // then the minimum width.
	  case '-':
	  case '+':
	  case '~':
	    if (test_wid < min)
		  test_wid = min;
	    break;
      }

      expr_type_ = expr_type__;
      expr_width_ = test_wid;
      return test_wid;
}


NetExpr* PEUnary::elaborate_expr(Design*des, NetScope*scope,
				 int expr_wid, bool) const
{
	/* Reduction operators and ! always have a self determined width. */
      switch (op_) {
	  case '!':
	  case '&': // Reduction AND
	  case '|': // Reduction OR
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    expr_wid = -1;
	  default:
	    break;
      }

      NetExpr*ip = expr_->elaborate_expr(des, scope, expr_wid, false);
      if (ip == 0) return 0;

      ivl_assert(*expr_, expr_type_ != IVL_VT_NO_TYPE);

      NetExpr*tmp;
      switch (op_) {
	  default:
	    tmp = new NetEUnary(op_, ip);
	    tmp->set_line(*this);
	    break;

	  case '-':
	    if (NetEConst*ipc = dynamic_cast<NetEConst*>(ip)) {

		  verinum val = ipc->value();
		  if (expr_wid > 0)
			val = pad_to_width(val, expr_wid);

		    /* When taking the - of a number, extend it one
		       bit to accommodate a possible sign bit.

		       NOTE: This may not be correct! The test_width
		       is supposed to detect the special case that we
		       want to do lossless self-determined
		       expressions, and the function that calls
		       elaborate_expr should account for that in the
		       expr_wid argument. */
		  unsigned use_len = val.len();
		  if (expr_wid < 0)
			use_len += 1;

		    /* Calculate unary minus as 0-val */
		  verinum zero (verinum::V0, use_len, val.has_len());
		  zero.has_sign(val.has_sign());
		  verinum nval = zero - val;

		  if (val.has_len())
			nval = verinum(nval, val.len());
		  nval.has_sign(val.has_sign());
		  tmp = new NetEConst(nval);
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
		  if (expr_wid > 0)
			ip = pad_to_width(ip, expr_wid, *this);
		  tmp = new NetEUnary(op_, ip);
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
	    break;

	  case '~':
	    tmp = elaborate_expr_bits_(ip, expr_wid);
	    break;
      }

      return tmp;
}

NetExpr* PEUnary::elaborate_expr_bits_(NetExpr*operand, int expr_wid) const
{
	// Handle the special case that the operand is a
	// constant. Simply calculate the constant results of the
	// expression and return that.
      if (NetEConst*ctmp = dynamic_cast<NetEConst*> (operand)) {
	    verinum value = ctmp->value();
	    if (expr_wid > (int)value.len())
		  value = pad_to_width(value, expr_wid);

	      // The only operand that I know can get here is the
	      // unary not (~).
	    ivl_assert(*this, op_ == '~');
	    value = v_not(value);

	    ctmp = new NetEConst(value);
	    ctmp->set_line(*this);
	    delete operand;
	    return ctmp;
      }

      if (expr_wid > (int)operand->expr_width())
	    operand = pad_to_width(operand, expr_wid, *this);

      NetEUBits*tmp = new NetEUBits(op_, operand);
      tmp->set_line(*this);
      return tmp;
}

NetNet* Design::find_discipline_reference(ivl_discipline_t dis, NetScope*scope)
{
      NetNet*gnd = discipline_references_[dis->name()];

      if (gnd) return gnd;

      string name = string(dis->name()) + "$gnd";
      gnd = new NetNet(scope, lex_strings.make(name), NetNet::WIRE, 1);
      gnd->set_discipline(dis);
      gnd->data_type(IVL_VT_REAL);
      discipline_references_[dis->name()] = gnd;

      if (debug_elaborate)
	    cerr << gnd->get_fileline() << ": debug: "
		 << "Create an implicit reference terminal"
		 << " for discipline=" << dis->name()
		 << " in scope=" << scope_path(scope) << endl;

      return gnd;
}
