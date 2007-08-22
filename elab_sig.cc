/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_sig.cc,v 1.52 2007/06/02 03:42:12 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "Module.h"
# include  "PExpr.h"
# include  "PGate.h"
# include  "PGenerate.h"
# include  "PTask.h"
# include  "PWire.h"
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"
# include  "ivl_assert.h"

/*
 * This local function checks if a named signal is connected to a
 * port. It looks in the array of ports passed, for NetEIdent objects
 * within the port_t that have a matching name.
 */
static bool signal_is_in_port(const svector<Module::port_t*>&ports,
			      NetNet*sig)
{
      perm_string name = sig->name();

      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {

	    Module::port_t*pp = ports[idx];
	      // Skip internally unconnected ports.
	    if (pp == 0)
		  continue;

	      // This port has an internal connection. In this case,
	      // the port has 0 or more NetEIdent objects concatenated
	      // together that form the port.

	      // Note that module ports should not have any heirarchy
	      // in their names: they are in the root of the module
	      // scope by definition.

	    for (unsigned cc = 0 ;  cc < pp->expr.count() ;  cc += 1) {
		  perm_string pname = peek_tail_name(pp->expr[cc]->path());
		  assert(pp->expr[cc]);
		  if (pname == name)
			return true;
	    }
      }

      return false;
}

#if 0
static NetNet*find_signal_in_scope(NetScope*scope, const hname_t&path)
{
      NetScope*cur = scope;
      unsigned idx = 0;

      while (path.peek_name(idx+1)) {
	    cur = cur->child(path.peek_name(idx));
	    if (cur == 0)
		  return 0;

	    idx += 1;
      }

      return cur->find_signal(path.peek_name(idx));
}
#endif

