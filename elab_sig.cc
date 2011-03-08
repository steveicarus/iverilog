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

# include  <cstdlib>
# include  <iostream>

# include  "Module.h"
# include  "PExpr.h"
# include  "PGate.h"
# include  "PGenerate.h"
# include  "PTask.h"
# include  "PWire.h"
# include  "Statement.h"
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "util.h"
# include  "ivl_assert.h"

/*
 * Set the following to true when you need to process an expression
 * that is being done in a constant context. This allows the
 * elaboration to explicitly say we do not currently support constant
 * user functions when the function is not found.
 */
bool need_constant_expr = false;

static bool get_const_argument(NetExpr*exp, verinum&res)
{
      switch (exp->expr_type()) {
	  case IVL_VT_REAL: {
	    NetECReal*cv = dynamic_cast<NetECReal*>(exp);
	    if (cv == 0) return false;
	    verireal tmp = cv->value();
	    res = verinum(tmp.as_long());
	    break;
	  }

	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC: {
	    NetEConst*cv = dynamic_cast<NetEConst*>(exp);
	    if (cv == 0) return false;
	    res = cv->value();
	    break;
	  }

	  default:
	    assert(0);;
      }

      return true;
}

void Statement::elaborate_sig(Design*des, NetScope*scope) const
{
}

bool PScope::elaborate_sig_wires_(Design*des, NetScope*scope) const
{
      bool flag = true;

      for (map<perm_string,PWire*>::const_iterator wt = wires.begin()
		 ; wt != wires.end() ; wt ++ ) {

	    PWire*cur = (*wt).second;
	    NetNet*sig = cur->elaborate_sig(des, scope);


	      /* If the signal is an input and is also declared as a
		 reg, then report an error. */

	    if (sig && (sig->scope() == scope)
		&& (scope->type() == NetScope::MODULE)
		&& (sig->port_type() == NetNet::PINPUT)
		&& (sig->type() == NetNet::REG)) {

		  cerr << cur->get_fileline() << ": error: Port "
		       << cur->basename() << " of module "
		       << scope->module_name()
		       << " is declared as input and as a reg type." << endl;
		  des->errors += 1;
	    }

	    if (sig && (sig->scope() == scope)
		&& (scope->type() == NetScope::MODULE)
		&& (sig->port_type() == NetNet::PINOUT)
		&& (sig->type() == NetNet::REG)) {

		  cerr << cur->get_fileline() << ": error: Port "
		       << cur->basename() << " of module "
		       << scope->module_name()
		       << " is declared as inout and as a reg type." << endl;
		  des->errors += 1;
	    }

	    if (sig && (sig->scope() == scope)
		&& (scope->type() == NetScope::MODULE)
		&& (sig->port_type() == NetNet::PINOUT)
		&& (sig->data_type() == IVL_VT_REAL)) {

		  cerr << cur->get_fileline() << ": error: Port "
		       << cur->basename() << " of module "
		       << scope->module_name()
		       << " is declared as a real inout port." << endl;
		  des->errors += 1;
	    }

      }

      return flag;
}

static void elaborate_sig_funcs(Design*des, NetScope*scope,
				const map<perm_string,PFunction*>&funcs)
{
      typedef map<perm_string,PFunction*>::const_iterator mfunc_it_t;

      for (mfunc_it_t cur = funcs.begin()
		 ; cur != funcs.end() ;  cur ++) {

	    hname_t use_name ( (*cur).first );
	    NetScope*fscope = scope->child(use_name);
	    if (fscope == 0) {
		  cerr << (*cur).second->get_fileline() << ": internal error: "
		       << "Child scope for function " << (*cur).first
		       << " missing in " << scope_path(scope) << "." << endl;
		  des->errors += 1;
		  continue;
	    }

	    (*cur).second->elaborate_sig(des, fscope);
      }
}

