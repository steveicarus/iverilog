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
#ident "$Id: net_scope.cc,v 1.18 2002/08/05 04:18:45 steve Exp $"
#endif

# include "config.h"

# include  "netlist.h"
# include  <strstream>

/*
 * The NetScope class keeps a scope tree organized. Each node of the
 * scope tree points to its parent, its right sibling and its leftmost
 * child. The root node has no parent or siblings. The node stores the
 * name of the scope. The complete hierarchical name of the scope is
 * formed by appending the path of scopes from the root to the scope
 * in question.
 */

NetScope::NetScope(NetScope*up, const char*n, NetScope::TYPE t)
: type_(t), up_(up), sib_(0), sub_(0)
{
      memories_ = 0;
      signals_ = 0;
      events_ = 0;
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
	    module_name_ = 0;
	    break;
      }
      name_ = new char[strlen(n)+1];
      strcpy(name_, n);
}

NetScope::~NetScope()
{
      assert(sib_ == 0);
      assert(sub_ == 0);
      lcounter_ = 0;
      delete[]name_;
      if ((type_ == MODULE) && module_name_)
	    free(module_name_);
}

NetExpr* NetScope::set_parameter(const string&key, NetExpr*expr)
{
      NetExpr*&ref = parameters_[key];
      NetExpr* res = ref;
      ref = expr;
      return res;
}

NetExpr* NetScope::set_localparam(const string&key, NetExpr*expr)
{
      NetExpr*&ref = localparams_[key];
      NetExpr* res = ref;
      ref = expr;
      return res;
}

const NetExpr* NetScope::get_parameter(const string&key) const
{
      map<string,NetExpr*>::const_iterator idx;

      idx = parameters_.find(key);
      if (idx != parameters_.end())
	    return (*idx).second;

      idx = localparams_.find(key);
      if (idx != localparams_.end())
	    return (*idx).second;

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

void NetScope::set_module_name(const char*n)
{
      assert(type_ == MODULE);
      module_name_ = strdup(n);
}

const char* NetScope::module_name() const
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

const char* NetScope::basename() const
{
      return name_;
}

string NetScope::name() const
{
      if (up_)
	    return up_->name() + "." + name_;
      else
	    return name_;
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


NetEvent* NetScope::find_event(const hname_t&name)
{
      for (NetEvent*cur = events_;  cur ;  cur = cur->snext_)
	    if (strcmp(cur->name(), name.peek_tail_name()) == 0)
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
NetNet* NetScope::find_signal(const string&key)
{
      if (signals_ == 0)
	    return 0;

      string fulname = name()+"."+key;
      NetNet*cur = signals_;
      do {
	    if (cur->name() == fulname)
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
	    if (cur->name() == key)
		  return cur;
	    cur = cur->sprev_;
      } while (cur != memories_);
      return 0;
}

/*
 * This method locates a child scope by name. The name is the simple
 * name of the child, no heirarchy is searched.
 */
NetScope* NetScope::child(const string&name)
{
      if (sub_ == 0) return 0;

      NetScope*cur = sub_;
      while (cur->name_ != name) {
	    if (cur->sib_ == 0) return 0;
	    cur = cur->sib_;
      }

      return cur;
}

const NetScope* NetScope::child(const string&name) const
{
      if (sub_ == 0) return 0;

      NetScope*cur = sub_;
      while (cur->name_ != name) {
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

string NetScope::local_symbol()
{
      strstream res;
      res << "_s" << (lcounter_++) << ends;
      return res.str();
}

string NetScope::local_hsymbol()
{
      return name() + "." + local_symbol();
}


/*
 * $Log: net_scope.cc,v $
 * Revision 1.18  2002/08/05 04:18:45  steve
 *  Store only the base name of memories.
 *
 * Revision 1.17  2002/07/22 21:07:08  steve
 *  Initialize the lcounter_ to 0.
 *
 * Revision 1.16  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.15  2001/11/08 05:15:50  steve
 *  Remove string paths from PExpr elaboration.
 *
 * Revision 1.14  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.13  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.12  2001/07/04 04:34:06  steve
 *  Scope-locals use _s instead of _l.
 *
 * Revision 1.11  2000/12/16 01:45:48  steve
 *  Detect recursive instantiations (PR#2)
 *
 * Revision 1.10  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.9  2000/08/27 15:51:50  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.8  2000/07/30 18:25:44  steve
 *  Rearrange task and function elaboration so that the
 *  NetTaskDef and NetFuncDef functions are created during
 *  signal enaboration, and carry these objects in the
 *  NetScope class instead of the extra, useless map in
 *  the Design class.
 */

