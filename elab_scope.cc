/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_scope.cc,v 1.1 2000/03/08 04:36:53 steve Exp $"
#endif

/*
 * Elaboration happens in two passes, generally. The first scans the
 * pform to generate the NetScope tree and attach it to the Design
 * object. The methods in this source file implement the elaboration
 * of the scopes.
 */

# include  "Module.h"
# include  "PExpr.h"
# include  "PGate.h"
# include  "PTask.h"
# include  "netlist.h"

bool Module::elaborate_scope(Design*des, NetScope*scope) const
{
	// Generate all the parameters that this instance of this
	// module introduces to the design. This loop elaborates the
	// parameters, but doesn't evaluate references to
	// parameters. This scan practically locates all the
	// parameters and puts them in the parameter table in the
	// design.

	// No expressions are evaluated, yet. For now, leave them in
	// the pform and just place a NetEParam placeholder in the
	// place of the elaborated expression.

      typedef map<string,PExpr*>::const_iterator mparm_it_t;


	// This loop scans the parameters in the module, and creates
	// stub parameter entries in the scope for the parameter name.

      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {

	    scope->set_parameter((*cur).first, new NetEParam);
      }

	// Now scan the parameters again, this time elaborating them
	// for use as parameter values. This is after the previous
	// scan so that local parameter names can be used in the
	// r-value expressions.

      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {

	    PExpr*ex = (*cur).second;
	    assert(ex);

	    NetExpr*val = ex->elaborate_pexpr(des, scope);
	    val = scope->set_parameter((*cur).first, val);
	    assert(val);
	    delete val;
      }

	// Run through the defparams for this module, elaborate the
	// expressions in this context and save the result is a table
	// for later final override.

	// It is OK to elaborate the expressions of the defparam here
	// because Verilog requires that the expressions only use
	// local parameter names. It is *not* OK to do the override
	// here becuase the parameter receiving the assignment may be
	// in a scope not discovered by this pass.

      for (mparm_it_t cur = defparms.begin()
		 ; cur != defparms.end() ;  cur ++ ) {

	    PExpr*ex = (*cur).second;
	    assert(ex);

	    NetExpr*val = ex->elaborate_pexpr(des, scope);
	    if (val == 0) continue;
	    scope->defparams[(*cur).first] = val;
      }


	// Tasks introduce new scopes, so scan the tasks in this
	// module. Create a scope for the task and pass that to the
	// elaborate_scope method of the PTask for detailed
	// processing.

      typedef map<string,PTask*>::const_iterator tasks_it_t;

      for (tasks_it_t cur = tasks_.begin()
		 ; cur != tasks_.end() ;  cur ++ ) {

	    NetScope*task_scope = new NetScope(scope, (*cur).first,
					       NetScope::TASK);
	    (*cur).second->elaborate_scope(des, task_scope);
      }


	// Functions are very similar to tasks, at least from the
	// perspective of scopes. So handle them exactly the same
	// way.

      typedef map<string,PFunction*>::const_iterator funcs_it_t;

      for (funcs_it_t cur = funcs_.begin()
		 ; cur != funcs_.end() ;  cur ++ ) {

	    NetScope*func_scope = new NetScope(scope, (*cur).first,
					       NetScope::FUNC);
	    (*cur).second->elaborate_scope(des, func_scope);
      }


	// Gates include modules, which might introduce new scopes, so
	// scan all of them to create those scopes.

      typedef list<PGate*>::const_iterator gates_it_t;

      for (gates_it_t cur = gates_.begin()
		 ; cur != gates_.end() ;  cur ++ ) {

	    (*cur)->elaborate_scope(des, scope);
      }


      return des->errors == 0;
}

void PGModule::elaborate_scope_mod_(Design*des, Module*mod, NetScope*sc) const
{
	// Missing module instance names have already been rejected.
      assert(get_name() != "");

      string path = sc->name();

	// Check for duplicate scopes. Simply look up the scope I'm
	// about to create, and if I find it then somebody beat me to
	// it.

      if (NetScope*tmp = des->find_scope(path + "." + get_name())) {
	    cerr << get_line() << ": error: Instance/Scope name " <<
		  get_name() << " already used in this context." <<
		  endl;
	    des->errors += 1;
	    return;
      }

	// Create the new scope as a MODULE with my name.
      NetScope*my_scope = des->make_scope(path, NetScope::MODULE, get_name());

	// This call actually arranges for the description of the
	// module type to process this instance and handle parameters
	// and sub-scopes that might occur. Parameters are also
	// created in that scope, as they exist. (I'll override them
	// later.)
      mod->elaborate_scope(des, my_scope);

	// Look for module parameter replacements. This map receives
	// those replacements.

      typedef map<string,PExpr*>::const_iterator mparm_it_t;
      map<string,PExpr*> replace;


	// Positional parameter overrides are matched to parameter
	// names by using the param_names list of parameter
	// names. This is an ordered list of names so the first name
	// is parameter 0, the second parameter 1, and so on.

      if (overrides_) {
	    assert(parms_ == 0);
	    list<string>::const_iterator cur = mod->param_names.begin();
	    for (unsigned idx = 0
		       ;  idx < overrides_->count()
		       ; idx += 1, cur++) {
		  replace[*cur] = (*overrides_)[idx];
	    }

      }

	// Named parameter overrides carry a name with each override
	// so the mapping into the replace list is much easier.
      if (parms_) {
	    assert(overrides_ == 0);
	    for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1)
		  replace[parms_[idx].name] = parms_[idx].parm;

      }


	// And here we scan the replacements we collected. Elaborate
	// the expression in my context, then replace the sub-scope
	// parameter value with the new expression.

      for (mparm_it_t cur = replace.begin()
		 ; cur != replace.end() ;  cur ++ ) {

	    PExpr*tmp = (*cur).second;
	    NetExpr*val = tmp->elaborate_pexpr(des, sc);
	    val = my_scope->set_parameter((*cur).first, val);
	    assert(val);
	    delete val;
      }
}

void PFunction::elaborate_scope(Design*des, NetScope*scope) const
{
}

void PTask::elaborate_scope(Design*des, NetScope*scope) const
{
}


/*
 * $Log: elab_scope.cc,v $
 * Revision 1.1  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 */

