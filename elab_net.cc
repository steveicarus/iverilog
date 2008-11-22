/*
 * Copyright (c) 1999-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_net.cc,v 1.138.2.3 2005/09/11 02:56:37 steve Exp $"
#endif

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

# include  <iostream>
# include  <cstring>

/*
 * This is a state flag that determines whether an elaborate_net must
 * report an error when it encounters an unsized number. Normally, it
 * is fine to make an unsized number as small as it can be, but there
 * are a few cases where the size must be fully self-determined. For
 * example, within a {...} (concatenation) operator.
 */
static bool must_be_self_determined_flag = false;

NetNet* PExpr::elaborate_net(Design*des, NetScope*scope, unsigned,
			     unsigned long,
			     unsigned long,
			     unsigned long,
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
				unsigned long rise,
				unsigned long fall,
				unsigned long decay,
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

      if (NetSubnet*tmp = dynamic_cast<NetSubnet*>(lsig))
	    delete tmp;
      if (NetSubnet*tmp = dynamic_cast<NetSubnet*>(rsig))
	    delete tmp;

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
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
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

      unsigned width = lsig->pin_count();
      if (rsig->pin_count() > lsig->pin_count())
	    width = rsig->pin_count();


	/* The owidth is the output width of the lpm_add_sub
	   device. If the desired width is greater then the width of
	   the operands, then widen the adder and let code below pad
	   the operands. If this is an adder, we can take advantage of
	   the carry bit. */
      unsigned owidth = width;
      switch (op_) {
	  case '+':
	    if (lwidth > owidth) {
		  owidth = lwidth;
		  width = lwidth-1;
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
      if (lsig->pin_count() < width)
	    lsig = pad_to_width(des, lsig, width);

      if (rsig->pin_count() < width)
	    rsig = pad_to_width(des, rsig, width);

	// Make the adder as wide as the widest operand
      osig = new NetNet(scope, scope->local_symbol(),
			NetNet::WIRE, owidth);
      osig->local_flag(true);
      NetAddSub*adder = new NetAddSub(scope, scope->local_symbol(), width);

	// Connect the adder to the various parts.
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(lsig->pin(idx), adder->pin_DataA(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(rsig->pin(idx), adder->pin_DataB(idx));
      for (unsigned idx = 0 ;  idx < width ; idx += 1)
	    connect(osig->pin(idx), adder->pin_Result(idx));
      if (owidth > width)
	    connect(osig->pin(width), adder->pin_Cout());

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
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
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

      if (lsig->pin_count() < rsig->pin_count())
	    lsig = pad_to_width(des, lsig, rsig->pin_count());
      if (rsig->pin_count() < lsig->pin_count())
	    rsig = pad_to_width(des, rsig, lsig->pin_count());

      if (lsig->pin_count() != rsig->pin_count()) {
	    cerr << get_line() << ": internal error: lsig pin count ("
		 << lsig->pin_count() << ") != rsig pin count ("
		 << rsig->pin_count() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(lsig->pin_count() == rsig->pin_count());

      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			       lsig->pin_count());
      osig->local_flag(true);

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

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
	  NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					       3, gtype);
	  connect(gate->pin(1), lsig->pin(idx));
	  connect(gate->pin(2), rsig->pin(idx));
	  connect(gate->pin(0), osig->pin(idx));
	  gate->rise_time(rise);
	  gate->fall_time(fall);
	  gate->decay_time(decay);
	  des->add_node(gate);
       }

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
				   unsigned long rise,
				   unsigned long fall,
				   unsigned long decay)
{
      if (op_code != 'e' && op_code != 'n')
	    return 0;

      verinum val = rexp->value();

	/* Abandon special case if there are x or z bits in the
	   constant. We can't get the right behavior out of
	   OR/NOR in this case. */
      if (! val.is_defined())
	    return 0;

      if (val.len() < lsig->pin_count())
	    val = verinum(val, lsig->pin_count());

	/* Look for the very special case that we know the compare
	   results a priori due to different high bits, that are
	   constant pad in the signal. */
      if (val.len() > lsig->pin_count()) {
	    unsigned idx = lsig->pin_count();
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
			return osig;
		  }

		  idx +=1;
	    }
      }

      unsigned zeros = 0;
      unsigned ones = 0;
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
	    if (val.get(idx) == verinum::V0)
		  zeros += 1;
	    if (val.get(idx) == verinum::V1)
		  ones += 1;
      }

	/* Now make reduction logic to test that all the 1 bits are 1,
	   and all the 0 bits are 0. The results will be ANDed
	   together later, if needed. NOTE that if the compare is !=,
	   and we know that we will not need an AND later, then fold
	   the final invert into the reduction gate to get the right
	   sense of the output. If we do need the AND later, then we
	   will put the invert on that instead. */
      NetLogic*zero_gate = 0;
      NetLogic*ones_gate = 0;
      if (zeros > 0)
	    zero_gate = new NetLogic(scope,
			    scope->local_symbol(), zeros + 1,
			    (op_code == 'n') ? NetLogic::OR : NetLogic::NOR);
      if (ones > 0)
	    ones_gate = new NetLogic(scope,
			    scope->local_symbol(), ones + 1,
			    (op_code == 'n') ? NetLogic::NAND : NetLogic::AND);

      unsigned zidx = 0;
      unsigned oidx = 0;
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
	    if (val.get(idx) == verinum::V0) {
		  zidx += 1;
		  connect(zero_gate->pin(zidx), lsig->pin(idx));
	    }
	    if (val.get(idx) == verinum::V1) {
		  oidx += 1;
		  connect(ones_gate->pin(oidx), lsig->pin(idx));
	    }
      }

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, 1);
      osig->local_flag(true);

      if (zero_gate && ones_gate) {
	    NetNet*and_sig = new NetNet(scope, scope->local_symbol(),
					NetNet::WIRE, 2);
	    and_sig->local_flag(true);
	    connect(and_sig->pin(0), zero_gate->pin(0));
	    connect(and_sig->pin(1), ones_gate->pin(0));
	    NetLogic*and_gate = new NetLogic(scope,
			        scope->local_symbol(), 3,
			        (op_code == 'n') ? NetLogic::OR : NetLogic::AND);
	    connect(and_gate->pin(0), osig->pin(0));
	    connect(and_gate->pin(1), and_sig->pin(0));
	    connect(and_gate->pin(2), and_sig->pin(1));

	    des->add_node(and_gate);
	    des->add_node(zero_gate);
	    des->add_node(ones_gate);

      } else if (zero_gate) {
	    connect(zero_gate->pin(0), osig->pin(0));
	    des->add_node(zero_gate);
      } else {
	    assert(ones_gate);
	    connect(ones_gate->pin(0), osig->pin(0));
	    des->add_node(ones_gate);
      }

      return osig;
}

