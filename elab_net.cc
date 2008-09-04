/*
 * Copyright (c) 1999-2008 Stephen Williams (steve@icarus.com)
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

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

# include  <cstdlib>
# include  <cstring>
# include  <iostream>
# include  "ivl_assert.h"

/*
 * This is a state flag that determines whether an elaborate_net must
 * report an error when it encounters an unsized number. Normally, it
 * is fine to make an unsized number as small as it can be, but there
 * are a few cases where the size must be fully self-determined. For
 * example, within a {...} (concatenation) operator.
 */
static bool must_be_self_determined_flag = false;

NetNet* PExpr::elaborate_net(Design*des, NetScope*scope, unsigned,
			     const NetExpr*,
			     const NetExpr*,
			     const NetExpr*,
			     Link::strength_t,
			     Link::strength_t) const
{
      cerr << get_fileline() << ": error: Unable to elaborate `"
	   << *this << "' as gates." << endl;
      return 0;
}

/*
 * Elaborating binary operations generally involves elaborating the
 * left and right expressions, then making an output wire and
 * connecting the lot together with the right kind of gate.
 */
NetNet* PEBinary::elaborate_net(Design*des, NetScope*scope,
				unsigned width,
				const NetExpr* rise,
				const NetExpr* fall,
				const NetExpr* decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{
      switch (op_) {
	  case '*':
	    return elaborate_net_mul_(des, scope, width, rise, fall, decay);
	  case '%':
	    return elaborate_net_mod_(des, scope, width, rise, fall, decay);
	  case '/':
	    return elaborate_net_div_(des, scope, width, rise, fall, decay);
	  case 'p': // **
	    return elaborate_net_pow_(des, scope, width, rise, fall, decay);
	  case '+':
	  case '-':
	    return elaborate_net_add_(des, scope, width, rise, fall, decay);
	  case '|': // Bitwise OR
	  case '&':
	  case '^':
	  case 'A': // Bitwise NAND (~&)
	  case 'O': // Bitwise NOR (~|)
	  case 'X': // Exclusive NOR
	    return elaborate_net_bit_(des, scope, width, rise, fall, decay);
	  case 'E': // === (case equals)
	  case 'e': // ==
	  case 'N': // !== (case not-equals)
	  case 'n': // !=
	  case '<':
	  case '>':
	  case 'L': // <=
	  case 'G': // >=
	    return elaborate_net_cmp_(des, scope, width, rise, fall, decay);
	  case 'a': // && (logical and)
	  case 'o': // || (logical or)
	    return elaborate_net_log_(des, scope, width, rise, fall, decay);
	  case 'l': // <<
	  case 'r': // >>
	  case 'R': // >>>
	    return elaborate_net_shift_(des, scope, width, rise, fall, decay);
      }

        /* This is an undefined operator, but we may as well check the
           arguments since we are here. */
      NetNet*lsig = left_->elaborate_net(des, scope, width, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, scope, width, 0, 0, 0);
      if (lsig == 0) {
	    cerr << get_fileline() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
      }
      if (rsig == 0) {
	    cerr << get_fileline() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
      }

        /* We can only get here with an undefined operator. */
      cerr << get_fileline() << ": internal error: unsupported"
              " combinational operator (" << op_ << ")." << endl;
      des->errors += 1;

      return 0;
}

/*
 * Elaborate the structural +/- as an AddSub object. Connect DataA and
 * DataB to the parameters, and connect the output signal to the
 * Result. In this context, the device is a combinational adder with
 * fixed direction, so leave Add_Sub unconnected and set the
 * LPM_Direction property.
 */
NetNet* PEBinary::elaborate_net_add_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

      NetNet*osig;

      unsigned width = lsig->vector_width();
      if (rsig->vector_width() > lsig->vector_width())
	    width = rsig->vector_width();


	/* The owidth is the output width of the lpm_add_sub
	   device. If the desired width is greater than the width of
	   the operands, then widen the adder and let code below pad
	   the operands. */
      unsigned owidth = width;
      switch (op_) {
	  case '+':
	    if (lwidth > owidth) {
		  owidth = lwidth;
		  width = lwidth;
	    }
	    break;
	  case '-':
	    if (lwidth > owidth) {
		  owidth = lwidth;
		  width = lwidth;
	    }
	    break;
	  default:
	    assert(0);
      }

      bool expr_signed = lsig->get_signed() && rsig->get_signed();

	// Pad out the operands, if necessary, the match the width of
	// the adder device.
      if (lsig->vector_width() < width) {
	    if (expr_signed)
		  lsig = pad_to_width_signed(des, lsig, width);
	    else
		  lsig = pad_to_width(des, lsig, width);
      }

      if (rsig->vector_width() < width) {
	    if (expr_signed)
		  rsig = pad_to_width_signed(des, rsig, width);
	    else
		  rsig = pad_to_width(des, rsig, width);
      }

	// Check that the argument types match.
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Arguments of add/sub "
		 << "have different data types." << endl;
	    cerr << get_fileline() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Make the adder as wide as the widest operand
      osig = new NetNet(scope, scope->local_symbol(),
			NetNet::WIRE, owidth);
      osig->data_type(lsig->data_type());
      osig->set_signed(expr_signed);
      osig->local_flag(true);
      osig->set_line(*this);
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate NetAddSub "
		 << "width=" << width << " lwidth=" << lwidth
		 << endl;
      }
      NetAddSub*adder = new NetAddSub(scope, scope->local_symbol(), width);

	// Connect the adder to the various parts.
      connect(lsig->pin(0), adder->pin_DataA());
      connect(rsig->pin(0), adder->pin_DataB());
      connect(osig->pin(0), adder->pin_Result());
#ifdef XXXX
      if (owidth > width)
	    connect(osig->pin(width), adder->pin_Cout());
#endif
      NetNode*gate = adder;
      gate->set_line(*this);
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      gate->attribute(perm_string::literal("LPM_Direction"),
		      verinum(op_ == '+' ? "ADD" : "SUB"));

      return osig;
}

/*
 * Elaborate various bitwise logic operators. These are all similar in
 * that they take operants of equal width, and each bit does not
 * affect any other bits. Also common about all this is how bit widths
 * of the operands are handled, when they do not match.
 */
NetNet* PEBinary::elaborate_net_bit_(Design*des, NetScope*scope,
				     unsigned width,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, width, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, scope, width, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

      if (lsig->vector_width() < rsig->vector_width())
	    lsig = pad_to_width(des, lsig, rsig->vector_width());
      if (rsig->vector_width() < lsig->vector_width())
	    rsig = pad_to_width(des, rsig, lsig->vector_width());

      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Types of "
		 << "operands of " << op_ << " do not match: "
		 << lsig->data_type() << " vs. " << rsig->data_type()
		 << endl;
	    des->errors += 1;
	    return 0;
      }

        /* The types match here and real is not supported. */
      if (lsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: " << human_readable_op(op_)
	         << " operator may not have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (lsig->vector_width() != rsig->vector_width()) {
	    cerr << get_fileline() << ": internal error: lsig width ("
		 << lsig->vector_width() << ") != rsig pin width ("
		 << rsig->vector_width() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(lsig->vector_width() == rsig->vector_width());

      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			       lsig->vector_width());
      osig->local_flag(true);
      osig->set_line(*this);
      osig->data_type( lsig->data_type() );

      NetLogic::TYPE gtype=NetLogic::AND;
      switch (op_) {
	  case '^': gtype = NetLogic::XOR;  break;  // XOR
	  case 'X': gtype = NetLogic::XNOR; break;  // XNOR
	  case '&': gtype = NetLogic::AND;  break;  // AND
	  case 'A': gtype = NetLogic::NAND; break;  // NAND (~&)
	  case '|': gtype = NetLogic::OR;   break;  // Bitwise OR
	  case 'O': gtype = NetLogic::NOR;  break;  // Bitwise NOR
	  default: assert(0);
      }

      NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
				   3, gtype, osig->vector_width());
      gate->set_line(*this);
      connect(gate->pin(0), osig->pin(0));
      connect(gate->pin(1), lsig->pin(0));
      connect(gate->pin(2), rsig->pin(0));
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      return osig;
}

/*
 * This function attempts to handle the special case of == or !=
 * compare to a constant value. The caller has determined already that
 * one of the operands is a NetEConst, and has already elaborated the
 * other.
 */
static NetNet* compare_eq_constant(Design*des, NetScope*scope,
				   NetNet*lsig, NetEConst*rexp,
				   char op_code,
				   const NetExpr* rise,
				   const NetExpr* fall,
				   const NetExpr* decay)
{
      if (op_code != 'e' && op_code != 'n')
	    return 0;

      verinum val = rexp->value();

	/* Abandon special case if there are x or z bits in the
	   constant. We can't get the right behavior out of
	   OR/NOR in this case. */
      if (! val.is_defined())
	    return 0;

      if (val.len() < lsig->vector_width())
	    val = verinum(val, lsig->vector_width());

	/* Look for the very special case that we know the compare
	   results a priori due to different high bits, that are
	   constant pad in the signal. */
      if (val.len() > lsig->vector_width()) {
	    unsigned idx = lsig->vector_width();
	    verinum::V lpad = verinum::V0;

	    while (idx < val.len()) {
		  if (val.get(idx) != lpad) {
			verinum oval (op_code == 'e'
				      ? verinum::V0
				      : verinum::V1,
				      1);
			NetEConst*ogate = new NetEConst(oval);
			NetNet*osig = ogate->synthesize(des);
			osig->data_type(lsig->data_type());
			osig->set_line(*lsig);
			osig->rise_time(rise);
			osig->fall_time(fall);
			osig->decay_time(decay);
			delete ogate;

			if (debug_elaborate)
			      cerr << lsig->get_fileline() << ": debug: "
				   << "Equality replaced with "
				   << oval << " due to high pad mismatch"
				   << endl;

			return osig;
		  }

		  idx +=1;
	    }
      }

      unsigned zeros = 0;
      unsigned ones = 0;
      for (unsigned idx = 0 ;  idx < lsig->vector_width() ;  idx += 1) {
	    if (val.get(idx) == verinum::V0)
		  zeros += 1;
	    if (val.get(idx) == verinum::V1)
		  ones += 1;
      }

	/* Handle the special case that the gate is a compare that can
	   be replaces with a reduction AND or NOR. */

      if (ones == 0 || zeros == 0) {
	    NetUReduce::TYPE type;

	    if (zeros > 0) {
		  type = op_code == 'e'? NetUReduce::NOR : NetUReduce::OR;

		  if (debug_elaborate)
			cerr << lsig->get_fileline() << ": debug: "
			     << "Replace net==" << val << " equality with "
			     << zeros << "-input reduction [N]OR gate." << endl;

	    } else {
		  type = op_code == 'e'? NetUReduce::AND : NetUReduce::NAND;

		  if (debug_elaborate)
			cerr << lsig->get_fileline() << ": debug: "
			     << "Replace net==" << val << " equality with "
			     << ones << "-input reduction AND gate." << endl;
	    }

	    NetUReduce*red = new NetUReduce(scope, scope->local_symbol(),
					    type, zeros+ones);
	    des->add_node(red);
	    red->set_line(*lsig);
	    red->rise_time(rise);
	    red->fall_time(fall);
	    red->decay_time(decay);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, 0, 0);
	    tmp->data_type(lsig->data_type());
	    tmp->local_flag(true);
	    tmp->set_line(*lsig);

	    connect(red->pin(1), lsig->pin(0));
	    connect(red->pin(0), tmp->pin(0));
	    return tmp;
      }

      if (debug_elaborate)
	    cerr << lsig->get_fileline() << ": debug: "
		 << "Give up trying to replace net==" << val
		 << " equality with "
		 << ones << "-input AND and "
		 << zeros << "-input NOR gates." << endl;

      return 0;
}

/*
 * Elaborate the various binary comparison operators. The comparison
 * operators return a single bit result, no matter what, so the left
 * and right values can have their own size. The only restriction is
 * that they have the same size.
 */
