/*
 * Copyright (c) 2000Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_design.cc,v 1.8 2000/05/02 16:27:38 steve Exp $"
#endif

/*
 * This source file contains all the implementations of the Design
 * class declared in netlist.h.
 */

# include  "netlist.h"
# include  "util.h"
# include  <strstream>

static string parse_last_name(string&path)
{
      unsigned pos = path.rfind('.');
      if (pos > path.length()) {
	    string res = path;
	    path = "";
	    return res;
      }

      string res = path.substr(pos+1, path.length());
      path = path.substr(0, pos);
      return res;
}

Design:: Design()
: errors(0), root_scope_(0), nodes_(0), procs_(0), lcounter_(0)
{
}

Design::~Design()
{
}

string Design::local_symbol(const string&path)
{
      strstream res;
      res << "_L" << (lcounter_++) << ends;

      return path + "." + res.str();
}

NetScope* Design::make_root_scope(const string&root)
{
      assert(root_scope_ == 0);
      root_scope_ = new NetScope(root);
      return root_scope_;
}

NetScope* Design::find_root_scope()
{
      assert(root_scope_);
      return root_scope_;
}

/*
 * This method locates a scope in the design, given its rooted
 * heirarchical name. Each component of the key is used to scan one
 * more step down the tree until the name runs out or the search
 * fails.
 */
NetScope* Design::find_scope(const string&key) const
{
      if (key == root_scope_->name())
	    return root_scope_;

      string path = key;
      string root = parse_first_name(path);

      NetScope*cur = root_scope_;
      if (root != cur->name())
	    return 0;

      while (cur) {
	    string next = parse_first_name(path);
	    cur = cur->child(next);
	    if (path == "") return cur;
      }

      return cur;
}

/*
 * This is a relative lookup of a scope by name. The starting point is
 * the scope parameter is the place within which I start looking for
 * the scope. If I do not find the scope within the passed scope,
 * start looking in parent scopes until I find it, or I run out of
 * parent scopes.
 */
NetScope* Design::find_scope(NetScope*scope, const string&name) const
{
      assert(scope);

      for ( ; scope ;  scope = scope->parent()) {
	    string path = name;
	    string key = parse_first_name(path);

	    NetScope*cur = scope;
	    do {
		  cur = cur->child(key);
		  if (cur == 0) break;
		  key = parse_first_name(path);
	    } while (key != "");

	    if (cur) return cur;
      }

	// Last chance. Look for the name starting at the root.
      return find_scope(name);
}

/*
 * Find a parameter from within a specified context. If the name is
 * not here, keep looking up until I run out of up to look at. The
 * method works by scanning scopes, starting with the passed scope and
 * working up towards the root, looking for the named parameter. The
 * name in this case can be hierarchical, so there is an inner loop to
 * follow the scopes of the name down to to key.
 */
