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
#ident "$Id: elaborate.cc,v 1.217 2001/07/25 03:10:49 steve Exp $"
#endif

# include "config.h"

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
# include  "util.h"

  // Urff, I don't like this global variable. I *will* figure out a
  // way to get rid of it. But, for now the PGModule::elaborate method
  // needs it to find the module definition.
static const map<string,Module*>* modlist = 0;
static const map<string,PUdp*>*   udplist = 0;

static Link::strength_t drive_type(PGate::strength_t drv)
{
      switch (drv) {
	  case PGate::HIGHZ:
	    return Link::HIGHZ;
	  case PGate::WEAK:
	    return Link::WEAK;
	  case PGate::PULL:
	    return Link::PULL;
	  case PGate::STRONG:
	    return Link::STRONG;
	  case PGate::SUPPLY:
	    return Link::SUPPLY;
	  default:
	    assert(0);
      }
      return Link::STRONG;
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
      NetScope*scope = des->find_scope(path);
      assert(scope);

      unsigned long rise_time, fall_time, decay_time;
      eval_delays(des, path, rise_time, fall_time, decay_time);

      Link::strength_t drive0 = drive_type(strength0());
      Link::strength_t drive1 = drive_type(strength1());

      assert(pin(0));
      assert(pin(1));

	/* Elaborate the l-value. */
      NetNet*lval = pin(0)->elaborate_lnet(des, path);
      if (lval == 0) {
	    des->errors += 1;
	    return;
      }


	/* Handle the special case that the rval is simply an
	   identifier. Get the rval as a NetNet, then use NetBUFZ
	   objects to connect it to the l-value. This is necessary to
	   direct drivers. This is how I attach strengths to the
	   assignment operation. */
      if (const PEIdent*id = dynamic_cast<const PEIdent*>(pin(1))) {
	    NetNet*rid = id->elaborate_net(des, path, lval->pin_count(),
					   0, 0, 0, Link::STRONG,
					   Link::STRONG);
	    if (rid == 0) {
		  des->errors += 1;
		  return;
	    }

	    assert(rid);


	      /* If the right hand net is the same type as the left
		 side net (i.e. WIRE/WIRE) then it is enough to just
		 connect them together. Otherwise, put a bufz between
		 them to carry strengths from the rval.

		 While we are at it, handle the case where the r-value
		 is not as wide as th l-value by padding with a
		 constant-0. */

	    unsigned cnt = lval->pin_count();
	    if (rid->pin_count() < cnt)
		  cnt = rid->pin_count();

	    if ((rid->type() == lval->type()) && (rise_time == 0)) {
		  unsigned idx;
		  for (idx = 0 ;  idx < cnt; idx += 1)
			connect(lval->pin(idx), rid->pin(idx));

		  if (cnt < lval->pin_count()) {
			verinum tmpv (0UL, lval->pin_count()-cnt);
			NetConst*tmp = new NetConst(des->local_symbol(path),
						    tmpv);
			des->add_node(tmp);
			for (idx = cnt ;  idx < lval->pin_count() ; idx += 1)
			      connect(lval->pin(idx), tmp->pin(idx-cnt));
		  }

	    } else {
		  unsigned idx;
		  for (idx = 0 ; idx < cnt ;  idx += 1) {
			NetBUFZ*dev = new NetBUFZ(scope,
						  des->local_symbol(path));
			connect(lval->pin(idx), dev->pin(0));
			connect(rid->pin(idx), dev->pin(1));
			dev->rise_time(rise_time);
			dev->fall_time(fall_time);
			dev->decay_time(decay_time);
			dev->pin(0).drive0(drive0);
			dev->pin(0).drive1(drive1);
			des->add_node(dev);
		  }

		  if (cnt < lval->pin_count()) {
			NetConst*dev = new NetConst(des->local_symbol(path),
						    verinum::V0);
			
			des->add_node(dev);
			dev->pin(0).drive0(drive0);
			dev->pin(0).drive1(drive1);
			for (idx = cnt ;  idx < lval->pin_count() ; idx += 1)
			      connect(lval->pin(idx), dev->pin(0));
		  }
	    }

	    return;
      }

	/* Elaborate the r-value. Account for the initial decays,
	   which are going to be attached to the last gate before the
	   generated NetNet. */
      NetNet*rval = pin(1)->elaborate_net(des, path,
					  lval->pin_count(),
					  rise_time, fall_time, decay_time,
					  drive0, drive1);
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

      NetScope*scope = des->find_scope(path);

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
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::AND);
		  break;
		case BUF:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::BUF);
		  break;
		case BUFIF0:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::BUFIF0);
		  break;
		case BUFIF1:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::BUFIF1);
		  break;
		case NAND:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NAND);
		  break;
		case NMOS:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NMOS);
		  break;
		case NOR:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NOR);
		  break;
		case NOT:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NOT);
		  break;
		case NOTIF0:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NOTIF0);
		  break;
		case NOTIF1:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::NOTIF1);
		  break;
		case OR:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::OR);
		  break;
		case RNMOS:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::RNMOS);
		  break;
		case RPMOS:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::RPMOS);
		  break;
		case PMOS:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::PMOS);
		  break;
		case PULLDOWN:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::PULLDOWN);
		  break;
		case PULLUP:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::PULLUP);
		  break;
		case XNOR:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::XNOR);
		  break;
		case XOR:
		  cur[idx] = new NetLogic(scope, inm, pin_count(),
					  NetLogic::XOR);
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

	    cur[idx]->pin(0).drive0(drive_type(strength0()));
	    cur[idx]->pin(0).drive1(drive_type(strength1()));

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

	// This can get rather complicated because the port can be
	// unconnected (meaning an empty paramter is passed) connected
	// to a concatenation, or connected to an internally
	// unconnected port.

      for (unsigned idx = 0 ;  idx < pins->count() ;  idx += 1) {

	      // Skip unconnected module ports. This happens when a
	      // null parameter is passed in.
	    if ((*pins)[idx] == 0)
		  continue;

	      // Inside the module, the port is zero or more signals
	      // that were already elaborated. List all those signals
	      // and the NetNet equivilents.
	    svector<PEIdent*> mport = rmod->get_port(idx);
	    svector<NetNet*>prts (mport.count());

	      // Count the internal pins of the port.
	    unsigned prts_pin_count = 0;
	    for (unsigned ldx = 0 ;  ldx < mport.count() ;  ldx += 1) {
		  PEIdent*pport = mport[ldx];
		  assert(pport);
		  prts[ldx] = pport->elaborate_port(des, my_scope);
		  if (prts[ldx] == 0)
			continue;

		  assert(prts[ldx]);
		  prts_pin_count += prts[ldx]->pin_count();
	    }

	      // If I find that the port in unconnected inside the
	      // module, then there is nothing to connect. Skip the
	      // paramter.
	    if (prts_pin_count == 0) {
		  continue;
	    }

	      // Elaborate the expression that connects to the module
	      // port. sig is the thing outside the module that
	      // connects to the port.

	    NetNet*sig = (*pins)[idx]->elaborate_net(des, path,
						     prts_pin_count,
						     0, 0, 0);
	    if (sig == 0) {
		  cerr << "internal error: Expression too complicated "
			"for elaboration." << endl;
		  continue;
	    }

	    assert(sig);

	      // Check that a reg is not passed as an output or inout
	      // port of the module. sig is the elaborated signal in
	      // the outside that is to be passed, and prts is a
	      // concatenation of signals on the input that receive a
	      // reg value.
	    if ((sig->type() == NetNet::REG)
		&& (prts.count() >= 1)
		&& (prts[0]->port_type() != NetNet::PINPUT)) {
		  cerr << get_line() << ": error: reg/variable "
		       << sig->name() << " cannot connect to "
		       << "output port " << (idx+1) << " of "
		       << my_scope->name() << "." << endl;
		  des->errors += 1;
		  continue;
	    }


	      // Check that the parts have matching pin counts. If
	      // not, they are different widths. Note that idx is 0
	      // based, but users count parameter positions from 1.
	    if (prts_pin_count != sig->pin_count()) {
		  cerr << get_line() << ": warning: Port " << (idx+1) << " of "
		       << type_ << " expects " << prts_pin_count <<
			" pins, got " << sig->pin_count() << "." << endl;

		  if (prts_pin_count > sig->pin_count()) {
			cerr << get_line() << ":        : Leaving "
			     << (prts_pin_count-sig->pin_count())
			     << " high bits of the port unconnected."
			     << endl;
		  } else {
			cerr << get_line() << ":        : Leaving "
			     << (sig->pin_count()-prts_pin_count)
			     << " high bits of the parameter dangling."
			     << endl;
		  }
	    }

	      // Connect the sig expression that is the context of the
	      // module instance to the ports of the elaborated module.

	      // The prts_pin_count variable is the total width of the
	      // port and is the maximum number of connections to
	      // make. sig is the elaborated expression that connects
	      // to that port. If sig has too few pins, then reduce
	      // the number of connections to make.

	      // Connect this many of the port pins. If the expression
	      // is too small, the reduce the number of connects.
	    unsigned ccount = prts_pin_count;
	    if (sig->pin_count() < ccount)
		  ccount = sig->pin_count();

	      // Now scan the concatenation that makes up the port,
	      // connecting pins until we run out of port pins or sig
	      // pins.

	    unsigned spin = 0;
	    for (unsigned ldx = prts.count() ;  ldx > 0 ;  ldx -= 1) {
		  unsigned cnt = prts[ldx-1]->pin_count();
		  if (cnt > ccount)
			cnt = ccount;
		  for (unsigned p = 0 ;  p < cnt ;  p += 1) {
			connect(sig->pin(spin), prts[ldx-1]->pin(p));
			ccount -= 1;
			spin += 1;
		  }
		  if (ccount == 0)
			break;
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
      NetScope*scope = des->find_scope(path);

      string my_name = get_name();
      if (my_name == "")
	    my_name = des->local_symbol(path);
      else
	    my_name = path+"."+my_name;

      NetUDP*net = new NetUDP(scope, my_name, udp->ports.count(), udp);
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
      
	// All done. Add the object to the design.
      des->add_node(net);
}


bool PGModule::elaborate_sig(Design*des, NetScope*scope) const
{
	// Look for the module type
      map<string,Module*>::const_iterator mod = modlist->find(type_);
      if (mod != modlist->end())
	    return elaborate_sig_mod_(des, scope, (*mod).second);

      return true;
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
      NetScope*scope = des->find_scope(path);
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

NetProc* Statement::elaborate(Design*des, const string&path) const
{
      cerr << get_line() << ": internal error: elaborate: What kind of statement? " <<
	    typeid(*this).name() << endl;
      NetProc*cur = new NetProc;
      des->errors += 1;
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
      if (ix == 0) {
	    cerr << get_line() << ": internal error: No index in lval "
		 << "of assignment to memory?" << endl;
	    return 0;
      }

      assert(ix);
      NetExpr*idx = ix->elaborate_expr(des, scope);
      assert(idx);

      if (rv->expr_width() < mem->width())
	    rv = pad_to_width(rv, mem->width());

      NetAssignMem*am = new NetAssignMem(mem, idx, rv);
      am->set_line(*this);
      return am;
}

NetAssign_* PAssign_::elaborate_lval(Design*des, NetScope*scope) const
{
      return lval_->elaborate_lval(des, scope);
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

	    NetNet*net = des->find_signal(scope, id->name());
	    if (net && (net->scope() == scope))
		  break;

	    if (NetMemory*mem = des->find_memory(scope, id->name()))
		  return assign_to_memory_(mem, id->msb_, des, path);

      } while(0);


	/* elaborate the lval. This detects any part selects and mux
	   expressions that might exist. */
      NetAssign_*lv = elaborate_lval(des, scope);
      if (lv == 0) return 0;

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
	    unsigned wid = lv->pin_count();

	    rv->set_width(lv->pin_count());
	    rv = pad_to_width(rv, lv->pin_count());

	    if (! rv->set_width(lv->pin_count())) {
		  cerr << get_line() << ": error: Unable to match "
			"expression width of " << rv->expr_width() <<
			" to l-value width of " << wid << "." << endl;
		    //XXXX delete rv;
		  return 0;
	    }

	    NetNet*tmp = new NetNet(scope, n, NetNet::REG, wid);
	    tmp->set_line(*this);

	    NetESignal*sig = new NetESignal(tmp);

	      /* Generate an assignment of the l-value to the temporary... */
	    n = des->local_symbol(path);
	    NetAssign_*lvt = new NetAssign_(n, wid);
	    des->add_node(lvt);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(lvt->pin(idx), tmp->pin(idx));

	    NetAssign*a1 = new NetAssign(lvt, rv);
	    a1->set_line(*this);

	      /* Generate an assignment of the temporary to the r-value... */
	    NetAssign*a2 = new NetAssign(lv, sig);
	    a2->set_line(*this);

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

      { unsigned wid = count_lval_width(lv);
        rv->set_width(wid);
	rv = pad_to_width(rv, wid);
	assert(rv->expr_width() >= wid);
      }

      NetAssign*cur = new NetAssign(lv, rv);
      cur->set_line(*this);

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
      NetExpr*idx = ix->elaborate_expr(des, scope);
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

	    if (NetMemory*mem = des->find_memory(scope, id->name()))
		  return assign_to_memory_(mem, id->msb_, des, path);

      } while(0);


      NetAssign_*lv = elaborate_lval(des, scope);
      if (lv == 0) return 0;

      assert(rval());

	/* Elaborate the r-value expression. This generates a
	   procedural expression that I attach to the assignment. */
      NetExpr*rv = rval()->elaborate_expr(des, scope);
      if (rv == 0)
	    return 0;

      assert(rv);

      { unsigned wid = count_lval_width(lv);
        rv->set_width(wid);
	rv = pad_to_width(rv, wid);
      }


      unsigned long rise_time, fall_time, decay_time;
      delay_.eval_delays(des, path, rise_time, fall_time, decay_time);
      lv->rise_time(rise_time);
      lv->fall_time(fall_time);
      lv->decay_time(decay_time);


	/* All done with this node. mark its line number and check it in. */
      NetAssignNB*cur = new NetAssignNB(lv, rv);
      cur->set_line(*this);
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

	      // If the result turns out to be a noop, then skip it.
	    if (NetBlock*tbl = dynamic_cast<NetBlock*>(tmp))
		  if (tbl->proc_first() == 0) {
			delete tbl;
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

      if (expr->expr_width() > 1) {
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

      NetScope*task = des->find_task(scope, name_);
      NetTaskDef*def = task->task_def();
      if (def == 0) {
	    cerr << get_line() << ": error: Enable of unknown task ``" <<
		  scope->name() << "." << name_ << "''." << endl;
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
	    cur = new NetUTask(task);
	    return cur;
      }

      NetBlock*block = new NetBlock(NetBlock::SEQU);


	/* Detect the case where the definition of the task is known
	   empty. In this case, we need not bother with calls to the
	   task, all the assignments, etc. Just return a no-op. */

      if (const NetBlock*tp = dynamic_cast<const NetBlock*>(def->proc())) {
	    if (tp->proc_first() == 0)
		  return block;
      }

	/* Generate assignment statement statements for the input and
	   INOUT ports of the task. These are managed by writing
	   assignments with the task port the l-value and the passed
	   expression the r-value. We know by definition that the port
	   is a reg type, so this elaboration is pretty obvious. */

      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {

	    NetNet*port = def->port(idx);
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::POUTPUT)
		  continue;

	    NetExpr*rv = parms_[idx]->elaborate_expr(des, scope);
	    NetAssign_*lv = new NetAssign_("@", port->pin_count());
	    des->add_node(lv);
	    for (unsigned pi = 0 ;  pi < port->pin_count() ;  pi += 1)
		  connect(port->pin(pi), lv->pin(pi));
	    NetAssign*pr = new NetAssign(lv, rv);
	    block->append(pr);
      }

	/* Generate the task call proper... */
      cur = new NetUTask(task);
      block->append(cur);


	/* Generate assignment statements for the output and INOUT
	   ports of the task. The l-value in this case is the
	   expression passed as a parameter, and the r-value is the
	   port to be copied out.

	   We know by definition that the r-value of this copy-out is
	   the port, which is a reg. The l-value, however, may be any
	   expression that can be a target to a procedural
	   assignment, including a memory word. */

      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {

	    NetNet*port = def->port(idx);

	      /* Skip input ports. */
	    assert(port->port_type() != NetNet::NOT_A_PORT);
	    if (port->port_type() == NetNet::PINPUT)
		  continue;

	      /* Handle the special case that the output parameter is
		 a memory word. Generate a NetAssignMem instead of a
		 NetAssign. */
	    NetMemory*mem;
	    const PEIdent*id = dynamic_cast<const PEIdent*>(parms_[idx]);
	    if (id && (mem = des->find_memory(scope, id->name()))) {

		  NetExpr*ix = id->msb_->elaborate_expr(des, scope);
		  assert(ix);

		  NetExpr*rx = new NetESignal(port);
		  NetAssignMem*am = new NetAssignMem(mem, ix, rx);
		  block->append(am);
		  continue;
	    }


	    NetAssign_*lv = parms_[idx]
		  ? parms_[idx]->elaborate_lval(des, scope)
		  : 0;
	    if (lv == 0)
		  continue;

	    NetESignal*sig = new NetESignal(port);

	      /* Generate the assignment statement. */
	    NetAssign*ass = new NetAssign(lv, sig);

	    block->append(ass);
      }

      return block;
}

NetCAssign* PCAssign::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lval = lval_->elaborate_anet(des, scope);
      if (lval == 0)
	    return 0;

      NetNet*rval = expr_->elaborate_net(des, path, lval->pin_count(),
					 0, 0, 0);
      if (rval == 0)
	    return 0;

      if (rval->pin_count() < lval->pin_count())
	    rval = pad_to_width(des, rval, lval->pin_count());

      NetCAssign* dev = new NetCAssign(des->local_symbol(path), lval);
      dev->set_line(*this);
      des->add_node(dev);

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1)
	    connect(dev->pin(idx), rval->pin(idx));

      return dev;
}

