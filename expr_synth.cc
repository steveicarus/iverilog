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
# include "compiler.h"

# include  <cstdlib>
# include  <iostream>

# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_assert.h"

static NetNet* convert_to_real_const(Design*des, NetScope*scope, NetEConst*expr)
{
      verireal vrl(expr->value().as_double());
      NetECReal rlval(vrl);
      NetNet* sig = rlval.synthesize(des, scope, 0);

      return sig;
}

  /* Note that lsig, rsig and real_args are references. */
static bool process_binary_args(Design*des, NetScope*scope, NetExpr*root,
				NetExpr*left, NetExpr*right,
				NetNet*&lsig, NetNet*&rsig, bool&real_args,
				NetExpr*obj)
{
      if (left->expr_type() == IVL_VT_REAL ||
          right->expr_type() == IVL_VT_REAL) {
	    real_args = true;

	      /* Convert the arguments to real. Handle the special
	         cases of constants, which can be converted more directly. */
	    if (left->expr_type() == IVL_VT_REAL) {
		  lsig = left->synthesize(des, scope, root);
	    } else if (NetEConst*tmpc = dynamic_cast<NetEConst*> (left)) {
		  lsig = convert_to_real_const(des, scope, tmpc);
	    } else {
		  NetNet*tmp = left->synthesize(des, scope, root);
		  lsig = cast_to_real(des, scope, tmp);
	    }

	    if (right->expr_type() == IVL_VT_REAL) {
		  rsig = right->synthesize(des, scope, root);
	    } else if (NetEConst*tmpc = dynamic_cast<NetEConst*> (right)) {
		  rsig = convert_to_real_const(des, scope, tmpc);
	    } else {
		  NetNet*tmp = right->synthesize(des, scope, root);
		  rsig = cast_to_real(des, scope, tmp);
	    }

      } else {
            real_args = false;
	    lsig = left->synthesize(des, scope, root);
	    rsig = right->synthesize(des, scope, root);

      }

      if (lsig == 0 || rsig == 0) return true;
      else return false;
}

NetNet* NetExpr::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      cerr << get_fileline() << ": internal error: cannot synthesize expression: "
	   << *this << endl;
      des->errors += 1;
      return 0;
}

/*
 * Make an LPM_ADD_SUB device from addition operators.
 */
NetNet* NetEBAdd::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      ivl_assert(*this, (op()=='+') || (op()=='-'));

      NetNet *lsig=0, *rsig=0;
      bool real_args=false;
      if (process_binary_args(des, scope, root, left_, right_, lsig, rsig,
                              real_args, this)) {
	    return 0;
      }

      ivl_assert(*this, expr_width() >= lsig->vector_width());
      ivl_assert(*this, expr_width() >= rsig->vector_width());

      unsigned width;
      if (expr_type() == IVL_VT_REAL) {
	    width = 1;
	    if (lsig->data_type() != IVL_VT_REAL)
		  lsig = cast_to_real(des, scope, lsig);
	    if (rsig->data_type() != IVL_VT_REAL)
		  rsig = cast_to_real(des, scope, rsig);

      } else {
	    lsig = pad_to_width(des, lsig, expr_width(), *this);
	    rsig = pad_to_width(des, rsig, expr_width(), *this);

	    assert(lsig->vector_width() == rsig->vector_width());
	    width=lsig->vector_width();
      }

      perm_string path = lsig->scope()->local_symbol();
      NetNet*osig = new NetNet(lsig->scope(), path, NetNet::IMPLICIT, width);
      osig->local_flag(true);
      osig->data_type(expr_type());
      osig->set_signed(has_sign());

      perm_string oname = osig->scope()->local_symbol();
      NetAddSub *adder = new NetAddSub(lsig->scope(), oname, width);
      connect(lsig->pin(0), adder->pin_DataA());
      connect(rsig->pin(0), adder->pin_DataB());
      connect(osig->pin(0), adder->pin_Result());
      des->add_node(adder);

      switch (op()) {
	  case '+':
	    adder->attribute(perm_string::literal("LPM_Direction"), verinum("ADD"));
	    break;
	  case '-':
	    adder->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));
	    break;
      }

      return osig;
}

/*
 * The bitwise logic operators are turned into discrete gates pretty
 * easily. Synthesize the left and right sub-expressions to get
 * signals, then just connect a single gate to each bit of the vector
 * of the expression.
 */