NetNet* PEBinary::elaborate_net_cmp_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
	/* Elaborate the operands of the compare first as expressions
	   (so that the eval_tree method can reduce constant
	   expressions, including parameters) then turn those results
	   into synthesized nets. */
      NetExpr*lexp = elab_and_eval(des, scope, left_, -1),
             *rexp = elab_and_eval(des, scope, right_, -1);

      if (lexp == 0 || rexp == 0) return 0;

      bool real_arg = true;
      if (lexp->expr_type() != IVL_VT_REAL &&
          rexp->expr_type() != IVL_VT_REAL) {
	/* Choose the operand width to be the width of the widest
	   self-determined operand. */
	    unsigned operand_width = lexp->expr_width();
	    if (rexp->expr_width() > operand_width)
	          operand_width = rexp->expr_width();

	    lexp->set_width(operand_width);
	    lexp = pad_to_width(lexp, operand_width);
	    rexp->set_width(operand_width);
	    rexp = pad_to_width(rexp, operand_width);

	    real_arg = false;
      }

      NetNet*lsig = 0;
      NetNet*rsig = 0;

	/* Handle the special case that the right or left
	   sub-expression is a constant value. The compare_eq_constant
	   function will return an elaborated result if it can make
	   use of the situation, or 0 if it cannot. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(rexp)) {

	    lsig = lexp->synthesize(des);
	    if (lsig == 0) return 0;
	    delete lexp;
	    lexp = 0;

	    if (real_arg) {
		  verireal vrl(tmp->value().as_double());
		  NetECReal rlval(vrl);
		  rsig = rlval.synthesize(des);
		  delete rexp;
		  rexp = 0;
	    } else {
		  NetNet*osig = compare_eq_constant(des, scope,
					            lsig, tmp, op_,
					            rise, fall, decay);
		  if (osig != 0) {
		        delete rexp;
		        return osig;
		  }
	    }
      }

      if (NetEConst*tmp = dynamic_cast<NetEConst*>(lexp)) {

	    rsig = rexp->synthesize(des);
	    if (rsig == 0) return 0;
	    delete rexp;
	    rexp = 0;

	    if (real_arg) {
		  verireal vrl(tmp->value().as_double());
		  NetECReal rlval(vrl);
		  lsig = rlval.synthesize(des);
		  delete lexp;
		  lexp = 0;
	    } else {
		  NetNet*osig = compare_eq_constant(des, scope,
					            rsig, tmp, op_,
					            rise, fall, decay);
		  if (osig != 0) {
		        delete lexp;
		        return osig;
		  }
	    }
      }

      if (lsig == 0) {
	    lsig = lexp->synthesize(des);
	    if (lsig == 0) return 0;
	    delete lexp;
      }

      if (rsig == 0) {
	    rsig = rexp->synthesize(des);
	    if (rsig == 0) return 0;
	    delete rexp;
      }

      unsigned dwidth = lsig->vector_width();
      if (rsig->vector_width() > dwidth) dwidth = rsig->vector_width();

	/* Operands of binary compare need to be padded to equal
	   size. Figure the pad bit needed to extend the narrowest
	   vector. */
      if (!real_arg && lsig->vector_width() < dwidth)
	    lsig = pad_to_width(des, lsig, dwidth);
      if (!real_arg && rsig->vector_width() < dwidth)
	    rsig = pad_to_width(des, rsig, dwidth);

        /* For now the runtime cannot convert a vec4 to a real value. */
      if (real_arg && (rsig->data_type() != IVL_VT_REAL ||
                       lsig->data_type() != IVL_VT_REAL)) {
	    cerr << get_fileline() << ": sorry: comparing bit based signals "
	            "and real values is not supported." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
      osig->data_type(IVL_VT_LOGIC);
      osig->set_line(*this);
      osig->local_flag(true);

      NetNode*gate;

      switch (op_) {
	  case '<':
	  case '>':
	  case 'L':
	  case 'G': {
		NetCompare*cmp = new
		      NetCompare(scope, scope->local_symbol(), dwidth);
		connect(cmp->pin_DataA(), lsig->pin(0));
		connect(cmp->pin_DataB(), rsig->pin(0));

		switch (op_) {
		    case '<':
		      connect(cmp->pin_ALB(), osig->pin(0));
		      break;
		    case '>':
		      connect(cmp->pin_AGB(), osig->pin(0));
		      break;
		    case 'L':
		      connect(cmp->pin_ALEB(), osig->pin(0));
		      break;
		    case 'G':
		      connect(cmp->pin_AGEB(), osig->pin(0));
		      break;
		}
		  /* If both operands are signed, then do a signed
		     compare. */
		if (lsig->get_signed() && rsig->get_signed())
		      cmp->set_signed(true);

		gate = cmp;
		break;
	  }

	  case 'E': // Case equals (===)
	    if (real_arg) {
		cerr << get_fileline() << ": error: Case equality may not "
		        "have real operands." << endl;
		des->errors += 1;
		return 0;
	    }
	    gate = new NetCaseCmp(scope, scope->local_symbol(), dwidth, true);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    break;

	  case 'N': // Case equals (!==)
	    if (real_arg) {
		cerr << get_fileline() << ": error: Case inequality may not "
		        "have real operands." << endl;
		des->errors += 1;
		return 0;
	    }
	    gate = new NetCaseCmp(scope, scope->local_symbol(), dwidth, false);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    break;

	  case 'e': // ==

	      /* Handle the special case of single bit compare with a
		 single XNOR gate. This is easy and direct. */
	    if (dwidth == 1 && !real_arg){
		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XNOR, 1);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate net == gate."
		       << endl;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_symbol(),
					      dwidth);
	      connect(cmp->pin_DataA(), lsig->pin(0));
	      connect(cmp->pin_DataB(), rsig->pin(0));
	      connect(cmp->pin_AEB(), osig->pin(0));
	      gate = cmp;
	    }
	    break;

	  case 'n': // !=

	      /* Handle the special case of single bit compare with a
		 single XOR gate. This is easy and direct. */
	    if (dwidth == 1 && lsig->data_type() != IVL_VT_REAL &&
	                       rsig->data_type() != IVL_VT_REAL) {
		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XOR, 1);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_symbol(),
					      dwidth);
	      connect(cmp->pin_DataA(), lsig->pin(0));
	      connect(cmp->pin_DataB(), rsig->pin(0));
	      connect(cmp->pin_ANEB(), osig->pin(0));
	      gate = cmp;
	    }
	    break;

	  default:
	    assert(0);
      }

      gate->set_line(*this);
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      return osig;
}

/*
 * Elaborate a divider gate. This function create a NetDivide gate
 * which has exactly the right sized DataA, DataB and Result ports. If
 * the l-value is wider then the result, then pad.
 */
NetNet* PEBinary::elaborate_net_div_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

	// Check the l-value width. If it is unspecified, then use the
	// largest operand width as the l-value width. Restrict the
	// result width to the width of the largest operand, because
	// there is no value is excess divider.

      unsigned rwidth = lwidth;

	// If either operand is IVL_VT_REAL, then cast the other to
	// IVL_VT_REAL so that the division can become IVL_VT_REAL.

      if (lsig->data_type()==IVL_VT_REAL || rsig->data_type()==IVL_VT_REAL) {
	    if (lsig->data_type() != IVL_VT_REAL)
		  lsig = cast_to_real(des, scope, lsig);
	    if (rsig->data_type() != IVL_VT_REAL)
		  rsig = cast_to_real(des, scope, rsig);
      }

      if (rwidth == 0) {
	    rwidth = lsig->vector_width();
	    if (rsig->vector_width() > rwidth)
		  rwidth = rsig->vector_width();

	    lwidth = rwidth;
      }

      if ((rwidth > lsig->vector_width()) && (rwidth > rsig->vector_width())) {
	    rwidth = lsig->vector_width();
	    if (rsig->vector_width() > rwidth)
		  rwidth = rsig->vector_width();
      }

	/* The arguments of a divide must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Arguments of divide "
		 << "have different data types." << endl;
	    cerr << get_fileline() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Create a device with the calculated dimensions.
      NetDivide*div = new NetDivide(scope, scope->local_symbol(), rwidth,
				    lsig->vector_width(),
				    rsig->vector_width());
      div->set_line(*this);
      div->rise_time(rise);
      div->fall_time(fall);
      div->decay_time(decay);
      des->add_node(div);

      div->set_signed(lsig->get_signed() && rsig->get_signed());

	// Connect the left and right inputs of the divider to the
	// nets that are the left and right expressions.

      connect(div->pin_DataA(), lsig->pin(0));
      connect(div->pin_DataB(), rsig->pin(0));


	// Make an output signal that is the width of the l-value.
	// Due to above calculation of rwidth, we know that the result
	// will be no more than the l-value, so it is safe to connect
	// all the result pins to the osig.

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);
      osig->set_line(*this);
      osig->data_type( lsig->data_type() );
      osig->set_signed(div->get_signed());

      connect(div->pin_Result(), osig->pin(0));


      return osig;
}

/*
 * Elaborate a modulo gate.
 */
