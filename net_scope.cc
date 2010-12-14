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
# include "compiler.h"

# include  "netlist.h"
# include  <sstream>
# include  <cstring>

/*
 * The NetScope class keeps a scope tree organized. Each node of the
 * scope tree points to its parent, its right sibling and its leftmost
 * child. The root node has no parent or siblings. The node stores the
 * name of the scope. The complete hierarchical name of the scope is
 * formed by appending the path of scopes from the root to the scope
 * in question.
 */

NetScope::NetScope(NetScope*up, perm_string n, NetScope::TYPE t)
: type_(t), up_(up), sib_(0), sub_(0)
{
      memories_ = 0;
      signals_ = 0;
      events_ = 0;
      vars_ = 0;
      lcounter_ = 0;

      if (up) {
	    time_unit_ = up->time_unit();
	    time_prec_ = up->time_precision();
	    sib_ = up_->sub_;
	    up_->sub_ = this;
      } else {
	    time_unit_ = 0;
	    time_prec_ = 0;
	    assert(t == MODULE);
      }

      switch (t) {
	  case NetScope::TASK:
	    task_ = 0;
	    break;
	  case NetScope::FUNC:
	    func_ = 0;
	    break;
	  case NetScope::MODULE:
	    module_name_ = perm_string();
	    break;
	  default:  /* BEGIN_END and FORK_JOIN, do nothing */
	    break;
      }
      name_ = n;
}

NetScope::~NetScope()
{
      assert(sib_ == 0);
      assert(sub_ == 0);
      lcounter_ = 0;

	/* name_ and module_name_ are perm-allocated. */
}

NetExpr* NetScope::set_parameter(perm_string key, NetExpr*expr,
				 NetExpr*msb, NetExpr*lsb, bool signed_flag)
{
      param_expr_t&ref = parameters[key];
      NetExpr* res = ref.expr;
      ref.expr = expr;
      ref.msb = msb;
      ref.lsb = lsb;
      ref.signed_flag = signed_flag;
      return res;
}

/*
 * Return false if this creates a new parameter.
 */
bool NetScope::replace_parameter(perm_string key, NetExpr*expr)
{
      bool flag = true;
      param_expr_t&ref = parameters[key];

      NetExpr* res = ref.expr;

      if (res) {
	    delete res;
      } else {
	    flag = false;
	    ref.msb = 0;
	    ref.lsb = 0;
	    ref.signed_flag = false;
      }

      ref.expr = expr;
      return flag;
}

NetExpr* NetScope::set_localparam(perm_string key, NetExpr*expr)
{
      param_expr_t&ref = localparams[key];
      NetExpr* res = ref.expr;
      ref.expr = expr;
      ref.msb = 0;
      ref.lsb = 0;
      ref.signed_flag = false;
      return res;
}

/*
 * NOTE: This method takes a const char* as a key to lookup a
 * parameter, because we don't save that pointer. However, due to the
 * way the map<> template works, we need to *cheat* and use the
 * perm_string::literal method to fake the compiler into doing the
 * compare without actually creating a perm_string.
 */
const NetExpr* NetScope::get_parameter(const char* key) const
{
      map<perm_string,param_expr_t>::const_iterator idx;

      idx = parameters.find(perm_string::literal(key));
      if (idx != parameters.end())
	    return (*idx).second.expr;

      idx = localparams.find(perm_string::literal(key));
      if (idx != localparams.end())
	    return (*idx).second.expr;

      return 0;
}

NetScope::TYPE NetScope::type() const
{
      return type_;
}

void NetScope::set_task_def(NetTaskDef*def)
{
      assert( type_ == TASK );
      assert( task_ == 0 );
      task_ = def;
}

NetTaskDef* NetScope::task_def()
{
      assert( type_ == TASK );
      return task_;
}

const NetTaskDef* NetScope::task_def() const
{
      assert( type_ == TASK );
      return task_;
}

void NetScope::set_func_def(NetFuncDef*def)
{
      assert( type_ == FUNC );
      assert( func_ == 0 );
      func_ = def;
}

NetFuncDef* NetScope::func_def()
{
      assert( type_ == FUNC );
      return func_;
}

const NetFuncDef* NetScope::func_def() const
{
      assert( type_ == FUNC );
      return func_;
}

void NetScope::set_module_name(perm_string n)
{
      assert(type_ == MODULE);
      module_name_ = n; /* NOTE: n must have been permallocated. */
}

perm_string NetScope::module_name() const
{
      assert(type_ == MODULE);
      return module_name_;
}

void NetScope::time_unit(int val)
{
      time_unit_ = val;
}

void NetScope::time_precision(int val)
{
      time_prec_ = val;
}

int NetScope::time_unit() const
{
      return time_unit_;
}

int NetScope::time_precision() const
{
      return time_prec_;
}

void NetScope::default_nettype(NetNet::Type nt)
{
      default_nettype_ = nt;
}

NetNet::Type NetScope::default_nettype() const
{
      return default_nettype_;
}

perm_string NetScope::basename() const
{
      return name_;
}

string NetScope::name() const
{
      if (up_)
	    return up_->name() + "." + string(name_);
      else
	    return string(name_);
}