NetNet* NetEBBits::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet*lsig = left_->synthesize(des, scope, root);
      NetNet*rsig = right_->synthesize(des, scope, root);

      if (lsig == 0 || rsig == 0) return 0;

        /* You cannot do bitwise operations on real values. */
      if (lsig->data_type() == IVL_VT_REAL ||
          rsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: " << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

      unsigned width = expr_width();
      if (rsig->vector_width() > width) width = rsig->vector_width();

      lsig = pad_to_width(des, lsig, width, *this);
      rsig = pad_to_width(des, rsig, width, *this);

      assert(lsig->vector_width() == rsig->vector_width());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->local_flag(true);
      osig->data_type(expr_type());

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case '&':
	    gate = new NetLogic(scope, oname, 3, NetLogic::AND, width);
	    break;
	  case 'A':
	    gate = new NetLogic(scope, oname, 3, NetLogic::NAND, width);
	    break;
	  case '|':
	    gate = new NetLogic(scope, oname, 3, NetLogic::OR, width);
	    break;
	  case '^':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XOR, width);
	    break;
	  case 'O':
	    gate = new NetLogic(scope, oname, 3, NetLogic::NOR, width);
	    break;
	  case 'X':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XNOR, width);
	    break;
	  default:
	    gate = NULL;
	    assert(0);
      }

      connect(osig->pin(0), gate->pin(0));
      connect(lsig->pin(0), gate->pin(1));
      connect(rsig->pin(0), gate->pin(2));

      gate->set_line(*this);
      des->add_node(gate);

      return osig;
}

NetNet* NetEBComp::synthesize(Design*des, NetScope*scope, NetExpr*root)
{

      NetNet *lsig=0, *rsig=0;
      unsigned width;
      bool real_args=false;
      if (process_binary_args(des, scope, root, left_, right_, lsig, rsig,
                              real_args, this)) {
	    return 0;
      }

      if (real_args) {
	    width = 1;
      } else {
	    width = lsig->vector_width();
	    if (rsig->vector_width() > width) width = rsig->vector_width();

	    if (lsig->get_signed())
		  lsig = pad_to_width_signed(des, lsig, width, *this);
	    else
		  lsig = pad_to_width(des, lsig, width, *this);
	    if (rsig->get_signed())
		  rsig = pad_to_width_signed(des, rsig, width, *this);
	    else
		  rsig = pad_to_width(des, rsig, width, *this);
      }

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->set_line(*this);
      osig->local_flag(true);
      osig->data_type(IVL_VT_LOGIC);

	// Test if the comparison is signed.
	//
	// Note 1: This is not the same as asking if the result is
	// signed. In fact, the result will typically be UNsigned. The
	// decision to make the comparison signed depends on the
	// operand expressions.
	//
	// Note 2: The operand expressions may be signed even if the
	// sig that comes out of synthesis is unsigned. The $signed()
	// function marks the expression but doesn't change the
	// underlying signals.
      bool signed_compare = left_->has_sign() && right_->has_sign();
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Comparison (" << op_ << ")"
		 << " is " << (signed_compare? "signed"  : "unsigned")
		 << endl;
	    cerr << get_fileline() << ":      : lsig is "
		 << (lsig->get_signed()? "signed" : "unsigned")
		 << " rsig is " << (rsig->get_signed()? "signed" : "unsigned")
		 << endl;
      }

      if (op_ == 'E' || op_ == 'N') {
	    NetCaseCmp*gate = new NetCaseCmp(scope, scope->local_symbol(),
					     width, op_=='E'?true:false);
	    gate->set_line(*this);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }

	/* Handle the special case of a single bit equality
	   operation. Make an XNOR gate instead of a comparator. */
      if ((width == 1) && (op_ == 'e') && !real_args) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::XNOR, 1);
	    gate->set_line(*this);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }

	/* Handle the special case of a single bit inequality
	   operation. This is similar to single bit equality, but uses
	   an XOR instead of an XNOR gate. */
      if ((width == 1) && (op_ == 'n')  && !real_args) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::XOR, 1);
	    gate->set_line(*this);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }


      NetCompare*dev = new NetCompare(scope, scope->local_symbol(), width);
      dev->set_line(*this);
      des->add_node(dev);

      connect(dev->pin_DataA(), lsig->pin(0));
      connect(dev->pin_DataB(), rsig->pin(0));


      switch (op_) {
	  case '<':
	    connect(dev->pin_ALB(), osig->pin(0));
	    dev->set_signed(signed_compare);
	    break;
	  case '>':
	    connect(dev->pin_AGB(), osig->pin(0));
	    dev->set_signed(signed_compare);
	    break;
	  case 'E': // === ?
	    if (real_args) {
		  cerr << get_fileline() << ": error: Case equality may "
		          "not have real operands." << endl;
		  des->errors += 1;
		  return 0;
	    }
	  case 'e': // ==
	    connect(dev->pin_AEB(), osig->pin(0));
	    break;
	  case 'G': // >=
	    connect(dev->pin_AGEB(), osig->pin(0));
	    dev->set_signed(signed_compare);
	    break;
	  case 'L': // <=
	    connect(dev->pin_ALEB(), osig->pin(0));
	    dev->set_signed(signed_compare);
	    break;
	  case 'N': // !==
	    if (real_args) {
		  cerr << get_fileline() << ": error: Case inequality may "
		          "not have real operands." << endl;
		  des->errors += 1;
		  return 0;
	    }
	  case 'n': // !=
	    connect(dev->pin_ANEB(), osig->pin(0));
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: cannot synthesize "
		  "comparison: " << *this << endl;
	    des->errors += 1;
	    return 0;
      }

      return osig;
}

