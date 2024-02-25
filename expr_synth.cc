/*
 * Copyright (c) 1999-2024 Stephen Williams (steve@icarus.com)
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
# include "compiler.h"

# include  <cstdlib>
# include  <iostream>

# include  "netlist.h"
# include  "netvector.h"
# include  "netparray.h"
# include  "netmisc.h"
# include  "ivl_assert.h"

using namespace std;

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
				NetNet*&lsig, NetNet*&rsig, bool&real_args)
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

NetNet* NetExpr::synthesize(Design*des, NetScope*, NetExpr*)
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
                              real_args)) {
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

	    ivl_assert(*this, lsig->vector_width() == rsig->vector_width());
	    width=lsig->vector_width();
      }

      perm_string path = lsig->scope()->local_symbol();
      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(lsig->scope(), path, NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      perm_string oname = osig->scope()->local_symbol();
      NetAddSub *adder = new NetAddSub(lsig->scope(), oname, width);
      adder->set_line(*this);
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

      ivl_assert(*this, lsig->vector_width() == rsig->vector_width());
      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case '&':
	    gate = new NetLogic(scope, oname, 3, NetLogic::AND, width, true);
	    break;
	  case 'A':
	    gate = new NetLogic(scope, oname, 3, NetLogic::NAND, width, true);
	    break;
	  case '|':
	    gate = new NetLogic(scope, oname, 3, NetLogic::OR, width, true);
	    break;
	  case '^':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XOR, width, true);
	    break;
	  case 'O':
	    gate = new NetLogic(scope, oname, 3, NetLogic::NOR, width, true);
	    break;
	  case 'X':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XNOR, width, true);
	    break;
	  default:
	    gate = NULL;
	    ivl_assert(*this, 0);
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
                              real_args)) {
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

      netvector_t*osig_vec = new netvector_t(IVL_VT_LOGIC);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

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
					     width, op_=='E' ? NetCaseCmp::EEQ : NetCaseCmp::NEQ);
	    gate->set_line(*this);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }

      if (op_ == 'w' || op_ == 'W') {
	    NetCaseCmp*gate = new NetCaseCmp(scope, scope->local_symbol(),
					     width, op_=='w' ? NetCaseCmp::WEQ : NetCaseCmp::WNE);
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
					 3, NetLogic::XNOR, 1, true);
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
					 3, NetLogic::XOR, 1, true);
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
	    // fallthrough
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
	    // fallthrough
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
                              real_args)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      NetPow*powr = new NetPow(scope, scope->local_symbol(), width,
			       lsig->vector_width(),
			       rsig->vector_width());
      powr->set_line(*this);
      des->add_node(powr);

        // The lpm_pwr object only cares about the signedness of the exponent.
      powr->set_signed( right_->has_sign() );

      connect(powr->pin_DataA(), lsig->pin(0));
      connect(powr->pin_DataB(), rsig->pin(0));

      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
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
                              real_args)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      NetMult*mult = new NetMult(scope, scope->local_symbol(),
				 width,
				 lsig->vector_width(),
				 rsig->vector_width());
      mult->set_line(*this);
      des->add_node(mult);

      mult->set_signed( has_sign() );

      connect(mult->pin_DataA(), lsig->pin(0));
      connect(mult->pin_DataB(), rsig->pin(0));

      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
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
                              real_args)) {
	    return 0;
      }

      if (real_args) width = 1;
      else width = expr_width();

      netvector_t*osig_vec = new netvector_t(lsig->data_type(), width-1, 0);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
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

        /* Any real value should have already been converted to a bit value. */
      if (lsig->data_type() == IVL_VT_REAL ||
          rsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": internal error: "
	         << human_readable_op(op_)
	         << " is missing real to bit conversion." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetLogic*olog;
      perm_string oname = scope->local_symbol();

	/* Create the logic OR/AND gate. This has a single bit output,
	 * with single bit inputs for the two operands. */
      switch (op()) {
	case 'a':
	    olog = new NetLogic(scope, oname, 3, NetLogic::AND, 1, true);
	    break;
	case 'o':
	    olog = new NetLogic(scope, oname, 3, NetLogic::OR, 1, true);
	    break;
	case 'q':
	    olog = new NetLogic(scope, oname, 3, NetLogic::IMPL, 1, true);
	    break;
	case 'Q':
	    olog = new NetLogic(scope, oname, 3, NetLogic::EQUIV, 1, true);
	    break;
	default:
	    cerr << get_fileline() << ": sorry: "
	         << human_readable_op(op_)
	         << " is not currently supported." << endl;
	    des->errors += 1;
	    return 0;
      }
      olog->set_line(*this);
      des->add_node(olog);

      netvector_t*osig_tmp = new netvector_t(expr_type());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_tmp);
      osig->set_line(*this);
      osig->local_flag(true);

      connect(osig->pin(0), olog->pin(0));

	/* The left and right operands have already been reduced to a
	 * single bit value, so just connect then to the logic gate. */
      ivl_assert(*this, lsig->pin_count() == 1);
      connect(lsig->pin(0), olog->pin(1));

      ivl_assert(*this, rsig->pin_count() == 1);
      connect(rsig->pin(0), olog->pin(2));

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

	    netvector_t*osig_vec = new netvector_t(expr_type(), expr_width()-1,0);
	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, osig_vec);
	    osig->set_line(*this);
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
						   NetPartSelect::VP,
	                                           signed_flag && right_flag);
	    psel->set_line(*this);
	    des->add_node(psel);

	    netvector_t*psig_vec = new netvector_t(expr_type(), part_width-1, 0);
	    NetNet*psig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, psig_vec);
	    psig->set_line(*this);
	    psig->local_flag(true);
	    connect(psig->pin(0), psel->pin(0));

	      // Handle the special case of a signed right shift. In
	      // this case, use the NetSignExtend device to pad the
	      // result to the desired width.
	    if (signed_flag && right_flag) {
		  NetSignExtend*pad = new NetSignExtend(scope, scope->local_symbol(),
							osig->vector_width());
		  pad->set_line(*this);
		  des->add_node(pad);

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

	    netvector_t*zsig_vec = new netvector_t(osig->data_type(),
						   znum.len()-1, 0);
	    NetNet*zsig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, zsig_vec);
	    zsig->set_line(*this);
	    zsig->local_flag(true);
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

      netvector_t*osig_vec = new netvector_t(expr_type(), expr_width()-1, 0);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      NetCLShift*dev = new NetCLShift(scope, scope->local_symbol(),
				      osig->vector_width(),
				      rsig->vector_width(),
				      right_flag, signed_flag);
      dev->set_line(*this);
      des->add_node(dev);

      connect(dev->pin_Result(), osig->pin(0));

      ivl_assert(*this, lsig->vector_width() == dev->width());
      connect(dev->pin_Data(), lsig->pin(0));

      connect(dev->pin_Distance(), rsig->pin(0));

      return osig;
}

