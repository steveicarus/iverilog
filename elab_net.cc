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
#ident "$Id: elab_net.cc,v 1.31 2000/05/02 00:58:11 steve Exp $"
#endif

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

/*
 * Elaborating binary operations generally involves elaborating the
 * left and right expressions, then making an output wire and
 * connecting the lot together with the right kind of gate.
 */
NetNet* PEBinary::elaborate_net(Design*des, const string&path,
				unsigned width,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      switch (op_) {
	  case '*':
	    return elaborate_net_mul_(des, path, width, rise, fall, decay);
	      //case '%':
	  case '/':
	    return elaborate_net_div_(des, path, width, rise, fall, decay);
	  case '+':
	  case '-':
	    return elaborate_net_add_(des, path, width, rise, fall, decay);
	  case '|': // Bitwise OR
	  case '&':
	  case '^':
	  case 'X': // Exclusing NOR
	    return elaborate_net_bit_(des, path, width, rise, fall, decay);
	  case 'E':
	  case 'e':
	  case 'n':
	  case '<':
	  case '>':
	  case 'L': // <=
	  case 'G': // >=
	    return elaborate_net_cmp_(des, path, width, rise, fall, decay);
	  case 'a': // && (logical and)
	  case 'o': // || (logical or)
	    return elaborate_net_log_(des, path, width, rise, fall, decay);
	  case 'l': // <<
	  case 'r': // >>
	    return elaborate_net_shift_(des, path, width, rise, fall, decay);
      }

      NetNet*lsig = left_->elaborate_net(des, path, width, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, path, width, 0, 0, 0);
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
NetNet* PEBinary::elaborate_net_add_(Design*des, const string&path,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      NetNet*lsig = left_->elaborate_net(des, path, lwidth, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, path, lwidth, 0, 0, 0);
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

      string name = des->local_symbol(path);
      unsigned width = lsig->pin_count();
      if (rsig->pin_count() > lsig->pin_count())
	    width = rsig->pin_count();

	// If the desired output size if creater then the largest
	// operand, then include the carry of the adder as an output.
      unsigned owidth = width;
      if (lwidth > owidth)
	    owidth = width + 1;

	// Pad out the operands, if necessary, the match the width of
	// the adder device.
      if (lsig->pin_count() < width)
	    lsig = pad_to_width(des, path, lsig, width);

      if (rsig->pin_count() < width)
	    rsig = pad_to_width(des, path, rsig, width);

	// Make the adder as wide as the widest operand
      osig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE, owidth);
      NetAddSub*adder = new NetAddSub(name, width);

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
NetNet* PEBinary::elaborate_net_bit_(Design*des, const string&path,
				     unsigned width,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      NetNet*lsig = left_->elaborate_net(des, path, width, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, path, width, 0, 0, 0);
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
	    lsig = pad_to_width(des, path, lsig, rsig->pin_count());
      if (rsig->pin_count() < lsig->pin_count())
	    rsig = pad_to_width(des, path, rsig, lsig->pin_count());

      if (lsig->pin_count() != rsig->pin_count()) {
	    cerr << get_line() << ": internal error: lsig pin count ("
		 << lsig->pin_count() << ") != rsig pin count ("
		 << rsig->pin_count() << ")." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(lsig->pin_count() == rsig->pin_count());

      NetNet*osig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE,
			       lsig->pin_count());
      osig->local_flag(true);

      switch (op_) {
	  case '^': // XOR
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::XOR);
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
		  NetLogic*gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::XNOR);
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
		  NetLogic*gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::AND);
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
		  NetLogic*gate = new NetLogic(des->local_symbol(path),
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

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(lsig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(rsig))
	    delete tmp;

      return osig;
}

/*
 * Elaborate the various binary comparison operators. The comparison
 * operators return a single bit result, no matter what, so the left
 * and right values can have their own size. The only restriction is
 * that they have the same size.
 */