NetDeassign* PDeassign::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lval = lval_->elaborate_net(des, path, 0, 0, 0, 0);
      if (lval == 0)
	    return 0;

      NetDeassign*dev = new NetDeassign(lval);
      dev->set_line( *this );
      return dev;
}

/*
 * Elaborate the delay statment (of the form #<expr> <statement>) as a
 * NetPDelay object. If the expression is constant, evaluate it now
 * and make a constant delay. If not, then pass an elaborated
 * expression to the constructor of NetPDelay so that the code
 * generator knows to evaluate the expression at run time.
 */
NetProc* PDelayStatement::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

	/* Catch the special case that the delay is given as a
	   floating point number. In this case, we need to scale the
	   delay to the units of the design. */

      if (verireal*fn = delay_? delay_->eval_rconst(des, scope) : 0) {
	    int shift = scope->time_unit() - des->get_precision();

	    long delay = fn->as_long(shift);
	    if (delay < 0)
		  delay = 0;

	    delete fn;

	    if (statement_)
		return new NetPDelay(delay, statement_->elaborate(des, path));
	    else
		return new NetPDelay(delay, 0);

      }


      verinum*num = delay_->eval_const(des, path);
      if (num == 0) {
	      /* Ah, the delay is not constant. OK, elaborate the
		 expression and let the run-time handle it. */
	    NetExpr*dex = delay_->elaborate_expr(des, scope);
	    if (statement_)
		  return new NetPDelay(dex, statement_->elaborate(des, path));
	    else
		  return new NetPDelay(dex, 0);
      }
      assert(num);

	/* Convert the delay in the units of the scope to the
	   precision of the design as a whole. */
      unsigned long val = des->scale_to_precision(num->as_ulong(), scope);
      delete num;

	/* If there is a statement, then elaborate it and create a
	   NetPDelay statement to contain it. Note that we create a
	   NetPDelay statement even if the value is 0 because #0 does
	   in fact have a well defined meaning in Verilog. */

      if (statement_) {
	    NetProc*stmt = statement_->elaborate(des, path);
	    return new NetPDelay(val, stmt);

      }  else {
	    return new NetPDelay(val, 0);
      }
}