NetNet* NetEConcat::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
	/* First, synthesize the operands. */
      unsigned num_parms = parms_.size();
      NetNet**tmp = new NetNet*[parms_.size()];
      bool flag = true;
      ivl_variable_type_t data_type = IVL_VT_NO_TYPE;
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    if (parms_[idx]->expr_width() == 0) {
		    /* We need to synthesize a replication of zero. */
		  tmp[idx] = parms_[idx]->synthesize(des, scope, root);
		  ivl_assert(*this, tmp[idx] == 0);
		  num_parms -= 1;
	    } else {
		  tmp[idx] = parms_[idx]->synthesize(des, scope, root);
		  if (tmp[idx] == 0) flag = false;
		    /* Set the data type to the first one found. */
		  switch (data_type) {
		      case IVL_VT_NO_TYPE:
			data_type = tmp[idx]->data_type();
			break;
		      case IVL_VT_BOOL:
			if (tmp[idx]->data_type()==IVL_VT_LOGIC)
			      data_type = IVL_VT_LOGIC;
			break;
		      default:
			break;
		  }
	    }
      }

      if (flag == false) {
	    delete[]tmp;
	    return 0;
      }

      ivl_assert(*this, data_type != IVL_VT_NO_TYPE);

	/* If this is a replication of zero just return 0. */
      if (expr_width() == 0) {
	    delete[]tmp;
	    return 0;
      }

      NetNet *osig = nullptr;

      if (num_parms != 1) {
	      /* Make a NetNet object to carry the output vector. */
	    auto osig_vec = new netvector_t(data_type, expr_width() / repeat() - 1, 0);
	    osig = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT,
			      osig_vec);
	    osig->set_line(*this);
	    osig->local_flag(true);

	    auto cncat = new NetConcat(scope, scope->local_symbol(),
				       osig->vector_width(), num_parms);
	    cncat->set_line(*this);
	    des->add_node(cncat);
	    connect(cncat->pin(0), osig->pin(0));

	    unsigned count_input_width = 0;
	    unsigned cur_pin = 1;
	    for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
		  unsigned concat_item = parms_.size()-idx-1;
		  if (tmp[concat_item] == 0) continue;
		  connect(cncat->pin(cur_pin), tmp[concat_item]->pin(0));
		  cur_pin += 1;
		  count_input_width += tmp[concat_item]->vector_width();
	    }

	    if (count_input_width != osig->vector_width()) {
		  cerr << get_fileline() << ": internal error: "
		       << "NetEConcat input width = " << count_input_width
		       << ", expecting " << osig->vector_width()
		       << endl;
		  des->errors += 1;
	    }
      } else {
	    /* There is exactly one input signal */
	    for (unsigned idx = 0; idx < parms_.size(); idx++) {
		  if (tmp[idx]) {
			osig = tmp[idx];
			break;
		  }
	    }
	    ivl_assert(*this, osig);
	    if (osig->get_signed()) {
		    // A concatenation is always unsigned, so make a new signal to
		    // reflect this.
		  NetNet*isig = osig;
		  auto osig_vec = new netvector_t(data_type, isig->vector_width() - 1, 0);
		  osig = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT,
				    osig_vec);
		  osig->set_line(*this);
		  osig->local_flag(true);
		  connect(isig->pin(0), osig->pin(0));
	    }
      }

      if (repeat() != 1) {
	    auto rep = new NetReplicate(scope, scope->local_symbol(),
					expr_width(), repeat());
	    rep->set_line(*this);
	    des->add_node(rep);
	    connect(rep->pin(1), osig->pin(0));

	    auto osig_vec = new netvector_t(data_type, expr_width() - 1, 0);
	    osig = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT,
			      osig_vec);
	    osig->set_line(*this);
	    osig->local_flag(true);
	    connect(rep->pin(0), osig->pin(0));
      }

      delete[]tmp;
      return osig;
}

