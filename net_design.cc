/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
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

# include  <iostream>
# include  <set>
# include  <cstdlib>

/*
 * This source file contains all the implementations of the Design
 * class declared in netlist.h.
 */

# include  "netlist.h"
# include  "netscalar.h"
# include  "netvector.h"
# include  "util.h"
# include  "compiler.h"
# include  "netmisc.h"
# include  "PExpr.h"
# include  "PTask.h"
# include  <sstream>
# include  "ivl_assert.h"

using namespace std;

Design:: Design()
    : errors(0), nodes_(0), procs_(0), aprocs_(0)
{
      branches_ = 0;
      procs_idx_ = 0;
      des_precision_ = 0;
      nodes_functor_cur_ = 0;
      nodes_functor_nxt_ = 0;
      des_delay_sel_ = Design::TYP;
}

Design::~Design()
{
}

void Design::set_precision(int val)
{
      if (val < des_precision_)
	    des_precision_ = val;
}

int Design::get_precision() const
{
      return des_precision_;
}

void Design::set_delay_sel(delay_sel_t sel)
{
      des_delay_sel_ = sel;
}

const char* Design::get_delay_sel() const
{
      switch (des_delay_sel_) {
	case Design::MIN:
	    return "MINIMUM";
	    break;
	case Design::TYP:
	    return "TYPICAL";
	    break;
	case Design::MAX:
	    return "MAXIMUM";
	    break;
	default:
	    assert(0);
	    return "TYPICAL";
      }
}


uint64_t Design::scale_to_precision(uint64_t val,
				    const NetScope*scope) const
{
      int units = scope->time_unit();
      assert( units >= des_precision_ );

      while (units > des_precision_) {
	    units -= 1;
	    val *= 10;
      }

      return val;
}

NetScope* Design::make_root_scope(perm_string root, NetScope*unit_scope,
				  bool program_block, bool is_interface)
{
      NetScope *root_scope_;
      root_scope_ = new NetScope(0, hname_t(root), NetScope::MODULE, unit_scope,
				 false, program_block, is_interface);
	/* This relies on the fact that the basename return value is
	   permallocated. */
      root_scope_->set_module_name(root_scope_->basename());
      root_scopes_.push_back(root_scope_);
      return root_scope_;
}

NetScope* Design::find_root_scope()
{
      assert(root_scopes_.front());
      return root_scopes_.front();
}

list<NetScope*> Design::find_root_scopes() const
{
      return root_scopes_;
}

NetScope* Design::make_package_scope(perm_string name, NetScope*unit_scope,
				     bool is_unit)
{
      NetScope*scope;

      scope = new NetScope(0, hname_t(name), NetScope::PACKAGE, unit_scope,
			   false, false, false, is_unit);
      scope->set_module_name(scope->basename());
      packages_[name] = scope;
      return scope;
}

NetScope* Design::find_package(perm_string name) const
{
      map<perm_string,NetScope*>::const_iterator cur = packages_.find(name);
      if (cur == packages_.end())
	    return 0;

      return cur->second;
}

list<NetScope*> Design::find_package_scopes() const
{
      list<NetScope*>res;
      for (map<perm_string,NetScope*>::const_iterator cur = packages_.begin()
		 ; cur != packages_.end() ; ++cur) {
	    res.push_back (cur->second);
      }

      return res;
}

/*
 * This method locates a scope in the design, given its rooted
 * hierarchical name. Each component of the key is used to scan one
 * more step down the tree until the name runs out or the search
 * fails.
 */
NetScope* Design::find_scope(const std::list<hname_t>&path) const
{
      if (path.empty())
	    return 0;

      for (list<NetScope*>::const_iterator scope = root_scopes_.begin()
		 ; scope != root_scopes_.end(); ++ scope ) {

	    NetScope*cur = *scope;
	    if (path.front() != cur->fullname())
		  continue;

	    std::list<hname_t> tmp = path;
	    tmp.pop_front();

	    while (cur) {
		  if (tmp.empty()) return cur;

		  cur = cur->child( tmp.front() );

		  tmp.pop_front();
	    }
      }

      return 0;
}

