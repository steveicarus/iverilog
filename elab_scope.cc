/*
 * Copyright (c) 2000-2015 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "compiler.h"
# include  "netmisc.h"
# include  <cstring>
# include  <iostream>
# include  <cstdlib>
# include  <cstdio>

/*
 * Elaboration happens in two passes, generally. The first scans the
 * pform to generate the NetScope tree and attach it to the Design
 * object. The methods in this source file implement the elaboration
 * of the scopes.
 */

# include  "Module.h"
# include  "PClass.h"
# include  "PExpr.h"
# include  "PEvent.h"
# include  "PClass.h"
# include  "PGate.h"
# include  "PGenerate.h"
# include  "PPackage.h"
# include  "PTask.h"
# include  "PWire.h"
# include  "Statement.h"
# include  "AStatement.h"
# include  "netlist.h"
# include  "netclass.h"
# include  "netenum.h"
# include  "parse_api.h"
# include  "util.h"
# include  <typeinfo>
# include  <cassert>
# include  "ivl_assert.h"

typedef map<perm_string,LexicalScope::param_expr_t>::const_iterator mparm_it_t;

static void collect_parm_item_(Design*des, NetScope*scope, perm_string name,
			       const LexicalScope::param_expr_t&cur,
                               bool is_annotatable,
                               bool local_flag)
{
      NetScope::range_t*range_list = 0;
      for (LexicalScope::range_t*range = cur.range ; range ; range = range->next) {
	    NetScope::range_t*tmp = new NetScope::range_t;
	    tmp->exclude_flag = range->exclude_flag;
	    tmp->low_open_flag = range->low_open_flag;
	    tmp->high_open_flag = range->high_open_flag;

	    if (range->low_expr) {
		  tmp->low_expr = elab_and_eval(des, scope, range->low_expr, -1);
		  ivl_assert(*range->low_expr, tmp->low_expr);
	    } else {
		  tmp->low_expr = 0;
	    }

	    if (range->high_expr && range->high_expr==range->low_expr) {
		    // Detect the special case of a "point"
		    // range. These are called out by setting the high
		    // and low expression ranges to the same
		    // expression. The exclude_flags should be false
		    // in this case
		  ivl_assert(*range->high_expr, tmp->low_open_flag==false && tmp->high_open_flag==false);
		  tmp->high_expr = tmp->low_expr;

	    } else if (range->high_expr) {
		  tmp->high_expr = elab_and_eval(des, scope, range->high_expr, -1);
		  ivl_assert(*range->high_expr, tmp->high_expr);
	    } else {
		  tmp->high_expr = 0;
	    }

	    tmp->next = range_list;
	    range_list = tmp;
      }


      scope->set_parameter(name, is_annotatable, cur.expr, cur.type, cur.msb,
			   cur.lsb, cur.signed_flag, local_flag, range_list, cur);
}

static void collect_scope_parameters_(Design*des, NetScope*scope,
      const map<perm_string,LexicalScope::param_expr_t>&parameters)
{
      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  ++ cur ) {

	      // A parameter can not have the same name as a genvar.
	    if (scope->find_genvar((*cur).first)) {
		  cerr << cur->second.get_fileline()
		       << ": error: parameter and genvar in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	    collect_parm_item_(des, scope, (*cur).first, (*cur).second, false, false);
      }
}