/*
 * The disable statement is not yet supported.
 */
NetProc* PDisable::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetScope*target = des->find_scope(scope, scope_);
      if (target == 0) {
	    cerr << get_line() << ": error: Cannot find scope "
		 << scope_ << " in " << scope->name() << endl;
	    des->errors += 1;
	    return 0;
      }

      switch (target->type()) {
	  case NetScope::FUNC:
	    cerr << get_line() << ": error: Cannot disable functions." << endl;
	    des->errors += 1;
	    return 0;

	  case NetScope::MODULE:
	    cerr << get_line() << ": error: Cannot disable modules." << endl;
	    des->errors += 1;
	    return 0;

	  default:
	    break;
      }

      NetDisable*obj = new NetDisable(target);
      obj->set_line(*this);
      return obj;
}

/*
 * An event statement is an event delay of some sort, attached to a
 * statement. Some Verilog examples are:
 *
 *      @(posedge CLK) $display("clock rise");
 *      @event_1 $display("event triggered.");
 *      @(data or negedge clk) $display("data or clock fall.");
 *
 * The elaborated netlist uses the NetEvent, NetEvWait and NetEvProbe
 * classes. The NetEvWait class represents the part of the netlist
 * that is executed by behavioral code. The process starts waiting on
 * the NetEvent when it executes the NetEvWait step. Net NetEvProbe
 * and NetEvTrig are structural and behavioral equivilents that
 * trigger the event, and awakens any processes blocking in the
 * associated wait.
 *
 * The basic data structure is:
 *
 *       NetEvWait ---/--->  NetEvent  <----\---- NetEvProbe
 *        ...         |                     |         ...
 *       NetEvWait ---+                     +---- NetEvProbe
 *                                          |         ...
 *                                          +---- NetEvTrig
 *
 * That is, many NetEvWait statements may wait on a single NetEvent
 * object, and Many NetEvProbe objects may trigger the NetEvent
 * object. The many NetEvWait objects pointing to the NetEvent object
 * reflects the possibility of different places in the code blocking
 * on the same named event, like so:
 *
 *         event foo;
 *           [...]
 *         always begin @foo <statement1>; @foo <statement2> end
 *
 * This tends to not happen with signal edges. The multiple probes
 * pointing to the same event reflect the possibility of many
 * expressions in the same blocking statement, like so:
 *
 *         wire reset, clk;
 *           [...]
 *         always @(reset or posedge clk) <stmt>;
 *
 * Conjunctions like this cause a NetEvent object be created to
 * represent the overall conjuction, and NetEvProbe objects for each
 * event expression.
 *
 * If the NetEvent object represents a named event from the source,
 * then there are NetEvTrig objects that represent the trigger
 * statements instead of the NetEvProbe objects representing signals.
 * For example:
 *
 *         event foo;
 *         always @foo <stmt>;
 *         initial begin
 *                [...]
 *            -> foo;
 *                [...]
 *            -> foo;
 *                [...]
 *         end
 *
 * Each trigger statement in the source generates a separate NetEvTrig
 * object in the netlist. Those trigger objects are elaborated
 * elsewhere.
 *
 * Additional complications arise when named events show up in
 * conjunctions. An example of such a case is:
 *
 *         event foo;
 *         wire bar;
 *         always @(foo or posedge bar) <stmt>;
 *
 * Since there is by definition a NetEvent object for the foo object,
 * this is handled by allowing the NetEvWait object to point to
 * multiple NetEvent objects. All the NetEvProbe based objects are
 * collected and pointed as the synthetic NetEvent object, and all the
 * named events are added into the list of NetEvent object that the
 * NetEvWait object can refer to.
 */