/*
 * This method locates a scope in the design, given its rooted
 * hierarchical name. Each component of the key is used to scan one
 * more step down the tree until the name runs out or the search
 * fails.
 */
NetScope* Design::find_scope(const hname_t&path) const
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin()
		 ; scope != root_scopes_.end(); ++ scope ) {

	    NetScope*cur = *scope;
	    if (path.peek_name() == cur->basename())
		  return cur;

      }

      return 0;
}

static bool is_design_unit(NetScope*scope)
{
      return (scope->type() == NetScope::MODULE && !scope->nested_module())
	  || (scope->type() == NetScope::PACKAGE);
}

static bool is_subroutine(NetScope::TYPE type)
{
      return type == NetScope::TASK || type == NetScope::FUNC;
}

/*
 * This method locates a scope within another scope, given its relative
 * hierarchical name. Each component of the key is used to scan one
 * more step down the tree until the name runs out or the search
 * fails.
 */
NetScope* Design::find_scope_(NetScope*scope, const std::list<hname_t>&path,
                              NetScope::TYPE type) const
{
      std::list<hname_t> tmp = path;

      do {
	    hname_t key = tmp.front();
	      /* If we are looking for a module or we are not
	       * looking at the last path component check for
	       * a name match (second line). */
	    if (scope->type() == NetScope::MODULE
		&& (type == NetScope::MODULE || tmp.size() > 1)
		&& scope->module_name()==key.peek_name()) {

		    /* Up references may match module name */

	    } else {
		  NetScope*found_scope = scope->child(key);
		  if (found_scope == 0) {
			found_scope = scope->find_import(this, key.peek_name());
			if (found_scope)
			      found_scope = found_scope->child(key);
		  }
		  scope = found_scope;
		  if (scope == 0) break;
	    }
	    tmp.pop_front();
      } while (! tmp.empty());

      return scope;
}

/*
 * This is a relative lookup of a scope by name. The starting point is
 * the scope parameter within which I start looking for the scope. If
 * I do not find the scope within the passed scope, start looking in
 * parent scopes until I find it, or I run out of parent scopes.
 */
NetScope* Design::find_scope(NetScope*scope, const std::list<hname_t>&path,
                             NetScope::TYPE type) const
{
      assert(scope);
      if (path.empty())
	    return scope;

	// Record the compilation unit scope for use later.
      NetScope*unit_scope = scope->unit();

	// First search upwards through the hierarchy.
      while (scope) {
	    NetScope*found_scope = find_scope_(scope, path, type);
	    if (found_scope)
		  return found_scope;

	      // Avoid searching the unit scope twice.
	    if (scope == unit_scope)
		  unit_scope = 0;

	      // Special case - see IEEE 1800-2012 section 23.8.1.
	    if (unit_scope && is_design_unit(scope) && is_subroutine(type)) {
		  found_scope = find_scope_(unit_scope, path, type);
		  if (found_scope)
			  return found_scope;

		  unit_scope = 0;
	    }

	    scope = scope->parent();
      }

	// If we haven't already done so, search the compilation unit scope.
      if (unit_scope) {
	    NetScope*found_scope = find_scope_(unit_scope, path, type);
	    if (found_scope)
		  return found_scope;
      }

	// Last chance. Look for the name starting at the root.
      return find_scope(path);
}

/*
 * This method locates a scope within another scope, given its relative
 * hierarchical name. Each component of the key is used to scan one
 * more step down the tree until the name runs out or the search
 * fails.
 */
NetScope* Design::find_scope_(NetScope*scope, const hname_t&path,
                              NetScope::TYPE type) const
{
	/* If we are looking for a module or we are not
	 * looking at the last path component check for
	 * a name match (second line). */
      if ((scope->type() == NetScope::MODULE) && (type == NetScope::MODULE)
	  && (scope->module_name() == path.peek_name())) {
	      /* Up references may match module name */
	    return scope;
      }
      NetScope*found_scope = scope->child(path);
      if (found_scope == 0) {
	    found_scope = scope->find_import(this, path.peek_name());
	    if (found_scope)
		  found_scope = found_scope->child(path);
      }
      return found_scope;
}