static void collect_scope_localparams_(Design*des, NetScope*scope,
      const map<perm_string,LexicalScope::param_expr_t>&localparams)
{
      for (mparm_it_t cur = localparams.begin()
		 ; cur != localparams.end() ;  ++ cur ) {

	      // A localparam can not have the same name as a genvar.
	    if (scope->find_genvar((*cur).first)) {
		  cerr << cur->second.get_fileline()
		       << ": error: localparam and genvar in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	    collect_parm_item_(des, scope, (*cur).first, (*cur).second, false, true);
      }
}

static void collect_scope_specparams_(Design*des, NetScope*scope,
      const map<perm_string,LexicalScope::param_expr_t>&specparams)
{
      for (mparm_it_t cur = specparams.begin()
		 ; cur != specparams.end() ;  ++ cur ) {

	      // A specparam can not have the same name as a genvar.
	    if (scope->find_genvar((*cur).first)) {
		  cerr << cur->second.get_fileline()
		       << ": error: specparam and genvar in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	    collect_parm_item_(des, scope, (*cur).first, (*cur).second, true, false);
      }
}

/*
 * Elaborate the enumeration into the given scope. If scope==0, then
 * the enumeration goes into $root instead of a scope.
 */
static void elaborate_scope_enumeration(Design*des, NetScope*scope,
					enum_type_t*enum_type)
{
      bool rc_flag;
      assert(enum_type->range->size() == 1);
      pform_range_t&range = enum_type->range->front();
      NetExpr*msb_ex = elab_and_eval(des, scope, range.first, -1);
      NetExpr*lsb_ex = elab_and_eval(des, scope, range.second, -1);

      long msb = 0;
      rc_flag = eval_as_long(msb, msb_ex);
      assert(rc_flag);
      long lsb = 0;
      rc_flag = eval_as_long(lsb, lsb_ex);
      assert(rc_flag);

      netenum_t*use_enum = new netenum_t(enum_type->base_type,
					 enum_type->signed_flag,
					 enum_type->integer_flag, msb, lsb,
					 enum_type->names->size(),
					 enum_type);

      use_enum->set_line(enum_type->li);
      if (scope)
	    scope->add_enumeration_set(enum_type, use_enum);
      else
	    des->add_enumeration_set(enum_type, use_enum);

      size_t name_idx = 0;
	// Find the enumeration width.
      long raw_width = use_enum->packed_width();
      assert(raw_width > 0);
      unsigned enum_width = (unsigned)raw_width;
	// Define the default start value and the increment value to be the
	// correct type for this enumeration.
      verinum cur_value ((uint64_t)0, enum_width);
      cur_value.has_sign(enum_type->signed_flag);
      verinum one_value ((uint64_t)1, enum_width);
      one_value.has_sign(enum_type->signed_flag);
	// Find the maximum allowed enumeration value.
      verinum max_value (0);
      if (enum_type->signed_flag) {
	    max_value = pow(verinum(2), verinum(enum_width-1)) - one_value;
      } else {
	    max_value = pow(verinum(2), verinum(enum_width)) - one_value;
      }
      max_value.has_sign(enum_type->signed_flag);
	// Variable to indicate when a defined value wraps.
      bool implicit_wrapped = false;
	// Process the enumeration definition.
      for (list<named_pexpr_t>::const_iterator cur = enum_type->names->begin()
		 ; cur != enum_type->names->end() ;  ++ cur, name_idx += 1) {
	      // Check to see if the enumeration name has a value given.
	    if (cur->parm) {
		    // There is an explicit value. elaborate/evaluate
		    // the value and assign it to the enumeration name.
		  NetExpr*val = elab_and_eval(des, scope, cur->parm, -1);
		  NetEConst*val_const = dynamic_cast<NetEConst*> (val);
		  if (val_const == 0) {
			cerr << use_enum->get_fileline()
			     << ": error: Enumeration expression for "
			     << cur->name <<" is not an integer constant."
			     << endl;
			des->errors += 1;
			continue;
		  }
		  cur_value = val_const->value();
		    // Clear the implicit wrapped flag if a parameter is given.
		  implicit_wrapped = false;

		    // A 2-state value can not have a constant with X/Z bits.
		  if (enum_type->base_type==IVL_VT_BOOL &&
		      ! cur_value.is_defined()) {
			cerr << use_enum->get_fileline()
			     << ": error: Enumeration name " << cur->name
			     << " can not have an undefined value." << endl;
			des->errors += 1;
		  }
		    // If this is a literal constant and it has a defined
		    // width then the width must match the enumeration width.
		  if (PENumber *tmp = dynamic_cast<PENumber*>(cur->parm)) {
			if (tmp->value().has_len() &&
			    (tmp->value().len() != enum_width)) {
			      cerr << use_enum->get_fileline()
			           << ": error: Enumeration name " << cur->name
			           << " has an incorrectly sized constant."
			           << endl;
			      des->errors += 1;
			}
		  }

		    // If we are padding/truncating a negative value for an
		    // unsigned enumeration that is an error or if the new
		    // value does not have a defined width.
		  if (((cur_value.len() != enum_width) ||
		       ! cur_value.has_len()) &&
		      ! enum_type->signed_flag && cur_value.is_negative()) {
			cerr << use_enum->get_fileline()
			     << ": error: Enumeration name " << cur->name
			     << " has a negative value." << endl;
			des->errors += 1;
		  }

		    // Narrower values need to be padded to the width of the
		    // enumeration and defined to have the specified width.
		  if (cur_value.len() < enum_width) {
			cur_value = pad_to_width(cur_value, enum_width);
		  }

		    // Some wider values can be truncated.
		  if (cur_value.len() > enum_width) {
			unsigned check_width = enum_width - 1;
			  // Check that the upper bits match the MSB
			for (unsigned idx = enum_width;
			     idx < cur_value.len();
			     idx += 1) {
			      if (cur_value[idx] != cur_value[check_width]) {
				      // If this is an unsigned enumeration
				      // then zero padding is okay.
				    if (! enum_type->signed_flag &&
				        (idx == enum_width) &&
				        (cur_value[idx] == verinum::V0)) {
					  check_width += 1;
					  continue;
				    }
				    if (cur_value.is_defined()) {
					  cerr << use_enum->get_fileline()
					       << ": error: Enumeration name "
					       << cur->name
					       << " has a value that is too "
					       << ((cur_value > max_value) ?
					           "large" : "small")
					       << " " << cur_value << "."
					       << endl;
				    } else {
					  cerr << use_enum->get_fileline()
					       << ": error: Enumeration name "
					       << cur->name
					       << " has trimmed bits that do "
					       << "not match the enumeration "
					       << "MSB: " << cur_value << "."
					       << endl;
				    }
				    des->errors += 1;
				    break;
			      }
			}
			  // If this is an unsigned value then make sure
			  // The upper bits are not 1.
			if (! cur_value.has_sign() &&
			    (cur_value[enum_width] == verinum::V1)) {
			      cerr << use_enum->get_fileline()
			           << ": error: Enumeration name "
			           << cur->name
			           << " has a value that is too large: "
			           << cur_value << "." << endl;
			      des->errors += 1;
			      break;
			}
			cur_value = verinum(cur_value, enum_width);
		  }

		    // At this point the value has the correct size and needs
		    // to have the correct sign attribute set.
		  cur_value.has_len(true);
		  cur_value.has_sign(enum_type->signed_flag);

	    } else if (! cur_value.is_defined()) {
		  cerr << use_enum->get_fileline()
		       << ": error: Enumeration name " << cur->name
		       << " has an undefined inferred value." << endl;
		  des->errors += 1;
		  continue;
	    }

	      // Check to see if an implicitly wrapped value is used.
	    if (implicit_wrapped) {
		  cerr << use_enum->get_fileline()
		       << ": error: Enumeration name " << cur->name
		       << " has an inferred value that overflowed." << endl;
		  des->errors += 1;
	    }

	    // The enumeration value must be unique.
	    perm_string dup_name = use_enum->find_value(cur_value);
	    if (dup_name) {
		  cerr << use_enum->get_fileline()
		       << ": error: Enumeration name "
		       << cur->name << " and " << dup_name
		       << " have the same value: " << cur_value << endl;
		  des->errors += 1;
	    }

	    rc_flag = use_enum->insert_name(name_idx, cur->name, cur_value);
	    if (scope)
		  rc_flag &= scope->add_enumeration_name(use_enum, cur->name);
	    else
		  rc_flag &= des->add_enumeration_name(use_enum, cur->name);

	    if (! rc_flag) {
		  cerr << use_enum->get_fileline()
		       << ": error: Duplicate enumeration name "
		       << cur->name << endl;
		  des->errors += 1;
	    }

	      // In case the next name has an implicit value,
	      // increment the current value by one.
	    if (cur_value.is_defined()) {
		  if (cur_value == max_value) implicit_wrapped = true;
		  cur_value = cur_value + one_value;
	    }
      }

      use_enum->insert_name_close();
}

static void elaborate_scope_enumerations(Design*des, NetScope*scope,
					 const set<enum_type_t*>&enum_types)
{
      for (set<enum_type_t*>::const_iterator cur = enum_types.begin()
		 ; cur != enum_types.end() ; ++ cur) {
	    enum_type_t*curp = *cur;
	    elaborate_scope_enumeration(des, scope, curp);
      }
}

void elaborate_rootscope_enumerations(Design*des)
{
      for (set<enum_type_t*>::const_iterator cur = pform_enum_sets.begin()
		 ; cur != pform_enum_sets.end() ; ++ cur) {
	    enum_type_t*curp = *cur;
	    elaborate_scope_enumeration(des, 0, curp);
      }
}

/*
 * If the pclass includes an implicit and explicit constructor, then
 * merge the implicit constructor into the explicit constructor as
 * statements in the beginning.
 *
 * This is not necessary for proper functionality, it is an
 * optimization, so we can easily give up if it doesn't seem like it
 * will obviously work.
 */
static void blend_class_constructors(PClass*pclass)
{
      perm_string new1 = perm_string::literal("new");
      perm_string new2 = perm_string::literal("new@");

      PFunction*use_new;
      PFunction*use_new2;

	// Locate the explicit constructor.
      map<perm_string,PFunction*>::iterator iter_new = pclass->funcs.find(new1);
      if (iter_new == pclass->funcs.end())
	    use_new = 0;
      else
	    use_new = iter_new->second;

	// Locate the implicit constructor.
      map<perm_string,PFunction*>::iterator iter_new2 = pclass->funcs.find(new2);
      if (iter_new2 == pclass->funcs.end())
	    use_new2 = 0;
      else
	    use_new2 = iter_new2->second;

	// If there are no constructors, then we are done.
      if (use_new==0 && use_new2==0)
	    return;

	// While we're here, look for a super.new() call. If we find
	// it, strip it out of the constructor and set it aside for
	// when we actually call the chained constructor.
      PChainConstructor*chain_new = use_new? use_new->extract_chain_constructor() : 0;

	// If we do not have an explicit constructor chain, but there
	// is a parent class, then create an implicit chain.
      if (chain_new==0 && pclass->type->base_type!=0) {
	    chain_new = new PChainConstructor(pclass->type->base_args);
	    chain_new->set_line(*pclass);
      }

	// If there are both an implicit and explicit constructor,
	// then blend the implicit constructor into the explicit
	// constructor. This eases the task for the elaborator later.
      if (use_new && use_new2) {
	      // These constructors must be methods of the same class.
	    ivl_assert(*use_new, use_new->method_of() == use_new2->method_of());

	    Statement*def_new = use_new->get_statement();
	    Statement*def_new2 = use_new2->get_statement();

	      // It is possible, i.e. recovering from a parse error,
	      // for the statement from the constructor to be
	      // missing. In that case, create an empty one.
	    if (def_new==0) {
		  def_new = new PBlock(PBlock::BL_SEQ);
		  use_new->set_statement(def_new);
	    }

	    if (def_new2) use_new->push_statement_front(def_new2);

	      // Now the implicit initializations are all built into
	      // the constructor. Delete the "new@" constructor.
	    pclass->funcs.erase(iter_new2);
	    delete use_new2;
	    use_new2 = 0;
      }

      if (chain_new) {
	    if (use_new2) {
		  use_new2->push_statement_front(chain_new);
	    } else {
		  use_new->push_statement_front(chain_new);
	    }
	    chain_new = 0;
      }
}

static void elaborate_scope_class(Design*des, NetScope*scope, PClass*pclass)
{
      class_type_t*use_type = pclass->type;

      if (debug_scopes) {
	    cerr << pclass->get_fileline() <<": elaborate_scope_class: "
		 << "Elaborate scope class " << pclass->pscope_name() << endl;
      }

      class_type_t*base_class = dynamic_cast<class_type_t*> (use_type->base_type);
      if (use_type->base_type && !base_class) {
	    cerr << pclass->get_fileline() << ": error: "
		 << "Base type of " << use_type->name
		 << " is not a class." << endl;
	    des->errors += 1;
      }

      netclass_t*use_base_class = 0;
      if (base_class) {
	    ivl_assert(*pclass, scope);
	    use_base_class = scope->find_class(base_class->name);
	    if (use_base_class == 0) {
		  cerr << pclass->get_fileline() << ": error: "
		       << "Base class " << base_class->name
		       << " not found." << endl;
		  des->errors += 1;
	    }
      }

      netclass_t*use_class = new netclass_t(use_type->name, use_base_class);

      ivl_assert(*pclass, use_type->save_elaborated_type == 0);
      use_type->save_elaborated_type = use_class;

	// Class scopes have no parent scope, because references are
	// not allowed to escape a class method.
      NetScope*class_scope = new NetScope(0, hname_t(pclass->pscope_name()),
					  NetScope::CLASS);
      class_scope->set_line(pclass);
      class_scope->set_class_def(use_class);
      use_class->set_class_scope(class_scope);
      use_class->set_definition_scope(scope);

	// Collect the properties, elaborate them, and add them to the
	// elaborated class definition.
      for (map<perm_string, class_type_t::prop_info_t>::iterator cur = use_type->properties.begin()
		 ; cur != use_type->properties.end() ; ++ cur) {

	    ivl_type_s*tmp = cur->second.type->elaborate_type(des, scope);
	    ivl_assert(*pclass, tmp);
	    if (debug_scopes) {
		  cerr << pclass->get_fileline() << ": elaborate_scope_class: "
		       << "  Property " << cur->first
		       << " type=" << *tmp << endl;
	    }
	    use_class->set_property(cur->first, cur->second.qual, tmp);

      }

      for (map<perm_string,PTask*>::iterator cur = pclass->tasks.begin()
		 ; cur != pclass->tasks.end() ; ++cur) {

	    hname_t use_name (cur->first);
	    NetScope*method_scope = new NetScope(class_scope, use_name, NetScope::TASK);
	      // Task methods are always automatic...
	    method_scope->is_auto(true);
	    method_scope->set_line(cur->second);

	    if (debug_scopes) {
		  cerr << cur->second->get_fileline() << ": elaborate_scope_class: "
		       << "Elaborate method (task) scope "
		       << scope_path(method_scope) << endl;
	    }

	    cur->second->elaborate_scope(des, method_scope);
      }

      for (map<perm_string,PFunction*>::iterator cur = pclass->funcs.begin()
		 ; cur != pclass->funcs.end() ; ++cur) {

	    hname_t use_name (cur->first);
	    NetScope*method_scope = new NetScope(class_scope, use_name, NetScope::FUNC);
	      // Function methods are always automatic...
	    method_scope->is_auto(true);
	    method_scope->set_line(cur->second);

	    if (debug_scopes) {
		  cerr << cur->second->get_fileline() << ": elaborate_scope_class: "
		       << "Elaborate method (function) scope "
		       << scope_path(method_scope) << endl;
	    }

	    cur->second->elaborate_scope(des, method_scope);
      }

      if (scope) {
	    scope->add_class(use_class);

      } else {
	    des->add_class(use_class, pclass);
      }
}

static void elaborate_scope_classes(Design*des, NetScope*scope,
				    const vector<PClass*>&classes)
{
      for (size_t idx = 0 ; idx < classes.size() ; idx += 1) {
	    blend_class_constructors(classes[idx]);
	    elaborate_scope_class(des, scope, classes[idx]);
      }
}

void elaborate_rootscope_classes(Design*des)
{
      if (pform_classes.empty())
	    return;

      for (map<perm_string,PClass*>::iterator cur = pform_classes.begin()
		 ; cur != pform_classes.end() ; ++ cur) {
	    blend_class_constructors(cur->second);
	    elaborate_scope_class(des, 0, cur->second);
      }
}

static void replace_scope_parameters_(NetScope*scope, const LineInfo&loc,
			              const Module::replace_t&replacements)
{
      for (Module::replace_t::const_iterator cur = replacements.begin()
		 ; cur != replacements.end() ;  ++ cur ) {

	    PExpr*val = (*cur).second;
	    if (val == 0) {
		  cerr << loc.get_fileline() << ": internal error: "
		       << "Missing expression in parameter replacement for "
		       << (*cur).first << endl;;
	    }
	    assert(val);
	    if (debug_scopes) {
		  cerr << loc.get_fileline() << ": debug: "
		       << "Replace " << (*cur).first
		       << " with expression " << *val
		       << " from " << val->get_fileline() << "." << endl;
		  cerr << loc.get_fileline() << ":      : "
		       << "Type=" << val->expr_type() << endl;
	    }
	    bool flag = scope->replace_parameter((*cur).first, val,
                                                 scope->parent());
	    if (! flag) {
		  cerr << val->get_fileline() << ": warning: parameter "
		       << (*cur).first << " not found in "
		       << scope_path(scope) << "." << endl;
	    }
      }
}

static void elaborate_scope_events_(Design*des, NetScope*scope,
                                    const map<perm_string,PEvent*>&events)
{
      for (map<perm_string,PEvent*>::const_iterator et = events.begin()
		 ; et != events.end() ;  ++ et ) {

	    (*et).second->elaborate_scope(des, scope);
      }
}

static void elaborate_scope_task(Design*des, NetScope*scope, PTask*task)
{
      hname_t use_name( task->pscope_name() );

      NetScope*task_scope = new NetScope(scope, use_name, NetScope::TASK);
      task_scope->is_auto(task->is_auto());
      task_scope->set_line(task);

      if (scope==0)
	    des->add_root_task(task_scope, task);

      if (debug_scopes) {
	    cerr << task->get_fileline() << ": elaborate_scope_task: "
		 << "Elaborate task scope " << scope_path(task_scope) << endl;
      }

      task->elaborate_scope(des, task_scope);
}

static void elaborate_scope_tasks(Design*des, NetScope*scope,
				  const map<perm_string,PTask*>&tasks)
{
      typedef map<perm_string,PTask*>::const_iterator tasks_it_t;

      for (tasks_it_t cur = tasks.begin()
		 ; cur != tasks.end() ;  ++ cur ) {

	    hname_t use_name( (*cur).first );
	      // A task can not have the same name as another scope object.
	    const NetScope *child = scope->child(use_name);
	    if (child) {
		  cerr << cur->second->get_fileline() << ": error: task and ";
		  child->print_type(cerr);
		  cerr << " in '" << scope->fullname()
		       << "' have the same name '" << use_name << "'." << endl;
		  des->errors += 1;
		  continue;
	    }

	      // A task can not have the same name as a genvar.
	    if (scope->find_genvar((*cur).first)) {
		  cerr << cur->second->get_fileline()
		       << ": error: task and genvar in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	      // A task can not have the same name as a parameter.
	    const NetExpr *ex_msb, *ex_lsb;
	    const NetExpr *parm = scope->get_parameter(des, (*cur).first,
                                                       ex_msb, ex_lsb);
	    if (parm) {
		  cerr << cur->second->get_fileline()
		       << ": error: task and parameter in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	    elaborate_scope_task(des, scope, cur->second);
      }

}

static void elaborate_scope_func(Design*des, NetScope*scope, PFunction*task)
{
      hname_t use_name( task->pscope_name() );

      NetScope*task_scope = new NetScope(scope, use_name, NetScope::FUNC);
      task_scope->is_auto(task->is_auto());
      task_scope->set_line(task);

      if (scope==0)
	    des->add_root_task(task_scope, task);

      if (debug_scopes) {
	    cerr << task->get_fileline() << ": elaborate_scope_func: "
		 << "Elaborate task scope " << scope_path(task_scope) << endl;
      }

      task->elaborate_scope(des, task_scope);
}

static void elaborate_scope_funcs(Design*des, NetScope*scope,
				  const map<perm_string,PFunction*>&funcs)
{
      typedef map<perm_string,PFunction*>::const_iterator funcs_it_t;

      for (funcs_it_t cur = funcs.begin()
		 ; cur != funcs.end() ;  ++ cur ) {

	    hname_t use_name( (*cur).first );
	      // A function can not have the same name as another scope object.
	    const NetScope *child = scope->child(use_name);
	    if (child) {
		  cerr << cur->second->get_fileline()
		       << ": error: function and ";
		  child->print_type(cerr);
		  cerr << " in '" << scope->fullname()
		       << "' have the same name '" << use_name << "'." << endl;
		  des->errors += 1;
		  continue;
	    }

	      // A function can not have the same name as a genvar.
	    if (scope->find_genvar((*cur).first)) {
		  cerr << cur->second->get_fileline()
		       << ": error: function and genvar in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	      // A function can not have the same name as a parameter.
	    const NetExpr *ex_msb, *ex_lsb;
	    const NetExpr *parm = scope->get_parameter(des, (*cur).first,
                                                       ex_msb, ex_lsb);
	    if (parm) {
		  cerr << cur->second->get_fileline()
		       << ": error: function and parameter in '"
		       << scope->fullname() << "' have the same name '"
		       << (*cur).first << "'." << endl;
		  des->errors += 1;
	    }

	    elaborate_scope_func(des, scope, cur->second);
      }

}

void elaborate_rootscope_tasks(Design*des)
{
      for (map<perm_string,PTaskFunc*>::iterator cur = pform_tasks.begin()
		 ; cur != pform_tasks.end() ; ++ cur) {

	    if (PTask*task = dynamic_cast<PTask*> (cur->second)) {
		  elaborate_scope_task(des, 0, task);
		  continue;
	    }

	    if (PFunction*func = dynamic_cast<PFunction*>(cur->second)) {
		  elaborate_scope_func(des, 0, func);
		  continue;
	    }

	    cerr << cur->second->get_fileline() << ": internal error: "
		 << "elaborate_rootscope_tasks does not understand "
		 << "this object," << endl;
	    des->errors += 1;
      }
}

class generate_schemes_work_item_t : public elaborator_work_item_t {
    public:
      generate_schemes_work_item_t(Design*des__, NetScope*scope, Module*mod)
      : elaborator_work_item_t(des__), scope_(scope), mod_(mod)
      { }

      void elaborate_runrun()
      {
	    if (debug_scopes)
		  cerr << mod_->get_fileline() << ": debug: "
		       << "Processing generate schemes for "
		       << scope_path(scope_) << endl;

	      // Generate schemes can create new scopes in the form of
	      // generated code. Scan the generate schemes, and *generate*
	      // new scopes, which is slightly different from simple
	      // elaboration.
	    typedef list<PGenerate*>::const_iterator generate_it_t;
	    for (generate_it_t cur = mod_->generate_schemes.begin()
		       ; cur != mod_->generate_schemes.end() ; ++ cur ) {
		  (*cur) -> generate_scope(des, scope_);
	    }
      }

    private:
	// The scope_ is the scope that contains the generate scheme
	// we are to work on. the mod_ is the Module definition for
	// that scope, and contains the parsed generate schemes.
      NetScope*scope_;
      Module*mod_;
};

bool PPackage::elaborate_scope(Design*des, NetScope*scope)
{
      if (debug_scopes) {
	    cerr << get_fileline() << ": PPackage::elaborate_scope: "
		 << "Elaborate package " << scope_path(scope) << "." << endl;
      }

      collect_scope_parameters_(des, scope, parameters);
      collect_scope_localparams_(des, scope, localparams);
      elaborate_scope_enumerations(des, scope, enum_sets);
      elaborate_scope_classes(des, scope, classes_lexical);
      elaborate_scope_funcs(des, scope, funcs);
      elaborate_scope_tasks(des, scope, tasks);
      return true;
}

bool Module::elaborate_scope(Design*des, NetScope*scope,
			     const replace_t&replacements)
{
      if (debug_scopes) {
	    cerr << get_fileline() << ": Module::elaborate_scope: "
		 << "Elaborate " << scope_path(scope) << "." << endl;
      }

	// Add the genvars to the scope.
      typedef map<perm_string,LineInfo*>::const_iterator genvar_it_t;
      for (genvar_it_t cur = genvars.begin(); cur != genvars.end(); ++ cur ) {
	    scope->add_genvar((*cur).first, (*cur).second);
      }

	// Scan the parameters in the module, and store the information
	// needed to evaluate the parameter expressions. The expressions
	// will be evaluated later, once all parameter overrides for this
	// module have been done.

      collect_scope_parameters_(des, scope, parameters);

      collect_scope_localparams_(des, scope, localparams);

      collect_scope_specparams_(des, scope, specparams);

	// Run parameter replacements that were collected from the
	// containing scope and meant for me.

      replace_scope_parameters_(scope, *this, replacements);

      elaborate_scope_enumerations(des, scope, enum_sets);

      assert(classes.size() == classes_lexical.size());
      elaborate_scope_classes(des, scope, classes_lexical);

	// Run through the defparams for this module and save the result
	// in a table for later final override.

      typedef list<Module::named_expr_t>::const_iterator defparms_iter_t;
      for (defparms_iter_t cur = defparms.begin()
		 ; cur != defparms.end() ; ++ cur ) {
	    scope->defparams.push_back(make_pair(cur->first, cur->second));
      }

	// Evaluate the attributes. Evaluate them in the scope of the
	// module that the attribute is attached to. Is this correct?
      unsigned nattr;
      attrib_list_t*attr = evaluate_attributes(attributes, nattr, des, scope);

      for (unsigned idx = 0 ;  idx < nattr ;  idx += 1)
	    scope->attribute(attr[idx].key, attr[idx].val);

      delete[]attr;

	// Generate schemes need to have their scopes elaborated, but
	// we can not do that until defparams are run, so push it off
	// into an elaborate work item.
      if (debug_scopes)
	    cerr << get_fileline() << ": debug: "
		 << "Schedule generates within " << scope_path(scope)
		 << " for elaboration after defparams." << endl;

      des->elaboration_work_list.push_back(new generate_schemes_work_item_t(des, scope, this));

	// Tasks introduce new scopes, so scan the tasks in this
	// module. Create a scope for the task and pass that to the
	// elaborate_scope method of the PTask for detailed
	// processing.

      elaborate_scope_tasks(des, scope, tasks);


	// Functions are very similar to tasks, at least from the
	// perspective of scopes. So handle them exactly the same
	// way.

      elaborate_scope_funcs(des, scope, funcs);

	// Look for implicit modules and implicit gates for them.

      for (map<perm_string,Module*>::iterator cur = nested_modules.begin()
		 ; cur != nested_modules.end() ; ++cur) {
	      // Skip modules that must be explicitly instantiated.
	    if (cur->second->port_count() > 0)
		  continue;

	    PGModule*nested_gate = new PGModule(cur->second, cur->second->mod_name());
	    nested_gate->set_line(*cur->second);
	    gates_.push_back(nested_gate);
      }

	// Gates include modules, which might introduce new scopes, so
	// scan all of them to create those scopes.

      typedef list<PGate*>::const_iterator gates_it_t;
      for (gates_it_t cur = gates_.begin()
		 ; cur != gates_.end() ; ++ cur ) {

	    (*cur) -> elaborate_scope(des, scope);
      }


	// initial and always blocks may contain begin-end and
	// fork-join blocks that can introduce scopes. Therefore, I
	// get to scan processes here.

      typedef list<PProcess*>::const_iterator proc_it_t;

      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ; ++ cur ) {

	    (*cur) -> statement() -> elaborate_scope(des, scope);
      }

	// Scan through all the named events in this scope. We do not
	// need anything more than the current scope to do this
	// elaboration, so do it now. This allows for normal
	// elaboration to reference these events.

      elaborate_scope_events_(des, scope, events);

      scope->is_cell(is_cell);

      return des->errors == 0;
}

bool PGenerate::generate_scope(Design*des, NetScope*container)
{
      switch (scheme_type) {
	  case GS_LOOP:
	    return generate_scope_loop_(des, container);

	  case GS_CONDIT:
	    return generate_scope_condit_(des, container, false);

	  case GS_ELSE:
	    return generate_scope_condit_(des, container, true);

	  case GS_CASE:
	    return generate_scope_case_(des, container);

	  case GS_NBLOCK:
	    return generate_scope_nblock_(des, container);

	  case GS_CASE_ITEM:
	    cerr << get_fileline() << ": internal error: "
		 << "Case item outside of a case generate scheme?" << endl;
	    return false;

	  default:
	    cerr << get_fileline() << ": sorry: Generate of this sort"
		 << " is not supported yet!" << endl;
	    return false;
      }
}

/*
 * This is the elaborate scope method for a generate loop.
 */
bool PGenerate::generate_scope_loop_(Design*des, NetScope*container)
{
	// Check that the loop_index variable was declared in a
	// genvar statement.
      NetScope*cscope = container;
      while (cscope && !cscope->find_genvar(loop_index))
            cscope = cscope->parent();
      if (!cscope) {
	    cerr << get_fileline() << ": error: genvar is missing for "
	            "generate \"loop\" variable '" << loop_index << "'."
	         << endl;
	    des->errors += 1;
	    return false;
      }

	// We're going to need a genvar...
      int genvar;

	// The initial value for the genvar does not need (nor can it
	// use) the genvar itself, so we can evaluate this expression
	// the same way any other parameter value is evaluated.
      NetExpr*init_ex = elab_and_eval(des, container, loop_init, -1, true);
      NetEConst*init = dynamic_cast<NetEConst*> (init_ex);
      if (init == 0) {
	    cerr << get_fileline() << ": error: Cannot evaluate genvar"
		 << " init expression: " << *loop_init << endl;
	    des->errors += 1;
	    return false;
      }

	// Check the generate block name.

	// A generate "loop" can not have the same name as another
	// scope object. Find any scope with this name, not just an
	// exact match scope.
      const NetScope *child = container->child_byname(scope_name);
      if (child) {
	    cerr << get_fileline() << ": error: generate \"loop\" and ";
	    child->print_type(cerr);
	    cerr << " in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
	    return false;
      }

	// A generate "loop" can not have the same name as a genvar.
      if (container->find_genvar(scope_name)) {
	    cerr << get_fileline() << ": error: generate \"loop\" and "
	            "genvar in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "loop" can not have the same name as a named event.
      const NetEvent *event = container->find_event(scope_name);
      if (event) {
	    cerr << get_fileline() << ": error: generate \"loop\" and "
	            "named event in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "loop" can not have the same name as a parameter.
      const NetExpr*tmsb;
      const NetExpr*tlsb;
      const NetExpr*texpr = container->get_parameter(des, scope_name,
                                                     tmsb, tlsb);
      if (texpr != 0) {
	    cerr << get_fileline() << ": error: generate \"loop\" and "
	            "parameter in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
      }

	// These have all been checked so we just need to skip the actual
	// generation for these name conflicts. Not skipping these two will
	// cause the compiler to have problems (assert, inf. loop, etc.).
      if (container->get_parameter(des, loop_index, tmsb, tlsb)) return false;
      if (container->find_event(loop_index)) return false;

      genvar = init->value().as_long();
      delete init_ex;

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: genvar init = " << genvar << endl;
      container->genvar_tmp = loop_index;
      container->genvar_tmp_val = genvar;
      NetExpr*test_ex = elab_and_eval(des, container, loop_test, -1, true);
      NetEConst*test = dynamic_cast<NetEConst*>(test_ex);
      if (test == 0) {
	    cerr << get_fileline() << ": error: Cannot evaluate genvar"
		 << " conditional expression: " << *loop_test << endl;
	    des->errors += 1;
	    return false;
      }
      while (test->value().as_long()) {

	      // The actual name of the scope includes the genvar so
	      // that each instance has a unique name in the
	      // container. The format of using [] is part of the
	      // Verilog standard.
	    hname_t use_name (scope_name, genvar);

	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: "
		       << "Create generated scope " << use_name << endl;

	    NetScope*scope = new NetScope(container, use_name,
					  NetScope::GENBLOCK);
	    scope->set_line(get_file(), get_lineno());

	      // Set in the scope a localparam for the value of the
	      // genvar within this instance of the generate
	      // block. Code within this scope thus has access to the
	      // genvar as a constant.
	    {
		  verinum genvar_verinum(genvar);
		  genvar_verinum.has_sign(true);
		  NetEConstParam*gp = new NetEConstParam(scope,
							 loop_index,
							 genvar_verinum);
		    // The file and line information should really come
		    // from the genvar statement, not the for loop.
		  scope->set_parameter(loop_index, gp, *this);
		  if (debug_scopes)
			cerr << get_fileline() << ": debug: "
			     << "Create implicit localparam "
			     << loop_index << " = " << genvar_verinum << endl;
	    }

	    elaborate_subscope_(des, scope);

	      // Calculate the step for the loop variable.
	    NetExpr*step_ex = elab_and_eval(des, container, loop_step, -1, true);
	    NetEConst*step = dynamic_cast<NetEConst*>(step_ex);
	    if (step == 0) {
		  cerr << get_fileline() << ": error: Cannot evaluate genvar"
		       << " step expression: " << *loop_step << endl;
		  des->errors += 1;
		  return false;
	    }
	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: genvar step from "
		       << genvar << " to " << step->value().as_long() << endl;

	    genvar = step->value().as_long();
	    container->genvar_tmp_val = genvar;
	    delete step;
	    delete test_ex;
	    test_ex = elab_and_eval(des, container, loop_test, -1);
	    test = dynamic_cast<NetEConst*>(test_ex);
	    assert(test);
      }

	// Clear the genvar_tmp field in the scope to reflect that the
	// genvar is no longer valid for evaluating expressions.
      container->genvar_tmp = perm_string();

      return true;
}

bool PGenerate::generate_scope_condit_(Design*des, NetScope*container, bool else_flag)
{
      NetExpr*test_ex = elab_and_eval(des, container, loop_test, -1, true);
      NetEConst*test = dynamic_cast<NetEConst*> (test_ex);
      if (test == 0) {
	    cerr << get_fileline() << ": error: Cannot evaluate genvar"
		 << " conditional expression: " << *loop_test << endl;
	    des->errors += 1;
	    return false;
      }

	// If the condition evaluates as false, then do not create the
	// scope.
      if ( (test->value().as_long() == 0 && !else_flag)
	|| (test->value().as_long() != 0 &&  else_flag) ) {
	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: Generate condition "
		       << (else_flag? "(else)" : "(if)")
		       << " value=" << test->value() << ": skip generation"
		       << endl;
	    delete test_ex;
	    return true;
      }

      hname_t use_name (scope_name);
	// A generate "if" can not have the same name as another scope object.
      const NetScope *child = container->child(use_name);
      if (child) {
	    cerr << get_fileline() << ": error: generate \"if\" and ";
	    child->print_type(cerr);
	    cerr << " in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
	    return false;
      }

	// A generate "if" can not have the same name as a genvar.
      if (container->find_genvar(scope_name)) {
	    cerr << get_fileline() << ": error: generate \"if\" and "
	            "genvar in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "if" can not have the same name as a named event.
      const NetEvent *event = container->find_event(scope_name);
      if (event) {
	    cerr << get_fileline() << ": error: generate \"if\" and "
	            "named event in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "if" can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = container->get_parameter(des, scope_name,
                                                     ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: generate \"if\" and "
	            "parameter in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: Generate condition "
		 << (else_flag? "(else)" : "(if)")
		 << " value=" << test->value() << ": Generate scope="
		 << use_name << endl;

      probe_for_direct_nesting_();
      if (direct_nested_) {
	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: Generate condition "
		       << (else_flag? "(else)" : "(if)")
		       << " detected direct nesting." << endl;
	    elaborate_subscope_direct_(des, container);
	    return true;
      }

	// If this is not directly nested, then generate a scope
	// for myself. That is what I will pass to the subscope.
      NetScope*scope = new NetScope(container, use_name, NetScope::GENBLOCK);
      scope->set_line(get_file(), get_lineno());

      elaborate_subscope_(des, scope);

      return true;
}

bool PGenerate::generate_scope_case_(Design*des, NetScope*container)
{
      NetExpr*case_value_ex = elab_and_eval(des, container, loop_test, -1, true);
      NetEConst*case_value_co = dynamic_cast<NetEConst*>(case_value_ex);
      if (case_value_co == 0) {
	    cerr << get_fileline() << ": error: Cannot evaluate genvar case"
		 << " expression: " << *loop_test << endl;
	    des->errors += 1;
	    return false;
      }

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: Generate case "
		 << "switch value=" << case_value_co->value() << endl;

      PGenerate*default_item = 0;

      typedef list<PGenerate*>::const_iterator generator_it_t;
      generator_it_t cur = generate_schemes.begin();
      while (cur != generate_schemes.end()) {
	    PGenerate*item = *cur;
	    assert( item->scheme_type == PGenerate::GS_CASE_ITEM );

	      // Detect that the item is a default.
	    if (item->item_test.size() == 0) {
		  default_item = item;
		  ++ cur;
		  continue;
	    }

	    bool match_flag = false;
	    for (unsigned idx = 0 ; idx < item->item_test.size() && !match_flag ; idx +=1 ) {
		  NetExpr*item_value_ex = elab_and_eval(des, container,
                                                        item->item_test[idx],
                                                        -1, true);
		  NetEConst*item_value_co = dynamic_cast<NetEConst*>(item_value_ex);
		  if (item_value_co == 0) {
			cerr << get_fileline() << ": error: Cannot evaluate "
			     << " genvar case item expression: "
			     << *item->item_test[idx] << endl;
			des->errors += 1;
			return false;
		  }

		  if (debug_scopes)
			cerr << get_fileline() << ": debug: Generate case "
			     << "item value=" << item_value_co->value() << endl;

		  if (case_value_co->value() == item_value_co->value())
			match_flag = true;
		  delete item_value_co;
	    }

	      // If we stumble on the item that matches, then break out now.
	    if (match_flag)
		  break;

	    ++ cur;
      }

      delete case_value_co;
      case_value_co = 0;

      PGenerate*item = (cur == generate_schemes.end())? default_item : *cur;
      if (item == 0) {
	    cerr << get_fileline() << ": debug: "
		 << "No generate items found" << endl;
	    return true;
      }

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: "
		 << "Generate case matches item at "
		 << item->get_fileline() << endl;

	// The name of the scope to generate, whatever that item is.
      hname_t use_name (item->scope_name);
	// A generate "case" can not have the same name as another scope object.
      const NetScope *child = container->child(use_name);
      if (child) {
	    cerr << get_fileline() << ": error: generate \"case\" and ";
	    child->print_type(cerr);
	    cerr << " in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
	    return false;
      }

	// A generate "case" can not have the same name as a genvar.
      if (container->find_genvar(item->scope_name)) {
	    cerr << get_fileline() << ": error: generate \"case\" and "
	            "genvar in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "case" can not have the same name as a named event.
      const NetEvent *event = container->find_event(item->scope_name);
      if (event) {
	    cerr << get_fileline() << ": error: generate \"case\" and "
	            "named event in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "case" can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = container->get_parameter(des, item->scope_name,
                                                     ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: generate \"case\" and "
	            "parameter in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

      item->probe_for_direct_nesting_();
      if (item->direct_nested_) {
	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: Generate case item " << scope_name
		       << " detected direct nesting." << endl;
	    item->elaborate_subscope_direct_(des, container);
	    return true;
      }

      if (debug_scopes) {
	    cerr << get_fileline() << ": PGenerate::generate_scope_case_: "
		 << "Generate subscope " << use_name
		 << " and elaborate." << endl;
      }

      NetScope*scope = new NetScope(container, use_name,
				    NetScope::GENBLOCK);
      scope->set_line(get_file(), get_lineno());
      item->elaborate_subscope_(des, scope);

      return true;
}

bool PGenerate::generate_scope_nblock_(Design*des, NetScope*container)
{
      hname_t use_name (scope_name);
	// A generate "block" can not have the same name as another scope
	// object.
      const NetScope *child = container->child(use_name);
      if (child) {
	    cerr << get_fileline() << ": error: generate \"block\" and ";
	    child->print_type(cerr);
	    cerr << " in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
	    return false;
      }

	// A generate "block" can not have the same name as a genvar.
      if (container->find_genvar(scope_name)) {
	    cerr << get_fileline() << ": error: generate \"block\" and "
	            "genvar in '" << container->fullname()
	         << "' have the same name '" << scope_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "block" can not have the same name as a named event.
      const NetEvent *event = container->find_event(scope_name);
      if (event) {
	    cerr << get_fileline() << ": error: generate \"block\" and "
	            "named event in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

	// A generate "block" can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = container->get_parameter(des, scope_name,
                                                     ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: generate \"block\" and "
	            "parameter in '" << container->fullname()
	         << "' have the same name '" << use_name << "'." << endl;
	    des->errors += 1;
      }

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: Generate named block "
		 << ": Generate scope=" << use_name << endl;

      NetScope*scope = new NetScope(container, use_name,
				    NetScope::GENBLOCK);
      scope->set_line(get_file(), get_lineno());

      elaborate_subscope_(des, scope);

      return true;
}

void PGenerate::elaborate_subscope_direct_(Design*des, NetScope*scope)
{
      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    PGenerate*curp = *cur;
	    if (debug_scopes) {
		  cerr << get_fileline() << ": elaborate_subscope_direct_: "
		       << "Elaborate direct subscope " << curp->scope_name
		       << " within scope " << scope_name << endl;
	    }
	    curp -> generate_scope(des, scope);
      }
}

void PGenerate::elaborate_subscope_(Design*des, NetScope*scope)
{
	// Add the genvars to this scope.
      typedef map<perm_string,LineInfo*>::const_iterator genvar_it_t;
      for (genvar_it_t cur = genvars.begin(); cur != genvars.end(); ++ cur ) {
	    scope->add_genvar((*cur).first, (*cur).second);
      }

	// Scan the localparams in this scope, and store the information
        // needed to evaluate the parameter expressions. The expressions
	// will be evaluated later, once all parameter overrides for this
	// module have been done.
      collect_scope_localparams_(des, scope, localparams);

	// Run through the defparams for this scope and save the result
	// in a table for later final override.

      typedef list<PGenerate::named_expr_t>::const_iterator defparms_iter_t;
      for (defparms_iter_t cur = defparms.begin()
		 ; cur != defparms.end() ; ++ cur ) {
	    scope->defparams.push_back(make_pair(cur->first, cur->second));
      }

	// Scan the generated scope for nested generate schemes,
	// and *generate* new scopes, which is slightly different
	// from simple elaboration.

      typedef list<PGenerate*>::const_iterator generate_it_t;
      for (generate_it_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur) -> generate_scope(des, scope);
      }

        // Scan through all the task and function declarations in this
        // scope.
      elaborate_scope_tasks(des, scope, tasks);
      elaborate_scope_funcs(des, scope, funcs);

	// Scan the generated scope for gates that may create
	// their own scopes.
      typedef list<PGate*>::const_iterator pgate_list_it_t;
      for (pgate_list_it_t cur = gates.begin()
		 ; cur != gates.end() ; ++ cur ) {
	    (*cur) ->elaborate_scope(des, scope);
      }

      typedef list<PProcess*>::const_iterator proc_it_t;
      for (proc_it_t cur = behaviors.begin()
		 ; cur != behaviors.end() ; ++ cur ) {
	    (*cur) -> statement() -> elaborate_scope(des, scope);
      }

	// Scan through all the named events in this scope.
      elaborate_scope_events_(des, scope, events);

      if (debug_scopes)
	    cerr << get_fileline() << ": debug: Generated scope " << scope_path(scope)
		 << " for generate block " << scope_name << endl;

	// Save the scope that we created, for future use.
      scope_list_.push_back(scope);
}

class delayed_elaborate_scope_mod_instances : public elaborator_work_item_t {

    public:
      delayed_elaborate_scope_mod_instances(Design*des__,
					    const PGModule*obj,
					    Module*mod,
					    NetScope*sc)
      : elaborator_work_item_t(des__), obj_(obj), mod_(mod), sc_(sc)
      { }
      ~delayed_elaborate_scope_mod_instances() { }

      virtual void elaborate_runrun();

    private:
      const PGModule*obj_;
      Module*mod_;
      NetScope*sc_;
};

void delayed_elaborate_scope_mod_instances::elaborate_runrun()
{
      if (debug_scopes)
	    cerr << obj_->get_fileline() << ": debug: "
		 << "Resume scope elaboration of instances of "
		 << mod_->mod_name() << "." << endl;

      obj_->elaborate_scope_mod_instances_(des, mod_, sc_);
}

/*
 * Here we handle the elaborate scope of a module instance. The caller
 * has already figured out that this "gate" is a module, and has found
 * the module definition. The "sc" argument is the scope that will
 * contain this instance.
 */
void PGModule::elaborate_scope_mod_(Design*des, Module*mod, NetScope*sc) const
{
      if (get_name() == "") {
	    cerr << get_fileline() << ": error: Instantiation of module "
		 << mod->mod_name() << " requires an instance name." << endl;
	    des->errors += 1;
	    return;
      }

	// Missing module instance names have already been rejected.
      assert(get_name() != "");

	// A module instance can not have the same name as another scope object.
      const NetScope *child = sc->child(hname_t(get_name()));
      if (child) {
	    cerr << get_fileline() << ": error: module <" << mod->mod_name()
	         << "> instance and ";
	    child->print_type(cerr);
	    cerr << " in '" << sc->fullname()
	         << "' have the same name '" << get_name() << "'." << endl;
	    des->errors += 1;
	    return;
      }

	// A module instance can not have the same name as a genvar.
      if (sc->find_genvar(get_name())) {
	    cerr << get_fileline() << ": error: module <" << mod->mod_name()
	         << "> instance and genvar in '" << sc->fullname()
	         << "' have the same name '" << get_name() << "'." << endl;
	    des->errors += 1;
      }

	// A module instance can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = sc->get_parameter(des, get_name(), ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: module <" << mod->mod_name()
	         << "> instance and parameter in '" << sc->fullname()
	         << "' have the same name '" << get_name() << "'." << endl;
	    des->errors += 1;
      }

	// check for recursive instantiation by scanning the current
	// scope and its parents. Look for a module instantiation of
	// the same module, but farther up in the scope.
      unsigned rl_count = 0;
      bool in_genblk = false;
      for (NetScope*scn = sc ;  scn ;  scn = scn->parent()) {
	      // We need to know if we are inside a generate block to allow
	      // recursive instances.
	    if (scn->type() == NetScope::GENBLOCK) {
		  in_genblk = true;
		  continue;
	    }

	    if (scn->type() != NetScope::MODULE) continue;

	    if (strcmp(mod->mod_name(), scn->module_name()) != 0) continue;

	      // We allow nested scopes if they are inside a generate block,
	      // but only to a certain nesting depth.
	    if (in_genblk) {
		  rl_count += 1;
		  if (rl_count > recursive_mod_limit) {
			cerr << get_fileline() << ": error: instance "
			     << scope_path(sc) << "." << get_name()
			     << " of module " << mod->mod_name()
			     << " is nested too deep." << endl;
			cerr << get_fileline() << ":      : check for "
			        "proper recursion termination or increase the "
			        "limit (" << recursive_mod_limit
			     << ") with the -pRECURSIVE_MOD_LIMIT flag."
			     << endl;
			des->errors += 1;
			return;
		  }
		  continue;
	    }

	    cerr << get_fileline() << ": error: You can not instantiate "
		 << "module " << mod->mod_name() << " within itself." << endl;
	    cerr << get_fileline() << ":      : The offending instance is "
		 << get_name() << " within " << scope_path(scn) << "." << endl;
	    des->errors += 1;
	    return;
      }

      if (msb_ || lsb_) {
	      // If there are expressions to evaluate in order to know
	      // the actual number of instances that will be
	      // instantiated, then we have to delay further scope
	      // elaboration until after defparams (above me) are
	      // run. Do that by appending a work item to the
	      // elaboration work list.
	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: delay elaborate_scope"
		       << " of array of " << get_name()
		       << " in scope " << scope_path(sc) << "." << endl;

	    elaborator_work_item_t*tmp
		  = new delayed_elaborate_scope_mod_instances(des, this, mod, sc);
	    des->elaboration_work_list.push_back(tmp);

      } else {
	      // If there are no expressions that need to be evaluated
	      // to elaborate the scope of this next instances, then
	      // get right to it.
	    elaborate_scope_mod_instances_(des, mod, sc);
      }
}

/*
 * This method is called to process a module instantiation after basic
 * sanity testing is already complete.
 */
void PGModule::elaborate_scope_mod_instances_(Design*des, Module*mod, NetScope*sc) const
{
      NetExpr*mse = msb_ ? elab_and_eval(des, sc, msb_, -1, true) : 0;
      NetExpr*lse = lsb_ ? elab_and_eval(des, sc, lsb_, -1, true) : 0;
      NetEConst*msb = dynamic_cast<NetEConst*> (mse);
      NetEConst*lsb = dynamic_cast<NetEConst*> (lse);

      assert( (msb == 0) || (lsb != 0) );

      long instance_low  = 0;
      long instance_high = 0;
      long instance_count  = 1;
      bool instance_array = false;

      if (msb) {
	    instance_array = true;
	    instance_high = msb->value().as_long();
	    instance_low  = lsb->value().as_long();
	    if (instance_high > instance_low)
		  instance_count = instance_high - instance_low + 1;
	    else
		  instance_count = instance_low - instance_high + 1;

	    delete mse;
	    delete lse;
      }

      NetScope::scope_vec_t instances (instance_count);
      if (debug_scopes) {
	    cerr << get_fileline() << ": debug: Create " << instance_count
		 << " instances of " << get_name()
		 << "." << endl;
      }

	// Run through the module instances, and make scopes out of
	// them. Also do parameter overrides that are done on the
	// instantiation line.
      for (int idx = 0 ;  idx < instance_count ;  idx += 1) {

	    hname_t use_name (get_name());

	    if (instance_array) {
		  int instance_idx = idx;
		  if (instance_low < instance_high)
			instance_idx = instance_low + idx;
		  else
			instance_idx = instance_low - idx;

		  use_name = hname_t(get_name(), instance_idx);
	    }

	    if (debug_scopes) {
		  cerr << get_fileline() << ": debug: Module instance " << use_name
		       << " becomes child of " << scope_path(sc)
		       << "." << endl;
	    }

	      // Create the new scope as a MODULE with my name. Note
	      // that if this is a nested module, mark it thus so that
	      // scope searches will continue into the parent scope.
	    NetScope*my_scope = new NetScope(sc, use_name, NetScope::MODULE,
					     bound_type_? true : false,
					     mod->program_block,
					     mod->is_interface);
	    my_scope->set_line(get_file(), mod->get_file(),
	                       get_lineno(), mod->get_lineno());
	    my_scope->set_module_name(mod->mod_name());

	    instances[idx] = my_scope;

	      // Set time units and precision.
	    my_scope->time_unit(mod->time_unit);
	    my_scope->time_precision(mod->time_precision);
	    my_scope->time_from_timescale(mod->time_from_timescale);
	    des->set_precision(mod->time_precision);

	      // Look for module parameter replacements. The "replace" map
	      // maps parameter name to replacement expression that is
	      // passed. It is built up by the ordered overrides or named
	      // overrides.

	    Module::replace_t replace;

	      // Positional parameter overrides are matched to parameter
	      // names by using the param_names list of parameter
	      // names. This is an ordered list of names so the first name
	      // is parameter 0, the second parameter 1, and so on.

	    if (overrides_) {
		  assert(parms_ == 0);
		  list<perm_string>::const_iterator cur
			= mod->param_names.begin();
		  list<PExpr*>::const_iterator jdx = overrides_->begin();
		  for (;;) {
			if (jdx == overrides_->end())
			      break;
			if (cur == mod->param_names.end())
			      break;

		          // No expression means that the parameter is not
		          // replaced at all.
			if (*jdx)
			      replace[*cur] = *jdx;

			++ jdx;
			++ cur;
		  }
	    }

	      // Named parameter overrides carry a name with each override
	      // so the mapping into the replace list is much easier.
	    if (parms_) {
		  assert(overrides_ == 0);
		  for (unsigned jdx = 0 ;  jdx < nparms_ ;  jdx += 1) {
		          // No expression means that the parameter is not
		          // replaced.
			if (parms_[jdx].parm)
			      replace[parms_[jdx].name] = parms_[jdx].parm;
		  }

	    }

	      // This call actually arranges for the description of the
	      // module type to process this instance and handle parameters
	      // and sub-scopes that might occur. Parameters are also
	      // created in that scope, as they exist. (I'll override them
	      // later.)
	    mod->elaborate_scope(des, my_scope, replace);

      }

	/* Stash the instance array of scopes into the parent
	   scope. Later elaboration passes will use this vector to
	   further elaborate the array.

	   Note that the array is ordered from LSB to MSB. We will use
	   that fact in the main elaborate to connect things in the
	   correct order. */
      sc->instance_arrays[get_name()] = instances;
}

/*
 * The isn't really able to create new scopes, but it does create the
 * event name in the current scope, so can be done during the
 * elaborate_scope scan. Note that the name_ of the PEvent object has
 * no hierarchy, but neither does the NetEvent, until it is stored in
 * the NetScope object.
 */
void PEvent::elaborate_scope(Design*des, NetScope*scope) const
{
	// A named event can not have the same name as another scope object.
      const NetScope *child = scope->child(hname_t(name_));
      if (child) {
	    cerr << get_fileline() << ": error: named event and ";
	    child->print_type(cerr);
	    cerr << " in '" << scope->fullname()
	         << "' have the same name '" << name_ << "'." << endl;
	    des->errors += 1;
      }

	// A named event can not have the same name as a genvar.
      if (scope->find_genvar(name_)) {
	    cerr << get_fileline() << ": error: named event and "
	         << "genvar in '" << scope->fullname()
	         << "' have the same name '" << name_ << "'." << endl;
	    des->errors += 1;
      }

	// A named event can not have the same name as a parameter.
      const NetExpr *ex_msb, *ex_lsb;
      const NetExpr *parm = scope->get_parameter(des, name_, ex_msb, ex_lsb);
      if (parm) {
	    cerr << get_fileline() << ": error: named event and "
	         << "parameter in '" << scope->fullname()
	         << "' have the same name '" << name_ << "'." << endl;
	    des->errors += 1;
      }

      NetEvent*ev = new NetEvent(name_);
      ev->set_line(*this);
      scope->add_event(ev);
}

void PFunction::elaborate_scope(Design*des, NetScope*scope) const
{
      assert(scope->type() == NetScope::FUNC);

        // Save a reference to the pform representation of the function
        // in case we need to perform early elaboration.
      scope->set_func_pform(this);

        // Assume the function is a constant function until we
        // find otherwise.
      scope->is_const_func(true);

	// Scan the parameters in the function, and store the information
        // needed to evaluate the parameter expressions.

      collect_scope_parameters_(des, scope, parameters);

      collect_scope_localparams_(des, scope, localparams);

	// Scan through all the named events in this scope.
      elaborate_scope_events_(des, scope, events);

      if (statement_)
	    statement_->elaborate_scope(des, scope);
}

void PTask::elaborate_scope(Design*des, NetScope*scope) const
{
      assert(scope->type() == NetScope::TASK);

	// Scan the parameters in the task, and store the information
        // needed to evaluate the parameter expressions.

      collect_scope_parameters_(des, scope, parameters);

      collect_scope_localparams_(des, scope, localparams);

	// Scan through all the named events in this scope.
      elaborate_scope_events_(des, scope, events);

      if (statement_)
	    statement_->elaborate_scope(des, scope);
}


/*
 * The base statement does not have sub-statements and does not
 * introduce any scope, so this is a no-op.
 */
void Statement::elaborate_scope(Design*, NetScope*) const
{
}

/*
 * When I get a behavioral block, check to see if it has a name. If it
 * does, then create a new scope for the statements within it,
 * otherwise use the current scope. Use the selected scope to scan the
 * statements that I contain.
 */
void PBlock::elaborate_scope(Design*des, NetScope*scope) const
{
      NetScope*my_scope = scope;

      if (pscope_name() != 0) {
	    hname_t use_name(pscope_name());
	      // A named block can not have the same name as another scope
	      // object.
	    const NetScope *child = scope->child(use_name);
	    if (child) {
		  cerr << get_fileline() << ": error: named block and ";
		  child->print_type(cerr);
		  cerr << " in '" << scope->fullname()
		       << "' have the same name '" << use_name << "'." << endl;
		  des->errors += 1;
		  return;
	    }

	      // A named block can not have the same name as a genvar.
	    if (scope->find_genvar(pscope_name())) {
		  cerr << get_fileline() << ": error: named block and "
		          "genvar in '" << scope->fullname()
		       << "' have the same name '" << use_name << "'." << endl;
		  des->errors += 1;
	    }

	      // A named block can not have the same name as a parameter.
	    const NetExpr *ex_msb, *ex_lsb;
	    const NetExpr *parm = scope->get_parameter(des, pscope_name(),
                                                       ex_msb, ex_lsb);
	    if (parm) {
		  cerr << get_fileline() << ": error: named block and "
		          "parameter in '" << scope->fullname()
		       << "' have the same name '" << use_name << "'." << endl;
		  des->errors += 1;
	    }

	    if (debug_scopes)
		  cerr << get_fileline() << ": debug: "
		       << "Elaborate block scope " << use_name
		       << " within " << scope_path(scope) << endl;

	      // The scope type is begin-end or fork-join. The
	      // sub-types of fork-join are not interesting to the scope.
	    my_scope = new NetScope(scope, use_name, bl_type_!=BL_SEQ
				    ? NetScope::FORK_JOIN
				    : NetScope::BEGIN_END);
	    my_scope->set_line(get_file(), get_lineno());
            my_scope->is_auto(scope->is_auto());

	      // Scan the parameters in the scope, and store the information
	      // needed to evaluate the parameter expressions.

            collect_scope_parameters_(des, my_scope, parameters);

            collect_scope_localparams_(des, my_scope, localparams);

              // Scan through all the named events in this scope.
            elaborate_scope_events_(des, my_scope, events);
      }

      for (unsigned idx = 0 ;  idx < list_.size() ;  idx += 1)
	    list_[idx] -> elaborate_scope(des, my_scope);
}

/*
 * The case statement itself does not introduce scope, but contains
 * other statements that may be named blocks. So scan the case items
 * with the elaborate_scope method.
 */
void PCase::elaborate_scope(Design*des, NetScope*scope) const
{
      assert(items_);
      for (unsigned idx = 0 ;  idx < (*items_).count() ;  idx += 1) {
	    assert( (*items_)[idx] );

	    if (Statement*sp = (*items_)[idx]->stat)
		  sp -> elaborate_scope(des, scope);
      }
}

/*
 * The conditional statement (if-else) does not introduce scope, but
 * the statements of the clauses may, so elaborate_scope the contained
 * statements.
 */
void PCondit::elaborate_scope(Design*des, NetScope*scope) const
{
      if (if_)
	    if_ -> elaborate_scope(des, scope);

      if (else_)
	    else_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PDelayStatement::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PDoWhile::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PEventStatement::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * The standard says that we create an implicit scope for foreach
 * loops, but that is just to hold the index variables, and we'll
 * handle them by creating unique names. So just jump into the
 * contained statement for scope elaboration.
 */
void PForeach::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PForever::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PForStatement::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PRepeat::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}

/*
 * Statements that contain a further statement but do not
 * intrinsically add a scope need to elaborate_scope the contained
 * statement.
 */
void PWhile::elaborate_scope(Design*des, NetScope*scope) const
{
      if (statement_)
	    statement_ -> elaborate_scope(des, scope);
}