NetNet* PEBinary::elaborate_net_mod_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

	/* The arguments of a modulus must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Arguments of modulus "
		 << "have different data types." << endl;
	    cerr << get_fileline() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

        /* The % operator does not support real arguments in baseline
           Verilog. But we allow it in our extended form of Verilog. */
      if (gn_icarus_misc_flag==false && lsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: Modulus operator may not "
	    "have REAL operands." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* rwidth is result width. */
      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	      /* Reals are always 1 wide and lsig/rsig types match here. */
	    if (lsig->data_type() == IVL_VT_REAL) {
		  lwidth = 1;
		  rwidth = 1;
	    } else {
		  rwidth = lsig->vector_width();
		  if (rsig->vector_width() > rwidth)
		        rwidth = rsig->vector_width();

		  lwidth = rwidth;
	    }
      }

      NetModulo*mod = new NetModulo(scope, scope->local_symbol(), rwidth,
				    lsig->vector_width(),
				    rsig->vector_width());
      mod->set_line(*this);
      mod->rise_time(rise);
      mod->fall_time(fall);
      mod->decay_time(decay);
      des->add_node(mod);

      connect(mod->pin_DataA(), lsig->pin(0));
      connect(mod->pin_DataB(), rsig->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, rwidth);
      osig->set_line(*this);
      osig->data_type( lsig->data_type() );
      osig->local_flag(true);

      connect(mod->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* PEBinary::elaborate_net_log_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     const NetExpr* rise,
				     const NetExpr* fall,
				     const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

      if (rsig->data_type() == IVL_VT_REAL ||
          lsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": sorry: " << human_readable_op(op_)
	         << " is currently unsupported for real values." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetLogic*gate;
      switch (op_) {
	  case 'a':
	    gate = new NetLogic(scope, scope->local_symbol(),
				3, NetLogic::AND, 1);
	    break;
	  case 'o':
	    gate = new NetLogic(scope, scope->local_symbol(),
				3, NetLogic::OR, 1);
	    break;
	  default:
	    assert(0);
      }

	// The first OR gate returns 1 if the left value is true...
      if (lsig->vector_width() > 1) {
	    NetUReduce*gate_tmp = new NetUReduce(scope, scope->local_symbol(),
						 NetUReduce::OR,
						 lsig->vector_width());
	    connect(gate_tmp->pin(1), lsig->pin(0));
	    connect(gate->pin(1), gate_tmp->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->data_type(IVL_VT_LOGIC);
	    tmp->local_flag(true);
	    connect(gate->pin(1), tmp->pin(0));

	    des->add_node(gate_tmp);

      } else {
	    connect(gate->pin(1), lsig->pin(0));
      }

	// The second OR gate returns 1 if the right value is true...
      if (rsig->vector_width() > 1) {
	    NetUReduce*gate_tmp = new NetUReduce(scope, scope->local_symbol(),
						 NetUReduce::OR,
						 rsig->vector_width());
	    connect(gate_tmp->pin(1), rsig->pin(0));
	    connect(gate->pin(2), gate_tmp->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->data_type(IVL_VT_LOGIC);
	    tmp->local_flag(true);
	    connect(gate->pin(2), tmp->pin(0));

	    des->add_node(gate_tmp);

      } else {
	    connect(gate->pin(2), rsig->pin(0));
      }

	// The output is the AND/OR of the two logic values.
      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
      osig->local_flag(true);
      osig->data_type(IVL_VT_LOGIC);
      connect(gate->pin(0), osig->pin(0));

      gate->set_line(*this);
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);
      return osig;
}

NetNet* PEBinary::elaborate_net_mul_(Design*des, NetScope*scope,
				       unsigned lwidth,
				       const NetExpr* rise,
				       const NetExpr* fall,
				       const NetExpr* decay) const
{
      verinum*lnum = left_->eval_const(des, scope);
      verinum*rnum = right_->eval_const(des, scope);

	/* Detect and handle the special case that both the operands
	   of the multiply are constant expressions. Evaluate the
	   value and make this a simple constant. */
      if (lnum && rnum) {
	    verinum prod = *lnum * *rnum;
	    if (lwidth == 0)
		  lwidth = prod.len();

	    verinum res (verinum::V0, lwidth);
	    for (unsigned idx = 0
		       ;  idx < prod.len() && idx < lwidth
		       ;  idx += 1) {
		  res.set(idx, prod.get(idx));
	    }

	    NetConst*odev = new NetConst(scope, scope->local_symbol(), res);
	    des->add_node(odev);
	    odev->rise_time(rise);
	    odev->fall_time(fall);
	    odev->decay_time(decay);
	    odev->set_line(*this);

	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, lwidth);
	    osig->set_line(*this);
	    osig->local_flag(true);
	    osig->data_type(IVL_VT_LOGIC);

	    connect(odev->pin(0), osig->pin(0));

	    return osig;
      }

      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

	/* The arguments of a multiply must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Arguments of multiply "
		 << "have different data types." << endl;
	    cerr << get_fileline() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	// The mult is signed if both its operands are signed.
      bool arith_is_signed = lsig->get_signed() && rsig->get_signed();

      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	      /* Reals are always 1 wide and lsig/rsig types match here. */
	    if (lsig->data_type() == IVL_VT_REAL) {
		  rwidth = 1;
		  lwidth = 1;
	    } else {
		  rwidth = lsig->vector_width() + rsig->vector_width();
		  lwidth = rwidth;
	    }
      }

      if (arith_is_signed) {
	    lsig = pad_to_width_signed(des, lsig, rwidth);
	    rsig = pad_to_width_signed(des, rsig, rwidth);
      }

      NetMult*mult = new NetMult(scope, scope->local_symbol(), rwidth,
				 lsig->vector_width(),
				 rsig->vector_width());
      mult->set_line(*this);
      mult->rise_time(rise);
      mult->fall_time(fall);
      mult->decay_time(decay);
      des->add_node(mult);

      mult->set_signed( arith_is_signed );

      connect(mult->pin_DataA(), lsig->pin(0));
      connect(mult->pin_DataB(), rsig->pin(0));

	/* Make a signal to carry the output from the multiply. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, rwidth);
      osig->data_type( lsig->data_type() );
      osig->local_flag(true);
      connect(mult->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* PEBinary::elaborate_net_pow_(Design*des, NetScope*scope,
				       unsigned lwidth,
				       const NetExpr* rise,
				       const NetExpr* fall,
				       const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0),
            *rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0 || rsig == 0) return 0;

	/* The arguments of a power must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_fileline() << ": error: Arguments of power "
		 << "have different data types." << endl;
	    cerr << get_fileline() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* The power is signed if either of its operands are signed. */
      bool arith_is_signed = lsig->get_signed() || rsig->get_signed();

      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	      /* Reals are always 1 wide and lsig/rsig types match here. */
	    if (lsig->data_type() == IVL_VT_REAL) {
		  rwidth = 1;
		  lwidth = 1;
	    } else {
		    /* This is incorrect! a * (2^b - 1) is close. */
		  rwidth = lsig->vector_width() + rsig->vector_width();
		  lwidth = rwidth;
	    }
      }

      if (arith_is_signed) {
	    lsig = pad_to_width_signed(des, lsig, rwidth);
	    rsig = pad_to_width_signed(des, rsig, rwidth);
      }

      NetPow*powr = new NetPow(scope, scope->local_symbol(), rwidth,
			       lsig->vector_width(),
			       rsig->vector_width());
      powr->set_line(*this);
      powr->rise_time(rise);
      powr->fall_time(fall);
      powr->decay_time(decay);
      des->add_node(powr);

      powr->set_signed( arith_is_signed );

      connect(powr->pin_DataA(), lsig->pin(0));
      connect(powr->pin_DataB(), rsig->pin(0));

	/* Make a signal to carry the output from the power. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, rwidth);
      osig->data_type( lsig->data_type() );
      osig->local_flag(true);
      connect(powr->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* PEBinary::elaborate_net_shift_(Design*des, NetScope*scope,
				       unsigned lwidth,
				       const NetExpr* rise,
				       const NetExpr* fall,
				       const NetExpr* decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0);

      if (lsig == 0) return 0;

        /* Cannot shift a real value. */
      if (lsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: shift operators ("
	         << human_readable_op(op_)
	         << ") cannot shift a real value." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (lsig->vector_width() > lwidth)
	    lwidth = lsig->vector_width();

      bool right_flag  =  op_ == 'r' || op_ == 'R';
      bool signed_flag =  op_ == 'R';

	/* Handle the special case of a constant shift amount. There
	   is no reason in this case to create a gate at all, just
	   connect the lsig to the osig with the bit positions
	   shifted. Use a NetPartSelect to select the parts of the
	   left expression that survive the shift, and a NetConcat to
	   concatenate a constant for padding. */
      if (verinum*rval = right_->eval_const(des, scope)) {
	    assert(rval->is_defined());
	    unsigned dist = rval->as_ulong();

	      /* Very special case: constant 0 shift. Simply return
		 the left signal again. */
	    if (dist == 0) return lsig;

	      /* The construction that I'm making will ultimately
		 connect its output to the osig here. This will be the
		 result that I return from this function. */
	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, lwidth);
	    osig->data_type( lsig->data_type() );
	    osig->local_flag(true);


	      /* Make the constant zero's that I'm going to pad to the
		 top or bottom of the left expression. Attach a signal
		 to its output so that I don't have to worry about it
		 later. If the left expression is less than the
		 desired width (and we are doing right shifts) then we
		 can combine the expression padding with the distance
		 padding to reduce nodes. */
	    unsigned pad_width = dist;
	    unsigned part_width = lwidth - dist;
	    if (op_ == 'r' || op_ == 'R') {
		  if (lsig->vector_width() < lwidth) {
			pad_width += lwidth - lsig->vector_width();
			part_width -= lwidth - lsig->vector_width();
		  }
	    } else {

		    /* The left net must be the same width as the
		       result. The part select that I'm about to make relies
		       on that. */
		  lsig = pad_to_width(des, lsig, lwidth);

	    }

	    NetNet*zero = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, pad_width);
	    zero->data_type( lsig->data_type() );
	    zero->local_flag(true);
	    zero->set_line(*this);

	    /* Padding bits are zero in most cases, but copies of
	     * the sign bit in the case of a signed right shift */
	    if (op_ == 'R') {
		  NetPartSelect*sign_bit
			= new NetPartSelect(lsig, lsig->vector_width()-1,
					    1, NetPartSelect::VP);
		  des->add_node(sign_bit);
		  NetReplicate*sign_pad
			= new NetReplicate(scope, scope->local_symbol(),
					   pad_width, pad_width);
		  des->add_node(sign_pad);
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE, 1);
		  tmp->data_type( lsig->data_type() );
		  tmp->local_flag(true);
		  tmp->set_line(*this);
		  connect(sign_bit->pin(0), tmp->pin(0));
		  connect(sign_bit->pin(0), sign_pad->pin(1));

		  connect(zero->pin(0), sign_pad->pin(0));

	    } else {
		  NetConst*zero_c = new NetConst(scope, scope->local_symbol(),
					      verinum(verinum::V0, pad_width));
		  des->add_node(zero_c);
		  connect(zero->pin(0), zero_c->pin(0));
	    }

	    /* If all data bits get shifted away, connect the zero or
	     * padding bits directly to output, and stop before building the
	     * concatenation. */
	    if (dist >= lwidth) {
		  connect(osig->pin(0), zero->pin(0));
		  return osig;
	    }

	      /* Make a concatenation operator that will join the
		 part-selected right expression at the pad values. */
	    NetConcat*cc = new NetConcat(scope, scope->local_symbol(),
					 lwidth, 2);
	    cc->set_line(*this);
	    des->add_node(cc);
	    connect(cc->pin(0), osig->pin(0));

	      /* Make the part select of the left expression and
		 connect it to the LSB or MSB of the concatenation,
		 depending on the direction of the shift. */
	    NetPartSelect*part;

	    switch (op_) {
		case 'l': // Left shift === {lsig, zero}
		  part = new NetPartSelect(lsig, 0, part_width,
					   NetPartSelect::VP);
		  connect(cc->pin(1), zero->pin(0));
		  connect(cc->pin(2), part->pin(0));
		  break;
		case 'R':
		case 'r': // right-shift === {zero, lsig}
		  part = new NetPartSelect(lsig, dist, part_width,
					   NetPartSelect::VP);
		  connect(cc->pin(1), part->pin(0));
		  connect(cc->pin(2), zero->pin(0));
		  break;
		default:
		  assert(0);
	    }

	    part->set_line(*this);
	    part->rise_time(rise);
	    part->fall_time(fall);
	    part->decay_time(decay);
	    des->add_node(part);

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate shift "
		       << "(" << op_ << ") as concatenation of "
		       << pad_width << " zeros with " << part_width
		       << " bits of expression." << endl;
	    }

	      /* Attach a signal to the part select output (NetConcat
		 input) */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, part_width);
	    tmp->data_type( lsig->data_type() );
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    connect(part->pin(0), tmp->pin(0));

	    return osig;
      }

	// Calculate the number of useful bits for the shift amount,
	// and elaborate the right_ expression as the shift amount.
      unsigned dwid = 0;
      while ((1U << dwid) < lwidth)
	    dwid += 1;

      NetNet*rsig = right_->elaborate_net(des, scope, dwid, 0, 0, 0);

      if (rsig == 0) return 0;

        /* You cannot shift a value by a real amount. */
      if (rsig->data_type() == IVL_VT_REAL) {
	    cerr << get_fileline() << ": error: shift operators "
	            "cannot shift by a real value." << endl;
	    des->errors += 1;
	    return 0;
      }

	// Make the shift device itself, and the output
	// NetNet. Connect the Result output pins to the osig signal
      NetCLShift*gate = new NetCLShift(scope, scope->local_symbol(),
				       lwidth, rsig->vector_width(),
				       right_flag, signed_flag);
      gate->set_line(*this);
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, lwidth);
      osig->data_type( lsig->data_type() );
      osig->local_flag(true);
      osig->set_signed(signed_flag);

      connect(osig->pin(0), gate->pin_Result());

	// Connect the lsig (the left expression) to the Data input,
	// and pad it if necessary. The lwidth is the width of the
	// NetCLShift gate, and the D input must match.
      if (lsig->vector_width() < lwidth)
	    lsig = pad_to_width(des, lsig, lwidth);

      assert(lsig->vector_width() <= lwidth);
      connect(lsig->pin(0), gate->pin_Data());

	// Connect the rsig (the shift amount expression) to the
	// Distance input.
      connect(rsig->pin(0), gate->pin_Distance());

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: "
		 << "Elaborate LPM_SHIFT: width="<<gate->width()
		 << ", swidth="<< gate->width_dist() << endl;
      }

      return osig;
}

/*
 * This method elaborates a call to a function in the context of a
 * continuous assignment. The definition of the function contains a
 * list of the ports, and an output port. The NetEUFunc that I create
 * here has a port for all the input ports and the output port. The
 * ports are connected by pins.
 */
NetNet* PECallFunction::elaborate_net(Design*des, NetScope*scope,
				      unsigned width,
				      const NetExpr* rise,
				      const NetExpr* fall,
				      const NetExpr* decay,
				      Link::strength_t drive0,
				      Link::strength_t drive1) const
{
      unsigned errors = 0;
      unsigned func_pins = 0;

      if (path_.front().name[0] == '$')
	    return elaborate_net_sfunc_(des, scope,
					width, rise, fall, decay,
					drive0, drive1);


	/* Look up the function definition. */
      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	    cerr << get_fileline() << ": error: No function " << path_ <<
		  " in this context (" << scope_path(scope) << ")." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetScope*dscope = def->scope();
      assert(dscope);

	/* This must be a function that returns a signal. */
      assert(def->return_sig());

	/* check the validity of the parameters. */
      if (! check_call_matches_definition_(des, dscope))
	    return 0;

	/* Elaborate all the parameters of the function call,
	   and collect the resulting NetNet objects. All the
	   parameters take on the size of the target port. */

      svector<NetNet*> eparms (def->port_count());
      for (unsigned idx = 0 ;  idx < eparms.count() ;  idx += 1) {
	    const NetNet* port_reg = def->port(idx);
	    NetNet*tmp = parms_[idx]->elaborate_net(des, scope,
						    port_reg->vector_width(),
						    0, 0, 0,
						    Link::STRONG,
						    Link::STRONG);
	    if (tmp == 0) {
		  cerr << get_fileline() << ": error: Unable to elaborate "
		       << "port " << idx << " of call to " << path_ <<
			"." << endl;
		  errors += 1;
		  des->errors += 1;
		  continue;
	    }

	    func_pins += tmp->pin_count();
	    eparms[idx] = tmp;
      }

      if (errors > 0) return 0;

      NetUserFunc*net = new NetUserFunc(scope,
					scope->local_symbol(),
					dscope);
      net->set_line(*this);
      net->rise_time(rise);
      net->fall_time(fall);
      net->decay_time(decay);
      des->add_node(net);

	/* Create an output signal and connect it to the output pins
	   of the function net. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE,
			       def->return_sig()->vector_width());
      osig->local_flag(true);
      osig->data_type(def->return_sig()->data_type());

      connect(net->pin(0), osig->pin(0));

	/* Connect the parameter pins to the parameter expressions. */
      for (unsigned idx = 0 ; idx < eparms.count() ; idx += 1) {
	    const NetNet* port = def->port(idx);
	    NetNet*cur = eparms[idx];

	    NetNet*tmp = pad_to_width(des, cur, port->vector_width());

	    connect(net->pin(idx+1), tmp->pin(0));
      }

      return osig;
}