NetNet* NetEBPow::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet *lsig=0, *rsig=0;
      unsigned width;
      bool real_args=false;
      if (process_binary_args(des, scope, root, left_, right_, lsig, rsig,
                              real_args, this)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      NetPow*powr = new NetPow(scope, scope->local_symbol(), width,
			       lsig->vector_width(),
			       rsig->vector_width());
      des->add_node(powr);

      powr->set_signed( has_sign() );
      powr->set_line(*this);

      connect(powr->pin_DataA(), lsig->pin(0));
      connect(powr->pin_DataB(), rsig->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->set_line(*this);
      osig->data_type(expr_type());
      osig->set_signed(has_sign());
      osig->local_flag(true);

      connect(powr->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* NetEBMult::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet *lsig=0, *rsig=0;
      unsigned width;
      bool real_args=false;
      if (process_binary_args(des, scope, root, left_, right_, lsig, rsig,
                              real_args, this)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      NetMult*mult = new NetMult(scope, scope->local_symbol(),
				 width,
				 lsig->vector_width(),
				 rsig->vector_width());
      des->add_node(mult);

      mult->set_signed( has_sign() );
      mult->set_line(*this);

      connect(mult->pin_DataA(), lsig->pin(0));
      connect(mult->pin_DataB(), rsig->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->set_line(*this);
      osig->data_type(expr_type());
      osig->set_signed(has_sign());
      osig->local_flag(true);

      connect(mult->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* NetEBDiv::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet *lsig=0, *rsig=0;
      unsigned width;
      bool real_args=false;
      if (process_binary_args(des, scope, root, left_, right_, lsig, rsig,
                              real_args, this)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->set_line(*this);
      osig->data_type(lsig->data_type());
      osig->set_signed(has_sign());
      osig->local_flag(true);

      switch (op()) {

	  case '/': {
		NetDivide*div = new NetDivide(scope, scope->local_symbol(),
					      width,
					      lsig->vector_width(),
					      rsig->vector_width());
		div->set_line(*this);
		div->set_signed(has_sign());
		des->add_node(div);

		connect(div->pin_DataA(), lsig->pin(0));
		connect(div->pin_DataB(), rsig->pin(0));
		connect(div->pin_Result(),osig->pin(0));
		break;
	  }

	  case '%': {
		  /* Baseline Verilog does not support the % operator with
		     real arguments, but we allow it in our extended form. */
		if (real_args && !gn_icarus_misc_flag) {
		      cerr << get_fileline() << ": error: Modulus operator "
		              "may not have REAL operands." << endl;
		      des->errors += 1;
		      return 0;
		}
		NetModulo*div = new NetModulo(scope, scope->local_symbol(),
					      width,
					      lsig->vector_width(),
					      rsig->vector_width());
		div->set_line(*this);
		div->set_signed(has_sign());
		des->add_node(div);

		connect(div->pin_DataA(), lsig->pin(0));
		connect(div->pin_DataB(), rsig->pin(0));
		connect(div->pin_Result(),osig->pin(0));
		break;
	  }

	  default: {
		cerr << get_fileline() << ": internal error: "
		     << "NetEBDiv has unexpected op() code: "
		     << op() << endl;
		des->errors += 1;

		delete osig;
		return 0;
	  }
      }

      return osig;
}

NetNet* NetEBLogic::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet*lsig = left_->synthesize(des, scope, root);
      NetNet*rsig = right_->synthesize(des, scope, root);

      if (lsig == 0 || rsig == 0) return 0;

        /* You cannot currently do logical operations on real values. */
      if (lsig->data_type() == IVL_VT_REAL ||
          rsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": sorry: " << human_readable_op(op_)
	         << " is currently unsupported for real values." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->data_type(expr_type());
      osig->local_flag(true);


      if (op() == 'o') {

	      /* Logic OR can handle the reduction *and* the logical
		 comparison with a single wide OR gate. So handle this
		 magically. */

	    perm_string oname = scope->local_symbol();

	    NetLogic*olog = new NetLogic(scope, oname,
					 lsig->pin_count()+rsig->pin_count()+1,
					 NetLogic::OR, 1);

	    connect(osig->pin(0), olog->pin(0));

	    unsigned pin = 1;
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx = 1)
		  connect(olog->pin(pin+idx), lsig->pin(idx));

	    pin += lsig->pin_count();
	    for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx = 1)
		  connect(olog->pin(pin+idx), rsig->pin(idx));

	    des->add_node(olog);

      } else {
	    assert(op() == 'a');

	      /* Create the logic AND gate. This is a single bit
		 output, with inputs for each of the operands. */
	    NetLogic*olog;
	    perm_string oname = scope->local_symbol();

	    olog = new NetLogic(scope, oname, 3, NetLogic::AND, 1);

	    connect(osig->pin(0), olog->pin(0));
	    des->add_node(olog);

	      /* XXXX Here, I need to reduce the parameters with
		 reduction or. */


	      /* By this point, the left and right parameters have been
		 reduced to single bit values. Now we just connect them to
		 the logic gate. */
	    assert(lsig->pin_count() == 1);
	    connect(lsig->pin(0), olog->pin(1));

	    assert(rsig->pin_count() == 1);
	    connect(rsig->pin(0), olog->pin(2));
      }


      return osig;
}

NetNet* NetEBShift::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      eval_expr(right_);

      NetNet*lsig = left_->synthesize(des, scope, root);

      if (lsig == 0) return 0;

        /* Cannot shift a real values. */
      if (lsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: shift operator ("
	         << human_readable_op(op_)
	         << ") cannot shift a real values." << endl;
	    des->errors += 1;
	    return 0;
      }

      const bool right_flag  = op_ == 'r' || op_ == 'R';
      const bool signed_flag = has_sign() && op_ == 'R';

	/* Detect the special case where the shift amount is
	   constant. Evaluate the shift amount, and simply reconnect
	   the left operand to the output, but shifted. */
      if (NetEConst*rcon = dynamic_cast<NetEConst*>(right_)) {
	    verinum shift_v = rcon->value();
	    long shift = shift_v.as_long();

	    if (right_flag)
		  shift = 0-shift;

	    if (shift == 0)
		  return lsig;

	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, expr_width());
	    osig->data_type(expr_type());
	    osig->local_flag(true);

	      // ushift is the amount of pad created by the shift.
	    unsigned long ushift = shift>=0? shift : -shift;
	    ivl_assert(*this, ushift < osig->vector_width());

	      // part_width is the bits of the vector that survive the shift.
	    unsigned long part_width = osig->vector_width() - ushift;

	      // Create a part select to reduce the width of the lsig
	      // to the amount left by the shift.
	    NetPartSelect*psel = new NetPartSelect(lsig, shift<0? ushift : 0,
						   part_width,
						   NetPartSelect::VP);
	    des->add_node(psel);

	    NetNet*psig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, part_width);
	    psig->data_type(expr_type());
	    psig->local_flag(true);
	    psig->set_line(*this);
	    connect(psig->pin(0), psel->pin(0));

	      // Handle the special case of a signed right shift. In
	      // this case, use the NetSignExtend device to pad the
	      // result to the desired width.
	    if (signed_flag && right_flag) {
		  NetSignExtend*pad = new NetSignExtend(scope, scope->local_symbol(),
							osig->vector_width());
		  des->add_node(pad);
		  pad->set_line(*this);

		  connect(pad->pin(1), psig->pin(0));
		  connect(pad->pin(0), osig->pin(0));
		  return osig;
	    }

	      // Other cases are handled by zero-extending on the
	      // proper end.
	    verinum znum (verinum::V0, ushift, true);
	    NetConst*zcon = new NetConst(scope, scope->local_symbol(),
					 znum);
	    des->add_node(zcon);

	    NetNet*zsig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, znum.len());
	    zsig->data_type(osig->data_type());
	    zsig->local_flag(true);
	    zsig->set_line(*this);
	    connect(zcon->pin(0), zsig->pin(0));

	    NetConcat*ccat = new NetConcat(scope, scope->local_symbol(),
					   osig->vector_width(), 2);
	    ccat->set_line(*this);
	    des->add_node(ccat);

	    connect(ccat->pin(0), osig->pin(0));
	    if (shift > 0) {
		    // Left shift.
		  connect(ccat->pin(1), zsig->pin(0));
		  connect(ccat->pin(2), psig->pin(0));
	    } else {
		    // Right shift
		  connect(ccat->pin(1), psig->pin(0));
		  connect(ccat->pin(2), zsig->pin(0));
	    }

	    return osig;
      }

      NetNet*rsig = right_->synthesize(des, scope, root);

      if (rsig == 0) return 0;

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->data_type(expr_type());
      osig->local_flag(true);

      NetCLShift*dev = new NetCLShift(scope, scope->local_symbol(),
				      osig->vector_width(),
				      rsig->vector_width(),
				      right_flag, signed_flag);
      dev->set_line(*this);
      des->add_node(dev);

      connect(dev->pin_Result(), osig->pin(0));

      assert(lsig->vector_width() == dev->width());
      connect(dev->pin_Data(), lsig->pin(0));

      connect(dev->pin_Distance(), rsig->pin(0));

      return osig;
}

NetNet* NetEConcat::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
	/* First, synthesize the operands. */
      unsigned num_parms = parms_.count();
      NetNet**tmp = new NetNet*[parms_.count()];
      bool flag = true;
      ivl_variable_type_t data_type = IVL_VT_NO_TYPE;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    if (parms_[idx]->expr_width() == 0) {
		    /* We need to synthesize a replication of zero. */
		  tmp[idx] = parms_[idx]->synthesize(des, scope, root);
		  assert(tmp[idx] == 0);
		  num_parms -= 1;
	    } else {
		  tmp[idx] = parms_[idx]->synthesize(des, scope, root);
		  if (tmp[idx] == 0) flag = false;
		    /* Set the data type to the first one found. */
		  if (data_type == IVL_VT_NO_TYPE) {
			 data_type = tmp[idx]->data_type();
		  }
	    }
      }

      if (flag == false) return 0;

      ivl_assert(*this, data_type != IVL_VT_NO_TYPE);

	/* If this is a replication of zero just return 0. */
      if (expr_width() == 0) return 0;

	/* Make a NetNet object to carry the output vector. */
      perm_string path = scope->local_symbol();
      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);
      osig->data_type(data_type);

      NetConcat*concat = new NetConcat(scope, scope->local_symbol(),
				       osig->vector_width(),
				       num_parms * repeat());
      concat->set_line(*this);
      des->add_node(concat);
      connect(concat->pin(0), osig->pin(0));

      unsigned count_input_width = 0;
      unsigned cur_pin = 1;
      for (unsigned rpt = 0; rpt < repeat(); rpt += 1) {
	    for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
		  unsigned concat_item = parms_.count()-idx-1;
		  if (tmp[concat_item] == 0) continue;
		  connect(concat->pin(cur_pin), tmp[concat_item]->pin(0));
		  cur_pin += 1;
		  count_input_width += tmp[concat_item]->vector_width();
	    }
      }

      if (count_input_width != osig->vector_width()) {
	    cerr << get_fileline() << ": internal error: "
		 << "NetEConcat input width = " << count_input_width
		 << ", expecting " << osig->vector_width()
		 << " (repeat=" << repeat() << ")" << endl;
	    des->errors += 1;
      }

      delete[]tmp;
      return osig;
}

