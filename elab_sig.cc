/*
 * Copyright (c) 2000-2024 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012 / Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <typeinfo>
# include  <cstdlib>
# include  <iostream>

# include  "Module.h"
# include  "PClass.h"
# include  "PExpr.h"
# include  "PGate.h"
# include  "PGenerate.h"
# include  "PPackage.h"
# include  "PTask.h"
# include  "PWire.h"
# include  "Statement.h"
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "netclass.h"
# include  "netenum.h"
# include  "netvector.h"
# include  "netdarray.h"
# include  "netparray.h"
# include  "netqueue.h"
# include  "netscalar.h"
# include  "util.h"
# include  "ivl_assert.h"

using namespace std;

#if 0
/* These functions are not currently used. */
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
	    ivl_assert(*exp, 0);;
      }

      return true;
}

static bool get_const_argument(NetExpr*exp, long&res)
{
      verinum tmp;
      bool rc = get_const_argument(exp, tmp);
      if (rc == false) return false;
      res = tmp.as_long();
      return true;
}
#endif

void Statement::elaborate_sig(Design*, NetScope*) const
{
}

static void sig_check_data_type(Design*des, NetScope*scope,
			        PWire *wire, NetNet *sig)
{
      ivl_type_t type = sig->net_type();

      if (!type)
	    return;

      if ((sig->type() == NetNet::WIRE) && (sig->data_type() != IVL_VT_LOGIC)) {
	    if (gn_cadence_types_flag) {
		  sig->type(NetNet::UNRESOLVED_WIRE);
	    } else {
		  cerr << wire->get_fileline() << ": error: Net `"
		       << wire->basename() << "` can not be of type `"
		       << sig->data_type() << "`." << endl;
		  des->errors++;
	    }
      }

      if (type->packed()) {
	    switch (type->base_type()) {
	    case IVL_VT_LOGIC: // 4-state packed is allowed by the standard
	    case IVL_VT_BOOL: // Icarus allows 2-state packed as an extension
		  return;
	    default:
		  break;
	    }
      }

      // Icarus allows real nets as an extension
      if (type->base_type() == IVL_VT_REAL)
	    return;

      if (wire->symbol_type() == PNamedItem::NET) {
	    cerr << wire->get_fileline() << ": error: Net `"
	         << wire->basename() << "` can not be of type `"
		 << sig->data_type() << "`." << endl;
	    des->errors++;
      } else if (scope->type() == NetScope::MODULE &&
	         sig->port_type() != NetNet::NOT_A_PORT) {
	    // Module ports only support wire types a the moment
	    cerr << wire->get_fileline() << ": sorry: Port `"
	         << wire->basename() << "` of module `"
	         << scope->module_name()
	         << "` with type `" << sig->data_type()
		 << "` is not supported."
	         << endl;
	    des->errors++;
      }
}

static void sig_check_port_type(Design*des, NetScope*scope,
			        PWire *wire, NetNet *sig)
{
      if (sig->port_type() == NetNet::PREF) {
	    cerr << wire->get_fileline() << ": sorry: "
		 << "Reference ports not supported yet." << endl;
	    des->errors += 1;
      }

      // Some extra checks for module ports
      if (scope->type() != NetScope::MODULE)
	    return;

	/* If the signal is an input and is also declared as a
	   reg, then report an error. In SystemVerilog a input
	   is allowed to be a register. It will get converted
	   to a unresolved wire when the port is connected. */

      if (sig->port_type() == NetNet::PINPUT &&
	  sig->type() == NetNet::REG && !gn_var_can_be_uwire()) {
	    cerr << wire->get_fileline() << ": error: Port `"
		 << wire->basename() << "` of module `"
		 << scope->module_name()
		 << "` is declared as input and as a reg type." << endl;
	    des->errors += 1;
      }

      if (sig->port_type() == NetNet::PINOUT &&
	  sig->type() == NetNet::REG) {
	    cerr << wire->get_fileline() << ": error: Port `"
		 << wire->basename() << "` of module `"
		 << scope->module_name()
		 << "` is declared as inout and as a reg type." << endl;
	    des->errors += 1;
      }

      if (sig->port_type() == NetNet::PINOUT &&
	  sig->data_type() == IVL_VT_REAL) {
	    cerr << wire->get_fileline() << ": error: Port `"
		 << wire->basename() << "` of module `"
		 << scope->module_name()
		 << "` is declared as a real inout port." << endl;
	    des->errors += 1;
      }
}