/*
 * Elaborate the various binary comparison operators. The comparison
 * operators return a single bit result, no matter what, so the left
 * and right values can have their own size. The only restriction is
 * that they have the same size.
 */
NetNet* PEBinary::elaborate_net_cmp_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
	/* Elaborate the operands of the compare first as expressions
	   (so that the eval_tree method can reduce constant
	   expressions, including parameters) then turn those results
	   into synthesized nets. */
      NetExpr*lexp = left_->elaborate_expr(des, scope);
      if (lexp == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      if (NetExpr*tmp = lexp->eval_tree()) {
	    delete lexp;
	    lexp = tmp;
      }

      NetExpr*rexp = right_->elaborate_expr(des, scope);
      if (rexp == 0) {
	    cerr << get_line() << ": error: Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      if (NetExpr*tmp = rexp->eval_tree()) {
	    delete rexp;
	    rexp = tmp;
      }

      NetNet*lsig = 0;
      NetNet*rsig = 0;

	/* Handle the special case that the right or left
	   sub-expression is a constant value. The compare_eq_constant
	   function will return an elaborated result if it can make
	   use of the situation, or 0 if it cannot. */
      if (NetEConst*tmp = dynamic_cast<NetEConst*>(rexp)) {

	    lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
	    if (lsig == 0) {
		  cerr << get_line() << ": internal error: "
			"Cannot elaborate net for " << *lexp << endl;
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

	    rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
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
	    lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
	    assert(lsig);
	    delete lexp;
      }

      if (rsig == 0) {
	    rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
	    assert(rsig);
	    delete rexp;
      }

      unsigned dwidth = lsig->pin_count();
      if (rsig->pin_count() > dwidth) dwidth = rsig->pin_count();

	/* Operands of binary compare need to be padded to equal
	   size. Figure the pad bit needed to extend the narrowest
	   vector. */
      NetNet*padbit = 0;
      if (lsig->pin_count() != rsig->pin_count()) {
	    unsigned lwid = lsig->pin_count();
	    unsigned rwid = rsig->pin_count();

	    padbit = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
	    padbit->local_flag(true);

	    if (lsig->get_signed() && (lwid < rwid)) {

		  connect(padbit->pin(0), lsig->pin(lwid-1));

	    } else if (rsig->get_signed() && (rwid < lwid)) {

		  connect(padbit->pin(0), rsig->pin(rwid-1));

	    } else {
		  NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					      verinum::V0);
		  des->add_node(tmp);
		  connect(tmp->pin(0), padbit->pin(0));
	    }
      }

      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
      osig->local_flag(true);

      NetNode*gate;
	//NetNode*gate_t;

      switch (op_) {
	  case '<':
	  case '>':
	  case 'L':
	  case 'G': {
		NetCompare*cmp = new
		      NetCompare(scope, scope->local_symbol(), dwidth);
		for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
		      connect(cmp->pin_DataA(idx), lsig->pin(idx));
		for (unsigned idx = lsig->pin_count(); idx < dwidth ; idx += 1)
		      connect(cmp->pin_DataA(idx), padbit->pin(0));
		for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		      connect(cmp->pin_DataB(idx), rsig->pin(idx));
		for (unsigned idx = rsig->pin_count(); idx < dwidth ; idx += 1)
		      connect(cmp->pin_DataB(idx), padbit->pin(0));

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
	  case 'N': // Case equals (!==)
	      // The comparison generates gates to bitwise compare
	      // each pair, and AND all the comparison results.

	    gate = new NetLogic(scope, scope->local_symbol(),
				1+dwidth,
				(op_ == 'E')? NetLogic::AND : NetLogic::NAND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
		  NetCaseCmp*cmp = new NetCaseCmp(scope,
						  scope->local_symbol());

		  if (idx < lsig->pin_count())
			connect(cmp->pin(1), lsig->pin(idx));
		  else
			connect(cmp->pin(1), padbit->pin(0));

		  if (idx < rsig->pin_count())
			connect(cmp->pin(2), rsig->pin(idx));
		  else
			connect(cmp->pin(2), padbit->pin(0));

		  connect(cmp->pin(0), gate->pin(idx+1));
		  des->add_node(cmp);

		    // Attach a label to this intermediate wire
		  NetNet*tmp = new NetNet(scope, scope->local_symbol(),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(cmp->pin(0), tmp->pin(0));
	    }
	    break;


	  case 'e': // ==

	      /* Handle the special case of single bit compare with a
		 single XNOR gate. This is easy and direct. */
	    if (dwidth == 1) {
		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XNOR);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_symbol(),
					      dwidth);
	      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {

		    if (idx < lsig->pin_count())
			  connect(cmp->pin_DataA(idx), lsig->pin(idx));
		    else
			  connect(cmp->pin_DataA(idx), padbit->pin(0));

		    if (idx < rsig->pin_count())
			  connect(cmp->pin_DataB(idx), rsig->pin(idx));
		    else
			  connect(cmp->pin_DataB(idx), padbit->pin(0));

	      }
	      connect(cmp->pin_AEB(), osig->pin(0));
	      gate = cmp;
	    }
	    break;

	  case 'n': // !=

	      /* Handle the special case of single bit compare with a
		 single XOR gate. This is easy and direct. */
	    if (dwidth == 1) {
		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XOR);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_symbol(),
					      dwidth);
	      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {

		    if (idx < lsig->pin_count())
			  connect(cmp->pin_DataA(idx), lsig->pin(idx));
		    else
			  connect(cmp->pin_DataA(idx), padbit->pin(0));

		    if (idx < rsig->pin_count())
			  connect(cmp->pin_DataB(idx), rsig->pin(idx));
		    else
			  connect(cmp->pin_DataB(idx), padbit->pin(0));

	      }
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
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
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
	    rwidth = lsig->pin_count();
	    if (rsig->pin_count() > rwidth)
		  rwidth = rsig->pin_count();

	    lwidth = rwidth;
      }

      if ((rwidth > lsig->pin_count()) && (rwidth > rsig->pin_count())) {
	    rwidth = lsig->pin_count();
	    if (rsig->pin_count() > rwidth)
		  rwidth = rsig->pin_count();
      }

	// Create a device with the calculated dimensions.
      NetDivide*div = new NetDivide(scope, scope->local_symbol(), rwidth,
				    lsig->pin_count(),
				    rsig->pin_count());
      des->add_node(div);

      div->set_signed(lsig->get_signed() && rsig->get_signed());

	// Connect the left and right inputs of the divider to the
	// nets that are the left and right expressions.

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(div->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(div->pin_DataB(idx), rsig->pin(idx));


	// Make an output signal that is the width of the l-value.
	// Due to above calculation of rwidth, we know that the result
	// will be no more then the l-value, so it is safe to connect
	// all the result pins to the osig.

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);
      osig->set_signed(div->get_signed());

      for (unsigned idx = 0 ;  idx < rwidth ;  idx += 1)
	    connect(div->pin_Result(idx), osig->pin(idx));


	// If the lvalue is larger then the result, then pad the
	// output with constant 0. This can happen for example in
	// cases like this:
	//    wire [3;0] a, b;
	//    wire [7:0] r = a / b;

      if (rwidth < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					verinum::V0);
	    des->add_node(tmp);
	    for (unsigned idx = rwidth ;  idx < osig->pin_count() ;  idx += 1)
		  connect(osig->pin(idx), tmp->pin(0));
      }

      return osig;
}