NetNet* NetEConst::synthesize(Design*des, NetScope*scope, NetExpr*)
{
      perm_string path = scope->local_symbol();
      unsigned width=expr_width();
      if (width == 0) {
	    cerr << get_fileline() << ": internal error: "
	         << "Found a zero width constant!" << endl;
	    return 0;
      }

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, width);
      osig->local_flag(true);
      osig->data_type(expr_type());
      osig->set_signed(has_sign());
      osig->set_line(*this);

      NetConst*con = new NetConst(scope, scope->local_symbol(), value());
      des->add_node(con);
      con->set_line(*this);

      connect(osig->pin(0), con->pin(0));
      return osig;
}

/*
* Create a NetLiteral object to represent real valued constants.
*/
NetNet* NetECReal::synthesize(Design*des, NetScope*scope, NetExpr*)
{
      perm_string path = scope->local_symbol();

      NetNet*osig = new NetNet(scope, path, NetNet::WIRE, 1);
      osig->local_flag(true);
      osig->data_type(IVL_VT_REAL);
      osig->set_signed(has_sign());
      osig->set_line(*this);

      NetLiteral*con = new NetLiteral(scope, scope->local_symbol(), value_);
      des->add_node(con);
      con->set_line(*this);

      connect(osig->pin(0), con->pin(0));
      return osig;
}

