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
#ident "$Id: net_scope.cc,v 1.7 2000/07/22 22:09:03 steve Exp $"
#endif

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
NetScope::NetScope(const string&n)
: type_(NetScope::MODULE), name_(n), up_(0), sib_(0), sub_(0)
{
      memories_ = 0;
      signals_ = 0;
      events_ = 0;
      lcounter_ = 0;
}

NetScope::NetScope(NetScope*up, const string&n, NetScope::TYPE t)
: type_(t), name_(n), up_(up), sib_(0), sub_(0)
{
      memories_ = 0;
      signals_ = 0;
      events_ = 0;
      time_unit_ = up->time_unit();
      time_prec_ = up->time_precision();
      sib_ = up_->sub_;
      up_->sub_ = this;
}

NetScope::~NetScope()
{
      assert(sib_ == 0);
      assert(sub_ == 0);
      lcounter_ = 0;
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

string NetScope::basename() const
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


NetEvent* NetScope::find_event(const string&name)
{
      for (NetEvent*cur = events_;  cur ;  cur = cur->snext_)
	    if (cur->name() == name)
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
      assert(net->scope_ == this);
      if (signals_ == net)
	    signals_ = net->sig_prev_;

      if (signals_ == net) {
	    signals_ = 0;
      } else {
	    net->sig_prev_->sig_next_ = net->sig_next_;
	    net->sig_next_->sig_prev_ = net->sig_prev_;
      }
}

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

      string fulname = name()+"."+key;
      NetMemory*cur = memories_;
      do {
	    if (cur->name() == fulname)
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
      res << "_l" << (lcounter_++) << ends;
      return res.str();
}


/*
 * $Log: net_scope.cc,v $
 * Revision 1.7  2000/07/22 22:09:03  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.6  2000/05/02 03:13:31  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.5  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.4  2000/04/18 04:50:20  steve
 *  Clean up unneeded NetEvent objects.
 *
 * Revision 1.3  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.2  2000/04/09 17:04:56  steve
 *  uninitialized event_ list.
 *
 * Revision 1.1  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
 */