bool Module::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

	// Scan all the ports of the module, and make sure that each
	// is connected to wires that have port declarations.
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {
	    Module::port_t*pp = ports[idx];
	    if (pp == 0)
		  continue;

	    map<pform_name_t,PWire*>::const_iterator wt;
	    for (unsigned cc = 0 ;  cc < pp->expr.count() ;  cc += 1) {
		  pform_name_t port_path (pp->expr[cc]->path());
		  wt = wires_.find(port_path);

		  if (wt == wires_.end()) {
			cerr << get_line() << ": error: "
			     << "Port " << pp->expr[cc]->path() << " ("
			     << (idx+1) << ") of module " << name_
			     << " is not declared within module." << endl;
			des->errors += 1;
			continue;
		  }

		  if ((*wt).second->get_port_type() == NetNet::NOT_A_PORT) {
			cerr << get_line() << ": error: "
			     << "Port " << pp->expr[cc]->path() << " ("
			     << (idx+1) << ") of module " << name_
			     << " has no direction declaration."
			     << endl;
			des->errors += 1;
		  }
	    }
      }

      for (map<pform_name_t,PWire*>::const_iterator wt = wires_.begin()
		 ; wt != wires_.end() ; wt ++ ) {

	    PWire*cur = (*wt).second;
	    NetNet*sig = cur->elaborate_sig(des, scope);

	      // If this wire is a signal of the module (as opposed to
	      // a port of a function) and is a port, then check that
	      // the module knows about it. We know that the signal is
	      // the name of a signal within a subscope of a module
	      // (a task, a function, etc.) if the name for the PWire
	      // has hierarchy.

	    if (sig && (sig->scope() == scope)
		&& (cur->get_port_type() != NetNet::NOT_A_PORT)) {

		  if (! signal_is_in_port(ports, sig)) {

			cerr << cur->get_line() << ": error: Signal "
			     << sig->name() << " has a declared direction "
			     << "but is not a port." << endl;
			des->errors += 1;
		  }
	    }


	      /* If the signal is an input and is also declared as a
		 reg, then report an error. */

	    if (sig && (sig->scope() == scope)
		&& (sig->port_type() == NetNet::PINPUT)
		&& (sig->type() == NetNet::REG)) {

		  cerr << cur->get_line() << ": error: "
		       << cur->path() << " in module "
		       << scope->module_name()
		       << " declared as input and as a reg type." << endl;
		  des->errors += 1;
	    }

	    if (sig && (sig->scope() == scope)
		&& (sig->port_type() == NetNet::PINOUT)
		&& (sig->type() == NetNet::REG)) {

		  cerr << cur->get_line() << ": error: "
		       << cur->path() << " in  module "
		       << scope->module_name()
		       << " declared as inout and as a reg type." << endl;
		  des->errors += 1;
	    }

      }

	// Run through all the generate schemes to enaborate the
	// signals that they hold. Note that the generate schemes hold
	// the scopes that they instantiated, so we don't pass any
	// scope in.
      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; cur ++ ) {
	    (*cur) -> elaborate_sig(des, scope);
      }

	// Get all the gates of the module and elaborate them by
	// connecting them to the signals. The gate may be simple or
	// complex. What we are looking for is gates that are modules
	// that can create scopes and signals.

      const list<PGate*>&gl = get_gates();

      for (list<PGate*>::const_iterator gt = gl.begin()
		 ; gt != gl.end()
		 ; gt ++ ) {

	    flag &= (*gt)->elaborate_sig(des, scope);
      }


      typedef map<perm_string,PFunction*>::const_iterator mfunc_it_t;

      for (mfunc_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++) {

	    hname_t use_name ( (*cur).first );
	    NetScope*fscope = scope->child(use_name);
	    if (scope == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
		       << "Child scope for function " << (*cur).first
		       << " missing in " << scope_path(scope) << "." << endl;
		  des->errors += 1;
		  continue;
	    }

	    (*cur).second->elaborate_sig(des, fscope);
      }


	// After all the wires are elaborated, we are free to
	// elaborate the ports of the tasks defined within this
	// module. Run through them now.

      typedef map<perm_string,PTask*>::const_iterator mtask_it_t;

      for (mtask_it_t cur = tasks_.begin()
		 ; cur != tasks_.end() ;  cur ++) {
	    NetScope*tscope = scope->child( hname_t((*cur).first) );
	    assert(tscope);
	    (*cur).second->elaborate_sig(des, tscope);
      }

      return flag;
}

bool PGModule::elaborate_sig_mod_(Design*des, NetScope*scope,
				  Module*rmod) const
{
      bool flag = true;

      NetScope::scope_vec_t instance = scope->instance_arrays[get_name()];

      for (unsigned idx = 0 ;  idx < instance.count() ;  idx += 1) {
	      // I know a priori that the elaborate_scope created the scope
	      // already, so just look it up as a child of the current scope.
	    NetScope*my_scope = instance[idx];
	    assert(my_scope);

	    if (my_scope->parent() != scope) {
		  cerr << get_line() << ": internal error: "
		       << "Instance " << scope_path(my_scope)
		       << " is in parent " << scope_path(my_scope->parent())
		       << " instead of " << scope_path(scope)
		       << endl;
	    }
	    assert(my_scope->parent() == scope);

	    if (! rmod->elaborate_sig(des, my_scope))
		  flag = false;

      }

      return flag;
}

bool PGenerate::elaborate_sig(Design*des,  NetScope*container) const
{
      bool flag = true;

      typedef list<NetScope*>::const_iterator scope_list_it_t;
      for (scope_list_it_t cur = scope_list_.begin()
		 ; cur != scope_list_.end() ; cur ++ ) {

	    NetScope*scope = *cur;

	    if (scope->parent() != container)
		  continue;

	    if (debug_elaborate)
		  cerr << get_line() << ": debug: Elaborate nets in "
		       << "scope " << scope_path(*cur)
		       << " in generate " << id_number << endl;
	    flag = elaborate_sig_(des, *cur) & flag;
      }

      return flag;
}

