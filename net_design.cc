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
# include  <set>
# include  <cstdlib>

/*
 * This source file contains all the implementations of the Design
 * class declared in netlist.h.
 */

# include  "netlist.h"
# include  "util.h"
# include  "compiler.h"
# include  "netmisc.h"
# include  <sstream>
# include  "ivl_assert.h"

Design:: Design()
    : errors(0), nodes_(0), procs_(0), aprocs_(0)
{
      branches_ = 0;
      procs_idx_ = 0;
      des_precision_ = 0;
      nodes_functor_cur_ = 0;
      nodes_functor_nxt_ = 0;
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

NetScope* Design::make_root_scope(perm_string root)
{
      NetScope *root_scope_;
      root_scope_ = new NetScope(0, hname_t(root), NetScope::MODULE);
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

list<NetScope*> Design::find_root_scopes()
{
      return root_scopes_;
}

const list<NetScope*> Design::find_root_scopes() const
{
      return root_scopes_;
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
		 ; scope != root_scopes_.end(); scope++) {

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

      for ( ; scope ;  scope = scope->parent()) {

	    std::list<hname_t> tmp = path;

	    NetScope*cur = scope;
	    do {
		  hname_t key = tmp.front();
		    /* If we are looking for a module or we are not
		     * looking at the last path component check for
		     * a name match (second line). */
		  if (cur->type() == NetScope::MODULE
		      && (type == NetScope::MODULE || tmp.size() > 1)
		      && cur->module_name()==key.peek_name()) {

			  /* Up references may match module name */

		  } else {
			cur = cur->child( key );
			if (cur == 0) break;
		  }
		  tmp.pop_front();
	    } while (!tmp.empty());

	    if (cur) return cur;
      }

	// Last chance. Look for the name starting at the root.
      return find_scope(path);
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
	   scope != root_scopes_.end(); scope++)
	    (*scope)->run_defparams(this);
}

void NetScope::run_defparams(Design*des)
{
      { NetScope*cur = sub_;
        while (cur) {
	      cur->run_defparams(des);
	      cur = cur->sib_;
	}
      }

      while (! defparams.empty()) {
	    pair<pform_name_t,NetExpr*> pp = defparams.front();
	    defparams.pop_front();

	    pform_name_t path = pp.first;
	    NetExpr*val = pp.second;

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

	      // Once placed in the parameter map, the expression may
	      // be deleted when evaluated, so give it a copy of this
	      // expression, not the original.
	    val = val->dup_expr();
	    bool flag = targ_scope->replace_parameter(perm_name, val);
	    if (! flag) {
		  cerr << val->get_fileline() << ": warning: parameter "
		       << perm_name << " not found in "
		       << scope_path(targ_scope) << "." << endl;
	    }

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
      list<pair<list<hname_t>,NetExpr*> > defparams_even_later;

      while (! defparams_later.empty()) {
	    pair<list<hname_t>,NetExpr*> cur = defparams_later.front();
	    defparams_later.pop_front();

	    list<hname_t>eval_path = cur.first;
	    perm_string name = eval_path.back().peek_name();
	    eval_path.pop_back();

	    NetExpr*val = cur.second;

	    NetScope*targ_scope = des->find_scope(this, eval_path);
	    if (targ_scope == 0) {
		    // If a scope in the target path is not found,
		    // then push this defparam for handling even
		    // later. Maybe a later generate scheme or
		    // instance array will create the scope.
		  defparams_even_later.push_back(cur);
		  continue;
	    }

	      // Once placed in the parameter map, the expression may
	      // be deleted when evaluated, so give it a copy of this
	      // expression, not the original.
	    val = val->dup_expr();
	    bool flag = targ_scope->replace_parameter(name, val);
	    if (! flag) {
		  cerr << val->get_fileline() << ": warning: parameter "
		       << name << " not found in "
		       << scope_path(targ_scope) << "." << endl;
	    }

	      // We'll need to re-evaluate parameters in this scope
	    target_scopes.insert(targ_scope);
      }

	// All the scopes that this defparam set touched should have
	// their parameters re-evaluated.
      for (set<NetScope*>::iterator cur = target_scopes.begin()
		 ; cur != target_scopes.end() ; cur ++ )
	    (*cur)->evaluate_parameters(des);

	// If there are some scopes that still have missing scopes,
	// then save them back into the defparams_later list for a
	// later pass.
      defparams_later = defparams_even_later;
      if (! defparams_later.empty())
	    des->defparams_later.insert(this);
}

void Design::evaluate_parameters()
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->evaluate_parameters(this);
}

void NetScope::evaluate_parameter_logic_(Design*des, param_ref_t cur)
{
      long msb = 0;
      long lsb = 0;
      bool range_flag = false;

	/* Evaluate the msb expression, if it is present. */
      if ((*cur).second.msb) {
	    eval_expr((*cur).second.msb);
	    if (! eval_as_long(msb, (*cur).second.msb)) {
		  cerr << (*cur).second.expr->get_fileline()
		       << ": error: Unable to evaluate msb expression "
		       << "for parameter " << (*cur).first << ": "
		       << *(*cur).second.msb << endl;
		  des->errors += 1;
		  return;
	    }

	    range_flag = true;
      }

	/* Evaluate the lsb expression, if it is present. */
      if ((*cur).second.lsb) {
	    eval_expr((*cur).second.lsb);
	    if (! eval_as_long(lsb, (*cur).second.lsb)) {
		  cerr << (*cur).second.expr->get_fileline()
		       << ": error: Unable to evaluate lsb expression "
		       << "for parameter " << (*cur).first << ": "
		       << *(*cur).second.lsb << endl;
		  des->errors += 1;
		  return;
	    }

	    range_flag = true;
      }


	/* Evaluate the parameter expression, if necessary. */
      NetExpr*expr = (*cur).second.expr;
      if (expr == NULL) return;  // This is an invalid parameter so return.

      eval_expr(expr);

      /* The eval_expr may delete and replace the expr pointer, so the
         second.expr value cannot be relied on. Might as well replace
         it now with the expression that we evaluated. */
      (*cur).second.expr = expr;

      switch (expr->expr_type()) {
	  case IVL_VT_REAL:
	    if (! dynamic_cast<const NetECReal*>(expr)) {
		  cerr << expr->get_fileline()
		       << ": error: Unable to evaluate real parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  (*cur).second.expr = NULL;
		  return;
	    }
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    if (! dynamic_cast<const NetEConst*>(expr)) {
		  cerr << expr->get_fileline()
		       << ": error: Unable to evaluate parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  (*cur).second.expr = NULL;
		  return;
	    }
	    break;

	  default:
	    cerr << expr->get_fileline()
		 << ": internal error: "
		 << "Unhandled expression type?" << endl;
	    des->errors += 1;
	    (*cur).second.expr = NULL;
	    return;
      }

	/* If the parameter has type or range information, then make
	   sure the type is set right. Note that if the parameter
	   doesn't have an explicit type or range, then it will get
	   the signedness from the expression itself. */
      if (range_flag) {
	    unsigned long wid = (msb >= lsb)? msb - lsb : lsb - msb;
	    wid += 1;

	    /* If we have a real value convert it to an integer. */
	    if(NetECReal*tmp = dynamic_cast<NetECReal*>(expr)) {
		  verinum nval(tmp->value().as_long64());
		  expr = new NetEConst(nval);
		  expr->set_line(*((*cur).second.expr));
		  (*cur).second.expr = expr;
	    }

	    NetEConst*val = dynamic_cast<NetEConst*>(expr);
	    assert(val);

	    verinum value = val->value();

	    if (! (value.has_len()
		   && (value.len() == wid)
		   && (value.has_sign() == (*cur).second.signed_flag))) {

		  verinum tmp (value, wid);
		  tmp.has_sign ( (*cur).second.signed_flag );
		  delete val;
		  val = new NetEConst(tmp);
		  (*cur).second.expr = expr = val;
	    }
      } else if ((*cur).second.signed_flag) {
            (*cur).second.expr->cast_signed(true);
      }

	// If there are no value ranges to test the value against,
	// then we are done.
      if ((*cur).second.range == 0) {
	    return;
      }

      NetEConst*val = dynamic_cast<NetEConst*>((*cur).second.expr);
      ivl_assert(*(*cur).second.expr, (*cur).second.expr);
      ivl_assert(*(*cur).second.expr, val);

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
      NetExpr*expr = (*cur).second.expr;
      if (expr == NULL) return;  // This is an invalid parameter so return.

      NetECReal*res = 0;
      eval_expr(expr);

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
		  (*cur).second.expr = NULL;
		  return;
	    }
	    break;

	  case IVL_VT_LOGIC:
	  case IVL_VT_BOOL:
	    if (NetEConst*tmp = dynamic_cast<NetEConst*>(expr)) {
		  verireal val (tmp->value().as_long());
		  res = new NetECReal(val);
		  res->set_line(*tmp);
	    } else {
		  cerr << expr->get_fileline()
		       << ": error: "
		       << "Unable to evaluate parameter "
		       << (*cur).first << " value: " << *expr << endl;
		  des->errors += 1;
		  (*cur).second.expr = NULL;
		  return;
	    }
	    break;

	  default:
	    cerr << expr->get_fileline()
		 << ": internal error: "
		 << "Unhandled expression type?" << endl;
	    des->errors += 1;
	    (*cur).second.expr = NULL;
	    return;
	    break;
      }

      (*cur).second.expr = res;
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