NetNet* PEBinary::elaborate_net_cmp_(Design*des, const string&path,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      NetNet*lsig = left_->elaborate_net(des, path, 0, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, path, 0, 0, 0, 0);
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
	    NetConst*tmp = new NetConst(des->local_symbol(path), verinum::V0);
	    des->add_node(tmp);
	    zero = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    connect(tmp->pin(0), zero->pin(0));
      }

      NetNet*osig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
      osig->local_flag(true);

      NetNode*gate;
	//NetNode*gate_t;

      switch (op_) {
	  case '<':
	  case '>':
	  case 'L':
	  case 'G': {
		NetCompare*cmp = new
		      NetCompare(des->local_symbol(path), dwidth);
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
	      // The comparison generates gates to bitwise compare
	      // each pair, and AND all the comparison results.
	    gate = new NetLogic(des->local_symbol(path),
				1+lsig->pin_count(),
				NetLogic::AND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
		  NetCaseCmp*cmp = new NetCaseCmp(des->local_symbol(path));

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
		  NetNet*tmp = new NetNet(scope, des->local_symbol(path),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(cmp->pin(0), tmp->pin(0));
	    }
	    break;


	  case 'e': // ==
	    gate = new NetLogic(des->local_symbol(path),
				1+dwidth,NetLogic::AND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < dwidth ;  idx += 1) {
		  NetLogic*cmp = new NetLogic(des->local_symbol(path),
					      3, NetLogic::XNOR);
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

		  NetNet*tmp = new NetNet(scope, des->local_symbol(path),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(cmp->pin(0), tmp->pin(0));
	    }
	    break;

	  case 'n': // !=
	    gate = new NetLogic(des->local_symbol(path),
				1+dwidth, NetLogic::OR);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  NetLogic*cmp = new NetLogic(des->local_symbol(path), 3,
					      NetLogic::XOR);
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

		  NetNet*tmp = new NetNet(scope, des->local_symbol(path),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(cmp->pin(0), tmp->pin(0));
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
 * Elaborate a divider gate.
 */
NetNet* PEBinary::elaborate_net_div_(Design*des, const string&path,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);
      NetNet*lsig = left_->elaborate_net(des, path, 0, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, path, 0, 0, 0, 0);
      if (rsig == 0) return 0;

      unsigned rwidth = lsig->pin_count();
      if (rsig->pin_count() > rwidth)
	    rwidth = rsig->pin_count();
      NetDivide*div = new NetDivide(des->local_symbol(path), rwidth,
				    lsig->pin_count(),
				    rsig->pin_count());
      des->add_node(div);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(div->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(div->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, des->local_symbol(path),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(div->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(des->local_symbol(path), verinum::V0);
	    des->add_node(tmp);
	    for (unsigned idx = cnt ;  idx < osig->pin_count() ;  idx += 1)
		  connect(osig->pin(idx), tmp->pin(0));
      }

      return osig;
}

NetNet* PEBinary::elaborate_net_log_(Design*des, const string&path,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lsig = left_->elaborate_net(des, path, 0, 0, 0, 0),
	    *rsig = right_->elaborate_net(des, path, 0, 0, 0, 0);
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
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::AND);
	    break;
	  case 'o':
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::OR);
	    break;
	  default:
	    assert(0);
      }
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);

	// The first OR gate returns 1 if the left value is true...
      if (lsig->pin_count() > 1) {
	    gate_t = new NetLogic(des->local_symbol(path),
				  1+lsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), lsig->pin(idx));

	    connect(gate->pin(1), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetTmp(scope, des->local_symbol(path));
	    connect(gate->pin(1), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(1), lsig->pin(0));
      }

	// The second OR gate returns 1 if the right value is true...
      if (rsig->pin_count() > 1) {
	    gate_t = new NetLogic(des->local_symbol(path),
				  1+rsig->pin_count(), NetLogic::OR);
	    for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		  connect(gate_t->pin(idx+1), rsig->pin(idx));
	    connect(gate->pin(2), gate_t->pin(0));

	      /* The reduced logical value is a new nexus, create a
		 temporary signal to represent it. */
	    NetNet*tmp = new NetTmp(scope, des->local_symbol(path));
	    connect(gate->pin(2), tmp->pin(0));

	    des->add_node(gate_t);

      } else {
	    connect(gate->pin(2), rsig->pin(0));
      }

	// The output is the AND/OR of the two logic values.
      NetNet*osig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
      osig->local_flag(true);
      connect(gate->pin(0), osig->pin(0));
      des->add_node(gate);
      return osig;
}

NetNet* PEBinary::elaborate_net_mul_(Design*des, const string&path,
				       unsigned lwidth,
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lsig = left_->elaborate_net(des, path, 0, 0, 0, 0);
      if (lsig == 0) return 0;
      NetNet*rsig = right_->elaborate_net(des, path, 0, 0, 0, 0);
      if (rsig == 0) return 0;

      unsigned rwidth = lsig->pin_count() + rsig->pin_count();
      NetMult*mult = new NetMult(des->local_symbol(path), rwidth,
				 lsig->pin_count(),
				 rsig->pin_count());
      des->add_node(mult);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataA(idx), lsig->pin(idx));
      for (unsigned idx = 0 ;  idx < rsig->pin_count() ; idx += 1)
	    connect(mult->pin_DataB(idx), rsig->pin(idx));

      if (lwidth == 0) lwidth = rwidth;
      NetNet*osig = new NetNet(scope, des->local_symbol(path),
			       NetNet::IMPLICIT, lwidth);
      osig->local_flag(true);

      unsigned cnt = osig->pin_count();
      if (cnt > rwidth) cnt = rwidth;

      for (unsigned idx = 0 ;  idx < cnt ;  idx += 1)
	    connect(mult->pin_Result(idx), osig->pin(idx));

	/* If the lvalue is larger then the result, then pad the
	   output with constant 0. */
      if (cnt < osig->pin_count()) {
	    NetConst*tmp = new NetConst(des->local_symbol(path), verinum::V0);
	    des->add_node(tmp);
	    for (unsigned idx = cnt ;  idx < osig->pin_count() ;  idx += 1)
		  connect(osig->pin(idx), tmp->pin(0));
      }

      return osig;
}

NetNet* PEBinary::elaborate_net_shift_(Design*des, const string&path,
				       unsigned lwidth,
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lsig = left_->elaborate_net(des, path, lwidth, 0, 0, 0);
      if (lsig == 0) return 0;

      if (lsig->pin_count() > lwidth)
	    lwidth = lsig->pin_count();

	/* Handle the special case of a constant shift amount. There
	   is no reason in this case to create a gate at all, just
	   connect the lsig to the osig with the bit positions
	   shifted. */
      if (verinum*rval = right_->eval_const(des, path)) {
	    assert(rval->is_defined());
	    unsigned dist = rval->as_ulong();
	    if (dist > lsig->pin_count())
		  dist = lsig->pin_count();

	      /* Very special case, constant 0 shift. */
	    if (dist == 0) return lsig;

	    NetNet*osig = new NetNet(scope, des->local_symbol(path),
				     NetNet::WIRE, lsig->pin_count());
	    osig->local_flag(true);

	    NetConst*zero = new NetConst(des->local_symbol(path), verinum::V0);
	    des->add_node(zero);

	    if (op_ == 'l') {
		  unsigned idx;
		  for (idx = 0 ;  idx < dist ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));
		  for (idx = dist ;  idx < lsig->pin_count() ;  idx += 1)
			connect(osig->pin(idx), lsig->pin(idx-dist));

	    } else {
		  assert(op_ == 'r');
		  unsigned idx;
		  unsigned keep = lsig->pin_count()-dist;
		  for (idx = 0 ;  idx < keep ;  idx += 1)
			connect(osig->pin(idx), lsig->pin(idx+dist));
		  for (idx = keep ;  idx < lsig->pin_count() ;  idx += 1)
			connect(osig->pin(idx), zero->pin(0));
	    }

	    return osig;
      }

	// Calculate the number of useful bits for the shift amount,
	// and elaborate the right_ expression as the shift amount.
      unsigned dwid = 0;
      while ((1 << dwid) < lwidth)
	    dwid += 1;

      NetNet*rsig = right_->elaborate_net(des, path, dwid, 0, 0, 0);
      if (rsig == 0) return 0;

	// Make the shift device itself, and the output
	// NetNet. Connect the Result output pins to the osig signal
      NetCLShift*gate = new NetCLShift(des->local_symbol(path),
				       lwidth, rsig->pin_count());

      NetNet*osig = new NetNet(scope, des->local_symbol(path),
			       NetNet::WIRE, lwidth);
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < lwidth ;  idx += 1)
	    connect(osig->pin(idx), gate->pin_Result(idx));

	// Connect the lsig (the left expression) to the Data input,
	// and pad it if necessary with constant zeros.
      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(lsig->pin(idx), gate->pin_Data(idx));

      if (lsig->pin_count() < lwidth) {
	    NetConst*zero = new NetConst(des->local_symbol(path), verinum::V0);
	    NetTmp*tmp = new NetTmp(scope, des->local_symbol(path));
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
	    NetConst*dir = new NetConst(des->local_symbol(path), verinum::V1);
	    connect(dir->pin(0), gate->pin_Direction());
	    des->add_node(dir);
      }

      des->add_node(gate);

      return osig;
}

