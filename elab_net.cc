/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: elab_net.cc,v 1.12 1999/12/16 02:42:14 steve Exp $"
#endif

# include  "PExpr.h"
# include  "netlist.h"

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
	  case '+':
	  case '-':
	    return elaborate_net_add_(des, path, width, rise, fall, decay);
	  case 'E':
	  case 'e':
	  case 'n':
	  case '<':
	  case '>':
	  case 'L': // <=
	  case 'G': // >=
	    return elaborate_net_cmp_(des, path, width, rise, fall, decay);
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
	    assert(lsig->pin_count() == rsig->pin_count());
	    osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
			      lsig->pin_count());
	    osig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::XOR);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    des->add_signal(osig);
	    break;

	  case '&': // AND
	    assert(lsig->pin_count() == rsig->pin_count());
	    osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
			      lsig->pin_count());
	    osig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::AND);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    des->add_signal(osig);
	    break;

	  case '|': // Bitwise OR
	    assert(lsig->pin_count() == rsig->pin_count());
	    osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
			      lsig->pin_count());
	    osig->local_flag(true);
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate = new NetLogic(des->local_symbol(path), 3,
				      NetLogic::OR);
		  connect(gate->pin(1), lsig->pin(idx));
		  connect(gate->pin(2), rsig->pin(idx));
		  connect(gate->pin(0), osig->pin(idx));
		  gate->rise_time(rise);
		  gate->fall_time(fall);
		  gate->decay_time(decay);
		  des->add_node(gate);
	    }
	    des->add_signal(osig);
	    break;

	  case 'a': // && (logical AND)
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::AND);

	      // The first OR gate returns 1 if the left value is true...
	    if (lsig->pin_count() > 1) {
		  gate_t = new NetLogic(des->local_symbol(path),
					1+lsig->pin_count(), NetLogic::OR);
		  for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
			connect(gate_t->pin(idx+1), lsig->pin(idx));
		  connect(gate->pin(1), gate_t->pin(0));
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
		  des->add_node(gate_t);
	    } else {
		  connect(gate->pin(2), rsig->pin(0));
	    }

	      // The output is the AND of the two logic values.
	    osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    des->add_node(gate);
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

	// Make the adder as wide as the widest operand
      osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE, owidth);
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
      des->add_signal(osig);
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

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(lsig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(rsig))
	    delete tmp;

      return osig;
}

/*
 * Elaborate the various binary comparison operators.
 */
NetNet* PEBinary::elaborate_net_cmp_(Design*des, const string&path,
				     unsigned lwidth,
				     unsigned long rise,
				     unsigned long fall,
				     unsigned long decay) const
{
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

      if (lsig->pin_count() != rsig->pin_count()) {
	    cerr << get_line() << ": internal error: Cannot match "
		  "structural net widths " << lsig->pin_count() <<
		  " and " << rsig->pin_count() << "." << endl;
	    delete lsig;
	    delete rsig;
	    des->errors += 1;
	    return 0;
      }

      NetNet*osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE);
      osig->local_flag(true);

      NetNode*gate;
      NetNode*gate_t;

      switch (op_) {
	  case '<':
	  case '>':
	  case 'L':
	  case 'G': {
		NetCompare*cmp = new NetCompare(des->local_symbol(path),
						lsig->pin_count());
		for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		      connect(cmp->pin_DataA(idx), lsig->pin(idx));
		      connect(cmp->pin_DataB(idx), rsig->pin(idx));
		}
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
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate_t = new NetCaseCmp(des->local_symbol(path));
		  connect(gate_t->pin(1), lsig->pin(idx));
		  connect(gate_t->pin(2), rsig->pin(idx));
		  connect(gate_t->pin(0), gate->pin(idx+1));
		  des->add_node(gate_t);

		    // Attach a label to this intermediate wire
		  NetNet*tmp = new NetNet(0, des->local_symbol(path),
					  NetNet::WIRE);
		  tmp->local_flag(true);
		  connect(gate_t->pin(0), tmp->pin(0));
		  des->add_signal(tmp);
	    }
	    break;


	  case 'e': // ==
	    gate = new NetLogic(des->local_symbol(path),
				1+lsig->pin_count(),
				NetLogic::AND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate_t = new NetLogic(des->local_symbol(path), 3,
				        NetLogic::XNOR);
		  connect(gate_t->pin(1), lsig->pin(idx));
		  connect(gate_t->pin(2), rsig->pin(idx));
		  connect(gate_t->pin(0), gate->pin(idx+1));
		  des->add_node(gate_t);
	    }
	    break;

	  case 'n': // !=
	    gate = new NetLogic(des->local_symbol(path),
				1+lsig->pin_count(),
				NetLogic::OR);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate_t = new NetLogic(des->local_symbol(path), 3,
				        NetLogic::XOR);
		  connect(gate_t->pin(1), lsig->pin(idx));
		  connect(gate_t->pin(2), rsig->pin(idx));
		  connect(gate_t->pin(0), gate->pin(idx+1));
		  des->add_node(gate_t);
	    }
	    break;

	  default:
	    assert(0);
      }

      des->add_signal(osig);
      gate->rise_time(rise);
      gate->fall_time(fall);
      gate->decay_time(decay);
      des->add_node(gate);

      return osig;
}