bool PGenerate::elaborate_sig_(Design*des, NetScope*scope) const
{
	// Scan the declared PWires to elaborate the obvious signals
	// in the current scope.
      typedef map<pform_name_t,PWire*>::const_iterator wires_it_t;
      for (wires_it_t wt = wires.begin()
		 ; wt != wires.end() ;  wt ++ ) {

	    PWire*cur = (*wt).second;

	    if (debug_elaborate)
		  cerr << get_line() << ": debug: Elaborate PWire "
		       << cur->path() << " in scope " << scope_path(scope) << endl;

	    cur->elaborate_sig(des, scope);
      }

      typedef list<PGate*>::const_iterator pgate_list_it_t;
      for (pgate_list_it_t cur = gates.begin()
		 ; cur != gates.end() ;  cur ++) {
	    (*cur) ->elaborate_sig(des, scope);
      }

      return true;
}


/*
 * A function definition exists within an elaborated module. This
 * matters when elaborating signals, as the ports of the function are
 * created as signals/variables for each instance of the
 * function. That is why PFunction has an elaborate_sig method.
 */
void PFunction::elaborate_sig(Design*des, NetScope*scope) const
{
      perm_string fname = scope->basename();
      assert(scope->type() == NetScope::FUNC);

	/* Make sure the function has at least one input port. If it
	   fails this test, print an error message. Keep going so we
	   can find more errors. */
      if (ports_ == 0) {
	    cerr << get_line() << ": error: Function " << fname
		 << " has no ports." << endl;
	    cerr << get_line() << ":      : Functions must have"
		 << " at least one input port." << endl;
	    des->errors += 1;
      }

      NetNet*ret_sig = 0;

	/* Create the signals/variables of the return value and write
	   them into the function scope. */
      switch (return_type_.type) {

	  case PTF_REG:
	  case PTF_REG_S:
	    if (return_type_.range) {
		  NetExpr*me = elab_and_eval(des, scope,
					     (*return_type_.range)[0], -1);
		  assert(me);
		  NetExpr*le = elab_and_eval(des, scope,
					     (*return_type_.range)[1], -1);
		  assert(le);

		  long mnum = 0, lnum = 0;
		  if (NetEConst*tmp = dynamic_cast<NetEConst*>(me)) {
			mnum = tmp->value().as_long();
		  } else {
			cerr << me->get_line() << ": error: "
			      "Unable to evaluate constant expression "
			     << *me << "." << endl;
			des->errors += 1;
		  }

		  if (NetEConst*tmp = dynamic_cast<NetEConst*>(le)) {
			lnum = tmp->value().as_long();
		  } else {
			cerr << le->get_line() << ": error: "
			      "Unable to evaluate constant expression "
			     << *le << "." << endl;
			des->errors += 1;
		  }

		  ret_sig = new NetNet(scope, fname, NetNet::REG, mnum, lnum);

	    } else {
		  ret_sig = new NetNet(scope, fname, NetNet::REG);
	    }
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(return_type_.type == PTF_REG_S);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_LOGIC);
	    break;

	  case PTF_INTEGER:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, integer_width);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(true);
	    ret_sig->set_isint(true);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_LOGIC);
	    break;

	  case PTF_TIME:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, 64);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(false);
	    ret_sig->set_isint(false);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_LOGIC);
	    break;

	  case PTF_REAL:
	  case PTF_REALTIME:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, 1);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(true);
	    ret_sig->set_isint(false);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_REAL);
	    break;

	  default:
	    cerr << get_line() << ": internal error: I don't know how "
		 << "to deal with return type of function "
		 << scope->basename() << "." << endl;
      }

      svector<NetNet*>ports (ports_? ports_->count() : 0);

      if (ports_)
	    for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {

		    /* Parse the port name into the task name and the reg
		       name. We know by design that the port name is given
		       as two components: <func>.<port>. */

		  pform_name_t path = (*ports_)[idx]->path();
		  ivl_assert(*this, path.size() == 2);

		  perm_string pname = peek_tail_name(path);
		  perm_string ppath = peek_head_name(path);

		  if (ppath != scope->basename()) {
			cerr << get_line() << ": internal error: function "
			     << "port " << (*ports_)[idx]->path()
			     << " has wrong name for function "
			     << scope_path(scope) << "." << endl;
			des->errors += 1;
		  }

		  NetNet*tmp = scope->find_signal(pname);
		  if (tmp == 0) {
			cerr << get_line() << ": internal error: function "
			     << scope_path(scope) << " is missing port "
			     << pname << "." << endl;
			scope->dump(cerr);
			cerr << get_line() << ": Continuing..." << endl;
			des->errors += 1;
		  }

		  ports[idx] = tmp;
	    }


      NetFuncDef*def = 0;
      if (ret_sig)  def = new NetFuncDef(scope, ret_sig, ports);

      assert(def);
      scope->set_func_def(def);
}