/*
 * The concatenation operator, as a net, is a wide signal that is
 * connected to all the pins of the elaborated expression nets.
 */
NetNet* PEConcat::elaborate_net(Design*des, const string&path,
				unsigned,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;

      if (repeat_) {
	    verinum*rep = repeat_->eval_const(des, path);
	    if (rep == 0) {
		  cerr << get_line() << ": internal error: Unable to "
		       << "evaluate constant repeat expression." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    unsigned long repeat = rep->as_ulong();

	    assert(parms_.count() == 1);
	    NetNet*obj = parms_[0]->elaborate_net(des, path, 0, rise,
						  fall, decay);
	    NetTmp*tmp = new NetTmp(scope, des->local_symbol(path),
				    repeat * obj->pin_count());

	    for (unsigned idx = 0 ;  idx < repeat ;  idx += 1) {
		  unsigned base = obj->pin_count() * idx;
		  for (unsigned pin = 0 ;  pin < obj->pin_count() ; pin += 1)
			connect(tmp->pin(base+pin), obj->pin(pin));
	    }

	    return tmp;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
	    nets[idx] = parms_[idx]->elaborate_net(des, path, 0,
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
	   which is opposite from how they are given in the list. */
      NetNet*osig = new NetNet(scope, des->local_symbol(path),
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

NetNet* PEIdent::elaborate_net(Design*des, const string&path,
			       unsigned lwidth,
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      NetNet*sig = des->find_signal(path, text_);

      if (sig == 0) {
	      /* If the identifier is a memory instead of a signal,
		 then handle it elsewhere. Create a RAM. */
	    if (NetMemory*mem = des->find_memory(path, text_))
		  return elaborate_net_ram_(des, path, mem, lwidth,
					    rise, fall, decay);


	    if (const NetExpr*pe = des->find_parameter(scope, text_)) {

		  const NetEConst*pc = dynamic_cast<const NetEConst*>(pe);
		  assert(pc);
		  verinum pvalue = pc->value();
		  sig = new NetNet(scope, path+"."+text_, NetNet::IMPLICIT,
				   pc->expr_width());
		  NetConst*cp = new NetConst(des->local_symbol(path), pvalue);
		  des->add_node(cp);
		  for (unsigned idx = 0;  idx <  sig->pin_count(); idx += 1)
			connect(sig->pin(idx), cp->pin(idx));

	    } else {

		  sig = new NetNet(scope, path+"."+text_, NetNet::IMPLICIT, 1);

		  if (warn_implicit)
			cerr << get_line() << ": warning: implicit "
			      " definition of wire " << path << "." <<
			      text_ << "." << endl;
	    }
      }

      assert(sig);

      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const(des, path);
	    if (mval == 0) {
		  cerr << msb_->get_line() << ": error: unable to "
			"evaluate constant expression: " << *msb_ <<
			endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum*lval = lsb_->eval_const(des, path);
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

	    if (midx >= lidx) {
		  NetTmp*tmp = new NetTmp(scope, des->local_symbol(path),
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
		  NetTmp*tmp = new NetTmp(scope, des->local_symbol(path),
					  lidx-midx+1);
		  assert(tmp->pin_count() <= sig->pin_count());
		  for (unsigned idx = lidx ;  idx >= midx ;  idx -= 1)
			connect(tmp->pin(idx-midx), sig->pin(idx));

		  sig = tmp;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, path);
	    if (mval == 0) {
		  cerr << get_line() << ": index of " << text_ <<
			" needs to be constant in this context." <<
			endl;
		  des->errors += 1;
		  return 0;
	    }
	    assert(mval);
	    unsigned idx = sig->sb_to_idx(mval->as_long());
	    if (idx >= sig->pin_count()) {
		  cerr << get_line() << ": index " << sig->name() <<
			"[" << mval->as_long() << "] out of range." << endl;
		  des->errors += 1;
		  idx = 0;
	    }
	    NetTmp*tmp = new NetTmp(scope, des->local_symbol(path), 1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 * When I run into an identifier in an expression that referrs to a
 * memory, create a RAM port object.
 */
NetNet* PEIdent::elaborate_net_ram_(Design*des, const string&path,
				    NetMemory*mem, unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      if (msb_ == 0) {
	    cerr << get_line() << ": error: memory reference without"
		  " the required index expression." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetNet*adr = msb_->elaborate_net(des, path, 0, 0, 0, 0);
      if (adr == 0)
	    return 0;


      NetRamDq*ram = new NetRamDq(des->local_symbol(mem->name()),
				  mem, adr->pin_count());
      des->add_node(ram);

      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(ram->pin_Address(idx), adr->pin(idx));

      NetNet*osig = new NetTmp(scope, des->local_symbol(mem->name()),
			       ram->width());

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(ram->pin_Q(idx), osig->pin(idx));

      return osig;
}

/*
 * Identifiers in continuous assignment l-values are limited to wires
 * and that ilk. Detect registers and memories here and report errors.
 */
NetNet* PEIdent::elaborate_lnet(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*sig = des->find_signal(path, text_);
      if (sig == 0) {
	      /* Don't allow memories here. Is it a memory? */
	    if (des->find_memory(path, text_)) {
		  cerr << get_line() << ": error: memories (" << text_
		       << ") cannot be l-values in continuous "
		       << "assignments." << endl;
		  return 0;
	    }

	      /* Fine, create an implicit wire as an l-value. */
	    sig = new NetNet(scope, path+"."+text_, NetNet::IMPLICIT, 1);

	    if (warn_implicit)
		  cerr << get_line() << ": warning: implicit "
			" definition of wire " << path << "." <<
			text_ << "." << endl;
      }

      assert(sig);

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_line() << ": error: registers (" << sig->name()
		 << ") cannot be l-values in continuous"
		 << " assignments." << endl;
	    return 0;
      }

      if (msb_ && lsb_) {
	      /* Detect a part select. Evaluate the bits and elaborate
		 the l-value by creating a sub-net that links to just
		 the right pins. */ 
	    verinum*mval = msb_->eval_const(des, path);
	    assert(mval);
	    verinum*lval = lsb_->eval_const(des, path);
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

	    if (midx >= lidx) {
		  NetTmp*tmp = new NetTmp(scope, des->local_symbol(path),
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
		  NetTmp*tmp = new NetTmp(scope, des->local_symbol(path),
					  lidx-midx+1);
		  assert(tmp->pin_count() <= sig->pin_count());
		  for (unsigned idx = lidx ;  idx >= midx ;  idx -= 1)
			connect(tmp->pin(idx-midx), sig->pin(idx));

		  sig = tmp;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const(des, path);
	    if (mval == 0) {
		  cerr << get_line() << ": index of " << text_ <<
			" needs to be constant in this context." <<
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
	    NetTmp*tmp = new NetTmp(scope, des->local_symbol(path), 1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 * Elaborate a number as a NetConst object.
 */
NetNet* PENumber::elaborate_net(Design*des, const string&path,
				unsigned lwidth,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* If we are constrained by a l-value size, then just make a
	   number constant with the correct size and set as many bits
	   in that constant as make sense. Pad excess with zeros. */
      if (lwidth > 0) {
	    NetNet*net = new NetNet(scope, des->local_symbol(path),
				    NetNet::IMPLICIT, lwidth);
	    net->local_flag(true);

	    verinum num(verinum::V0, net->pin_count());
	    unsigned idx;
	    for (idx = 0 ;  idx < num.len() && idx < value_->len(); idx += 1)
		  num.set(idx, value_->get(idx));

	    NetConst*tmp = new NetConst(des->local_symbol(path), num);
	    for (idx = 0 ;  idx < net->pin_count() ;  idx += 1)
		  connect(net->pin(idx), tmp->pin(idx));

	    des->add_node(tmp);
	    return net;
      }

	/* If the number has a length, then use that to size the
	   number. Generate a constant object of exactly the user
	   specified size. */
      if (value_->has_len()) {
	    NetNet*net = new NetNet(scope, des->local_symbol(path),
				    NetNet::IMPLICIT, value_->len());
	    net->local_flag(true);
	    NetConst*tmp = new NetConst(des->local_symbol(path), *value_);
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

      NetNet*net = new NetNet(scope, des->local_symbol(path),
			      NetNet::IMPLICIT, width);
      net->local_flag(true);
      NetConst*tmp = new NetConst(des->local_symbol(path), num);
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    connect(net->pin(idx), tmp->pin(idx));

      des->add_node(tmp);
      return net;
}


/*
 * Elaborate the ternary operator in a netlist by creating a LPM_MUX
 * with width matching the result, size == 2 and 1 select input.
 */
NetNet* PETernary::elaborate_net(Design*des, const string&path,
				 unsigned width,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet* expr_sig = expr_->elaborate_net(des, path, 0, 0, 0, 0);
      NetNet* tru_sig = tru_->elaborate_net(des, path, width, 0, 0, 0);
      NetNet* fal_sig = fal_->elaborate_net(des, path, width, 0, 0, 0);
      if (expr_sig == 0 || tru_sig == 0 || fal_sig == 0) {
	    des->errors += 1;
	    return 0;
      }

      assert(tru_sig->pin_count() == fal_sig->pin_count());
      assert(width == tru_sig->pin_count());
      assert(expr_sig->pin_count() == 1);

      NetNet*sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE,
		       tru_sig->pin_count());
      sig->local_flag(true);

      NetMux*mux = new NetMux(des->local_symbol(path), width, 2, 1);
      connect(mux->pin_Sel(0), expr_sig->pin(0));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
	    connect(mux->pin_Result(idx), sig->pin(idx));
	    connect(mux->pin_Data(idx,0), fal_sig->pin(idx));
	    connect(mux->pin_Data(idx,1), tru_sig->pin(idx));
      }

      des->add_node(mux);

      return sig;
}

NetNet* PEUnary::elaborate_net(Design*des, const string&path,
			       unsigned width,
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet* sub_sig = expr_->elaborate_net(des, path,
					     op_=='~'?width:0,
					     0, 0, 0);
      if (sub_sig == 0) {
	    des->errors += 1;
	    return 0;
      }
      assert(sub_sig);

      NetNet* sig;
      NetLogic*gate;
      switch (op_) {
	  case '~': // Bitwise NOT
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE,
			     sub_sig->pin_count());
	    sig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(des->local_symbol(path), 2,
				      NetLogic::NOT);
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
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::NOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '&': // Reduction AND
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::AND);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '|': // Reduction OR
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::OR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '^': // Reduction XOR
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::XOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case 'A': // Reduction NAND (~&)
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::NAND);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;


	  case 'X': // Reduction XNOR (~^)
	    sig = new NetNet(scope, des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::XNOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  default:
	    cerr << "internal error: Unhandled UNARY '" << op_ << "'" << endl;
	    sig = 0;
      }

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(sub_sig))
	    delete tmp;

      return sig;
}

/*
 * $Log: elab_net.cc,v $
 * Revision 1.31  2000/05/02 00:58:11  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.30  2000/04/28 21:00:29  steve
 *  Over agressive signal elimination in constant probadation.
 *
 * Revision 1.29  2000/04/01 21:40:22  steve
 *  Add support for integer division.
 *
 * Revision 1.28  2000/03/27 04:38:15  steve
 *  Speling error.
 *
 * Revision 1.27  2000/03/20 17:54:10  steve
 *  Remove dangerous tmp signal delete.
 *
 * Revision 1.26  2000/03/17 21:50:25  steve
 *  Switch to control warnings.
 *
 * Revision 1.25  2000/03/16 19:03:03  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.24  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.23  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.22  2000/02/16 03:58:27  steve
 *  Fix up width matching in structural bitwise operators.
 *
 * Revision 1.21  2000/02/14 06:04:52  steve
 *  Unary reduction operators do not set their operand width
 *
 * Revision 1.20  2000/01/18 04:53:40  steve
 *  Support structural XNOR.
 *
 * Revision 1.19  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.18  2000/01/11 04:20:57  steve
 *  Elaborate net widths of constants to as small
 *  as is possible, obeying context constraints.
 *
 *  Comparison operators can handle operands with
 *  different widths.
 *
 * Revision 1.17  2000/01/02 22:07:09  steve
 *  Add a signal to nexus of padding constant.
 *
 * Revision 1.16  2000/01/02 21:45:31  steve
 *  Add structural reduction NAND,
 *  Fix size coercion of structural shifts.
 *
 * Revision 1.15  2000/01/02 19:39:03  steve
 *  Structural reduction XNOR.
 *
 * Revision 1.14  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 *
 * Revision 1.13  1999/12/16 03:46:39  steve
 *  Structural logical or.
 *
 * Revision 1.12  1999/12/16 02:42:14  steve
 *  Simulate carry output on adders.
 *
 * Revision 1.11  1999/12/02 04:08:10  steve
 *  Elaborate net repeat concatenations.
 *
 * Revision 1.10  1999/11/30 04:33:41  steve
 *  Put implicitly defined signals in the scope.
 *
 * Revision 1.9  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.8  1999/11/21 17:35:37  steve
 *  Memory name lookup handles scopes.
 *
 * Revision 1.7  1999/11/21 00:13:08  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.6  1999/11/14 23:43:45  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.5  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.4  1999/11/05 23:36:31  steve
 *  Forgot to return the mux for use after elaboration.
 *
 * Revision 1.3  1999/11/05 21:45:19  steve
 *  Fix NetConst being set to zero width, and clean
 *  up elaborate_set_cmp_ for NetEBinary.
 *
 * Revision 1.2  1999/11/04 03:53:26  steve
 *  Patch to synthesize unary ~ and the ternary operator.
 *  Thanks to Larry Doolittle <LRDoolittle@lbl.gov>.
 *
 *  Add the LPM_MUX device, and integrate it with the
 *  ternary synthesis from Larry. Replace the lpm_mux
 *  generator in t-xnf.cc to use XNF EQU devices to
 *  put muxs into function units.
 *
 *  Rewrite elaborate_net for the PETernary class to
 *  also use the LPM_MUX device.
 *
 * Revision 1.1  1999/10/31 20:08:24  steve
 *  Include subtraction in LPM_ADD_SUB device.
 *
 */