/*
 * The bitwise unary logic operator (there is only one) is turned
 * into discrete gates just as easily as the binary ones above.
 */
NetNet* NetEUBits::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet*isig = expr_->synthesize(des, scope, root);

      if (isig == 0) return 0;

      if (isig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: bit-wise negation ("
	         << human_readable_op(op_)
	         << ") may not have a REAL operand." << endl;
	    des->errors += 1;
	    return 0;
      }

      unsigned width = isig->vector_width();
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->data_type(expr_type());
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case '~':
	    gate = new NetLogic(scope, oname, 2, NetLogic::NOT, width);
	    break;
	  default:
	    gate = NULL;
	    assert(0);
      }

      connect(osig->pin(0), gate->pin(0));
      connect(isig->pin(0), gate->pin(1));

      des->add_node(gate);

      return osig;
}

NetNet* NetEUnary::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      if (op_ == '+')
	    return expr_->synthesize(des, scope, root);

      if (op_ == '-') {
	    NetNet*sig = expr_->synthesize(des, scope, root);
	    sig = sub_net_from(des, scope, 0, sig);
	    return sig;
      }

      if (op_ == 'm') {
	    NetNet*sub = expr_->synthesize(des, scope, root);
	    if (expr_->has_sign() == false)
		  return sub;

	    NetNet*sig = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, sub->vector_width());
	    sig->set_line(*this);
	    sig->local_flag(true);
	    sig->data_type(sub->data_type());

	    NetAbs*tmp = new NetAbs(scope, scope->local_symbol(), sub->vector_width());
	    des->add_node(tmp);
	    tmp->set_line(*this);

	    connect(tmp->pin(1), sub->pin(0));
	    connect(tmp->pin(0), sig->pin(0));
	    return sig;
      }

      cerr << get_fileline() << ": internal error: "
	   << "NetEUnary::synthesize cannot handle op_=" << op_ << endl;
      des->errors += 1;
      return expr_->synthesize(des, scope, root);
}

