/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elaborate.cc,v 1.102 1999/09/29 18:36:03 steve Exp $"
#endif

/*
 * Elaboration takes as input a complete parse tree and the name of a
 * root module, and generates as output the elaborated design. This
 * elaborated design is presented as a Module, which does not
 * reference any other modules. It is entirely self contained.
 */

# include  <typeinfo>
# include  <strstream>
# include  "pform.h"
# include  "netlist.h"
# include  "netmisc.h"

string Design::local_symbol(const string&path)
{
      strstream res;
      res << "_L" << (lcounter_++) << ends;
      return path + "." + res.str();
}

static void do_assign(Design*des, const string&path,
		      NetNet*lval, NetNet*rval)
{
      assert(lval->pin_count() == rval->pin_count());
      const unsigned pin_count = lval->pin_count();

      if (NetTmp* tmp = dynamic_cast<NetTmp*>(rval)) {

	    for (unsigned idx = 0 ;  idx < pin_count ;  idx += 1)
		  connect(lval->pin(idx), tmp->pin(idx));
	    delete tmp;

	    if ((tmp = dynamic_cast<NetTmp*>(lval)))
		  delete tmp;

      } else if (NetTmp* tmp = dynamic_cast<NetTmp*>(lval)) {

	    for (unsigned idx = 0 ;  idx < pin_count ;  idx += 1)
		  connect(tmp->pin(idx), rval->pin(idx));
	    delete tmp;

      } else if (rval->local_flag()) {

	    for (unsigned idx = 0 ;  idx < pin_count ;  idx += 1)
		  connect(lval->pin(idx), rval->pin(idx));
	    delete rval;

      } else if (lval->local_flag()) {

	    for (unsigned idx = 0 ;  idx < pin_count ;  idx += 1)
		  connect(lval->pin(idx), rval->pin(idx));
	    delete lval;

      } else for (unsigned idx = 0 ;  idx < pin_count ;  idx += 1) {
	    NetBUFZ*cur = new NetBUFZ(des->local_symbol(path));

	    connect(cur->pin(0), lval->pin(idx));
	    connect(cur->pin(1), rval->pin(idx));

	    des->add_node(cur);
      }
}


  // Urff, I don't like this global variable. I *will* figure out a
  // way to get rid of it. But, for now the PGModule::elaborate method
  // needs it to find the module definition.
static const map<string,Module*>* modlist = 0;
static const map<string,PUdp*>*   udplist = 0;

/*
 * Elaborate a source wire. The "wire" is the declaration of wires,
 * registers, ports and memories. The parser has already merged the
 * multiple properties of a wire (i.e. "input wire") so come the
 * elaboration this creates an object in the design that represent the
 * defined item.
 */
void PWire::elaborate(Design*des, const string&path) const
{
      NetNet::Type wtype = type_;
      if (wtype == NetNet::IMPLICIT)
	    wtype = NetNet::WIRE;
      if (wtype == NetNet::IMPLICIT_REG)
	    wtype = NetNet::REG;

      unsigned wid = 1;
      long lsb = 0, msb = 0;

      if (msb_.count()) {
	    svector<long>mnum (msb_.count());
	    svector<long>lnum (msb_.count());

	      /* There may be multiple declarations of ranges, because
		 the symbol may have its range declared in i.e. input
		 and reg declarations. Calculate *all* the numbers
		 here. I will resolve the values later. */

	    for (unsigned idx = 0 ;  idx < msb_.count() ;  idx += 1) {
		  verinum*mval = msb_[idx]->eval_const(des,path);
		  if (mval == 0) {
			cerr << msb_[idx]->get_line() << ": Unable to "
			      "evaluate constant expression ``" <<
			      *msb_[idx] << "''." << endl;
			des->errors += 1;
			return;
		  }
		  verinum*lval = lsb_[idx]->eval_const(des, path);
		  if (mval == 0) {
			cerr << lsb_[idx]->get_line() << ": Unable to "
			      "evaluate constant expression ``" <<
			      *lsb_[idx] << "''." << endl;
			des->errors += 1;
			return;
		  }

		  mnum[idx] = mval->as_long();
		  lnum[idx] = lval->as_long();
		  delete mval;
		  delete lval;
	    }

	      /* Make sure all the values for msb and lsb match by
		 value. If not, report an error. */
	    for (unsigned idx = 1 ;  idx < msb_.count() ;  idx += 1) {
		  if ((mnum[idx] != mnum[0]) || (lnum[idx] != lnum[0])) {
			cerr << get_line() << ": Inconsistent width, "
			      "[" << mnum[idx] << ":" << lnum[idx] << "]"
			      " vs. [" << mnum[0] << ":" << lnum[0] << "]"
			      " for signal ``" << name_ << "''" << endl;
			des->errors += 1;
			return;
		  }
	    }

	    lsb = lnum[0];
	    msb = mnum[0];
	    if (mnum[0] > lnum[0])
		  wid = mnum[0] - lnum[0] + 1;
	    else
		  wid = lnum[0] - mnum[0] + 1;


      }

      if (lidx_ || ridx_) {
	    assert(lidx_ && ridx_);

	      // If the register has indices, then this is a
	      // memory. Create the memory object.
	    verinum*lval = lidx_->eval_const(des, path);
	    assert(lval);
	    verinum*rval = ridx_->eval_const(des, path);
	    assert(rval);

	    long lnum = lval->as_long();
	    long rnum = rval->as_long();
	    delete lval;
	    delete rval;
	    NetMemory*sig = new NetMemory(path+"."+name_, wid, lnum, rnum);
	    sig->set_attributes(attributes);
	    des->add_memory(sig);

      } else {

	    NetNet*sig = new NetNet(path + "." + name_, wtype, msb, lsb);
	    sig->set_line(*this);
	    sig->port_type(port_type_);
	    sig->set_attributes(attributes);

	    verinum::V iv = verinum::Vz;
	    if (wtype == NetNet::REG)
		  iv = verinum::Vx;

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  sig->set_ival(idx, iv);

	    des->add_signal(sig);
      }
}

void PGate::elaborate(Design*des, const string&path) const
{
      cerr << "what kind of gate? " << typeid(*this).name() << endl;
}

/*
 * Elaborate the continuous assign. (This is *not* the procedural
 * assign.) Elaborate the lvalue and rvalue, and do the assignment.
 */
void PGAssign::elaborate(Design*des, const string&path) const
{
      unsigned long rise_time, fall_time, decay_time;
      eval_delays(des, path, rise_time, fall_time, decay_time);

      assert(pin(0));
      assert(pin(1));

	/* Elaborate the l-value. */
      NetNet*lval = pin(0)->elaborate_lnet(des, path);
      if (lval == 0) {
	    des->errors += 1;
	    return;
      }


	/* Elaborate the r-value. Account for the initial decays,
	   which are going to be attached to the last gate before the
	   generated NetNet. */
      NetNet*rval = pin(1)->elaborate_net(des, path, rise_time,
					  fall_time, decay_time);
      if (rval == 0) {
	    cerr << get_line() << ": Unable to elaborate r-value: " <<
		  *pin(1) << endl;
	    des->errors += 1;
	    return;
      }

      assert(lval && rval);

      if (lval->pin_count() != rval->pin_count()) {
	    cerr << get_line() << ": lval width (" <<
		  lval->pin_count() << ") != rval width (" <<
		  rval->pin_count() << ")." << endl;
	    delete lval;
	    delete rval;
	    des->errors += 1;
	    return;
      }

      do_assign(des, path, lval, rval);

}

/*
 * Elaborate a Builtin gate. These normally get translated into
 * NetLogic nodes that reflect the particular logic function.
 */
