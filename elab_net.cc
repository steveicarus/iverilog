/*
 * Copyright (c) 1999-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_net.cc,v 1.177 2006/01/02 05:33:19 steve Exp $"
#endif

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

# include  <iostream>

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
      cerr << get_line() << ": error: Unable to elaborate `"
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

      NetNet*lsig = left_->elaborate_net(des, scope, width, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, scope, width, 0, 0, 0);
      if (lsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      NetNet*osig;

      switch (op_) {
	  case '^': // XOR
	  case 'X': // XNOR
	  case '&': // AND
	  case '|': // Bitwise OR
	    assert(0);
	    break;

	  case 'E': // === (Case equals)
	  case 'e': // ==
	  case 'n': // !=
	  case '<':
	  case '>':
	  case 'G': // >=
	  case 'L': // <=
	    assert(0);
	    break;

	  case '+':
	    assert(0);
	    break;

	  case 'l':
	  case 'r':
	  case 'R':
	    assert(0);
	    break;
	  default:
	    cerr << get_line() << ": internal error: unsupported"
		  " combinational operator (" << op_ << ")." << endl;
	    des->errors += 1;
	    osig = 0;
      }

      return osig;
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
      if (lsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      NetNet*osig;

      unsigned width = lsig->vector_width();
      if (rsig->vector_width() > lsig->vector_width())
	    width = rsig->vector_width();


	/* The owidth is the output width of the lpm_add_sub
	   device. If the desired width is greater then the width of
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


	// Pad out the operands, if necessary, the match the width of
	// the adder device.
      if (lsig->vector_width() < width)
	    lsig = pad_to_width(des, lsig, width);

      if (rsig->vector_width() < width)
	    rsig = pad_to_width(des, rsig, width);

	// Check that the argument types match.
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_line() << ": error: Arguments of add/sub "
		 << "have different data types." << endl;
	    cerr << get_line() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
      }

	// Make the adder as wide as the widest operand
      osig = new NetNet(scope, scope->local_symbol(),
			NetNet::WIRE, owidth);
      osig->data_type(lsig->data_type());
      osig->local_flag(true);
      if (debug_elaborate) {
	    cerr << get_line() << ": debug: Elaborate NetAddSub "
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
      if (lsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      if (lsig->vector_width() < rsig->vector_width())
	    lsig = pad_to_width(des, lsig, rsig->vector_width());
      if (rsig->vector_width() < lsig->vector_width())
	    rsig = pad_to_width(des, rsig, lsig->vector_width());

      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_line() << ": error: Types of "
		 << "operands of " << op_ << " do not match: "
		 << lsig->data_type() << " vs. " << rsig->data_type()
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      if (lsig->vector_width() != rsig->vector_width()) {
	    cerr << get_line() << ": internal error: lsig width ("
		 << lsig->vector_width() << ") != rsig pin width ("
		 << rsig->vector_width() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(lsig->vector_width() == rsig->vector_width());

      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			       lsig->vector_width());
      osig->local_flag(true);
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
			delete ogate;

			if (debug_elaborate)
			      cerr << lsig->get_line() << ": debug: "
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
			cerr << lsig->get_line() << ": debug: "
			     << "Replace net==" << val << " equality with "
			     << zeros << "-input reduction [N]OR gate." << endl;

	    } else {
		  type = op_code == 'e'? NetUReduce::AND : NetUReduce::NAND;

		  if (debug_elaborate) 
			cerr << lsig->get_line() << ": debug: "
			     << "Replace net==" << val << " equality with "
			     << ones << "-input reduction AND gate." << endl;
	    }

	    NetUReduce*red = new NetUReduce(scope, scope->local_symbol(),
					    type, zeros+ones);
	    des->add_node(red);
	    red->set_line(*lsig);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, 0, 0);
	    tmp->data_type(IVL_VT_LOGIC);
	    tmp->local_flag(true);
	    tmp->set_line(*lsig);

	    connect(red->pin(1), lsig->pin(0));
	    connect(red->pin(0), tmp->pin(0));
	    return tmp;
      }

      if (debug_elaborate)
	    cerr << lsig->get_line() << ": debug: "
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
      NetExpr*lexp = elab_and_eval(des, scope, left_);
      if (lexp == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      NetExpr*rexp = elab_and_eval(des, scope, right_);
      if (rexp == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

	/* Choose the operand width to be the width of the widest
	   self-determined operand. */
      unsigned operand_width = lexp->expr_width();
      if (rexp->expr_width() > operand_width)
	    operand_width = rexp->expr_width();

      lexp->set_width(operand_width);
      lexp = pad_to_width(lexp, operand_width);
      rexp->set_width(operand_width);
      rexp = pad_to_width(rexp, operand_width);

      NetNet*lsig = 0;
      NetNet*rsig = 0;

	/* Handle the special case that the right or left
	   sub-expression is a constant value. The compare_eq_constant
	   function will return an elaborated result if it can make
	   use of the situation, or 0 if it cannot. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(rexp)) {

	    lsig = lexp->synthesize(des);
	    if (lsig == 0) {
		  cerr << get_line() << ": internal error: "
			"Cannot elaborate net for " << *lexp << endl;
		  return 0;
	    }
	    assert(lsig);
	    delete lexp;
	    lexp = 0;

	    NetNet*osig = compare_eq_constant(des, scope,
					      lsig, tmp, op_,
					      rise, fall, decay);
	    if (osig != 0) {
		  delete rexp;
		  return osig;
	    }
      }

      if (NetEConst*tmp = dynamic_cast<NetEConst*>(lexp)) {

	    rsig = rexp->synthesize(des);
	    assert(rsig);
	    delete rexp;

	    NetNet*osig = compare_eq_constant(des, scope,
					      rsig, tmp, op_,
					      rise, fall, decay);
	    if (osig != 0) {
		  delete lexp;
		  return osig;
	    }
      }

      if (lsig == 0) {
	    lsig = lexp->synthesize(des);
	    assert(lsig);
	    delete lexp;
      }

      if (rsig == 0) {
	    rsig = rexp->synthesize(des);
	    assert(rsig);
	    delete rexp;
      }

      unsigned dwidth = lsig->vector_width();
      if (rsig->vector_width() > dwidth) dwidth = rsig->vector_width();

	/* Operands of binary compare need to be padded to equal
	   size. Figure the pad bit needed to extend the narrowest
	   vector. */
      if (lsig->vector_width() < dwidth)
	    lsig = pad_to_width(des, lsig, dwidth);
      if (rsig->vector_width() < dwidth)
	    rsig = pad_to_width(des, rsig, dwidth);


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
	    gate = new NetCaseCmp(scope, scope->local_symbol(), dwidth, true);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    break;

	  case 'N': // Case equals (!==)
	    gate = new NetCaseCmp(scope, scope->local_symbol(), dwidth, false);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    break;

	  case 'e': // ==

	      /* Handle the special case of single bit compare with a
		 single XNOR gate. This is easy and direct. */
	    if (dwidth == 1) {
		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XNOR, 1);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: Elaborate net == gate."
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
	    if (dwidth == 1) {
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
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);
      if (rsig == 0) return 0;


	// Check the l-value width. If it is unspecified, then use the
	// largest operand width as the l-value width. Restrict the
	// result width to the width of the largest operand, because
	// there is no value is excess divider.

      unsigned rwidth = lwidth;

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
	    cerr << get_line() << ": error: Arguments of divide "
		 << "have different data types." << endl;
	    cerr << get_line() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
      }

      ivl_variable_type_t data_type = lsig->data_type();

	// Create a device with the calculated dimensions.
      NetDivide*div = new NetDivide(scope, scope->local_symbol(), rwidth,
				    lsig->vector_width(),
				    rsig->vector_width());
      des->add_node(div);

      div->set_signed(lsig->get_signed() && rsig->get_signed());

	// Connect the left and right inputs of the divider to the
	// nets that are the left and right expressions.

      connect(div->pin_DataA(), lsig->pin(0));
      connect(div->pin_DataB(), rsig->pin(0));


	// Make an output signal that is the width of the l-value.
	// Due to above calculation of rwidth, we know that the result
	// will be no more then the l-value, so it is safe to connect
	// all the result pins to the osig.

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);
      osig->data_type(data_type);
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
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (rsig == 0) return 0;

	/* The arguments of a modulus must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_line() << ": error: Arguments of modulus "
		 << "have different data types." << endl;
	    cerr << get_line() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
      }

      ivl_variable_type_t data_type = lsig->data_type();

	/* rwidth is result width. */
      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	    rwidth = lsig->vector_width();
	    if (rsig->vector_width() > rwidth)
		  rwidth = rsig->vector_width();

	    lwidth = rwidth;
      }

      NetModulo*mod = new NetModulo(scope, scope->local_symbol(), rwidth,
				    lsig->vector_width(),
				    rsig->vector_width());
      mod->set_line(*this);
      des->add_node(mod);

      connect(mod->pin_DataA(), lsig->pin(0));
      connect(mod->pin_DataB(), rsig->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, rwidth);
      osig->set_line(*this);
      osig->data_type(data_type);
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
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
      NetNet*rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (lsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
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
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);

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
	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, lwidth);
	    for (unsigned idx = 0 ;  idx < lwidth ;  idx += 1)
		  connect(odev->pin(idx), osig->pin(idx));

	    des->add_node(odev);
	    osig->local_flag(true);
	    return osig;
      }

      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, scope, lwidth, 0, 0, 0);
      if (rsig == 0) return 0;

	// The mult is signed if both its operands are signed.
      bool arith_is_signed = lsig->get_signed() && rsig->get_signed();

	/* The arguments of a divide must have the same type. */
      if (lsig->data_type() != rsig->data_type()) {
	    cerr << get_line() << ": error: Arguments of multiply "
		 << "have different data types." << endl;
	    cerr << get_line() << ":      : Left argument is "
		 << lsig->data_type() << ", right argument is "
		 << rsig->data_type() << "." << endl;
	    des->errors += 1;
      }

      ivl_variable_type_t data_type = lsig->data_type();

      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	    rwidth = lsig->vector_width() + rsig->vector_width();
	    lwidth = rwidth;
      }

      if (arith_is_signed) {
	    lsig = pad_to_width_signed(des, lsig, rwidth);
	    rsig = pad_to_width_signed(des, rsig, rwidth);
      }

      NetMult*mult = new NetMult(scope, scope->local_symbol(), rwidth,
				 lsig->vector_width(),
				 rsig->vector_width());
      mult->set_line(*this);
      des->add_node(mult);

      mult->set_signed( arith_is_signed );

      connect(mult->pin_DataA(), lsig->pin(0));
      connect(mult->pin_DataB(), rsig->pin(0));

	// Make a signal to carry the output from the multiply.
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, rwidth);
      osig->data_type(data_type);
      osig->local_flag(true);
      connect(mult->pin_Result(), osig->pin(0));

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

      if (lsig->vector_width() > lwidth)
	    lwidth = lsig->vector_width();

      bool right_flag  =  op_ == 'r' || op_ == 'R';
      bool signed_flag =  op_ == 'R';
      ivl_variable_type_t data_type = lsig->data_type();

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

	      /* Another very special case: constant shift the entire
		 value away. The result is a const. */
	    if (dist > lwidth) {
		  assert(0);
	    }

	      /* The construction that I'm making will ultimately
		 connect its output to the osig here. This will be the
		 result that I return from this function. */
	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, lwidth);
	    osig->data_type( data_type );
	    osig->local_flag(true);


	      /* Make the constant zero's that I'm going to pad to the
		 top or bottom of the left expression. Attach a signal
		 to its output so that I don't have to worry about it
		 later. If the left expression is less then the
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
	    zero->data_type( data_type );
	    zero->local_flag(true);
	    zero->set_line(*this);

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
		  tmp->data_type( data_type );
		  connect(sign_bit->pin(0), tmp->pin(0));
		  connect(sign_bit->pin(0), sign_pad->pin(1));

		  connect(zero->pin(0), sign_pad->pin(0));

	    } else {
		  NetConst*zero_c = new NetConst(scope, scope->local_symbol(),
					      verinum(verinum::V0, pad_width));
		  des->add_node(zero_c);
		  connect(zero->pin(0), zero_c->pin(0));
	    }

	      /* Make a concatenation operator that will join the
		 part-selected right expression at the pad values. */
	    NetConcat*cc = new NetConcat(scope, scope->local_symbol(),
					 lwidth, 2);
	    cc->set_line(*this);
	    des->add_node(cc);
	    connect(cc->pin(0), osig->pin(0));

	      /* Make the part select of the left expression and
		 connect it to the lsb or msb of the concatenation,
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

	    des->add_node(part);

	      /* Attach a signal to the part select output (NetConcat
		 input) */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, part_width);
	    tmp->data_type( data_type );
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

	// Make the shift device itself, and the output
	// NetNet. Connect the Result output pins to the osig signal
      NetCLShift*gate = new NetCLShift(scope, scope->local_symbol(),
				       lwidth, rsig->vector_width(),
				       right_flag, signed_flag);
      des->add_node(gate);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, lwidth);
      osig->data_type( data_type );
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
	    cerr << get_line() << ": debug: "
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

	/* Handle the special case that the function call is to
	   $signed. This takes a single expression argument, and
	   forces it to be a signed result. Otherwise, it is as if the
	   $signed did not exist. */
      if (strcmp(path_.peek_name(0), "$signed") == 0) {
	    if ((parms_.count() != 1) || (parms_[0] == 0)) {
		  cerr << get_line() << ": error: The $signed() function "
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
      if (strcmp(path_.peek_name(0), "$unsigned") == 0) {
	    if ((parms_.count() != 1) || (parms_[0] == 0)) {
		  cerr << get_line() << ": error: The $unsigned() function "
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

	/* Look up the function definition. */
      NetFuncDef*def = des->find_function(scope, path_);
      if (def == 0) {
	    cerr << get_line() << ": error: No function " << path_ <<
		  " in this context (" << scope->name() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(def);

      NetScope*dscope = def->scope();
      assert(dscope);

	/* This must be a ufuction that returns a signal. */
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
		  cerr << get_line() << ": error: Unable to elaborate "
		       << "port " << idx << " of call to " << path_ <<
			"." << endl;
		  errors += 1;
		  continue;
	    }

	    func_pins += tmp->pin_count();
	    eparms[idx] = tmp;
      }

      if (errors > 0)
	    return 0;


      NetUserFunc*net = new NetUserFunc(scope,
					scope->local_symbol(),
					dscope);
      des->add_node(net);

	/* Create an output signal and connect it to the output pins
	   of the function net. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE,
			       def->return_sig()->vector_width());
      osig->local_flag(true);

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
	    NetExpr*etmp = elab_and_eval(des, scope, repeat_);
	    assert(etmp);
	    NetEConst*erep = dynamic_cast<NetEConst*>(etmp);

	    if (erep == 0) {
		  cerr << get_line() << ": internal error: Unable to "
		       << "evaluate constant repeat expression." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = erep->value().as_ulong();
	    delete etmp;

	    if (repeat == 0) {
		  cerr << get_line() << ": error: Concatenation epeat "
			"may not be 0."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }
      }

      if (debug_elaborate) {
	    cerr << get_line() <<": debug: PEConcat concat repeat="
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
		  cerr << get_line() << ": error: Empty expressions "
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
			cerr << get_line() << ": error: Number "
			     << tmp->value() << " with indefinite size"
			     << " in concatenation." << endl;
			errors += 1;
		  }
	    }

	    nets[idx] = parms_[idx]->elaborate_net(des, scope, 0,
						   rise,fall,decay);
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
	    des->errors += 1;
	    return 0;
      }

      if (debug_elaborate) {
	    cerr << get_line() <<": debug: PEConcat concat collected "
		 << "width=" << vector_width << ", repeat=" << repeat
		 << " of " << nets.count() << " expressions." << endl;
      }

      NetConcat*dev = new NetConcat(scope, scope->local_symbol(),
				    vector_width*repeat,
				    nets.count()*repeat);
      dev->set_line(*this);
      des->add_node(dev);

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from least significant to most significant,
	   which is opposite from how they are given in the list.

	   Allow for a repeat count other then 1 by repeating the
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
	/* Elaborate the selector. */
      NetNet*sel;

      if (sig->msb() < sig->lsb()) {
	    NetExpr*sel_expr = msb_->elaborate_expr(des, scope);
	    sel_expr = make_sub_expr(sig->lsb(), sel_expr);
	    if (NetExpr*tmp = sel_expr->eval_tree()) {
		  delete sel_expr;
		  sel_expr = tmp;
	    }

	    sel = sel_expr->synthesize(des);

      } else if (sig->lsb() != 0) {
	    NetExpr*sel_expr = msb_->elaborate_expr(des, scope);
	    sel_expr = make_add_expr(sel_expr, - sig->lsb());
	    if (NetExpr*tmp = sel_expr->eval_tree()) {
		  delete sel_expr;
		  sel_expr = tmp;
	    }

	    sel = sel_expr->synthesize(des);

      } else {
	    sel = msb_->elaborate_net(des, scope, 0, 0, 0, 0);
      }

      if (debug_elaborate) {
	    cerr << get_line() << ": debug: Create NetPartSelect "
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
      assert(scope);

      NetNet*       sig = 0;
      NetMemory*    mem = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, mem, par, eve);

	/* If the identifier is a memory instead of a signal,
	   then handle it elsewhere. Create a RAM. */
      if (mem != 0) {
	    return elaborate_net_ram_(des, scope, mem, lwidth,
				      rise, fall, decay);
      }

	/* If this is a parameter name, then create a constant node
	   that connects to a signal with the correct name. */
      if (par != 0) {

	    const NetEConst*pc = dynamic_cast<const NetEConst*>(par);
	    assert(pc);
	    verinum pvalue = pc->value();

	      /* If the desired lwidth is more then the width of the
		 constant value, extend the value to fit the desired
		 output. */
	    if (lwidth > pvalue.len()) {
		  verinum tmp (0UL, lwidth);
		  for (unsigned idx = 0 ;  idx < pvalue.len() ;  idx += 1)
			tmp.set(idx, pvalue.get(idx));

		  pvalue = tmp;
	    }

	    sig = new NetNet(scope, lex_strings.make(path_.peek_name(0)),
			     NetNet::IMPLICIT, pvalue.len());
	    sig->set_line(*this);
	    sig->data_type(IVL_VT_LOGIC);
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
      if (sig == 0 && path_.component_count() != 1) {
	    cerr << get_line() << ": error: The hierarchical name "
		 << path_ << " is undefined in "
		 << scope->name() << "." << endl;

	    hname_t tmp_path = path_;
	    delete[] tmp_path.remove_tail_name();

	    NetScope*tmp_scope = des->find_scope(scope, tmp_path);
	    if (tmp_scope == 0) {
		  cerr << get_line() << ":      : I can't even find "
		       << "the scope " << tmp_path << "." << endl;
	    }

	    des->errors += 1;
	    return 0;
      }

	/* Fallback, this may be an implicitly declared net. */
      if (sig == 0) {
	    NetNet::Type nettype = scope->default_nettype();
	    sig = new NetNet(scope, lex_strings.make(path_.peek_name(0)),
			     nettype, 1);
	    sig->data_type(IVL_VT_LOGIC);

	    if (error_implicit || (nettype == NetNet::NONE)) {
		  cerr << get_line() << ": error: "
		       << scope->name() << "." << path_.peek_name(0)
		       << " not defined in this scope." << endl;
		  des->errors += 1;

	    } else if (warn_implicit) {
		  cerr << get_line() << ": warning: implicit "
			"definition of wire " << scope->name()
		       << "." << path_.peek_name(0) << "." << endl;
	    }
      }

      assert(sig);

	/* Catch the case of a non-constant bit select. That should be
	   handled elsewhere. */
      if (msb_ && !lsb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  return elaborate_net_bitmux_(des, scope, sig, rise,
					       fall, decay, drive0, drive1);
	    }

	    delete mval;
      }

      unsigned midx, lidx;
      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;

      unsigned part_count = midx-lidx+1;

      if (part_count != sig->vector_width()) {
	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: Elaborate part select "
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


      return sig;
}

/*
 * When I run into an identifier in an expression that refers to a
 * memory, create a RAM port object.
 */
NetNet* PEIdent::elaborate_net_ram_(Design*des, NetScope*scope,
				    NetMemory*mem, unsigned lwidth,
				    const NetExpr* rise,
				    const NetExpr* fall,
				    const NetExpr* decay) const
{
      assert(scope);

      if (msb_ == 0) {
	    cerr << get_line() << ": error: memory reference without"
		  " the required index expression." << endl;
	    des->errors += 1;
	    return 0;
      }

	/* Even if this expression must be fully self determined, the
	   index expression does not, so make sure this flag is off
	   while elaborating the address expression. */
      const bool must_be_self_determined_save = must_be_self_determined_flag;
      must_be_self_determined_flag = false;

      NetExpr*adr_expr = elab_and_eval(des, scope, msb_);

	/* If an offset is needed, subtract it from the address to get
	   an expression for the canonical address. */
      if (mem->index_to_address(0) != 0) {
	    adr_expr = make_add_expr(adr_expr, mem->index_to_address(0));
	    if (NetExpr*tmp = adr_expr->eval_tree()) {
		  delete adr_expr;
		  adr_expr = tmp;
	    }
      }
      NetNet*adr = adr_expr->synthesize(des);
      delete adr_expr;

      must_be_self_determined_flag = must_be_self_determined_save;

      if (adr == 0)
	    return 0;

      NetRamDq*ram = new NetRamDq(scope, scope->local_symbol(),
				  mem, adr->vector_width());
      des->add_node(ram);

      connect(ram->pin_Address(), adr->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, ram->width());
      osig->data_type(IVL_VT_LOGIC);
      osig->local_flag(true);

      connect(ram->pin_Q(), osig->pin(0));

      return osig;
}

/*
 * The concatenation is also OK an an l-value. This method elaborates
 * it as a structural l-value. The return values is the *input* net of
 * the l-value, which may feed via part selects to the final
 * destination. The caller can connect gate outputs to this signal to
 * make the l-value connections.
 */
NetNet* PEConcat::elaborate_lnet(Design*des, NetScope*scope,
				 bool implicit_net_ok) const
{
      assert(scope);

      svector<NetNet*>nets (parms_.count());
      unsigned width = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_line() << ": sorry: I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: Elaborate subexpression "
		       << idx << " of " << nets.count() << " l-values: "
		       << *parms_[idx] << endl;
	    }

	    if (parms_[idx] == 0) {
		  cerr << get_line() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  errors += 1;
		  continue;
	    }

	    nets[idx] = parms_[idx]->elaborate_lnet(des, scope,
						    implicit_net_ok);
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
	    des->errors += 1;
	    return 0;
      }

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from most significant to least significant,
	   which is the order they are given in the concat list. */

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);


      if (debug_elaborate) {
	    cerr << get_line() << ": debug: Generating part selects "
		 << "to connect input l-value to subexpressions."
		 << endl;
      }

      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
	    unsigned wid = nets[idx]->vector_width();
	    unsigned off = width - wid;
	    NetPartSelect*ps = new NetPartSelect(osig, off, wid,
						 NetPartSelect::VP);
	    des->add_node(ps);

	    connect(ps->pin(1), osig->pin(0));
	    connect(ps->pin(0), nets[idx]->pin(0));

	    assert(wid <= width);
	    width -= wid;
      }
      assert(width == 0);

      osig->local_flag(true);
      return osig;
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
	    cerr << get_line() << ": debug: Elaborate real literal node, "
		 << "value=" << value() << "." << endl;
      }

      NetLiteral*obj = new NetLiteral(scope, scope->local_symbol(), value());
      obj->set_line(*this);
      des->add_node(obj);

      NetNet*net = new NetNet(scope, scope->local_symbol(), NetNet::WIRE, 1);
      net->data_type(IVL_VT_REAL);
      net->local_flag(true);

      connect(net->pin(0), obj->pin(0));
      return net;
}