NetProc* PEventStatement::elaborate_st(Design*des, const string&path,
				       NetProc*enet) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);


	/* The Verilog wait (<expr>) <statement> statement is a level
	   sensitive wait. Handle this special case by elaborating
	   something like this:

	       begin
	         if (! <expr>) @(posedge <expr>)
	         <statement>
	       end

	   This is equivilent, and uses the existing capapilities of
	   the netlist format. The resulting netlist should look like
	   this:

	       NetBlock ---+---> NetCondit --+--> <expr>
	                   |                 |
			   |     	     +--> NetEvWait--> NetEvent
			   |
			   +---> <statement>

	   This is quite a mouthful. Should I not move wait handling
	   to specialized objects? */


      if ((expr_.count() == 1) && (expr_[0]->type() == PEEvent::POSITIVE)) {

	    NetNet*ex = expr_[0]->expr()->elaborate_net(des, path,
							1, 0, 0, 0);
	    if (ex == 0) {
		  expr_[0]->dump(cerr);
		  cerr << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEvent*ev = new NetEvent(scope->local_symbol());
	    scope->add_event(ev);

	    NetEvWait*we = new NetEvWait(0);
	    we->add_event(ev);

	    NetEvProbe*po = new NetEvProbe(path+"."+scope->local_symbol(),
					   ev, NetEvProbe::POSEDGE, 1);
	    connect(po->pin(0), ex->pin(0));

	    des->add_node(po);

	    NetESignal*ce = new NetESignal(ex);
	    NetCondit*co = new NetCondit(new NetEUnary('!', ce), we, 0);

	    ev->set_line(*this);
	    we->set_line(*this);
	    co->set_line(*this);

	      /* If we don't have a sub-statement after all, then we
		 don't really need the block and we can save the
		 node. (i.e. wait (foo==1) ;) However, the common case
		 has a statement in the wait so we create a sequential
		 block to join the wait and the statement. */

	    if (enet) {
		  NetBlock*bl = new NetBlock(NetBlock::SEQU);
		  bl->set_line(*this);
		  bl->append(co);
		  bl->append(enet);
		  return bl;
	    }

	    return co;
      }


	/* Handle the special case of an event name as an identifier
	   in an expression. Make a named event reference. */

      if (expr_.count() == 1) {
	    assert(expr_[0]->expr());
	    PEIdent*id = dynamic_cast<PEIdent*>(expr_[0]->expr());
	    NetEvent*ev;
	    if (id && (ev = scope->find_event(id->name()))) {
		  NetEvWait*pr = new NetEvWait(enet);
		  pr->add_event(ev);
		  pr->set_line(*this);
		  return pr;
	    }
      }

	/* Create A single NetEvent and NetEvWait. Then, create a
	   NetEvProbe for each conjunctive event in the event
	   list. The NetEvProbe object al refer back to the NetEvent
	   object. */

      NetEvent*ev = new NetEvent(scope->local_symbol());
      ev->set_line(*this);
      unsigned expr_count = 0;

      NetEvWait*wa = new NetEvWait(enet);
      wa->set_line(*this);

      for (unsigned idx = 0 ;  idx < expr_.count() ;  idx += 1) {

	    assert(expr_[idx]->expr());

	      /* If the expression is an identifier that matches a
		 named event, then handle this case all at once at
		 skip the rest of the expression handling. */

	    if (PEIdent*id = dynamic_cast<PEIdent*>(expr_[idx]->expr())) {
		  NetEvent*tmp = scope->find_event(id->name());
		  if (tmp) {
			wa->add_event(tmp);
			continue;
		  }
	    }

	      /* So now we have a normal event expression. Elaborate
		 the sub-expression as a net and decide how to handle
		 the edge. */

	    NetNet*expr = expr_[idx]->expr()->elaborate_net(des, path,
							    0, 0, 0, 0);
	    if (expr == 0) {
		  expr_[idx]->dump(cerr);
		  cerr << endl;
		  des->errors += 1;
		  continue;
	    }
	    assert(expr);

	    unsigned pins = (expr_[idx]->type() == PEEvent::ANYEDGE)
		  ? expr->pin_count() : 1;

	    NetEvProbe*pr;
	    switch (expr_[idx]->type()) {
		case PEEvent::POSEDGE:
		  pr = new NetEvProbe(des->local_symbol(path), ev,
				      NetEvProbe::POSEDGE, pins);
		  break;

		case PEEvent::NEGEDGE:
		  pr = new NetEvProbe(des->local_symbol(path), ev,
				      NetEvProbe::NEGEDGE, pins);
		  break;

		case PEEvent::ANYEDGE:
		  pr = new NetEvProbe(des->local_symbol(path), ev,
				      NetEvProbe::ANYEDGE, pins);
		  break;

		default:
		  assert(0);
	    }

	    for (unsigned p = 0 ;  p < pr->pin_count() ; p += 1)
		  connect(pr->pin(p), expr->pin(p));

	    des->add_node(pr);
	    expr_count += 1;
      }

	/* If there was at least one conjunction that was an
	   expression (and not a named event) then add this
	   event. Otherwise, we didn't use it so delete it. */
      if (expr_count > 0) {
	    if (NetEvent*match = ev->find_similar_event()) {
		  delete ev;
		  wa->add_event(match);

	    } else {

		  scope->add_event(ev);
		  wa->add_event(ev);
	    }
      } else {
	    delete ev;
      }

      return wa;
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

NetProc* PForce::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lval = lval_->elaborate_net(des, path, 0, 0, 0, 0);
      if (lval == 0)
	    return 0;

      NetNet*rval = expr_->elaborate_net(des, path, lval->pin_count(),
					 0, 0, 0);
      if (rval == 0)
	    return 0;

      if (rval->pin_count() < lval->pin_count())
	    rval = pad_to_width(des, rval, lval->pin_count());

      NetForce* dev = new NetForce(des->local_symbol(path), lval);
      des->add_node(dev);

      for (unsigned idx = 0 ;  idx < dev->pin_count() ;  idx += 1)
	    connect(dev->pin(idx), rval->pin(idx));

      return dev;
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
      NetExpr*etmp;
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
      NetNet*sig = des->find_signal(scope, id1->name());
      if (sig == 0) {
	    cerr << id1->get_line() << ": register ``" << id1->name()
		 << "'' unknown in this context." << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(sig);
      NetAssign_*lv = new NetAssign_("@for-assign", sig->pin_count());
      for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1)
	    connect(lv->pin(idx), sig->pin(idx));
      des->add_node(lv);

	/* Make the r-value of the initial assignment, and size it
	   properly. Then use it to build the assignment statement. */
      etmp = expr1_->elaborate_expr(des, scope);
      etmp->set_width(lv->lwidth());

      NetAssign*init = new NetAssign(lv, etmp);

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
      sig = des->find_signal(scope, id2->name());
      assert(sig);
      lv = new NetAssign_("@for-assign", sig->pin_count());
      for (unsigned idx = 0 ;  idx < lv->pin_count() ;  idx += 1)
	    connect(lv->pin(idx), sig->pin(idx));
      des->add_node(lv);

	/* Make the rvalue of the increment expression, and size it
	   for the lvalue. */
      etmp = expr2_->elaborate_expr(des, scope);
      etmp->set_width(lv->lwidth());
      NetAssign*step = new NetAssign(lv, etmp);

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
	    if (dynamic_cast<NetEConst*>(tmp)) {
		  cerr << get_line() << ": warning: condition expression "
			"of for-loop is constant." << endl;
	    }
	    ce = tmp;
      }


	/* All done, build up the loop. */

      NetWhile*loop = new NetWhile(ce, body);
      top->append(loop);
      return top;
}