void NetScope::add_event(NetEvent*ev)
{
      assert(ev->scope_ == 0);
      ev->scope_ = this;
      ev->snext_ = events_;
      events_ = ev;
}

void NetScope::rem_event(NetEvent*ev)
{
      assert(ev->scope_ == this);
      ev->scope_ = 0;
      if (events_ == ev) {
	    events_ = ev->snext_;

      } else {
	    NetEvent*cur = events_;
	    while (cur->snext_ != ev) {
		  assert(cur->snext_);
		  cur = cur->snext_;
	    }
	    cur->snext_ = ev->snext_;
      }

      ev->snext_ = 0;
}


NetEvent* NetScope::find_event(const char*name)
{
      for (NetEvent*cur = events_;  cur ;  cur = cur->snext_)
	    if (strcmp(cur->name(), name) == 0)
		  return cur;

      return 0;
}

NetVariable* NetScope::find_variable(const char*name)
{
      for (NetVariable*cur = vars_;  cur ;  cur = cur->snext_)
	    if (strcmp(cur->basename(), name) == 0)
		  return cur;

      return 0;
}

void NetScope::add_signal(NetNet*net)
{
      if (signals_ == 0) {
	    net->sig_next_ = net;
	    net->sig_prev_ = net;
      } else {
	    net->sig_next_ = signals_->sig_next_;
	    net->sig_prev_ = signals_;
	    net->sig_next_->sig_prev_ = net;
	    net->sig_prev_->sig_next_ = net;
      }
      signals_ = net;
}

void NetScope::rem_signal(NetNet*net)
{
      assert(net->scope() == this);
      if (signals_ == net)
	    signals_ = net->sig_prev_;

      if (signals_ == net) {
	    signals_ = 0;
      } else {
	    net->sig_prev_->sig_next_ = net->sig_next_;
	    net->sig_next_->sig_prev_ = net->sig_prev_;
      }
}

/*
 * This method looks for a signal within the current scope. The name
 * is assumed to be the base name of the signal, so no sub-scopes are
 * searched.
 */
NetNet* NetScope::find_signal(const char*key)
{
      if (signals_ == 0)
	    return 0;

      NetNet*cur = signals_;
      do {
	    if (cur->name() == key)
		  return cur;
	    cur = cur->sig_prev_;
      } while (cur != signals_);
      return 0;
}

/*
 * This method searches for the signal within this scope. If the path
 * has hierarchy, I follow the child scopes until I get the base name,
 * and look for the key in the deepest scope.
 */
NetNet* NetScope::find_signal_in_child(const hname_t&path)
{
      NetScope*cur = this;
      unsigned idx = 0;

      while (path.peek_name(idx+1)) {
	    cur = cur->child(path.peek_name(idx));
	    if (cur == 0)
		  return 0;

	    idx += 1;
      }

      return cur->find_signal(path.peek_name(idx));
}

void NetScope::add_memory(NetMemory*mem)
{
      if (memories_ == 0) {
	    mem->snext_ = mem;
	    mem->sprev_ = mem;
      } else {
	    mem->snext_ = memories_->snext_;
	    mem->sprev_ = memories_;
	    mem->snext_->sprev_ = mem;
	    mem->sprev_->snext_ = mem;
      }
      memories_ = mem;
      mem->scope_ = this;
}

void NetScope::rem_memory(NetMemory*mem)
{
      assert(mem->scope_ == this);
      if (memories_ == mem)
	    memories_ = mem->sprev_;

      if (memories_ == mem) {
	    memories_ = 0;
      } else {
	    mem->sprev_->snext_ = mem->snext_;
	    mem->snext_->sprev_ = mem->sprev_;
      }
      mem->scope_ = 0;
}

NetMemory* NetScope::find_memory(const string&key)
{
      if (memories_ == 0)
	    return 0;

      NetMemory*cur = memories_;
      do {
	    if (cur->name() == key.c_str())
		  return cur;
	    cur = cur->sprev_;
      } while (cur != memories_);
      return 0;
}

void NetScope::add_variable(NetVariable*var)
{
      assert(var->scope_ == 0);
      var->scope_ = this;
      var->snext_ = vars_;
      vars_ = var;
}

/*
 * This method locates a child scope by name. The name is the simple
 * name of the child, no hierarchy is searched.
 */
NetScope* NetScope::child(const char*name)
{
      if (sub_ == 0) return 0;

      NetScope*cur = sub_;
      while (strcmp(cur->name_, name) != 0) {
	    if (cur->sib_ == 0) return 0;
	    cur = cur->sib_;
      }

      return cur;
}

const NetScope* NetScope::child(const char*name) const
{
      if (sub_ == 0) return 0;

      NetScope*cur = sub_;
      while (strcmp(cur->name_, name) != 0) {
	    if (cur->sib_ == 0) return 0;
	    cur = cur->sib_;
      }

      return cur;
}

NetScope* NetScope::parent()
{
      return up_;
}

const NetScope* NetScope::parent() const
{
      return up_;
}

perm_string NetScope::local_symbol()
{
      ostringstream res;
      res << "_s" << (lcounter_++);
      return lex_strings.make(res.str());
}

string NetScope::local_hsymbol()
{
      return string(name()) + "." + string(local_symbol());
}