const NetExpr* Design::find_parameter(const NetScope*scope,
				      const string&name) const
{
      for ( ; scope ;  scope = scope->parent()) {
	    string path = name;
	    string key = parse_first_name(path);

	    const NetScope*cur = scope;
	    while (path != "") {
		  cur = cur->child(key);
		  if (cur == 0) break;
		  key = parse_first_name(path);
	    }

	    if (cur == 0) continue;

	    if (const NetExpr*res = cur->get_parameter(key))
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
      root_scope_->run_defparams(this);
}

void NetScope::run_defparams(Design*des)
{
      NetScope*cur = sub_;
      while (cur) {
	    cur->run_defparams(des);
	    cur = cur->sib_;
      }

      map<string,NetExpr*>::const_iterator pp;
      for (pp = defparams.begin() ;  pp != defparams.end() ;  pp ++ ) {
	    NetExpr*val = (*pp).second;
	    string path = (*pp).first;
	    string name = parse_last_name(path);

	    NetScope*targ_scope = des->find_scope(this, path);
	    if (targ_scope == 0) {
		  cerr << val->get_line() << ": warning: scope of " <<
			path << "." << name << " not found." << endl;
		  continue;
	    }

	    val = targ_scope->set_parameter(name, val);
	    if (val == 0) {
		  cerr << val->get_line() << ": warning: parameter "
		       << name << " not found in " << targ_scope->name()
		       << "." << endl;
	    } else {
		  delete val;
	    }
      }
}

void Design::evaluate_parameters()
{
      root_scope_->evaluate_parameters(this);
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

      typedef map<string,NetExpr*>::iterator mparm_it_t;

      for (mparm_it_t cur = parameters_.begin()
		 ; cur != parameters_.end() ;  cur ++) {

	      // Get the NetExpr for the parameter.
	    NetExpr*expr = (*cur).second;
	    assert(expr);

	      // If it's already a NetEConst, then this parameter is done.
	    if (dynamic_cast<const NetEConst*>(expr))
		  continue;

	      // Try to evaluate the expression.
	    NetExpr*nexpr = expr->eval_tree();
	    if (nexpr == 0) {
		  cerr << (*cur).second->get_line() << ": internal error: "
			"unable to evaluate parm expression: " <<
			*expr << endl;
		  des->errors += 1;
		  continue;
	    }

	      // The evaluate worked, replace the old expression with
	      // this constant value.
	    assert(nexpr);
	    delete expr;
	    (*cur).second = nexpr;
      }

}

string Design::get_flag(const string&key) const
{
      map<string,string>::const_iterator tmp = flags_.find(key);
      if (tmp == flags_.end())
	    return "";
      else
	    return (*tmp).second;
}

/*
 * This method looks for a string given a current context as a
 * starting point.
 */
NetNet* Design::find_signal(NetScope*scope, const string&name)
{
      assert(scope);

	/* If the name has a path attached to it, parse it off and use
	   that to locate the desired scope. */
      string path = name;
      string key = parse_last_name(path);
      if (path != "")
	    scope = find_scope(scope, path);

      while (scope) {

	    if (NetNet*net = scope->find_signal(key))
		  return net;
	    scope = scope->parent();
      }

      return 0;
}

NetMemory* Design::find_memory(NetScope*scope, const string&name)
{
      assert(scope);

	/* If the name has a path attached to it, parse it off and use
	   that to locate the desired scope. */
      string path = name;
      string key = parse_last_name(path);
      if (path != "")
	    scope = find_scope(scope, path);

      while (scope) {

	    if (NetMemory*mem = scope->find_memory(key))
		  return mem;
	    scope = scope->parent();
      }
      return 0;
}

void Design::find_symbol(NetScope*scope, const string&name,
			 NetNet*&sig, NetMemory*&mem)
{
      sig = 0;
      mem = 0;


	/* If the name has a path attached to it, parse it off and use
	   that to locate the desired scope. Then locate the key
	   within that scope. */
      string path = name;
      string key = parse_last_name(path);
      if (path != "")
	    scope = find_scope(scope, path);


	/* If there is no path, then just search upwards for the key. */
      while (scope) {

	    if (NetNet*cur = scope->find_signal(key)) {
		  sig = cur;
		  return;
	    }

	    if (NetMemory*cur = scope->find_memory(key)) {
		  mem = cur;
		  return;
	    }

	    scope = scope->parent();
      }
}

void Design::add_function(const string&key, NetFuncDef*def)
{
      funcs_[key] = def;
}

NetFuncDef* Design::find_function(const string&path, const string&name)
{
      string root = path;
      for (;;) {
	    string key = root + "." + name;
	    map<string,NetFuncDef*>::const_iterator cur = funcs_.find(key);
	    if (cur != funcs_.end())
		  return (*cur).second;

	    unsigned pos = root.rfind('.');
	    if (pos > root.length())
		  break;

	    root = root.substr(0, pos);
      }

      return 0;
}

NetFuncDef* Design::find_function(const string&key)
{
      map<string,NetFuncDef*>::const_iterator cur = funcs_.find(key);
      if (cur != funcs_.end())
	    return (*cur).second;
      return 0;
}

void Design::add_task(const string&key, NetTaskDef*def)
{
      tasks_[key] = def;
}

NetTaskDef* Design::find_task(const string&path, const string&name)
{
      string root = path;
      for (;;) {
	    string key = root + "." + name;
	    map<string,NetTaskDef*>::const_iterator cur = tasks_.find(key);
	    if (cur != tasks_.end())
		  return (*cur).second;

	    unsigned pos = root.rfind('.');
	    if (pos > root.length())
		  break;

	    root = root.substr(0, pos);
      }

      return 0;
}

NetTaskDef* Design::find_task(const string&key)
{
      map<string,NetTaskDef*>::const_iterator cur = tasks_.find(key);
      if (cur == tasks_.end())
	    return 0;

      return (*cur).second;
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

void Design::clear_node_marks()
{
      if (nodes_ == 0)
	    return;

      NetNode*cur = nodes_;
      do {
	    cur->set_mark(false);
	    cur = cur->node_next_;
      } while (cur != nodes_);
}

NetNode* Design::find_node(bool (*func)(const NetNode*))
{
      if (nodes_ == 0)
	    return 0;

      NetNode*cur = nodes_->node_next_;
      do {
	    if ((cur->test_mark() == false) && func(cur))
		  return cur;

	    cur = cur->node_next_;
      } while (cur != nodes_->node_next_);

      return 0;
}

/*
 * $Log: net_design.cc,v $
 * Revision 1.8  2000/05/02 16:27:38  steve
 *  Move signal elaboration to a seperate pass.
 *
 * Revision 1.7  2000/05/02 03:13:31  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.6  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.5  2000/04/28 16:50:53  steve
 *  Catch memory word parameters to tasks.
 *
 * Revision 1.4  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.3  2000/03/11 03:25:52  steve
 *  Locate scopes in statements.
 *
 * Revision 1.2  2000/03/10 06:20:48  steve
 *  Handle defparam to partial hierarchical names.
 *
 * Revision 1.1  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 */

