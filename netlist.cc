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
#ident "$Id: netlist.cc,v 1.14 1998/12/18 05:16:25 steve Exp $"
#endif

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"

ostream& operator<< (ostream&o, NetNet::Type t)
{
      switch (t) {
	  case NetNet::IMPLICIT:
	    o << "wire /*implicit*/";
	    break;
	  case NetNet::REG:
	    o << "reg";
	    break;
	  case NetNet::SUPPLY0:
	    o << "supply0";
	    break;
	  case NetNet::SUPPLY1:
	    o << "supply1";
	    break;
	  case NetNet::TRI:
	    o << "tri";
	    break;
	  case NetNet::TRI0:
	    o << "tri0";
	    break;
	  case NetNet::TRI1:
	    o << "tri1";
	    break;
	  case NetNet::TRIAND:
	    o << "triand";
	    break;
	  case NetNet::TRIOR:
	    o << "trior";
	    break;
	  case NetNet::WAND:
	    o << "wand";
	    break;
	  case NetNet::WOR:
	    o << "wor";
	    break;
	  case NetNet::WIRE:
	    o << "wire";
	    break;
      }
      return o;
}


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

unsigned count_inputs(const NetObj::Link&pin)
{
      unsigned count = (pin.get_dir() == NetObj::Link::INPUT)? 1 : 0;
      const NetObj*cur;
      unsigned cpin;
      pin.next_link(cur, cpin);
      while (cur->pin(cpin) != pin) {
	    if (cur->pin(cpin).get_dir() == NetObj::Link::INPUT)
		  count += 1;
	    cur->pin(cpin).next_link(cur, cpin);
      }

      return count;
}

unsigned count_outputs(const NetObj::Link&pin)
{
      unsigned count = (pin.get_dir() == NetObj::Link::OUTPUT)? 1 : 0;
      const NetObj*cur;
      unsigned cpin;
      pin.next_link(cur, cpin);
      while (cur->pin(cpin) != pin) {
	    if (cur->pin(cpin).get_dir() == NetObj::Link::OUTPUT)
		  count += 1;
	    cur->pin(cpin).next_link(cur, cpin);
      }

      return count;
}

unsigned count_signals(const NetObj::Link&pin)
{
      unsigned count = 0;
      if (dynamic_cast<const NetNet*>(pin.get_obj()))
	    count += 1;

      const NetObj*cur;
      unsigned cpin;
      pin.next_link(cur, cpin);
      while (cur->pin(cpin) != pin) {
	    if (dynamic_cast<const NetNet*>(cur))
		  count += 1;

	    cur->pin(cpin).next_link(cur, cpin);
      }

      return count;
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

void NetObj::attribute(const string&key, const string&value)
{
      attributes_[key] = value;
}

bool NetObj::has_compat_attributes(const NetObj&that) const
{
      map<string,string>::const_iterator idx;
      for (idx = that.attributes_.begin()
		 ; idx != that.attributes_.end() ;  idx ++) {
	   map<string,string>::const_iterator cur;
	   cur = attributes_.find((*idx).first);

	   if (cur == attributes_.end())
		 return false;
	   if ((*cur).second != (*idx).second)
		 return false;
      }

      return true;
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

NetLogic::NetLogic(const string&n, unsigned pins, TYPE t)
: NetNode(n, pins), type_(t)
{
      pin(0).set_dir(Link::OUTPUT);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1)
	    pin(idx).set_dir(Link::INPUT);
}

NetUDP::NetUDP(const string&n, unsigned pins, bool sequ)
: NetNode(n, pins), sequential_(sequ), init_('x')
{
      pin(0).set_dir(Link::OUTPUT);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1)
	    pin(idx).set_dir(Link::INPUT);

}

NetUDP::state_t_* NetUDP::find_state_(const string&str)
{
      map<string,state_t_*>::iterator cur = fsm_.find(str);
      if (cur != fsm_.end())
	    return (*cur).second;

      state_t_*st = fsm_[str];
      if (st == 0) {
	    st = new state_t_(pin_count());
	    st->out = str[0];
	    fsm_[str] = st;
      }

      return st;
}

/*
 * This method takes the input string, which contains exactly one
 * edge, and connects it to the correct output state. The output state
 * will be generated if needed, and the value compared.
 */
bool NetUDP::set_sequ_(const string&input, char output)
{
      if (output == '-')
	    output = input[0];

      string frm = input;
      string to  = input;
      to[0] = output;

      unsigned edge = frm.find_first_not_of("01x");
      assert(frm.find_last_not_of("01x") == edge);

      switch (input[edge]) {
	  case 'r':
	    frm[edge] = '0';
	    to[edge] = '1';
	    break;
	  case 'R':
	    frm[edge] = 'x';
	    to[edge] = '1';
	    break;
	  case 'f':
	    frm[edge] = '1';
	    to[edge] = '0';
	    break;
	  case 'F':
	    frm[edge] = 'x';
	    to[edge] = '0';
	    break;
	  case 'P':
	    frm[edge] = '0';
	    to[edge] = 'x';
	    break;
	  case 'N':
	    frm[edge] = '1';
	    to[edge] = 'x';
	    break;
	  default:
	    assert(0);
      }

      state_t_*sfrm = find_state_(frm);
      state_t_*sto  = find_state_(to);

      switch (to[edge]) {
	  case '0':
	      // Notice that I might have caught this edge already
	    if (sfrm->pins[edge].zer != sto) {
		  assert(sfrm->pins[edge].zer == 0);
		  sfrm->pins[edge].zer = sto;
	    }
	    break;
	  case '1':
	      // Notice that I might have caught this edge already
	    if (sfrm->pins[edge].one != sto) {
		    assert(sfrm->pins[edge].one == 0);
		    sfrm->pins[edge].one = sto;
	    }
	    break;
	  case 'x':
	      // Notice that I might have caught this edge already
	    if (sfrm->pins[edge].xxx != sto) {
		    assert(sfrm->pins[edge].xxx == 0);
		    sfrm->pins[edge].xxx = sto;
	    }
	    break;
      }

      return true;
}