/*
 * (See the PTask::elaborate methods for basic common stuff.)
 *
 * The return value of a function is represented as a reg variable
 * within the scope of the function that has the name of the
 * function. So for example with the function:
 *
 *    function [7:0] incr;
 *      input [7:0] in1;
 *      incr = in1 + 1;
 *    endfunction
 *
 * The scope of the function is <parent>.incr and there is a reg
 * variable <parent>.incr.incr. The elaborate_1 method is called with
 * the scope of the function, so the return reg is easily located.
 *
 * The function parameters are all inputs, except for the synthetic
 * output parameter that is the return value. The return value goes
 * into port 0, and the parameters are all the remaining ports.
 */

void PFunction::elaborate(Design*des, NetScope*scope) const
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

NetProc* PRelease::elaborate(Design*des, const string&path) const
{
      NetScope*scope = des->find_scope(path);
      assert(scope);

      NetNet*lval = lval_->elaborate_net(des, path, 0, 0, 0, 0);
      if (lval == 0)
	    return 0;

      NetRelease*dev = new NetRelease(lval);
      dev->set_line( *this );
      return dev;
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
 *
 * The first elaboration pass finds the reg objects that match the
 * port names, and creates the NetTaskDef object. The port names are
 * in the form task.port.
 *
 *      task foo;
 *        output blah;
 *        begin <body> end
 *      endtask
 *
 * So in the foo example, the PWire objects that represent the ports
 * of the task will include a foo.blah for the blah port. This port is
 * bound to a NetNet object by looking up the name. All of this is
 * handled by the PTask::elaborate_sig method and the results stashed
 * in the created NetDaskDef attached to the scope.
 *
 * Elaboration pass 2 for the task definition causes the statement of
 * the task to be elaborated and attached to the NetTaskDef object
 * created in pass 1.
 *
 * NOTE: I am not sure why I bothered to prepend the task name to the
 * port name when making the port list. It is not really useful, but
 * that is what I did in pform_make_task_ports, so there it is.
 */

void PTask::elaborate(Design*des, const string&path) const
{
      NetScope*task = des->find_task(path);
      NetTaskDef*def = task->task_def();
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


	// Elaborate functions.
      typedef map<string,PFunction*>::const_iterator mfunc_it_t;
      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {

	    NetScope*fscope = scope->child((*cur).first);
	    assert(fscope);
	    (*cur).second->elaborate(des, fscope);
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
		  result_flag = false;
		  continue;
	    }

	    NetProcTop*top;
	    switch ((*st)->type()) {
		case PProcess::PR_INITIAL:
		  top = new NetProcTop(scope, NetProcTop::KINITIAL, cur);
		  break;
		case PProcess::PR_ALWAYS:
		  top = new NetProcTop(scope, NetProcTop::KALWAYS, cur);
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
      scope->time_unit(rmod->time_unit);
      scope->time_precision(rmod->time_precision);
      des->set_precision(rmod->time_precision);
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


	// With the parameters evaluated down to constants, we have
	// what we need to elaborate signals and memories. This pass
	// creates all the NetNet and NetMemory objects for declared
	// objects.
      if (! rmod->elaborate_sig(des, scope)) {
	    delete des;
	    return 0;
      }

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
 * Revision 1.217  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.216  2001/07/19 03:43:15  steve
 *  Do not connect reg to module outputs.
 *
 * Revision 1.215  2001/06/27 18:34:43  steve
 *  Report line of unsupported cassign.
 *
 * Revision 1.214  2001/05/17 03:35:22  steve
 *  do not assert if memory reference is invalid.
 *
 * Revision 1.213  2001/04/29 20:19:10  steve
 *  Add pullup and pulldown devices.
 *
 * Revision 1.212  2001/04/28 23:18:08  steve
 *  UDP instances need not have user supplied names.
 *
 * Revision 1.211  2001/04/24 02:23:58  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.210  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.209  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.208  2001/02/15 06:59:36  steve
 *  FreeBSD port has a maintainer now.
 *
 * Revision 1.207  2001/02/09 05:44:23  steve
 *  support evaluation of constant < in expressions.
 *
 * Revision 1.206  2001/02/07 21:47:13  steve
 *  Fix expression widths for rvalues and parameters (PR#131,132)
 *
 * Revision 1.205  2001/01/14 23:04:56  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.204  2001/01/10 03:13:23  steve
 *  Build task outputs as lval instead of nets. (PR#98)
 *
 * Revision 1.203  2001/01/09 05:58:47  steve
 *  Cope with width mismatches to module ports (PR#89)
 *
 * Revision 1.202  2000/12/15 01:24:17  steve
 *  Accept x in outputs of primitive. (PR#84)
 *
 * Revision 1.201  2000/12/10 22:01:36  steve
 *  Support decimal constants in behavioral delays.
 *
 * Revision 1.200  2000/12/10 06:41:59  steve
 *  Support delays on continuous assignment from idents. (PR#40)
 *
 * Revision 1.199  2000/12/06 06:31:09  steve
 *  Check lvalue of procedural continuous assign (PR#29)
 *
 * Revision 1.198  2000/12/01 23:52:49  steve
 *  Handle null statements inside a wait. (PR#60)
 *
 * Revision 1.197  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.196  2000/11/05 06:05:59  steve
 *  Handle connectsion to internally unconnected modules (PR#38)
 *
 * Revision 1.195  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.194  2000/10/26 17:09:46  steve
 *  Fix handling of errors in behavioral lvalues. (PR#28)
 *
 * Revision 1.193  2000/10/07 19:45:42  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.192  2000/09/29 22:58:57  steve
 *  Do not put noop statements into blocks.
 *
 * Revision 1.191  2000/09/24 17:41:13  steve
 *  fix null pointer when elaborating undefined task.
 *
 * Revision 1.190  2000/09/20 02:53:14  steve
 *  Correctly measure comples l-values of assignments.
 *
 * Revision 1.189  2000/09/09 15:21:26  steve
 *  move lval elaboration to PExpr virtual methods.
 *
 * Revision 1.188  2000/09/07 01:29:44  steve
 *  Fix bit padding of assign signal-to-signal
 *
 * Revision 1.187  2000/09/07 00:06:53  steve
 *  encapsulate access to the l-value expected width.
 *
 * Revision 1.186  2000/09/03 17:58:35  steve
 *  Change elaborate_lval to return NetAssign_ objects.
 *
 * Revision 1.185  2000/09/02 23:40:12  steve
 *  Pull NetAssign_ creation out of constructors.
 *
 * Revision 1.184  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.183  2000/08/18 04:38:57  steve
 *  Proper error messages when port direction is missing.
 *
 * Revision 1.182  2000/07/30 18:25:43  steve
 *  Rearrange task and function elaboration so that the
 *  NetTaskDef and NetFuncDef functions are created during
 *  signal enaboration, and carry these objects in the
 *  NetScope class instead of the extra, useless map in
 *  the Design class.
 *
 * Revision 1.181  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.180  2000/07/26 05:08:07  steve
 *  Parse disable statements to pform.
 *
 * Revision 1.179  2000/07/22 22:09:03  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.178  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.177  2000/07/07 04:53:54  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 * Revision 1.176  2000/06/13 03:24:48  steve
 *  Index in memory assign should be a NetExpr.
 *
 * Revision 1.175  2000/05/31 02:26:49  steve
 *  Globally merge redundant event objects.
 *
 * Revision 1.174  2000/05/27 19:33:23  steve
 *  Merge similar probes within a module.
 *
 * Revision 1.173  2000/05/16 04:05:16  steve
 *  Module ports are really special PEIdent
 *  expressions, because a name can be used
 *  many places in the port list.
 *
 * Revision 1.172  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.171  2000/05/08 05:28:29  steve
 *  Use bufz to make assignments directional.
 *
 * Revision 1.170  2000/05/07 21:17:21  steve
 *  non-blocking assignment to a bit select.
 *
 * Revision 1.169  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.168  2000/05/02 16:27:38  steve
 *  Move signal elaboration to a seperate pass.
 *
 * Revision 1.167  2000/05/02 03:13:31  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.166  2000/05/02 00:58:11  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.165  2000/04/28 23:12:12  steve
 *  Overly aggressive eliding of task calls.
 *
 * Revision 1.164  2000/04/28 22:17:47  steve
 *  Skip empty tasks.
 *
 * Revision 1.163  2000/04/28 16:50:53  steve
 *  Catch memory word parameters to tasks.
 *
 * Revision 1.162  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.161  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.160  2000/04/21 04:38:15  steve
 *  Bit padding in assignment to memory.
 *
 * Revision 1.159  2000/04/18 01:02:53  steve
 *  Minor cleanup of NetTaskDef.
 *
 * Revision 1.158  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 *
 * Revision 1.157  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 */

