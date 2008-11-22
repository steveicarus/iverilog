/*
 * Copyright (c) 2000-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_design.cc,v 1.45.2.1 2006/10/04 00:34:45 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

/*
 * This source file contains all the implementations of the Design
 * class declared in netlist.h.
 */

# include  "netlist.h"
# include  "util.h"
# include  "compiler.h"
# include  <sstream>
# include  <cstring>

Design:: Design()
: errors(0), nodes_(0), procs_(0), lcounter_(0)
{
      procs_idx_ = 0;
      des_precision_ = 0;
      nodes_functor_cur_ = 0;
      nodes_functor_nxt_ = 0;
}

Design::~Design()
{
}

string Design::local_symbol(const string&path)
{
      ostringstream res;
      res << path << "." << "_L" << lcounter_;
      lcounter_ += 1;

      return res.str();
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

unsigned long Design::scale_to_precision(unsigned long val,
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
      root_scope_ = new NetScope(0, root, NetScope::MODULE);
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
NetScope* Design::find_scope(const hname_t&path) const
{
      if (path.peek_name(0) == 0)
	    return 0;

      for (list<NetScope*>::const_iterator scope = root_scopes_.begin()
		 ; scope != root_scopes_.end(); scope++) {

	    NetScope*cur = *scope;
	    if (strcmp(path.peek_name(0), cur->basename()) != 0)
		  continue;

	    unsigned hidx = 1;
	    while (cur) {
		  const char*name = path.peek_name(hidx);
		  if (name == 0)
			return cur;

		  cur = cur->child(name);
		  hidx += 1;
	    }

      }

      return 0;
}

/*
 * This is a relative lookup of a scope by name. The starting point is
 * the scope parameter is the place within which I start looking for
 * the scope. If I do not find the scope within the passed scope,
 * start looking in parent scopes until I find it, or I run out of
 * parent scopes.
 */
NetScope* Design::find_scope(NetScope*scope, const hname_t&path) const
{
      assert(scope);
      if (path.peek_name(0) == 0)
	    return scope;

      for ( ; scope ;  scope = scope->parent()) {
	    unsigned hidx = 0;
	    const char*key = path.peek_name(hidx);

	    NetScope*cur = scope;
	    do {
		  cur = cur->child(key);
		  if (cur == 0) break;
		  hidx += 1;
		  key = path.peek_name(hidx);
	    } while (key);

	    if (cur) return cur;
      }

	// Last chance. Look for the name starting at the root.
      return find_scope(path);
}

/*
 * Find a parameter from within a specified context. If the name is
 * not here, keep looking up until I run out of up to look at. The
 * method works by scanning scopes, starting with the passed scope and
 * working up towards the root, looking for the named parameter. The
 * name in this case can be hierarchical, so there is an inner loop to
 * follow the scopes of the name down to to key.
 *
 * The expression value of the parameter is returned as the result,
 * and the scope that contains the parameter is returned in the out
 * argument found_in.
 */
const NetExpr* Design::find_parameter(NetScope*scope,
				      const hname_t&path,
				      NetScope*&found_in) const
{
      for ( ; scope ;  scope = scope->parent()) {
	    unsigned hidx = 0;

	    NetScope*cur = scope;
	    while (path.peek_name(hidx+1)) {
		  cur = cur->child(path.peek_name(hidx));
		  if (cur == 0)
			break;
		  hidx += 1;
	    }

	    if (cur == 0)
		  continue;

	    if (const NetExpr*res = cur->get_parameter(path.peek_name(hidx))) {
		  found_in = cur;
		  return res;
	    }
      }

      return 0;
}

const NetExpr* Design::find_parameter(const NetScope*scope,
				      const hname_t&path) const
{
      for ( ; scope ;  scope = scope->parent()) {
	    unsigned hidx = 0;

	    const NetScope*cur = scope;
	    while (path.peek_name(hidx+1)) {
		  cur = cur->child(path.peek_name(hidx));
		  if (cur == 0)
			break;
		  hidx += 1;
	    }

	    if (cur == 0)
		  continue;

	    if (const NetExpr*res = cur->get_parameter(path.peek_name(hidx)))
		  return res;

      }

      return 0;
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

      map<hname_t,NetExpr*>::const_iterator pp;
      for (pp = defparams.begin() ;  pp != defparams.end() ;  pp ++ ) {
	    NetExpr*val = (*pp).second;
	    hname_t path = (*pp).first;

	    char*tmp = path.remove_tail_name();
	    perm_string perm_name = lex_strings.make(tmp);
	    delete[]tmp;

	      /* If there is no path on the name, then the targ_scope
		 is the current scope. */
	    NetScope*targ_scope = des->find_scope(this, path);
	    if (targ_scope == 0) {
		  cerr << val->get_line() << ": warning: scope of " <<
			path << "." << perm_name << " not found." << endl;
		  continue;
	    }

	    bool flag = targ_scope->replace_parameter(perm_name, val->dup_expr());
	    if (! flag) {
		  cerr << val->get_line() << ": warning: parameter "
		       << perm_name << " not found in "
		       << targ_scope->name() << "." << endl;
	    }

      }
}

void Design::evaluate_parameters()
{
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->evaluate_parameters(this);
}

void NetScope::evaluate_parameters(Design*des)
{
      NetScope*cur = sub_;
      while (cur) {
	    cur->evaluate_parameters(des);
	    cur = cur->sib_;
      }


	// Evaluate the parameter values. The parameter expressions
	// have already been elaborated and replaced by the scope
	// scanning code. Now the parameter expression can be fully
	// evaluated, or it cannot be evaluated at all.

      typedef map<perm_string,param_expr_t>::iterator mparm_it_t;

      for (mparm_it_t cur = parameters.begin()
		 ; cur != parameters.end() ;  cur ++) {

	    long msb = 0;
	    long lsb = 0;
	    bool range_flag = false;
	    NetExpr*expr;

	      /* Evaluate the msb expression, if it is present. */
	    expr = (*cur).second.msb;

	    if (expr) {

		  NetEConst*tmp = dynamic_cast<NetEConst*>(expr);

		  if (! tmp) {

			NetExpr*nexpr = expr->eval_tree();
			if (nexpr == 0) {
			      cerr << (*cur).second.expr->get_line()
				   << ": internal error: "
				   << "unable to evaluate msb expression "
				   << "for parameter " << (*cur).first << ": "
				   << *expr << endl;
			      des->errors += 1;
			      continue;
			}

			assert(nexpr);
			delete expr;
			(*cur).second.msb = nexpr;

			tmp = dynamic_cast<NetEConst*>(nexpr);
		  }

		  assert(tmp);
		  msb = tmp->value().as_long();
		  range_flag = true;
	    }

	      /* Evaluate the lsb expression, if it is present. */
	    expr = (*cur).second.lsb;
	    if (expr) {

		  NetEConst*tmp = dynamic_cast<NetEConst*>(expr);

		  if (! tmp) {

			NetExpr*nexpr = expr->eval_tree();
			if (nexpr == 0) {
			      cerr << (*cur).second.expr->get_line()
				   << ": internal error: "
				   << "unable to evaluate lsb expression "
				   << "for parameter " << (*cur).first << ": "
				   << *expr << endl;
			      des->errors += 1;
			      continue;
			}

			assert(nexpr);
			delete expr;
			(*cur).second.lsb = nexpr;

			tmp = dynamic_cast<NetEConst*>(nexpr);
		  }

		  assert(tmp);
		  lsb = tmp->value().as_long();

		  assert(range_flag);
	    }


	      /* Evaluate the parameter expression, if necessary. */
	    expr = (*cur).second.expr;
	    assert(expr);

	    switch (expr->expr_type()) {
		case NetExpr::ET_REAL:
		  if (! dynamic_cast<const NetECReal*>(expr)) {
			cerr << (*cur).second.expr->get_line()
			     << ": internal error: "
			      "unable to evaluate real parameter values: " <<
			      *expr << endl;
			cerr << "FYI: V0.8 cannot evaluate constant real expressions." << endl;
			des->errors += 1;
			continue;
		  }
		  break;

		case NetExpr::ET_VECTOR:
		  if (! dynamic_cast<const NetEConst*>(expr)) {

			  // Try to evaluate the expression.
			NetExpr*nexpr = expr->eval_tree();
			if (nexpr == 0) {
			      cerr << (*cur).second.expr->get_line()
				   << ": internal error: "
				   << "unable to evaluate parameter "
				   << (*cur).first
				   << " value: " <<
				    *expr << endl;
			      des->errors += 1;
			      continue;
			}

			  // The evaluate worked, replace the old
			  // expression with this constant value.
			assert(nexpr);
			delete expr;
			(*cur).second.expr = nexpr;
		  }
		  break;

		default:
		  cerr << (*cur).second.expr->get_line()
		       << ": internal error: "
		       << "unhandled expression type?" << endl;
		  des->errors += 1;
		  continue;
	    }

	      /* If the parameter has range information, then make
		 sure the value is set right. Note that if the
		 parameter doesn't have an explicit range, then it
		 will get the signedness from the expression itself. */
	    if (range_flag) {
		  unsigned long wid = (msb >= lsb)? msb - lsb : lsb - msb;
		  wid += 1;

		  NetEConst*val = dynamic_cast<NetEConst*>((*cur).second.expr);
		  assert(val);

		  verinum value = val->value();

		  if (! (value.has_len()
			 && (value.len() == wid)
			 && (value.has_sign() == (*cur).second.signed_flag))) {

			verinum tmp (value, wid);
			tmp.has_sign ( (*cur).second.signed_flag );
			delete val;
			val = new NetEConst(tmp);
			(*cur).second.expr = val;
		  }
	    }
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
NetNet* Design::find_signal(NetScope*scope, hname_t path)
{
      assert(scope);

      char*key = path.remove_tail_name();
      if (path.peek_name(0))
	    scope = find_scope(scope, path);

      while (scope) {
	    if (NetNet*net = scope->find_signal(key)) {
		  delete key;
		  return net;
	    }

	    if (scope->type() == NetScope::MODULE)
		  break;
	    scope = scope->parent();
      }

      delete key;
      return 0;
}

NetFuncDef* Design::find_function(NetScope*scope, const hname_t&name)
{
      assert(scope);
      NetScope*func = find_scope(scope, name);
      if (func && (func->type() == NetScope::FUNC))
	    return func->func_def();

      return 0;
}

NetFuncDef* Design::find_function(const hname_t&key)
{
      NetScope*func = find_scope(key);
      if (func && (func->type() == NetScope::FUNC))
	    return func->func_def();

      return 0;
}

NetScope* Design::find_task(NetScope*scope, const hname_t&name)
{
      NetScope*task = find_scope(scope, name);
      if (task && (task->type() == NetScope::TASK))
	    return task;

      return 0;
}

NetScope* Design::find_task(const hname_t&key)
{
      NetScope*task = find_scope(key);
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

void Design::add_process(NetProcTop*pro)
{
      pro->next_ = procs_;
      procs_ = pro;
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

/*
 * $Log: net_design.cc,v $
 * Revision 1.45.2.1  2006/10/04 00:34:45  steve
 *  Fix a dangling reference to NetEParam objects in defparams.
 *
 * Revision 1.45  2004/10/04 01:10:54  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.44  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.43  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.42  2003/11/10 20:59:03  steve
 *  Design::get_flag returns const char* instead of string.
 *
 * Revision 1.41  2003/09/20 01:05:36  steve
 *  Obsolete find_symbol and find_event from the Design class.
 *
 * Revision 1.40  2003/09/19 03:50:12  steve
 *  Remove find_memory method from Design class.
 *
 * Revision 1.39  2003/09/19 03:30:05  steve
 *  Fix name search in elab_lval.
 *
 * Revision 1.38  2003/08/28 04:11:19  steve
 *  Spelling patch.
 *
 * Revision 1.37  2003/06/24 01:38:02  steve
 *  Various warnings fixed.
 *
 * Revision 1.36  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.35  2003/03/06 04:37:12  steve
 *  lex_strings.add module names earlier.
 *
 * Revision 1.34  2003/02/01 23:37:34  steve
 *  Allow parameter expressions to be type real.
 *
 * Revision 1.33  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.32  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.31  2003/01/14 21:16:18  steve
 *  Move strstream to ostringstream for compatibility.
 *
 * Revision 1.30  2002/12/07 02:49:24  steve
 *  Named event triggers can take hierarchical names.
 *
 * Revision 1.29  2002/11/02 03:27:52  steve
 *  Allow named events to be referenced by
 *  hierarchical names.
 *
 * Revision 1.28  2002/10/19 22:59:49  steve
 *  Redo the parameter vector support to allow
 *  parameter names in range expressions.
 *
 * Revision 1.27  2002/08/16 05:18:27  steve
 *  Fix intermix of node functors and node delete.
 *
 * Revision 1.26  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.25  2002/07/03 05:34:59  steve
 *  Fix scope search for events.
 *
 * Revision 1.24  2002/06/25 02:39:34  steve
 *  Fix mishandling of incorect defparam error message.
 *
 * Revision 1.23  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.22  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 */