void PGBuiltin::elaborate(Design*des, const string&path) const
{
      unsigned count = 1;
      unsigned low = 0, high = 0;
      string name = get_name();
      if (name == "")
	    name = des->local_symbol(path);

	/* If the verilog source has a range specification for the
	   gates, then I am expected to make more then one
	   gate. Figure out how many are desired. */
      if (msb_) {
	    verinum*msb = msb_->eval_const(des, path);
	    verinum*lsb = lsb_->eval_const(des, path);

	    if (msb == 0) {
		  cerr << get_line() << ": Unable to evaluate expression "
		       << *msb_ << endl;
		  des->errors += 1;
		  return;
	    }

	    if (lsb == 0) {
		  cerr << get_line() << ": Unable to evaluate expression "
		       << *lsb_ << endl;
		  des->errors += 1;
		  return;
	    }

	    if (msb->as_long() > lsb->as_long())
		  count = msb->as_long() - lsb->as_long() + 1;
	    else
		  count = lsb->as_long() - msb->as_long() + 1;

	    low = lsb->as_long();
	    high = msb->as_long();
      }


	/* Allocate all the getlist nodes for the gates. */
      NetLogic**cur = new NetLogic*[count];
      assert(cur);

	/* Calculate the gate delays from the delay expressions
	   given in the source. For logic gates, the decay time
	   is meaningless because it can never go to high
	   impedence. However, the bufif devices can generate
	   'bz output, so we will pretend that anything can.

	   If only one delay value expression is given (i.e. #5
	   nand(foo,...)) then rise, fall and decay times are
	   all the same value. If two values are given, rise and
	   fall times are use, and the decay time is the minimum
	   of the rise and fall times. Finally, if all three
	   values are given, they are taken as specified. */

      unsigned long rise_time, fall_time, decay_time;
      eval_delays(des, path, rise_time, fall_time, decay_time);

	/* Now make as many gates as the bit count dictates. Give each
	   a unique name, and set the delay times. */

      for (unsigned idx = 0 ;  idx < count ;  idx += 1) {
	    strstream tmp;
	    unsigned index;
	    if (low < high)
		  index = low + idx;
	    else
		  index = low - idx;

	    tmp << name << "<" << index << ">";
	    const string inm = tmp.str();

	    switch (type()) {
		case AND:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::AND);
		  break;
		case BUF:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::BUF);
		  break;
		case BUFIF0:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::BUFIF0);
		  break;
		case BUFIF1:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::BUFIF1);
		  break;
		case NAND:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::NAND);
		  break;
		case NOR:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::NOR);
		  break;
		case NOT:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::NOT);
		  break;
		case OR:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::OR);
		  break;
		case XNOR:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::XNOR);
		  break;
		case XOR:
		  cur[idx] = new NetLogic(inm, pin_count(), NetLogic::XOR);
		  break;
	    }


	    cur[idx]->rise_time(rise_time);
	    cur[idx]->fall_time(fall_time);
	    cur[idx]->decay_time(decay_time);

	    des->add_node(cur[idx]);
      }

	/* The gates have all been allocated, this loop runs through
	   the parameters and attaches the ports of the objects. */

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    const PExpr*ex = pin(idx);
	    NetNet*sig = ex->elaborate_net(des, path);
	    if (sig == 0)
		  continue;

	    assert(sig);

	    if (sig->pin_count() == 1)
		  for (unsigned gdx = 0 ;  gdx < count ;  gdx += 1)
			connect(cur[gdx]->pin(idx), sig->pin(0));

	    else if (sig->pin_count() == count)
		  for (unsigned gdx = 0 ;  gdx < count ;  gdx += 1)
			connect(cur[gdx]->pin(idx), sig->pin(gdx));

	    else {
		  cerr << get_line() << ": Gate count of " << count <<
			" does not match net width of " <<
			sig->pin_count() << " at pin " << idx << "."
		       << endl;
		  des->errors += 1;
	    }

	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig))
		  delete tmp;
      }
}

/*
 * Instantiate a module by recursively elaborating it. Set the path of
 * the recursive elaboration so that signal names get properly
 * set. Connect the ports of the instantiated module to the signals of
 * the parameters. This is done with BUFZ gates so that they look just
 * like continuous assignment connections.
 */
void PGModule::elaborate_mod_(Design*des, Module*rmod, const string&path) const
{
      assert(get_name() != "");
      const string my_name = path + "." + get_name();

      const svector<PExpr*>*pins;

	// Detect binding by name. If I am binding by name, then make
	// up a pins array that reflects the positions of the named
	// ports. If this is simply positional binding in the first
	// place, then get the binding from the base class.
      if (pins_) {
	    unsigned nexp = rmod->port_count();
	    svector<PExpr*>*exp = new svector<PExpr*>(nexp);

	      // Scan the bindings, matching them with port names.
	    for (unsigned idx = 0 ;  idx < npins_ ;  idx += 1) {

		    // Given a binding, look at the module port names
		    // for the position that matches the binding name.
		  unsigned pidx = rmod->find_port(pins_[idx].name);

		    // If the port name doesn't exist, the find_port
		    // method will return the port count. Detect that
		    // as an error.
		  if (pidx == nexp) {
			cerr << get_line() << ": port ``" <<
			      pins_[idx].name << "'' is not a port of "
			     << get_name() << "." << endl;
			des->errors += 1;
			continue;
		  }

		    // If I already bound something to this port, then
		    // the (*exp) array will already have a pointer
		    // value where I want to place this expression.
		  if ((*exp)[pidx]) {
			cerr << get_line() << ": port ``" <<
			      pins_[idx].name << "'' already bound." <<
			      endl;
			des->errors += 1;
			continue;
		  }

		    // OK, do the binding by placing the expression in
		    // the right place.
		  (*exp)[pidx] = pins_[idx].parm;
	    }

	    pins = exp;

      } else {

	    if (pin_count() != rmod->port_count()) {
		  cerr << get_line() << ": Wrong number "
			"of parameters. Expecting " << rmod->port_count() <<
			", got " << pin_count() << "."
		       << endl;
		  des->errors += 1;
		  return;
	    }

	      // No named bindings, just use the positional list I
	      // already have.
	    assert(pin_count() == rmod->port_count());
	    pins = get_pins();
      }

	// Elaborate this instance of the module. The recursive
	// elaboration causes the module to generate a netlist with
	// the ports represented by NetNet objects. I will find them
	// later.
      rmod->elaborate(des, my_name, overrides_);

	// Now connect the ports of the newly elaborated designs to
	// the expressions that are the instantiation parameters. Scan
	// the pins, elaborate the expressions attached to them, and
	// bind them to the port of the elaborated module.

      for (unsigned idx = 0 ;  idx < pins->count() ;  idx += 1) {
	      // Skip unconnected module ports.
	    if ((*pins)[idx] == 0)
		  continue;
	    NetNet*sig = (*pins)[idx]->elaborate_net(des, path);
	    if (sig == 0) {
		  cerr << "Expression too complicated for elaboration." << endl;
		  continue;
	    }

	    assert(sig);

	      // Inside the module, the port is one or more signals,
	      // that were already elaborated. List all those signals,
	      // and I will connect them up later.
	    svector<PWire*> mport = rmod->get_port(idx);
	    svector<NetNet*>prts (mport.count());

	    unsigned prts_pin_count = 0;
	    for (unsigned ldx = 0 ;  ldx < mport.count() ;  ldx += 1) {
		  PWire*pport = mport[ldx];
		  prts[ldx] = des->find_signal(my_name, pport->name());
		  assert(prts[ldx]);
		  prts_pin_count += prts[ldx]->pin_count();
	    }

	      // Check that the parts have matching pin counts. If
	      // not, they are different widths.
	    if (prts_pin_count != sig->pin_count()) {
		  cerr << get_line() << ": Port " << idx << " of " << type_ <<
			" expects " << prts_pin_count << " pins, got " <<
			sig->pin_count() << " from " << sig->name() << endl;
		  des->errors += 1;
		  continue;
	    }

	      // Connect the sig expression that is the context of the
	      // module instance to the ports of the elaborated
	      // module.

	    assert(prts_pin_count == sig->pin_count());
	    for (unsigned ldx = 0 ;  ldx < prts.count() ;  ldx += 1) {
		  for (unsigned p = 0 ;  p < prts[ldx]->pin_count() ; p += 1) {
			prts_pin_count -= 1;
			connect(sig->pin(prts_pin_count),
				prts[ldx]->pin(prts[ldx]->pin_count()-p-1));
		  }
	    }

	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig))
		  delete tmp;
      }
}

/*
 * From a UDP definition in the source, make a NetUDP
 * object. Elaborate the pin expressions as netlists, then connect
 * those networks to the pins.
 */
void PGModule::elaborate_udp_(Design*des, PUdp*udp, const string&path) const
{
      const string my_name = path+"."+get_name();
      NetUDP*net = new NetUDP(my_name, udp->ports.count(), udp->sequential);
      net->set_attributes(udp->attributes);

	/* Run through the pins, making netlists for the pin
	   expressions and connecting them to the pin in question. All
	   of this is independent of the nature of the UDP. */
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    if (pin(idx) == 0)
		  continue;

	    NetNet*sig = pin(idx)->elaborate_net(des, path);
	    if (sig == 0) {
		  cerr << "Expression too complicated for elaboration:"
		       << *pin(idx) << endl;
		  continue;
	    }

	    connect(sig->pin(0), net->pin(idx));

	      // Delete excess holding signal.
	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig))
		  delete tmp;
      }

	/* Build up the truth table for the netlist from the input
	   strings. */
      for (unsigned idx = 0 ;  idx < udp->tinput.count() ;  idx += 1) {
	    string input = udp->sequential
		  ? (string("") + udp->tcurrent[idx] + udp->tinput[idx])
		  : udp->tinput[idx];

	    net->set_table(input, udp->toutput[idx]);
      }

      net->cleanup_table();

      if (udp->sequential) switch (udp->initial) {
	  case verinum::V0:
	    net->set_initial('0');
	    break;
	  case verinum::V1:
	    net->set_initial('1');
	    break;
	  case verinum::Vx:
	  case verinum::Vz:
	    net->set_initial('x');
	    break;
      }

	// All done. Add the object to the design.
      des->add_node(net);
}

void PGModule::elaborate(Design*des, const string&path) const
{
	// Look for the module type
      map<string,Module*>::const_iterator mod = modlist->find(type_);
      if (mod != modlist->end()) {
	    elaborate_mod_(des, (*mod).second, path);
	    return;
      }

	// Try a primitive type
      map<string,PUdp*>::const_iterator udp = udplist->find(type_);
      if (udp != udplist->end()) {
	    elaborate_udp_(des, (*udp).second, path);
	    return;
      }

      cerr << get_line() << ": Unknown module: " << type_ << endl;
}

