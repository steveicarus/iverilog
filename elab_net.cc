/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: elab_net.cc,v 1.89 2002/04/23 03:53:59 steve Exp $"
#endif

# include "config.h"

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

# include  <iostream>

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
	  case 'X': // Exclusing NOR
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
      NetNode*gate;
      NetNode*gate_t;

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
	    assert(0);
	    break;
	  default:
	    cerr << get_line() << ": internal error: unsupported"
		  " combinational operator (" << op_ << ")." << endl;
	    des->errors += 1;
	    osig = 0;
      }

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(lsig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(rsig))
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
      NetNode*gate;
      NetNode*gate_t;

      string name = scope->local_hsymbol();
      unsigned width = lsig->pin_count();
      if (rsig->pin_count() > lsig->pin_count())
	    width = rsig->pin_count();


	/* The owidth is the output width of the lpm_add_sub
	   device. If the desired with is greater then the width of
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
      osig = new NetNet(scope, scope->local_hsymbol(),
			NetNet::WIRE, owidth);
      osig->local_flag(true);
      NetAddSub*adder = new NetAddSub(scope, name, width);

	// Connect the adder to the various parts.
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(lsig->pin(idx), adder->pin_DataA(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(rsig->pin(idx), adder->pin_DataB(idx));
      for (unsigned idx = 0 ;  idx < width ; idx += 1)
	    connect(osig->pin(idx), adder->pin_Result(idx));
      if (owidth > width)
	    connect(osig->pin(width), adder->pin_Cout());

      gate = adder;
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      switch (op_) {
	  case '+':
	    gate->attribute("LPM_Direction", "ADD");
	    break;
	  case '-':
	    gate->attribute("LPM_Direction", "SUB");
	    break;
      }


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

      NetNet*osig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE,
			       lsig->pin_count());
      osig->local_flag(true);

      switch (op_) {
	  case '^': // XOR
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*gate = new NetLogic(scope, scope->local_hsymbol(),
					       3, NetLogic::XOR);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    break;

	  case 'X': // XNOR
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*gate = new NetLogic(scope, scope->local_hsymbol(),
					       3, NetLogic::XNOR);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    break;

	  case '&': // AND
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*gate = new NetLogic(scope, scope->local_hsymbol(),
					       3, NetLogic::AND);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    break;

	  case '|': // Bitwise OR
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*gate = new NetLogic(scope, scope->local_hsymbol(),
					       3, NetLogic::OR);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    break;

	  default:
	    assert(0);
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
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
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

      unsigned dwidth = lsig->pin_count();
      if (rsig->pin_count() > dwidth) dwidth = rsig->pin_count();

      NetNet*zero = 0;
      if (lsig->pin_count() != rsig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
					verinum::V0);
	    des->add_node(tmp);
	    zero = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    connect(tmp->pin(0), zero->pin(0));
      }

      NetNet*osig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
      osig->local_flag(true);

      NetNode*gate;
	//NetNode*gate_t;

      switch (op_) {
	  case '<':
	  case '>':
	  case 'L':
	  case 'G': {
		NetCompare*cmp = new
		      NetCompare(scope, scope->local_hsymbol(), dwidth);
		for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
		      connect(cmp->pin_DataA(idx), lsig->pin(idx));
		for (unsigned idx = lsig->pin_count(); idx < dwidth ; idx += 1)
		      connect(cmp->pin_DataA(idx), zero->pin(0));
		for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		      connect(cmp->pin_DataB(idx), rsig->pin(idx));
		for (unsigned idx = rsig->pin_count(); idx < dwidth ; idx += 1)
		      connect(cmp->pin_DataB(idx), zero->pin(0));

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
		gate = cmp;
		break;
	  }

	  case 'E': // Case equals (===)
	  case 'N': // Case equals (!==)
	      // The comparison generates gates to bitwise compare
	      // each pair, and AND all the comparison results.

	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+dwidth,
				(op_ == 'E')? NetLogic::AND : NetLogic::NAND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
		  NetCaseCmp*cmp = new NetCaseCmp(scope,
						  scope->local_hsymbol());

		  if (idx < lsig->pin_count())
			connect(cmp->pin(1), lsig->pin(idx));
		  else
			connect(cmp->pin(1), zero->pin(0));

		  if (idx < rsig->pin_count())
			connect(cmp->pin(2), rsig->pin(idx));
		  else
			connect(cmp->pin(2), zero->pin(0));

		  connect(cmp->pin(0), gate->pin(idx+1));
		  des->add_node(cmp);

		    // Attach a label to this intermediate wire
		  NetNet*tmp = new NetNet(scope, scope->local_hsymbol(),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(cmp->pin(0), tmp->pin(0));
	    }
	    break;


	  case 'e': // ==

	      /* Handle the special case of single bit compare with a
		 single XNOR gate. This is easy and direct. */
	    if (dwidth == 1) {
		  gate = new NetLogic(scope, scope->local_hsymbol(),
				      3, NetLogic::XNOR);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_hsymbol(),
					      dwidth);
	      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {

		    if (idx < lsig->pin_count())
			  connect(cmp->pin_DataA(idx), lsig->pin(idx));
		    else
			  connect(cmp->pin_DataA(idx), zero->pin(0));

		    if (idx < rsig->pin_count())
			  connect(cmp->pin_DataB(idx), rsig->pin(idx));
		    else
			  connect(cmp->pin_DataB(idx), zero->pin(0));

	      }
	      connect(cmp->pin_AEB(), osig->pin(0));
	      gate = cmp;
	    }
	    break;

	  case 'n': // !=

	      /* Handle the special case of single bit compare with a
		 single XOR gate. This is easy and direct. */
	    if (dwidth == 1) {
		  gate = new NetLogic(scope, scope->local_hsymbol(),
				      3, NetLogic::XOR);
		  connect(gate->pin(0), osig->pin(0));
		  connect(gate->pin(1), lsig->pin(0));
		  connect(gate->pin(2), rsig->pin(0));
		  break;
	    }

	      /* Oh well, do the general case with a NetCompare. */
	    { NetCompare*cmp = new NetCompare(scope, scope->local_hsymbol(),
					      dwidth);
	      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {

		    if (idx < lsig->pin_count())
			  connect(cmp->pin_DataA(idx), lsig->pin(idx));
		    else
			  connect(cmp->pin_DataA(idx), zero->pin(0));

		    if (idx < rsig->pin_count())
			  connect(cmp->pin_DataB(idx), rsig->pin(idx));
		    else
			  connect(cmp->pin_DataB(idx), zero->pin(0));

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
 * Elaborate a divider gate. This function create a NetDevide gate
 * which has exactly the right sized DataA, DataB and Result ports. If
 * the l-value is wider then the result, then pad.
 */
NetNet* PEBinary::elaborate_net_div_(Design*des, NetScope*scope,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
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
      NetDivide*div = new NetDivide(scope, scope->local_hsymbol(), rwidth,
				    lsig->pin_count(),
				    rsig->pin_count());
      des->add_node(div);


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

      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < rwidth ;  idx += 1)
	    connect(div->pin_Result(idx), osig->pin(idx));


	// If the lvalue is larger then the result, then pad the
	// output with constant 0. This can happen for example in
	// cases like this:
	//    wire [3;0] a, b;
	//    wire [7:0] r = a / b;

      if (rwidth < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
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
      NetModulo*mod = new NetModulo(scope, scope->local_hsymbol(), rwidth,
				    lsig->pin_count(),
				    rsig->pin_count());
      des->add_node(mod);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(mod->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(mod->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(mod->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
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
      NetNet*lsig = left_->elaborate_net(des, scope, 0, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, scope, 0, 0, 0, 0);
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
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				3, NetLogic::AND);
	    break;
	  case 'o':
	    gate = new NetLogic(scope, scope->local_hsymbol(),
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
	    gate_t = new NetLogic(scope, scope->local_hsymbol(),
				  1+lsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), lsig->pin(idx));

	    connect(gate->pin(1), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetTmp(scope, scope->local_hsymbol());
	    connect(gate->pin(1), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(1), lsig->pin(0));
      }

	// The second OR gate returns 1 if the right value is true...
      if (rsig->pin_count() > 1) {
	    gate_t = new NetLogic(scope, scope->local_hsymbol(),
				  1+rsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), rsig->pin(idx));
	    connect(gate->pin(2), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetTmp(scope, scope->local_hsymbol());
	    connect(gate->pin(2), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(2), rsig->pin(0));
      }

	// The output is the AND/OR of the two logic values.
      NetNet*osig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
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
	   of the multiply are constant expressions. Evalulate the
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

	    NetConst*odev = new NetConst(scope, scope->local_hsymbol(), res);
	    NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
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

      NetMult*mult = new NetMult(scope, scope->local_hsymbol(), rwidth,
				 lsig->pin_count(),
				 rsig->pin_count());
      des->add_node(mult);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(mult->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
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

	    NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
				     NetNet::WIRE, lwidth);
	    osig->local_flag(true);

	    NetConst*zero = new NetConst(scope, scope->local_hsymbol(),
					 verinum::V0);
	    des->add_node(zero);

	    if (op_ == 'l') {
		  unsigned idx;
		  for (idx = 0 ;  idx < dist ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));
		  for (    ; (idx<lwidth) && ((idx-dist) < lsig->pin_count())
			   ; idx += 1)
			connect(osig->pin(idx), lsig->pin(idx-dist));
		  for (    ;  idx < lwidth ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));

	    } else {
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
      while ((1 << dwid) < lwidth)
	    dwid += 1;

      NetNet*rsig = right_->elaborate_net(des, scope, dwid, 0, 0, 0);
      if (rsig == 0) return 0;

	// Make the shift device itself, and the output
	// NetNet. Connect the Result output pins to the osig signal
      NetCLShift*gate = new NetCLShift(scope, scope->local_hsymbol(),
				       lwidth, rsig->pin_count());

      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::WIRE, lwidth);
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < lwidth ;  idx += 1)
	    connect(osig->pin(idx), gate->pin_Result(idx));

	// Connect the lsig (the left expression) to the Data input,
	// and pad it if necessary with constant zeros.
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(lsig->pin(idx), gate->pin_Data(idx));

      if (lsig->pin_count() < lwidth) {
	    NetConst*zero = new NetConst(scope, scope->local_hsymbol(),
					 verinum::V0);
	    NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol());
	    des->add_node(zero);
	    connect(zero->pin(0), tmp->pin(0));
	    for (unsigned idx = lsig->pin_count() ; idx < lwidth ;  idx += 1)
		  connect(zero->pin(0), gate->pin_Data(idx));
      }

	// Connect the rsig (the shift amount expression) to the
	// Distance input.
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
	    connect(rsig->pin(idx), gate->pin_Distance(idx));

      if (op_ == 'r') {
	    NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol());
	    NetConst*dir = new NetConst(scope, scope->local_hsymbol(),
					verinum::V1);
	    connect(dir->pin(0), gate->pin_Direction());
	    connect(tmp->pin(0), gate->pin_Direction());
	    des->add_node(dir);
      }

      des->add_node(gate);

      return osig;
}