NetNet* PECallFunction::elaborate_net_sfunc_(Design*des, NetScope*scope,
					     unsigned width,
					     const NetExpr* rise,
					     const NetExpr* fall,
					     const NetExpr* decay,
					     Link::strength_t drive0,
					     Link::strength_t drive1) const
{
      perm_string name = peek_tail_name(path_);

	/* Handle the special case that the function call is to
	   $signed. This takes a single expression argument, and
	   forces it to be a signed result. Otherwise, it is as if the
	   $signed did not exist. */
      if (strcmp(name, "$signed") == 0) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $signed() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetNet*sub = expr->elaborate_net(des, scope, width, rise,
					     fall, decay, drive0, drive1);
	    sub->set_signed(true);
	    return sub;
      }

      /* handle $unsigned like $signed */
      if (strcmp(name, "$unsigned") == 0) {
	    if ((parms_.size() != 1) || (parms_[0] == 0)) {
		  cerr << get_fileline() << ": error: The $unsigned() function "
		       << "takes exactly one(1) argument." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    PExpr*expr = parms_[0];
	    NetNet*sub = expr->elaborate_net(des, scope, width, rise,
					     fall, decay, drive0, drive1);
	    sub->set_signed(false);
	    return sub;
      }

      const struct sfunc_return_type*def = lookup_sys_func(name);

        /* We cannot use the default value for system functions in a
         * continuous assignment since the function name is NULL. */
      if (def == 0 || def->name == 0) {
	    cerr << get_fileline() << ": error: System function "
		 << peek_tail_name(path_) << " not defined in system "
		 "table or SFT file(s)." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Net system function "
		 << name << " returns " << def->type << endl;
      }

      NetSysFunc*net = new NetSysFunc(scope, scope->local_symbol(),
				      def, 1+parms_.size());
      net->set_line(*this);
      net->rise_time(rise);
      net->fall_time(fall);
      net->decay_time(decay);
      des->add_node(net);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, def->wid);
      osig->local_flag(true);
      osig->set_signed(def->type==IVL_VT_REAL? true : false);
      osig->data_type(def->type);
      osig->set_line(*this);

      connect(net->pin(0), osig->pin(0));

      unsigned errors = 0;
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1) {
	    NetNet*tmp = parms_[idx]->elaborate_net(des, scope, 0,
						    0, 0, 0,
						    Link::STRONG, Link::STRONG);
	    if (tmp == 0) {
		  cerr << get_fileline() << ": error: Unable to elaborate "
		       << "port " << idx << " of call to " << path_ <<
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


/*
 * The concatenation operator, as a net, is a wide signal that is
 * connected to all the pins of the elaborated expression nets.
 */
NetNet* PEConcat::elaborate_net(Design*des, NetScope*scope,
				unsigned,
				const NetExpr* rise,
				const NetExpr* fall,
				const NetExpr* decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{
      svector<NetNet*>nets (parms_.count());
      unsigned vector_width = 0;
      unsigned errors = 0;
      unsigned repeat = 1;

	/* The repeat expression must evaluate to a compile-time
	   constant. This is used to generate the width of the
	   concatenation. */
      if (repeat_) {
	    NetExpr*etmp = elab_and_eval(des, scope, repeat_, -1);
	    assert(etmp);
	    NetEConst*erep = dynamic_cast<NetEConst*>(etmp);

	    if (erep == 0) {
		  cerr << get_fileline() << ": error: Unable to "
		       << "evaluate constant repeat expression." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (!erep->value().is_defined()) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be undefined (" << erep->value()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (erep->value().is_negative()) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
		       << "may not be negative (" << erep->value().as_long()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = erep->value().as_ulong();
	    delete etmp;

	    if (repeat == 0) {
		  cerr << get_fileline() << ": error: Concatenation repeat "
			"may not be zero."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }
      }

      if (debug_elaborate) {
	    cerr << get_fileline() <<": debug: PEConcat concatenation repeat="
		 << repeat << "." << endl;
      }

	/* The operands of the concatenation must contain all
	   self-determined arguments. Set this flag to force an error
	   message if this is not the case. */
      const bool save_flag = must_be_self_determined_flag;
      must_be_self_determined_flag = true;

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  errors += 1;
		  continue;
	    }

	      /* Look for the special case of an unsized number in a
		 concatenation expression. Mark this as an error, but
		 allow elaboration to continue to see if I can find
		 more errors. */

	    if (PENumber*tmp = dynamic_cast<PENumber*>(parms_[idx])) {
		  if (tmp->value().has_len() == false) {
			cerr << get_fileline() << ": error: Number "
			     << tmp->value() << " with indefinite size"
			     << " in concatenation." << endl;
			errors += 1;
		  }
	    }

	    nets[idx] = parms_[idx]->elaborate_net(des, scope, 0, 0, 0, 0);
	    if (nets[idx] == 0)
		  errors += 1;
	    else
		  vector_width += nets[idx]->vector_width();
      }

      must_be_self_determined_flag = save_flag;

	/* If any of the sub expressions failed to elaborate, then
	   delete all those that did and abort myself. */
      if (errors) {
	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  if (nets[idx]) delete nets[idx];
	    }
	    des->errors += errors;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_fileline() <<": debug: PEConcat concat collected "
		 << "width=" << vector_width << ", repeat=" << repeat
		 << " of " << nets.count() << " expressions." << endl;
      }

      NetConcat*dev = new NetConcat(scope, scope->local_symbol(),
				    vector_width*repeat,
				    nets.count()*repeat);
      dev->set_line(*this);
      dev->rise_time(rise);
      dev->fall_time(fall);
      dev->decay_time(decay);
      des->add_node(dev);

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from least significant to most significant,
	   which is opposite from how they are given in the list.

	   Allow for a repeat count other than 1 by repeating the
	   connect loop as many times as necessary. */

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, vector_width * repeat);
      osig->data_type(IVL_VT_LOGIC);

      connect(dev->pin(0), osig->pin(0));

      unsigned cur_pin = 1;
      for (unsigned rpt = 0; rpt < repeat ;  rpt += 1) {
	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  NetNet*cur = nets[nets.count()-idx-1];
		  connect(dev->pin(cur_pin++), cur->pin(0));
	    }
      }

      osig->local_flag(true);
      return osig;
}

/*
 * This private method handles the special case that we have a
 * non-constant bit-select of an identifier. We already know that the
 * signal that is represented is "sig".
 */
NetNet* PEIdent::elaborate_net_bitmux_(Design*des, NetScope*scope,
				       NetNet*sig,
				       const NetExpr* rise,
				       const NetExpr* fall,
				       const NetExpr* decay,
				       Link::strength_t drive0,
				       Link::strength_t drive1) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.sel == index_component_t::SEL_BIT);
      ivl_assert(*this, index_tail.msb != 0);
      ivl_assert(*this, index_tail.lsb == 0);

	/* Elaborate the selector. */
      NetNet*sel;

      if (sig->msb() < sig->lsb()) {
	    NetExpr*sel_expr = index_tail.msb->elaborate_expr(des, scope, -1, false);
	    sel_expr = make_sub_expr(sig->lsb(), sel_expr);
	    eval_expr(sel_expr);

	    sel = sel_expr->synthesize(des);

      } else if (sig->lsb() != 0) {
	    NetExpr*sel_expr = index_tail.msb->elaborate_expr(des, scope, -1,false);
	    sel_expr = make_add_expr(sel_expr, - sig->lsb());
	    eval_expr(sel_expr);

	    sel = sel_expr->synthesize(des);

      } else {
	    sel = index_tail.msb->elaborate_net(des, scope, 0, 0, 0, 0);
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Create NetPartSelect "
		 << "using signal " << sel->name() << " as selector"
		 << endl;
      }

	/* Create a part select that takes a non-constant offset and a
	   width of 1. */
      NetPartSelect*mux = new NetPartSelect(sig, sel, 1);
      des->add_node(mux);
      mux->set_line(*this);

      NetNet*out = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, 1);
      out->data_type(sig->data_type());

      connect(out->pin(0), mux->pin(0));
      return out;
}

NetNet* PEIdent::elaborate_net(Design*des, NetScope*scope,
			       unsigned lwidth,
			       const NetExpr* rise,
			       const NetExpr* fall,
			       const NetExpr* decay,
			       Link::strength_t drive0,
			       Link::strength_t drive1) const
{
      ivl_assert(*this, scope);

      const name_component_t&name_tail = path_.back();

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*id_msb;
      const NetExpr*id_lsb;
      symbol_search(des, scope, path_, sig, par, eve, id_msb, id_lsb);

	/* If this is a parameter name, then create a constant node
	   that connects to a signal with the correct name. */
      if (par != 0) {

	      // Detect and handle the special case that we have a
	      // real valued parameter. Return a NetLiteral and a
	      // properly typed net.
	    if (const NetECReal*pc = dynamic_cast<const NetECReal*>(par)) {
		  NetLiteral*tmp = new NetLiteral(scope, scope->local_symbol(),
						  pc->value());
		  des->add_node(tmp);
		  tmp->set_line(*par);
		  sig = new NetNet(scope, scope->local_symbol(),
				   NetNet::IMPLICIT);
		  sig->set_line(*tmp);
		  sig->data_type(tmp->data_type());
		  sig->local_flag(true);

		  connect(tmp->pin(0), sig->pin(0));
		  return sig;
	    }

	    const NetEConst*pc = dynamic_cast<const NetEConst*>(par);
	    if (pc == 0) {
		  cerr << get_fileline() << ": internal error: "
		       << "Non-consant parameter value?: " << *par << endl;
		  cerr << get_fileline() << ":               : "
		       << "Expression type is " << par->expr_type() << endl;
	    }
	    ivl_assert(*this, pc);
	    verinum pvalue = pc->value();

	      /* If the parameter has declared dimensions, then apply
		 those to the dimensions of the net that we create. */
	    long msb = pvalue.len()-1;
	    long lsb = 0;
	    if (id_msb || id_lsb) {
		  assert(id_msb && id_lsb);
		  const NetEConst*tmp = dynamic_cast<const NetEConst*>(id_msb);
		  ivl_assert(*this, tmp);
		  msb = tmp->value().as_long();

		  tmp = dynamic_cast<const NetEConst*>(id_lsb);
		  ivl_assert(*this, tmp);
		  lsb = tmp->value().as_long();
	    }

	      /* If the constant is smaller than its defined width extend
		 the value. If needed this will be padded later to fit
		 the real signal width. */
	    unsigned  pwidth = msb > lsb ? msb - lsb : lsb - msb;
	    if (pwidth > pvalue.len()) {
		  verinum tmp ((uint64_t)0, pwidth);
		  for (unsigned idx = 0 ;  idx < pvalue.len() ;  idx += 1)
			tmp.set(idx, pvalue.get(idx));

		  pvalue = tmp;
	    }

	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::IMPLICIT, msb, lsb);
	    sig->set_line(*this);
	    sig->data_type(IVL_VT_LOGIC);
	    sig->local_flag(true);
	    NetConst*cp = new NetConst(scope, scope->local_symbol(),
				       pvalue);
	    cp->set_line(*this);
	    des->add_node(cp);
	    for (unsigned idx = 0;  idx <  sig->pin_count(); idx += 1)
		  connect(sig->pin(idx), cp->pin(idx));
      }

	/* Check for the error case that the name is not found, and it
	   is hierarchical. We can't just create a name in another
	   scope, it's just not allowed. */
      if (sig == 0 && path_.size() != 1) {
	    cerr << get_fileline() << ": error: The hierarchical name "
		 << path_ << " is undefined in "
		 << scope_path(scope) << "." << endl;

	    pform_name_t tmp_path = path_;
	    tmp_path.pop_back();

	    list<hname_t> stmp_path = eval_scope_path(des, scope, tmp_path);
	    NetScope*tmp_scope = des->find_scope(scope, stmp_path);
	    if (tmp_scope == 0) {
		  cerr << get_fileline() << ":      : I can't even find "
		       << "the scope " << tmp_path << "." << endl;
	    }

	    des->errors += 1;
	    return 0;
      }

	/* Fallback, this may be an implicitly declared net. */
      if (sig == 0) {
	    NetNet::Type nettype = scope->default_nettype();
	    sig = new NetNet(scope, name_tail.name,
			     nettype, 1);
	    sig->data_type(IVL_VT_LOGIC);

	    if (error_implicit || (nettype == NetNet::NONE)) {
		  cerr << get_fileline() << ": error: "
		       << scope_path(scope) << "." << name_tail.name
		       << " not defined in this scope." << endl;
		  des->errors += 1;

	    } else if (warn_implicit) {
		  cerr << get_fileline() << ": warning: implicit "
			"definition of wire " << scope_path(scope)
		       << "." << name_tail.name << "." << endl;
	    }
      }

      ivl_assert(*this, sig);

	/* Handle the case that this is an array elsewhere. */
      if (sig->array_dimensions() > 0) {
	    if (name_tail.index.size() == 0) {
		  cerr << get_fileline() << ": error: Array " << sig->name()
		       << " cannot be used here without an index." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    return elaborate_net_array_(des, scope, sig, lwidth,
					rise, fall, decay,
					drive0, drive1);
      }

      return elaborate_net_net_(des, scope, sig, lwidth,
				rise, fall, decay, drive0, drive1);

}

NetNet* PEIdent::process_select_(Design*des, NetScope*scope,
				 NetNet*sig) const
{

	// If there are more index items then there are array
	// dimensions, then treat them as word part selects. For
	// example, if this is a memory array, then array dimensions
	// is the first and part select the remainder.
      long midx, lidx;
      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return sig;

      ivl_assert(*this, lidx >= 0);
      unsigned part_count = midx-lidx+1;

	// Maybe this is a full-width constant part select? If
	// so, do nothing.
      if (part_count == sig->vector_width())
	    return sig;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate part select"
		 << " of word from " << sig->name() << "[base="<<lidx
		 << " wid=" << part_count << "]" << endl;
      }

      NetPartSelect*ps = new NetPartSelect(sig, lidx, part_count,
					   NetPartSelect::VP);
      ps->set_line(*sig);
      des->add_node(ps);

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, part_count-1, 0);
      tmp->data_type( sig->data_type() );
      tmp->local_flag(true);
      connect(tmp->pin(0), ps->pin(0));

      return tmp;
}