/*
 * Elaborating binary operations generally involves elaborating the
 * left and right expressions, then making an output wire and
 * connecting the lot together with the right kind of gate.
 */
NetNet* PEBinary::elaborate_net(Design*des, const string&path,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      NetNet*lsig = left_->elaborate_net(des, path),
	    *rsig = right_->elaborate_net(des, path);
      if (lsig == 0) {
	    cerr << get_line() << ": Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << get_line() << ": Cannot elaborate ";
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
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE,
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
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE,
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
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE,
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

	  case 'e': // ==
	    assert(lsig->pin_count() == rsig->pin_count());
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+lsig->pin_count(),
				NetLogic::AND);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate_t = new NetLogic(des->local_symbol(path), 3,
				        NetLogic::XNOR);
		  connect(gate_t->pin(1), lsig->pin(idx));
		  connect(gate_t->pin(2), rsig->pin(idx));
		  connect(gate_t->pin(0), gate->pin(idx));
		  des->add_node(gate_t);
	    }
	    des->add_signal(osig);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    des->add_node(gate);
	    break;

	  case 'n': // !=
	    assert(lsig->pin_count() == rsig->pin_count());
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+lsig->pin_count(),
				NetLogic::OR);
	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1) {
		  gate_t = new NetLogic(des->local_symbol(path), 3,
				        NetLogic::XOR);
		  connect(gate_t->pin(1), lsig->pin(idx));
		  connect(gate_t->pin(2), rsig->pin(idx));
		  connect(gate_t->pin(0), gate->pin(idx));
		  des->add_node(gate_t);
	    }
	    des->add_signal(osig);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    des->add_node(gate);
	    break;

	      // Elaborate the structural + as an AddSub
	      // object. Connect DataA and DataB to the parameters,
	      // and connect the output signal to the Result.
	  case '+': {
		assert(lsig->pin_count() == rsig->pin_count());
		string name = des->local_symbol(path);
		unsigned width = lsig->pin_count();
		osig = new NetNet(des->local_symbol(path),
				  NetNet::WIRE, width);
		NetAddSub*adder = new NetAddSub(name, width);
		for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		      connect(lsig->pin(idx), adder->pin_DataA(idx));
		      connect(rsig->pin(idx), adder->pin_DataB(idx));
		      connect(osig->pin(idx), adder->pin_Result(idx));
		}
		gate = adder;
		des->add_signal(osig);
		gate->rise_time(rise);
		gate->fall_time(fall);
		gate->decay_time(decay);
		des->add_node(gate);
		break;
	  }

	  default:
	    cerr << "Unhandled BINARY '" << op_ << "'" << endl;
	    osig = 0;
      }

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(lsig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(rsig))
	    delete tmp;

      return osig;
}

/*
 * The concatenation operator, as a net, is a wide signal that is
 * connected to all the pins of the elaborated expression nets.
 */