static void elaborate_sig_tasks(Design*des, NetScope*scope,
				const map<perm_string,PTask*>&tasks)
{
      typedef map<perm_string,PTask*>::const_iterator mtask_it_t;

      for (mtask_it_t cur = tasks.begin()
		 ; cur != tasks.end() ;  cur ++) {
	    NetScope*tscope = scope->child( hname_t((*cur).first) );
	    assert(tscope);
	    (*cur).second->elaborate_sig(des, tscope);
      }
}


bool Module::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

	// Scan all the ports of the module, and make sure that each
	// is connected to wires that have port declarations.
      for (unsigned idx = 0 ;  idx < ports.size() ;  idx += 1) {
	    Module::port_t*pp = ports[idx];
	    if (pp == 0)
		  continue;

	      // The port has a name and an array of expressions. The
	      // expression are all identifiers that should reference
	      // wires within the scope.
	    map<perm_string,PWire*>::const_iterator wt;
	    for (unsigned cc = 0 ;  cc < pp->expr.size() ;  cc += 1) {
		  pform_name_t port_path (pp->expr[cc]->path());
		    // A concatenated wire of a port really should not
		    // have any hierarchy.
		  if (port_path.size() != 1) {
			cerr << get_fileline() << ": internal error: "
			     << "Port " << port_path << " has a funny name?"
			     << endl;
			des->errors += 1;
		  }

		  wt = wires.find(peek_tail_name(port_path));

		  if (wt == wires.end()) {
			cerr << get_fileline() << ": error: "
			     << "Port " << port_path << " ("
			     << (idx+1) << ") of module " << mod_name()
			     << " is not declared within module." << endl;
			des->errors += 1;
			continue;
		  }

		  if ((*wt).second->get_port_type() == NetNet::NOT_A_PORT) {
			cerr << get_fileline() << ": error: "
			     << "Port " << pp->expr[cc]->path() << " ("
			     << (idx+1) << ") of module " << mod_name()
			     << " has no direction declaration."
			     << endl;
			des->errors += 1;
		  }
	    }
      }

      flag = elaborate_sig_wires_(des, scope) && flag;

	// Run through all the generate schemes to elaborate the
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

	// After all the wires are elaborated, we are free to
	// elaborate the ports of the tasks defined within this
	// module. Run through them now.

      elaborate_sig_funcs(des, scope, funcs);
      elaborate_sig_tasks(des, scope, tasks);

	// initial and always blocks may contain begin-end and
	// fork-join blocks that can introduce scopes. Therefore, I
	// get to scan processes here.

      typedef list<PProcess*>::const_iterator proc_it_t;

      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ;  cur ++ ) {

	    (*cur) -> statement() -> elaborate_sig(des, scope);
      }

      return flag;
}

bool PExpr::elaborate_sig(Design*des, NetScope*scope) const
{
      return true;
}

bool PEConcat::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;
      for (unsigned idx = 0 ; idx < parms_.count() ; idx += 1)
	    flag = parms_[idx]->elaborate_sig(des, scope) && flag;

      return flag;
}

bool PEIdent::elaborate_sig(Design*des, NetScope*scope) const
{
      NetNet*       sig = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

	// If implicit net creation is turned off, then stop now.
      if (scope->default_nettype() == NetNet::NONE)
	    return true;
      if (error_implicit)
	    return true;

      symbol_search(this, des, scope, path_, sig, par, eve);

      if (eve != 0)
	    return false;

      if (par != 0)
	    return true;

      if (sig == 0)
	    sig = make_implicit_net_(des, scope);

      return sig != 0;
}

bool PEBinary::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

      flag = left_->elaborate_sig(des, scope)  && flag;
      flag = right_->elaborate_sig(des, scope) && flag;
      return flag;
}

bool PETernary::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;
      flag = tru_->elaborate_sig(des, scope)  && flag;
      flag = fal_->elaborate_sig(des, scope)  && flag;
      return flag;
}

bool PEUnary::elaborate_sig(Design*des, NetScope*scope) const
{
      return expr_->elaborate_sig(des, scope);
}

bool PGate::elaborate_sig(Design*des, NetScope*scope) const
{
      return true;
}