NetNet *NetEArrayPattern::synthesize(Design *des, NetScope *scope, NetExpr *root)
{
      const netsarray_t *array_type = dynamic_cast<const netsarray_t *>(net_type());
      ivl_assert(*this, array_type);

      if (items_.empty())
	    return nullptr;

      bool failed = false;

      std::unique_ptr<NetNet*[]> nets(new NetNet*[items_.size()]);
      for (unsigned int idx = 0; idx < items_.size(); idx++) {
	    if (!items_[idx]) {
		  failed = true;
		  continue;
	    }
	    nets[idx] = items_[idx]->synthesize(des, scope, root);
	    if (!nets[idx])
		  failed = true;
      }

      if (failed)
	    return nullptr;

      // Infer which dimension we are in for nested assignment patterns based on
      // the dimensions of the element.
      size_t dim = nets[0]->unpacked_dims().size() + 1;
      const auto &type_dims = array_type->static_dimensions();

      if (dim > type_dims.size())
	    return nullptr;

      netranges_t dims(type_dims.end() - dim, type_dims.end());

      if (dims.front().width() != items_.size())
	    return nullptr;

      perm_string path = scope->local_symbol();
      NetNet *osig = new NetNet(scope, path, NetNet::IMPLICIT, dims,
			        array_type->element_type());
      osig->set_line(*this);
      osig->local_flag(true);

      unsigned int opin = 0;
      for (unsigned int idx = 0; idx < items_.size(); idx++) {
	    for (unsigned int net_pin = 0; net_pin < nets[idx]->pin_count(); net_pin++)
		  connect(osig->pin(opin++), nets[idx]->pin(net_pin));
      }

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

      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      NetConst*con = new NetConst(scope, scope->local_symbol(), value());
      con->set_line(*this);
      des->add_node(con);

      connect(osig->pin(0), con->pin(0));
      return osig;
}