NetNet* PEConcat::elaborate_net(Design*des, const string&path,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_line() << ": Sorry, I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
	    nets[idx] = parms_[idx]->elaborate_net(des, path, rise,fall,decay);
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
      NetNet*osig = new NetNet(des->local_symbol(path),
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

/*
 * The concatenation is also OK an an l-value. This method elaborates
 * it as a structural l-value.
 */
NetNet* PEConcat::elaborate_lnet(Design*des, const string&path) const
{
      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;

      if (repeat_) {
	    cerr << get_line() << ": Sorry, I do not know how to"
		  " elaborate repeat concatenation nets." << endl;
	    return 0;
      }

	/* Elaborate the operands of the concatenation. */
      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
	    nets[idx] = parms_[idx]->elaborate_lnet(des, path);
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
      NetNet*osig = new NetNet(des->local_symbol(path),
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
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay) const
{
      NetNet*sig = des->find_signal(path, text_);
      if (sig == 0) {
	    if (des->find_memory(path+"."+text_)) {
		  cerr << get_line() << ": Sorry, memories not supported"
			" in this context." << endl;
		  return 0;
	    }
	    sig = new NetNet(path+"."+text_, NetNet::IMPLICIT, 1);
	    des->add_signal(sig);
	    cerr << get_line() << ": warning: Implicitly defining "
		  "wire " << path << "." << text_ << "." << endl;
      }

      assert(sig);

      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const(des, path);
	    assert(mval);
	    verinum*lval = lsb_->eval_const(des, path);
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

	    if (midx >= lidx) {
		  NetTmp*tmp = new NetTmp(midx-lidx+1);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(lidx-midx+1);
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
	    NetTmp*tmp = new NetTmp(1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
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
	    if (des->find_memory(path+"."+text_)) {
		  cerr << get_line() << ": memories (" << text_
		       << ") cannot be l-values in continuous "
		       << "assignments." << endl;
		  return 0;
	    }

	      /* Fine, create an implicit wire as an l-value. */
	    sig = new NetNet(path+"."+text_, NetNet::IMPLICIT, 1);
	    des->add_signal(sig);
	    cerr << get_line() << ": warning: Implicitly defining "
		  "wire " << path << "." << text_ << "." << endl;
      }

      assert(sig);

	/* Don't allow registers as assign l-values. */
      if (sig->type() == NetNet::REG) {
	    cerr << get_line() << ": registers (" << sig->name()
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
		  NetTmp*tmp = new NetTmp(midx-lidx+1);
		  if (tmp->pin_count() > sig->pin_count()) {
			cerr << get_line() << ": bit select out of "
			     << "range for " << sig->name() << endl;
			return sig;
		  }

		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(lidx-midx+1);
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
	    NetTmp*tmp = new NetTmp(1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 */
NetNet* PENumber::elaborate_net(Design*des, const string&path,
				unsigned long rise,
				unsigned long fall,
				unsigned long decay) const
{
      unsigned width = value_->len();
      NetNet*net = new NetNet(des->local_symbol(path),
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

NetNet* PETernary::elaborate_net(Design*des, const string&path,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const
{
      NetNet* expr_sig = expr_->elaborate_net(des, path);
      NetNet* tru_sig = tru_->elaborate_net(des, path);
      NetNet* fal_sig = fal_->elaborate_net(des, path);
      if (expr_sig == 0 || tru_sig == 0 || fal_sig == 0) {
	    des->errors += 1;
	    return 0;
      }

      NetNet* sig;
      NetLogic*exprinv;
      NetLogic*and_tru;
      NetLogic*and_fal;
      NetLogic*gate;

      assert(tru_sig->pin_count() == fal_sig->pin_count());
      assert(expr_sig->pin_count() == 1);

      sig = new NetNet(des->local_symbol(path), NetNet::WIRE,
		       tru_sig->pin_count());
      sig->local_flag(true);

      for (unsigned idx = 0 ;  idx < tru_sig->pin_count() ;  idx += 1) {
	    exprinv = new NetLogic(des->local_symbol(path), 2, NetLogic::NOT);
	    and_tru = new NetLogic(des->local_symbol(path), 3, NetLogic::AND);
	    and_fal = new NetLogic(des->local_symbol(path), 3, NetLogic::AND);
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::OR);

	    connect(exprinv->pin(1), expr_sig->pin(0));
	    connect(and_tru->pin(1), expr_sig->pin(0));
	    connect(and_fal->pin(1), exprinv->pin(0));
	    connect(and_tru->pin(2), tru_sig->pin(idx));
	    connect(and_fal->pin(2), fal_sig->pin(idx));
	    connect(gate->pin(1), and_tru->pin(0));
	    connect(gate->pin(2), and_fal->pin(0));
	    connect(gate->pin(0), sig->pin(idx));

	    des->add_node(exprinv);
	    des->add_node(and_tru);
	    des->add_node(and_fal);
	    des->add_node(gate);

	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
      }

      des->add_signal(sig);

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(expr_sig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(tru_sig))
	    delete tmp;
      if (NetTmp*tmp = dynamic_cast<NetTmp*>(fal_sig))
	    delete tmp;

      return sig;
}

NetExpr*PETernary::elaborate_expr(Design*des, const string&path) const
{
      NetExpr*con = expr_->elaborate_expr(des, path);
      NetExpr*tru = tru_->elaborate_expr(des, path);
      NetExpr*fal = fal_->elaborate_expr(des, path);

      NetETernary*res = new NetETernary(con, tru, fal);
      return res;
}

NetNet* PEUnary::elaborate_net(Design*des, const string&path,
			       unsigned long rise,
			       unsigned long fall,
			       unsigned long decay) const
{
      NetNet* sub_sig = expr_->elaborate_net(des, path);
      if (sub_sig == 0) {
	    des->errors += 1;
	    return 0;
      }
      assert(sub_sig);

      NetNet* sig;
      NetLogic*gate;
      switch (op_) {
	  case '~': // Bitwise NOT
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE,
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
	    des->add_signal(sig);
	    break;

	  case 'N': // Reduction NOR
	  case '!': // Reduction NOT
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::NOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_signal(sig);
	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '&': // Reduction AND
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::AND);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_signal(sig);
	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '|': // Reduction OR
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::OR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_signal(sig);
	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  case '^': // Reduction XOR
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path),
				1+sub_sig->pin_count(),
				NetLogic::XOR);
	    connect(gate->pin(0), sig->pin(0));
	    for (unsigned idx = 0 ;  idx < sub_sig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), sub_sig->pin(idx));

	    des->add_signal(sig);
	    des->add_node(gate);
	    gate->rise_time(rise);
	    gate->fall_time(fall);
	    gate->decay_time(decay);
	    break;

	  default:
	    cerr << "Unhandled UNARY '" << op_ << "'" << endl;
	    sig = 0;
      }

      if (NetTmp*tmp = dynamic_cast<NetTmp*>(sub_sig))
	    delete tmp;

      return sig;
}

NetExpr* PEBinary::elaborate_expr(Design*des, const string&path) const
{
      bool flag;
      NetExpr*lp = left_->elaborate_expr(des, path);
      NetExpr*rp = right_->elaborate_expr(des, path);
      if ((lp == 0) || (rp == 0)) {
	    delete lp;
	    delete rp;
	    return 0;
      }

      NetEBinary*tmp;
      switch (op_) {
	  default:
	    tmp = new NetEBinary(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'a':
	  case 'o':
	    tmp = new NetEBLogic(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'l':
	  case 'r':
	    tmp = new NetEBShift(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '^':
	  case '&':
	  case '|':
	    tmp = new NetEBBits(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case '+':
	  case '-':
	    tmp = new NetEBAdd(op_, lp, rp);
	    tmp->set_line(*this);
	    break;

	  case 'e': /* == */
	  case 'E': /* === */
	  case 'n': /* != */
	  case 'N': /* !== */
	  case 'L': /* <= */
	  case 'G': /* >= */
	  case '<':
	  case '>':
	    tmp = new NetEBComp(op_, lp, rp);
	    tmp->set_line(*this);
	    flag = tmp->set_width(1);
	    if (flag == false) {
		  cerr << get_line() << ": internal error: "
			"expression bit width of comparison != 1." << endl;
		  des->errors += 1;
	    }
	    break;
      }

      return tmp;
}

NetExpr* PEConcat::elaborate_expr(Design*des, const string&path) const
{
      unsigned repeat = 1;

	/* If there is a repeat expression, then evaluate the constant
	   value and set the repeat count. */
      if (repeat_) {
	    verinum*vrep = repeat_->eval_const(des, path);
	    if (vrep == 0) {
		  cerr << get_line() << ": error: "
			"concatenation repeat expression cannot be evaluated."
		       << endl;
		  des->errors += 1;
		  return 0;
	    }

	    repeat = vrep->as_ulong();
	    delete vrep;
      }

	/* Make the empty concat expression. */
      NetEConcat*tmp = new NetEConcat(parms_.count(), repeat);
      tmp->set_line(*this);

	/* Elaborate all the parameters and attach them to the concat node. */
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    assert(parms_[idx]);
	    NetExpr*ex = parms_[idx]->elaborate_expr(des, path);
	    if (ex == 0) continue;
	    tmp->set(idx, parms_[idx]->elaborate_expr(des, path));
      }

      return tmp;
}

NetExpr* PENumber::elaborate_expr(Design*des, const string&path) const
{
      assert(value_);
      NetEConst*tmp = new NetEConst(*value_);
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PEString::elaborate_expr(Design*des, const string&path) const
{
      NetEConst*tmp = new NetEConst(value());
      tmp->set_line(*this);
      return tmp;
}

NetExpr* PExpr::elaborate_expr(Design*des, const string&path) const
{
      cerr << get_line() << ": I do not know how to elaborate expression: "
	   << *this << endl;
      return 0;
}

NetExpr* PEUnary::elaborate_expr(Design*des, const string&path) const
{
      NetEUnary*tmp = new NetEUnary(op_, expr_->elaborate_expr(des, path));
      tmp->set_line(*this);
      return tmp;
}

NetProc* Statement::elaborate(Design*des, const string&path) const
{
      cerr << "elaborate: What kind of statement? " <<
	    typeid(*this).name() << endl;
      NetProc*cur = new NetProc;
      return cur;
}

NetProc* PAssign::assign_to_memory_(NetMemory*mem, PExpr*ix,
				    Design*des, const string&path) const
{
      NetExpr*rv = rval()->elaborate_expr(des, path);
      if (rv == 0)
	    return 0;

      assert(rv);

      rv->set_width(mem->width());
      NetExpr*idx = ix->elaborate_expr(des, path);
      assert(idx);

      NetAssignMem*am = new NetAssignMem(mem, idx, rv);
      am->set_line(*this);
      return am;
}

/*
 * Elaborate an l-value as a NetNet (it may already exist) and make up
 * the part select stuff for where the assignment is going to be made.
 */
NetNet* PAssign_::elaborate_lval(Design*des, const string&path,
				 unsigned&msb, unsigned&lsb,
				 NetExpr*&mux) const
{
	/* Get the l-value, and assume that it is an identifier. */
      const PEIdent*id = dynamic_cast<const PEIdent*>(lval());

      if (id == 0) {
	    NetNet*ll = lval_->elaborate_net(des, path);
	    if (ll == 0) {
		  cerr << get_line() << ": Assignment l-value too complex."
		       << endl;
		  return 0;
	    }

	    lsb = 0;
	    msb = ll->pin_count()-1;
	    mux = 0;
	    return ll;
      }

      assert(id);

	/* Get the signal referenced by the identifier, and make sure
	   it is a register. */
      NetNet*reg = des->find_signal(path, id->name());

      if (reg == 0) {
	    cerr << get_line() << ": Could not match signal ``" <<
		  id->name() << "'' in ``" << path << "''" << endl;
	    return 0;
      }
      assert(reg);

      if ((reg->type() != NetNet::REG) && (reg->type() != NetNet::INTEGER)) {
	    cerr << get_line() << ": " << *lval() << " is not a register."
		 << endl;
	    return 0;
      }

      if (id->msb_ && id->lsb_) {
	    verinum*vl = id->lsb_->eval_const(des, path);
	    if (vl == 0) {
		  cerr << id->lsb_->get_line() << ": Expression must be"
			" constant in this context: " << *id->lsb_;
		  des->errors += 1;
		  return 0;
	    }
	    verinum*vm = id->msb_->eval_const(des, path);
	    if (vl == 0) {
		  cerr << id->msb_->get_line() << ": Expression must be"
			" constant in this context: " << *id->msb_;
		  des->errors += 1;
		  return 0;
	    }

	    msb = vm->as_ulong();
	    lsb = vl->as_ulong();
	    mux = 0;

      } else if (id->msb_) {
	    assert(id->lsb_ == 0);
	    verinum*v = id->msb_->eval_const(des, path);
	    if (v == 0) {
		  NetExpr*m = id->msb_->elaborate_expr(des, path);
		  assert(m);
		  msb = 0;
		  lsb = 0;
		  mux = m;

	    } else {

		  msb = v->as_ulong();
		  lsb = v->as_ulong();
		  mux = 0;
	    }

      } else {
	    assert(id->msb_ == 0);
	    assert(id->lsb_ == 0);
	    msb = reg->msb();
	    lsb = reg->lsb();
	    mux = 0;
      }

      return reg;
}

NetProc* PAssign::elaborate(Design*des, const string&path) const
{
	/* Catch the case where the lvalue is a reference to a memory
	   item. These are handled differently. */
      do {
	    const PEIdent*id = dynamic_cast<const PEIdent*>(lval());
	    if (id == 0) break;

	    if (NetMemory*mem = des->find_memory(path+"."+id->name()))
		  return assign_to_memory_(mem, id->msb_, des, path);

      } while(0);


	/* elaborate the lval. This detects any part selects and mux
	   expressions that might exist. */
      unsigned lsb, msb;
      NetExpr*mux;
      NetNet*reg = elaborate_lval(des, path, msb, lsb, mux);
      if (reg == 0) return 0;

	/* If there is a delay expression, elaborate it. */
      unsigned long rise_time, fall_time, decay_time;
      delay_.eval_delays(des, path, rise_time, fall_time, decay_time);


	/* Elaborate the r-value expression. */
      assert(rval());
      NetExpr*rv = rval()->elaborate_expr(des, path);
      if (rv == 0)
	    return 0;

      assert(rv);

	/* Try to evaluate the expression, at least as far as possible. */
      if (NetExpr*tmp = rv->eval_tree()) {
	    delete rv;
	    rv = tmp;
      }
      
      NetAssign*cur;

	/* Rewrite delayed assignments as assignments that are
	   delayed. For example, a = #<d> b; becomes:

	     begin
	        tmp = b;
		#<d> a = tmp;
	     end

	   If the delay is an event delay, then the transform is
	   similar, with the event delay replacing the time delay. It
	   is an event delay if the event_ member has a value.

	   This rewriting of the expression allows me to not bother to
	   actually and literally represent the delayed assign in the
	   netlist. The compound statement is exactly equivalent. */

      if (rise_time || event_) {
	    string n = des->local_symbol(path);
	    unsigned wid = reg->pin_count();

	    rv->set_width(reg->pin_count());
	    rv = pad_to_width(rv, reg->pin_count());

	    if (! rv->set_width(reg->pin_count())) {
		  cerr << get_line() << ": error: Unable to match "
			"expression width of " << rv->expr_width() <<
			" to l-value width of " << wid << "." << endl;
		    //XXXX delete rv;
		  return 0;
	    }

	    NetNet*tmp = new NetNet(n, NetNet::REG, wid);
	    tmp->set_line(*this);
	    des->add_signal(tmp);

	      /* Generate an assignment of the l-value to the temporary... */
	    n = des->local_symbol(path);
	    NetAssign*a1 = new NetAssign(n, des, wid, rv);
	    a1->set_line(*this);
	    des->add_node(a1);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(a1->pin(idx), tmp->pin(idx));

	      /* Generate an assignment of the temporary to the r-value... */
	    n = des->local_symbol(path);
	    NetESignal*sig = new NetESignal(tmp);
	    des->add_node(sig);
	    NetAssign*a2 = new NetAssign(n, des, wid, sig);
	    a2->set_line(*this);
	    des->add_node(a2);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(a2->pin(idx), reg->pin(idx));

	      /* Generate the delay statement with the final
		 assignment attached to it. If this is an event delay,
		 elaborate the PEventStatement. Otherwise, create the
		 right NetPDelay object. */
	    NetProc*st;
	    if (event_) {
		  st = event_->elaborate_st(des, path, a2);
		  if (st == 0) {
			cerr << event_->get_line() << ": error: "
			      "unable to elaborate event expression."
			     << endl;
			des->errors += 1;
			return 0;
		  }
		  assert(st);

	    } else {
		  NetPDelay*de = new NetPDelay(rise_time, a2);
		  st = de;
	    }

	      /* And build up the complex statement. */
	    NetBlock*bl = new NetBlock(NetBlock::SEQU);
	    bl->append(a1);
	    bl->append(st);

	    return bl;
      }

      if (mux == 0) {
	      /* This is a simple assign to a register. There may be a
		 part select, so take care that the width is of the
		 part, and using the lsb, make sure the correct range
		 of bits is assigned. */
	    unsigned wid = (msb >= lsb)? (msb-lsb+1) : (lsb-msb+1);
	    assert(wid <= reg->pin_count());

	    rv->set_width(wid);
	    rv = pad_to_width(rv, wid);
	    assert(rv->expr_width() >= wid);

	    cur = new NetAssign(des->local_symbol(path), des, wid, rv);
	    unsigned off = reg->sb_to_idx(lsb);
	    assert((off+wid) <= reg->pin_count());
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(cur->pin(idx), reg->pin(idx+off));

      } else {
	    assert(reg->pin_count() == 1);
	    cerr << get_line() << ": Sorry: l-value bit select expression"
		  " must be constant." << endl;
	    delete reg;
	    delete rv;
	    return 0;
      }


      cur->set_line(*this);
      des->add_node(cur);

      return cur;
}

/*
 * I do not really know how to elaborate mem[x] <= expr, so this
 * method pretends it is a blocking assign and elaborates
 * that. However, I report an error so that the design isn't actually
 * executed by anyone.
 */
NetProc* PAssignNB::assign_to_memory_(NetMemory*mem, PExpr*ix,
				      Design*des, const string&path) const
{
	/* Elaborate the r-value expression, ... */
      NetExpr*rv = rval()->elaborate_expr(des, path);
      if (rv == 0)
	    return 0;

      assert(rv);
      rv->set_width(mem->width());

	/* Elaborate the expression to calculate the index, ... */
      NetExpr*idx = ix->elaborate_expr(des, path);
      assert(idx);

	/* And connect them together in an assignment NetProc. */
      NetAssignMemNB*am = new NetAssignMemNB(mem, idx, rv);
      am->set_line(*this);

      return am;
}

/*
 * The l-value of a procedural assignment is a very much constrained
 * expression. To wit, only identifiers, bit selects and part selects
 * are allowed. I therefore can elaborate the l-value by hand, without
 * the help of recursive elaboration.
 *
 * (For now, this does not yet support concatenation in the l-value.)
 */
NetProc* PAssignNB::elaborate(Design*des, const string&path) const
{
	/* Catch the case where the lvalue is a reference to a memory
	   item. These are handled differently. */
      do {
	    const PEIdent*id = dynamic_cast<const PEIdent*>(lval());
	    if (id == 0) break;

	    if (NetMemory*mem = des->find_memory(path+"."+id->name()))
		  return assign_to_memory_(mem, id->msb_, des, path);

      } while(0);


      unsigned lsb, msb;
      NetExpr*mux;
      NetNet*reg = elaborate_lval(des, path, msb, lsb, mux);
      if (reg == 0) return 0;

      assert(rval());

	/* Elaborate the r-value expression. This generates a
	   procedural expression that I attach to the assignment. */
      NetExpr*rv = rval()->elaborate_expr(des, path);
      if (rv == 0)
	    return 0;

      assert(rv);

      NetAssignNB*cur;
      if (mux == 0) {
	    unsigned wid = msb - lsb + 1;

	    rv->set_width(wid);
	    rv = pad_to_width(rv, wid);
	    assert(wid <= rv->expr_width());

	    cur = new NetAssignNB(des->local_symbol(path), des, wid, rv);
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(cur->pin(idx), reg->pin(idx+lsb));

      } else {
	    assert(reg->pin_count() == 1);
	    cur = new NetAssignNB(des->local_symbol(path), des, 1, mux, rv);
	    connect(cur->pin(0), reg->pin(0));
      }

      unsigned long rise_time, fall_time, decay_time;
      delay_.eval_delays(des, path, rise_time, fall_time, decay_time);
      cur->rise_time(rise_time);
      cur->fall_time(fall_time);
      cur->decay_time(decay_time);


	/* All done with this node. mark its line number and check it in. */
      cur->set_line(*this);
      des->add_node(cur);
      return cur;
}


/*
 * This is the elaboration method for a begin-end block. Try to
 * elaborate the entire block, even if it fails somewhere. This way I
 * get all the error messages out of it. Then, if I detected a failure
 * then pass the failure up.
 */
NetProc* PBlock::elaborate(Design*des, const string&path) const
{
      NetBlock::Type type = (bl_type_==PBlock::BL_PAR)
	    ? NetBlock::PARA
	    : NetBlock::SEQU;
      NetBlock*cur = new NetBlock(type);
      bool fail_flag = false;

      string npath = name_.length()? (path+"."+name_) : path;

	// Handle the special case that the block contains only one
	// statement. There is no need to keep the block node.
      if (list_.count() == 1) {
	    NetProc*tmp = list_[0]->elaborate(des, npath);
	    return tmp;
      }

      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1) {
	    NetProc*tmp = list_[idx]->elaborate(des, npath);
	    if (tmp == 0) {
		  fail_flag = true;
		  continue;
	    }
	    cur->append(tmp);
      }

      if (fail_flag) {
	    delete cur;
	    cur = 0;
      }

      return cur;
}

/*
 * Elaborate a case statement.
 */
NetProc* PCase::elaborate(Design*des, const string&path) const
{
      NetExpr*expr = expr_->elaborate_expr(des, path);
      if (expr == 0) {
	    cerr << get_line() << ": Unable to elaborate the case"
		  " expression." << endl;
	    return 0;
      }

      unsigned icount = 0;
      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.count() == 0)
		  icount += 1;
	    else
		  icount += cur->expr.count();
      }

      NetCase*res = new NetCase(type_, expr, icount);
      res->set_line(*this);

      unsigned inum = 0;
      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {

	    assert(inum < icount);
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.count() == 0) {
		    /* If there are no expressions, then this is the
		       default case. */
		  NetProc*st = 0;
		  if (cur->stat)
			st = cur->stat->elaborate(des, path);

		  res->set_case(inum, 0, st);
		  inum += 1;

	    } else for (unsigned e = 0; e < cur->expr.count(); e += 1) {

		    /* If there are one or more expressions, then
		       iterate over the guard expressions, elaborating
		       a separate case for each. (Yes, the statement
		       will be elaborated again for each.) */
		  NetExpr*gu = 0;
		  NetProc*st = 0;
		  assert(cur->expr[e]);
		  gu = cur->expr[e]->elaborate_expr(des, path);

		  if (cur->stat)
			st = cur->stat->elaborate(des, path);

		  res->set_case(inum, gu, st);
		  inum += 1;
	    }
      }

      return res;
}

NetProc* PCondit::elaborate(Design*des, const string&path) const
{
	// Elaborate and try to evaluate the conditional expression.
      NetExpr*expr = expr_->elaborate_expr(des, path);
      if (expr == 0) {
	    cerr << get_line() << ": Unable to elaborate"
		  " condition expression." << endl;
	    des->errors += 1;
	    return 0;
      }
      NetExpr*tmp = expr->eval_tree();
      if (tmp) {
	    delete expr;
	    expr = tmp;
      }

	// If the condition of the conditional statement is constant,
	// then look at the value and elaborate either the if statement
	// or the else statement. I don't need both. If there is no
	// else_ statement, the use an empty block as a noop.
      if (NetEConst*ce = dynamic_cast<NetEConst*>(expr)) {
	    verinum val = ce->value();
	    delete expr;
	    if (val[0] == verinum::V1)
		  return if_->elaborate(des, path);
	    else if (else_)
		  return else_->elaborate(des, path);
	    else
		  return new NetBlock(NetBlock::SEQU);
      }

	// If the condition expression is more then 1 bits, then
	// generate a comparison operator to get the result down to
	// one bit. Turn <e> into <e> != 0;

      if (! expr->set_width(1)) {
	    assert(expr->expr_width() > 1);
	    verinum zero (verinum::V0, expr->expr_width());
	    NetEConst*ezero = new NetEConst(zero);
	    ezero->set_width(expr->expr_width());
	    NetEBComp*cmp = new NetEBComp('n', expr, ezero);
	    expr = cmp;
      }

	// Well, I actually need to generate code to handle the
	// conditional, so elaborate.
      NetProc*i = if_? if_->elaborate(des, path) : 0;
      NetProc*e = else_? else_->elaborate(des, path) : 0;

      NetCondit*res = new NetCondit(expr, i, e);
      return res;
}

NetProc* PCallTask::elaborate(Design*des, const string&path) const
{
      if (name_[0] == '$')
	    return elaborate_sys(des, path);
      else
	    return elaborate_usr(des, path);
}

/*
 * A call to a system task involves elaborating all the parameters,
 * then passing the list to the NetSTask object.
 */
NetProc* PCallTask::elaborate_sys(Design*des, const string&path) const
{
      svector<NetExpr*>eparms (nparms());

      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    PExpr*ex = parm(idx);
	    eparms[idx] = ex? ex->elaborate_expr(des, path) : 0;
      }

      NetSTask*cur = new NetSTask(name(), eparms);
      return cur;
}

/*
 * A call to a user defined task is different from a call to a system
 * task because a user task in a netlist has no parameters: the
 * assignments are done by the calling thread. For example:
 *
 *  task foo;
 *    input a;
 *    output b;
 *    [...]
 *  endtask;
 *
 *  [...] foo(x, y);
 *
 * is really:
 *
 *  task foo;
 *    reg a;
 *    reg b;
 *    [...]
 *  endtask;
 *
 *  [...]
 *  begin
 *    a = x;
 *    foo;
 *    y = b;
 *  end
 */
NetProc* PCallTask::elaborate_usr(Design*des, const string&path) const
{
      NetTaskDef*def = des->find_task(path + "." + name_);
      if (def == 0) {
	    cerr << get_line() << ": Enable of unknown task ``" <<
		  name_ << "''." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (nparms() != def->port_count()) {
	    cerr << get_line() << ": Port count mismatch in call to ``"
		 << name_ << "''." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetUTask*cur;

	/* Handle tasks with no parameters specially. There is no need
	   to make a sequential block to hold the generated code. */
      if (nparms() == 0) {
	    cur = new NetUTask(def);
	    return cur;
      }

      NetBlock*block = new NetBlock(NetBlock::SEQU);

	/* Generate assignment statement statements for the input and
	   INOUT ports of the task. These are managed by writing
	   assignments with the task port the l-value and the passed
	   expression the r-value. */
      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {

	    NetNet*port = def->port(idx);
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::POUTPUT)
		  continue;

	    NetExpr*rv = parms_[idx]->elaborate_expr(des, path);
	    NetAssign*pr = new NetAssign("@", des, port->pin_count(), rv);
	    for (unsigned pi = 0 ;  pi < port->pin_count() ;  pi += 1)
		  connect(port->pin(pi), pr->pin(pi));
	    des->add_node(pr);
	    block->append(pr);
      }

	/* Generate the task call proper... */
      cur = new NetUTask(def);
      block->append(cur);

	/* Generate assignment statement statements for the output and
	   INOUT ports of the task. The l-value in this case is the
	   expression passed as a parameter, and the r-value is the
	   port to be copied out. */
      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {

	    NetNet*port = def->port(idx);
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::PINPUT)
		  continue;

	      /* Elaborate the parameter expression as a net so that
		 it can be used as an l-value. Then check that the
		 parameter width match up. */
	    NetNet*val = parms_[idx]->elaborate_net(des, path);
	    assert(val);


	      /* Make an expression out of the actual task port. If
		 the port is smaller then the expression to redeive
		 the result, then expand the port by padding with
		 zeros. */
	    NetESignal*sig = new NetESignal(port);
	    NetExpr*pexp = sig;
	    if (sig->expr_width() < val->pin_count()) {
		  unsigned cwid = val->pin_count()-sig->expr_width();
		  verinum pad (verinum::V0, cwid);
		  NetEConst*cp = new NetEConst(pad);
		  cp->set_width(cwid);

		  NetEConcat*con = new NetEConcat(2);
		  con->set(0, cp);
		  con->set(1, sig);
		  con->set_width(val->pin_count());
		  pexp = con;
	    }


	      /* Generate the assignment statement. */
	    NetAssign*ass = new NetAssign("@", des, val->pin_count(), pexp);
	    for (unsigned pi = 0 ; pi < val->pin_count() ;  pi += 1)
		  connect(val->pin(pi), ass->pin(pi));

	    des->add_node(sig);
	    des->add_node(ass);
	    block->append(ass);
      }

      return block;
}

NetProc* PDelayStatement::elaborate(Design*des, const string&path) const
{
      verinum*num = delay_->eval_const(des, path);
      assert(num);

      unsigned long val = num->as_ulong();
      if (statement_)
	    return new NetPDelay(val, statement_->elaborate(des, path));
      else
	    return new NetPDelay(val, 0);
}

/*
 * An event statement gets elaborated as a gate net that drives a
 * special node, the NetPEvent. The NetPEvent is also a NetProc class
 * because execution flows through it. Thus, the NetPEvent connects
 * the structural and the behavioral.
 *
 * Note that it is possible for the statement_ pointer to be 0. This
 * happens when the source has something like "@(E) ;". Note the null
 * statement.
 */
NetProc* PEventStatement::elaborate_st(Design*des, const string&path,
				    NetProc*enet) const
{

	/* Create a single NetPEvent, and a unique NetNEvent for each
	   conjuctive event. An NetNEvent can have many pins only if
	   it is an ANYEDGE detector. Otherwise, only connect to the
	   least significant bit of the expression. */

      NetPEvent*pe = new NetPEvent(des->local_symbol(path), enet);
      for (unsigned idx = 0 ;  idx < expr_.count() ;  idx += 1) {
	    NetNet*expr = expr_[idx]->expr()->elaborate_net(des, path);
	    if (expr == 0) {
		  expr_[0]->dump(cerr);
		  cerr << endl;
		  des->errors += 1;
		  continue;
	    }
	    assert(expr);

	    unsigned pins = (expr_[idx]->type() == NetNEvent::ANYEDGE)
		  ? expr->pin_count() : 1;
	    NetNEvent*ne = new NetNEvent(des->local_symbol(path),
					 pins, expr_[idx]->type(), pe);

	    for (unsigned p = 0 ;  p < pins ;  p += 1)
		  connect(ne->pin(p), expr->pin(p));

	    des->add_node(ne);
      }

      return pe;
}

NetProc* PEventStatement::elaborate(Design*des, const string&path) const
{
      NetProc*enet = 0;
      if (statement_) {
	    enet = statement_->elaborate(des, path);
	    if (enet == 0)
		  return 0;
      }

      return elaborate_st(des, path, enet);
}

/*
 * Forever statements are represented directly in the netlist. It is
 * theoretically possible to use a while structure with a constant
 * expression to represent the loop, but why complicate the code
 * generators so?
 */
NetProc* PForever::elaborate(Design*des, const string&path) const
{
      NetProc*stat = statement_->elaborate(des, path);
      if (stat == 0) return 0;

      NetForever*proc = new NetForever(stat);
      return proc;
}

/*
 * elaborate the for loop as the equivalent while loop. This eases the
 * task for the target code generator. The structure is:
 *
 *     begin : top
 *       name1_ = expr1_;
 *       while (cond_) begin : body
 *          statement_;
 *          name2_ = expr2_;
 *       end
 *     end
 */
NetProc* PForStatement::elaborate(Design*des, const string&path) const
{
      const PEIdent*id1 = dynamic_cast<const PEIdent*>(name1_);
      assert(id1);
      const PEIdent*id2 = dynamic_cast<const PEIdent*>(name2_);
      assert(id2);

      NetBlock*top = new NetBlock(NetBlock::SEQU);

	/* make the expression, and later the initial assignment to
	   the condition variable. The statement in the for loop is
	   very specifically an assignment. */
      NetNet*sig = des->find_signal(path, id1->name());
      if (sig == 0) {
	    cerr << id1->get_line() << ": register ``" << id1->name()
		 << "'' unknown in this context." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(sig);
      NetAssign*init = new NetAssign("@for-assign", des, sig->pin_count(),
				     expr1_->elaborate_expr(des, path));
      for (unsigned idx = 0 ;  idx < init->pin_count() ;  idx += 1)
	    connect(init->pin(idx), sig->pin(idx));

      top->append(init);

      NetBlock*body = new NetBlock(NetBlock::SEQU);

	/* Elaborate the statement that is contained in the for
	   loop. If there is an error, this will return 0 and I should
	   skip the append. No need to worry, the error has been
	   reported so it's OK that the netlist is bogus. */
      NetProc*tmp = statement_->elaborate(des, path);
      if (tmp)
	    body->append(tmp);


	/* Elaborate the increment assignment statement at the end of
	   the for loop. This is also a very specific assignment
	   statement. Put this into the "body" block. */
      sig = des->find_signal(path, id2->name());
      assert(sig);
      NetAssign*step = new NetAssign("@for-assign", des, sig->pin_count(),
				     expr2_->elaborate_expr(des, path));
      for (unsigned idx = 0 ;  idx < step->pin_count() ;  idx += 1)
	    connect(step->pin(idx), sig->pin(idx));

      body->append(step);


	/* Elaborate the condition expression. Try to evaluate it too,
	   in case it is a constant. This is an interesting case
	   worthy of a warning. */
      NetExpr*ce = cond_->elaborate_expr(des, path);
      if (NetExpr*tmp = ce->eval_tree()) {
	    if (dynamic_cast<NetEConst*>(tmp))
		  cerr << get_line() << ": warning: condition expression "
			"is constant." << endl;

	    ce = tmp;
      }


	/* All done, build up the loop. */

      NetWhile*loop = new NetWhile(ce, body);
      top->append(loop);
      return top;
}

/*
 * Elaborating function definitions takes 2 passes. The first creates
 * the NetFuncDef object and attaches the ports to it. The second pass
 * (elaborate_2) elaborates the statement that is contained
 * within. These passes are needed because the statement may invoke
 * the function itself (or other functions) so can't be elaborated
 * until all the functions are partially elaborated.
 */
void PFunction::elaborate_1(Design*des, const string&path) const
{
	/* Translate the wires that are ports to NetNet pointers by
	   presuming that the name is already elaborated, and look it
	   up in the design. Then save that pointer for later use by
	   calls to the task. (Remember, the task itself does not need
	   these ports.) */
      svector<NetNet*>ports (ports_? ports_->count()+1 : 1);
      ports[0] = des->find_signal(path, path);
      for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {
	    NetNet*tmp = des->find_signal(path, (*ports_)[idx]->name());

	    ports[idx+1] = tmp;
      }

      NetFuncDef*def = new NetFuncDef(path, ports);
      des->add_function(path, def);
}

void PFunction::elaborate_2(Design*des, const string&path) const
{
      NetFuncDef*def = des->find_function(path);
      assert(def);

      NetProc*st = statement_->elaborate(des, path);
      if (st == 0) {
	    cerr << statement_->get_line() << ": Unable to elaborate "
		  "statement in function " << path << "." << endl;
	    des->errors += 1;
	    return;
      }

      def->set_proc(st);
}

NetProc* PRepeat::elaborate(Design*des, const string&path) const
{
      NetExpr*expr = expr_->elaborate_expr(des, path);
      if (expr == 0) {
	    cerr << get_line() << ": Unable to elaborate"
		  " repeat expression." << endl;
	    des->errors += 1;
	    return 0;
      }
      NetExpr*tmp = expr->eval_tree();
      if (tmp) {
	    delete expr;
	    expr = tmp;
      }

      NetProc*stat = statement_->elaborate(des, path);
      if (stat == 0) return 0;

	// If the expression is a constant, handle certain special
	// iteration counts.
      if (NetEConst*ce = dynamic_cast<NetEConst*>(expr)) {
	    verinum val = ce->value();
	    switch (val.as_ulong()) {
		case 0:
		  delete expr;
		  delete stat;
		  return new NetBlock(NetBlock::SEQU);
		case 1:
		  delete expr;
		  return stat;
		default:
		  break;
	    }
      }

      NetRepeat*proc = new NetRepeat(expr, stat);
      return proc;
}

/*
 * A task definition is elaborated by elaborating the statement that
 * it contains, and connecting its ports to NetNet objects. The
 * netlist doesn't really need the array of parameters once elaboration
 * is complete, but this is the best place to store them.
 */
void PTask::elaborate(Design*des, const string&path) const
{
      NetProc*st = statement_->elaborate(des, path);
      if (st == 0) {
	    cerr << statement_->get_line() << ": Unable to elaborate "
		  "statement in task " << path << " at " << get_line()
		 << "." << endl;
	    return;
      }

	/* Translate the wires that are ports to NetNet pointers by
	   presuming that the name is already elaborated, and look it
	   up in the design. Then save that pointer for later use by
	   calls to the task. (Remember, the task itself does not need
	   these ports.) */
      svector<NetNet*>ports (ports_? ports_->count() : 0);
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {
	    NetNet*tmp = des->find_signal(path, (*ports_)[idx]->name());

	    ports[idx] = tmp;
      }

      NetTaskDef*def = new NetTaskDef(path, st, ports);
      des->add_task(path, def);
}

/*
 * The while loop is fairly directly represented in the netlist.
 */
NetProc* PWhile::elaborate(Design*des, const string&path) const
{
      NetWhile*loop = new NetWhile(cond_->elaborate_expr(des, path),
				   statement_->elaborate(des, path));
      return loop;
}

bool Module::elaborate(Design*des, const string&path, svector<PExpr*>*overrides_) const
{
      bool result_flag = true;

	// Generate all the parameters that this instance of this
	// module introduce to the design. This needs to be done in
	// two passes. The first pass marks the parameter names as
	// available so that they can be discovered when the second
	// pass references them during elaboration.
      typedef map<string,PExpr*>::const_iterator mparm_it_t;

	// So this loop elaborates the parameters, ...
      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    des->set_parameter(pname, new NetEParam);
      }

	// and this loop elaborates the expressions.
      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    NetExpr*expr = (*cur).second->elaborate_expr(des, path);
	    des->set_parameter(pname, expr);
      }

        // Override parameters
        // FIXME: need to check if too many overrides given
	// FIXME: need to release the replaced expression.

      if (overrides_) {
            list<string>::const_iterator cur = param_names.begin();
            for (unsigned idx = 0 ;  idx < overrides_->count(); idx += 1, cur++) {
	          string pname = path + "." + (*cur);
	          NetExpr*expr = (*overrides_)[idx]->elaborate_expr(des, path);
	          des->set_parameter(pname, expr);
            }
      }

	// Finally, evaluate the parameter value. This step collapses
	// the parameters to NetEConst values.
      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {

	      // Get the NetExpr for the parameter.
	    string pname = path + "." + (*cur).first;
	    const NetExpr*expr = des->find_parameter(path, (*cur).first);
	    assert(expr);

	      // If it's already a NetEConst, then this parameter is done.
	    if (dynamic_cast<const NetEConst*>(expr))
		  continue;

	      // Get a non-constant copy of the expression to evaluate...
	    NetExpr*nexpr = expr->dup_expr();
	    if (nexpr == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
			"unable to dup expression: " << *expr << endl;
		  des->errors += 1;
		  continue;
	    }

	      // Try to evaluate the expression.
	    nexpr = nexpr->eval_tree();
	    if (nexpr == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
			"unable to evaluate parm expression: " <<
			*expr << endl;
		  des->errors += 1;
		  continue;
	    }

	      // The evaluate worked, replace the old expression with
	      // this constant value.
	    assert(nexpr);
	    des->set_parameter(pname, nexpr);
      }

	// Get all the explicitly declared wires of the module and
	// start the signals list with them.
      const list<PWire*>&wl = get_wires();

      for (list<PWire*>::const_iterator wt = wl.begin()
		 ; wt != wl.end()
		 ; wt ++ ) {

	    (*wt)->elaborate(des, path);
      }

	// Elaborate functions.
      typedef map<string,PFunction*>::const_iterator mfunc_it_t;

      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    (*cur).second->elaborate_1(des, pname);
      }

      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    (*cur).second->elaborate_2(des, pname);
      }

	// Elaborate the task definitions. This is done before the
	// behaviors so that task calls may reference these, and after
	// the signals so that the tasks can reference them.
      typedef map<string,PTask*>::const_iterator mtask_it_t;
      for (mtask_it_t cur = tasks_.begin()
		 ; cur != tasks_.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    (*cur).second->elaborate(des, pname);
      }

	// Get all the gates of the module and elaborate them by
	// connecting them to the signals. The gate may be simple or
	// complex.
      const list<PGate*>&gl = get_gates();

      for (list<PGate*>::const_iterator gt = gl.begin()
		 ; gt != gl.end()
		 ; gt ++ ) {

	    (*gt)->elaborate(des, path);
      }

	// Elaborate the behaviors, making processes out of them.
      const list<PProcess*>&sl = get_behaviors();

      for (list<PProcess*>::const_iterator st = sl.begin()
		 ; st != sl.end()
		 ; st ++ ) {

	    NetProc*cur = (*st)->statement()->elaborate(des, path);
	    if (cur == 0) {
		  cerr << (*st)->get_line() << ": Elaboration "
			"failed for this process." << endl;
		  result_flag = false;
		  continue;
	    }

	    NetProcTop*top;
	    switch ((*st)->type()) {
		case PProcess::PR_INITIAL:
		  top = new NetProcTop(NetProcTop::KINITIAL, cur);
		  break;
		case PProcess::PR_ALWAYS:
		  top = new NetProcTop(NetProcTop::KALWAYS, cur);
		  break;
	    }

	    top->set_line(*(*st));
	    des->add_process(top);
      }

      return result_flag;
}

Design* elaborate(const map<string,Module*>&modules,
		  const map<string,PUdp*>&primitives,
		  const string&root)
{
	// Look for the root module in the list.
      map<string,Module*>::const_iterator mod = modules.find(root);
      if (mod == modules.end())
	    return 0;

      Module*rmod = (*mod).second;

	// This is the output design. I fill it in as I scan the root
	// module and elaborate what I find.
      Design*des = new Design;

      modlist = &modules;
      udplist = &primitives;
      bool rc = rmod->elaborate(des, root, (svector<PExpr*>*)0);
      modlist = 0;
      udplist = 0;

      if (rc == false) {
	    delete des;
	    des = 0;
      }
      return des;
}


/*
 * $Log: elaborate.cc,v $
 * Revision 1.102  1999/09/29 18:36:03  steve
 *  Full case support
 *
 * Revision 1.101  1999/09/29 00:42:50  steve
 *  Allow expanding of additive operators.
 *
 * Revision 1.100  1999/09/25 02:57:30  steve
 *  Parse system function calls.
 *
 * Revision 1.99  1999/09/23 03:56:57  steve
 *  Support shift operators.
 *
 * Revision 1.98  1999/09/23 02:28:27  steve
 *  internal error message for funky comparison width.
 *
 * Revision 1.97  1999/09/23 00:21:54  steve
 *  Move set_width methods into a single file,
 *  Add the NetEBLogic class for logic expressions,
 *  Fix error setting with of && in if statements.
 *
 * Revision 1.96  1999/09/22 21:25:42  steve
 *  Expand bits in delayed assignments.
 *
 * Revision 1.95  1999/09/22 04:30:04  steve
 *  Parse and elaborate named for/join blocks.
 *
 * Revision 1.94  1999/09/22 02:00:48  steve
 *  assignment with blocking event delay.
 *
 * Revision 1.93  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 * Revision 1.92  1999/09/18 22:23:50  steve
 *  Match bit widths comming out of task output ports.
 *
 * Revision 1.91  1999/09/18 02:51:35  steve
 *  report non-constant part select expressions.
 *
 * Revision 1.90  1999/09/18 01:53:08  steve
 *  Detect constant lessthen-equal expressions.
 *
 * Revision 1.89  1999/09/17 02:06:25  steve
 *  Handle unconnected module ports.
 *
 * Revision 1.88  1999/09/16 04:18:15  steve
 *  elaborate concatenation repeats.
 *
 * Revision 1.87  1999/09/16 00:33:45  steve
 *  Handle implicit !=0 in if statements.
 *
 * Revision 1.86  1999/09/15 04:17:52  steve
 *  separate assign lval elaboration for error checking.
 *
 * Revision 1.85  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.84  1999/09/14 01:50:52  steve
 *  implicitly declare wires if needed.
 *
 * Revision 1.83  1999/09/13 03:10:59  steve
 *  Clarify msb/lsb in context of netlist. Properly
 *  handle part selects in lval and rval of expressions,
 *  and document where the least significant bit goes
 *  in NetNet objects.
 *
 * Revision 1.82  1999/09/12 01:16:51  steve
 *  Pad r-values in certain assignments.
 *
 * Revision 1.81  1999/09/10 04:04:06  steve
 *  Add ternary elaboration.
 *
 * Revision 1.80  1999/09/08 04:05:30  steve
 *  Allow assign to not match rvalue width.
 *
 * Revision 1.79  1999/09/08 02:24:39  steve
 *  Empty conditionals (pmonta@imedia.com)
 *
 * Revision 1.78  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.77  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.76  1999/09/02 01:59:27  steve
 *  Parse non-blocking assignment delays.
 *
 * Revision 1.75  1999/09/01 20:46:19  steve
 *  Handle recursive functions and arbitrary function
 *  references to other functions, properly pass
 *  function parameters and save function results.
 *
 * Revision 1.74  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.73  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.72  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.71  1999/08/18 04:00:02  steve
 *  Fixup spelling and some error messages. <LRDoolittle@lbl.gov>
 *
 * Revision 1.70  1999/08/08 20:06:06  steve
 *  Uninitialized low and high indices for single gate syntax
 *
 * Revision 1.69  1999/08/06 04:05:28  steve
 *  Handle scope of parameters.
 *
 * Revision 1.68  1999/08/05 04:58:57  steve
 *  Allow integers as register lvalues.
 *
 * Revision 1.67  1999/08/04 02:13:02  steve
 *  Elaborate module ports that are concatenations of
 *  module signals.
 *
 * Revision 1.66  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.65  1999/08/01 21:48:11  steve
 *  set width of procedural r-values when then
 *  l-value is a memory word.
 *
 * Revision 1.64  1999/08/01 21:18:55  steve
 *  elaborate rise/fall/decay for continuous assign.
 *
 * Revision 1.63  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.62  1999/07/31 03:16:54  steve
 *  move binary operators to derived classes.
 *
 * Revision 1.61  1999/07/28 03:46:57  steve
 *  Handle no ports at all for tasks.
 *
 * Revision 1.60  1999/07/24 19:19:06  steve
 *  Add support for task output and inout ports.
 *
 * Revision 1.59  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.58  1999/07/18 21:17:50  steve
 *  Add support for CE input to XNF DFF, and do
 *  complete cleanup of replaced design nodes.
 *
 * Revision 1.57  1999/07/17 19:50:59  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.56  1999/07/17 18:06:02  steve
 *  Better handling of bit width of + operators.
 *
 * Revision 1.55  1999/07/17 03:08:31  steve
 *  part select in expressions.
 *
 * Revision 1.54  1999/07/13 04:08:26  steve
 *  Construct delayed assignment as an equivalent block.
 *
 * Revision 1.53  1999/07/12 00:59:36  steve
 *  procedural blocking assignment delays.
 *
 * Revision 1.52  1999/07/10 03:00:05  steve
 *  Proper initialization of registers.
 *
 * Revision 1.51  1999/07/10 02:19:26  steve
 *  Support concatenate in l-values.
 *
 * Revision 1.50  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.49  1999/06/24 04:45:29  steve
 *  Elaborate wide structoral bitwise OR.
 *
 * Revision 1.48  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.47  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.46  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 * Revision 1.45  1999/06/15 05:38:39  steve
 *  Support case expression lists.
 *
 * Revision 1.44  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.43  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
 * Revision 1.42  1999/06/13 16:30:06  steve
 *  Unify the NetAssign constructors a bit.
 *
 * Revision 1.41  1999/06/13 04:46:54  steve
 *  Add part select lvalues to AssignNB.
 *
 * Revision 1.40  1999/06/12 23:16:37  steve
 *  Handle part selects as l-values to continuous assign.
 *
 * Revision 1.39  1999/06/10 04:03:53  steve
 *  Add support for the Ternary operator,
 *  Add support for repeat concatenation,
 *  Correct some seg faults cause by elaboration
 *  errors,
 *  Parse the casex anc casez statements.
 *
 * Revision 1.38  1999/06/09 03:00:06  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.37  1999/06/09 00:58:06  steve
 *  Support for binary | (Stephen Tell)
 *
 * Revision 1.36  1999/06/07 02:23:31  steve
 *  Support non-blocking assignment down to vvm.
 *
 * Revision 1.35  1999/06/06 23:07:43  steve
 *  Drop degenerate blocks.
 *
 * Revision 1.34  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.33  1999/06/03 05:16:25  steve
 *  Compile time evalutation of constant expressions.
 *
 * Revision 1.32  1999/06/02 15:38:46  steve
 *  Line information with nets.
 *
 * Revision 1.31  1999/05/31 15:45:35  steve
 *  Fix error message.
 *
 * Revision 1.30  1999/05/30 01:11:46  steve
 *  Exressions are trees that can duplicate, and not DAGS.
 *
 * Revision 1.29  1999/05/29 02:36:17  steve
 *  module parameter bind by name.
 *
 * Revision 1.28  1999/05/27 04:13:08  steve
 *  Handle expression bit widths with non-fatal errors.
 *
 * Revision 1.27  1999/05/20 04:31:45  steve
 *  Much expression parsing work,
 *  mark continuous assigns with source line info,
 *  replace some assertion failures with Sorry messages.
 *
 * Revision 1.26  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.25  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.24  1999/05/05 03:04:46  steve
 *  Fix handling of null delay statements.
 *
 * Revision 1.23  1999/05/01 20:43:55  steve
 *  Handle wide events, such as @(a) where a has
 *  many bits in it.
 *
 *  Add to vvm the binary ^ and unary & operators.
 *
 *  Dump events a bit more completely.
 *
 * Revision 1.22  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 */