NetNet* NetEUReduce::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet*isig = expr_->synthesize(des, scope, root);

      if (isig == 0) return 0;

      if (isig->data_type() == IVL_VT_REAL) {
	    if (op() == '!') {
		  cerr << get_fileline() << ": sorry: ! is currently "
		          "unsupported for real values." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    cerr << get_fileline() << ": error: reduction operator ("
	         << human_readable_op(op_)
	         << ") may not have a REAL operand." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetUReduce::TYPE rtype = NetUReduce::NONE;

      switch (op()) {
	  case 'N':
	  case '!':
	    rtype = NetUReduce::NOR;
	    break;
	  case '&':
	    rtype = NetUReduce::AND;
	    break;
	  case '|':
	    rtype = NetUReduce::OR;
	    break;
	  case '^':
	    rtype = NetUReduce::XOR;
	    break;
	  case 'A':
	    rtype = NetUReduce::NAND;
	    break;
	  case 'X':
	    rtype = NetUReduce::XNOR;
	    break;
	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unable to synthesize " << *this << "." << endl;
	    return 0;
      }

      NetUReduce*gate = new NetUReduce(scope, scope->local_symbol(),
				       rtype, isig->vector_width());
      des->add_node(gate);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->data_type(expr_type());
      osig->local_flag(true);

      connect(gate->pin(0), osig->pin(0));
      for (unsigned idx = 0 ;  idx < isig->pin_count() ;  idx += 1)
	    connect(gate->pin(1+idx), isig->pin(idx));

      return osig;
}

/*
 * Turn a part/bit select expression into gates.
 * We know some things about the expression that elaboration enforces
 * for us:
 *
 * - Expression elaboration already converted the offset expression into
 * canonical form, so we don't have to worry about that here.
 */
NetNet* NetESelect::synthesize(Design *des, NetScope*scope, NetExpr*root)
{

      NetNet*sub = expr_->synthesize(des, scope, root);

      if (sub == 0) return 0;

      NetNet*off = 0;

	// Detect the special case that there is a base expression and
	// it is constant. In this case we can generate fixed part selects.
      if (NetEConst*base_const = dynamic_cast<NetEConst*>(base_)) {
	    verinum base_tmp = base_const->value();
	    unsigned select_width = expr_width();

	      // Return 'bx for a constant undefined selections.
	    if (!base_tmp.is_defined()) {
		  NetNet*result = make_const_x(des, scope, select_width);
		  result->set_line(*this);
		  return result;
	    }

	    long base_val = base_tmp.as_long();

	      // Any below X bits?
	    NetNet*below = 0;
	    if (base_val < 0) {
		  unsigned below_width = abs(base_val);
		  base_val = 0;
		  if (below_width > select_width) {
			below_width = select_width;
			select_width = 0;
		  } else {
			select_width -= below_width;
		  }

		  below = make_const_x(des, scope, below_width);
		  below->set_line(*this);
		    // All the selected bits are below the signal.
		  if (select_width == 0) return below;
	    }

	      // Any above bits?
	    NetNet*above = 0;
	    if ((unsigned)base_val+select_width > sub->vector_width()) {
		  if (base_val > (long)sub->vector_width()) {
			select_width = 0;
		  } else {
			select_width = sub->vector_width() - base_val;
		  }
		  unsigned above_width = expr_width() - select_width;

		  above = make_const_x(des, scope, above_width);
		  above->set_line(*this);
		    // All the selected bits are above the signal.
		  if (select_width == 0) return above;
	    }

	      // Make the make part select.
	    NetPartSelect*sel = new NetPartSelect(sub, base_val, select_width,
						  NetPartSelect::VP);
	    des->add_node(sel);

	    ivl_assert(*this, select_width > 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, select_width);
	    tmp->data_type(sub->data_type());
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    connect(sel->pin(0), tmp->pin(0));

	    unsigned concat_count = 1;
	    if (above)
		  concat_count += 1;
	    if (below)
		  concat_count += 1;
	    if (concat_count > 1) {
		  NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
						expr_width(), concat_count);
		  cat->set_line(*this);
		  des->add_node(cat);
		  if (below) {
			connect(cat->pin(1), below->pin(0));
			connect(cat->pin(2), tmp->pin(0));
		  } else {
			connect(cat->pin(1), tmp->pin(0));
		  }
		  if (above) {
			connect(cat->pin(concat_count), above->pin(0));
		  }

		  tmp = new NetNet(scope, scope->local_symbol(),
				   NetNet::WIRE, expr_width());
		  tmp->data_type(sub->data_type());
		  tmp->local_flag(true);
		  tmp->set_line(*this);
		  connect(cat->pin(0), tmp->pin(0));
	    }
	    return tmp;
      }

	// This handles the case that the NetESelect exists to do an
	// actual part/bit select. Generate a NetPartSelect object to
	// do the work, and replace "sub" with the selected output.
      if (base_ != 0) {
	    off = base_->synthesize(des, scope, root);

	    NetPartSelect*sel = new NetPartSelect(sub, off, expr_width(),
	                                          base_->has_sign());
	    sel->set_line(*this);
	    des->add_node(sel);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, expr_width());
	    tmp->data_type(sub->data_type());
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    sub = tmp;
	    connect(sub->pin(0), sel->pin(0));
      }


	// Now look for the case that the NetESelect actually exists
	// to change the width of the expression. (i.e. to do
	// padding.) If this was for an actual part select that at
	// this point the output vector_width is exactly right, and we
	// are done.
      if (sub->vector_width() == expr_width())
	    return sub;

	// The vector_width is not exactly right, so the source is
	// probably asking for padding. Create nodes to do sign
	// extension or 0 extension, depending on the has_sign() mode
	// of the expression.

      NetNet*net = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, expr_width());
      net->data_type(expr_type());
      net->local_flag(true);
      net->set_line(*this);
      if (has_sign()) {
	    NetSignExtend*pad = new NetSignExtend(scope,
						  scope->local_symbol(),
						  expr_width());
	    pad->set_line(*this);
	    des->add_node(pad);

	    connect(pad->pin(1), sub->pin(0));
	    connect(pad->pin(0), net->pin(0));
	    net->set_signed(true);

      } else {

	    NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
					  expr_width(), 2);
	    cat->set_line(*this);
	    des->add_node(cat);

	    assert(expr_width() > sub->vector_width());
	    unsigned pad_width = expr_width() - sub->vector_width();
	    verinum pad((uint64_t)0, pad_width);
	    NetConst*con = new NetConst(scope, scope->local_symbol(),
					pad);
	    con->set_line(*this);
	    des->add_node(con);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, pad_width);
	    tmp->data_type(expr_type());
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    connect(tmp->pin(0), con->pin(0));

	    connect(cat->pin(0), net->pin(0));
	    connect(cat->pin(1), sub->pin(0));
	    connect(cat->pin(2), con->pin(0));
      }

      return net;
}

