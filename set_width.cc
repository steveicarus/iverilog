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

# include  <iostream>

/*
 * This file contains set_width methods for the various NetExpr
 * classes. The set_width method is used by elaboration to ask the
 * expression to resize itself. If the expression can't, then the
 * set_width method will return false and the caller will arrange for
 * whatever is needed to deal with the size mismatch.
 */
# include  "netlist.h"
# include  "netmisc.h"
# include  <typeinfo>


bool NetExpr::set_width(unsigned w)
{
      cerr << get_line() << ": internal warning:  "
	   <<typeid(*this).name() << ": set_width(unsigned) "
	   << "not implemented." << endl;
      expr_width(w);
      return false;
}

bool NetEBinary::set_width(unsigned w)
{
      bool flag = true;
      switch (op_) {

	  case 'l': // left shift  (<<)
	  case 'r': // right shift (>>)
	      /* these operators are handled in the derived class. */
	    assert(0);
	    break;

	      /* The default rule is that the operands of the binary
		 operator might as well use the same width as the
		 output from the binary operation. */
	  default:
	    expr_width(left_->expr_width() > right_->expr_width()
			  ? left_->expr_width() : right_->expr_width());
	    cerr << "NetEBinary::set_width(): Using default for " <<
		  op_ << "." << endl;
	    flag = false;

	  case '%':
	  case '/':
	    flag = left_->set_width(w) && flag;
	    flag = right_->set_width(w) && flag;
	    expr_width(w);
	    break;
      }
      return flag;
}

/*
 * The bitwise logical operators have operands the same size as the
 * result. Anything else is a mess.
 */
bool NetEBAdd::set_width(unsigned w)
{

      unsigned wid = w;
      if (left_->expr_width() > wid)
	    wid = left_->expr_width();
      if (right_->expr_width() > wid)
	    wid = right_->expr_width();

      left_->set_width(wid);
      right_->set_width(wid);

      if (left_->expr_width() < wid) {
	    NetExpr*tmp = new NetESelect(left_, 0, wid);
	    tmp->cast_signed(left_->has_sign());
	    left_ = tmp;
      }

      if (right_->expr_width() < wid) {
	    NetExpr*tmp = new NetESelect(right_, 0, wid);
	    tmp->cast_signed(right_->has_sign());
	    right_ = tmp;
      }

      expr_width(wid);
      return wid == w;
}

/*
 * The bitwise logical operators have operands the same size as the
 * result. Anything else is a mess. I first try to get the operands to
 * shrink to the desired size. I then expand operands that are too small.
 */
bool NetEBBits::set_width(unsigned w)
{
	/* First, give the operands a chance to adjust themselves to
	   the requested width. */
      left_->set_width(w);
      right_->set_width(w);


	/*  */

      unsigned use_width = w;


	/* If the operands end up too small, then pad them to suit. */

      if (left_->expr_width() < use_width) {
	    NetExpr*tmp = pad_to_width(left_, use_width);
	    assert(tmp);
	    left_ = tmp;
      }

      if (right_->expr_width() < w) {
	    NetExpr*tmp = pad_to_width(right_, use_width);
	    assert(tmp);
	    right_ = tmp;
      }


	/* And here is the final width. If this is not the size the
	   caller requested, then return false. Otherwise, return
	   true. */
      expr_width(use_width);
      return w == use_width;
}

/*
 * Comparison operators allow the subexpressions to have
 * their own natural width, but the comparison operator result has a
 * fixed width of 1.
 */
bool NetEBComp::set_width(unsigned w)
{
      return (w == 1);
}

/*
 * There is nothing we can do to the operands of a division to make it
 * confirm to the requested width. Force the context to pad or truncate.
 */
bool NetEBDiv::set_width(unsigned w)
{
      return w == expr_width();
}

/*
 * The logical binary operators do not care what the widths of their
 * operands are, the output is always 1 bit. The operands are
 * self-determined, so a set_width is stopped here.
 */
bool NetEBLogic::set_width(unsigned w)
{
      return (w == 1);
}

/*
 * There is nothing we can do to the operands of a multiply to make it
 * confirm to the requested width. Force the context to pad or truncate.
 */
bool NetEBMult::set_width(unsigned w)
{
      return w == expr_width();
}

/*
 * The shift operator allows the shift amount to have its own
 * natural width. The width of the operator result is the width of the
 * left operand, the value that is to be shifted.
 */
bool NetEBShift::set_width(unsigned w)
{
      bool flag = true;

      switch (op()) {

	  case 'l':
	    left_->set_width(w);
	    if (left_->expr_width() < w)
		  left_ = pad_to_width(left_, w);
	    break;

	  case 'r':
	  case 'R':
	    if (left_->expr_width() < w)
		  left_ = pad_to_width(left_, w);
	    break;

	  default:
	    assert(0);
      }

      expr_width(left_->expr_width());
      flag = expr_width() == w;

      return flag;
}