NetNet* PEIdent::elaborate_net_net_(Design*des, NetScope*scope,
				      NetNet*sig, unsigned lwidth,
				      const NetExpr* rise,
				      const NetExpr* fall,
				      const NetExpr* decay,
				      Link::strength_t drive0,
				      Link::strength_t drive1) const
{
      const name_component_t&name_tail = path_.back();

      index_component_t::ctype_t use_sel = index_component_t::SEL_NONE;
      if (!name_tail.index.empty())
	    use_sel = name_tail.index.back().sel;

      switch (use_sel) {
	  case index_component_t::SEL_IDX_UP:
	  case index_component_t::SEL_IDX_DO:
	    return elaborate_net_net_idx_up_(des, scope, sig, lwidth,
					     rise, fall, decay, drive0, drive1);

	  default:
	    break;
      }

	/* Catch the case of a non-constant bit select. That should be
	   handled elsewhere. */
      if (use_sel == index_component_t::SEL_BIT) {
	    const index_component_t&index_tail = name_tail.index.back();

	    verinum*mval = index_tail.msb->eval_const(des, scope);
	    if (mval == 0) {
		  return elaborate_net_bitmux_(des, scope, sig, rise,
					       fall, decay, drive0, drive1);
	    }

	    delete mval;
      }

      long midx, lidx;
      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;

      unsigned part_count = midx-lidx+1;
      unsigned output_width = part_count;

	/* Detect and handle the special case that the entire part
	   select is outside the range of the signal. Return a
	   constant xxx. */
      if (midx < 0 || lidx >= (long)sig->vector_width()) {
	    ivl_assert(*this, sig->data_type() == IVL_VT_LOGIC);
	    verinum xxx (verinum::Vx, part_count);
	    NetConst*con = new NetConst(scope, scope->local_symbol(), xxx);
	    con->set_line(*sig);
	    des->add_node(con);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, part_count-1, 0);
	    tmp->data_type( sig->data_type() );
	    tmp->local_flag(true);
	    connect(tmp->pin(0), con->pin(0));
	    return tmp;
      }

      NetNet*below = 0;
      if (lidx < 0) {
	    ivl_assert(*this, sig->data_type() == IVL_VT_LOGIC);
	    unsigned xxx_wid = 0-lidx;
	    verinum xxx (verinum::Vx, xxx_wid);
	    NetConst*con = new NetConst(scope, scope->local_symbol(), xxx);
	    con->set_line(*sig);
	    des->add_node(con);

	    below = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, xxx_wid);
	    below->data_type( sig->data_type() );
	    below->local_flag(true);
	    connect(below->pin(0), con->pin(0));

	    lidx = 0;
	    part_count = midx-lidx+1;
      }

      NetNet*above = 0;
      if (midx >= (long)sig->vector_width()) {
	    ivl_assert(*this, sig->data_type() == IVL_VT_LOGIC);
	    unsigned xxx_wid = midx - sig->vector_width() + 1;
	    verinum xxx (verinum::Vx, xxx_wid);
	    NetConst*con = new NetConst(scope, scope->local_symbol(), xxx);
	    con->set_line(*sig);
	    des->add_node(con);

	    above = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, xxx_wid);
	    above->data_type( sig->data_type() );
	    above->local_flag(true);
	    connect(above->pin(0), con->pin(0));

	    midx = sig->vector_width()-1;
	    part_count = midx-lidx+1;
      }

      ivl_assert(*this, lidx >= 0);
      if (part_count != sig->vector_width()) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate part select "
		       << sig->name() << "[base="<<lidx
		       << " wid=" << part_count << "]" << endl;
	    }

	    NetPartSelect*ps = new NetPartSelect(sig, lidx, part_count,
						 NetPartSelect::VP);
	    ps->set_line(*sig);
	    des->add_node(ps);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, part_count-1, 0);
	    tmp->data_type( sig->data_type() );
	    tmp->local_flag(true);
	    connect(tmp->pin(0), ps->pin(0));

	    sig = tmp;
      }

      unsigned segment_count = 1;
      if (below) segment_count += 1;
      if (above) segment_count += 1;
      if (segment_count > 1) {
	    NetConcat*cc = new NetConcat(scope, scope->local_symbol(),
					 output_width, segment_count);
	    cc->set_line(*sig);
	    des->add_node(cc);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, output_width);
	    tmp->data_type( sig->data_type() );
	    tmp->local_flag(true);
	    connect(tmp->pin(0), cc->pin(0));

	    unsigned pdx = 1;
	    if (below) {
		  connect(cc->pin(pdx), below->pin(0));
		  pdx += 1;
	    }
	    connect(cc->pin(pdx), sig->pin(0));
	    pdx += 1;
	    if (above) {
		  connect(cc->pin(pdx), above->pin(0));
		  pdx += 1;
	    }
	    ivl_assert(*sig, segment_count == pdx-1);

	    sig = tmp;
      }

      return sig;
}

NetNet* PEIdent::elaborate_net_net_idx_up_(Design*des, NetScope*scope,
					   NetNet*sig, unsigned lwidth,
					   const NetExpr* rise,
					   const NetExpr* fall,
					   const NetExpr* decay,
					   Link::strength_t drive0,
					   Link::strength_t drive1) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();
      ivl_assert(*this, index_tail.lsb != 0);
      ivl_assert(*this, index_tail.msb != 0);

      NetExpr*base = elab_and_eval(des, scope, index_tail.msb, -1);

      unsigned long wid = 0;
      calculate_up_do_width_(des, scope, wid);

      bool down_flag = name_tail.index.back().sel==index_component_t::SEL_IDX_DO;

	// Handle the special case that the base is constant as
	// well. In this case it can be converted to a conventional
	// part select.
      if (NetEConst*base_c = dynamic_cast<NetEConst*> (base)) {
	    long lsv = base_c->value().as_long();

	      // convert from -: to +: form.
	    if (down_flag) lsv -= (wid-1);

	      // If the part select covers exactly the entire
	      // vector, then do not bother with it. Return the
	      // signal itself.
	    if (sig->sb_to_idx(lsv) == 0 && wid == sig->vector_width())
		  return sig;

	    NetPartSelect*sel = new NetPartSelect(sig, sig->sb_to_idx(lsv),
						  wid, NetPartSelect::VP);
	    sel->set_line(*this);
	    des->add_node(sel);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, wid);
	    tmp->set_line(*this);
	    tmp->data_type(sig->data_type());
	    connect(tmp->pin(0), sel->pin(0));

	    delete base;
	    return tmp;
      }

      if (sig->msb() > sig->lsb()) {
	    long offset = sig->lsb();
	    if (down_flag)
		  offset += (wid-1);
	    if (offset != 0)
		  base = make_add_expr(base, 0-offset);
      } else {
	    long vwid = sig->lsb() - sig->msb() + 1;
	    long offset = sig->msb();
	    if (down_flag)
		  offset += (wid-1);
	    base = make_sub_expr(vwid-offset-wid, base);
      }

      NetPartSelect*sel = new NetPartSelect(sig, base->synthesize(des), wid);
      sel->set_line(*this);
      des->add_node(sel);

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, wid);
      tmp->set_line(*this);
      tmp->data_type(sig->data_type());
      connect(tmp->pin(0), sel->pin(0));

      delete base;
      return tmp;
}

NetNet* PEIdent::elaborate_net_array_(Design*des, NetScope*scope,
				      NetNet*sig, unsigned lwidth,
				      const NetExpr* rise,
				      const NetExpr* fall,
				      const NetExpr* decay,
				      Link::strength_t drive0,
				      Link::strength_t drive1) const
{
      const name_component_t&name_tail = path_.back();
      ivl_assert(*this, name_tail.index.size() >= 1);
      const index_component_t&index_head = name_tail.index.front();
      ivl_assert(*this, index_head.sel == index_component_t::SEL_BIT);
      ivl_assert(*this, index_head.msb != 0);
      ivl_assert(*this, index_head.lsb == 0);

      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: elaborate array "
		 << name_tail.name << " with index " << index_head << endl;

      NetExpr*index_ex = elab_and_eval(des, scope, index_head.msb, -1);
      if (index_ex == 0)
	    return 0;

      if (NetEConst*index_co = dynamic_cast<NetEConst*> (index_ex)) {
	    long index = index_co->value().as_long();

	    if (!sig->array_index_is_valid(index)) {
		    // Oops! The index is a constant known to be
		    // outside the array. Change the expression to a
		    // constant X vector.
		  verinum xxx (verinum::Vx, sig->vector_width());
		  NetConst*con_n = new NetConst(scope, scope->local_symbol(), xxx);
		  des->add_node(con_n);
		  con_n->set_line(*index_co);

		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::IMPLICIT, sig->vector_width());
		  tmp->set_line(*this);
		  tmp->local_flag(true);
		  tmp->data_type(sig->data_type());
		  connect(tmp->pin(0), con_n->pin(0));
		  return tmp;
	    }

	    assert(sig->array_index_is_valid(index));
	    index = sig->array_index_to_address(index);

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate word "
		       << index << " of " << sig->name() << endl;
	    }

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, sig->vector_width());
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    tmp->data_type(sig->data_type());
	    connect(tmp->pin(0), sig->pin(index));

	      // If there are more indices then needed to get to the
	      // word, then there is a part/bit select for the word.
	    if (name_tail.index.size() > sig->array_dimensions())
		  tmp = process_select_(des, scope, tmp);

	    return tmp;
      }

      unsigned selwid = index_ex->expr_width();

      NetArrayDq*mux = new NetArrayDq(scope, scope->local_symbol(),
				      sig, selwid);
      mux->set_line(*this);
      des->add_node(mux);

	// Adjust the expression to calculate the canonical offset?
      if (long array_base = sig->array_first()) {
	    index_ex = make_add_expr(index_ex, 0-array_base);
      }

      NetNet*index_net = index_ex->synthesize(des);
      connect(mux->pin_Address(), index_net->pin(0));

      NetNet*tmp = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, sig->vector_width());
      tmp->set_line(*this);
      tmp->local_flag(true);
      tmp->data_type(sig->data_type());
      connect(tmp->pin(0), mux->pin_Result());
#if 0

	// If there are more index items then there are array
	// dimensions, then treat them as word part selects. For
	// example, if this is a memory array, then array dimensions
	// is 1 and
      unsigned midx, lidx;
      if (eval_part_select_(des, scope, sig, midx, lidx)) do {

	    unsigned part_count = midx-lidx+1;

	      // Maybe this is a full-width constant part select? If
	      // so, do nothing.
	    if (part_count == sig->vector_width())
		  break;

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate part select"
		       << " of word from " << sig->name() << "[base="<<lidx
		       << " wid=" << part_count << "]" << endl;
	    }

	    NetPartSelect*ps = new NetPartSelect(sig, lidx, part_count,
						 NetPartSelect::VP);
	    ps->set_line(*sig);
	    des->add_node(ps);

	    NetNet*tmp2 = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, part_count-1, 0);
	    tmp2->data_type( tmp->data_type() );
	    tmp2->local_flag(true);
	    connect(tmp2->pin(0), ps->pin(0));

	    tmp = tmp2;
      } while (0);
#else
      if (name_tail.index.size() > sig->array_dimensions())
	    tmp = process_select_(des, scope, tmp);

#endif
      return tmp;
}

/*
 * The concatenation is also OK an an l-value. This method elaborates
 * it as a structural l-value. The return values is the *input* net of
 * the l-value, which may feed via part selects to the final
 * destination. The caller can connect gate outputs to this signal to
 * make the l-value connections.
 */