bool NetUDP::sequ_glob_(string input, char output)
{
      for (unsigned idx = 0 ;  idx < input.length() ;  idx += 1)
	    switch (input[idx]) {
		case '0':
		case '1':
		case 'x':
		case 'r':
		case 'R':
		case 'f':
		case 'F':
		case 'P':
		case 'N':
		  break;

		case '?': // Iterate over all the levels
		  input[idx] = '0';
		  sequ_glob_(input, output);
		  input[idx] = '1';
		  sequ_glob_(input, output);
		  input[idx] = 'x';
		  sequ_glob_(input, output);
		  return true;

		case 'n': // Iterate over (n) edges
		  input[idx] = 'f';
		  sequ_glob_(input, output);
		  input[idx] = 'F';
		  sequ_glob_(input, output);
		  input[idx] = 'N';
		  sequ_glob_(input, output);
		  return true;

		case 'p': // Iterate over (p) edges
		  input[idx] = 'r';
		  sequ_glob_(input, output);
		  input[idx] = 'R';
		  sequ_glob_(input, output);
		  input[idx] = 'P';
		  sequ_glob_(input, output);
		  return true;

		case '_': // Iterate over (?0) edges
		  input[idx] = 'f';
		  sequ_glob_(input, output);
		  input[idx] = 'F';
		  sequ_glob_(input, output);
		  return true;

		case '*': // Iterate over all the edges
		  input[idx] = 'r';
		  sequ_glob_(input, output);
		  input[idx] = 'R';
		  sequ_glob_(input, output);
		  input[idx] = 'f';
		  sequ_glob_(input, output);
		  input[idx] = 'F';
		  sequ_glob_(input, output);
		  input[idx] = 'P';
		  sequ_glob_(input, output);
		  input[idx] = 'N';
		  sequ_glob_(input, output);
		  return true;

		default:
		  assert(0);
	    }

      return set_sequ_(input, output);
}

bool NetUDP::set_table(const string&input, char output)
{
      assert((output == '0') || (output == '1') || (sequential_ &&
						    (output == '-')));

      if (sequential_) {
	    assert(input.length() == pin_count());
	      /* XXXX Need to check to make sure that the input vector
		 contains a legal combination of characters. */
	    return sequ_glob_(input, output);

      } else {
	    assert(input.length() == (pin_count()-1));
	      /* XXXX Need to check to make sure that the input vector
		 contains a legal combination of characters. In
		 combinational UDPs, only 0, 1 and x are allowed. */
	    assert(0);

	    return true;
      }
}

void NetUDP::cleanup_table()
{
      for (FSM_::iterator idx = fsm_.begin() ;  idx != fsm_.end() ; idx++) {
	    string str = (*idx).first;
	    state_t_*st = (*idx).second;
	    assert(str[0] == st->out);

	    for (unsigned pin = 0 ;  pin < pin_count() ;  pin += 1) {
		  if (st->pins[pin].zer && st->pins[pin].zer->out == 'x')
			st->pins[pin].zer = 0;
		  if (st->pins[pin].one && st->pins[pin].one->out == 'x')
			st->pins[pin].one = 0;
		  if (st->pins[pin].xxx && st->pins[pin].xxx->out == 'x')
			st->pins[pin].xxx = 0;
	    }
      }

      for (FSM_::iterator idx = fsm_.begin() ;  idx != fsm_.end() ; ) {
	    FSM_::iterator cur = idx;
	    idx ++;

	    state_t_*st = (*cur).second;

	    if (st->out != 'x')
		  continue;

	    for (unsigned pin = 0 ;  pin < pin_count() ;  pin += 1) {
		  if (st->pins[pin].zer)
			goto break_label;
		  if (st->pins[pin].one)
			goto break_label;
		  if (st->pins[pin].xxx)
			goto break_label;
	    }

		    //delete st;
	    fsm_.erase(cur);

      break_label:;
      }
}

char NetUDP::table_lookup(const string&from, char to, unsigned pin) const
{
      assert(pin <= pin_count());
      assert(from.length() == pin_count());
      FSM_::const_iterator idx = fsm_.find(from);
      if (idx == fsm_.end())
	    return 'x';

      state_t_*next;
      switch (to) {
	  case '0':
	    next = (*idx).second->pins[pin].zer;
	    break;
	  case '1':
	    next = (*idx).second->pins[pin].one;
	    break;
	  case 'x':
	    next = (*idx).second->pins[pin].xxx;
	    break;
	  default:
	    assert(0);
	    next = 0;
      }

      return next? next->out : 'x';
}

void NetUDP::set_initial(char val)
{
      assert(sequential_);
      assert((val == '0') || (val == '1') || (val == 'x'));
      init_ = val;
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
 * Revision 1.14  1998/12/18 05:16:25  steve
 *  Parse more UDP input edge descriptions.
 *
 * Revision 1.13  1998/12/17 23:54:58  steve
 *  VVM support for small sequential UDP objects.
 *
 * Revision 1.12  1998/12/14 02:01:35  steve
 *  Fully elaborate Sequential UDP behavior.
 *
 * Revision 1.11  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.10  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.9  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
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

