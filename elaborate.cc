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
#ident "$Id: elaborate.cc,v 1.21 1999/04/29 02:16:26 steve Exp $"
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

string Design::local_symbol(const string&path)
{
      string result = "_L";

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
      NetNet::Type wtype = type;
      if (wtype == NetNet::IMPLICIT)
	    wtype = NetNet::WIRE;

      unsigned wid = 1;

	/* Wires, registers and memories can have a width, expressed
	   as the msb index and lsb index. */
      if (msb && lsb) {
	    verinum*mval = msb->eval_const();
	    assert(mval);
	    verinum*lval = lsb->eval_const();
	    assert(lval);

	    long mnum = mval->as_long();
	    long lnum = lval->as_long();
	    delete mval;
	    delete lval;

	    if (mnum > lnum)
		  wid = mnum - lnum + 1;
	    else
		  wid = lnum - mnum + 1;

      } else if (msb) {
	    verinum*val = msb->eval_const();
	    assert(val);
	    assert(val->as_ulong() > 0);
	    wid = val->as_ulong();
      }

      if (lidx || ridx) {
	      // If the register has indices, then this is a
	      // memory. Create the memory object.
	    verinum*lval = lidx->eval_const();
	    assert(lval);
	    verinum*rval = ridx->eval_const();
	    assert(rval);

	    long lnum = lval->as_long();
	    long rnum = rval->as_long();
	    delete lval;
	    delete rval;
	    NetMemory*sig = new NetMemory(path+"."+name, wid, lnum, rnum);
	    sig->set_attributes(attributes);
	    des->add_memory(sig);

      } else {

	    NetNet*sig = new NetNet(path + "." + name, wtype, wid);
	    sig->port_type(port_type);
	    sig->set_attributes(attributes);
	    des->add_signal(sig);
      }
}

void PGate::elaborate(Design*des, const string&path) const
{
      cerr << "what kind of gate? " << typeid(*this).name() << endl;
}

/* Elaborate the continuous assign. (This is *not* the procedural
   assign.) Elaborate the lvalue and rvalue, and do the assignment. */
void PGAssign::elaborate(Design*des, const string&path) const
{
      NetNet*lval = pin(0)->elaborate_net(des, path);
      NetNet*rval = pin(1)->elaborate_net(des, path);
      assert(lval && rval);

      do_assign(des, path, lval, rval);
}

/*
 * Elaborate a Builtin gate. These normally get translated into
 * NetLogic nodes that reflect the particular logic function.
 */