NetNet* PEConcat::elaborate_lnet_common_(Design*des, NetScope*scope,
					 bool bidirectional_flag) const
{
      assert(scope);

      svector<NetNet*>nets (parms_.count());
      unsigned width = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_fileline() << ": sorry: I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Elaborate subexpression "
		       << idx << " of " << nets.count() << " l-values: "
		       << *parms_[idx] << endl;
	    }

	    if (parms_[idx] == 0) {
		  cerr << get_fileline() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  errors += 1;
		  continue;
	    }

	    if (bidirectional_flag) {
		  nets[idx] = parms_[idx]->elaborate_bi_net(des, scope);
	    } else {
		  nets[idx] = parms_[idx]->elaborate_lnet(des, scope);
	    }
	    if (nets[idx] == 0)
		  errors += 1;
	    else
		  width += nets[idx]->vector_width();
      }

	/* If any of the sub expressions failed to elaborate, then
	   delete all those that did and abort myself. */
      if (errors) {
	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  if (nets[idx]) delete nets[idx];
	    }
	    des->errors += errors;
	    return 0;
      }

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from most significant to least significant,
	   which is the order they are given in the concat list. */

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);

	/* Assume that the data types of the nets are all the same, so
	   we can take the data type of any, the first will do. */
      osig->data_type(nets[0]->data_type());
      osig->local_flag(true);
      osig->set_line(*this);

      if (bidirectional_flag) {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Generating tran(VP) "
		       << "to connect input l-value to subexpressions."
		       << endl;
	    }

	    for (unsigned idx = 0 ; idx < nets.count() ; idx += 1) {
		  unsigned wid = nets[idx]->vector_width();
		  unsigned off = width - wid;
		  NetTran*ps = new NetTran(scope, scope->local_symbol(),
					   osig->vector_width(), wid, off);
		  des->add_node(ps);
		  ps->set_line(*this);

		  connect(ps->pin(0), osig->pin(0));
		  connect(ps->pin(1), nets[idx]->pin(0));

		  ivl_assert(*this, wid <= width);
		  width -= wid;
	    }

      } else {
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: Generating part selects "
		       << "to connect input l-value to subexpressions."
		       << endl;
	    }

	    NetPartSelect::dir_t part_dir = NetPartSelect::VP;

	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  unsigned wid = nets[idx]->vector_width();
		  unsigned off = width - wid;
		  NetPartSelect*ps = new NetPartSelect(osig, off, wid, part_dir);
		  des->add_node(ps);
		  ps->set_line(*this);

		  connect(ps->pin(1), osig->pin(0));
		  connect(ps->pin(0), nets[idx]->pin(0));

		  assert(wid <= width);
		  width -= wid;
	    }
	    assert(width == 0);
      }

      osig->data_type(nets[0]->data_type());
      osig->local_flag(true);
      return osig;
}

NetNet* PEConcat::elaborate_lnet(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, false);
}

NetNet* PEConcat::elaborate_bi_net(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, true);
}

/*
 * Elaborate a number as a NetConst object.
 */
NetNet* PEFNumber::elaborate_net(Design*des, NetScope*scope,
				 unsigned lwidth,
				 const NetExpr* rise,
				 const NetExpr* fall,
				 const NetExpr* decay,
				 Link::strength_t drive0,
				 Link::strength_t drive1) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate real literal node, "
		 << "value=" << value() << "." << endl;
      }

      NetLiteral*obj = new NetLiteral(scope, scope->local_symbol(), value());
      obj->set_line(*this);
      obj->rise_time(rise);
      obj->fall_time(fall);
      obj->decay_time(decay);
      obj->pin(0).drive0(drive0);
      obj->pin(0).drive1(drive1);
      des->add_node(obj);

      NetNet*net = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, 1);
      net->data_type(IVL_VT_REAL);
      net->set_signed(true);
      net->local_flag(true);
      net->set_line(*this);

      connect(obj->pin(0), net->pin(0));

      return net;
}

/*
 * A private method to create an implicit net.
 */
NetNet* PEIdent::make_implicit_net_(Design*des, NetScope*scope) const
{
      NetNet::Type nettype = scope->default_nettype();
      assert(nettype != NetNet::NONE);

      NetNet*sig = new NetNet(scope, peek_tail_name(path_),
			      NetNet::IMPLICIT, 1);
      sig->set_line(*this);
	/* Implicit nets are always scalar logic. */
      sig->data_type(IVL_VT_LOGIC);

      if (warn_implicit) {
	    cerr << get_fileline() << ": warning: implicit "
		  "definition of wire logic " << scope_path(scope)
		 << "." << peek_tail_name(path_) << "." << endl;
      }

      return sig;
}

/*
 * This private method evaluates the part selects (if any) for the
 * signal. The sig argument is the NetNet already located for the
 * PEIdent name. The midx and lidx arguments are loaded with the
 * results, which may be the whole vector, or a single bit, or
 * anything in between. The values are in canonical indices.
 */
bool PEIdent::eval_part_select_(Design*des, NetScope*scope, NetNet*sig,
				long&midx, long&lidx) const
{
      const name_component_t&name_tail = path_.back();
	// Only treat as part/bit selects any index that is beyond the
	// word selects for an array. This is not an array, then
	// dimensions==0 and any index is treated as a select.
      if (name_tail.index.size() <= sig->array_dimensions()) {
	    midx = sig->vector_width()-1;
	    lidx = 0;
	    return true;
      }

      ivl_assert(*this, !name_tail.index.empty());

      const index_component_t&index_tail = name_tail.index.back();

      switch (index_tail.sel) {
	  default:
	    cerr << get_fileline() << ": internal error: "
		 << "Unexpected sel_ value = " << index_tail.sel << endl;
	    ivl_assert(*this, 0);
	    break;

	  case index_component_t::SEL_IDX_DO:
	  case index_component_t::SEL_IDX_UP: {
		NetExpr*tmp_ex = elab_and_eval(des, scope, index_tail.msb, -1);
		NetEConst*tmp = dynamic_cast<NetEConst*>(tmp_ex);
		if (!tmp) {
		      cerr << get_fileline() << ": error: indexed part select of "
		           << sig->name()
		           << " must be a constant in this context." << endl;
		      des->errors += 1;
		      return 0;
		}

		long midx_val = tmp->value().as_long();
		midx = sig->sb_to_idx(midx_val);
		delete tmp_ex;

		  /* The width (a constant) is calculated here. */
		unsigned long wid = 0;
		bool flag = calculate_up_do_width_(des, scope, wid);
		if (! flag)
		      return false;

		if (index_tail.sel == index_component_t::SEL_IDX_UP)
		      lidx = sig->sb_to_idx(midx_val+wid-1);
		else
		      lidx = sig->sb_to_idx(midx_val-wid+1);

		if (midx < lidx) {
		      long tmp = midx;
		      midx = lidx;
		      lidx = tmp;
		}

		break;
	  }

	  case index_component_t::SEL_PART: {

		long msb, lsb;
		/* bool flag = */ calculate_parts_(des, scope, msb, lsb);

		long lidx_tmp = sig->sb_to_idx(lsb);
		long midx_tmp = sig->sb_to_idx(msb);
		  /* Detect reversed indices of a part select. */
		if (lidx_tmp > midx_tmp) {
		      cerr << get_fileline() << ": error: Part select "
			   << sig->name() << "[" << msb << ":"
			   << lsb << "] indices reversed." << endl;
		      cerr << get_fileline() << ":      : Did you mean "
			   << sig->name() << "[" << lsb << ":"
			   << msb << "]?" << endl;
		      long tmp = midx_tmp;
		      midx_tmp = lidx_tmp;
		      lidx_tmp = tmp;
		      des->errors += 1;
		}

		  /* Detect a part select out of range. */
		if (midx_tmp >= (long)sig->vector_width() || lidx_tmp < 0) {
		      cerr << get_fileline() << ": warning: Part select "
			   << sig->name() << "[" << msb << ":"
			   << lsb << "] out of range." << endl;
#if 0
		      midx_tmp = sig->vector_width() - 1;
		      lidx_tmp = 0;
		      des->errors += 1;
#endif
		}

		midx = midx_tmp;
		lidx = lidx_tmp;
		break;
	  }

	  case index_component_t::SEL_BIT:
	    if (name_tail.index.size() > sig->array_dimensions()) {
		  verinum*mval = index_tail.msb->eval_const(des, scope);
		  if (mval == 0) {
			cerr << get_fileline() << ": error: Index of " << path_ <<
			      " needs to be constant in this context." <<
			      endl;
			cerr << get_fileline() << ":      : Index expression is: "
			     << *index_tail.msb << endl;
			cerr << get_fileline() << ":      : Context scope is: "
			     << scope_path(scope) << endl;
			des->errors += 1;
			return false;
		  }
		  assert(mval);

		  midx = sig->sb_to_idx(mval->as_long());
		  if (midx >= (long)sig->vector_width()) {
			cerr << get_fileline() << ": error: Index " << sig->name()
			     << "[" << mval->as_long() << "] out of range."
			     << endl;
			des->errors += 1;
			midx = 0;
		  }
		  lidx = midx;

	    } else {
		  cerr << get_fileline() << ": internal error: "
		       << "Bit select " << path_ << endl;
		  ivl_assert(*this, 0);
		  midx = sig->vector_width() - 1;
		  lidx = 0;
	    }
	    break;
      }

      return true;
}

/*
 * This is the common code for l-value nets and bi-directional
 * nets. There is very little that is different between the two cases,
 * so most of the work for both is done here.
 */
NetNet* PEIdent::elaborate_lnet_common_(Design*des, NetScope*scope,
					bool bidirectional_flag) const
{
      assert(scope);

      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, par, eve);

      if (eve != 0) {
	    cerr << get_fileline() << ": error: named events (" << path_
		 << ") cannot be l-values in continuous "
		 << "assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (sig == 0) {
	    cerr << get_fileline() << ": error: Net " << path_
		 << " is not defined in this context." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(sig);

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_fileline() << ": error: reg " << sig->name()
		 << "; cannot be driven by primitives"
		 << " or continuous assignment." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (sig->port_type() == NetNet::PINPUT) {
	    cerr << get_fileline() << ": warning: L-value ``"
		 << sig->name() << "'' is also an input port." << endl;
	    cerr << sig->get_fileline() << ": warning: input "
		 << sig->name() << "; is coerced to inout." << endl;
	    sig->port_type(NetNet::PINOUT);
      }

	// Default part select is the entire word.
      unsigned midx = sig->vector_width()-1, lidx = 0;
	// The default word select is the first.
      unsigned widx = 0;

      const name_component_t&name_tail = path_.back();

      if (sig->array_dimensions() > 0) {

	    if (name_tail.index.empty()) {
		  cerr << get_fileline() << ": error: array " << sig->name()
		       << " must be used with an index." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    const index_component_t&index_head = name_tail.index.front();
	    if (index_head.sel == index_component_t::SEL_PART) {
		  cerr << get_fileline() << ": error: cannot perform a part "
		       << "select on array " << sig->name() << "." << endl;
		  des->errors += 1;
		  return 0;
	    }
	    ivl_assert(*this, index_head.sel == index_component_t::SEL_BIT);

	    NetExpr*tmp_ex = elab_and_eval(des, scope, index_head.msb, -1);
	    NetEConst*tmp = dynamic_cast<NetEConst*>(tmp_ex);
	    if (!tmp) {
		  cerr << get_fileline() << ": error: array " << sig->name()
		       << " index must be a constant in this context." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    long widx_val = tmp->value().as_long();
	    widx = sig->array_index_to_address(widx_val);
	    delete tmp_ex;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Use [" << widx << "]"
		       << " to index l-value array." << endl;

	      /* The array has a part/bit select at the end. */
	    if (name_tail.index.size() > sig->array_dimensions()) {
		  long midx_tmp, lidx_tmp;
		  if (! eval_part_select_(des, scope, sig, midx_tmp, lidx_tmp))
		        return 0;
		  ivl_assert(*this, lidx_tmp >= 0);
		  midx = midx_tmp;
		  lidx = lidx_tmp;
	    }
      } else if (!name_tail.index.empty()) {
	    long midx_tmp, lidx_tmp;
	    if (! eval_part_select_(des, scope, sig, midx_tmp, lidx_tmp))
		  return 0;

	    ivl_assert(*this, lidx_tmp >= 0);
	    midx = midx_tmp;
	    lidx = lidx_tmp;
      }

      unsigned subnet_wid = midx-lidx+1;

      if (sig->pin_count() > 1) {
	    assert(widx < sig->pin_count());

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    sig->type(), sig->vector_width());
	    tmp->set_line(*this);
	    tmp->local_flag(true);
	    connect(sig->pin(widx), tmp->pin(0));
	    sig = tmp;
      }

	/* If the desired l-value vector is narrower then the
	   signal itself, then use a NetPartSelect node to
	   arrange for connection to the desired bits. All this
	   can be skipped if the desired with matches the
	   original vector. */

      if (subnet_wid != sig->vector_width()) {
	      /* If we are processing a tran or inout, then the
		 partselect is bi-directional. Otherwise, it is a
		 Part-to-Vector select. */

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: "
		       << "Elaborate lnet part select "
		       << sig->name()
		       << "[base=" << lidx
		       << " wid=" << subnet_wid <<"]"
		       << endl;

	    NetNet*subsig = new NetNet(sig->scope(),
				       sig->scope()->local_symbol(),
				       NetNet::WIRE, subnet_wid);
	    subsig->data_type( sig->data_type() );
	    subsig->local_flag(true);
	    subsig->set_line(*this);

	    if (bidirectional_flag) {
		    // Make a tran(VP)
		  NetTran*sub = new NetTran(scope, scope->local_symbol(),
					    sig->vector_width(),
					    subnet_wid, lidx);
		  sub->set_line(*this);
		  des->add_node(sub);
		  connect(sub->pin(0), sig->pin(0));
		  connect(sub->pin(1), subsig->pin(0));

	    } else {
		  NetPartSelect*sub = new NetPartSelect(sig, lidx, subnet_wid,
							NetPartSelect::PV);
		  des->add_node(sub);
		  sub->set_line(*this);
		  connect(sub->pin(0), subsig->pin(0));
	    }
	    sig = subsig;
      }

      return sig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, false);
}

NetNet* PEIdent::elaborate_bi_net(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, true);
}