/*
 * A task definition is a scope within an elaborated module. When we
 * are elaborating signals, the scopes have already been created, as
 * have the reg objects that are the parameters of this task. The
 * elaborate_sig method of PTask is therefore left to connect the
 * signals to the ports of the NetTaskDef definition. We know for
 * certain that signals exist (They are in my scope!) so the port
 * binding is sure to work.
 */
void PTask::elaborate_sig(Design*des, NetScope*scope) const
{
      assert(scope->type() == NetScope::TASK);

      svector<NetNet*>ports (ports_? ports_->count() : 0);
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {

	      /* Parse the port name into the task name and the reg
		 name. We know by design that the port name is given
		 as two components: <task>.<port>. */

	    pform_name_t path = (*ports_)[idx]->path();
	    ivl_assert(*this, path.size() == 2);

	    perm_string scope_name = peek_head_name(path);
	    perm_string port_name = peek_tail_name(path);

	      /* check that the current scope really does have the
		 name of the first component of the task port name. Do
		 this by looking up the task scope in the parent of
		 the current scope. */
	    ivl_assert(*this, scope->basename() == scope_name);

	      /* Find the signal for the port. We know by definition
		 that it is in the scope of the task, so look only in
		 the scope. */
	    NetNet*tmp = scope->find_signal(port_name);

	    if (tmp == 0) {
		  cerr << get_line() << ": internal error: "
		       << "Could not find port " << port_name
		       << " in scope " << scope_path(scope) << endl;
		  scope->dump(cerr);
	    }

	    ports[idx] = tmp;
      }

      NetTaskDef*def = new NetTaskDef(scope, ports);
      scope->set_task_def(def);
}

bool PGate::elaborate_sig(Design*des, NetScope*scope) const
{
      return true;
}

/*
 * Elaborate a source wire. The "wire" is the declaration of wires,
 * registers, ports and memories. The parser has already merged the
 * multiple properties of a wire (i.e., "input wire") so come the
 * elaboration this creates an object in the design that represent the
 * defined item.
 */