bool PGBuiltin::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

      for (unsigned idx = 0 ; idx < pin_count() ; idx += 1) {
	    const PExpr* pin_expr = pin(idx);
	    if (pin_expr == 0) {
		    // If there is no pin expression for this port,
		    // then skip it. Do not bother generating an error
		    // message here, that will be done during
		    // elaboration where these semantic details are tested.
		  continue;
	    }
	    ivl_assert(*this, pin_expr);
	    flag = pin_expr->elaborate_sig(des, scope) && flag;
      }

      return flag;
}

bool PGAssign::elaborate_sig(Design*des, NetScope*scope) const
{
      /* Normally, l-values to continuous assignments are NOT allowed
         to implicitly declare nets. However, so many tools do allow
         it that Icarus Verilog will allow it, at least if extensions
         are enabled. */
      if (gn_icarus_misc_flag)
	    return pin(0)->elaborate_sig(des, scope);

      return true;
}

bool PGModule::elaborate_sig_mod_(Design*des, NetScope*scope,
				  Module*rmod) const
{
      bool flag = true;

	// First, elaborate the signals that may be created implicitly
	// by ports to this module instantiation. Handle the case that
	// the ports are passed by name (pins_ != 0) or position.
      if (pins_)
	    for (unsigned idx =  0 ; idx < npins_ ; idx += 1) {
		  const PExpr*tmp = pins_[idx].parm;
		  if (tmp == 0)
			continue;
		  flag = tmp->elaborate_sig(des, scope) && flag;
	    }
      else
	    for (unsigned idx = 0 ; idx < pin_count() ; idx += 1) {
		  const PExpr*tmp = pin(idx);
		  if (tmp == 0)
			continue;
		  flag = tmp->elaborate_sig(des, scope) && flag;
	    }


      NetScope::scope_vec_t instance = scope->instance_arrays[get_name()];

      for (unsigned idx = 0 ;  idx < instance.size() ;  idx += 1) {
	      // I know a priori that the elaborate_scope created the scope
	      // already, so just look it up as a child of the current scope.
	    NetScope*my_scope = instance[idx];
	    assert(my_scope);

	    if (my_scope->parent() != scope) {
		  cerr << get_fileline() << ": internal error: "
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

bool PGModule::elaborate_sig_udp_(Design*des, NetScope*scope, PUdp*udp) const
{
      bool flag = true;

      if (pins_)
	    for (unsigned idx =  0 ; idx < npins_ ; idx += 1) {
		  const PExpr*tmp = pins_[idx].parm;
		  if (tmp == 0)
			continue;
		  flag = tmp->elaborate_sig(des, scope) && flag;
	    }
      else
	    for (unsigned idx = 0 ; idx < pin_count() ; idx += 1) {
		  const PExpr*tmp = pin(idx);
		  if (tmp == 0)
			continue;
		  flag = tmp->elaborate_sig(des, scope) && flag;
	    }

      return flag;
}

bool PGenerate::elaborate_sig(Design*des,  NetScope*container) const
{
      if (direct_nested_)
	    return elaborate_sig_direct_(des, container);

      bool flag = true;

	// Handle the special case that this is a CASE scheme. In this
	// case the PGenerate itself does not have the generated
	// item. Look instead for the case ITEM that has a scope
	// generated for it.
      if (scheme_type == PGenerate::GS_CASE) {
	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: generate case"
		       << " elaborate_sig in scope "
		       << scope_path(container) << "." << endl;

	    typedef list<PGenerate*>::const_iterator generate_it_t;
	    for (generate_it_t cur = generate_schemes.begin()
		       ; cur != generate_schemes.end() ; cur ++) {
		  PGenerate*item = *cur;
		  if (item->direct_nested_ || !item->scope_list_.empty()) {
			flag &= item->elaborate_sig(des, container);
		  }
	    }
	    return flag;
      }

      typedef list<NetScope*>::const_iterator scope_list_it_t;
      for (scope_list_it_t cur = scope_list_.begin()
		 ; cur != scope_list_.end() ; cur ++ ) {

	    NetScope*scope = *cur;

	    if (scope->parent() != container)
		  continue;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Elaborate nets in "
		       << "scope " << scope_path(*cur)
		       << " in generate " << id_number << endl;
	    flag = elaborate_sig_(des, *cur) & flag;
      }

      return flag;
}

bool PGenerate::elaborate_sig_direct_(Design*des, NetScope*container) const
{
      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: "
		 << "Direct nesting " << scope_name
		 << " (scheme_type=" << scheme_type << ")"
		 << " elaborate_sig in scope "
		 << scope_path(container) << "." << endl;

	// Elaborate_sig for a direct nested generated scheme knows
	// that there are only sub_schemes to be elaborated.  There
	// should be exactly 1 active generate scheme, search for it
	// using this loop.
      bool flag = true;
      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; cur ++) {
	    PGenerate*item = *cur;
	    if (item->direct_nested_ || !item->scope_list_.empty()) {
		    // Found the item, and it is direct nested.
		  flag &= item->elaborate_sig(des, container);
	    }
      }
      return flag;
}

bool PGenerate::elaborate_sig_(Design*des, NetScope*scope) const
{
	// Scan the declared PWires to elaborate the obvious signals
	// in the current scope.
      typedef map<perm_string,PWire*>::const_iterator wires_it_t;
      for (wires_it_t wt = wires.begin()
		 ; wt != wires.end() ;  wt ++ ) {

	    PWire*cur = (*wt).second;

	    if (debug_elaborate)
		  cerr << get_fileline() << ": debug: Elaborate PWire "
		       << cur->basename() << " in scope " << scope_path(scope) << endl;

	    cur->elaborate_sig(des, scope);
      }

      elaborate_sig_funcs(des, scope, funcs);
      elaborate_sig_tasks(des, scope, tasks);

      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; cur ++ ) {
	    (*cur) -> elaborate_sig(des, scope);
      }

      typedef list<PGate*>::const_iterator pgate_list_it_t;
      for (pgate_list_it_t cur = gates.begin()
		 ; cur != gates.end() ;  cur ++) {
	    (*cur) ->elaborate_sig(des, scope);
      }

      typedef list<PProcess*>::const_iterator proc_it_t;
      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ;  cur ++ ) {
	    (*cur) -> statement() -> elaborate_sig(des, scope);
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

      elaborate_sig_wires_(des, scope);

	/* Make sure the function has at least one input port. If it
	   fails this test, print an error message. Keep going so we
	   can find more errors. */
      if (ports_ == 0) {
	    cerr << get_fileline() << ": error: Function " << fname
		 << " has no ports." << endl;
	    cerr << get_fileline() << ":      : Functions must have"
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
		  probe_expr_width(des, scope, (*return_type_.range)[0]);
		  probe_expr_width(des, scope, (*return_type_.range)[1]);

		  need_constant_expr = true;
		  NetExpr*me = elab_and_eval(des, scope,
					     (*return_type_.range)[0], -1);
		  assert(me);
		  NetExpr*le = elab_and_eval(des, scope,
					     (*return_type_.range)[1], -1);
		  assert(le);
		  need_constant_expr = false;

		  long mnum = 0, lnum = 0;
		  if (NetEConst*tmp = dynamic_cast<NetEConst*>(me)) {
			mnum = tmp->value().as_long();
		  } else {
			cerr << me->get_fileline() << ": error: "
			      "Unable to evaluate constant expression "
			     << *me << "." << endl;
			des->errors += 1;
		  }

		  if (NetEConst*tmp = dynamic_cast<NetEConst*>(le)) {
			lnum = tmp->value().as_long();
		  } else {
			cerr << le->get_fileline() << ": error: "
			      "Unable to evaluate constant expression "
			     << *le << "." << endl;
			des->errors += 1;
		  }

		  ret_sig = new NetNet(scope, fname, NetNet::REG, mnum, lnum);
		  ret_sig->set_scalar(false);

	    } else {
		  ret_sig = new NetNet(scope, fname, NetNet::REG);
		  ret_sig->set_scalar(true);
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
	    ret_sig->set_scalar(false);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_LOGIC);
	    break;

	  case PTF_TIME:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, 64);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(false);
	    ret_sig->set_isint(false);
	    ret_sig->set_scalar(false);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_LOGIC);
	    break;

	  case PTF_REAL:
	  case PTF_REALTIME:
	    ret_sig = new NetNet(scope, fname, NetNet::REG, 1);
	    ret_sig->set_line(*this);
	    ret_sig->set_signed(true);
	    ret_sig->set_isint(false);
	    ret_sig->set_scalar(true);
	    ret_sig->port_type(NetNet::POUTPUT);
	    ret_sig->data_type(IVL_VT_REAL);
	    break;

	  default:
	    if (ports_) {
		  cerr << get_fileline() << ": internal error: I don't know "
		       << "how to deal with return type of function "
		       << scope->basename() << "." << endl;
	    } else {
		    /* If we do not have any ports or a return type this
		     * is probably a bad function definition. */
		  cerr << get_fileline() << ": error: Bad definition for "
		       << "function " << scope->basename() << "?" << endl;
		  return;
	    }
      }

      svector<NetNet*>ports (ports_? ports_->count() : 0);

      if (ports_)
	    for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {

		    /* Parse the port name into the task name and the reg
		       name. We know by design that the port name is given
		       as two components: <func>.<port>. */

		  perm_string pname = (*ports_)[idx]->basename();

		  NetNet*tmp = scope->find_signal(pname);
		  ports[idx] = 0;

		  if (tmp == 0) {
			cerr << get_fileline() << ": internal error: function "
			     << scope_path(scope) << " is missing port "
			     << pname << "." << endl;
			scope->dump(cerr);
			cerr << get_fileline() << ": Continuing..." << endl;
			des->errors += 1;
			continue;
		  }

		  if (tmp->port_type() == NetNet::NOT_A_PORT) {
			cerr << get_fileline() << ": internal error: function "
			     << scope_path(scope) << " port " << pname
			     << " is a port but is not a port?" << endl;
			des->errors += 1;
			scope->dump(cerr);
			continue;
		  }

		  ports[idx] = tmp;
		  if (tmp->port_type() != NetNet::PINPUT) {
			cerr << tmp->get_fileline() << ": error: function "
			     << scope_path(scope) << " port " << pname
			     << " is not an input port." << endl;
			cerr << tmp->get_fileline() << ":      : Function arguments "
			     << "must be input ports." << endl;
			des->errors += 1;
		  }
	    }


      NetFuncDef*def = 0;
      if (ret_sig)  def = new NetFuncDef(scope, ret_sig, ports);

      assert(def);
      if (debug_elaborate)
	    cerr << get_fileline() << ": debug: "
		 << "Attach function definition to scope "
		 << scope_path(scope) << "." << endl;

      scope->set_func_def(def);

	// Look for further signals in the sub-statement
      if (statement_)
	    statement_->elaborate_sig(des, scope);
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

      elaborate_sig_wires_(des, scope);

      svector<NetNet*>ports (ports_? ports_->count() : 0);
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {

	    perm_string port_name = (*ports_)[idx]->basename();

	      /* Find the signal for the port. We know by definition
		 that it is in the scope of the task, so look only in
		 the scope. */
	    NetNet*tmp = scope->find_signal(port_name);

	    if (tmp == 0) {
		  cerr << get_fileline() << ": internal error: "
		       << "Could not find port " << port_name
		       << " in scope " << scope_path(scope) << endl;
		  scope->dump(cerr);
		  des->errors += 1;
	    }

	    ports[idx] = tmp;
      }

      NetTaskDef*def = new NetTaskDef(scope, ports);
      scope->set_task_def(def);

	// Look for further signals in the sub-statement
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PBlock::elaborate_sig(Design*des, NetScope*scope) const
{
      NetScope*my_scope = scope;

      if (pscope_name() != 0) {
	    hname_t use_name (pscope_name());
	    my_scope = scope->child(use_name);
	    if (my_scope == 0) {
		  cerr << get_fileline() << ": internal error: "
		       << "Unable to find child scope " << pscope_name()
		       << " in this context?" << endl;
		  des->errors += 1;
		  my_scope = scope;
	    } else {
		  if (debug_elaborate)
			cerr << get_fileline() << ": debug: "
			     << "elaborate_sig descending into "
			     << scope_path(my_scope) << "." << endl;

		  elaborate_sig_wires_(des, my_scope);
	    }
      }

	// elaborate_sig in the statements included in the
	// block. There may be named blocks in there.
      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1)
	    list_[idx] -> elaborate_sig(des, my_scope);
}