bool PScope::elaborate_sig_wires_(Design*des, NetScope*scope) const
{
      bool flag = true;

      for (map<perm_string,PWire*>::const_iterator wt = wires.begin()
		 ; wt != wires.end() ; ++ wt ) {

	    PWire*cur = (*wt).second;
	    NetNet*sig = cur->elaborate_sig(des, scope);

	    if (!sig || sig->scope() != scope)
		  continue;

	    sig_check_data_type(des, scope, cur, sig);
	    sig_check_port_type(des, scope, cur, sig);

      }

      return flag;
}

static void elaborate_sig_funcs(Design*des, NetScope*scope,
				const map<perm_string,PFunction*>&funcs)
{
      typedef map<perm_string,PFunction*>::const_iterator mfunc_it_t;

      for (mfunc_it_t cur = funcs.begin()
		 ; cur != funcs.end() ; ++ cur ) {

	    hname_t use_name ( (*cur).first );
	    NetScope*fscope = scope->child(use_name);
	    if (fscope == 0) {
		  cerr << (*cur).second->get_fileline() << ": internal error: "
		       << "Child scope for function " << (*cur).first
		       << " missing in " << scope_path(scope) << "." << endl;
		  des->errors += 1;
		  continue;
	    }

	    if (debug_elaborate) {
		  cerr << cur->second->get_fileline() << ": elaborate_sig_funcs: "
		       << "Elaborate function " << use_name
		       << " in " << scope_path(fscope) << endl;
	    }

	    cur->second->elaborate_sig(des, fscope);
      }
}

static void elaborate_sig_tasks(Design*des, NetScope*scope,
				const map<perm_string,PTask*>&tasks)
{
      typedef map<perm_string,PTask*>::const_iterator mtask_it_t;

      for (mtask_it_t cur = tasks.begin()
		 ; cur != tasks.end() ; ++ cur ) {
	    NetScope*tscope = scope->child( hname_t((*cur).first) );
	    ivl_assert(*(*cur).second, tscope);
	    (*cur).second->elaborate_sig(des, tscope);
      }
}

static void elaborate_sig_classes(Design*des, NetScope*scope,
				  const map<perm_string,PClass*>&classes)
{
      for (map<perm_string,PClass*>::const_iterator cur = classes.begin()
		 ; cur != classes.end() ; ++ cur) {
	    netclass_t*use_class = scope->find_class(des, cur->second->pscope_name());
	    use_class->elaborate_sig(des, cur->second);
      }
}

bool PPackage::elaborate_sig(Design*des, NetScope*scope) const
{
      bool flag = true;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PPackage::elaborate_sig: "
		 << "Start package scope=" << scope_path(scope) << endl;
      }

      flag = elaborate_sig_wires_(des, scope) && flag;

	// After all the wires are elaborated, we are free to
	// elaborate the ports of the tasks defined within this
	// module. Run through them now.

      elaborate_sig_funcs(des, scope, funcs);
      elaborate_sig_tasks(des, scope, tasks);
      elaborate_sig_classes(des, scope, classes);

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PPackage::elaborate_sig: "
		 << "Done package scope=" << scope_path(scope)
		 << ", flag=" << flag << endl;
      }

      return flag;
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
		  pform_name_t port_path (pp->expr[cc]->path().name);
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
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur) -> elaborate_sig(des, scope);
      }

	// Get all the gates of the module and elaborate them by
	// connecting them to the signals. The gate may be simple or
	// complex. What we are looking for is gates that are modules
	// that can create scopes and signals.

      const list<PGate*>&gl = get_gates();

      for (list<PGate*>::const_iterator gt = gl.begin()
		 ; gt != gl.end() ; ++ gt ) {

	    flag &= (*gt)->elaborate_sig(des, scope);
      }

	// After all the wires are elaborated, we are free to
	// elaborate the ports of the tasks defined within this
	// module. Run through them now.

      elaborate_sig_funcs(des, scope, funcs);
      elaborate_sig_tasks(des, scope, tasks);
      elaborate_sig_classes(des, scope, classes);

	// initial and always blocks may contain begin-end and
	// fork-join blocks that can introduce scopes. Therefore, I
	// get to scan processes here.

      typedef list<PProcess*>::const_iterator proc_it_t;

      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ; ++ cur ) {

	    (*cur) -> statement() -> elaborate_sig(des, scope);
      }

      return flag;
}