/*
 * Synthesize a ?: operator as a NetMux device. Connect the condition
 * expression to the select input, then connect the true and false
 * expressions to the B and A inputs. This way, when the select input
 * is one, the B input, which is the true expression, is selected.
 */
NetNet* NetETernary::synthesize(Design *des, NetScope*scope, NetExpr*root)
{
      NetNet*csig = cond_->synthesize(des, scope, root),
            *tsig = true_val_->synthesize(des, scope, root),
            *fsig = false_val_->synthesize(des, scope, root);

      if (csig == 0 || tsig == 0 || fsig == 0) return 0;

      if (! NetETernary::test_operand_compat(tsig->data_type(),fsig->data_type())) {
	    cerr << get_fileline() << ": internal error: "
		 << " True and False clauses of ternary expression "
		 << " have incompatible types." << endl;
	    cerr << get_fileline() << ":      : True  clause is: "
	         << tsig->data_type()
		 << " (" << true_val_->expr_type() << "): "
		 << *true_val_ << endl;
	    cerr << get_fileline() << ":      : False clause is: "
	         << fsig->data_type()
		 << " (" << false_val_->expr_type() << "): "
		 << *false_val_ << endl;
	    des->errors += 1;
	    return 0;
      } else if (tsig->data_type() == IVL_VT_NO_TYPE) {
	    cerr << get_fileline() << ": internal error: True and False "
	            "clauses of ternary both have NO TYPE." << endl;
	    des->errors += 1;
	    return 0;
      }

      perm_string path = csig->scope()->local_symbol();

      ivl_assert(*this, csig->vector_width() == 1);

      unsigned width=expr_width();
      NetNet*osig = new NetNet(csig->scope(), path, NetNet::IMPLICIT, width);
      osig->data_type(expr_type());
      osig->local_flag(true);

	/* Make sure the types match. */
      if (expr_type() == IVL_VT_REAL) {
	    tsig = cast_to_real(des, scope, tsig);
	    fsig = cast_to_real(des, scope, fsig);

      }

	/* Make sure both value operands are the right width. */
      if (type_is_vectorable(expr_type())) {
	    tsig = crop_to_width(des, pad_to_width(des, tsig, width, *this), width);
	    fsig = crop_to_width(des, pad_to_width(des, fsig, width, *this), width);
	    ivl_assert(*this, width == tsig->vector_width());
	    ivl_assert(*this, width == fsig->vector_width());
      }


      perm_string oname = csig->scope()->local_symbol();
      NetMux *mux = new NetMux(csig->scope(), oname, width,
			       2, csig->vector_width());
      connect(tsig->pin(0), mux->pin_Data(1));
      connect(fsig->pin(0), mux->pin_Data(0));
      connect(osig->pin(0), mux->pin_Result());
      connect(csig->pin(0), mux->pin_Sel());
      des->add_node(mux);

      return osig;
}

/*
 * When synthesizing a signal expression, it is usually fine to simply
 * return the NetNet that it refers to. If this is an array word though,
 * a bit more work needs to be done. Return a temporary that represents
 * the selected word.
 */
NetNet* NetESignal::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      if (word_ == 0)
	    return net_;

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, net_->vector_width());
      tmp->set_line(*this);
      tmp->local_flag(true);
      tmp->data_type(net_->data_type());

	// For NetExpr objects, the word index is already converted to
	// a canonical (lsb==0) address. Just use the index directly.

      if (NetEConst*index_co = dynamic_cast<NetEConst*> (word_)) {

	    long index = index_co->value().as_long();
	    connect(tmp->pin(0), net_->pin(index));

      } else {
	    unsigned selwid = word_->expr_width();

	    NetArrayDq*mux = new NetArrayDq(scope, scope->local_symbol(),
					    net_, selwid);
	    mux->set_line(*this);
	    des->add_node(mux);

	    NetNet*index_net = word_->synthesize(des, scope, root);
	    connect(mux->pin_Address(), index_net->pin(0));

	    connect(tmp->pin(0), mux->pin_Result());
      }
      return tmp;
}