/*
 * A private method to create an implicit net.
 */
NetNet* PEIdent::make_implicit_net_(Design*des, NetScope*scope) const
{
      NetNet::Type nettype = scope->default_nettype();
      NetNet*sig = 0;

      if (!error_implicit && nettype!=NetNet::NONE) {
	    sig = new NetNet(scope, lex_strings.make(path_.peek_name(0)),
			     NetNet::IMPLICIT, 1);
	      /* Implicit nets are always scalar logic. */
	    sig->data_type(IVL_VT_LOGIC);

	    if (warn_implicit) {
		  cerr << get_line() << ": warning: implicit "
			"definition of wire logic " << scope->name()
		       << "." << path_.peek_name(0) << "." << endl;
	    }

      } else {
	    cerr << get_line() << ": error: Net " << path_
		 << " is not defined in this context." << endl;
	    des->errors += 1;
	    return 0;
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
				unsigned&midx, unsigned&lidx) const
{
      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    assert(mval);
	    verinum*lval = lsb_->eval_const(des, scope);
	    assert(lval);

	    midx = sig->sb_to_idx(mval->as_long());
	    lidx = sig->sb_to_idx(lval->as_long());

	      /* Detect reversed indices of a part select. */
	    if (lidx > midx) {
		  cerr << get_line() << ": error: Part select "
		       << sig->name() << "[" << mval->as_long() << ":"
		       << lval->as_long() << "] indices reversed." << endl;
		  cerr << get_line() << ":      : Did you mean "
		       << sig->name() << "[" << lval->as_long() << ":"
		       << mval->as_long() << "]?" << endl;
		  unsigned tmp = midx;
		  midx = lidx;
		  lidx = tmp;
		  des->errors += 1;
	    }

	      /* Detect a part select out of range. */
	    if (midx >= sig->vector_width()) {
		  cerr << get_line() << ": error: Part select "
		       << sig->name() << "[" << mval->as_long() << ":"
		       << lval->as_long() << "] out of range." << endl;
		  midx = sig->vector_width() - 1;
		  lidx = 0;
		  des->errors += 1;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  cerr << get_line() << ": index of " << path_ <<
			" needs to be constant in this context." <<
			endl;
		  des->errors += 1;
		  return false;
	    }
	    assert(mval);

	    midx = sig->sb_to_idx(mval->as_long());
	    if (midx >= sig->vector_width()) {
		  cerr << get_line() << ": error: Index " << sig->name()
		       << "[" << mval->as_long() << "] out of range."
		       << endl;
		  des->errors += 1;
		  midx = 0;
	    }
	    lidx = midx;

      } else {
	    assert(msb_ == 0 && lsb_ == 0);
	    midx = sig->vector_width() - 1;
	    lidx = 0;
      }

      return true;
}