NetNet* PWire::elaborate_sig(Design*des, NetScope*scope) const
{

	/* The parser may produce hierarchical names for wires. I here
	   follow the scopes down to the base where I actually want to
	   elaborate the NetNet object. */
      { pform_name_t tmp_path = hname_;
        tmp_path.pop_back();
	while (! tmp_path.empty()) {
	      name_component_t cur = tmp_path.front();
	      tmp_path.pop_front();

	      scope = scope->child( hname_t(cur.name) );

	      if (scope == 0) {
		    cerr << get_line() << ": internal error: "
			 << "Bad scope component for name "
			 << hname_ << endl;
		    assert(scope);
	      }
	}
      }

      NetNet::Type wtype = type_;
      if (wtype == NetNet::IMPLICIT)
	    wtype = NetNet::WIRE;
      if (wtype == NetNet::IMPLICIT_REG)
	    wtype = NetNet::REG;

      unsigned wid = 1;
      long lsb = 0, msb = 0;

      des->errors += error_cnt_;

      if (port_set_ || net_set_) {
	    long pmsb = 0, plsb = 0, nmsb = 0, nlsb = 0;
	    /* If they exist get the port definition MSB and LSB */
	    if (port_set_ && port_msb_ != 0) {
		  NetExpr*texpr = elab_and_eval(des, scope, port_msb_, -1);

		  if (! eval_as_long(pmsb, texpr)) {
			cerr << port_msb_->get_line() << ": error: "
			      "Unable to evaluate MSB constant expression ``"
			     << *port_msb_ << "''." << endl;
			des->errors += 1;
			return 0;
		  }

		  delete texpr;

		  texpr = elab_and_eval(des, scope, port_lsb_, -1);

		  if (! eval_as_long(plsb, texpr)) {
			cerr << port_lsb_->get_line() << ": error: "
			      "Unable to evaluate LSB constant expression ``"
			     << *port_lsb_ << "''." << endl;
			des->errors += 1;
			return 0;
		  }

		  delete texpr;
		  nmsb = pmsb;
		  nlsb = plsb;
	    }
            if (port_msb_ == 0) assert(port_lsb_ == 0);

	    /* If they exist get the net/etc. definition MSB and LSB */
	    if (net_set_ && net_msb_ != 0) {
		  NetExpr*texpr = elab_and_eval(des, scope, net_msb_, -1);

		  if (! eval_as_long(nmsb, texpr)) {
			cerr << net_msb_->get_line() << ": error: "
			      "Unable to evaluate MSB constant expression ``"
			     << *net_msb_ << "''." << endl;
			des->errors += 1;
			return 0;
		  }

		  delete texpr;

		  texpr = elab_and_eval(des, scope, net_lsb_, -1);

		  if (! eval_as_long(nlsb, texpr)) {
			cerr << net_lsb_->get_line() << ": error: "
			      "Unable to evaluate LSB constant expression ``"
			     << *net_lsb_ << "''." << endl;
			des->errors += 1;
			return 0;
		  }

		  delete texpr;
	    }
            if (net_msb_ == 0) assert(net_lsb_ == 0);

	    /* We have a port size error */
            if (port_set_ && net_set_ && (pmsb != nmsb || plsb != nlsb)) {

		  /* Scalar port with a vector net/etc. definition */
		  if (port_msb_ == 0) {
			if (!gn_io_range_error_flag) {
			      cerr << get_line()
			           << ": warning: Scalar port ``" << hname_
			           << "'' has a vectored net declaration ["
			           << nmsb << ":" << nlsb << "]." << endl;
			} else {
			      cerr << get_line()
			           << ": error: Scalar port ``" << hname_
			           << "'' has a vectored net declaration ["
			           << nmsb << ":" << nlsb << "]." << endl;
			      des->errors += 1;
			      return 0;
			}
		  }

		  /* Vectored port with a scalar net/etc. definition */
		  if (net_msb_ == 0) {
			cerr << port_msb_->get_line()
			     << ": error: Vectored port ``"
			     << hname_ << "'' [" << pmsb << ":" << plsb
			     << "] has a scalar net declaration at "
			     << get_line() << "." << endl;
			des->errors += 1;
			return 0;
		  }

		  /* Both vectored, but they have different ranges. */
		  if (port_msb_ != 0 && net_msb_ != 0) {
			cerr << port_msb_->get_line()
			     << ": error: Vectored port ``"
			     << hname_ << "'' [" << pmsb << ":" << plsb
			     << "] has a net declaration [" << nmsb << ":"
			     << nlsb << "] at " << net_msb_->get_line()
			     << " that does not match." << endl;
			des->errors += 1;
			return 0;
		  }
            }

	    lsb = nlsb;
	    msb = nmsb;
	    if (nmsb > nlsb)
		  wid = nmsb - nlsb + 1;
	    else
		  wid = nlsb - nmsb + 1;


      }

      unsigned nattrib = 0;
      attrib_list_t*attrib_list = evaluate_attributes(attributes, nattrib,
						      des, scope);

      long array_s0 = 0;
      long array_e0 = 0;
      unsigned array_dimensions = 0;

	/* If the ident has idx expressions, then this is a
	   memory. It can only have the idx registers after the msb
	   and lsb expressions are filled. And, if it has one index,
	   it has both. */
      if (lidx_ || ridx_) {
	    assert(lidx_ && ridx_);

	    NetExpr*lexp = elab_and_eval(des, scope, lidx_, -1);
	    NetExpr*rexp = elab_and_eval(des, scope, ridx_, -1);

	    if ((lexp == 0) || (rexp == 0)) {
		  cerr << get_line() << ": internal error: There is "
		       << "a problem evaluating indices for ``"
		       << hname_ << "''." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEConst*lcon = dynamic_cast<NetEConst*> (lexp);
	    NetEConst*rcon = dynamic_cast<NetEConst*> (rexp);

	    if ((lcon == 0) || (rcon == 0)) {
		  cerr << get_line() << ": internal error: The indices "
		       << "are not constant for array ``"
		       << hname_ << "''." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    verinum lval = lcon->value();
	    verinum rval = rcon->value();

	    delete lexp;
	    delete rexp;

	    array_dimensions = 1;
	    array_s0 = lval.as_long();
	    array_e0 = rval.as_long();
      }

	/* If the net type is supply0 or supply1, replace it
	   with a simple wire with a pulldown/pullup with supply
	   strength. In other words, transform:

	   supply0 foo;

	   to:

	   wire foo;
	   pulldown #(supply0) (foo);

	   This reduces the backend burden, and behaves exactly
	   the same. */

      NetLogic*pull = 0;
      if (wtype == NetNet::SUPPLY0 || wtype == NetNet::SUPPLY1) {
	    NetLogic::TYPE pull_type = (wtype==NetNet::SUPPLY1)
		  ? NetLogic::PULLUP
		  : NetLogic::PULLDOWN;
	    pull = new NetLogic(scope, scope->local_symbol(),
				1, pull_type, wid);
	    pull->set_line(*this);
	    pull->pin(0).drive0(Link::SUPPLY);
	    pull->pin(0).drive1(Link::SUPPLY);
	    des->add_node(pull);
	    wtype = NetNet::WIRE;

	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: "
		       << "Generate a SUPPLY pulldown for the "
		       << "supply0 net." << endl;
	    }
      }

      perm_string name = peek_tail_name(hname_);
      if (debug_elaborate) {
	    cerr << get_line() << ": debug: Create signal "
		 << wtype << " ["<<msb<<":"<<lsb<<"] " << name
		 << " in scope " << scope_path(scope) << endl;
      }


      NetNet*sig = array_dimensions > 0
	    ? new NetNet(scope, name, wtype, msb, lsb, array_s0, array_e0)
	    : new NetNet(scope, name, wtype, msb, lsb);

      ivl_variable_type_t use_data_type = data_type_;
      if (use_data_type == IVL_VT_NO_TYPE) {
	    use_data_type = IVL_VT_LOGIC;
	    if (debug_elaborate) {
		  cerr << get_line() << ": debug: "
		       << "Signal " << name
		       << " in scope " << scope_path(scope)
		       << " defaults to data type " << use_data_type << endl;
	    }
      }

      sig->data_type(use_data_type);
      sig->set_line(*this);
      sig->port_type(port_type_);
      sig->set_signed(get_signed());
      sig->set_isint(get_isint());

      if (pull)
	    connect(sig->pin(0), pull->pin(0));

      for (unsigned idx = 0 ;  idx < nattrib ;  idx += 1)
	    sig->attribute(attrib_list[idx].key, attrib_list[idx].val);

      return sig;
}

