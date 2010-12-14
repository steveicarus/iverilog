/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  <iostream>
# include  <cstdlib>

# include  "Module.h"
# include  "PExpr.h"
# include  "PGate.h"
# include  "PTask.h"
# include  "PWire.h"
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"

/*
 * This local function checks if a named signal is connected to a
 * port. It looks in the array of ports passed, for NetEIdent objects
 * within the port_t that have a matching name.
 */
static bool signal_is_in_port(const svector<Module::port_t*>&ports,
			      const hname_t&name)
{
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {

	    Module::port_t*pp = ports[idx];
	      // Skip internally unconnected ports.
	    if (pp == 0)
		  continue;

	      // This port has an internal connection. In this case,
	      // the port has 0 or more NetEIdent objects concatenated
	      // together that form the port.
	    for (unsigned cc = 0 ;  cc < pp->expr.count() ;  cc += 1) {
		  assert(pp->expr[cc]);
		  if (pp->expr[cc]->path() == name)
			return true;
	    }
      }

      return false;
}

bool Module::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

	// Get all the explicitly declared wires of the module and
	// start the signals list with them.
      const map<hname_t,PWire*>&wl = get_wires();

	// Scan all the ports of the module, and make sure that each
	// is connected to wires that have port declarations.
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {
	    Module::port_t*pp = ports[idx];
	    if (pp == 0)
		  continue;

	    map<hname_t,PWire*>::const_iterator wt;
	    for (unsigned cc = 0 ;  cc < pp->expr.count() ;  cc += 1) {
		  hname_t port_path (pp->expr[cc]->path());
		  wt = wl.find(port_path);

		  if (wt == wl.end()) {
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

      for (map<hname_t,PWire*>::const_iterator wt = wl.begin()
		 ; wt != wl.end()
		 ; wt ++ ) {

	    PWire*cur = (*wt).second;
	    cur->elaborate_sig(des, scope);

	    NetNet*sig = scope->find_signal_in_child(cur->path());

	      // If this wire is a signal of the module (as opposed to
	      // a port of a function) and is a port, then check that
	      // the module knows about it. We know that the signal is
	      // the name of a signal within a subscope of a module
	      // (a task, a function, etc.) if the name for the PWire
	      // has hierarchy.

	    if (sig && (sig->scope() == scope)
		&& (cur->get_port_type() != NetNet::NOT_A_PORT)) {

		  hname_t name = (*wt).first;

		  if (! signal_is_in_port(ports, name)) {

			cerr << cur->get_line() << ": error: Signal "
			     << name << " has a declared direction "
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
	    NetScope*fscope = scope->child((*cur).first);
	    if (scope == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
		       << "Child scope for function " << (*cur).first
		       << " missing in " << scope->name() << "." << endl;
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
	    NetScope*tscope = scope->child((*cur).first);
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
		       << "Instance " << my_scope->name()
		       << " is in parent " << my_scope->parent()->name()
		       << " instead of " << scope->name()
		       << endl;
	    }
	    assert(my_scope->parent() == scope);

	    if (! rmod->elaborate_sig(des, my_scope))
		  flag = false;

      }

      return flag;
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
      NetVariable*ret_real = 0;

	/* Create the signals/variables of the return value and write
	   them into the function scope. */
      switch (return_type_.type) {

	  case PTF_REG:
	    if (return_type_.range) {
		  NetExpr*me = elab_and_eval(des, scope,
					     (*return_type_.range)[0]);
		  assert(me);
		  NetExpr*le = elab_and_eval(des, scope,
					     (*return_type_.range)[1]);
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
	    ret_sig->port_type(NetNet::POUTPUT);
	    break;

	  case PTF_INTEGER:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, INTEGER_WIDTH);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(true);
	    ret_sig->set_isint(true);
	    ret_sig->port_type(NetNet::POUTPUT);
	    break;

	  case PTF_TIME:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, 64);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(false);
	    ret_sig->set_isint(false);
	    ret_sig->port_type(NetNet::POUTPUT);
	    break;

	  case PTF_REAL:
	  case PTF_REALTIME:
	    ret_real = new NetVariable(fname);
	    ret_real->set_line(*this);
	    scope->add_variable(ret_real);
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

		  hname_t path = (*ports_)[idx]->path();
		  perm_string pname = lex_strings.make(path.peek_name(1));
		  perm_string ppath = lex_strings.make(path.peek_name(0));

		  if (ppath != scope->basename()) {
			cerr << get_line() << ": internal error: function "
			     << "port " << (*ports_)[idx]->path()
			     << " has wrong name for function "
			     << scope->name() << "." << endl;
			des->errors += 1;
		  }

		  NetNet*tmp = scope->find_signal(pname);
		  if (tmp == 0) {
			cerr << get_line() << ": internal error: function "
			     << scope->name() << " is missing port "
			     << pname << "." << endl;
			scope->dump(cerr);
			cerr << get_line() << ": Continuing..." << endl;
			des->errors += 1;
		  }

		  ports[idx] = tmp;
	    }


      NetFuncDef*def = 0;
      if (ret_sig)  def = new NetFuncDef(scope, ret_sig, ports);
      if (ret_real) def = new NetFuncDef(scope, ret_real, ports);

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

	    hname_t path = (*ports_)[idx]->path();
	    assert(path.peek_name(0) && path.peek_name(1));

	      /* check that the current scope really does have the
		 name of the first component of the task port name. Do
		 this by looking up the task scope in the parent of
		 the current scope. */
	    if (scope->parent()->child(path.peek_name(0)) != scope) {
		  cerr << "internal error: task scope " << path
		       << " not the same as scope " << scope->name()
		       << "?!" << endl;
		  return;
	    }

	      /* Find the signal for the port. We know by definition
		 that it is in the scope of the task, so look only in
		 the scope. */
	    NetNet*tmp = scope->find_signal(path.peek_name(1));

	    if (tmp == 0) {
		  cerr << get_line() << ": internal error: "
		       << "Could not find port " << path.peek_name(1)
		       << " in scope " << scope->name() << endl;
		  scope->dump(cerr);
	    }

	    ports[idx] = tmp;
      }

      NetTaskDef*def = new NetTaskDef(scope->name(), ports);
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
void PWire::elaborate_sig(Design*des, NetScope*scope) const
{

	/* The parser may produce hierarchical names for wires. I here
	   follow the scopes down to the base where I actually want to
	   elaborate the NetNet object. */
      { hname_t tmp_path = hname_;
        free(tmp_path.remove_tail_name());
	for (unsigned idx = 0 ;  tmp_path.peek_name(idx) ;  idx += 1) {
	      scope = scope->child(tmp_path.peek_name(idx));

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

      assert(msb_.count() == lsb_.count());
      if (msb_.count()) {
	    svector<long>mnum (msb_.count());
	    svector<long>lnum (msb_.count());
	      /* There may be places where the signal is declared as a
		 scalar. Count those here, for consistency check
		 later. */
	    unsigned count_scalars = 0;

	      /* There may be multiple declarations of ranges, because
		 the symbol may have its range declared in e.g., input
		 and reg declarations. Calculate *all* the numbers
		 here. I will resolve the values later. */

	    for (unsigned idx = 0 ;  idx < msb_.count() ;  idx += 1) {

		  if (msb_[idx] == 0) {
			count_scalars += 1;
			assert(lsb_[idx] == 0);
			mnum[idx] = 0;
			lnum[idx] = 0;
			continue;
		  }

		  NetEConst*tmp;
		  NetExpr*texpr = elab_and_eval(des, scope, msb_[idx]);

		  tmp = dynamic_cast<NetEConst*>(texpr);
		  if (tmp == 0) {
			cerr << msb_[idx]->get_line() << ": error: "
			      "Unable to evaluate constant expression ``" <<
			      *msb_[idx] << "''." << endl;
			des->errors += 1;
			return;
		  }

		  mnum[idx] = tmp->value().as_long();
		  delete texpr;

		  texpr = elab_and_eval(des, scope, lsb_[idx]);
		  tmp = dynamic_cast<NetEConst*>(texpr);
		  if (tmp == 0) {
			cerr << msb_[idx]->get_line() << ": error: "
			      "Unable to evaluate constant expression ``" <<
			      *lsb_[idx] << "''." << endl;
			des->errors += 1;
			return;
		  }

		  lnum[idx] = tmp->value().as_long();
		  delete texpr;

	    }

	      /* Check that the declarations were all scalar or all
		 vector. It is an error to mix them. Use the
		 count_scalars to know. */
	    if ((count_scalars > 0) && (count_scalars != msb_.count())) {
		  cerr << get_line() << ": error: Signal ``" << hname_
		       << "'' declared both as a vector and a scalar."
		       << endl;
		  des->errors += 1;
		  return;
	    }


	      /* Make sure all the values for msb and lsb match by
		 value. If not, report an error. */
	    for (unsigned idx = 1 ;  idx < msb_.count() ;  idx += 1) {
		  if ((mnum[idx] != mnum[0]) || (lnum[idx] != lnum[0])) {
			cerr << get_line() << ": error: Inconsistent width, "
			      "[" << mnum[idx] << ":" << lnum[idx] << "]"
			      " vs. [" << mnum[0] << ":" << lnum[0] << "]"
			      " for signal ``" << hname_ << "''" << endl;
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

      unsigned nattrib = 0;
      attrib_list_t*attrib_list = evaluate_attributes(attributes, nattrib,
						      des, scope);

	/* If the ident has idx expressions, then this is a
	   memory. It can only have the idx registers after the msb
	   and lsb expressions are filled. And, if it has one index,
	   it has both. */
      if (lidx_ || ridx_) {
	    assert(lidx_ && ridx_);

	    NetExpr*lexp = elab_and_eval(des, scope, lidx_);
	    NetExpr*rexp = elab_and_eval(des, scope, ridx_);

	    if ((lexp == 0) || (rexp == 0)) {
		  cerr << get_line() << ": internal error: There is "
		       << "a problem evaluating indices for ``"
		       << hname_.peek_tail_name() << "''." << endl;
		  des->errors += 1;
		  return;
	    }

	    NetEConst*lcon = dynamic_cast<NetEConst*> (lexp);
	    NetEConst*rcon = dynamic_cast<NetEConst*> (rexp);

	    if ((lcon == 0) || (rcon == 0)) {
		  cerr << get_line() << ": internal error: The indices "
		       << "are not constant for memory ``"
		       << hname_.peek_tail_name() << "''." << endl;
		  des->errors += 1;
		  return;
	    }

	    verinum lval = lcon->value();
	    verinum rval = rcon->value();

	    delete lexp;
	    delete rexp;

	    perm_string name = lex_strings.make(hname_.peek_tail_name());

	    long lnum = lval.as_long();
	    long rnum = rval.as_long();

	    new NetMemory(scope, name, wid, lnum, rnum);
	      // The constructor automatically adds the memory object
	      // to the scope. Do I need to set line number information?

      } else {

	    perm_string name = lex_strings.make(hname_.peek_tail_name());
	    NetNet*sig = new NetNet(scope, name, wtype, msb, lsb);
	    sig->set_line(*this);
	    sig->port_type(port_type_);
	    sig->set_signed(get_signed());
	    sig->set_isint(get_isint());

	    for (unsigned idx = 0 ;  idx < nattrib ;  idx += 1)
		  sig->attribute(attrib_list[idx].key, attrib_list[idx].val);
      }
}