/*
 * This is a relative lookup of a scope by name. The starting point is
 * the scope parameter within which I start looking for the scope. If
 * I do not find the scope within the passed scope, start looking in
 * parent scopes until I find it, or I run out of parent scopes.
 */
NetScope* Design::find_scope(NetScope*scope, const hname_t&path,
                             NetScope::TYPE type) const
{
      assert(scope);

	// Record the compilation unit scope for use later.
      NetScope*unit_scope = scope->unit();

	// First search upwards through the hierarchy.
      while (scope) {
	    NetScope*found_scope = find_scope_(scope, path, type);
	    if (found_scope)
		  return found_scope;

	      // Avoid searching the unit scope twice.
	    if (scope == unit_scope)
		  unit_scope = 0;

	      // Special case - see IEEE 1800-2012 section 23.8.1.
	    if (unit_scope && is_design_unit(scope) && is_subroutine(type)) {
		  found_scope = find_scope_(unit_scope, path, type);
		  if (found_scope)
			  return found_scope;

		  unit_scope = 0;
	    }

	    scope = scope->parent();
      }

	// If we haven't already done so, search the compilation unit scope.
      if (unit_scope) {
	    NetScope*found_scope = find_scope_(unit_scope, path, type);
	    if (found_scope)
		  return found_scope;
      }

	// Last chance. Look for the name starting at the root.
      list<hname_t>path_list;
      path_list.push_back(path);
      return find_scope(path_list);
}

/*
 * This method runs through the scope, noticing the defparam
 * statements that were collected during the elaborate_scope pass and
 * applying them to the target parameters. The implementation actually
 * works by using a specialized method from the NetScope class that
 * does all the work for me.
 */
void Design::run_defparams()
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); ++ scope )
	    (*scope)->run_defparams(this);
}

void NetScope::run_defparams(Design*des)
{
      for (map<hname_t,NetScope*>::const_iterator cur = children_.begin()
		 ; cur != children_.end() ; ++ cur )
	    cur->second->run_defparams(des);

      while (! defparams.empty()) {
	    pair<pform_name_t,PExpr*> pp = defparams.front();
	    defparams.pop_front();

	    pform_name_t path = pp.first;
	    PExpr*val = pp.second;

	    perm_string perm_name = peek_tail_name(path);
	    path.pop_back();

	    list<hname_t> eval_path = eval_scope_path(des, this, path);

	      /* If there is no path on the name, then the targ_scope
		 is the current scope. */
	    NetScope*targ_scope = des->find_scope(this, eval_path);
	    if (targ_scope == 0) {

		    // Push the defparam onto a list for retry
		    // later. It is possible for the scope lookup to
		    // fail if the scope being defparam'd into is
		    // generated by an index array for generate.
		  eval_path.push_back(hname_t(perm_name));
		  defparams_later.push_back(make_pair(eval_path,val));
		  continue;
	    }

	    targ_scope->replace_parameter(des, perm_name, val, this, true);
      }

	// If some of the defparams didn't find a scope in the name,
	// then try again later. It may be that the target scope is
	// created later by generate scheme or instance array.
      if (! defparams_later.empty())
	    des->defparams_later.insert(this);
}

void NetScope::run_defparams_later(Design*des)
{
      set<NetScope*> target_scopes;
      list<pair<list<hname_t>,PExpr*> > defparams_even_later;

      while (! defparams_later.empty()) {
	    pair<list<hname_t>,PExpr*> cur = defparams_later.front();
	    defparams_later.pop_front();

	    list<hname_t>eval_path = cur.first;
	    perm_string name = eval_path.back().peek_name();
	    eval_path.pop_back();

	    PExpr*val = cur.second;

	    NetScope*targ_scope = des->find_scope(this, eval_path);
	    if (targ_scope == 0) {
		    // If a scope in the target path is not found,
		    // then push this defparam for handling even
		    // later. Maybe a later generate scheme or
		    // instance array will create the scope.
		  defparams_even_later.push_back(cur);
		  continue;
	    }

	    targ_scope->replace_parameter(des, name, val, this, true);

	      // We'll need to re-evaluate parameters in this scope
	    target_scopes.insert(targ_scope);
      }

	// The scopes that this defparam set touched will be
	// re-evaluated later it a top_defparams work item. So do not
	// do the evaluation now.

	// If there are some scopes that still have missing scopes,
	// then save them back into the defparams_later list for a
	// later pass.
      defparams_later = defparams_even_later;
      if (! defparams_later.empty())
	    des->defparams_later.insert(this);
}