/*
 * Elaborate a modulo gate.
 */
NetNet* PEBinary::elaborate_net_mod_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (rsig == 0) return 0;

      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	    rwidth = lsig->pin_count();
	    if (rsig->pin_count() > rwidth)
		  rwidth = rsig->pin_count();
      }
      NetModulo*mod = new NetModulo(scope, scope->local_symbol(), rwidth,
				    lsig->pin_count(),
				    rsig->pin_count());
      des->add_node(mod);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(mod->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(mod->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(mod->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					verinum::V0);
	    des->add_node(tmp);
	    for (unsigned idx = cnt ;  idx < osig->pin_count() ;  idx += 1)
		  connect(osig->pin(idx), tmp->pin(0));
      }

      return osig;
}

NetNet* PEBinary::elaborate_net_log_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
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
      NetLogic*gate_t;
      switch (op_) {
	  case 'a':
	    gate = new NetLogic(scope, scope->local_symbol(),
				3, NetLogic::AND);
	    break;
	  case 'o':
	    gate = new NetLogic(scope, scope->local_symbol(),
				3, NetLogic::OR);
	    break;
	  default:
	    assert(0);
      }
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);

	// The first OR gate returns 1 if the left value is true...
      if (lsig->pin_count() > 1) {
	    gate_t = new NetLogic(scope, scope->local_symbol(),
				  1+lsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), lsig->pin(idx));

	    connect(gate->pin(1), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->local_flag(true);
	    connect(gate->pin(1), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(1), lsig->pin(0));
      }

	// The second OR gate returns 1 if the right value is true...
      if (rsig->pin_count() > 1) {
	    gate_t = new NetLogic(scope, scope->local_symbol(),
				  1+rsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), rsig->pin(idx));
	    connect(gate->pin(2), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->local_flag(true);
	    connect(gate->pin(2), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(2), rsig->pin(0));
      }

	// The output is the AND/OR of the two logic values.
      NetNet*osig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
      osig->local_flag(true);
      connect(gate->pin(0), osig->pin(0));
      des->add_node(gate);
      return osig;
}

NetNet* PEBinary::elaborate_net_mul_(Design*des, NetScope*scope,
				       unsigned lwidth,
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay) const
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

      unsigned rwidth = lwidth;
      if (rwidth == 0) {
	    rwidth = lsig->pin_count() + rsig->pin_count();
      }

      NetMult*mult = new NetMult(scope, scope->local_symbol(), rwidth,
				 lsig->pin_count(),
				 rsig->pin_count());
      des->add_node(mult);

      mult->set_signed( lsig->get_signed() && rsig->get_signed() );

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(mult->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					verinum::V0);
	    des->add_node(tmp);
	    for (unsigned idx = cnt ;  idx < osig->pin_count() ;  idx += 1)
		  connect(osig->pin(idx), tmp->pin(0));
      }

      return osig;
}