/*
 * Add up the widths from all the expressions that are concatenated
 * together. This is the width of the expression, tough luck if you
 * want it otherwise.
 *
 * If during the course of elaboration one of the sub-expressions is
 * broken, then don't count it in the width. This doesn't really
 * matter because the null expression is indication of an error and
 * the compiler will not go beyond elaboration.
 */
bool NetEConcat::set_width(unsigned w)
{
      unsigned sum = 0;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    if (parms_[idx] != 0)
		  sum += parms_[idx]->expr_width();

      sum *= repeat();
      expr_width(sum);
      if (sum != w) return false;
      return true;
}

bool NetEConst::set_width(unsigned w)
{
      if (w == value_.len()) {
	    expr_width(w);
	    return true;
      }

      assert(w != 0);

      if (w > value_.len()) {
	    verinum::V pad = verinum::V0;
	    if (value_.has_sign()) {
		  pad = value_.get(value_.len()-1);

	    } else if (value_.len() == 0) {
		  pad = verinum::V0;

	    } else switch (value_.get(value_.len()-1)) {
		case verinum::V1:
		case verinum::V0:
		  break;
		case verinum::Vx:
		  pad = verinum::Vx;
		  break;
		case verinum::Vz:
		  pad = verinum::Vz;
		  break;
	    }

	    verinum tmp (verinum::V0, w, has_width());
	    for (unsigned idx = 0 ;  idx < value_.len() ;  idx += 1)
		  tmp.set(idx, value_[idx]);
	    for (unsigned idx = value_.len() ;  idx < w  ; idx += 1)
		  tmp.set(idx, pad);

	    tmp.has_sign(value_.has_sign());
	    value_ = tmp;

	    expr_width(w);
	    return true;

      } else {
	    unsigned use_w = w;

	    verinum::V pad_bit = value_.has_sign()
		  ? value_[value_.len() - 1]
		  : verinum::V0;

	      // Don't reduce a number too small to hold all the
	      // significant bits.
	    for (unsigned idx = w ;  idx < value_.len() ;  idx += 1)
		  if (value_[idx] != pad_bit)
			use_w = idx+1;

	      // Correct for the special case of signed value. We
	      // cannot have the result change sign on us.
	    if (value_.has_sign() && (use_w < value_.len())
		&& (value_[use_w-1] != pad_bit))
		use_w += 1;

	    verinum tmp (verinum::V0, use_w, has_width());
	    for (unsigned idx = 0 ;  idx < use_w ;  idx += 1)
		  tmp.set(idx, value_[idx]);

	    tmp.has_sign(value_.has_sign());
	    value_ = tmp;
	    expr_width(use_w);
	    return use_w == w;
      }
}

/*
 * Real constants can have whatever width the environment wants,
 * because it isn't really a vector. The environment will convert this
 * to a vector at the right time.
 */
bool NetECReal::set_width(unsigned w)
{
      expr_width(w);
      return true;
}

bool NetEMemory::set_width(unsigned w)
{
      if (w != mem_->width())
	    return false;

      expr_width(w);
      return true;
}

bool NetEParam::set_width(unsigned)
{
      return false;
}

bool NetESFunc::set_width(unsigned w)
{
      return w == expr_width();
}

/*
 * The signal should automatically pad with zeros to get to the desired
 * width. Do not allow signal bits to be truncated, however.
 */
bool NetESignal::set_width(unsigned w)
{
      if (w != bit_count())
	    return false;

      return true;
}

bool NetEBitSel::set_width(unsigned w)
{
      if (w != 1) return false;
      return true;
}

bool NetETernary::set_width(unsigned w)
{
      bool flag = true;
      flag = flag && true_val_->set_width(w);
      flag = flag && false_val_->set_width(w);
      expr_width(true_val_->expr_width());
      return flag;
}

/*
 * XXXX FIX ME: For now, just take whatever the caller says as my
 * width. What I really need to do is note the width of the output
 * parameter of the function definition and take that into account.
 */
bool NetEUFunc::set_width(unsigned wid)
{
      expr_width(wid);
      return true;
}

bool NetEUnary::set_width(unsigned w)
{
      bool flag = true;
      switch (op_) {
	  case '~':
	  case '-':
	    flag = expr_->set_width(w);
	    expr_width(w);
	    break;
	  case '!':
	    return w == 1;
	  default:
	    flag = expr_width() == w;
	    break;
      }

      return flag;
}

/*
 * Unary reduction operators allow its operand to have any width. The
 * result is defined to be 1.
 */
bool NetEUReduce::set_width(unsigned w)
{
      return w == 1;
}