void Design::evaluate_parameters()
{
      for (map<perm_string,NetScope*>::const_iterator cur = packages_.begin()
		 ; cur != packages_.end() ; ++ cur) {
	    cur->second->evaluate_parameters(this);
      }

      for (list<NetScope*>::const_iterator scope = root_scopes_.begin()
		 ; scope != root_scopes_.end() ; ++ scope ) {
	    (*scope)->evaluate_parameters(this);
      }
}

void NetScope::evaluate_parameter_logic_(Design*des, param_ref_t cur)
{
	/* Evaluate the parameter expression. */
      PExpr*val_expr = (*cur).second.val_expr;
      NetScope*val_scope = (*cur).second.val_scope;

      // The param_type may be nil if the parameter has no declared type. In
      // this case, we'll try to take our type from the r-value.
      ivl_type_t param_type = cur->second.ivl_type;

      // Most parameter declarations are packed vectors, of the form:
      //   parameter [H:L] foo == bar;
      // so get the netvector_t. Note that this may also be the special
      // case of a netvector_t with no dimensions, that exists only to carry
      // signed-ness, e.g.:
      //  parameter signed foo = bar;
      // These will be marked as scalar, but also have the implict flag set.
      const netvector_t* param_vect = dynamic_cast<const netvector_t*> (param_type);

      if (debug_elaborate) {
	    cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		 << "parameter=" << cur->first << endl;
	    if (param_type)
		  cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		       << "param_type=" << *param_type << endl;
	    else
		  cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		       << "param_type=(nil)" << endl;
	    cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		 << "val_expr=" << *val_expr << endl;
      }

      ivl_variable_type_t use_type;
      int lv_width = -2;
      if (param_type) {
	    use_type = param_type->base_type();
	    // Is this an implicit netvector_t with no dimenions?
	    if (param_vect && param_vect->get_implicit() &&
	        param_vect->get_scalar())
		  lv_width = -2;
	    else if (param_type->packed())
		  lv_width = param_type->packed_width();
      } else {
	    use_type = val_expr->expr_type();
      }

      if (debug_elaborate) {
	    cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		 << "use_type = " << use_type << endl;
      }

      NetExpr *expr;

      // Handle assignment patterns as a special case as they need the type to
      // be evaluated correctly.
      if (param_type && dynamic_cast<PEAssignPattern*>(val_expr)) {
	    expr = elab_and_eval(des, val_scope, val_expr, param_type, true);
      } else {
	    expr = elab_and_eval(des, val_scope, val_expr, lv_width, true,
				 cur->second.is_annotatable, use_type);
      }
      if (! expr)
            return;

      // Make sure to carry the signed-ness from a vector type.
      if (param_vect)
	    expr->cast_signed(param_vect->get_signed());

      if (debug_elaborate) {
	    cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		 << "expr = " << *expr << endl;
	    cerr << val_expr->get_fileline() << ": " << __func__ << ": "
		 << "expr type = " << expr->expr_type() << endl;
      }

      switch (expr->expr_type()) {
	  case IVL_VT_REAL:
	    if (! dynamic_cast<const NetECReal*>(expr)) {
		  cerr << expr->get_fileline()
		       << ": error: Unable to evaluate real parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  return;
	    }

	    // If the parameter has no type, then infer its type from the
	    // r-value expression.
	    if (param_type==0) {
		  param_type = &netreal_t::type_real;
		  cur->second.ivl_type = param_type;
	    }
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    if (! dynamic_cast<const NetEConst*>(expr)) {
		  cerr << expr->get_fileline()
		       << ": error: Unable to evaluate parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  return;
	    }

	    // If the parameter has no type, then infer its type from the
	    // r-value expression.
	    if (param_type==0) {
		  param_type = new netvector_t(expr->expr_type(), expr->expr_width()-1,
					       0, expr->has_sign());
		  cur->second.ivl_type = param_type;
	    }

	    if (param_type->base_type() != IVL_VT_NO_TYPE)
		  expr->cast_signed(param_type->get_signed());

	    if (!expr->has_width()) {
		  expr = pad_to_width(expr, integer_width, *expr);
	    } else if (param_type->slice_dimensions().size()==0 && !expr->has_width()) {
		  expr = pad_to_width(expr, integer_width, *expr);
	    }
	    break;

	  default:
	    cerr << expr->get_fileline()
		 << ": internal error: "
		 << "Unhandled expression type "
		 << expr->expr_type() << "?" << endl;
	    cerr << expr->get_fileline()
		 << ":               : "
		 << "param_type: " << *param_type << endl;
	    des->errors += 1;
	    return;
      }

      // By the time we're done with the above, we should certainly know the
      // type of the parameter.
      ivl_assert(*expr, cur->second.ivl_type);

      cur->second.val = expr;

	// If there are no value ranges to test the value against,
	// then we are done.
      if ((*cur).second.range == 0)
	    return;

      NetEConst*val = dynamic_cast<NetEConst*>((*cur).second.val);
      ivl_assert(*expr, val);

      verinum value = val->value();

      bool from_flag = (*cur).second.range == 0? true : false;
      for (range_t*value_range = (*cur).second.range
		 ; value_range ; value_range = value_range->next) {

	      // If we already know that the value is
	      // within a "from" range, then do not test
	      // any more of the from ranges.
	    if (from_flag && value_range->exclude_flag==false)
		  continue;

	    if (value_range->low_expr) {
		  NetEConst*tmp = dynamic_cast<NetEConst*>(value_range->low_expr);
		  ivl_assert(*value_range->low_expr, tmp);
		  if (value_range->low_open_flag && value <= tmp->value())
			continue;
		  else if (value < tmp->value())
			continue;
	    }

	    if (value_range->high_expr) {
		  NetEConst*tmp = dynamic_cast<NetEConst*>(value_range->high_expr);
		  ivl_assert(*value_range->high_expr, tmp);
		  if (value_range->high_open_flag && value >= tmp->value())
			continue;
		  else if (value > tmp->value())
			continue;
	    }

	      // Within the range. If this is a "from"
	      // range, then set the from_flag and continue.
	    if (value_range->exclude_flag == false) {
		  from_flag = true;
		  continue;
	    }

	      // OH NO! In an excluded range. signal an error.
	    from_flag = false;
	    break;
      }

	// If we found no from range that contains the
	// value, then report an error.
      if (! from_flag) {
	    cerr << val->get_fileline() << ": error: "
		 << "Parameter value " << value
		 << " is out of range for parameter " << (*cur).first
		 << "." << endl;
	    des->errors += 1;
      }
}