NetNet* PEBinary::elaborate_net_shift_(Design*des, const string&path,
				       unsigned lwidth,
				       unsigned long rise,
				       unsigned long fall,
				       unsigned long decay) const
{
      NetNet*lsig = left_->elaborate_net(des, path, lwidth, 0, 0, 0);
      if (lsig == 0) return 0;

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

	    NetNet*osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
				     lsig->pin_count());
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

	    des->add_signal(osig);
	    return osig;
      }

      unsigned dwid = 0;
      while ((1 << dwid) < lsig->pin_count())
	    dwid += 1;

      NetNet*rsig = right_->elaborate_net(des, path, dwid, 0, 0, 0);
      if (rsig == 0) return 0;

      NetCLShift*gate = new NetCLShift(des->local_symbol(path),
				       lsig->pin_count(),
				       rsig->pin_count());

      NetNet*osig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
			       lsig->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    connect(osig->pin(idx), gate->pin_Result(idx));
	    connect(lsig->pin(idx), gate->pin_Data(idx));
      }

      for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
	    connect(rsig->pin(idx), gate->pin_Distance(idx));

      if (op_ == 'r') {
	    NetConst*dir = new NetConst(des->local_symbol(path), verinum::V1);
	    connect(dir->pin(0), gate->pin_Direction());
	    des->add_node(dir);
      }

      des->add_signal(osig);
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
	    NetTmp*tmp = new NetTmp(des->local_symbol(path),
				    repeat * obj->pin_count());

	    for (unsigned idx = 0 ;  idx < repeat ;  idx += 1) {
		  unsigned base = obj->pin_count() * idx;
		  for (unsigned pin = 0 ;  pin < obj->pin_count() ; pin += 1)
			connect(tmp->pin(base+pin), obj->pin(pin));
	    }

	    des->add_signal(tmp);
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
      NetNet*osig = new NetNet(0, des->local_symbol(path),
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
      des->add_signal(osig);
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


	    if (const NetExpr*pe = des->find_parameter(path, text_)) {

		  const NetEConst*pc = dynamic_cast<const NetEConst*>(pe);
		  assert(pc);
		  verinum pvalue = pc->value();
		  sig = new NetNet(0, path+"."+text_, NetNet::IMPLICIT,
				   pc->expr_width());
		  for (unsigned idx = 0;  idx <  sig->pin_count(); idx += 1) {
			NetConst*cp = new NetConst(des->local_symbol(path),
						   pvalue[idx]);
			connect(sig->pin(idx), cp->pin(0));
			des->add_node(cp);
		  }

	    } else {

		  sig = new NetNet(scope, path+"."+text_, NetNet::IMPLICIT, 1);
		  des->add_signal(sig);
		  cerr << get_line() << ": warning: Implicitly defining "
			"wire " << path << "." << text_ << "." << endl;
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
		  NetTmp*tmp = new NetTmp(des->local_symbol(path),
					  midx-lidx+1);
		  des->add_signal(tmp);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(des->local_symbol(path),
					  lidx-midx+1);
		  des->add_signal(tmp);
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
	    NetTmp*tmp = new NetTmp(des->local_symbol(path), 1);
	    des->add_signal(tmp);
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

      NetNet*osig = new NetTmp(des->local_symbol(mem->name()), ram->width());
      des->add_signal(osig);

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
	    sig = new NetNet(0, path+"."+text_, NetNet::IMPLICIT, 1);
	    des->add_signal(sig);
	    cerr << get_line() << ": warning: Implicitly defining "
		  "wire " << path << "." << text_ << "." << endl;
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
		  NetTmp*tmp = new NetTmp(des->local_symbol(path),
					  midx-lidx+1);
		  des->add_signal(tmp);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(des->local_symbol(path),
					  lidx-midx+1);
		  des->add_signal(tmp);
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
	    NetTmp*tmp = new NetTmp(des->local_symbol(path), 1);
	    des->add_signal(tmp);
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
      unsigned width = value_->len();
      if ((lwidth > 0) && (lwidth < width))
	    width = lwidth;

      NetNet*net = new NetNet(0, des->local_symbol(path),
			      NetNet::IMPLICIT, width);
      net->local_flag(true);
      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
	    NetConst*tmp = new NetConst(des->local_symbol(path),
					value_->get(idx));
	    des->add_node(tmp);
	    connect(net->pin(idx), tmp->pin(0));
      }

      des->add_signal(net);
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

      NetNet*sig = new NetNet(0, des->local_symbol(path), NetNet::WIRE,
		       tru_sig->pin_count());
      sig->local_flag(true);

      NetMux*mux = new NetMux(des->local_symbol(path), width, 2, 1);
      connect(mux->pin_Sel(0), expr_sig->pin(0));

      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
	    connect(mux->pin_Result(idx), sig->pin(idx));
	    connect(mux->pin_Data(idx,0), fal_sig->pin(idx));
	    connect(mux->pin_Data(idx,1), tru_sig->pin(idx));
      }

      des->add_signal(sig);
      des->add_node(mux);

      return sig;
}

/*
 * $Log: elab_net.cc,v $
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