/*
 * This is the common code for l-value nets and bi-directional
 * nets. There is very little that is different between the two cases,
 * so most of the work for both is done here.
 */
NetNet* PEIdent::elaborate_lnet_common_(Design*des, NetScope*scope,
					bool implicit_net_ok,
					bool bidirectional_flag) const
{
      assert(scope);

      NetNet*       sig = 0;
      NetMemory*    mem = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, mem, par, eve);

      if (mem != 0) {
	    cerr << get_line() << ": error: memories (" << path_
		 << ") cannot be l-values in continuous "
		 << "assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (eve != 0) {
	    cerr << get_line() << ": error: named events (" << path_
		 << ") cannot be l-values in continuous "
		 << "assignments." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (sig == 0) {

	    if (implicit_net_ok) {

		  sig = make_implicit_net_(des, scope);
		  if (sig == 0)
			return 0;

	    } else {
		  cerr << get_line() << ": error: Net " << path_
		       << " is not defined in this context." << endl;
		  des->errors += 1;
		  return 0;
	    }
      }

      assert(sig);

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_line() << ": error: reg " << sig->name()
		 << "; cannot be driven by primitives"
		 << " or continuous assignment." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (sig->port_type() == NetNet::PINPUT) {
	    cerr << get_line() << ": warning: L-value ``"
		 << sig->name() << "'' is also an input port." << endl;
	    cerr << sig->get_line() << ": warning: input "
		 << sig->name() << "; is coerced to inout." << endl;
	    sig->port_type(NetNet::PINOUT);
      }

      unsigned midx, lidx;
      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;

      unsigned subnet_wid = midx-lidx+1;

	/* If the desired l-value vector is narrower then the
	   signal itself, then use a NetPartSelect node to
	   arrange for connection to the desired bits. All this
	   can be skipped if the desired with matches the
	   original vector. */

      if (subnet_wid != sig->vector_width()) {
	      /* If we are processing a tran or inout, then the
		 partselect is bi-directional. Otherwise, it is a
		 Part-to-Vector select. */
	    NetPartSelect::dir_t part_dir;
	    if (bidirectional_flag)
		  part_dir = NetPartSelect::BI;
	    else
		  part_dir = NetPartSelect::PV;

	    if (debug_elaborate)
		  cerr << get_line() << ": debug: "
		       << "Elaborate lnet part select "
		       << sig->name()
		       << "[base=" << lidx
		       << " wid=" << subnet_wid <<"]"
		       << endl;

	    NetNet*subsig = new NetNet(sig->scope(),
				       sig->scope()->local_symbol(),
				       NetNet::WIRE, subnet_wid);
	    subsig->data_type( sig->data_type() );

	    NetPartSelect*sub = new NetPartSelect(sig, lidx, subnet_wid,
						  part_dir);
	    des->add_node(sub);
	    connect(sub->pin(0), subsig->pin(0));

	    sig = subsig;
      }

      return sig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, NetScope*scope,
					bool implicit_net_ok) const
{
      return elaborate_lnet_common_(des, scope, implicit_net_ok, false);
}