/*
 * This method is used to elaborate identifiers that are ports to a
 * scope. The scope is presumed to be that of the module that has the
 * port. This elaboration is done inside the module, and is only done
 * to PEIdent objects. This method is used by elaboration of a module
 * instantiation (PGModule::elaborate_mod_) to get NetNet objects for
 * the port.
 */
NetNet* PEIdent::elaborate_port(Design*des, NetScope*scope) const
{
      NetNet*sig = des->find_signal(scope, path_);
      if (sig == 0) {
	    cerr << get_fileline() << ": error: no wire/reg " << path_
		 << " in module " << scope_path(scope) << "." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Check the port_type of the signal to make sure it is really
	   a port, and its direction is resolved. */
      switch (sig->port_type()) {
	  case NetNet::PINPUT:
	  case NetNet::POUTPUT:
	  case NetNet::PINOUT:
	    break;

	      /* If the name matches, but the signal is not a port,
		 then the user declared the object but there is no
		 matching input/output/inout declaration. */

	  case NetNet::NOT_A_PORT:
	    cerr << get_fileline() << ": error: signal " << path_ << " in"
		 << " module " << scope_path(scope) << " is not a port." << endl;
	    cerr << get_fileline() << ":      : Are you missing an input/"
		 << "output/inout declaration?" << endl;
	    des->errors += 1;
	    return 0;

	      /* This should not happen. A PWire can only become
		 PIMPLICIT if this is a UDP reg port, and the make_udp
		 function should turn it into an output.... I think. */

	  case NetNet::PIMPLICIT:
	    cerr << get_fileline() << ": internal error: signal " << path_
		 << " in module " << scope_path(scope) << " is left as "
		 << "port type PIMPLICIT." << endl;
	    des->errors += 1;
	    return 0;
      }

      long midx;
      long lidx;

	/* Evaluate the part/bit select expressions, to get the part
	   select of the signal that attaches to the port. Also handle
	   range and direction checking here. */

      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;


      unsigned swid = midx - lidx + 1;

      if (swid < sig->vector_width()) {
	    cerr << get_fileline() << ": XXXX: Forgot to implement part select"
		 << " of signal port." << endl;
      }

      return sig;
}

/*
 * Elaborate a number as a NetConst object.
 *
 * The code assumes that the result is IVL_VT_LOGIC.
 */
NetNet* PENumber::elaborate_net(Design*des, NetScope*scope,
				unsigned lwidth,
				const NetExpr* rise,
				const NetExpr* fall,
				const NetExpr* decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{
      NetNet *net;
      NetConst *con;

	/* If we are constrained by a l-value size, then just make a
	   number constant with the correct size and set as many bits
	   in that constant as make sense. Pad excess with
	   zeros. Also, assume that numbers are meant to be logic
	   type. */
      if (lwidth > 0) {
	    net = new NetNet(scope, scope->local_symbol(),
	                     NetNet::IMPLICIT, lwidth);
	    net->data_type(IVL_VT_LOGIC);
	    net->local_flag(true);
	    net->set_signed(value_->has_sign());

	    verinum num = pad_to_width(*value_, lwidth);
	    if (num.len() > lwidth)
		  num = verinum(num, lwidth);

	    con = new NetConst(scope, scope->local_symbol(), num);

	/* If the number has a length, then use that to size the
	   number. Generate a constant object of exactly the user
	   specified size. */
      } else if (value_->has_len()) {
	    net = new NetNet(scope, scope->local_symbol(),
	                     NetNet::IMPLICIT, value_->len());
	    net->data_type(IVL_VT_LOGIC);
	    net->local_flag(true);
	    net->set_signed(value_->has_sign());
	    con = new NetConst(scope, scope->local_symbol(),
	                       *value_);

	/* None of the above tight constraints are present, so make a
	   plausible choice for the width. Try to reduce the width as
	   much as possible by eliminating high zeros of unsigned
	   numbers. */
      } else {

	    if (must_be_self_determined_flag) {
		  cerr << get_fileline() << ": error: No idea how wide to "
		  << "make the unsized constant " << *value_ << "." << endl;
		  des->errors += 1;
	    }

	    unsigned width = value_->len();

	    if (value_->has_sign() && (value_->get(width-1) == verinum::V0)) {

		    /* If the number is signed, but known to be positive,
		       then reduce it down as if it were unsigned.  */
		  while (width > 1) {
			if (value_->get(width-1) != verinum::V0)
			      break;
			width -= 1;

	    }

	    } else if (value_->has_sign() == false) {
		  while ( (width > 1) && (value_->get(width-1) == verinum::V0))
			width -= 1;
	    }

	    verinum num (verinum::V0, width);
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1)
		  num.set(idx, value_->get(idx));

	    net = new NetNet(scope, scope->local_symbol(),
	                     NetNet::IMPLICIT, width);
	    net->data_type(IVL_VT_LOGIC);
	    net->local_flag(true);
	    con = new NetConst(scope, scope->local_symbol(), num);
      }

      con->rise_time(rise);
      con->fall_time(fall);
      con->decay_time(decay);
      con->pin(0).drive0(drive0);
      con->pin(0).drive1(drive1);

      connect(con->pin(0), net->pin(0));

      des->add_node(con);
      return net;
}

/*
 * A string is a NetEConst node that is made of the ASCII bits of the
 * string instead of the bits of a number. In fact, a string is just
 * another numeric notation.
 */
NetNet* PEString::elaborate_net(Design*des, NetScope*scope,
				unsigned lwidth,
				const NetExpr* rise,
				const NetExpr* fall,
				const NetExpr* decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{
      unsigned strbits = strlen(text_) * 8;
      NetNet*net;

	/* If we are constrained by a l-value size, then just make a
	   number constant with the correct size and set as many bits
	   in that constant as make sense. Pad excess with zeros. */
      if (lwidth > 0) {
	    net = new NetNet(scope, scope->local_symbol(),
			     NetNet::IMPLICIT, lwidth);

      } else {
	    net = new NetNet(scope, scope->local_symbol(),
			     NetNet::IMPLICIT, strbits);
      }
      net->data_type(IVL_VT_BOOL);
      net->local_flag(true);

	/* Make a verinum that is filled with the 0 pad. */
      verinum num(verinum::V0, net->vector_width());

      unsigned idx;
      for (idx = 0 ;  idx < num.len() && idx < strbits; idx += 1) {
	    char byte = text_[strbits/8 - 1 - idx/8];
	    char mask = 1<<(idx%8);
	    num.set(idx, (byte & mask)? verinum::V1 : verinum::V0);
      }

      NetConst*con = new NetConst(scope, scope->local_symbol(), num);
      con->set_line(*this);
      con->rise_time(rise);
      con->fall_time(fall);
      con->decay_time(decay);
      con->pin(0).drive0(drive0);
      con->pin(0).drive1(drive1);

      des->add_node(con);

      connect(con->pin(0), net->pin(0));

      return net;
}

/* Common processing for the case when a single argument is always
 * selected in a ternary operator. */
static void process_single_ternary(Design*des, const PExpr*base,
                                   unsigned width, NetNet*&sig)
{
	/* We must have a type for the signal. */
      if (sig->data_type() == IVL_VT_NO_TYPE) {
	    cerr << base->get_fileline() << ": internal error: constant "
		 << "selected ternary clause has NO TYPE." << endl;
	    des->errors += 1;
	    sig = 0;
      }

	/* Use the signal width if one is not provided.
	 * Pad or crop as needed. */
      if (width == 0) width = sig->vector_width();

      if (sig->vector_width() < width) sig = pad_to_width(des, sig, width);

      if (width < sig->vector_width()) sig = crop_to_width(des, sig, width);

      sig->set_line(*base);
}

/*
 * Elaborate the ternary operator in a netlist by creating a LPM_MUX
 * with width matching the result, size == 2 and 1 select input. These
 * expressions come from code like:
 *
 *        res = test ? a : b;
 *
 * The res has the width requested of this method, and the a and b
 * expressions have their own similar widths. The test expression is
 * only a single bit wide. The output from this function is a NetNet
 * object the width of the <res> expression and connected to the
 * Result pins of the LPM_MUX device. Any width not covered by the
 * width of the mux is padded with a NetConst device.
 */
NetNet* PETernary::elaborate_net(Design*des, NetScope*scope,
				 unsigned width,
				 const NetExpr* rise,
				 const NetExpr* fall,
				 const NetExpr* decay,
				 Link::strength_t drive0,
				 Link::strength_t drive1) const
{
      NetNet *expr_sig, *tru_sig, *fal_sig;

      NetExpr*expr = elab_and_eval(des, scope, expr_, 0);
      if (expr == 0) return 0;

	/* If we have a constant conditional we can avoid some steps. */
      switch (const_logical(expr)) {
	  case C_0:
            fal_sig = fal_->elaborate_net(des, scope, width, 0, 0, 0);
            if (fal_sig == 0) return 0;
            process_single_ternary(des, this, width, fal_sig);
            return fal_sig;
            break;

	  case C_1:
            tru_sig = tru_->elaborate_net(des, scope, width, 0, 0, 0);
            if (tru_sig == 0) return 0;
            process_single_ternary(des, this, width, tru_sig);
            return tru_sig;
            break;

	  default:
            tru_sig = tru_->elaborate_net(des, scope, width, 0, 0, 0);
            fal_sig = fal_->elaborate_net(des, scope, width, 0, 0, 0);
            /* We should really see if these are constant values and
             * mix them as needed so we can omit the MUX? below, but
             * since this is a very uncommon case, I'm going to pass
             * on this for now. */
            break;
      }
      expr_sig = expr->synthesize(des);

      if (expr_sig == 0 || tru_sig == 0 || fal_sig == 0) return 0;

	/* The type of the true and false expressions must
	   match. These become the type of the resulting
	   expression. */

      ivl_variable_type_t expr_type = tru_sig->data_type();

      if (tru_sig->data_type() != fal_sig->data_type()) {
	    cerr << get_fileline() << ": error: True and False clauses of"
		 << " ternary expression have different types." << endl;
	    cerr << get_fileline() << ":      : True clause is "
		 << tru_sig->data_type() << ", false clause is "
		 << fal_sig->data_type() << "." << endl;

	    des->errors += 1;
	    expr_type = IVL_VT_NO_TYPE;
	    return 0;

      } else if (expr_type == IVL_VT_NO_TYPE) {
	    cerr << get_fileline() << ": internal error: True and false "
		 << "clauses of ternary both have NO TYPE." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* The natural width of the expression is the width of the
	   largest condition. Normally they should be the same size,
	   but if we do not get a size from the context, or the
	   expressions resist, we need to cope. */
      unsigned iwidth = tru_sig->vector_width();
      if (fal_sig->vector_width() > iwidth) iwidth = fal_sig->vector_width();

	/* If the width is not passed from the context, then take the
	   widest result as our width. */
      if (width == 0) width = iwidth;

	/* If the expression has width, then generate a boolean result
	   by connecting an OR gate to calculate the truth value of
	   the result. In the end, the result needs to be a single bit. */
      if (expr_sig->vector_width() > 1) {
	    NetUReduce*log = new NetUReduce(scope, scope->local_symbol(),
					    NetUReduce::OR,
					    expr_sig->vector_width());
	    log->set_line(*this);
	    des->add_node(log);
	    connect(log->pin(1), expr_sig->pin(0));

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->data_type(IVL_VT_LOGIC);
	    tmp->local_flag(true);
	    connect(log->pin(0), tmp->pin(0));

	    expr_sig = tmp;
      }

      assert(expr_sig->vector_width() == 1);

	/* This is the width of the LPM_MUX device that I'm about to
	   create. It may be smaller then the desired output, but I'll
	   handle padding below. Note that in principle the
	   alternatives should be padded to the output width first,
	   but it is more efficient to pad them only just enough to
	   prevent loss, and do the finished padding later.

	   Create a NetNet object wide enough to hold the
	   result. Also, pad the result values (if necessary) so that
	   the mux inputs can be fully connected. */

      unsigned dwidth = (iwidth > width)? width : iwidth;

      NetNet*sig = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, dwidth);
      sig->data_type(expr_type);
      sig->local_flag(true);

      if (fal_sig->vector_width() < dwidth)
	    fal_sig = pad_to_width(des, fal_sig, dwidth);

      if (tru_sig->vector_width() < dwidth)
	    tru_sig = pad_to_width(des, tru_sig, dwidth);

      if (dwidth < fal_sig->vector_width())
	    fal_sig = crop_to_width(des, fal_sig, dwidth);

      if (dwidth < tru_sig->vector_width())
	    tru_sig = crop_to_width(des, tru_sig, dwidth);

	/* Make the device and connect its outputs to the osig and
	   inputs to the tru and false case nets. Also connect the
	   selector bit to the sel input.

	   The inputs are the 0 (false) connected to fal_sig and 1
	   (true) connected to tru_sig.  */

      NetMux*mux = new NetMux(scope, scope->local_symbol(), dwidth, 2, 1);
      connect(mux->pin_Sel(), expr_sig->pin(0));

	/* Connect the data inputs. */
      connect(mux->pin_Data(0), fal_sig->pin(0));
      connect(mux->pin_Data(1), tru_sig->pin(0));

	/* If there are non-zero output delays, then create bufz
	   devices to carry the propagation delays. Otherwise, just
	   connect the result to the output. */
      if (rise || fall || decay) {
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, dwidth);
	    tmp->data_type(expr_type);
	    tmp->local_flag(true);

	    NetBUFZ*tmpz = new NetBUFZ(scope, scope->local_symbol(), dwidth);
	    tmpz->rise_time(rise);
	    tmpz->fall_time(fall);
	    tmpz->decay_time(decay);
	    tmpz->pin(0).drive0(drive0);
	    tmpz->pin(0).drive1(drive1);

	    connect(mux->pin_Result(), tmp->pin(0));
	    connect(tmp->pin(0), tmpz->pin(1));
	    connect(sig->pin(0), tmpz->pin(0));

	    des->add_node(tmpz);
      } else {
	    connect(mux->pin_Result(), sig->pin(0));
      }

	/* If the MUX device result is too narrow to fill out the
	   desired result, pad with zeros... */
      assert(dwidth <= width);

      des->add_node(mux);

	/* If the MUX device results is too narrow to fill out the
	   desired result, then pad it. It is OK to have a too-narrow
	   result here because the dwidth choice is >= the width of
	   both alternatives. Thus, padding here is equivalent to
	   padding inside, and is cheaper. */
      if (dwidth < width)
	    sig = pad_to_width(des, sig, width);

      return sig;
}

NetNet* PEUnary::elaborate_net(Design*des, NetScope*scope,
			       unsigned width,
			       const NetExpr* rise,
			       const NetExpr* fall,
			       const NetExpr* decay,
			       Link::strength_t drive0,
			       Link::strength_t drive1) const
{
	// Some unary operands allow the operand to be
	// self-determined, and some do not.
      unsigned owidth = 0;
      switch (op_) {
	  case '~':
	  case '-':
	    owidth = width;
	    break;
      }

      NetExpr*expr = elab_and_eval(des, scope, expr_, owidth);
      if (expr == 0) return 0;

      NetNet* sig = 0;
      NetLogic*gate;


	// Handle the special case of a 2's complement of a constant
	// value. This can be reduced to a no-op on a precalculated
	// result.
      if (op_ == '-') {
	    if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
		  return elab_net_uminus_const_logic_(des, scope, tmp,
						      width, rise, fall, decay,
						      drive0, drive1);
	    }

	    if (NetECReal*tmp = dynamic_cast<NetECReal*>(expr)) {
		  return elab_net_uminus_const_real_(des, scope, tmp,
						     width, rise, fall, decay,
						     drive0, drive1);
	    }
      }

	// Handle the case that the expression is real-valued.
      if (expr->expr_type() == IVL_VT_REAL) {
	    return elab_net_unary_real_(des, scope, expr,
					width, rise, fall, decay,
					drive0, drive1);
      }

      NetNet* sub_sig = expr->synthesize(des);

      if (sub_sig == 0) return 0;

      delete expr;
      expr = 0;

      bool reduction=false;
      NetUReduce::TYPE rtype = NetUReduce::NONE;

      switch (op_) {
	  case '~': // Bitwise NOT
	    sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			     sub_sig->vector_width());
	    sig->data_type(sub_sig->data_type());
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_symbol(),
				2, NetLogic::NOT, sub_sig->vector_width());
	    gate->set_line(*this);
	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);

	    connect(gate->pin(1), sub_sig->pin(0));
	    connect(gate->pin(0), sig->pin(0));
	    break;

	  case 'm': // abs(sub_sig)
	      // If this expression is self determined, get its width
	      // from the sub_expression.
	    if (owidth == 0)
		  owidth = sub_sig->vector_width();

	    if (sub_sig->vector_width() < owidth)
		  sub_sig = pad_to_width(des, sub_sig, owidth);

	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::WIRE, owidth);
	    sig->set_line(*this);
	    sig->data_type(sub_sig->data_type());
	    sig->local_flag(true);

	    { NetAbs*tmp = new NetAbs(scope, scope->local_symbol(), sub_sig->vector_width());
	      tmp->set_line(*this);
	      des->add_node(tmp);
	      tmp->rise_time(rise);
	      tmp->fall_time(fall);
	      tmp->decay_time(decay);

	      connect(tmp->pin(1), sub_sig->pin(0));
	      connect(tmp->pin(0), sig->pin(0));
	    }
	    break;

	  case 'N': // Reduction NOR
	  case '!': // Reduction NOT
	    reduction=true; rtype = NetUReduce::NOR; break;
	  case '&': // Reduction AND
	    reduction=true; rtype = NetUReduce::AND; break;
	  case '|': // Reduction OR
	    reduction=true; rtype = NetUReduce::OR; break;
	  case '^': // Reduction XOR
	    reduction=true; rtype = NetUReduce::XOR; break;
	  case 'A': // Reduction NAND (~&)
	    reduction=true; rtype = NetUReduce::NAND; break;
	  case 'X': // Reduction XNOR (~^)
	    reduction=true; rtype = NetUReduce::XNOR; break;

	  case '-': // Unary 2's complement.
	      // If this expression is self determined, get its width
	      // from the sub_expression.
	    if (owidth == 0)
		  owidth = sub_sig->vector_width();

	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::WIRE, owidth);
	    sig->data_type(sub_sig->data_type());
	    sig->local_flag(true);

	    if (sub_sig->vector_width() < owidth)
		  sub_sig = pad_to_width(des, sub_sig, owidth);

	    switch (sub_sig->vector_width()) {
		case 0:
		  assert(0);
		  break;

		case 1:
		  gate = new NetLogic(scope, scope->local_symbol(),
				      2, NetLogic::BUF, 1);
		  connect(gate->pin(0), sig->pin(0));
		  connect(gate->pin(1), sub_sig->pin(0));
		  des->add_node(gate);
		  gate->set_line(*this);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  break;

		default:
		  NetAddSub*sub = new NetAddSub(scope, scope->local_symbol(),
						sig->vector_width());
		  sub->attribute(perm_string::literal("LPM_Direction"),
				 verinum("SUB"));
		  sub->set_line(*this);
		  sub->rise_time(rise);
		  sub->fall_time(fall);
		  sub->decay_time(decay);
		  des->add_node(sub);

		  connect(sig->pin(0), sub->pin_Result());
		  connect(sub_sig->pin(0), sub->pin_DataB());

		  verinum tmp_num (verinum::V0, sub->width(), true);
		  NetConst*tmp_con = new NetConst(scope,
						  scope->local_symbol(),
						  tmp_num);
		  des->add_node(tmp_con);

		  NetNet*tmp_sig = new NetNet(scope, scope->local_symbol(),
					      NetNet::WIRE,
					      sub_sig->vector_width());
		  tmp_sig->data_type(sub_sig->data_type());
		  tmp_sig->local_flag(true);

		  connect(tmp_sig->pin(0), sub->pin_DataA());
		  connect(tmp_sig->pin(0), tmp_con->pin(0));
		  break;
	    }
	    break;

	  default:
	    cerr << get_fileline() << ": internal error: Unhandled UNARY '" << op_ << "'" << endl;
	    sig = 0;
      }

      if (reduction) {
	    NetUReduce*rgate;

	      // The output of a reduction operator is 1 bit
	    sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, 1);
	    sig->data_type(sub_sig->data_type());
	    sig->local_flag(true);

	    rgate = new NetUReduce(scope, scope->local_symbol(),
				   rtype, sub_sig->vector_width());
	    rgate->set_line(*this);
	    connect(rgate->pin(0), sig->pin(0));
	    connect(rgate->pin(1), sub_sig->pin(0));

	    des->add_node(rgate);
	    rgate->rise_time(rise);
	    rgate->fall_time(fall);
	    rgate->decay_time(decay);
      }

      return sig;
}