/*
* Create a NetLiteral object to represent real valued constants.
*/
NetNet* NetECReal::synthesize(Design*des, NetScope*scope, NetExpr*)
{
      perm_string path = scope->local_symbol();

      netvector_t*osig_vec = new netvector_t(IVL_VT_REAL);
      osig_vec->set_signed(has_sign());
      NetNet*osig = new NetNet(scope, path, NetNet::WIRE, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      NetLiteral*con = new NetLiteral(scope, scope->local_symbol(), value_);
      con->set_line(*this);
      des->add_node(con);

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
      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case '~':
	    gate = new NetLogic(scope, oname, 2, NetLogic::NOT, width, true);
	    gate->set_line(*this);
	    break;
	  default:
	    gate = NULL;
	    ivl_assert(*this, 0);
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

	    netvector_t*sig_vec = new netvector_t(sub->data_type(),
						  sub->vector_width()-1, 0);
	    NetNet*sig = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, sig_vec);
	    sig->set_line(*this);
	    sig->local_flag(true);

	    NetAbs*tmp = new NetAbs(scope, scope->local_symbol(), sub->vector_width());
	    tmp->set_line(*this);
	    des->add_node(tmp);

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
      gate->set_line(*this);
      des->add_node(gate);

      netvector_t*osig_vec = new netvector_t(expr_type());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      connect(gate->pin(0), osig->pin(0));
      for (unsigned idx = 0 ;  idx < isig->pin_count() ;  idx += 1)
	    connect(gate->pin(1+idx), isig->pin(idx));

      return osig;
}