NetNet* PEBinary::elaborate_net_shift_(Design*des, NetScope*scope,
				       unsigned lwidth,
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, lwidth, 0, 0, 0);
      if (lsig == 0) return 0;

      if (lsig->pin_count() > lwidth)
	    lwidth = lsig->pin_count();

      bool right_flag  =  op_ == 'r' || op_ == 'R';
      bool signed_flag =  op_ == 'R';

	/* Handle the special case of a constant shift amount. There
	   is no reason in this case to create a gate at all, just
	   connect the lsig to the osig with the bit positions
	   shifted. */
      if (verinum*rval = right_->eval_const(des, scope)) {
	    assert(rval->is_defined());
	    unsigned dist = rval->as_ulong();
	    if (dist > lwidth)
		  dist = lwidth;

	      /* Very special case, constant 0 shift. */
	    if (dist == 0) return lsig;

	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, lwidth);
	    osig->local_flag(true);

	    NetConst*zero = new NetConst(scope, scope->local_symbol(),
					 verinum::V0);
	    des->add_node(zero);

	    if (op_ == 'l') {
		    /* Left shift means put some zeros on the bottom
		       of the vector. */
		  unsigned idx;
		  for (idx = 0 ;  idx < dist ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));
		  for (    ; (idx<lwidth) && ((idx-dist) < lsig->pin_count())
			   ; idx += 1)
			connect(osig->pin(idx), lsig->pin(idx-dist));
		  for (    ;  idx < lwidth ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));

	    } else if (op_ == 'R') {
		    /* Signed right shift. */
		  unsigned idx;
		  unsigned keep = lsig->pin_count()-dist;
		  for (idx = 0 ;  idx < keep ;  idx += 1)
			connect(osig->pin(idx), lsig->pin(idx+dist));
		  for (idx = keep ;  idx < lwidth ;  idx += 1)
			connect(osig->pin(idx), lsig->pin(keep+dist-1));

	    } else {
		    /* Unsigned right shift. */
		  assert(op_ == 'r');
		  unsigned idx;
		  unsigned keep = lsig->pin_count()-dist;
		  for (idx = 0 ;  idx < keep ;  idx += 1)
			connect(osig->pin(idx), lsig->pin(idx+dist));
		  for (idx = keep ;  idx < lwidth ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));
	    }

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
				       lwidth, rsig->pin_count(),
				       right_flag, signed_flag);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, lwidth);
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < lwidth ;  idx += 1)
	    connect(osig->pin(idx), gate->pin_Result(idx));

	// Connect the lsig (the left expression) to the Data input,
	// and pad it if necessary with constant zeros.
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(lsig->pin(idx), gate->pin_Data(idx));

      if (lsig->pin_count() < lwidth) {
	    NetConst*zero = new NetConst(scope, scope->local_symbol(),
					 verinum::V0);
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->local_flag(true);
	    des->add_node(zero);
	    connect(zero->pin(0), tmp->pin(0));
	    for (unsigned idx = lsig->pin_count() ; idx < lwidth ;  idx += 1)
		  connect(zero->pin(0), gate->pin_Data(idx));
      }

	// Connect the rsig (the shift amount expression) to the
	// Distance input.
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
	    connect(rsig->pin(idx), gate->pin_Distance(idx));

      des->add_node(gate);

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
				      unsigned long rise,
				      unsigned long fall,
				      unsigned long decay,
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

      if (path_.peek_name(0)[0] == '$') {
	    cerr << get_line() << ": sorry: System functions " << path_ <<
		  " not supported in continuous assignments." << endl;
	    des->errors += 1;
	    return 0;
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
						    port_reg->pin_count(),
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
			       def->return_sig()->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(net->port_pin(0, idx), osig->pin(idx));

	/* Connect the parameter pins to the parameter expressions. */
      for (unsigned idx = 0 ; idx < eparms.count() ; idx += 1) {
	    const NetNet* port = def->port(idx);
	    NetNet*cur = eparms[idx];

	    NetNet*tmp = pad_to_width(des, cur, port->pin_count());

	    for (unsigned pin = 0 ;  pin < port->pin_count() ;  pin += 1)
		  connect(net->port_pin(idx+1, pin), tmp->pin(pin));
      }

      return osig;
}


/*
 * The concatenation operator, as a net, is a wide signal that is
 * connected to all the pins of the elaborated expression nets.
 */
NetNet* PEConcat::elaborate_net(Design*des, NetScope*scope,
				unsigned,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{
      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;
      unsigned repeat = 1;

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
		  cerr << get_line() << ": error: Concatenation repeat "
			"may not be 0."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (!erep->value().is_defined()) {
		  cerr << get_line() << ": error: Concatenation repeat "
		       << "may not be undefined (" << erep->value()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (erep->value().is_negative()) {
		  cerr << get_line() << ": error: Concatenation repeat "
		       << "may not be negative (" << erep->value().as_long()
		       << ")." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (erep->value().is_zero()) {
		  cerr << get_line() << ": error: Concatenation repeat "
		       << "may not be zero." << endl;
		  des->errors += 1;
		  return 0;
	    }
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
		  pins += nets[idx]->pin_count();
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

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from least significant to most significant,
	   which is opposite from how they are given in the list.

	   Allow for a repeat count other then 1 by repeating the
	   connect loop as many times as necessary. */

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, pins * repeat);

      pins = 0;
      for (unsigned rpt = 0 ;  rpt < repeat ;  rpt += 1)
	    for (unsigned idx = nets.count() ;  idx > 0 ;  idx -= 1) {
		  NetNet*cur = nets[idx-1];
		  for (unsigned pin = 0;  pin < cur->pin_count();  pin += 1) {
			connect(osig->pin(pins), cur->pin(pin));
			pins += 1;
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
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay,
				       Link::strength_t drive0,
				       Link::strength_t drive1) const
{
	/* Elaborate the selector. */
      NetNet*sel = msb_->elaborate_net(des, scope, 0, 0, 0, 0);

      unsigned sig_width = sig->pin_count();

	/* Detect the case of some bits not accessible by the given
	   select. Figure out how many bits can be selected by the
	   full range of the select, and limit the input of the mux to
	   that width. */
      { unsigned max_width_by_sel = 1 << sel->pin_count();
        if (sig_width > max_width_by_sel)
	      sig_width = max_width_by_sel;
      }

      NetMux*mux = new NetMux(scope, scope->local_symbol(), 1,
			      sig_width, sel->pin_count());
      mux->set_line(*this);

	/* Connect the signal bits to the mux. Account for the
	   direction of the numbering (lsb to msb vs. msb to lsb) by
	   swapping the connection order. */

      if (sig->msb() > sig->lsb()) {

	    sel = add_to_net(des, sel, -sig->lsb());
	    for (unsigned idx = 0 ;  idx < sig_width ;  idx += 1)
		  connect(mux->pin_Data(0, idx), sig->pin(idx));
      } else {

	    sel = add_to_net(des, sel, -sig->msb());
	    for (unsigned idx = 0 ;  idx < sig_width ;  idx += 1)
		  connect(mux->pin_Data(0, idx), sig->pin(sig_width-idx-1));
      }

      for (unsigned idx = 0 ;  idx < sel->pin_count() ;  idx += 1)
	    connect(mux->pin_Sel(idx), sel->pin(idx));

      NetNet*out = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, 1);
      connect(mux->pin_Result(0), out->pin(0));

      des->add_node(mux);
      out->local_flag(true);
      return out;
}

NetNet* PEIdent::elaborate_net(Design*des, NetScope*scope,
			       unsigned lwidth,
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay,
			       Link::strength_t drive0,
			       Link::strength_t drive1) const
{
      assert(scope);

      NetNet*       sig = 0;
      NetMemory*    mem = 0;
      NetVariable*  var = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, mem, var, par, eve);

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

	    sig = new NetNet(scope, lex_strings.make(path_.peek_name(0)),
			     NetNet::IMPLICIT, pc->expr_width());
	    NetConst*cp = new NetConst(scope, scope->local_symbol(),
				       pvalue);
	    des->add_node(cp);
	    for (unsigned idx = 0;  idx <  sig->pin_count(); idx += 1)
		  connect(sig->pin(idx), cp->pin(idx));
      }

      if (var != 0) {
	    cerr << get_line() << ": sorry: " << path_
		 << " is a real in a net/wire context." << endl;
	    des->errors += 1;
	    return 0;
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

      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  cerr << msb_->get_line() << ": error: unable to "
			"evaluate constant MSB expression: " << *msb_ <<
			endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum*lval = lsb_->eval_const(des, scope);
	    if (lval == 0) {
		  cerr << lsb_->get_line() << ": error: unable to "
			"evaluate constant LSB expression: " << *lsb_ <<
			endl;
		  delete mval;
		  des->errors += 1;
		  return 0;
	    }

	    assert(mval);
	    assert(lval);

	    long mbit = mval->as_long();
	    long lbit = lval->as_long();

	      /* Check that the part select is valid. Both ends of the
		 constant part select must be within the range of the
		 signal for the part select to be correct. */
	    if (! (sig->sb_is_valid(mbit) && sig->sb_is_valid(lbit))) {
		  cerr << get_line() << ": error: bit/part select ["
		       << mval->as_long() << ":" << lval->as_long()
		       << "] out of range for " << sig->name() << endl;
		  des->errors += 1;
		  return sig;
	    }

	    unsigned midx = sig->sb_to_idx(mbit);
	    unsigned lidx = sig->sb_to_idx(lbit);

	      /* This is a part select, create a new NetNet object
		 that connects to just the desired parts of the
		 identifier. Make sure the NetNet::Type is compatible
		 with the sig type.

		 Be careful to check the bit ordering. If the msb is
		 less significant then the msb, then the source is
		 broken. I can hack it in order to go on, but report
		 an error. */

	    if (midx < lidx) {
		  cerr << get_line() << ": error: part select "
		       << sig->name() << "[" << mval->as_long() << ":"
		       << lval->as_long() << "] "
		       << "has bit order reversed." << endl;
		  des->errors += 1;

		  unsigned tmp = midx;
		  midx = lidx;
		  lidx = tmp;
	    }

	    unsigned part_count = midx-lidx+1;

	    NetSubnet*tmp = new NetSubnet(sig, lidx, part_count);

	    sig = tmp;


      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  return elaborate_net_bitmux_(des, scope, sig, rise,
					       fall, decay, drive0, drive1);
	    }

	    assert(mval);
	    unsigned idx = sig->sb_to_idx(mval->as_long());
	    if (idx >= sig->pin_count()) {
		  cerr << get_line() << ": error: index " << sig->name() <<
			"[" << mval->as_long() << "] out of range." << endl;
		  des->errors += 1;
		  idx = 0;
	    }

	    NetSubnet*tmp = new NetSubnet(sig, idx, 1);
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
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay) const
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

      NetNet*adr = msb_->elaborate_net(des, scope, 0, 0, 0, 0);

      must_be_self_determined_flag = must_be_self_determined_save;

      if (adr == 0)
	    return 0;

      NetRamDq*ram = new NetRamDq(scope, scope->local_symbol(),
				  mem, adr->pin_count());
      des->add_node(ram);

      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(ram->pin_Address(idx), adr->pin(idx));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, ram->width());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(ram->pin_Q(idx), osig->pin(idx));

      return osig;
}

