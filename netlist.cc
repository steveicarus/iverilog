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
#ident "$Id: netlist.cc,v 1.8 1998/11/23 00:20:23 steve Exp $"
#endif

# include  <cassert>
# include  <typeinfo>
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

bool NetObj::Link::is_linked(const NetObj&that) const
{
      for (const Link*idx = next_ ; this != idx ;  idx = idx->next_)
	    if (idx->node_ == &that)
		  return true;

      return false;
}

bool NetObj::Link::is_linked(const NetObj::Link&that) const
{
      for (const Link*idx = next_ ; this != idx ;  idx = idx->next_)
	    if (idx == &that)
		  return true;

      return false;
}

bool connected(const NetObj&l, const NetObj&r)
{
      for (unsigned idx = 0 ;  idx < l.pin_count() ;  idx += 1)
	    if (! l.pin(idx).is_linked(r))
		  return false;

      return true;
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
: name_(n), npins_(np), delay1_(0), delay2_(0), delay3_(0), mark_(false)
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

void NetObj::set_attributes(const map<string,string>&attr)
{
      assert(attributes_.size() == 0);
      attributes_ = attr;
}

string NetObj::attribute(const string&key) const
{
      map<string,string>::const_iterator idx = attributes_.find(key);
      if (idx == attributes_.end())
	    return "";

      return (*idx).second;
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
: NetNode("@assign", lv->pin_count()), rval_(rv)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    connect(pin(idx), lv->pin(idx));
      }

      rval_->set_width(lv->pin_count());
}

NetAssign::~NetAssign()
{
}

/*
 * This method looks at the objects connected to me, and searches for
 * a signal that I am fully connected to. Return that signal, and the
 * range of bits that I use.
 */
void NetAssign::find_lval_range(const NetNet*&net, unsigned&msb,
				unsigned&lsb) const
{
      const NetObj*cur;
      unsigned cpin;

      for (pin(0).next_link(cur,cpin) ; pin(0) != cur->pin(cpin)
		 ; cur->pin(cpin).next_link(cur, cpin)) {
	    const NetNet*s = dynamic_cast<const NetNet*>(cur);
	    if (s == 0)
		  continue;

	    if (!connected(*this, *s))
		  continue;

	    unsigned idx;
	    for (idx = 1 ;  idx < pin_count() ;  idx += 1) {
		  if (idx+cpin > s->pin_count())
			break;
		  if (! connected(pin(idx), s->pin(idx+cpin)))
			break;
	    }

	    if (idx < pin_count())
		  continue;

	    net = s;
	    lsb = cpin;
	    msb = cpin+pin_count()-1;
	    return;
      }

      assert(0); // No suitable signals??
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

void NetExpr::set_width(unsigned w)
{
      cerr << typeid(*this).name() << ": set_width(unsigned) "
	    "not implemented." << endl;
      expr_width(w);
}


NetEBinary::~NetEBinary()
{
}


NetEBinary::NetEBinary(char op, NetExpr*l, NetExpr*r)
: op_(op), left_(l), right_(r)
{
      switch (op_) {
	      // comparison operators return a 1-bin wide result.
	  case 'e':
	  case 'n':
	    expr_width(1);
	    break;
	  default:
	    expr_width(left_->expr_width() > right_->expr_width()
			  ? left_->expr_width() : right_->expr_width());
	    break;
      }
}

void NetEBinary::set_width(unsigned w)
{
      switch (op_) {
	      /* Comparison operators allow the subexpressions to have
		 their own natural width. Do not recurse the
		 set_width(). */
	  case 'e':
	    assert(w == 1);
	    expr_width(w);
	    break;;

	      /* The default rule is that the operands of the binary
		 operator might as well use the same width as the
		 output from the binary operation. */
	  default:
	    cerr << "NetEBinary::set_width(): Using default for " <<
		  op_ << "." << endl;

	  case '+':
	  case '-':
	    left_->set_width(w);
	    right_->set_width(w);
	    expr_width(w);
	    break;
      }
}

NetEConst::~NetEConst()
{
}

void NetEConst::set_width(unsigned w)
{
      assert(w <= value_.len());
      expr_width(w);
}

NetESignal::~NetESignal()
{
}

void NetESignal::set_width(unsigned w)
{
      assert(w == sig_->pin_count());
      expr_width(w);
}

NetEUnary::~NetEUnary()
{
}

void NetEUnary::set_width(unsigned w)
{
      expr_->set_width(w);
      expr_width(w);
}

string Design::get_flag(const string&key) const
{
      map<string,string>::const_iterator tmp = flags_.find(key);
      if (tmp == flags_.end())
	    return "";
      else
	    return (*tmp).second;
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

void Design::clear_signal_marks()
{
      if (signals_ == 0)
	    return;

      NetNet*cur = signals_;
      do {
	    cur->set_mark(false);
	    cur = cur->sig_next_;
      } while (cur != signals_);
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

NetNet* Design::find_signal(bool (*func)(const NetNet*))
{
      if (signals_ == 0)
	    return 0;

      NetNet*cur = signals_->sig_next_;
      do {
	    if ((cur->test_mark() == false) && func(cur))
		  return cur;

	    cur = cur->sig_next_;
      } while (cur != signals_->sig_next_);

      return 0;
}

/*
 * $Log: netlist.cc,v $
 * Revision 1.8  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.7  1998/11/18 04:25:22  steve
 *  Add -f flags for generic flag key/values.
 *
 * Revision 1.6  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 * Revision 1.5  1998/11/13 06:23:17  steve
 *  Introduce netlist optimizations with the
 *  cprop function to do constant propogation.
 *
 * Revision 1.4  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.3  1998/11/07 19:17:10  steve
 *  Calculate expression widths at elaboration time.
 *
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