void PGBuiltin::elaborate(Design*des, const string&path) const
{
      unsigned count = 1;
      unsigned low, high;
      string name = get_name();
      if (name == "")
	    name = des->local_symbol(path);

	/* If the verilog source has a range specification for the
	   gates, then I am expected to make more then one
	   gate. Figure out how many are desired. */
      if (msb_) {
	    verinum*msb = msb_->eval_const();
	    verinum*lsb = lsb_->eval_const();

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

	    cur[idx]->delay1(get_delay());
	    cur[idx]->delay2(get_delay());
	    cur[idx]->delay3(get_delay());
	    des->add_node(cur[idx]);
      }

	/* The gates have all been allocated, this loop runs through
	   the parameters and attaches the ports of the objects. */

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    const PExpr*ex = pin(idx);
	    NetNet*sig = ex->elaborate_net(des, path);
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
      string my_name;
      if (get_name() == "")
	    my_name = des->local_symbol(path);
      else
	    my_name = path + "." + get_name();

	// Elaborate this instance of the module. The recursive
	// elaboration causes the module to generate a netlist with
	// the ports represented by NetNet objects. I will find them
	// later.
      rmod->elaborate(des, my_name);

	// Now connect the ports of the newly elaborated designs to
	// the expressions that are the instantiation parameters.

      if (pin_count() != rmod->ports.size()) {
	    cerr << get_line() << ": Wrong number "
		  "of parameters. Expecting " << rmod->ports.size() <<
		  ", got " << pin_count() << "."
		 << endl;
	    des->errors += 1;
	    return;
      }

      assert(pin_count() == rmod->ports.size());

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	      // Skip unconnected module ports.
	    if (pin(idx) == 0)
		  continue;
	    NetNet*sig = pin(idx)->elaborate_net(des, path);
	    if (sig == 0) {
		  cerr << "Expression too complicated for elaboration." << endl;
		  continue;
	    }

	    assert(sig);
	    NetNet*prt = des->find_signal(my_name + "." +
					  rmod->ports[idx]->name);
	    assert(prt);

	      // Check that the parts have matching pin counts. If
	      // not, they are different widths.
	    if (prt->pin_count() != sig->pin_count()) {
		  cerr << get_line() << ": Port " <<
			rmod->ports[idx]->name << " of " << type_ <<
			" expects " << prt->pin_count() << " pins, got " <<
			sig->pin_count() << " from " << sig->name() << endl;
		  des->errors += 1;
		  continue;
	    }

	    assert(prt->pin_count() == sig->pin_count());
	    switch (prt->port_type()) {
		    // INPUT and OUTPUT ports are directional. Handle
		    // them like assignments.
		case NetNet::PINPUT:
		  do_assign(des, path, prt, sig);
		  break;
		case NetNet::POUTPUT:
		  do_assign(des, path, sig, prt);
		  break;

		    // INOUT ports are like terminal posts. Just
		    // connect the inside and the outside nets
		    // together.
		case NetNet::PINOUT:
		  for (unsigned p = 0 ;  p < sig->pin_count() ;  p += 1)
			connect(prt->pin(p), sig->pin(p));
		  break;
		default:
		  assert(0);
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
      NetUDP*net = new NetUDP(my_name, udp->ports.size(), udp->sequential);
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
      for (unsigned idx = 0 ;  idx < udp->tinput.size() ;  idx += 1) {
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

      cerr << "Unknown module: " << type_ << endl;
}

NetNet* PExpr::elaborate_net(Design*des, const string&path) const
{
      cerr << "Don't know how to elaborate `" << *this << "' as gates." << endl;
      return 0;
}

/*
 * Elaborating binary operations generally involves elaborating the
 * left and right expressions, then making an output wire and
 * connecting the lot together with the right kind of gate.
 */
NetNet* PEBinary::elaborate_net(Design*des, const string&path) const
{
      NetNet*lsig = left_->elaborate_net(des, path),
	    *rsig = right_->elaborate_net(des, path);
      if (lsig == 0) {
	    cerr << "Cannot elaborate ";
	    left_->dump(cerr);
	    cerr << endl;
	    return 0;
      }
      if (rsig == 0) {
	    cerr << "Cannot elaborate ";
	    right_->dump(cerr);
	    cerr << endl;
	    return 0;
      }

      NetNet*osig;
      NetLogic*gate;

      switch (op_) {
	  case '^': // XOR
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::XOR);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

	  case '&': // AND
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::AND);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

	  case 'e': // ==
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::XNOR);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

	  case 'n': // !=
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(des->local_symbol(path), 3, NetLogic::XOR);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

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

NetNet* PEIdent::elaborate_net(Design*des, const string&path) const
{
      NetNet*sig = des->find_signal(path+"."+text_);

      if (msb_ && lsb_) {
	    verinum*mval = msb_->eval_const();
	    assert(mval);
	    verinum*lval = lsb_->eval_const();
	    assert(lval);
	    unsigned midx = sig->sb_to_idx(mval->as_long());
	    unsigned lidx = sig->sb_to_idx(lval->as_long());

	    if (midx >= lidx) {
		  NetTmp*tmp = new NetTmp(midx-lidx+1);
		  for (unsigned idx = lidx ;  idx <= midx ;  idx += 1)
			connect(tmp->pin(idx-lidx), sig->pin(idx));

		  sig = tmp;

	    } else {
		  NetTmp*tmp = new NetTmp(lidx-midx+1);
		  for (unsigned idx = lidx ;  idx >= midx ;  idx -= 1)
			connect(tmp->pin(idx-midx), sig->pin(idx));

		  sig = tmp;
	    }

      } else if (msb_) {
	    verinum*mval = msb_->eval_const();
	    assert(mval);
	    unsigned idx = sig->sb_to_idx(mval->as_long());
	    NetTmp*tmp = new NetTmp(1);
	    connect(tmp->pin(0), sig->pin(idx));
	    sig = tmp;
      }

      return sig;
}

/*
 * XXXX For now, only generate a single bit. I am going to have to add
 * code to properly calculate expression bit widths, eventually.
 */
NetNet* PENumber::elaborate_net(Design*des, const string&path) const
{
      NetNet*net = new NetNet(des->local_symbol(path), NetNet::IMPLICIT);
      net->local_flag(true);
      NetConst*tmp = new NetConst(des->local_symbol(path), value_->get(0));
      des->add_node(tmp);
      des->add_signal(net);
      connect(net->pin(0), tmp->pin(0));
      return net;
}

NetNet* PEUnary::elaborate_net(Design*des, const string&path) const
{
      NetNet* sub_sig = expr_->elaborate_net(des, path);
      assert(sub_sig);

      NetNet* sig;
      NetLogic*gate;
      switch (op_) {
	  case '~': // Bitwise NOT
	    assert(sub_sig->pin_count() == 1);
	    sig = new NetNet(des->local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(des->local_symbol(path), 2, NetLogic::NOT);
	    connect(gate->pin(0), sig->pin(0));
	    connect(gate->pin(1), sub_sig->pin(0));
	    des->add_signal(sig);
	    des->add_node(gate);
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
      return new NetEBinary(op_, left_->elaborate_expr(des, path),
			    right_->elaborate_expr(des, path));
}

NetExpr* PENumber::elaborate_expr(Design*des, const string&path) const
{
      assert(value_);
      return new NetEConst(*value_);
}

NetExpr* PEString::elaborate_expr(Design*des, const string&path) const
{
      return new NetEConst(value());
}

NetExpr*PEIdent::elaborate_expr(Design*des, const string&path) const
{
	// System identifiers show up in the netlist as identifiers.
      if (text_[0] == '$')
	    return new NetEIdent(text_, 64);

      string name = path+"."+text_;

	// If the identifier name a paramter name, then return
	// the expression that it represents.
      if (NetExpr*ex = des->get_parameter(name))
	    return ex;

	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (NetNet*net = des->find_signal(name)) {
	    NetESignal*node = des->get_esignal(net);
	    assert(idx_ == 0);
	    assert(lsb_ == 0);
	    if (msb_) {
		  NetExpr*ex = msb_->elaborate_expr(des, path);
		  NetESubSignal*ss = new NetESubSignal(node, ex);
		  return ss;
	    }
	    assert(msb_ == 0);
	    return node;
      }

	// If the identifier names a memory, then this is a
	// memory reference and I must generate a NetEMemory
	// object to handle it.
      if (NetMemory*mem = des->find_memory(name)) {
	    assert(msb_ != 0);
	    assert(lsb_ == 0);
	    assert(idx_ == 0);
	    NetExpr*i = msb_->elaborate_expr(des, path);
	    if (i == 0) {
		  cerr << get_line() << ": Unable to exaborate "
			"index expression `" << *msb_ << "'" << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEMemory*node = new NetEMemory(mem, i);
	    return node;
      }

	// I cannot interpret this identifier. Error message.
      cerr << get_line() << ": Unable to bind wire/reg/memory "
	    "`" << path << "." << text_ << "'" << endl;
      des->errors += 1;
      return 0;
}

NetExpr* PExpr::elaborate_expr(Design*des, const string&path) const
{
      cerr << "Cannot elaborate expression: " << *this << endl;
      return new NetEConst(verinum());
}

NetExpr* PEUnary::elaborate_expr(Design*des, const string&path) const
{
      return new NetEUnary(op_, expr_->elaborate_expr(des, path));
}

NetProc* Statement::elaborate(Design*des, const string&path) const
{
      cerr << "elaborate: What kind of statement? " <<
	    typeid(*this).name() << endl;
      NetProc*cur = new NetProc;
      return cur;
}

NetProc* PAssign::elaborate(Design*des, const string&path) const
{
      NetNet*reg = des->find_signal(path+"."+lval());
      if (reg == 0) {
	    cerr << get_line() << ": Could not match signal: " <<
		  lval() << endl;
	    return 0;
      }
      assert(reg);

      if (reg->type() != NetNet::REG) {
	    cerr << get_line() << ": " << lval() << " is not a register."
		 << endl;
	    return 0;
      }
      assert(reg->type() == NetNet::REG);
      assert(expr_);

      NetExpr*rval = expr_->elaborate_expr(des, path);
      if (rval == 0) {
	    cerr << get_line() << ": " << "failed to elaborate expression."
		 << endl;
	    return 0;
      }
      assert(rval);

      NetAssign*cur = new NetAssign(reg, rval);
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
      NetBlock*cur = new NetBlock(NetBlock::SEQU);
      bool fail_flag = false;

      for (unsigned idx = 0 ;  idx < size() ;  idx += 1) {
	    NetProc*tmp = stat(idx)->elaborate(des, path);
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

NetProc* PCase::elaborate(Design*des, const string&path) const
{
      NetExpr*expr = expr_->elaborate_expr(des, path);
      NetCase*res = new NetCase(expr, nitems_);

      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    NetExpr*gu = 0;
	    NetProc*st = 0;
	    if (items_[idx].expr)
		  gu = items_[idx].expr->elaborate_expr(des, path);

	    if (items_[idx].stat)
		  st = items_[idx].stat->elaborate(des, path);

	    res->set_case(idx, gu, st);
      }

      return res;
}

NetProc* PCondit::elaborate(Design*des, const string&path) const
{
      NetExpr*expr = expr_->elaborate_expr(des, path);
      NetProc*i = if_->elaborate(des, path);
      NetProc*e = else_? else_->elaborate(des, path) : 0;

      NetCondit*res = new NetCondit(expr, i, e);
      return res;
}

NetProc* PCallTask::elaborate(Design*des, const string&path) const
{
      NetTask*cur = new NetTask(name(), nparms());

      for (unsigned idx = 0 ;  idx < nparms() ;  idx += 1) {
	    PExpr*ex = parm(idx);
	    cur->parm(idx, ex? ex->elaborate_expr(des, path) : 0);
      }

      return cur;
}

NetProc* PDelayStatement::elaborate(Design*des, const string&path) const
{
      verinum*num = delay_->eval_const();
      assert(num);

      unsigned long val = num->as_ulong();
      return new NetPDelay(val, statement_->elaborate(des, path));
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
NetProc* PEventStatement::elaborate(Design*des, const string&path) const
{
      NetProc*enet = 0;
      if (statement_) {
	    enet = statement_->elaborate(des, path);
	    if (enet == 0)
		  return 0;
      }

      if (expr_.count() != 1) {
	    cerr << get_line() << ": Sorry, unable to elaborate event "
		  "OR expressions." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetPEvent*ev = new NetPEvent(des->local_symbol(path),
				   expr_[0]->type(), enet);

      NetNet*expr = expr_[0]->expr()->elaborate_net(des, path);
      if (expr == 0) {
	    cerr << get_line() << ": Failed to elaborate expression: ";
	    expr_[0]->dump(cerr);
	    cerr << endl;
	    delete ev;
	    return 0;
      }
      assert(expr);
      connect(ev->pin(0), expr->pin(0));

      des->add_node(ev);
      return ev;
}

/*
 * elaborate the for loop as the equivilent while loop. This eases the
 * task for the target code generator. The structure is:
 *
 *     begin
 *       name1_ = expr1_;
 *       while (cond_) begin
 *          statement_;
 *          name2_ = expr2_;
 *       end
 *     end
 */
NetProc* PForStatement::elaborate(Design*des, const string&path) const
{
      NetBlock*top = new NetBlock(NetBlock::SEQU);
      NetNet*sig = des->find_signal(path+"."+name1_);
      assert(sig);
      NetAssign*init = new NetAssign(sig, expr1_->elaborate_expr(des, path));
      top->append(init);

      NetBlock*body = new NetBlock(NetBlock::SEQU);

      body->append(statement_->elaborate(des, path));

      sig = des->find_signal(path+"."+name2_);
      assert(sig);
      NetAssign*step = new NetAssign(sig, expr2_->elaborate_expr(des, path));
      body->append(step);

      NetWhile*loop = new NetWhile(cond_->elaborate_expr(des, path), body);
      top->append(loop);
      return top;
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

bool Module::elaborate(Design*des, const string&path) const
{
      bool result_flag = true;

	// Generate all the parameters that this instance of this
	// module introduce to the design.
      typedef map<string,PExpr*>::const_iterator mparm_it_t;
      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {
	    string pname = path + "." + (*cur).first;
	    NetExpr*expr = (*cur).second->elaborate_expr(des, path);
	    des->set_parameter(pname, expr);
      }

	// Get all the explicitly declared wires of the module and
	// start the signals list with them.
      const list<PWire*>&wl = get_wires();

      for (list<PWire*>::const_iterator wt = wl.begin()
		 ; wt != wl.end()
		 ; wt ++ ) {

	    (*wt)->elaborate(des, path);
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
      bool rc = rmod->elaborate(des, root);
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
 * Revision 1.21  1999/04/29 02:16:26  steve
 *  Parse OR of event expressions.
 *
 * Revision 1.20  1999/04/25 00:44:10  steve
 *  Core handles subsignal expressions.
 *
 * Revision 1.19  1999/04/19 01:59:36  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.18  1999/03/15 02:43:32  steve
 *  Support more operators, especially logical.
 *
 * Revision 1.17  1999/03/01 03:27:53  steve
 *  Prevent the duplicate allocation of ESignal objects.
 *
 * Revision 1.16  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.15  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.14  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.13  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
 * Revision 1.12  1999/02/01 00:26:49  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.11  1999/01/25 05:45:56  steve
 *  Add the LineInfo class to carry the source file
 *  location of things. PGate, Statement and PProcess.
 *
 *  elaborate handles module parameter mismatches,
 *  missing or incorrect lvalues for procedural
 *  assignment, and errors are propogated to the
 *  top of the elaboration call tree.
 *
 *  Attach line numbers to processes, gates and
 *  assignment statements.
 *
 * Revision 1.10  1998/12/14 02:01:34  steve
 *  Fully elaborate Sequential UDP behavior.
 *
 * Revision 1.9  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.8  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.7  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.6  1998/11/23 00:20:22  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.5  1998/11/21 19:19:44  steve
 *  Give anonymous modules a name when elaborated.
 *
 * Revision 1.4  1998/11/11 03:13:04  steve
 *  Handle while loops.
 *
 * Revision 1.3  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:56  steve
 *  Introduce verilog to CVS.
 *
 */

