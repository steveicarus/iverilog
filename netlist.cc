/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: netlist.cc,v 1.2 1998/11/07 17:05:05 steve Exp $"
#endif

# include  <cassert>
# include  "netlist.h"

void connect(NetObj::Link&l, NetObj::Link&r)
{
      NetObj::Link* cur = &l;
      do {
	    NetObj::Link*tmp = cur->next_;
	      // Pull cur out of left list...
	    cur->prev_->next_ = cur->next_;
	    cur->next_->prev_ = cur->prev_;

	      // Put cur in right list
	    cur->next_ = r.next_;
	    cur->prev_ = &r;
	    cur->next_->prev_ = cur;
	    cur->prev_->next_ = cur;

	      // Go to the next item in the left list.
	    cur = tmp;
      } while (cur != &l);
}

const NetNet* find_link_signal(const NetObj*net, unsigned pin, unsigned&bidx)
{
      const NetObj*cur;
      unsigned cpin;
      net->pin(pin).next_link(cur, cpin);

      while (cur != net) {
	    const NetNet*sig = dynamic_cast<const NetNet*>(cur);
	    if (sig) {
		  bidx = cpin;
		  return sig;
	    }
	    cur->pin(cpin).next_link(cur, cpin);
      }

      return 0;
}

NetObj::NetObj(const string&n, unsigned np)
: name_(n), npins_(np), delay1_(0), delay2_(0), delay3_(0)
{
      pins_ = new Link[npins_];
      for (unsigned idx = 0 ;  idx < npins_ ;  idx += 1) {
	    pins_[idx].node_ = this;
	    pins_[idx].pin_  = idx;
      }
}

NetObj::~NetObj()
{
      delete[]pins_;
}

NetNode::~NetNode()
{
      if (design_)
	    design_->del_node(this);
}

NetNet::~NetNet()
{
      if (design_)
	    design_->del_signal(this);
}

NetProc::~NetProc()
{
}

NetAssign::NetAssign(NetNet*lv, NetExpr*rv)
: NetNode("@assign", lv->pin_count()), lval_(lv), rval_(rv)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    connect(pin(idx), lv->pin(idx));
      }
}

NetAssign::~NetAssign()
{
}

NetBlock::~NetBlock()
{
}

void NetBlock::append(NetProc*cur)
{
      if (last_ == 0) {
	    last_ = cur;
	    cur->next_ = cur;
      } else {
	    cur->next_ = last_->next_;
	    last_->next_ = cur;
	    last_ = cur;
      }
}

NetTask::~NetTask()
{
      delete[]parms_;
}

NetExpr::~NetExpr()
{
}

NetEBinary::~NetEBinary()
{
}

NetEConst::~NetEConst()
{
}

NetEBinary::NetEBinary(char op, NetExpr*l, NetExpr*r)
: op_(op), left_(l), right_(r)
{
      switch (op_) {
	  case 'e':
	    natural_width(1);
	    break;
	  default:
	    natural_width(left_->natural_width() > right_->natural_width()
			  ? left_->natural_width() : right_->natural_width());
	    break;
      }
}

void Design::add_signal(NetNet*net)
{
      assert(net->design_ == 0);
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
      net->design_ = this;
}

void Design::del_signal(NetNet*net)
{
      assert(net->design_ == this);
      if (signals_ == net)
	    signals_ = net->sig_prev_;

      if (signals_ == net) {
	    signals_ = 0;
      } else {
	    net->sig_prev_->sig_next_ = net->sig_next_;
	    net->sig_next_->sig_prev_ = net->sig_prev_;
      }
      net->design_ = 0;
}

NetNet* Design::find_signal(const string&name)
{
      if (signals_ == 0)
	    return 0;

      NetNet*cur = signals_;
      do {
	    if (cur->name() == name)
		  return cur;

	    cur = cur->sig_prev_;
      } while (cur != signals_);

      return 0;
}

void Design::scan_signals(SigFunctor*fun)
{
      if (signals_ == 0)
	    return;

      NetNet*cur = signals_->sig_next_;
      do {
	    NetNet*next = cur->sig_next_;
	    fun->sig_function(cur);
	    cur = next;
      } while (cur != signals_->sig_next_);
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


/*
 * $Log: netlist.cc,v $
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:00  steve
 *  Introduce verilog to CVS.
 *
 */