NetNet* NetECast::synthesize(Design*des, NetScope*scope, NetExpr*root)
{
      NetNet*isig = expr_->synthesize(des, scope, root);

      if (isig == 0) return 0;

      switch (op()) {
	  case 'v':
	    isig = cast_to_int4(des, scope, isig, expr_width());
	    break;
	  case '2':
	    isig = cast_to_int2(des, scope, isig, expr_width());
	    break;
	  case 'r':
	    isig = cast_to_real(des, scope, isig);
	    break;
	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unable to synthesize " << *this << "." << endl;
	    return 0;
      }

      return isig;
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
	    unsigned below_width = 0;

	      // Any below X bits?
	    NetNet*below = 0;
	    if (base_val < 0) {
		  below_width = abs(base_val);
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
		  ivl_assert(*this, expr_width() > (select_width+below_width));
		  unsigned above_width = expr_width() - select_width - below_width;

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
	    netvector_t*tmp_vec = new netvector_t(sub->data_type(),
						  select_width-1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
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

		  tmp_vec = new netvector_t(sub->data_type(), expr_width()-1, 0);
		  tmp = new NetNet(scope, scope->local_symbol(),
				   NetNet::WIRE, tmp_vec);
		  tmp->set_line(*this);
		  tmp->local_flag(true);
		  connect(cat->pin(0), tmp->pin(0));
	    }
	    return tmp;
      }

	// This handles the case that the NetESelect exists to do an
	// actual part/bit select. Generate a NetPartSelect object to
	// do the work, and replace "sub" with the selected output.
      if (base_ != 0) {
	    NetNet*off = base_->synthesize(des, scope, root);

	    NetPartSelect*sel = new NetPartSelect(sub, off, expr_width(),
	                                          base_->has_sign());
	    sel->set_line(*this);
	    des->add_node(sel);

	    netvector_t*tmp_vec = new netvector_t(sub->data_type(),
						  expr_width()-1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, tmp_vec);
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
      if (sub->vector_width() == expr_width()) {
	    if (sub->get_signed() == has_sign())
		  return sub;

	      // If the signal and expression type don't match, we
	      // need to add an intermediate signal to reflect that.

	    auto tmp_vec = new netvector_t(sub->data_type(), sub->vector_width() - 1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT,
				    tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    connect(sub->pin(0), tmp->pin(0));
	    return tmp;
      }

      netvector_t*net_vec = new netvector_t(expr_type(), expr_width()-1, 0);
      net_vec->set_signed(has_sign());
      NetNet*net = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, net_vec);
      net->set_line(*this);
      net->local_flag(true);

	// It may still happen that the expression is wider than the selection,
	// and there was no part select created earlier (size casting).
      if(expr_width() < sub->vector_width()) {
	    NetPartSelect*sel = new NetPartSelect(sub, 0, expr_width(),
                                                  NetPartSelect::VP, has_sign());
	    sel->set_line(*this);
	    des->add_node(sel);

	    connect(net->pin(0), sel->pin(0));

	// The vector_width is not exactly right, so the source is
	// probably asking for padding. Create nodes to do sign
	// extension or 0 extension, depending on the has_sign() mode
	// of the expression.

      } else if (has_sign()) {
	    NetSignExtend*pad = new NetSignExtend(scope,
						  scope->local_symbol(),
						  expr_width());
	    pad->set_line(*this);
	    des->add_node(pad);

	    connect(pad->pin(1), sub->pin(0));
	    connect(pad->pin(0), net->pin(0));

      } else {

	    NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
					  expr_width(), 2);
	    cat->set_line(*this);
	    des->add_node(cat);

	    unsigned pad_width = expr_width() - sub->vector_width();
	    verinum pad((uint64_t)0, pad_width);
	    NetConst*con = new NetConst(scope, scope->local_symbol(),
					pad);
	    con->set_line(*this);
	    des->add_node(con);

	    netvector_t*tmp_vec = new netvector_t(expr_type(), pad_width-1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
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
      netvector_t*osig_vec = new netvector_t(expr_type(), width-1, 0);
      NetNet*osig = new NetNet(csig->scope(), path, NetNet::IMPLICIT, osig_vec);
      osig->set_line(*this);
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
      mux->set_line(*this);
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
	// If this is a synthesis with a specific value for the
	// signal, then replace it (here) with a constant value.
      if (net_->scope()==scope && net_->name()==scope->genvar_tmp) {
	    netvector_t*tmp_vec = new netvector_t(net_->data_type(),
						  net_->vector_width()-1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    verinum tmp_val ((uint64_t)scope->genvar_tmp_val, net_->vector_width());
	    NetConst*tmp_const = new NetConst(scope, scope->local_symbol(), tmp_val);
	    tmp_const->set_line(*this);
	    des->add_node(tmp_const);

	    connect(tmp->pin(0), tmp_const->pin(0));
	    return tmp;
      }

      if (word_ == 0) {
	    if (net_->get_signed() == has_sign())
		  return net_;

	      // If the signal has been cast to a different type, we
	      // need to add an intermediate signal to reflect that.

	    auto tmp_vec = new netvector_t(net_->data_type(), net_->vector_width() - 1, 0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(), NetNet::IMPLICIT,
				    tmp_vec);
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    connect(net_->pin(0), tmp->pin(0));
	    return tmp;
      }

      netvector_t*tmp_vec = new netvector_t(net_->data_type(),
					    net_->vector_width()-1, 0);
      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, tmp_vec);
      tmp->set_line(*this);
      tmp->local_flag(true);

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
      if (nset && (nset->size() > 0)) {
            NetEvent*ev = new NetEvent(scope->local_symbol());
            ev->set_line(*root);
            ev->local_flag(true);

            NetEvProbe*pr = new NetEvProbe(scope, scope->local_symbol(),
                                           ev, NetEvProbe::ANYEDGE,
                                           nset->size());
            pr->set_line(*root);
            for (unsigned idx = 0 ;  idx < nset->size() ;  idx += 1)
                  connect(nset->at(idx).lnk, pr->pin(idx));

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
      if (parms_.empty()) {
            trigger = make_func_trigger(des, scope, root);
      }

      NetSysFunc*net = new NetSysFunc(scope, scope->local_symbol(),
				      def, 1+parms_.size(), trigger);
      net->set_line(*this);
      des->add_node(net);

      netvector_t*osig_vec = new netvector_t(def->type, def->wid-1, 0);
      osig_vec->set_signed(def->type==IVL_VT_REAL? true : false);
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);

      connect(net->pin(0), osig->pin(0));

      unsigned errors = 0;
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
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
      vector<NetNet*> eparms (parms_.size());

        /* Synthesize the arguments. */
      bool errors = false;
      for (unsigned idx = 0; idx < eparms.size(); idx += 1) {
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
      netvector_t*osig_vec = new netvector_t(result_sig_->expr_type(),
					     result_sig_->vector_width()-1, 0);
      NetNet*osig = new NetNet(scope_, scope_->local_symbol(), NetNet::WIRE,
                               osig_vec);
      osig->set_line(*this);
      osig->local_flag(true);
      connect(net->pin(0), osig->pin(0));

      if (debug_synth2) {
	    cerr << get_fileline() << ": NetEUFunc::synthesize: "
		 << "result_sig_->vector_width()=" << result_sig_->vector_width()
		 << ", osig->vector_width()=" << osig->vector_width() << endl;
      }

        /* Connect the pins to the arguments. */
      NetFuncDef*def = func_->func_def();
      for (unsigned idx = 0; idx < eparms.size(); idx += 1) {
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