/*
 * This method elaborates a call to a function in the context of a
 * continuous assignment.
 */
NetNet* PECallFunction::elaborate_net(Design*des, NetScope*scope, unsigned,
				      unsigned long,
				      unsigned long,
				      unsigned long,
				      Link::strength_t,
				      Link::strength_t) const
{
      unsigned errors = 0;
      unsigned func_pins = 0;

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

	/* check the validity of the parameters. */
      if (! check_call_matches_definition_(des, dscope))
	    return 0;

	/* Elaborate all the parameters of the function call,
	   and collect the resulting NetNet objects. All the
	   parameters take on the size of the target port. */

      svector<NetNet*> eparms (def->port_count()-1);
      for (unsigned idx = 0 ;  idx < eparms.count() ;  idx += 1) {
	    const NetNet* port_reg = def->port(idx+1);
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
					scope->local_hsymbol().c_str(),
					dscope);
      des->add_node(net);

	/* Create an output signal and connect it to the output pins
	   of the function net. */
      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
			       NetNet::WIRE,
			       def->port(0)->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(net->port_pin(0, idx), osig->pin(idx));

	/* Connect the parameter pins to the parameter expressions. */
      for (unsigned idx = 0 ; idx < eparms.count() ; idx += 1) {
	    const NetNet* port = def->port(idx+1);
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
	    verinum*rep = repeat_->eval_const(des, scope);
	    if (rep == 0) {
		  cerr << get_line() << ": internal error: Unable to "
		       << "evaluate constant repeat expression." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = rep->as_ulong();

	    if (repeat == 0) {
		  cerr << get_line() << ": error: Invalid repeat value."
		       << endl;
		  des->errors += 1;
		  delete rep;
		  return 0;
	    }
      }

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

      NetNet*osig = new NetNet(scope, scope->local_hsymbol(),
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
 * This provate method handles the special case that we have a
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

      NetMux*mux = new NetMux(scope, scope->local_hsymbol(), 1,
			      sig->pin_count(),
			      sel->pin_count());

      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1)
	    connect(mux->pin_Data(0, idx), sig->pin(idx));

      for (unsigned idx = 0 ;  idx < sel->pin_count() ;  idx += 1)
	    connect(mux->pin_Sel(idx), sel->pin(idx));

      NetNet*out = new NetNet(scope, scope->local_hsymbol(),
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
      NetNet*sig = des->find_signal(scope, path_);

      if (sig == 0) {
	      /* If the identifier is a memory instead of a signal,
		 then handle it elsewhere. Create a RAM. */
	    if (NetMemory*mem = des->find_memory(scope, path_))
		  return elaborate_net_ram_(des, scope, mem, lwidth,
					    rise, fall, decay);


	    if (const NetExpr*pe = des->find_parameter(scope, path_)) {

		  const NetEConst*pc = dynamic_cast<const NetEConst*>(pe);
		  assert(pc);
		  verinum pvalue = pc->value();
		  sig = new NetNet(scope,
				   scope->name()+"."+path_.peek_name(0),
				   NetNet::IMPLICIT, pc->expr_width());
		  NetConst*cp = new NetConst(scope, scope->local_hsymbol(),
					     pvalue);
		  des->add_node(cp);
		  for (unsigned idx = 0;  idx <  sig->pin_count(); idx += 1)
			connect(sig->pin(idx), cp->pin(idx));

	    } else {

		  sig = new NetNet(scope, scope->name()+"."+path_.peek_name(0),
				   NetNet::IMPLICIT, 1);

		  if (error_implicit) {
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
      }

      assert(sig);

      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const(des, scope);
	    if (mval == 0) {
		  cerr << msb_->get_line() << ": error: unable to "
			"evaluate constant expression: " << *msb_ <<
			endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum*lval = lsb_->eval_const(des, scope);
	    if (lval == 0) {
		  cerr << lsb_->get_line() << ": error: unable to "
			"evaluate constant expression: " << *lsb_ <<
			endl;
		  delete mval;
		  des->errors += 1;
		  return 0;
	    }

	    assert(mval);
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

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

	    NetNet*tmp = new NetNet(scope, scope->local_hsymbol(),
				    sig->type(), midx-lidx+1);
	    tmp->local_flag(true);

	      /* Check that the bit or part select of the signal is
		 within the range of the part. The lidx is the
		 normalized index of the LSB, so that plus the desired
		 width must be <= the width of the references signal. */
	    if ((lidx + tmp->pin_count()) > sig->pin_count()) {
		  cerr << get_line() << ": error: bit/part select ["
		       << mval->as_long() << ":" << lval->as_long()
		       << "] out of range for " << sig->name() << endl;
		  des->errors += 1;
		  return sig;
	    }

	    for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
		  connect(tmp->pin(idx-lidx), sig->pin(idx));

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

	      /* This is a bit select, create a compatible NetNet with
		 a single bit that links to the selected bit of the
		 expression. */
	    NetNet*tmp = new NetNet(scope, scope->local_hsymbol(),
				    sig->type(), 1);
	    tmp->local_flag(true);

	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 * When I run into an identifier in an expression that referrs to a
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

      NetNet*adr = msb_->elaborate_net(des, scope, 0, 0, 0, 0);
      if (adr == 0)
	    return 0;


      NetRamDq*ram = new NetRamDq(scope, des->local_symbol(mem->name()),
				  mem, adr->pin_count());
      des->add_node(ram);

      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(ram->pin_Address(idx), adr->pin(idx));

      NetNet*osig = new NetNet(scope, des->local_symbol(mem->name()),
			       NetNet::IMPLICIT, ram->width());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(ram->pin_Q(idx), osig->pin(idx));

      return osig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, NetScope*scope) const
{
      string path = scope->name();

      NetNet*sig = des->find_signal(scope, path_);
      if (sig == 0) {
	      /* Don't allow memories here. Is it a memory? */
	    if (des->find_memory(scope, path_)) {
		  cerr << get_line() << ": error: memories (" << path_
		       << ") cannot be l-values in continuous "
		       << "assignments." << endl;
		  return 0;
	    }

	    cerr << get_line() << ": error: Net " << path_
		 << " is not defined in this context." << endl;
	    cerr << get_line() << ":      : Do you mean this? wire "
		 << path_ << " = <expr>;" << endl;
	    return 0;
      }

      assert(sig);

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_line() << ": error: registers (" << sig->name()
		 << ") cannot be l-values in continuous"
		 << " assignments." << endl;
	    return 0;
      }

      if (sig->port_type() == NetNet::PINPUT) {
	    cerr << get_line() << ": warning: assign l-value ``"
		 << sig->name() << "'' is also an input to "
		 << sig->scope()->name() << "." << endl;
	    cerr << sig->get_line() << ": warning: input ``"
		 << sig->name() << "'' is coerced to inout." << endl;
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
		  NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(),
					  midx-lidx+1);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(),
					  lidx-midx+1);

		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": error: "
			     << "part select out of range for "
			     << sig->name() << "." << endl;
			des->errors += 1;
			return sig;
		  }

		  assert(tmp->pin_count() <= sig->pin_count());
		  for (unsigned idx = lidx ;  idx >= midx ;  idx -= 1)
			connect(tmp->pin(idx-midx), sig->pin(idx));

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
	    NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(), 1);
	    connect(tmp->pin(0), sig->pin(idx));
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
		  NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(),
					  midx-lidx+1);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(),
					  lidx-midx+1);
		  assert(tmp->pin_count() <= sig->pin_count());
		  for (unsigned idx = lidx ;  idx >= midx ;  idx -= 1)
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
	    NetTmp*tmp = new NetTmp(scope, scope->local_hsymbol(), 1);
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
	    NetNet*net = new NetNet(scope, scope->local_hsymbol(),
				    NetNet::IMPLICIT, lwidth);
	    net->local_flag(true);

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
	    }

	    verinum num(top_v, net->pin_count());
	    unsigned idx;
	    for (idx = 0 ;  idx < num.len() && idx < value_->len(); idx += 1)
		  num.set(idx, value_->get(idx));

	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
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
	    NetNet*net = new NetNet(scope, scope->local_hsymbol(),
				    NetNet::IMPLICIT, value_->len());
	    net->local_flag(true);
	    NetConst*tmp = new NetConst(scope, scope->local_hsymbol(),
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

      NetNet*net = new NetNet(scope, scope->local_hsymbol(),
			      NetNet::IMPLICIT, width);
      net->local_flag(true);
      NetConst*tmp = new NetConst(scope, scope->local_hsymbol(), num);
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
	    net = new NetNet(scope, scope->local_hsymbol(),
			     NetNet::IMPLICIT, lwidth);

      } else {
	    net = new NetNet(scope, scope->local_hsymbol(),
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

      NetConst*tmp = new NetConst(scope, scope->local_hsymbol(), num);
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
	    NetLogic*log = new NetLogic(scope, scope->local_hsymbol(),
					expr_sig->pin_count()+1,
					NetLogic::OR);
	    for (unsigned idx = 0;  idx < expr_sig->pin_count(); idx += 1)
		  connect(log->pin(idx+1), expr_sig->pin(idx));

	    NetNet*tmp = new NetTmp(scope, scope->local_hsymbol());
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

      NetNet*sig = new NetNet(scope, scope->local_hsymbol(),
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

      NetMux*mux = new NetMux(scope, scope->local_hsymbol(), dwidth, 2, 1);
      connect(mux->pin_Sel(0), expr_sig->pin(0));

      for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
	    connect(mux->pin_Result(idx), sig->pin(idx));
	    connect(mux->pin_Data(idx,0), fal_sig->pin(idx));
	    connect(mux->pin_Data(idx,1), tru_sig->pin(idx));
      }


	/* If the MUX device result is too narrow to fill out the
	   desired result, pad with zeros by creating a NetConst device. */

      if (dwidth < width) {
	    verinum vpad (verinum::V0, width-dwidth);
	    NetConst*pad = new NetConst(scope, scope->local_hsymbol(), vpad);
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

      NetNet* sig;
      NetLogic*gate;

	// Handle the special case of a 2's complement of a constant
	// value. This can be reduced to a no-op on a precalculated
	// result.
      if (op_ == '-') do {
	    verinum*val = expr_->eval_const(des, scope);
	    if (val == 0)
		  break;

	    sig = new NetNet(scope, scope->local_hsymbol(),
			     NetNet::WIRE, width);
	    sig->local_flag(true);

	    verinum tmp(v_not(*val) + verinum(1UL, width), width);
	    NetConst*con = new NetConst(scope, scope->local_hsymbol(), tmp);
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

      switch (op_) {
	  case '~': // Bitwise NOT
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE,
			     sub_sig->pin_count());
	    sig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(scope, scope->local_hsymbol(),
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
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::NOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '&': // Reduction AND
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::AND);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '|': // Reduction OR
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::OR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '^': // Reduction XOR
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::XOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case 'A': // Reduction NAND (~&)
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::NAND);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;


	  case 'X': // Reduction XNOR (~^)
	    sig = new NetNet(scope, scope->local_hsymbol(), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(scope, scope->local_hsymbol(),
				1+sub_sig->pin_count(), NetLogic::XNOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '-': // Unary 2's complement.
	    sig = new NetNet(scope, scope->local_hsymbol(),
			     NetNet::WIRE, sub_sig->pin_count());
	    sig->local_flag(true);

	    switch (sub_sig->pin_count()) {
		case 0:
		  assert(0);
		  break;

		case 1:
		  gate = new NetLogic(scope, scope->local_hsymbol(),
				      2, NetLogic::BUF);
		  connect(gate->pin(0), sig->pin(0));
		  connect(gate->pin(1), sub_sig->pin(0));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  break;

		case 2:
		  gate = new NetLogic(scope, scope->local_hsymbol(),
				      2, NetLogic::BUF);
		  connect(gate->pin(0), sig->pin(0));
		  connect(gate->pin(1), sub_sig->pin(0));
		  des->add_node(gate);
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);

		  gate = new NetLogic(scope, scope->local_hsymbol(),
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
		  cerr << get_line() << ": internal error: Wide unary " 
		       << "minus not supported here." << endl;
		  des->errors += 1;
		  sig = 0;
		  break;
	    }
	    break;

	  default:
	    cerr << "internal error: Unhandled UNARY '" << op_ << "'" << endl;
	    sig = 0;
      }


      return sig;
}

/*
 * $Log: elab_net.cc,v $
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
 *
 * Revision 1.83  2001/12/30 21:32:03  steve
 *  Support elaborate_net for PEString objects.
 *
 * Revision 1.82  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.81  2001/11/10 02:08:49  steve
 *  Coerse input to inout when assigned to.
 *
 * Revision 1.80  2001/11/08 05:15:50  steve
 *  Remove string paths from PExpr elaboration.
 *
 * Revision 1.79  2001/11/07 04:26:46  steve
 *  elaborate_lnet uses scope instead of string path.
 *
 * Revision 1.78  2001/11/07 04:01:59  steve
 *  eval_const uses scope instead of a string path.
 *
 * Revision 1.77  2001/10/28 01:14:53  steve
 *  NetObj constructor finally requires a scope.
 *
 * Revision 1.76  2001/10/16 02:19:26  steve
 *  Support IVL_LPM_DIVIDE for structural divide.
 *
 * Revision 1.75  2001/09/14 04:20:49  steve
 *  dead code.
 *
 * Revision 1.74  2001/09/14 04:16:52  steve
 *  Elaborate == to NetCompare instead of XNOR and AND
 *  gates. This allows code generators to generate
 *  better code in certain cases.
 */

