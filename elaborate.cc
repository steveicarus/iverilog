/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elaborate.cc,v 1.7 1998/12/01 00:42:14 steve Exp $"
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

static string local_symbol(const string&path)
{
      static unsigned counter = 0;
      string result = "_L";

      strstream res;
      res << "_L" << (counter++) << ends;
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
	    NetBUFZ*cur = new NetBUFZ(local_symbol(path));

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

/* Elaborate a source wire. Generally pretty easy. */
void PWire::elaborate(Design*des, const string&path) const
{
      NetNet::Type wtype = type;
      if (wtype == NetNet::IMPLICIT)
	    wtype = NetNet::WIRE;

      unsigned wid = 1;

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

      NetNet*sig = new NetNet(path + "." + name, wtype, wid);
      sig->port_type(port_type);
      sig->set_attributes(attributes);
      des->add_signal(sig);
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

/* Elaborate a Builtin gate. These normally get translated into
   NetLogic nodes that reflect the particular logic function. */
void PGBuiltin::elaborate(Design*des, const string&path) const
{
      NetLogic*cur = 0;
      string name = get_name();
      if (name == "")
	    name = local_symbol(path);

      switch (type()) {
	  case AND:
	    cur = new NetLogic(name, pin_count(), NetLogic::AND);
	    break;
	  case NAND:
	    cur = new NetLogic(name, pin_count(), NetLogic::NAND);
	    break;
	  case NOR:
	    cur = new NetLogic(name, pin_count(), NetLogic::NOR);
	    break;
	  case NOT:
	    cur = new NetLogic(name, pin_count(), NetLogic::NOT);
	    break;
	  case OR:
	    cur = new NetLogic(name, pin_count(), NetLogic::OR);
	    break;
	  case XNOR:
	    cur = new NetLogic(name, pin_count(), NetLogic::XNOR);
	    break;
	  case XOR:
	    cur = new NetLogic(name, pin_count(), NetLogic::XOR);
	    break;
      }

      assert(cur);
      cur->delay1(get_delay());
      cur->delay2(get_delay());
      cur->delay3(get_delay());
      des->add_node(cur);

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    const PExpr*ex = pin(idx);
	    NetNet*sig = ex->elaborate_net(des, path);
	    assert(sig);
	    connect(cur->pin(idx), sig->pin(0));
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
	    my_name = local_symbol(path);
      else
	    my_name = path + "." + get_name();

	// Elaborate this instance of the module. The recursive
	// elaboration causes the module to generate a netlist with
	// the ports represented by NetNet objects. I will find them
	// later.
      rmod->elaborate(des, my_name);

	// Now connect the ports of the newly elaborated designs to
	// the expressions that are the instantiation parameters.

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

	    assert(prt->pin_count() == sig->pin_count());
	    switch (prt->port_type()) {
		case NetNet::PINPUT:
		  do_assign(des, path, prt, sig);
		  break;
		case NetNet::POUTPUT:
		  do_assign(des, path, sig, prt);
		  break;
		default:
		  assert(0);
	    }

	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig))
		  delete tmp;
      }
}

void PGModule::elaborate_udp_(Design*des, PUdp*udp, const string&path) const
{
      const string my_name = path+"."+get_name();
      NetUDP*net = new NetUDP(my_name, udp->ports.size());
      net->set_attributes(udp->attributes);

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    NetNet*sig = pin(idx)->elaborate_net(des, path);
	    if (sig == 0) {
		  cerr << "Expression too complicated for elaboration:"
		       << *pin(idx) << endl;
		  continue;
	    }

	    connect(sig->pin(0), net->pin(idx));

	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig))
		  delete tmp;
      }

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
	    gate = new NetLogic(local_symbol(path), 3, NetLogic::XOR);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

	  case '&': // AND
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(local_symbol(path), 3, NetLogic::AND);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(local_symbol(path), NetNet::WIRE);
	    osig->local_flag(true);
	    connect(gate->pin(0), osig->pin(0));
	    des->add_signal(osig);
	    des->add_node(gate);
	    break;

	  case 'e': // ==
	    assert(lsig->pin_count() == 1);
	    assert(rsig->pin_count() == 1);
	    gate = new NetLogic(local_symbol(path), 3, NetLogic::XNOR);
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    osig = new NetNet(local_symbol(path), NetNet::WIRE);
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
      NetNet*net = new NetNet(local_symbol(path), NetNet::IMPLICIT);
      net->local_flag(true);
      NetConst*tmp = new NetConst(local_symbol(path), value_->get(0));
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
	    sig = new NetNet(local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(local_symbol(path), 2, NetLogic::NOT);
	    connect(gate->pin(0), sig->pin(0));
	    connect(gate->pin(1), sub_sig->pin(0));
	    des->add_signal(sig);
	    des->add_node(gate);
	    break;

	  case '&': // Reduction AND
	    sig = new NetNet(local_symbol(path), NetNet::WIRE);
	    sig->local_flag(true);
	    gate = new NetLogic(local_symbol(path),
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
      if (text_[0] == '$')
	    return new NetEIdent(text_, 64);
      else {
	    string name = path+"."+text_;
	    NetNet*net = des->find_signal(name);
	    assert(net);
	    return new NetESignal(net);
      }
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
	    cerr << "Could not match signal: " << lval() << endl;
	    return new NetProc;
      }
      assert(reg);
      assert(reg->type() == NetNet::REG);
      assert(expr_);

      NetExpr*rval = expr_->elaborate_expr(des, path);
      assert(rval);

      NetAssign*cur = new NetAssign(reg, rval);
      des->add_node(cur);

      return cur;
}

NetProc* PBlock::elaborate(Design*des, const string&path) const
{
      NetBlock*cur = new NetBlock(NetBlock::SEQU);
      for (unsigned idx = 0 ;  idx < size() ;  idx += 1) {
	    cur->append(stat(idx)->elaborate(des, path));
      }

      return cur;
}

NetProc* PCondit::elaborate(Design*des, const string&path) const
{
      NetExpr*expr = expr_->elaborate_expr(des, path);
      NetProc*i = if_->elaborate(des, path);
      NetProc*e = else_->elaborate(des, path);

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
 * becuase execution flows through it. Thus, the NetPEvent connects
 * the structural and the behavioral.
 */
NetProc* PEventStatement::elaborate(Design*des, const string&path) const
{
      NetProc*enet = statement_->elaborate(des, path);
      NetPEvent*ev = new NetPEvent(local_symbol(path), type_, enet);

      NetNet*expr = expr_->elaborate_net(des, path);
      if (expr == 0) {
	    cerr << "Failed to elaborate expression: ";
	    expr_->dump(cerr);
	    cerr << endl;
	    exit(1);
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

void Module::elaborate(Design*des, const string&path) const
{
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
	    NetProcTop*top;
	    switch ((*st)->type()) {
		case PProcess::PR_INITIAL:
		  top = new NetProcTop(NetProcTop::KINITIAL, cur);
		  break;
		case PProcess::PR_ALWAYS:
		  top = new NetProcTop(NetProcTop::KALWAYS, cur);
		  break;
	    }

	    des->add_process(top);
      }
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
      rmod->elaborate(des, root);
      modlist = 0;
      udplist = 0;

      return des;
}


/*
 * $Log: elaborate.cc,v $
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