void NetScope::evaluate_parameter_real_(Design*des, param_ref_t cur)
{
      PExpr*val_expr = (*cur).second.val_expr;
      NetScope*val_scope = (*cur).second.val_scope;
      ivl_type_t param_type = cur->second.ivl_type;

      ivl_assert(*val_expr, param_type);
      NetExpr*expr = elab_and_eval(des, val_scope, val_expr, -1, true,
                                   cur->second.is_annotatable,
                                   param_type->base_type());
      if (! expr)
            return;

      NetECReal*res = 0;

      switch (expr->expr_type()) {
	  case IVL_VT_REAL:
	    if (NetECReal*tmp = dynamic_cast<NetECReal*>(expr)) {
		  res = tmp;
	    } else {
		  cerr << expr->get_fileline()
		       << ": error: "
		       << "Unable to evaluate real parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  return;
	    }
	    break;

	  default:
	    cerr << expr->get_fileline()
		 << ": internal error: "
		 << "Failed to cast expression?" << endl;
	    des->errors += 1;
	    return;
	    break;
      }

      (*cur).second.val = res;
      double value = res->value().as_double();

      bool from_flag = (*cur).second.range == 0? true : false;
      for (range_t*value_range = (*cur).second.range
		 ; value_range ; value_range = value_range->next) {

	    if (from_flag && value_range->exclude_flag==false)
		  continue;

	    if (value_range->low_expr) {
		  double tmp;
		  bool flag = eval_as_double(tmp, value_range->low_expr);
		  ivl_assert(*value_range->low_expr, flag);
		  if (value_range->low_open_flag && value <= tmp)
			continue;
		  else if (value < tmp)
			continue;
	    }

	    if (value_range->high_expr) {
		  double tmp;
		  bool flag = eval_as_double(tmp, value_range->high_expr);
		  ivl_assert(*value_range->high_expr, flag);
		  if (value_range->high_open_flag && value >= tmp)
			continue;
		  else if (value > tmp)
			continue;
	    }

	    if (value_range->exclude_flag == false) {
		  from_flag = true;
		  continue;
	    }

	      // All the above tests failed, so we must have tripped
	      // an exclude rule.
	    from_flag = false;
	    break;
      }

      if (! from_flag) {
	    cerr << res->get_fileline() << ": error: "
		 << "Parameter value " << value
		 << " is out of range for real parameter " << (*cur).first
		 << "." << endl;
	    des->errors += 1;
      }
}