NetNet* PEUnary::elab_net_uminus_const_logic_(Design*des, NetScope*scope,
					      NetEConst*expr,
					      unsigned width,
					      const NetExpr* rise,
					      const NetExpr* fall,
					      const NetExpr* decay,
					      Link::strength_t drive0,
					      Link::strength_t drive1) const
{
      verinum val = expr->value();

      if (width == 0)
	    width = val.len();

      assert(width > 0);
      NetNet*sig = new NetNet(scope, scope->local_symbol(),
		       NetNet::WIRE, width);
      sig->data_type(IVL_VT_LOGIC);
      sig->local_flag(true);
      sig->set_line(*this);

	/* Take the 2s complement by taking the 1s complement
	and adding 1. */
      verinum tmp (v_not(val), width);
      verinum one (1UL, width);
      tmp = verinum(tmp + one, width);
      tmp.has_sign(val.has_sign());

      NetConst*con = new NetConst(scope, scope->local_symbol(), tmp);

      connect(con->pin(0), sig->pin(0));

      con->set_line(*this);
      con->rise_time(rise);
      con->fall_time(fall);
      con->decay_time(decay);
      con->pin(0).drive0(drive0);
      con->pin(0).drive1(drive1);

      des->add_node(con);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Replace expression "
		 << *this << " with constant " << tmp << "."<<endl;
      }

      delete expr;

      return sig;
}

NetNet* PEUnary::elab_net_uminus_const_real_(Design*des, NetScope*scope,
					      NetECReal*expr,
					      unsigned width,
					      const NetExpr* rise,
					      const NetExpr* fall,
					      const NetExpr* decay,
					      Link::strength_t drive0,
					      Link::strength_t drive1) const
{
      verireal val = expr->value();

      NetNet*sig = new NetNet(scope, scope->local_symbol(),
		       NetNet::WIRE, width);
      sig->data_type(IVL_VT_REAL);
      sig->set_signed(true);
      sig->local_flag(true);
      sig->set_line(*this);

      NetLiteral*con = new NetLiteral(scope, scope->local_symbol(), -val);

      connect(con->pin(0), sig->pin(0));

      con->set_line(*this);
      con->rise_time(rise);
      con->fall_time(fall);
      con->decay_time(decay);
      con->pin(0).drive0(drive0);
      con->pin(0).drive1(drive1);

      des->add_node(con);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Replace expression "
		 << *this << " with constant " << con->value_real() << "."<<endl;
      }

      delete expr;

      return sig;
}

NetNet* PEUnary::elab_net_unary_real_(Design*des, NetScope*scope,
				      NetExpr*expr,
				      unsigned width,
				      const NetExpr* rise,
				      const NetExpr* fall,
				      const NetExpr* decay,
				      Link::strength_t drive0,
				      Link::strength_t drive1) const
{
      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Elaborate real expression "
		 << *this << "."<<endl;
      }

      NetNet* sub_sig = expr->synthesize(des);

      if (sub_sig == 0) return 0;
      delete expr;

      NetNet*sig = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, 1);
      sig->data_type(IVL_VT_REAL);
      sig->set_signed(true);
      sig->local_flag(true);
      sig->set_line(*this);

      switch (op_) {

	  default:
	    cerr << get_fileline() << ": internal error: Unhandled UNARY "
		 << op_ << " expression with real values." << endl;
	    des->errors += 1;
	    break;
	  case '~':
	    cerr << get_fileline() << ": error: bit-wise negation ("
	         << human_readable_op(op_)
	         << ") may not have a REAL operand." << endl;
	    des->errors += 1;
	    break;
	  case '&':
	  case 'A':
	  case '|':
	  case 'N':
	  case '^':
	  case 'X':
	    cerr << get_fileline() << ": error: reduction operator ("
	         << human_readable_op(op_)
	         << ") may not have a REAL operand." << endl;
	    des->errors += 1;
	    break;
	  case '!':
	    cerr << get_fileline() << ": sorry: ! is currently unsupported"
		    " for real values." << endl;
	    des->errors += 1;
	    break;

	  case 'm': { // abs()
		NetAbs*tmp = new NetAbs(scope, scope->local_symbol(), 1);
		tmp->set_line(*this);
		tmp->rise_time(rise);
		tmp->fall_time(fall);
		tmp->decay_time(decay);
		des->add_node(tmp);

		connect(tmp->pin(0), sig->pin(0));
		connect(tmp->pin(1), sub_sig->pin(0));
		break;
	  }

	  case '-':
	    NetAddSub*sub = new NetAddSub(scope, scope->local_symbol(), 1);
	    sub->attribute(perm_string::literal("LPM_Direction"),
				 verinum("SUB"));
	    sub->set_line(*this);
	    sub->rise_time(rise);
	    sub->fall_time(fall);
	    sub->decay_time(decay);
	    des->add_node(sub);
	    connect(sig->pin(0), sub->pin_Result());
	    connect(sub_sig->pin(0), sub->pin_DataB());

	    NetLiteral*tmp_con = new NetLiteral(scope, scope->local_symbol(),
						verireal(0.0));
	    tmp_con->set_line(*this);
	    des->add_node(tmp_con);

	    NetNet*tmp_sig = new NetNet(scope, scope->local_symbol(),
					NetNet::WIRE, 1);
	    tmp_sig->data_type(IVL_VT_REAL);
	    tmp_sig->set_signed(true);
	    tmp_sig->local_flag(true);
	    tmp_sig->set_line(*this);

	    connect(tmp_sig->pin(0), sub->pin_DataA());
	    connect(tmp_sig->pin(0), tmp_con->pin(0));
	    break;
      }

      return sig;
}