NetNet* PEIdent::elaborate_bi_net(Design*des, NetScope*scope) const
{
      return elaborate_lnet_common_(des, scope, true, true);
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
	    cerr << get_line() << ": error: no wire/reg " << path_
		 << " in module " << scope->name() << "." << endl;
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
	    cerr << get_line() << ": error: signal " << path_ << " in"
		 << " module " << scope->name() << " is not a port." << endl;
	    cerr << get_line() << ":      : Are you missing an input/"
		 << "output/inout declaration?" << endl;
	    des->errors += 1;
	    return 0;

	      /* This should not happen. A PWire can only become
		 PIMPLICIT if this is a udp reg port, and the make_udp
		 function should turn it into an output.... I think. */

	  case NetNet::PIMPLICIT:
	    cerr << get_line() << ": internal error: signal " << path_
		 << " in module " << scope->name() << " is left as "
		 << "port type PIMPLICIT." << endl;
	    des->errors += 1;
	    return 0;
      }

      unsigned midx;
      unsigned lidx;

	/* Evaluate the part/bit select expressions, to get the part
	   select of the signal that attaches to the port. Also handle
	   range and direction checking here. */

      if (! eval_part_select_(des, scope, sig, midx, lidx))
	    return 0;


      unsigned swid = midx - lidx + 1;

      if (swid < sig->vector_width()) {
	    cerr << get_line() << ": XXXX: Forgot to implement part select"
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

	/* If we are constrained by a l-value size, then just make a
	   number constant with the correct size and set as many bits
	   in that constant as make sense. Pad excess with
	   zeros. Also, assume that numbers are meant to be logic
	   type. */

      if (lwidth > 0) {
	    NetNet*net = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, lwidth);
	    net->data_type(IVL_VT_LOGIC);
	    net->local_flag(true);
	    net->set_signed(value_->has_sign());

	      /* when expanding a constant to fit into the net, extend
		 the Vx or Vz values if they are in the sign position,
		 otherwise extend the number with 0 bits. */
	    verinum::V top_v = verinum::V0;
	    switch (value_->get(value_->len()-1)) {
		case verinum::Vx:
		  top_v = verinum::Vx;
		  break;
		case verinum::Vz:
		  top_v = verinum::Vz;
		  break;
		default:   /* V0 and V1, do nothing */
		  break;
	    }

	    verinum num(top_v, net->vector_width());
	    unsigned idx;
	    for (idx = 0 ;  idx < num.len() && idx < value_->len(); idx += 1)
		  num.set(idx, value_->get(idx));

	    NetConst*tmp = new NetConst(scope, scope->local_symbol(), num);
	    tmp->pin(0).drive0(drive0);
	    tmp->pin(0).drive1(drive1);
	    connect(net->pin(0), tmp->pin(0));

	    des->add_node(tmp);
	    return net;
      }

	/* If the number has a length, then use that to size the
	   number. Generate a constant object of exactly the user
	   specified size. */
      if (value_->has_len()) {
	    NetNet*net = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, value_->len());
	    net->data_type(IVL_VT_LOGIC);
	    net->local_flag(true);
	    net->set_signed(value_->has_sign());
	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					*value_);
	    connect(net->pin(0), tmp->pin(0));

	    des->add_node(tmp);
	    return net;
      }

	/* None of the above tight constraints are present, so make a
	   plausible choice for the width. Try to reduce the width as
	   much as possible by eliminating high zeros of unsigned
	   numbers. */

      if (must_be_self_determined_flag) {
	    cerr << get_line() << ": error: No idea how wide to make "
		 << "the unsized constant " << *value_ << "." << endl;
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

      NetNet*net = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, width);
      net->data_type(IVL_VT_LOGIC);
      net->local_flag(true);
      NetConst*tmp = new NetConst(scope, scope->local_symbol(), num);
      connect(net->pin(0), tmp->pin(0));

      des->add_node(tmp);
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
      net->local_flag(true);

	/* Make a verinum that is filled with the 0 pad. */
      verinum num(verinum::V0, net->vector_width());

      unsigned idx;
      for (idx = 0 ;  idx < num.len() && idx < strbits; idx += 1) {
	    char byte = text_[strbits/8 - 1 - idx/8];
	    char mask = 1<<(idx%8);
	    num.set(idx, (byte & mask)? verinum::V1 : verinum::V0);
      }

      NetConst*tmp = new NetConst(scope, scope->local_symbol(), num);
      tmp->set_line(*this);
      des->add_node(tmp);

      connect(net->pin(0), tmp->pin(0));

      return net;
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
      NetNet* expr_sig = expr_->elaborate_net(des, scope, 0, 0, 0, 0);
      NetNet* tru_sig = tru_->elaborate_net(des, scope, width, 0, 0, 0);
      NetNet* fal_sig = fal_->elaborate_net(des, scope, width, 0, 0, 0);
      if (expr_sig == 0 || tru_sig == 0 || fal_sig == 0) {
	    des->errors += 1;
	    return 0;
      }

	/* The type of the true and false expressions must
	   match. These become the type of the resulting
	   expression. */

      ivl_variable_type_t expr_type = tru_sig->data_type();

      if (tru_sig->data_type() != fal_sig->data_type()) {
	    cerr << get_line() << ": error: True and False clauses of"
		 << " ternary expression have differnt types." << endl;
	    cerr << get_line() << ":      : True clause is "
		 << tru_sig->data_type() << ", false clause is "
		 << fal_sig->data_type() << "." << endl;

	    des->errors += 1;
	    expr_type = IVL_VT_NO_TYPE;

      } else if (expr_type == IVL_VT_NO_TYPE) {
	    cerr << get_line() << ": internal error: True and false "
		 << "clauses of ternary both have NO TYPE." << endl;
	    des->errors += 1;
      }

	/* The natural width of the expression is the width of the
	   largest condition. Normally they should be the same size,
	   but if we do not get a size from the context, or the
	   expressions resist, we need to cope. */
      unsigned iwidth = tru_sig->vector_width();
      if (fal_sig->vector_width() > iwidth)
	    iwidth = fal_sig->vector_width();


	/* If the width is not passed from the context, then take the
	   widest result as our width. */
      if (width == 0)
	    width = iwidth;

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
	    tmp->local_flag(true);
	    connect(log->pin(0), tmp->pin(0));

	    expr_sig = tmp;
      }

      assert(expr_sig->vector_width() == 1);

	/* This is the width of the LPM_MUX device that I'm about to
	   create. It may be smaller then the desired output, but I'll
	   handle padding below.

	   Create a NetNet object wide enough to hold the
	   result. Also, pad the result values (if necessary) so that
	   the mux inputs can be fully connected. */

      unsigned dwidth = (iwidth > width)? width : iwidth;

      NetNet*sig = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, width);
      sig->data_type(expr_type);
      sig->local_flag(true);

      if (fal_sig->vector_width() < dwidth)
	    fal_sig = pad_to_width(des, fal_sig, dwidth);

      if (tru_sig->vector_width() < dwidth)
	    tru_sig = pad_to_width(des, tru_sig, dwidth);


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
      assert(dwidth == width);

      des->add_node(mux);

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

      NetNet* sig = 0;
      NetLogic*gate;


	// Handle the special case of a 2's complement of a constant
	// value. This can be reduced to a no-op on a precalculated
	// result.
      if (op_ == '-') do {
	      // TODO: Should replace this with a call to
	      // elab_and_eval. Possibly blend this with the rest of
	      // the elaboration as well.
	    verinum*val = expr_->eval_const(des, scope);
	    if (val == 0)
		  break;

	    if (width == 0)
		  width = val->len();

	    assert(width > 0);
	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::WIRE, width);
	    sig->data_type(IVL_VT_LOGIC);
	    sig->local_flag(true);

	      /* Take the 2s complement by taking the 1s complement
		 and adding 1. */
	    verinum tmp (v_not(*val), width);
	    verinum one (1UL, width);
	    tmp = verinum(tmp + one, width);
	    tmp.has_sign(val->has_sign());

	    NetConst*con = new NetConst(scope, scope->local_symbol(), tmp);
	    connect(sig->pin(0), con->pin(0));

	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: Replace expression "
		       << *this << " with constant " << tmp << "."<<endl;
	    }

	    delete val;
	    des->add_node(con);
	    return sig;

      } while (0);

      NetNet* sub_sig = expr_->elaborate_net(des, scope, owidth, 0, 0, 0);
      if (sub_sig == 0) {
	    des->errors += 1;
	    return 0;
      }
      assert(sub_sig);

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
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  break;

		default:
		  NetAddSub*sub = new NetAddSub(scope, scope->local_symbol(),
						sig->vector_width());
		  sub->attribute(perm_string::literal("LPM_Direction"),
				 verinum("SUB"));

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
		  tmp_sig->local_flag(true);

		  connect(tmp_sig->pin(0), sub->pin_DataA());
		  connect(tmp_sig->pin(0), tmp_con->pin(0));
		  break;
	    }
	    break;

	  default:
	    cerr << "internal error: Unhandled UNARY '" << op_ << "'" << endl;
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