void netclass_t::elaborate_sig(Design*des, PClass*pclass)
{
	// Collect the properties, elaborate them, and add them to the
	// elaborated class definition.
      for (map<perm_string,struct class_type_t::prop_info_t>::iterator cur = pclass->type->properties.begin()
		 ; cur != pclass->type->properties.end() ; ++ cur) {

	    ivl_type_t use_type = cur->second.type->elaborate_type(des, class_scope_);
	    if (debug_scopes) {
		  cerr << pclass->get_fileline() << ": elaborate_scope_class: "
		       << "  Property " << cur->first
		       << " type=" << *use_type << endl;
	    }

	    if (dynamic_cast<const netqueue_t *> (use_type)) {
		  cerr << cur->second.get_fileline() << ": sorry: "
		       << "Queues inside classes are not yet supported." << endl;
		  des->errors++;
	    }
	    set_property(cur->first, cur->second.qual, use_type);

	    if (! cur->second.qual.test_static())
		  continue;

	    if (debug_elaborate) {
		  cerr << pclass->get_fileline() << ": netclass_t::elaborate_sig: "
		       << "Elaborate static property " << cur->first
		       << " as signal in scope " << scope_path(class_scope_)
		       << "." << endl;
	    }

	    /* NetNet*sig = */ new NetNet(class_scope_, cur->first, NetNet::REG,
					  use_type);
      }

      for (map<perm_string,PFunction*>::iterator cur = pclass->funcs.begin()
		 ; cur != pclass->funcs.end() ; ++ cur) {
	    if (debug_elaborate) {
		  cerr << cur->second->get_fileline() << ": netclass_t::elaborate_sig: "
		       << "Elaborate signals in function method " << cur->first << endl;
	    }

	    NetScope*scope = class_scope_->child( hname_t(cur->first) );
	    ivl_assert(*cur->second, scope);
	    cur->second->elaborate_sig(des, scope);
      }

      for (map<perm_string,PTask*>::iterator cur = pclass->tasks.begin()
		 ; cur != pclass->tasks.end() ; ++ cur) {
	    if (debug_elaborate) {
		  cerr << cur->second->get_fileline() << ": netclass_t::elaborate_sig: "
		       << "Elaborate signals in task method " << cur->first << endl;
	    }

	    NetScope*scope = class_scope_->child( hname_t(cur->first) );
	    ivl_assert(*cur->second, scope);
	    cur->second->elaborate_sig(des, scope);
      }

}

bool PGate::elaborate_sig(Design*, NetScope*) const
{
      return true;
}

bool PGBuiltin::elaborate_sig(Design*, NetScope*) const
{
      return true;
}

bool PGAssign::elaborate_sig(Design*, NetScope*) const
{
      return true;
}