/*
 * The concatenation is also OK an an l-value. This method elaborates
 * it as a structural l-value.
 */
NetNet* PEConcat::elaborate_lnet(Design*des, NetScope*scope,
				 bool implicit_net_ok) const
{
      assert(scope);

      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_line() << ": sorry: I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

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
		  pins += nets[idx]->pin_count();
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
	   concat operator from least significant to most significant,
	   which is opposite from how they are given in the list. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, pins);
      pins = 0;
      for (unsigned idx = nets.count() ;  idx > 0 ;  idx -= 1) {
	    NetNet*cur = nets[idx-1];
	    for (unsigned pin = 0 ;  pin < cur->pin_count() ;  pin += 1) {
		  connect(osig->pin(pins), cur->pin(pin));
		  pins += 1;
	    }
      }

      osig->local_flag(true);
      return osig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, NetScope*scope,
				bool implicit_net_ok) const
{
      assert(scope);

      NetNet*       sig = 0;
      NetMemory*    mem = 0;
      NetVariable*  var = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, mem, var, par, eve);

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
	    NetNet::Type nettype = scope->default_nettype();

	    if (implicit_net_ok && !error_implicit && nettype!=NetNet::NONE) {

		  sig = new NetNet(scope, lex_strings.make(path_.peek_name(0)),
				   NetNet::IMPLICIT, 1);

		  if (warn_implicit) {
			cerr << get_line() << ": warning: implicit "
			      "definition of wire " << scope->name()
			     << "." << path_.peek_name(0) << "." << endl;
		  }

	    } else {
		  cerr << get_line() << ": error: Net " << path_
		       << " is not defined in this context." << endl;
		  cerr << get_line() << ":      : Do you mean this? wire "
		       << path_ << " = <expr>;" << endl;
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

      if (msb_ && lsb_) {
	      /* Detect a part select. Evaluate the bits and elaborate
		 the l-value by creating a sub-net that links to just
		 the right pins. */
	    verinum*mval = msb_->eval_const(des, scope);
	    assert(mval);
	    verinum*lval = lsb_->eval_const(des, scope);
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

	    if (midx >= lidx) {
		  unsigned subnet_wid = midx-lidx+1;
		  if (subnet_wid > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  NetSubnet*tmp = new NetSubnet(sig, lidx, subnet_wid);

		  sig = tmp;

	    } else {
		  unsigned subnet_wid = midx-lidx+1;

		  if (subnet_wid > sig->pin_count()) {
			cerr << get_line() << ": error: "
			     << "part select out of range for "
			     << sig->name() << "." << endl;
			des->errors += 1;
			return sig;
		  }

		  NetSubnet*tmp = new NetSubnet(sig, lidx, subnet_wid);

		  sig = tmp;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  cerr << get_line() << ": error: index of " << path_ <<
			" needs to be constant in l-value of assignment." <<
			endl;
		  des->errors += 1;
		  return 0;
	    }
	    assert(mval);
	    unsigned idx = sig->sb_to_idx(mval->as_long());
	    if (idx >= sig->pin_count()) {
		  cerr << get_line() << "; index " << sig->name() <<
			"[" << mval->as_long() << "] out of range." << endl;
		  des->errors += 1;
		  idx = 0;
	    }

	    NetSubnet*tmp = new NetSubnet(sig, idx, 1);
	    sig = tmp;
      }

      return sig;
}

/*
 * This method is used to elaborate identifiers that are ports to a
 * scope. The scope is presumed to be that of the module that has the
 * port.
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


      if (msb_ && lsb_) {
	      /* Detect a part select. Evaluate the bits and elaborate
		 the l-value by creating a sub-net that links to just
		 the right pins. */
	    verinum*mval = msb_->eval_const(des, scope);
	    assert(mval);
	    verinum*lval = lsb_->eval_const(des, scope);
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

	    if (midx >= lidx) {
		  unsigned part_count = midx-lidx+1;
		  if (part_count > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  NetSubnet*tmp = new NetSubnet(sig, lidx, part_count);
		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		    /* XXXX Signals reversed?? */
		  unsigned part_count = lidx-midx+1;
		  assert(part_count <= sig->pin_count());

		  NetSubnet*tmp = new NetSubnet(sig, midx, part_count);
		  for (unsigned idx = midx ;  idx >= lidx ;  idx -= 1)
			connect(tmp->pin(idx-midx), sig->pin(idx));

		  sig = tmp;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  cerr << get_line() << ": index of " << path_ <<
			" needs to be constant in port context." <<
			endl;
		  des->errors += 1;
		  return 0;
	    }
	    assert(mval);
	    unsigned idx = sig->sb_to_idx(mval->as_long());
	    if (idx >= sig->pin_count()) {
		  cerr << get_line() << "; index " << sig->name() <<
			"[" << mval->as_long() << "] out of range." << endl;
		  des->errors += 1;
		  idx = 0;
	    }
	    NetSubnet*tmp = new NetSubnet(sig, idx, 1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 * Elaborate a number as a NetConst object.
 */
NetNet* PENumber::elaborate_net(Design*des, NetScope*scope,
				unsigned lwidth,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay,
				Link::strength_t drive0,
				Link::strength_t drive1) const
{

	/* If we are constrained by a l-value size, then just make a
	   number constant with the correct size and set as many bits
	   in that constant as make sense. Pad excess with zeros. */
      if (lwidth > 0) {
	    NetNet*net = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, lwidth);
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

	    verinum num(top_v, net->pin_count());
	    unsigned idx;
	    for (idx = 0 ;  idx < num.len() && idx < value_->len(); idx += 1)
		  num.set(idx, value_->get(idx));

	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					num);
	    for (idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
		  tmp->pin(idx).drive0(drive0);
		  tmp->pin(idx).drive1(drive1);
		  connect(net->pin(idx), tmp->pin(idx));
	    }

	    des->add_node(tmp);
	    return net;
      }

	/* If the number has a length, then use that to size the
	   number. Generate a constant object of exactly the user
	   specified size. */
      if (value_->has_len()) {
	    NetNet*net = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, value_->len());
	    net->local_flag(true);
	    net->set_signed(value_->has_sign());
	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					*value_);
	    for (unsigned idx = 0 ;  idx < value_->len() ;  idx += 1)
		  connect(net->pin(idx), tmp->pin(idx));

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
      net->local_flag(true);
      NetConst*tmp = new NetConst(scope, scope->local_symbol(), num);
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(net->pin(idx), tmp->pin(idx));

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
				unsigned long rise,
				unsigned long fall,
				unsigned long decay,
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
      verinum num(verinum::V0, net->pin_count());

      unsigned idx;
      for (idx = 0 ;  idx < num.len() && idx < strbits; idx += 1) {
	    char byte = text_[strbits/8 - 1 - idx/8];
	    char mask = 1<<(idx%8);
	    num.set(idx, (byte & mask)? verinum::V1 : verinum::V0);
      }

      NetConst*tmp = new NetConst(scope, scope->local_symbol(), num);
      for (idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    tmp->pin(idx).drive0(drive0);
	    tmp->pin(idx).drive1(drive1);
	    connect(net->pin(idx), tmp->pin(idx));
      }

      des->add_node(tmp);
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
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay,
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


	/* The natural width of the expression is the width of the
	   largest condition. Normally they should be the same size,
	   but if we do not get a size from the context, or the
	   expressions resist, we need to cope. */
      unsigned iwidth = tru_sig->pin_count();
      if (fal_sig->pin_count() > iwidth)
	    iwidth = fal_sig->pin_count();


	/* If the width is not passed from the context, then take the
	   widest result as our width. */
      if (width == 0)
	    width = iwidth;

	/* If the expression has width, then generate a boolean result
	   by connecting an OR gate to calculate the truth value of
	   the result. In the end, the result needs to be a single bit. */
      if (expr_sig->pin_count() > 1) {
	    NetLogic*log = new NetLogic(scope, scope->local_symbol(),
					expr_sig->pin_count()+1,
					NetLogic::OR);
	    for (unsigned idx = 0;  idx < expr_sig->pin_count(); idx += 1)
		  connect(log->pin(idx+1), expr_sig->pin(idx));

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, 1);
	    tmp->local_flag(true);
	    connect(tmp->pin(0), log->pin(0));
	    des->add_node(log);

	    expr_sig = tmp;
      }

      assert(expr_sig->pin_count() == 1);

	/* This is the width of the LPM_MUX device that I'm about to
	   create. It may be smaller then the desired output, but I'll
	   handle padding below.

	   Create a NetNet object wide enough to hold the
	   result. Also, pad the result values (if necessary) so that
	   the mux inputs can be fully connected. */

      unsigned dwidth = (iwidth > width)? width : iwidth;

      NetNet*sig = new NetNet(scope, scope->local_symbol(),
			      NetNet::WIRE, width);
      sig->local_flag(true);

      if (fal_sig->pin_count() < dwidth)
	    fal_sig = pad_to_width(des, fal_sig, dwidth);

      if (tru_sig->pin_count() < dwidth)
	    tru_sig = pad_to_width(des, tru_sig, dwidth);


	/* Make the device and connect its outputs to the osig and
	   inputs to the tru and false case nets. Also connect the
	   selector bit to the sel input.

	   The inputs are the 0 (false) connected to fal_sig and 1
	   (true) connected to tru_sig.  */

      NetMux*mux = new NetMux(scope, scope->local_symbol(), dwidth, 2, 1);
      mux->set_line(*this);
      connect(mux->pin_Sel(0), expr_sig->pin(0));

	/* Connect the data inputs. */
      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
	    connect(mux->pin_Data(idx,0), fal_sig->pin(idx));
	    connect(mux->pin_Data(idx,1), tru_sig->pin(idx));
      }

	/* If there are non-zero output delays, then create bufz
	   devices to carry the propagation delays. Otherwise, just
	   connect the result to the output. */
      if (rise || fall || decay) {
	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::WIRE, dwidth);
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {

		  NetBUFZ*tmpz = new NetBUFZ(scope, scope->local_symbol());
		  tmpz->rise_time(rise);
		  tmpz->fall_time(fall);
		  tmpz->decay_time(decay);
		  tmpz->pin(0).drive0(drive0);
		  tmpz->pin(0).drive1(drive1);

		  connect(mux->pin_Result(idx), tmp->pin(idx));
		  connect(tmp->pin(idx), tmpz->pin(1));
		  connect(sig->pin(idx), tmpz->pin(0));

		  des->add_node(tmpz);
	    }

      } else {
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
		  connect(mux->pin_Result(idx), sig->pin(idx));
	    }
      }

	/* If the MUX device result is too narrow to fill out the
	   desired result, pad with zeros by creating a NetConst device. */

      if (dwidth < width) {
	    verinum vpad (verinum::V0, width-dwidth);
	    NetConst*pad = new NetConst(scope, scope->local_symbol(), vpad);
	    des->add_node(pad);
	    for (unsigned idx = dwidth ;  idx < width ;  idx += 1)
		  connect(sig->pin(idx), pad->pin(idx-dwidth));
      }

      des->add_node(mux);

      return sig;
}