void PCase::elaborate_sig(Design*des, NetScope*scope) const
{
      if (items_ == 0)
	    return;

      for (unsigned idx = 0 ; idx < items_->count() ; idx += 1) {
	    if ( (*items_)[idx]->stat )
		  (*items_)[idx]->stat ->elaborate_sig(des,scope);
      }
}

void PCondit::elaborate_sig(Design*des, NetScope*scope) const
{
      if (if_)
	    if_->elaborate_sig(des, scope);
      if (else_)
	    else_->elaborate_sig(des, scope);
}

void PDelayStatement::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PEventStatement::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PForever::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PForStatement::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PRepeat::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PWhile::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

/*
 * Elaborate a source wire. The "wire" is the declaration of wires,
 * registers, ports and memories. The parser has already merged the
 * multiple properties of a wire (i.e., "input wire"), so come the
 * elaboration this creates an object in the design that represents the
 * defined item.
 */
NetNet* PWire::elaborate_sig(Design*des, NetScope*scope) const
{
      NetNet::Type wtype = type_;
      bool is_implicit_scalar = false;
      if (wtype == NetNet::IMPLICIT) {
	    wtype = NetNet::WIRE;
	    is_implicit_scalar = true;
      }
      if (wtype == NetNet::IMPLICIT_REG) {
	    wtype = NetNet::REG;
	    is_implicit_scalar = true;
      }

      unsigned wid = 1;
      long lsb = 0, msb = 0;

      des->errors += error_cnt_;

	// A signal can not have the same name as a scope object.
      const NetScope *child = scope->child(hname_t(name_));
      if (child) {
	    cerr << get_fileline() << ": error: signal and ";
	    child->print_type(cerr);
	    cerr << " in '" << scope->fullname()
	         << "' have the same name '" << name_ << "'." << endl;
	    des->errors += 1;
      }
	// A signal can not have the same name as a genvar.
      const LineInfo *genvar = scope->find_genvar(name_);
      if (genvar) {
	    cerr << get_fileline() << ": error: signal and genvar in '"
	         << scope->fullname() << "' have the same name '" << name_
	         << "'." << endl;
	    des->errors += 1;
      }
	// A signal can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = scope->get_parameter(name_, ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: signal and parameter in '"
	         << scope->fullname() << "' have the same name '" << name_
	         << "'." << endl;
	    des->errors += 1;
      }
	// A signal can not have the same name as a named event.
      const NetEvent *event = scope->find_event(name_);
      if (event) {
	    cerr << get_fileline() << ": error: signal and named event in '"
	         << scope->fullname() << "' have the same name '" << name_
	         << "'." << endl;
	    des->errors += 1;
      }

      if (port_set_ || net_set_) {
	    long pmsb = 0, plsb = 0, nmsb = 0, nlsb = 0;
            bool bad_lsb = false, bad_msb = false;
	    /* If they exist get the port definition MSB and LSB */
	    if (port_set_ && port_msb_ != 0) {
		  probe_expr_width(des, scope, port_msb_);
		  /* We do not currently support constant user function. */
		  need_constant_expr = true;
		  NetExpr*texpr = elab_and_eval(des, scope, port_msb_, -1);
		  need_constant_expr = false;

		  if (! eval_as_long(pmsb, texpr)) {
			cerr << port_msb_->get_fileline() << ": error: "
			      "Range expressions must be constant." << endl;
			cerr << port_msb_->get_fileline() << "       : "
			      "This MSB expression violates the rule: "
                             << *port_msb_ << endl;
			des->errors += 1;
                        bad_msb = true;
		  }

		  delete texpr;

		  probe_expr_width(des, scope, port_lsb_);
		  /* We do not currently support constant user function. */
		  need_constant_expr = true;
		  texpr = elab_and_eval(des, scope, port_lsb_, -1);
		  need_constant_expr = false;

		  if (! eval_as_long(plsb, texpr)) {
			cerr << port_lsb_->get_fileline() << ": error: "
			      "Range expressions must be constant." << endl;
			cerr << port_lsb_->get_fileline() << "       : "
			      "This LSB expression violates the rule: "
                             << *port_lsb_ << endl;
			des->errors += 1;
                        bad_lsb = true;
		  }

		  delete texpr;
		  nmsb = pmsb;
		  nlsb = plsb;
		    /* An implicit port can have a range so note that here. */
		  is_implicit_scalar = false;
	    }
            if (!port_set_) assert(port_msb_ == 0 && port_lsb_ == 0);
            if (port_msb_ == 0) assert(port_lsb_ == 0);
            if (port_lsb_ == 0) assert(port_msb_ == 0);

	    /* If they exist get the net/etc. definition MSB and LSB */
	    if (net_set_ && net_msb_ != 0 && !bad_msb && !bad_lsb) {
		  probe_expr_width(des, scope, net_msb_);
		  /* We do not currently support constant user function. */
		  need_constant_expr = true;
		  NetExpr*texpr = elab_and_eval(des, scope, net_msb_, -1);
		  need_constant_expr = false;

		  if (! eval_as_long(nmsb, texpr)) {
			cerr << net_msb_->get_fileline() << ": error: "
			      "Range expressions must be constant." << endl;
			cerr << net_msb_->get_fileline() << "       : "
			      "This MSB expression violates the rule: "
                             << *net_msb_ << endl;
			des->errors += 1;
                        bad_msb = true;
		  }

		  delete texpr;

		  probe_expr_width(des, scope, net_lsb_);
		  /* We do not currently support constant user function. */
		  need_constant_expr = true;
		  texpr = elab_and_eval(des, scope, net_lsb_, -1);
		  need_constant_expr = false;

		  if (! eval_as_long(nlsb, texpr)) {
			cerr << net_lsb_->get_fileline() << ": error: "
			      "Range expressions must be constant." << endl;
			cerr << net_lsb_->get_fileline() << "       : "
			      "This LSB expression violates the rule: "
                             << *net_lsb_ << endl;
			des->errors += 1;
                        bad_lsb = true;
		  }

		  delete texpr;
	    }
            if (!net_set_) assert(net_msb_ == 0 && net_lsb_ == 0);
            if (net_msb_ == 0) assert(net_lsb_ == 0);
            if (net_lsb_ == 0) assert(net_msb_ == 0);

	    /* We have a port size error */
            if (port_set_ && net_set_ && (pmsb != nmsb || plsb != nlsb)) {

		  /* Scalar port with a vector net/etc. definition */
		  if (port_msb_ == 0) {
			if (!gn_io_range_error_flag) {
			      cerr << get_fileline()
			           << ": warning: Scalar port ``" << name_
			           << "'' has a vectored net declaration ["
			           << nmsb << ":" << nlsb << "]." << endl;
			} else {
			      cerr << get_fileline()
			           << ": error: Scalar port ``" << name_
			           << "'' has a vectored net declaration ["
			           << nmsb << ":" << nlsb << "]." << endl;
			      des->errors += 1;
			      return 0;
			}
		  }

		  /* Vectored port with a scalar net/etc. definition */
		  if (net_msb_ == 0) {
			cerr << port_msb_->get_fileline()
			     << ": error: Vectored port ``"
			     << name_ << "'' [" << pmsb << ":" << plsb
			     << "] has a scalar net declaration at "
			     << get_fileline() << "." << endl;
			des->errors += 1;
			return 0;
		  }

		  /* Both vectored, but they have different ranges. */
		  if (port_msb_ != 0 && net_msb_ != 0) {
			cerr << port_msb_->get_fileline()
			     << ": error: Vectored port ``"
			     << name_ << "'' [" << pmsb << ":" << plsb
			     << "] has a net declaration [" << nmsb << ":"
			     << nlsb << "] at " << net_msb_->get_fileline()
			     << " that does not match." << endl;
			des->errors += 1;
			return 0;
		  }
            }

              /* Attempt to recover from errors. */
            if (bad_lsb) nlsb = 0;
            if (bad_msb) nmsb = nlsb;

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

	    probe_expr_width(des, scope, lidx_);
	    probe_expr_width(des, scope, ridx_);

	    need_constant_expr = true;
	    NetExpr*lexp = elab_and_eval(des, scope, lidx_, -1);
	    NetExpr*rexp = elab_and_eval(des, scope, ridx_, -1);
	    need_constant_expr = false;

	    if ((lexp == 0) || (rexp == 0)) {
		  cerr << get_fileline() << ": internal error: There is "
		       << "a problem evaluating indices for ``"
		       << name_ << "''." << endl;
		  des->errors += 1;
		  return 0;
	    }

	    bool const_flag = true;
	    verinum lval, rval;
	    const_flag &= get_const_argument(lexp, lval);
	    const_flag &= get_const_argument(rexp, rval);
	    delete rexp;
	    delete lexp;

	    if (!const_flag) {
		  cerr << get_fileline() << ": error: The indices "
		       << "are not constant for array ``"
		       << name_ << "''." << endl;
		  des->errors += 1;
                    /* Attempt to recover from error, */
	          array_s0 = 0;
	          array_e0 = 0;
	    } else {
	          array_s0 = lval.as_long();
	          array_e0 = rval.as_long();
            }
	    array_dimensions = 1;
      }

      if (data_type_ == IVL_VT_REAL && (msb != 0 || lsb != 0)) {
	    cerr << get_fileline() << ": error: real ";
	    if (wtype == NetNet::REG) cerr << "variable";
	    else cerr << "net";
	    cerr << " '" << name_
	         << "' cannot be declared as a vector, found a range ["
	         << msb << ":" << lsb << "]." << endl;
	    des->errors += 1;
	    return 0;
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
		  cerr << get_fileline() << ": debug: "
		       << "Generate a SUPPLY pull for the ";
		  if (wtype == NetNet::SUPPLY0) cerr << "supply0";
		  else cerr << "supply1";
		  cerr << " net." << endl;
	    }
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Create signal " << wtype;
	    if (!get_scalar()) {
		  cerr << " ["<<msb<<":"<<lsb<<"]";
	    }
	    cerr << " " << name_;
	    if (array_dimensions > 0) {
		  cerr << " [" << array_s0 << ":" << array_e0 << "]" << endl;
	    }
	    cerr << " in scope " << scope_path(scope) << endl;
      }


      NetNet*sig = array_dimensions > 0
	    ? new NetNet(scope, name_, wtype, msb, lsb, array_s0, array_e0)
	    : new NetNet(scope, name_, wtype, msb, lsb);

      if (wtype == NetNet::WIRE) sig->devirtualize_pins();

      ivl_variable_type_t use_data_type = data_type_;
      if (use_data_type == IVL_VT_NO_TYPE) {
	    use_data_type = IVL_VT_LOGIC;
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: "
		       << "Signal " << name_
		       << " in scope " << scope_path(scope)
		       << " defaults to data type " << use_data_type << endl;
	    }
      }

      sig->data_type(use_data_type);
      sig->set_line(*this);
      sig->port_type(port_type_);
      sig->set_signed(get_signed());
      sig->set_isint(get_isint());
      if (is_implicit_scalar) sig->set_scalar(true);
      else sig->set_scalar(get_scalar());

      if (ivl_discipline_t dis = get_discipline()) {
	    sig->set_discipline(dis);
      }

      if (pull)
	    connect(sig->pin(0), pull->pin(0));

      for (unsigned idx = 0 ;  idx < nattrib ;  idx += 1)
	    sig->attribute(attrib_list[idx].key, attrib_list[idx].val);

      return sig;
}