/*
 * $Log: elab_net.cc,v $
 * Revision 1.177  2006/01/02 05:33:19  steve
 *  Node delays can be more general expressions in structural contexts.
 *
 * Revision 1.176  2005/10/11 16:15:52  steve
 *  Logical or/and return VT_LOGIC type.
 *
 * Revision 1.175  2005/09/19 15:21:09  steve
 *  Fix data type of parameters to logic.
 *
 * Revision 1.174  2005/09/15 23:04:09  steve
 *  Make sure div, mod and mult nodes have line number info.
 *
 * Revision 1.173  2005/09/14 15:15:44  steve
 *  fit type elaboration of logical not.
 *
 * Revision 1.172  2005/09/01 04:10:47  steve
 *  Check operand types for compatibility.
 *
 * Revision 1.171  2005/08/31 05:07:31  steve
 *  Handle memory references is continuous assignments.
 *
 * Revision 1.170  2005/08/06 17:58:16  steve
 *  Implement bi-directional part selects.
 *
 * Revision 1.169  2005/07/15 04:13:25  steve
 *  Match data type of PV select input/output.
 *
 * Revision 1.168  2005/07/15 00:42:02  steve
 *  Get output type correct for binary mux (ternary) expression.
 *
 * Revision 1.167  2005/07/11 16:56:50  steve
 *  Remove NetVariable and ivl_variable_t structures.
 *
 * Revision 1.166  2005/07/07 16:22:49  steve
 *  Generalize signals to carry types.
 *
 * Revision 1.165  2005/05/24 01:44:27  steve
 *  Do sign extension of structuran nets.
 *
 * Revision 1.164  2005/05/19 03:51:38  steve
 *  Make sure comparison widths match.
 *
 * Revision 1.163  2005/05/10 05:10:40  steve
 *  Make sig-eq-constant optimization more effective.
 *
 * Revision 1.162  2005/05/08 23:44:08  steve
 *  Add support for variable part select.
 *
 * Revision 1.161  2005/05/06 00:25:13  steve
 *  Handle synthesis of concatenation expressions.
 *
 * Revision 1.160  2005/04/08 04:52:31  steve
 *  Make clear that memory addresses are cannonical.
 *
 * Revision 1.159  2005/04/06 05:29:08  steve
 *  Rework NetRamDq and IVL_LPM_RAM nodes.
 *
 * Revision 1.158  2005/03/19 06:59:53  steve
 *  Handle wide operands to logical AND.
 *
 * Revision 1.157  2005/03/19 06:23:49  steve
 *  Handle LPM shifts.
 *
 * Revision 1.156  2005/03/18 02:56:03  steve
 *  Add support for LPM_UFUNC user defined functions.
 *
 * Revision 1.155  2005/03/13 01:26:48  steve
 *  UPdate elabrate continuous assighn of string to net.
 *
 * Revision 1.154  2005/03/12 06:43:35  steve
 *  Update support for LPM_MOD.
 *
 * Revision 1.153  2005/03/09 05:52:03  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.152  2005/02/19 02:43:38  steve
 *  Support shifts and divide.
 *
 * Revision 1.151  2005/02/12 06:25:40  steve
 *  Restructure NetMux devices to pass vectors.
 *  Generate NetMux devices from ternary expressions,
 *  Reduce NetMux devices to bufif when appropriate.
 *
 * Revision 1.150  2005/02/03 04:56:20  steve
 *  laborate reduction gates into LPM_RED_ nodes.
 *
 * Revision 1.149  2005/01/30 05:20:38  steve
 *  Elaborate unary subtract and NOT in netlist
 *  contexts, and concatenation too.
 *
 * Revision 1.148  2005/01/29 18:46:18  steve
 *  Netlist boolean expressions generate gate vectors.
 *
 * Revision 1.147  2005/01/29 16:46:22  steve
 *  Elaborate parameter reference to desired width without concats.
 *
 * Revision 1.146  2005/01/29 00:37:06  steve
 *  Integrate pr1072 fix from v0_8-branch.
 *
 * Revision 1.145  2005/01/28 05:39:33  steve
 *  Simplified NetMult and IVL_LPM_MULT.
 *
 * Revision 1.144  2005/01/22 18:16:00  steve
 *  Remove obsolete NetSubnet class.
 *
 * Revision 1.143  2005/01/22 01:06:55  steve
 *  Change case compare from logic to an LPM node.
 *
 * Revision 1.142  2005/01/16 04:20:32  steve
 *  Implement LPM_COMPARE nodes as two-input vector functors.
 *
 * Revision 1.141  2005/01/13 00:23:10  steve
 *  Fix elaboration of == compared to constants.
 *
 * Revision 1.140  2005/01/09 20:16:00  steve
 *  Use PartSelect/PV and VP to handle part selects through ports.
 *
 * Revision 1.139  2004/12/11 02:31:25  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.138  2004/10/04 03:09:38  steve
 *  Fix excessive error message.
 *
 * Revision 1.137  2004/10/04 01:10:52  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.136  2004/10/04 00:25:46  steve
 *  Error message to match assertion.
 */