/*
 * Evaluate a parameter that is forced to type string. This comes to pass when
 * the input is something like this:
 *
 *     parameter string foo = <expr>;
 *
 * The param_type should be netstring_t, the val_expr is the pform of the
 * input <expr>, and we try to elaborate/evaluate down to a IVL_VT_STRING
 * expression.
 */
void NetScope::evaluate_parameter_string_(Design*des, param_ref_t cur)
{
      PExpr*val_expr = (*cur).second.val_expr;
      NetScope*val_scope = (*cur).second.val_scope;
      ivl_type_t param_type = cur->second.ivl_type;

      ivl_assert(cur->second, val_expr);
      ivl_assert(cur->second, param_type);

      NetExpr*expr = elab_and_eval(des, val_scope, val_expr, param_type, true);
      if (! expr)
	    return;

      cur->second.val = expr;

      if (debug_elaborate) {
	    cerr << cur->second.get_fileline() << ": " << __func__ << ": "
		 << "Parameter type: " << *param_type << endl;
	    cerr << cur->second.get_fileline() << ": " << __func__ << ": "
		 << "Parameter value: " << *val_expr << endl;
	    cerr << cur->second.get_fileline() << ": " << __func__ << ": "
		 << "Elaborated value: " << *expr << endl;
      }
}

void NetScope::evaluate_type_parameter_(Design *des, param_ref_t cur)
{
      const PETypename *type_expr = dynamic_cast<const PETypename*>(cur->second.val_expr);
      if (!type_expr) {
	    cerr << this->get_fileline() << ": error: "
		 << "Type parameter `" << cur->first << "` value `"
	         << *cur->second.val_expr << "` is not a type."
		 << endl;
	    des->errors++;

	    // Recover
	    cur->second.ivl_type = netvector_t::integer_type();
	    return;
      }

      data_type_t *ptype = type_expr->get_type();
      NetScope *type_scope = cur->second.val_scope;
      cur->second.ivl_type = ptype->elaborate_type(des, type_scope);
}