NetNet* PEUnary::elaborate_net(Design*des, NetScope*scope,
			       unsigned width,
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay,
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
	    verinum*val = expr_->eval_const(des, scope);
	    if (val == 0)
		  break;

	    if (width == 0)
		  width = val->len();

	    assert(width > 0);
	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::WIRE, width);
	    sig->local_flag(true);

	      /* Take the 2s complement by taking the 1s complement
		 and adding 1. */
	    verinum tmp (v_not(*val));
	    verinum one (1UL, width);
	    tmp = tmp + one;
	    NetConst*con = new NetConst(scope, scope->local_symbol(), tmp);
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1)
		  connect(sig->pin(idx), con->pin(idx));

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
      NetLogic::TYPE gtype = NetLogic::AND;

      switch (op_) {
	  case '~': // Bitwise NOT
	    sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE,
			     sub_sig->pin_count());
	    sig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(scope, scope->local_symbol(),
				      2, NetLogic::NOT);
		  connect(gate->pin(1), sub_sig->pin(idx));
		  connect(gate->pin(0), sig->pin(idx));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
	    }
	    break;

	  case 'N': // Reduction NOR
	  case '!': // Reduction NOT
	    reduction=true; gtype = NetLogic::NOR; break;
	  case '&': // Reduction AND
	    reduction=true; gtype = NetLogic::AND; break;
	  case '|': // Reduction OR
	    reduction=true; gtype = NetLogic::OR; break;
	  case '^': // Reduction XOR
	    reduction=true; gtype = NetLogic::XOR; break;
	  case 'A': // Reduction NAND (~&)
	    reduction=true; gtype = NetLogic::NAND; break;
	  case 'X': // Reduction XNOR (~^)
	    reduction=true; gtype = NetLogic::XNOR; break;

	  case '-': // Unary 2's complement.
	    if (owidth == 0)
		  owidth = sub_sig->pin_count();

	    sig = new NetNet(scope, scope->local_symbol(),
			     NetNet::WIRE, owidth);
	    sig->local_flag(true);

	    if (sub_sig->pin_count() < owidth)
		  sub_sig = pad_to_width(des, sub_sig, owidth);

	    switch (sub_sig->pin_count()) {
		case 0:
		  assert(0);
		  break;

		case 1:
		  gate = new NetLogic(scope, scope->local_symbol(),
				      2, NetLogic::BUF);
		  connect(gate->pin(0), sig->pin(0));
		  connect(gate->pin(1), sub_sig->pin(0));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  break;

		case 2:
		  gate = new NetLogic(scope, scope->local_symbol(),
				      2, NetLogic::BUF);
		  connect(gate->pin(0), sig->pin(0));
		  connect(gate->pin(1), sub_sig->pin(0));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);

		  gate = new NetLogic(scope, scope->local_symbol(),
				      3, NetLogic::XOR);
		  connect(gate->pin(0), sig->pin(1));
		  connect(gate->pin(1), sub_sig->pin(0));
		  connect(gate->pin(2), sub_sig->pin(1));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  break;

		default:
		  NetAddSub*sub = new NetAddSub(scope, scope->local_symbol(),
						owidth);
		  sub->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));

		  des->add_node(sub);

		  for (unsigned idx = 0 ;  idx < owidth ;  idx += 1)
			connect(sig->pin(idx), sub->pin_Result(idx));

		  for (unsigned idx = 0; idx < owidth; idx += 1)
			connect(sub_sig->pin(idx), sub->pin_DataB(idx));

		  verinum tmp_num (verinum::V0, sub->width(), true);
		  NetConst*tmp_con = new NetConst(scope,
						  scope->local_symbol(),
						  tmp_num);
		  des->add_node(tmp_con);

		  NetNet*tmp_sig = new NetNet(scope, scope->local_symbol(),
					      NetNet::WIRE, owidth);
		  tmp_sig->local_flag(true);

		  for (unsigned idx = 0; idx < sig->pin_count(); idx += 1) {
			connect(tmp_sig->pin(idx), sub->pin_DataA(idx));
			connect(tmp_sig->pin(idx), tmp_con->pin(idx));
		  }
		  break;
	    }
	    break;

	  default:
	    cerr << "internal error: Unhandled UNARY '" << op_ << "'" << endl;
	    sig = 0;
      }
      if (reduction) {
	    sig = new NetNet(scope, scope->local_symbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_symbol(),
				1+sub_sig->pin_count(), gtype);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
      }

      return sig;
}

