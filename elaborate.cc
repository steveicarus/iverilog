/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elaborate.cc,v 1.155 2000/04/09 16:43:50 steve Exp $"
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
# include  "PEvent.h"
# include  "netlist.h"
# include  "netmisc.h"

string Design::local_symbol(const string&path)
{
      strstream res;
      res << "_L" << (lcounter_++) << ends;
      return path + "." + res.str();
}


  // Urff, I don't like this global variable. I *will* figure out a
  // way to get rid of it. But, for now the PGModule::elaborate method
  // needs it to find the module definition.
static const map<string,Module*>* modlist = 0;
static const map<string,PUdp*>*   udplist = 0;

void PEvent::elaborate(Design*des, NetScope*scope) const
{
      NetEvent*ev = new NetEvent(name_);
      ev->set_line(*this);
      scope->add_event(ev);
}

/*
 * Elaborate a source wire. The "wire" is the declaration of wires,
 * registers, ports and memories. The parser has already merged the
 * multiple properties of a wire (i.e. "input wire") so come the
 * elaboration this creates an object in the design that represent the
 * defined item.
 */
void PWire::elaborate(Design*des, NetScope*scope) const
{
      const string path = scope->name();
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
			cerr << msb_[idx]->get_line() << ": error: "
			      "Unable to evaluate constant expression ``" <<
			      *msb_[idx] << "''." << endl;
			des->errors += 1;
			return;
		  }
		  verinum*lval = lsb_[idx]->eval_const(des, path);
		  if (mval == 0) {
			cerr << lsb_[idx]->get_line() << ": error: "
			      "Unable to evaluate constant expression ``" <<
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
			cerr << get_line() << ": error: Inconsistent width, "
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

	    NetNet*sig = new NetNet(scope, path + "." + name_, wtype, msb, lsb);
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
      cerr << "internal error: what kind of gate? " <<
	    typeid(*this).name() << endl;
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
      NetNet*rval = pin(1)->elaborate_net(des, path,
					  lval->pin_count(),
					  rise_time, fall_time, decay_time);
      if (rval == 0) {
	    cerr << get_line() << ": error: Unable to elaborate r-value: "
		 << *pin(1) << endl;
	    des->errors += 1;
	    return;
      }

      assert(lval && rval);

      if (lval->pin_count() > rval->pin_count()) {
	    cerr << get_line() << ": sorry: lval width (" <<
		  lval->pin_count() << ") > rval width (" <<
		  rval->pin_count() << ")." << endl;
	    delete lval;
	    delete rval;
	    des->errors += 1;
	    return;
      }

      for (unsigned idx = 0 ;  idx < lval->pin_count() ;  idx += 1)
	    connect(lval->pin(idx), rval->pin(idx));

      if (lval->local_flag())
	    delete lval;

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
      else
	    name = path+"."+name;

	/* If the verilog source has a range specification for the
	   gates, then I am expected to make more then one
	   gate. Figure out how many are desired. */
      if (msb_) {
	    verinum*msb = msb_->eval_const(des, path);
	    verinum*lsb = lsb_->eval_const(des, path);

	    if (msb == 0) {
		  cerr << get_line() << ": error: Unable to evaluate "
			"expression " << *msb_ << endl;
		  des->errors += 1;
		  return;
	    }

	    if (lsb == 0) {
		  cerr << get_line() << ": error: Unable to evaluate "
			"expression " << *lsb_ << endl;
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

	    tmp << name << "<" << index << ">" << ends;
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
		default:
		  cerr << get_line() << ": internal error: unhandled "
			"gate type." << endl;
		  des->errors += 1;
		  return;
	    }

	    cur[idx]->set_attributes(attributes);
	    cur[idx]->rise_time(rise_time);
	    cur[idx]->fall_time(fall_time);
	    cur[idx]->decay_time(decay_time);

	    des->add_node(cur[idx]);
      }

	/* The gates have all been allocated, this loop runs through
	   the parameters and attaches the ports of the objects. */

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    const PExpr*ex = pin(idx);
	    NetNet*sig = ex->elaborate_net(des, path, 0, 0, 0, 0);
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
		  cerr << get_line() << ": error: Gate count of " <<
			count << " does not match net width of " <<
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
	// Missing module instance names have already been rejected.
      assert(get_name() != "");

      if (msb_) {
	    cerr << get_line() << ": sorry: Module instantiation arrays "
		  "are not yet supported." << endl;
	    des->errors += 1;
	    return;
      }

      NetScope*scope = des->find_scope(path);
      assert(scope);

	// I know a priori that the elaborate_scope created the scope
	// already, so just look it up as a child of the current scope.
      NetScope*my_scope = scope->child(get_name());
      assert(my_scope);
      const string my_name = my_scope -> name();

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
			cerr << get_line() << ": error: port ``" <<
			      pins_[idx].name << "'' is not a port of "
			     << get_name() << "." << endl;
			des->errors += 1;
			continue;
		  }

		    // If I already bound something to this port, then
		    // the (*exp) array will already have a pointer
		    // value where I want to place this expression.
		  if ((*exp)[pidx]) {
			cerr << get_line() << ": error: port ``" <<
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
		  cerr << get_line() << ": error: Wrong number "
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
      rmod->elaborate(des, my_scope);

	// Now connect the ports of the newly elaborated designs to
	// the expressions that are the instantiation parameters. Scan
	// the pins, elaborate the expressions attached to them, and
	// bind them to the port of the elaborated module.

      for (unsigned idx = 0 ;  idx < pins->count() ;  idx += 1) {
	      // Skip unconnected module ports.
	    if ((*pins)[idx] == 0)
		  continue;

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

	    NetNet*sig = (*pins)[idx]->elaborate_net(des, path,
						     prts_pin_count,
						     0, 0, 0);
	    if (sig == 0) {
		  cerr << "internal error: Expression too complicated "
			"for elaboration." << endl;
		  continue;
	    }

	    assert(sig);

	      // Check that the parts have matching pin counts. If
	      // not, they are different widths.
	    if (prts_pin_count != sig->pin_count()) {
		  cerr << get_line() << ": error: Port " << idx << " of "
		       << type_ << " expects " << prts_pin_count <<
			" pins, got " << sig->pin_count() << " from "
		       << sig->name() << endl;
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

void PGModule::elaborate_cudp_(Design*des, PUdp*udp, const string&path) const
{
      const string my_name = path+"."+get_name();
      NetUDP_COMB*net = new NetUDP_COMB(my_name, udp->ports.count());
      net->set_attributes(udp->attributes);

	/* Run through the pins, making netlists for the pin
	   expressions and connecting them to the pin in question. All
	   of this is independent of the nature of the UDP. */
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    if (pin(idx) == 0)
		  continue;

	    NetNet*sig = pin(idx)->elaborate_net(des, path, 1, 0, 0, 0);
	    if (sig == 0) {
		  cerr << "internal error: Expression too complicated "
			"for elaboration:" << *pin(idx) << endl;
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
	    string input = udp->tinput[idx];

	    net->set_table(input, udp->toutput[idx]);
      }

      net->cleanup_table();

	// All done. Add the object to the design.
      des->add_node(net);
}

/*
 * From a UDP definition in the source, make a NetUDP
 * object. Elaborate the pin expressions as netlists, then connect
 * those networks to the pins.
 */
void PGModule::elaborate_sudp_(Design*des, PUdp*udp, const string&path) const
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

	    NetNet*sig = pin(idx)->elaborate_net(des, path, 1, 0, 0, 0);
	    if (sig == 0) {
		  cerr << "internal error: Expression too complicated "
			"for elaboration:" << *pin(idx) << endl;
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
	    string input = string("") + udp->tcurrent[idx] + udp->tinput[idx];
	    net->set_table(input, udp->toutput[idx]);
      }

      net->cleanup_table();

      switch (udp->initial) {
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
	    if ((*udp).second->sequential)
		  elaborate_sudp_(des, (*udp).second, path);
	    else
		  elaborate_cudp_(des, (*udp).second, path);
	    return;
      }

      cerr << get_line() << ": internal error: Unknown module type: " <<
	    type_ << endl;
}

void PGModule::elaborate_scope(Design*des, NetScope*sc) const
{
	// Look for the module type
      map<string,Module*>::const_iterator mod = modlist->find(type_);
      if (mod != modlist->end()) {
	    elaborate_scope_mod_(des, (*mod).second, sc);
	    return;
      }

	// Try a primitive type
      map<string,PUdp*>::const_iterator udp = udplist->find(type_);
      if (udp != udplist->end())
	    return;


      cerr << get_line() << ": error: Unknown module type: " << type_ << endl;
      des->errors += 1;
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
	    cerr << get_line() << ": sorry: I do not know how to"
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

NetProc* Statement::elaborate(Design*des, const string&path) const
{
      cerr << "internal error: elaborate: What kind of statement? " <<
	    typeid(*this).name() << endl;
      NetProc*cur = new NetProc;
      return cur;
}

NetProc* PAssign::assign_to_memory_(NetMemory*mem, PExpr*ix,
				    Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);
      NetExpr*rv = rval()->elaborate_expr(des, scope);
      if (rv == 0)
	    return 0;

      assert(rv);

      rv->set_width(mem->width());
      NetNet*idx = ix->elaborate_net(des, path, 0, 0, 0, 0);
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Get the l-value, and assume that it is an identifier. */
      const PEIdent*id = dynamic_cast<const PEIdent*>(lval());

	/* If the l-value is not a register, then make a structural
	   elaboration. Make a synthetic register that connects to the
	   generated circuit and return that as the l-value. */
      if (id == 0) {
	    NetNet*ll = lval_->elaborate_net(des, path, 0, 0, 0, 0);
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
	   it is a register. (Wires are not allows in this context. */
      NetNet*reg = des->find_signal(path, id->name());

      if (reg == 0) {
	    cerr << get_line() << ": error: Could not match signal ``" <<
		  id->name() << "'' in ``" << path << "''" << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(reg);

      if ((reg->type() != NetNet::REG) && (reg->type() != NetNet::INTEGER)) {
	    cerr << get_line() << ": error: " << *lval() <<
		  " is not a register." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (id->msb_ && id->lsb_) {
	      /* This handles part selects. In this case, there are
		 two bit select expressions, and both must be
		 constant. Evaluate them and pass the results back to
		 the caller. */
	    verinum*vl = id->lsb_->eval_const(des, path);
	    if (vl == 0) {
		  cerr << id->lsb_->get_line() << ": error: "
			"Expression must be constant in this context: "
		       << *id->lsb_;
		  des->errors += 1;
		  return 0;
	    }
	    verinum*vm = id->msb_->eval_const(des, path);
	    if (vl == 0) {
		  cerr << id->msb_->get_line() << ": error: "
			"Expression must be constant in this context: "
		       << *id->msb_;
		  des->errors += 1;
		  return 0;
	    }

	    msb = vm->as_ulong();
	    lsb = vl->as_ulong();
	    mux = 0;

      } else if (id->msb_) {

	      /* If there is only a single select expression, it is a
		 bit select. Evaluate the constant value and treat it
		 as a part select with a bit width of 1. If the
		 expression it not constant, then return the
		 expression as a mux. */
	    assert(id->lsb_ == 0);
	    verinum*v = id->msb_->eval_const(des, path);
	    if (v == 0) {
		  NetExpr*m = id->msb_->elaborate_expr(des, scope);
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

	      /* No select expressions, so presume a part select the
		 width of the register. */

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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Catch the case where the lvalue is a reference to a memory
	   item. These are handled differently. */
      do {
	    const PEIdent*id = dynamic_cast<const PEIdent*>(lval());
	    if (id == 0) break;

	    if (NetMemory*mem = des->find_memory(path, id->name()))
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

      NetExpr*rv;

      if (verinum*val = rval()->eval_const(des,path)) {
	    rv = new NetEConst(*val);
	    delete val;

      } else if (rv = rval()->elaborate_expr(des, scope)) {

	      /* OK, go on. */

      } else {
	      /* Unable to elaborate expression. Retreat. */
	    return 0;
      }

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

	    NetNet*tmp = new NetNet(0, n, NetNet::REG, wid);
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

	    assert(msb == lsb);
	    cur = new NetAssign(des->local_symbol(path), des,
				reg->pin_count(), mux, rv);
	    for (unsigned idx = 0 ;  idx < reg->pin_count() ;  idx += 1)
		  connect(cur->pin(idx), reg->pin(idx));
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Elaborate the r-value expression, ... */
      NetExpr*rv = rval()->elaborate_expr(des, scope);
      if (rv == 0)
	    return 0;

      assert(rv);
      rv->set_width(mem->width());

	/* Elaborate the expression to calculate the index, ... */
      NetNet*idx = ix->elaborate_net(des, path, 0, 0, 0, 0);
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Catch the case where the lvalue is a reference to a memory
	   item. These are handled differently. */
      do {
	    const PEIdent*id = dynamic_cast<const PEIdent*>(lval());
	    if (id == 0) break;

	    if (NetMemory*mem = des->find_memory(path, id->name()))
		  return assign_to_memory_(mem, id->msb_, des, path);

      } while(0);


      unsigned lsb, msb;
      NetExpr*mux;
      NetNet*reg = elaborate_lval(des, path, msb, lsb, mux);
      if (reg == 0) return 0;

      assert(rval());

	/* Elaborate the r-value expression. This generates a
	   procedural expression that I attach to the assignment. */
      NetExpr*rv = rval()->elaborate_expr(des, scope);
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
		  connect(cur->pin(idx), reg->pin(reg->sb_to_idx(idx+lsb)));

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
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetBlock::Type type = (bl_type_==PBlock::BL_PAR)
	    ? NetBlock::PARA
	    : NetBlock::SEQU;
      NetBlock*cur = new NetBlock(type);
      bool fail_flag = false;

      string npath;
      NetScope*nscope;
      if (name_.length()) {
	    nscope = scope->child(name_);
	    if (nscope == 0) {
		  cerr << get_line() << ": internal error: "
			"unable to find block scope " << scope->name()
		       << "<" << name_ << ">" << endl;
		  des->errors += 1;
		  delete cur;
		  return 0;
	    }

	    assert(nscope);
	    npath = nscope->name();

      } else {
	    nscope = scope;
	    npath = path;
      }

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
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetExpr*expr = expr_->elaborate_expr(des, scope);
      if (expr == 0) {
	    cerr << get_line() << ": error: Unable to elaborate this case"
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
		  gu = cur->expr[e]->elaborate_expr(des, scope);

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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	// Elaborate and try to evaluate the conditional expression.
      NetExpr*expr = expr_->elaborate_expr(des, scope);
      if (expr == 0) {
	    cerr << get_line() << ": error: Unable to elaborate"
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

      if (expr->expr_width() < 1) {
	    cerr << get_line() << ": internal error: "
		  "incomprehensible expression width (0)." << endl;
	    return 0;
      }

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
      res->set_line(*this);
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
 *XXXX
 * There is a single special in the call to a system task. Normally,
 * an expression cannot take an unindexed memory. However, it is
 * possible to take a system task parameter a memory if the expression
 * is trivial.
 */
NetProc* PCallTask::elaborate_sys(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      svector<NetExpr*>eparms (nparms());

      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    PExpr*ex = parm(idx);

	    eparms[idx] = ex? ex->elaborate_expr(des, scope) : 0;
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetTaskDef*def = des->find_task(path, name_);
      if (def == 0) {
	    cerr << get_line() << ": error: Enable of unknown task ``" <<
		  path << "." << name_ << "''." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (nparms() != def->port_count()) {
	    cerr << get_line() << ": error: Port count mismatch in call to ``"
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

	    NetExpr*rv = parms_[idx]->elaborate_expr(des, scope);
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
	    NetNet*val = parms_[idx]->elaborate_net(des, path,
						    0, 0, 0, 0);
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

	    des->add_node(ass);
	    block->append(ass);
      }

      return block;
}

NetProc* PDelayStatement::elaborate(Design*des, const string&path) const
{
      verinum*num = delay_->eval_const(des, path);
      if (num == 0) {
	    cerr << get_line() << ": sorry: delay expression "
		  "must be constant." << endl;
	    return 0;
      }
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Handle as a special case the block on an event. In this
	   case I generate a NetEvWait object to represent me.

	   XXXX Do I want to move all event handling to NetEvent style
	   event support? I think I do. */

      if ((expr_.count() == 1) && (expr_[0]->expr() == 0)) {
	    string ename = expr_[0]->name();
	    NetEvent*ev = scope->find_event(ename);
	    if (ev == 0) {
		  cerr << get_line() << ": error: no such named event "
		       << "``" << ename << "''." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEvWait*pr = new NetEvWait(ev, enet);
	    pr->set_line(*this);
	    return pr;
      }

	/* Handle the special case of an event name as an identifier
	   in an expression. Make a named event reference. */
      if (expr_.count() == 1) {
	    PEIdent*id = dynamic_cast<PEIdent*>(expr_[0]->expr());
	    NetEvent*ev;
	    if (id && (ev = scope->find_event(id->name()))) {
		  NetEvWait*pr = new NetEvWait(ev, enet);
		  pr->set_line(*this);
		  return pr;
	    }
      }

	/* Create a single NetPEvent, and a unique NetNEvent for each
	   conjuctive event. An NetNEvent can have many pins only if
	   it is an ANYEDGE detector. Otherwise, only connect to the
	   least significant bit of the expression. */

      NetPEvent*pe = new NetPEvent(des->local_symbol(path), enet);
      for (unsigned idx = 0 ;  idx < expr_.count() ;  idx += 1) {
	    if (expr_[idx]->expr() == 0) {
		  cerr << get_line() << ": sorry: block on named events "
		       << "not supported." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    if (PEIdent*id = dynamic_cast<PEIdent*>(expr_[idx]->expr())) {
		  NetEvent*ev = scope->find_event(id->name());
		  if (ev) {
			cerr << get_line() << ": sorry: block on named events "
			     << "not supported." << endl;
			des->errors += 1;
			return 0;
		  }
	    }

	    NetNet*expr = expr_[idx]->expr()->elaborate_net(des, path,
							    0, 0, 0, 0);
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

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
				     expr1_->elaborate_expr(des, scope));
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
				     expr2_->elaborate_expr(des, scope));
      for (unsigned idx = 0 ;  idx < step->pin_count() ;  idx += 1)
	    connect(step->pin(idx), sig->pin(idx));

      body->append(step);


	/* Elaborate the condition expression. Try to evaluate it too,
	   in case it is a constant. This is an interesting case
	   worthy of a warning. */
      NetExpr*ce = cond_->elaborate_expr(des, scope);
      if (ce == 0) {
	    delete top;
	    return 0;
      }

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
 *
 * In both cases, the scope parameter is the scope of the function. In
 * other words, it is the scope that has the name of the function with
 * the path of the containing module.
 */
void PFunction::elaborate_1(Design*des, NetScope*scope) const
{
	/* Translate the wires that are ports to NetNet pointers by
	   presuming that the name is already elaborated, and look it
	   up in the design. Then save that pointer for later use by
	   calls to the task. (Remember, the task itself does not need
	   these ports.) */
      svector<NetNet*>ports (ports_? ports_->count()+1 : 1);
      ports[0] = des->find_signal(scope->name(), scope->name());
      for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {
	    NetNet*tmp = des->find_signal(scope->name(),
					  (*ports_)[idx]->name());

	    ports[idx+1] = tmp;
      }

      NetFuncDef*def = new NetFuncDef(scope, ports);
      des->add_function(scope->name(), def);
}

void PFunction::elaborate_2(Design*des, NetScope*scope) const
{
      NetFuncDef*def = des->find_function(scope->name());
      assert(def);

      NetProc*st = statement_->elaborate(des, scope->name());
      if (st == 0) {
	    cerr << statement_->get_line() << ": error: Unable to elaborate "
		  "statement in function " << def->name() << "." << endl;
	    des->errors += 1;
	    return;
      }

      def->set_proc(st);
}

NetProc* PRepeat::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetExpr*expr = expr_->elaborate_expr(des, scope);
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
void PTask::elaborate_1(Design*des, const string&path) const
{
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

      NetTaskDef*def = new NetTaskDef(path, ports);
      des->add_task(path, def);
}

void PTask::elaborate_2(Design*des, const string&path) const
{
      NetTaskDef*def = des->find_task(path);
      assert(def);

      NetProc*st;
      if (statement_ == 0) {
	    cerr << get_line() << ": warning: task has no statement." << endl;
	    st = new NetBlock(NetBlock::SEQU);

      } else {

	    st = statement_->elaborate(des, path);
	    if (st == 0) {
		  cerr << statement_->get_line() << ": Unable to elaborate "
			"statement in task " << path << " at " << get_line()
		       << "." << endl;
		  return;
	    }
      }

      def->set_proc(st);
}

NetProc* PTrigger::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetEvent*ev = scope->find_event(event_);
      if (ev == 0) {
	    cerr << get_line() << ": error: event <" << event_ << ">"
		 << " not found." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetEvTrig*trig = new NetEvTrig(ev);
      trig->set_line(*this);
      return trig;
}

/*
 * The while loop is fairly directly represented in the netlist.
 */
NetProc* PWhile::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetWhile*loop = new NetWhile(cond_->elaborate_expr(des, scope),
				   statement_->elaborate(des, path));
      return loop;
}

/*
 * When a module is instantiated, it creates the scope then uses this
 * method to elaborate the contents of the module.
 */
bool Module::elaborate(Design*des, NetScope*scope) const
{
      const string path = scope->name();
      bool result_flag = true;

	// Scan through the named events and elaborate them.
      for (map<string,PEvent*>::const_iterator et = events.begin()
		 ; et != events.end() ;  et ++ ) {

	    (*et).second->elaborate(des, scope);

      }

	// Get all the explicitly declared wires of the module and
	// start the signals list with them.
      const map<string,PWire*>&wl = get_wires();

      for (map<string,PWire*>::const_iterator wt = wl.begin()
		 ; wt != wl.end()
		 ; wt ++ ) {

	    (*wt).second->elaborate(des, scope);
      }

	// Elaborate functions.
      typedef map<string,PFunction*>::const_iterator mfunc_it_t;

      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {
	    NetScope*fscope = scope->child((*cur).first);
	    if (scope == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
		       << "Child scope for function " << (*cur).first
		       << " missing in " << scope->name() << "." << endl;
		  des->errors += 1;
		  continue;
	    }

	    (*cur).second->elaborate_1(des, fscope);
      }

      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {

	    NetScope*fscope = scope->child((*cur).first);
	    assert(fscope);
	    (*cur).second->elaborate_2(des, fscope);
      }

	// Elaborate the task definitions. This is done before the
	// behaviors so that task calls may reference these, and after
	// the signals so that the tasks can reference them.
      typedef map<string,PTask*>::const_iterator mtask_it_t;

      for (mtask_it_t cur = tasks_.begin()
		 ; cur != tasks_.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    (*cur).second->elaborate_1(des, pname);
      }

      for (mtask_it_t cur = tasks_.begin()
		 ; cur != tasks_.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    (*cur).second->elaborate_2(des, pname);
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
		  cerr << (*st)->get_line() << ": error: Elaboration "
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

	// Make the root scope, then scan the pform looking for scopes
	// and parameters. 
      NetScope*scope = des->make_root_scope(root);
      if (! rmod->elaborate_scope(des, scope)) {
	    delete des;
	    return 0;
      }

	// This method recurses through the scopes, looking for
	// defparam assignments to apply to the parameters in the
	// various scopes. This needs to be done after all the scopes
	// and basic parameters are taken care of because the defparam
	// can assign to a paramter declared *after* it.
      des->run_defparams();


	// At this point, all parameter overrides are done. Scane the
	// scopes and evaluate the parameters all the way down to
	// constants.
      des->evaluate_parameters();


	// Now that the structure and parameters are taken care of,
	// run through the pform again and generate the full netlist.
      bool rc = rmod->elaborate(des, scope);


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
 * Revision 1.155  2000/04/09 16:43:50  steve
 *  Catch event names in parentheses.
 *
 * Revision 1.154  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 * Revision 1.153  2000/04/01 22:14:19  steve
 *  detect unsupported block on named events.
 *
 * Revision 1.152  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.151  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.150  2000/03/20 15:28:58  steve
 *  Fix lval part select of non-blocking assign.
 *
 * Revision 1.149  2000/03/12 21:41:47  steve
 *  Connect output of NB assign to indexed pin.
 *
 * Revision 1.148  2000/03/11 03:25:52  steve
 *  Locate scopes in statements.
 *
 * Revision 1.147  2000/03/10 06:20:48  steve
 *  Handle defparam to partial hierarchical names.
 *
 * Revision 1.146  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.145  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.144  2000/02/18 05:15:02  steve
 *  Catch module instantiation arrays.
 *
 * Revision 1.143  2000/02/14 00:11:11  steve
 *  Mark the line numbers of NetCondit nodes.
 *
 * Revision 1.142  2000/02/06 23:13:14  steve
 *  Include the scope in named gates.
 *
 * Revision 1.141  2000/01/10 01:35:23  steve
 *  Elaborate parameters afer binding of overrides.
 */