void NetScope::evaluate_parameter_(Design*des, param_ref_t cur)
{

      // If the parameter has already been evaluated, quietly return.
      if (cur->second.val || cur->second.ivl_type)
            return;

      if (cur->second.val_expr == 0) {
	    cerr << this->get_fileline() << ": error: "
	         << "Missing value for parameter `"
	         << cur->first << "`." << endl;
	    des->errors += 1;

	    cur->second.val = new NetEConst(verinum(verinum::Vx));
	    return;
      }

      if (cur->second.type_flag) {
	    evaluate_type_parameter_(des, cur);
	    return;
      }

      ivl_type_t param_type = cur->second.ivl_type;

      // If the parameter type is present, then elaborate it now. Elaborate
      // the type in the current scope, and not the scope of the expression.
      if (cur->second.val_type) {
	    param_type = cur->second.val_type->elaborate_type(des, this);
	    cur->second.ivl_type = param_type;
	    cur->second.val_type = 0;
      }

      // Guess the varaiable type of the parameter. If the parameter has no
      // given type, then guess the type from the expression and use that to
      // evaluate (this is currently handled in evaluate_parameter_logic_()).
      ivl_variable_type_t use_type;
      if (param_type)
	    use_type = param_type->base_type();
      else
	    use_type = IVL_VT_NO_TYPE;

      if (cur->second.solving) {
            cerr << cur->second.get_fileline() << ": error: "
	         << "Recursive parameter reference found involving "
                 << cur->first << "." << endl;
	    des->errors += 1;

      } else {
            cur->second.solving = true;
            switch (use_type) {
                case IVL_VT_NO_TYPE:
                case IVL_VT_BOOL:
                case IVL_VT_LOGIC:
                  evaluate_parameter_logic_(des, cur);
                  break;

                case IVL_VT_REAL:
                  evaluate_parameter_real_(des, cur);
                  break;

		case IVL_VT_STRING:
		  evaluate_parameter_string_(des, cur);
		  break;

                default:
                  cerr << cur->second.get_fileline() << ": internal error: "
                       << "Unexpected parameter type " << use_type
                       << "." << endl;
                  cerr << cur->second.get_fileline() << ":               : "
                       << "Parameter name: " << cur->first << endl;
		  if (param_type)
			cerr << cur->second.get_fileline() << ":               : "
			     << "Parameter ivl_type: " << *param_type << endl;
                  cerr << cur->second.get_fileline() << ":               : "
                       << "Expression is: " << *cur->second.val_expr << endl;
                  ivl_assert(cur->second, 0);
                  break;
            }
            cur->second.solving = false;
      }

        // If we have failed to evaluate the expression, create a dummy
        // value. This prevents spurious error messages being output.
      if (cur->second.val == 0) {
            verinum val(verinum::Vx);
            cur->second.val = new NetEConst(val);
      }

        // Flag that the expression has been evaluated.
      cur->second.val_expr = 0;
}

void NetScope::evaluate_parameters(Design*des)
{
      for (map<hname_t,NetScope*>::const_iterator cur = children_.begin()
		 ; cur != children_.end() ; ++ cur )
	    cur->second->evaluate_parameters(des);

      if (debug_scopes)
	    cerr << "debug: "
		 << "Evaluating parameters in " << scope_path(this) << endl;

      for (param_ref_t cur = parameters.begin()
		 ; cur != parameters.end() ;  ++ cur) {

            evaluate_parameter_(des, cur);
      }
}

void Design::residual_defparams()
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); ++ scope )
	    (*scope)->residual_defparams(this);
}

void NetScope::residual_defparams(Design*des)
{
	// Clean out the list of defparams that never managed to match
	// a scope. Print a warning for each.
      while (! defparams_later.empty()) {
	    pair<list<hname_t>,PExpr*> cur = defparams_later.front();
	    defparams_later.pop_front();

	    cerr << cur.second->get_fileline() << ": warning: "
		 << "Scope of " << cur.first << " not found." << endl;
      }

      for (map<hname_t,NetScope*>::const_iterator cur = children_.begin()
		 ; cur != children_.end() ; ++ cur )
	    cur->second->residual_defparams(des);
}

const char* Design::get_flag(const string&key) const
{
      map<string,const char*>::const_iterator tmp = flags_.find(key);
      if (tmp == flags_.end())
	    return "";
      else
	    return (*tmp).second;
}

/*
 * This method looks for a signal (reg, wire, whatever) starting at
 * the specified scope. If the name is hierarchical, it is split into
 * scope and name and the scope used to find the proper starting point
 * for the real search.
 *
 * It is the job of this function to properly implement Verilog scope
 * rules as signals are concerned.
 */