bool PGModule::elaborate_sig_mod_(Design*des, NetScope*scope,
				  Module*rmod) const
{
      bool flag = true;

      NetScope::scope_vec_t instance = scope->instance_arrays[get_name()];

      for (unsigned idx = 0 ;  idx < instance.size() ;  idx += 1) {
	      // I know a priori that the elaborate_scope created the scope
	      // already, so just look it up as a child of the current scope.
	    NetScope*my_scope = instance[idx];
	    ivl_assert(*this, my_scope);

	    if (my_scope->parent() != scope) {
		  cerr << get_fileline() << ": internal error: "
		       << "Instance " << scope_path(my_scope)
		       << " is in parent " << scope_path(my_scope->parent())
		       << " instead of " << scope_path(scope)
		       << endl;
	    }
	    ivl_assert(*this, my_scope->parent() == scope);

	    if (! rmod->elaborate_sig(des, my_scope))
		  flag = false;

      }

      return flag;
}

	// Not currently used.
#if 0
bool PGModule::elaborate_sig_udp_(Design*des, NetScope*scope, PUdp*udp) const
{
      return true;
}
#endif

bool PGenerate::elaborate_sig(Design*des,  NetScope*container) const
{
      if (directly_nested)
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
		       ; cur != generate_schemes.end() ; ++ cur ) {
		  PGenerate*item = *cur;
		  if (item->directly_nested || !item->scope_list_.empty()) {
			flag &= item->elaborate_sig(des, container);
		  }
	    }
	    return flag;
      }

      typedef list<NetScope*>::const_iterator scope_list_it_t;
      for (scope_list_it_t cur = scope_list_.begin()
		 ; cur != scope_list_.end() ; ++ cur ) {

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
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    PGenerate*item = *cur;
	    if (item->scheme_type == PGenerate::GS_CASE) {
		  for (generate_it_t icur = item->generate_schemes.begin()
			     ; icur != item->generate_schemes.end() ; ++ icur ) {
			PGenerate*case_item = *icur;
			if (case_item->directly_nested || !case_item->scope_list_.empty()) {
			      flag &= case_item->elaborate_sig(des, container);
			}
		  }
	    } else {
		  if (item->directly_nested || !item->scope_list_.empty()) {
			  // Found the item, and it is direct nested.
			flag &= item->elaborate_sig(des, container);
		  }
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
		 ; wt != wires.end() ; ++ wt ) {

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
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur) -> elaborate_sig(des, scope);
      }

      typedef list<PGate*>::const_iterator pgate_list_it_t;
      for (pgate_list_it_t cur = gates.begin()
		 ; cur != gates.end() ; ++ cur ) {
	    (*cur) ->elaborate_sig(des, scope);
      }

      typedef list<PProcess*>::const_iterator proc_it_t;
      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ; ++ cur ) {
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
      if (scope->elab_stage() > 1)
            return;

      scope->set_elab_stage(2);

      perm_string fname = scope->basename();
      ivl_assert(*this, scope->type() == NetScope::FUNC);

      elaborate_sig_wires_(des, scope);

      NetNet*ret_sig;
      if (gn_system_verilog() && (fname=="new" || fname=="new@")) {
	      // Special case: this is a constructor, so the return
	      // signal is also the first argument. For example, the
	      // source code for the definition may be:
	      //   function new(...);
	      //   endfunction
	      // In this case, the "@" port (THIS_TOKEN) is the synthetic
	      // "this" argument and we also use it as a return value at
	      // the same time.
	    ret_sig = scope->find_signal(perm_string::literal(THIS_TOKEN));
	    ivl_assert(*this, ret_sig);

	    if (debug_elaborate)
		  cerr << get_fileline() << ": PFunction::elaborate_sig: "
		       << "Scope " << scope_path(scope)
		       << " is a CONSTRUCTOR, so use \"this\" argument"
		       << " as return value." << endl;

      } else {
	    ivl_type_t ret_type;

	    if (return_type_) {
		  if (dynamic_cast<const struct void_type_t*> (return_type_)) {
			ret_type = 0;
		  } else {
			ret_type = return_type_->elaborate_type(des, scope->parent());
			ivl_assert(*this, ret_type);
		  }
	    } else {
		  netvector_t*tmp = new netvector_t(IVL_VT_LOGIC);
		  ret_type = tmp;
	    }

	    if (ret_type) {
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PFunction::elaborate_sig: "
			     << "return type: " << *ret_type << endl;
			if (return_type_)
			      return_type_->pform_dump(cerr, 8);
		  }
		  ret_sig = new NetNet(scope, fname, NetNet::REG, ret_type);

		  ret_sig->set_line(*this);
		  ret_sig->port_type(NetNet::POUTPUT);
	    } else {
		  ret_sig = 0;
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PFunction::elaborate_sig: "
			     << "Detected that function is void." << endl;
		  }
	    }
      }

      vector<NetNet*>ports;
      vector<NetExpr*>pdef;
      vector<perm_string> port_names;
      elaborate_sig_ports_(des, scope, ports, pdef, port_names);

      NetFuncDef*def = new NetFuncDef(scope, ret_sig, ports, pdef);

      if (debug_elaborate)
	    cerr << get_fileline() << ": PFunction::elaborate_sig: "
		 << "Attach function definition " << scope_path(scope)
		 << " with ret_sig width=" << (ret_sig? ret_sig->vector_width() : 0)
		 << "." << endl;

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
      ivl_assert(*this, scope->type() == NetScope::TASK);

      elaborate_sig_wires_(des, scope);

      vector<NetNet*>ports;
      vector<NetExpr*>pdefs;
      vector<perm_string> port_names;
      elaborate_sig_ports_(des, scope, ports, pdefs, port_names);
      NetTaskDef*def = new NetTaskDef(scope, ports, pdefs);
      scope->set_task_def(def);

	// Look for further signals in the sub-statement
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PTaskFunc::elaborate_sig_ports_(Design*des, NetScope*scope,
				     vector<NetNet*> &ports,
				     vector<NetExpr*> &pdefs,
				     vector<perm_string> &port_names) const
{
      if (ports_ == 0) {
	    ports.clear();
	    pdefs.clear();
	    port_names.clear();

	      /* Make sure the function has at least one input
		 port. If it fails this test, print an error
		 message. Keep going so we can find more errors. */
	    if (scope->type()==NetScope::FUNC && !gn_system_verilog()) {
		  cerr << get_fileline() << ": error: "
		       << "Function " << scope->basename()
		       << " has no ports." << endl;
		  cerr << get_fileline() << ":      : "
		       << "Functions must have at least one input port." << endl;
		  des->errors += 1;
	    }

	    return;
      }

      ports.resize(ports_->size());
      pdefs.resize(ports_->size());
      port_names.resize(ports_->size());

      for (size_t idx = 0 ; idx < ports_->size() ; idx += 1) {

	    perm_string port_name = ports_->at(idx).port->basename();

	    ports[idx] = 0;
	    pdefs[idx] = 0;
	    NetNet*tmp = scope->find_signal(port_name);
	    NetExpr*tmp_def = 0;
	    if (tmp == 0) {
		  cerr << get_fileline() << ": internal error: "
		       << "task/function " << scope_path(scope)
		       << " is missing port " << port_name << "." << endl;
		  scope->dump(cerr);
		  cerr << get_fileline() << ": Continuing..." << endl;
		  des->errors += 1;
		  continue;
	    }

	      // If the port has a default expression, elaborate
	      // that expression here.
	    if (ports_->at(idx).defe != 0) {
		  if (tmp->port_type() == NetNet::PINPUT) {
			  // Elaborate a class port default in the context of
			  // the class type.
			if (tmp->data_type() == IVL_VT_CLASS) {
			      tmp_def = elab_and_eval(des, scope,
			                              ports_->at(idx).defe,
			                              tmp->net_type(),
			                              scope->need_const_func());
			} else {
			      tmp_def = elab_and_eval(des, scope,
			                              ports_->at(idx).defe,
			                              -1,
			                              scope->need_const_func());
			}
			if (tmp_def == 0) {
			      cerr << get_fileline()
				   << ": error: Unable to evaluate "
				   << *ports_->at(idx).defe
				   << " as a port default expression." << endl;
			      des->errors += 1;
			}
		  } else {
			cerr << get_fileline() << ": sorry: Default arguments "
			        "for subroutine output or inout ports are not "
			        "yet supported." << endl;
			des->errors += 1;
		  }
	    }

	    if (tmp->port_type() == NetNet::NOT_A_PORT) {
		  cerr << get_fileline() << ": internal error: "
		       << "task/function " << scope_path(scope)
		       << " port " << port_name
		       << " is a port but is not a port?" << endl;
		  des->errors += 1;
		  scope->dump(cerr);
		  continue;
	    }

	    ports[idx] = tmp;
	    port_names[idx] = port_name;
	    pdefs[idx] = tmp_def;
	    if (scope->type()==NetScope::FUNC && tmp->port_type()!=NetNet::PINPUT) {
		  cerr << tmp->get_fileline() << ": error: "
		       << "Function " << scope_path(scope)
		       << " port " << port_name
		       << " is not an input port." << endl;
		  cerr << tmp->get_fileline() << ":      : "
		       << "Function arguments must be input ports." << endl;
		  des->errors += 1;
	    }
	    if (tmp->unpacked_dimensions() != 0) {
		  cerr << get_fileline() << ": sorry: Subroutine ports with "
			  "unpacked dimensions are not yet supported." << endl;
		 des->errors += 1;
	    }
      }
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
      for (unsigned idx = 0 ;  idx < list_.size() ;  idx += 1)
	    list_[idx] -> elaborate_sig(des, my_scope);
}