void NetScope::evaluate_parameters(Design*des)
{
      NetScope*curs = sub_;
      while (curs) {
	    curs->evaluate_parameters(des);
	    curs = curs->sib_;
      }

      if (debug_scopes)
	    cerr << ":0" << ": debug: "
		 << "Evaluate parameters in " << scope_path(this) << endl;

	// Evaluate the parameter values. The parameter expressions
	// have already been elaborated and replaced by the scope
	// scanning code. Now the parameter expression can be fully
	// evaluated, or it cannot be evaluated at all.

      for (param_ref_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {

	      // Resolve the expression type (signed/unsigned) if the
	      // expression is present. It is possible to not be
	      // present if there are earlier errors in elaboration.
	    if (cur->second.expr)
		  cur->second.expr->resolve_pexpr_type();

	    switch ((*cur).second.type) {
		case IVL_VT_BOOL:
		case IVL_VT_LOGIC:
		  evaluate_parameter_logic_(des, cur);
		  break;

		case IVL_VT_REAL:
		  evaluate_parameter_real_(des, cur);
		  break;

		default:
		  cerr << (*cur).second.get_fileline() << ": internal error: "
		       << "Unexpected expression type " << (*cur).second.type
		       << "." << endl;
		  cerr << (*cur).second.get_fileline() << ":               : "
		       << "Parameter name: " << (*cur).first << endl;
		  cerr << (*cur).second.get_fileline() << ":               : "
		       << "Expression is: " << *(*cur).second.expr << endl;
		  ivl_assert((*cur).second, 0);
		  break;
	    }
      }

}

void Design::residual_defparams()
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->residual_defparams(this);
}

void NetScope::residual_defparams(Design*des)
{
	// Clean out the list of defparams that never managed to match
	// a scope. Print a warning for each.
      while (! defparams_later.empty()) {
	    pair<list<hname_t>,NetExpr*> cur = defparams_later.front();
	    defparams_later.pop_front();

	    cerr << cur.second->get_fileline() << ": warning: "
		 << "Scope of " << cur.first << " not found." << endl;
      }

      NetScope*cur = sub_;
      while (cur) {
	    cur->residual_defparams(des);
	    cur = cur->sib_;
      }
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
      if (func && (func->type() == NetScope::FUNC))
	    return func->func_def();

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
      assert(net->design_ == this);
      assert(net != 0);

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