static NetEvWait* make_func_trigger(Design*des, NetScope*scope, NetExpr*root)
{
      NetEvWait*trigger = 0;

      NexusSet*nset = root->nex_input(false);
      if (nset && (nset->count() > 0)) {
            NetEvent*ev = new NetEvent(scope->local_symbol());
            ev->set_line(*root);

            NetEvProbe*pr = new NetEvProbe(scope, scope->local_symbol(),
                                           ev, NetEvProbe::ANYEDGE,
                                           nset->count());
            for (unsigned idx = 0 ;  idx < nset->count() ;  idx += 1)
                  connect(nset[0][idx], pr->pin(idx));

            des->add_node(pr);

            scope->add_event(ev);

            trigger = new NetEvWait(0);
            trigger->set_line(*root);
            trigger->add_event(ev);
      }
      delete nset;

      return trigger;
}

NetNet* NetESFunc::synthesize(Design*des, NetScope*scope, NetExpr*root)
{

      const struct sfunc_return_type*def = lookup_sys_func(name_);

        /* We cannot use the default value for system functions in a
         * continuous assignment since the function name is NULL. */
      if (def == 0 || def->name == 0) {
	    cerr << get_fileline() << ": error: System function "
		 << name_ << " not defined in system "
		 "table or SFT file(s)." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Net system function "
		 << name_ << " returns " << def->type << endl;
      }

      NetEvWait*trigger = 0;
      if (nparms_ == 0) {
            trigger = make_func_trigger(des, scope, root);
      }

      NetSysFunc*net = new NetSysFunc(scope, scope->local_symbol(),
				      def, 1+nparms_, trigger);
      net->set_line(*this);
      des->add_node(net);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, def->wid);
      osig->local_flag(true);
      osig->set_signed(def->type==IVL_VT_REAL? true : false);
      osig->data_type(def->type);
      osig->set_line(*this);

      connect(net->pin(0), osig->pin(0));

      unsigned errors = 0;
      for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1) {
	    NetNet*tmp = parms_[idx]->synthesize(des, scope, root);
	    if (tmp == 0) {
		  cerr << get_fileline() << ": error: Unable to elaborate "
		       << "argument " << idx << " of call to " << name_ <<
			"." << endl;
		  errors += 1;
		  des->errors += 1;
		  continue;
	    }

	    connect(net->pin(1+idx), tmp->pin(0));
      }

      if (errors > 0) return 0;

      return osig;
}

NetNet* NetEUFunc::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      svector<NetNet*> eparms (parms_.count());

        /* Synthesize the arguments. */
      bool errors = false;
      for (unsigned idx = 0; idx < eparms.count(); idx += 1) {
	    if (dynamic_cast<NetEEvent*> (parms_[idx])) {
		  errors = true;
		  continue;
	    }
	    NetNet*tmp = parms_[idx]->synthesize(des, scope, root);
	    if (tmp == 0) {
		  cerr << get_fileline() << ": error: Unable to synthesize "
		          "port " << idx << " of call to "
		       << func_->basename() << "." << endl;
		  errors = true;
		  des->errors += 1;
		  continue;
	    }
	    eparms[idx] = tmp;
      }
      if (errors) return 0;

      NetEvWait*trigger = 0;
      if (gn_strict_ca_eval_flag) {
              /* Ideally we would only do this for functions that have hidden
                 dependencies or side effects. Once constant functions are
                 implemented, we may be able to reuse some code to achieve
                 this. */
            trigger = make_func_trigger(des, scope, root);
      }

      NetUserFunc*net = new NetUserFunc(scope_, scope_->local_symbol(), func_,
                                        trigger);
      net->set_line(*this);
      des->add_node(net);

        /* Create an output signal and connect it to the function. */
      NetNet*osig = new NetNet(scope_, scope_->local_symbol(), NetNet::WIRE,
                               result_sig_->vector_width());
      osig->local_flag(true);
      osig->data_type(result_sig_->expr_type());
      connect(net->pin(0), osig->pin(0));

        /* Connect the pins to the arguments. */
      NetFuncDef*def = func_->func_def();
      for (unsigned idx = 0; idx < eparms.count(); idx += 1) {
	    unsigned width = def->port(idx)->vector_width();
	    NetNet*tmp;
	    if (eparms[idx]->get_signed()) {
		  tmp = pad_to_width_signed(des, eparms[idx], width, *this);
	    } else {
		  tmp = pad_to_width(des, eparms[idx], width, *this);
	    }
	    NetNet*tmpc = crop_to_width(des, tmp, width);
	    connect(net->pin(idx+1), tmpc->pin(0));
      }

      return osig;
}