void PCase::elaborate_sig(Design*des, NetScope*scope) const
{
      if (items_ == 0)
	    return;

      for (unsigned idx = 0 ; idx < items_->size() ; idx += 1) {
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

void PDoWhile::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PEventStatement::elaborate_sig(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_->elaborate_sig(des, scope);
}

void PForeach::elaborate_sig(Design*des, NetScope*scope) const
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

bool test_ranges_eeq(const netranges_t&lef, const netranges_t&rig)
{
      if (lef.size() != rig.size())
	    return false;

      netranges_t::const_iterator lcur = lef.begin();
      netranges_t::const_iterator rcur = rig.begin();
      while (lcur != lef.end()) {
	    if (lcur->get_msb() != rcur->get_msb())
		  return false;
	    if (lcur->get_lsb() != rcur->get_lsb())
		  return false;

	    ++ lcur;
	    ++ rcur;
      }

      return true;
}

ivl_type_t PWire::elaborate_type(Design*des, NetScope*scope,
			         const netranges_t &packed_dimensions) const
{
      vector_type_t *vec_type = dynamic_cast<vector_type_t*>(set_data_type_.get());
      if (set_data_type_ && !vec_type) {
	    ivl_assert(*this, packed_dimensions.empty());
	    return set_data_type_->elaborate_type(des, scope);
      }

      // Fallback method. Create vector type.

      ivl_variable_type_t use_data_type;
      if (vec_type) {
	    use_data_type = vec_type->base_type;
      } else {
	    use_data_type = IVL_VT_LOGIC;
      }

      if (use_data_type == IVL_VT_NO_TYPE) {
	    use_data_type = IVL_VT_LOGIC;
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PWire::elaborate_sig: "
		       << "Signal " << name_
		       << " in scope " << scope_path(scope)
		       << " defaults to data type " << use_data_type << endl;
	    }
      }

      ivl_assert(*this, use_data_type == IVL_VT_LOGIC ||
			use_data_type == IVL_VT_BOOL);

      netvector_t*vec = new netvector_t(packed_dimensions, use_data_type);
      vec->set_signed(get_signed());

      return vec;
}

/*
 * Elaborate a source wire. The "wire" is the declaration of wires,
 * registers, ports and memories. The parser has already merged the
 * multiple properties of a wire (i.e., "input wire"), so come the
 * elaboration this creates an object in the design that represents the
 * defined item.
 */
NetNet* PWire::elaborate_sig(Design*des, NetScope*scope)
{
	// This sets the vector or array dimension size that will
	// cause a warning. For now, these warnings are permanently
	// enabled.
      const long warn_dimension_size = 1 << 30;

	// Check if we elaborated this signal earlier because it was
	// used in another declaration.
      if (NetNet*sig = scope->find_signal(name_))
            return sig;

      if (is_elaborating_) {
	    cerr << get_fileline() << ": error: Circular dependency "
		    "detected in declaration of '" << name_ << "'."
		 << endl;
	    des->errors += 1;
	    return 0;
      }
      is_elaborating_ = true;

      NetNet::Type wtype = type_;
      if (wtype == NetNet::IMPLICIT)
	    wtype = NetNet::WIRE;

	// Certain contexts, such as arguments to functions, presume
	// "reg" instead of "wire". The parser reports these as
	// IMPLICIT_REG.
      if (wtype == NetNet::IMPLICIT_REG)
	    wtype = NetNet::REG;

      if (debug_elaborate) {
	    cerr << get_fileline() << ": PWire::elaborate_sig: "
		 << "Signal " << basename()
		 << ", wtype=" << wtype;
	    if (set_data_type_)
		  cerr << ", set_data_type_=" << *set_data_type_;
	    cerr << ", unpacked_.size()=" << unpacked_.size()
		 << endl;
      }

      unsigned wid = 1;
      netranges_t packed_dimensions;

      des->errors += error_cnt_;

      if (port_set_ || net_set_) {

	    if (warn_implicit_dimensions
		&& port_set_ && net_set_
		&& net_.empty() && !port_.empty()) {
		  cerr << get_fileline() << ": warning: "
		       << "var/net declaration of " << basename()
		       << " inherits dimensions from port declaration." << endl;
	    }

	    if (warn_implicit_dimensions
		&& port_set_ && net_set_
		&& port_.empty() && !net_.empty()) {
		  cerr << get_fileline() << ": warning: "
		       << "Port declaration of " << basename()
		       << " inherits dimensions from var/net." << endl;
	    }

	    bool dimensions_ok = true;
	    netranges_t plist, nlist;
	    /* If they exist get the port definition MSB and LSB */
	    if (port_set_ && !port_.empty()) {
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PWire::elaborate_sig: "
			     << "Evaluate ranges for port " << basename() << endl;
		  }
		  dimensions_ok &= evaluate_ranges(des, scope, this, plist, port_);
	    }
            ivl_assert(*this, port_set_ || port_.empty());

	    /* If they exist get the net/etc. definition MSB and LSB */
	    if (net_set_ && !net_.empty() && dimensions_ok) {
		  nlist.clear();
		  if (debug_elaborate) {
			cerr << get_fileline() << ": PWire::elaborate_sig: "
			     << "Evaluate ranges for net " << basename() << endl;
		  }
		  dimensions_ok &= evaluate_ranges(des, scope, this, nlist, net_);
	    }
            ivl_assert(*this, net_set_ || net_.empty());

	    if (debug_elaborate) {
		  cerr << get_fileline() << ": PWire::elaborate_sig: "
		       << "Calculated ranges for " << basename()
		       << ". Now check for consistency." << endl;
	    }

	    /* We have a port size error. Skip this if the dimensions could not
	     * be evaluated since it will likely print nonsensical errors. */
            if (port_set_ && net_set_ && !test_ranges_eeq(plist, nlist) &&
	        dimensions_ok) {
		  /* Scalar port with a vector net/etc. definition */
		  if (port_.empty()) {
			if (gn_io_range_error_flag) {
			      cerr << get_fileline()
			           << ": error: Scalar port ``" << name_
			           << "'' has a vectored net declaration "
				   << nlist << "." << endl;
			      des->errors += 1;
			} else if (warn_anachronisms) {
			      cerr << get_fileline()
			           << ": warning: Scalar port ``" << name_
			           << "'' has a vectored net declaration "
				   << nlist << "." << endl;
			}
		  }

		  /* Vectored port with a scalar net/etc. definition */
		  if (net_.empty()) {
			cerr << port_.front().first->get_fileline()
			     << ": error: Vectored port ``"
			     << name_ << "'' " << plist
			     << " has a scalar net declaration at "
			     << get_fileline() << "." << endl;
			des->errors += 1;
		  }

		  /* Both vectored, but they have different ranges. */
		  if (!port_.empty() && !net_.empty()) {
			cerr << port_.front().first->get_fileline()
			     << ": error: Vectored port ``"
			     << name_ << "'' " << plist
			     << " has a net declaration " << nlist
			     << " at " << net_.front().first->get_fileline()
			     << " that does not match." << endl;
			des->errors += 1;
		  }
            }

	    packed_dimensions = net_set_ ? nlist : plist;
	    wid = netrange_width(packed_dimensions);
	    if (wid > warn_dimension_size) {
		  cerr << get_fileline() << ": warning: Vector size "
		          "is greater than " << warn_dimension_size
		       << "." << endl;
	    }
      }

      unsigned nattrib = 0;
      attrib_list_t*attrib_list = evaluate_attributes(attributes, nattrib,
						      des, scope);

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
	    if (debug_elaborate) {
		  cerr << get_fileline() << ": debug: "
		       << "Generate a SUPPLY pull for the ";
		  if (wtype == NetNet::SUPPLY0) cerr << "supply0";
		  else cerr << "supply1";
		  cerr << " net." << endl;
	    }

	    NetLogic::TYPE pull_type = (wtype==NetNet::SUPPLY1)
		  ? NetLogic::PULLUP
		  : NetLogic::PULLDOWN;
	    pull = new NetLogic(scope, scope->local_symbol(),
				1, pull_type, wid);
	    pull->set_line(*this);
	    pull->pin(0).drive0(IVL_DR_SUPPLY);
	    pull->pin(0).drive1(IVL_DR_SUPPLY);
	    des->add_node(pull);
	    wtype = NetNet::WIRE;
      }

      ivl_type_t type = elaborate_type(des, scope, packed_dimensions);
	// Create the type for the unpacked dimensions. If the
	// unpacked_dimensions are empty this will just return the base type.
      type = elaborate_array_type(des, scope, *this, type, unpacked_);

      netranges_t unpacked_dimensions;
	// If this is an unpacked array extract the base type and unpacked
	// dimensions as these are separate properties of the NetNet.
      while (const netuarray_t *atype = dynamic_cast<const netuarray_t*>(type)) {
	    unpacked_dimensions.insert(unpacked_dimensions.begin(),
				       atype->static_dimensions().begin(),
				       atype->static_dimensions().end());
	    type = atype->element_type();
      }

      if (debug_elaborate) {
	    cerr << get_fileline() << ": debug: Create signal " << wtype;
	    if (set_data_type_)
		  cout << " " << *set_data_type_;
	    cout << " " << name_ << unpacked_dimensions << " in scope "
		 << scope_path(scope) << endl;
      }

      NetNet*sig = new NetNet(scope, name_, wtype, unpacked_dimensions, type);

      if (wtype == NetNet::WIRE) sig->devirtualize_pins();
      sig->set_line(*this);
      sig->port_type(port_type_);
      sig->lexical_pos(lexical_pos_);

      if (ivl_discipline_t dis = get_discipline()) {
	    sig->set_discipline(dis);
      }

      if (pull)
	    connect(sig->pin(0), pull->pin(0));

      for (unsigned idx = 0 ;  idx < nattrib ;  idx += 1)
	    sig->attribute(attrib_list[idx].key, attrib_list[idx].val);

      sig->set_const(is_const_);

      scope->rem_signal_placeholder(this);
      is_elaborating_ = false;

      return sig;
}