/*
 * $Log: elab_net.cc,v $
 * Revision 1.138.2.3  2005/09/11 02:56:37  steve
 *  Attach line numbers to NetMux devices.
 *
 * Revision 1.138.2.2  2005/02/19 16:39:30  steve
 *  Spellig fixes.
 *
 * Revision 1.138.2.1  2005/01/29 00:18:23  steve
 *  Fix evaluate of constants in netlist concatenation repeats.
 *
 * Revision 1.138  2004/10/04 03:09:38  steve
 *  Fix excessive error message.
 *
 * Revision 1.137  2004/10/04 01:10:52  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.136  2004/10/04 00:25:46  steve
 *  Error message to match assertion.
 *
 * Revision 1.135  2004/09/24 04:25:19  steve
 *  Detect and prevent implicit declaration of hierarchical names.
 *
 * Revision 1.134  2004/08/28 15:42:12  steve
 *  Add support for $unsigned.
 *
 * Revision 1.133  2004/06/30 02:16:26  steve
 *  Implement signed divide and signed right shift in nets.
 *
 * Revision 1.132  2004/06/24 15:22:23  steve
 *  Code cleanup from Larry.
 *
 * Revision 1.131  2004/06/22 18:41:48  steve
 *  Fix broken calcuation of NE for constant.
 *
 * Revision 1.130  2004/06/18 16:38:22  steve
 *  compare-to-constant uses sig len, not val len.
 *
 * Revision 1.129  2004/06/16 23:32:58  steve
 *  Handle equality compare to constants specially.
 *
 * Revision 1.128  2004/06/13 04:56:53  steve
 *  Add support for the default_nettype directive.
 *
 * Revision 1.127  2004/06/01 01:04:57  steve
 *  Fix synthesis method for logical and/or
 *
 * Revision 1.126  2004/05/31 23:34:36  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.125  2004/02/20 18:53:34  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.124  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.123  2004/02/15 04:23:48  steve
 *  Fix evaluation of compare to constant expression.
 *
 * Revision 1.122  2003/10/30 04:31:34  steve
 *  Catch real variables in net expressions.
 *
 * Revision 1.121  2003/10/20 01:44:28  steve
 *  memory index need not be self determined width.
 *
 * Revision 1.120  2003/09/23 03:31:28  steve
 *  Catch unsized expressions in continuous assigns.
 *
 * Revision 1.119  2003/09/19 03:50:12  steve
 *  Remove find_memory method from Design class.
 *
 * Revision 1.118  2003/09/13 01:30:07  steve
 *  Missing case warnings.
 *
 * Revision 1.117  2003/09/03 04:29:18  steve
 *  Only build a mux as wide as can be selected.
 *
 * Revision 1.116  2003/08/28 04:11:17  steve
 *  Spelling patch.
 *
 * Revision 1.115  2003/08/05 03:01:58  steve
 *  Primitive outputs have same limitations as continuous assignment.
 *
 * Revision 1.114  2003/06/21 01:21:43  steve
 *  Harmless fixup of warnings.
 *
 * Revision 1.113  2003/05/01 01:13:57  steve
 *  More complete bit range internal error message,
 *  Better test of part select ranges on non-zero
 *  signal ranges.
 *
 * Revision 1.112  2003/04/11 05:18:08  steve
 *  Handle signed magnitude compare all the
 *  way through to the vvp code generator.
 *
 * Revision 1.111  2003/03/29 05:51:25  steve
 *  Sign extend NetMult inputs if result is signed.
 *
 * Revision 1.110  2003/03/26 06:16:38  steve
 *  Some better internal error messages.
 *
 * Revision 1.109  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.108  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.107  2003/02/26 01:29:24  steve
 *  LPM objects store only their base names.
 *
 * Revision 1.106  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.105  2003/01/19 00:35:39  steve
 *  Detect null arguments to concatenation operator.
 *
 * Revision 1.104  2003/01/17 05:48:35  steve
 *  Remove useless variable.
 *
 * Revision 1.103  2002/12/06 03:08:19  steve
 *  Reword some error messages for clarity.
 *
 * Revision 1.102  2002/11/09 19:20:48  steve
 *  Port expressions for output ports are lnets, not nets.
 *
 * Revision 1.101  2002/09/18 04:29:55  steve
 *  Add support for binary NOR operator.
 *
 * Revision 1.100  2002/09/12 15:49:43  steve
 *  Add support for binary nand operator.
 *
 * Revision 1.99  2002/09/08 01:37:13  steve
 *  Fix padding of operand of unary minus.
 *
 * Revision 1.98  2002/08/31 03:48:50  steve
 *  Fix reverse bit ordered bit select in continuous assignment.
 *
 * Revision 1.97  2002/08/21 02:28:03  steve
 *  Carry mux output delays.
 *
 * Revision 1.96  2002/08/14 03:57:27  steve
 *  Constants can self-size themselves in unsized contexts.
 *
 * Revision 1.95  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.94  2002/08/05 04:18:45  steve
 *  Store only the base name of memories.
 *
 * Revision 1.93  2002/07/05 21:26:17  steve
 *  Avoid emitting to vvp local net symbols.
 *
 * Revision 1.92  2002/06/22 04:22:40  steve
 *  Wide unary minus in continuous assignments.
 *
 * Revision 1.91  2002/06/19 04:20:03  steve
 *  Remove NetTmp and add NetSubnet class.
 *
 * Revision 1.90  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.89  2002/04/23 03:53:59  steve
 *  Add support for non-constant bit select.
 *
 * Revision 1.88  2002/04/22 00:53:39  steve
 *  Do not allow implicit wires in sensitivity lists.
 *
 * Revision 1.87  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.86  2002/01/23 05:23:17  steve
 *  No implicit declaration in assign l-values.
 *
 * Revision 1.85  2002/01/03 04:19:01  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.84  2001/12/31 04:23:59  steve
 *  Elaborate multiply nets with constant operands ad NetConst.
 */