NetNet* Design::find_signal(NetScope*scope, pform_name_t path)
{
      assert(scope);

      perm_string key = peek_tail_name(path);
      path.pop_back();
      if (! path.empty()) {
	    list<hname_t> eval_path = eval_scope_path(this, scope, path);
	    scope = find_scope(scope, eval_path);
      }

      while (scope) {
	    if (NetNet*net = scope->find_signal(key))
		  return net;

	    if (NetScope*import_scope = scope->find_import(this, key)) {
		  scope = import_scope;
		  continue;
	    }

	    if (scope->type() == NetScope::MODULE)
		  break;

	    scope = scope->parent();
      }

      return 0;
}

NetFuncDef* Design::find_function(NetScope*scope, const pform_name_t&name)
{
      assert(scope);

      std::list<hname_t> eval_path = eval_scope_path(this, scope, name);
      NetScope*func = find_scope(scope, eval_path, NetScope::FUNC);
      if (func && (func->type() == NetScope::FUNC)) {
              // If a function is used in a parameter definition or in
              // a signal declaration, it is possible to get here before
              // the function's signals have been elaborated. If this is
              // the case, elaborate them now.
            if (func->elab_stage() < 2) {
		  func->need_const_func(true);
                  const PFunction*pfunc = func->func_pform();
                  assert(pfunc);
                  pfunc->elaborate_sig(this, func);
            }
	    return func->func_def();
      }
      return 0;
}

NetScope* Design::find_task(NetScope*scope, const pform_name_t&name)
{
      std::list<hname_t> eval_path = eval_scope_path(this, scope, name);
      NetScope*task = find_scope(scope, eval_path, NetScope::TASK);
      if (task && (task->type() == NetScope::TASK))
	    return task;

      return 0;
}

void Design::add_node(NetNode*net)
{
      assert(net->design_ == 0);
      if (nodes_ == 0) {
	    net->node_next_ = net;
	    net->node_prev_ = net;
      } else {
	    net->node_next_ = nodes_->node_next_;
	    net->node_prev_ = nodes_;
	    net->node_next_->node_prev_ = net;
	    net->node_prev_->node_next_ = net;
      }
      nodes_ = net;
      net->design_ = this;
}

void Design::del_node(NetNode*net)
{
      assert(net != 0);
      assert(net->design_ == this);

	/* Interact with the Design::functor method by manipulating the
	   cur and nxt pointers that it is using. */
      if (net == nodes_functor_nxt_)
	    nodes_functor_nxt_ = nodes_functor_nxt_->node_next_;
      if (net == nodes_functor_nxt_)
	    nodes_functor_nxt_ = 0;

      if (net == nodes_functor_cur_)
	    nodes_functor_cur_ = 0;

	/* Now perform the actual delete. */
      if (nodes_ == net)
	    nodes_ = net->node_prev_;

      if (nodes_ == net) {
	    nodes_ = 0;
      } else {
	    net->node_next_->node_prev_ = net->node_prev_;
	    net->node_prev_->node_next_ = net->node_next_;
      }

      net->design_ = 0;
}

void Design::add_branch(NetBranch*bra)
{
      bra->next_ = branches_;
      branches_ = bra;
}

void Design::add_process(NetProcTop*pro)
{
      pro->next_ = procs_;
      procs_ = pro;
}

void Design::add_process(NetAnalogTop*pro)
{
      pro->next_ = aprocs_;
      aprocs_ = pro;
}
void Design::delete_process(NetProcTop*top)
{
      assert(top);
      if (procs_ == top) {
	    procs_ = top->next_;

      } else {
	    NetProcTop*cur = procs_;
	    while (cur->next_ != top) {
		  assert(cur->next_);
		  cur = cur->next_;
	    }

	    cur->next_ = top->next_;
      }

      if (procs_idx_ == top)
	    procs_idx_ = top->next_;

      delete top;
}

void Design::join_islands(void)
{
      if (nodes_ == 0)
	    return;

      NetNode*cur = nodes_->node_next_;
      do {
	    join_island(cur);
	    cur = cur->node_next_;
      } while (cur != nodes_->node_next_);
}
