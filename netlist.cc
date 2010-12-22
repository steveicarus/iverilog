/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include <iostream>

# include  <cassert>
# include  <typeinfo>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"


ostream& operator<< (ostream&o, NetNet::Type t)
{
      switch (t) {
	  case NetNet::NONE:
	    o << "net_none";
	    break;
	  case NetNet::IMPLICIT:
	    o << "wire /*implicit*/";
	    break;
	  case NetNet::IMPLICIT_REG:
	    o << "reg /*implicit*/";
	    break;
	  case NetNet::INTEGER:
	    o << "integer";
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


unsigned count_inputs(const Link&pin)
{
      unsigned count = 0;

      const Nexus*nex = pin.nexus();
      for (const Link*clnk = nex->first_nlink()
		 ; clnk ; clnk = clnk->next_nlink()) {
	    const NetObj*cur;
	    unsigned cpin;
	    clnk->cur_link(cur, cpin);
	    if (cur->pin(cpin).get_dir() == Link::INPUT)
		  count += 1;
      }

      return count;
}

unsigned count_outputs(const Link&pin)
{
      unsigned count = 0;

      const Nexus*nex = pin.nexus();
      for (const Link*clnk = nex->first_nlink()
		 ; clnk ; clnk = clnk->next_nlink()) {
	    const NetObj*cur;
	    unsigned cpin;
	    clnk->cur_link(cur, cpin);
	    if (cur->pin(cpin).get_dir() == Link::OUTPUT)
		  count += 1;
      }

      return count;
}

unsigned count_signals(const Link&pin)
{
      unsigned count = 0;

      const Nexus*nex = pin.nexus();
      for (const Link*clnk = nex->first_nlink()
		 ; clnk ; clnk = clnk->next_nlink()) {
	    const NetObj*cur;
	    unsigned cpin;
	    clnk->cur_link(cur, cpin);
	    if (dynamic_cast<const NetNet*>(cur))
		  count += 1;
      }

      return count;
}

const NetNet* find_link_signal(const NetObj*net, unsigned pin, unsigned&bidx)
{
      const Nexus*nex = net->pin(pin).nexus();

      for (const Link*clnk = nex->first_nlink()
		 ; clnk ; clnk = clnk->next_nlink()) {

	    const NetObj*cur;
	    unsigned cpin;
	    clnk->cur_link(cur, cpin);

	    const NetNet*sig = dynamic_cast<const NetNet*>(cur);
	    if (sig) {
		  bidx = cpin;
		  return sig;
	    }
      }

      return 0;
}

Link* find_next_output(Link*lnk)
{
      Link*cur = lnk->next_nlink();
      while (cur != lnk) {
	    if (cur->get_dir() == Link::OUTPUT)
		  return cur;

	    cur = cur->next_nlink();
	    if (cur == 0)
		  cur = lnk->nexus()->first_nlink();
      }

      return 0;
}

NetObj::NetObj(NetScope*s, perm_string n, unsigned np)
: scope_(s), name_(n), npins_(np), delay1_(0), delay2_(0), delay3_(0)
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

NetScope* NetObj::scope()
{
      return scope_;
}

const NetScope* NetObj::scope() const
{
      return scope_;
}

Link& NetObj::pin(unsigned idx)
{
      if (idx >= npins_) {
	    cerr << get_line() << ": internal error: pin("<<idx<<")"
		 << " out of bounds("<<npins_<<")" << endl;
	    cerr << get_line() << ":               : typeid="
		 << typeid(*this).name() << endl;
      }

      assert(idx < npins_);
      return pins_[idx];
}

const Link& NetObj::pin(unsigned idx) const
{
      assert(idx < npins_);
      return pins_[idx];
}

NetNode::NetNode(NetScope*s, perm_string n, unsigned npins)
: NetObj(s, n, npins), node_next_(0), node_prev_(0), design_(0)
{
}

NetNode::~NetNode()
{
      if (design_)
	    design_->del_node(this);
}

NetNet::NetNet(NetScope*s, perm_string n, Type t, unsigned npins)
: NetObj(s, n, npins), sig_next_(0), sig_prev_(0),
    type_(t), port_type_(NOT_A_PORT), signed_(false), msb_(npins-1), lsb_(0),
    local_flag_(false), eref_count_(0), lref_count_(0), mref_(0)
{
      assert(s);

      release_list_ = 0;

      verinum::V init_value = verinum::Vz;
      Link::DIR dir = Link::PASSIVE;

      switch (t) {
	  case REG:
	  case INTEGER:
	  case IMPLICIT_REG:
	    init_value = verinum::Vx;
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY0:
	    init_value = verinum::V0;
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY1:
	    init_value = verinum::V1;
	    dir = Link::OUTPUT;
	    break;
	  default:
	    break;
      }

      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    pin(idx).set_name(perm_string::literal("P"), idx);
	    pin(idx).set_dir(dir);
	    pin(idx).set_init(init_value);
      }

      scope()->add_signal(this);
}

NetNet::NetNet(NetScope*s, perm_string n, Type t, long ms, long ls)
: NetObj(s, n, ((ms>ls)?ms-ls:ls-ms) + 1),
    sig_next_(0), sig_prev_(0), type_(t),
    port_type_(NOT_A_PORT), signed_(false), msb_(ms), lsb_(ls),
    local_flag_(false), eref_count_(0), lref_count_(0), mref_(0)
{
      assert(s);

      release_list_ = 0;

      verinum::V init_value = verinum::Vz;
      Link::DIR dir = Link::PASSIVE;

      switch (t) {
	  case REG:
	  case IMPLICIT_REG:
	    init_value = verinum::Vx;
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY0:
	    init_value = verinum::V0;
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY1:
	    init_value = verinum::V1;
	    dir = Link::OUTPUT;
	    break;
	  default:
	    break;
      }

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_name(perm_string::literal("P"), idx);
	    pin(idx).set_dir(dir);
	    pin(idx).set_init(init_value);
      }

      s->add_signal(this);
}

NetNet::~NetNet()
{
      if (eref_count_ > 0) {
	    cerr << get_line() << ": internal error: attempt to delete "
		 << "signal ``" << name() << "'' which has "
		 << "expression references." << endl;
	    dump_net(cerr, 4);
      }
      assert(eref_count_ == 0);
      if (lref_count_ > 0) {
	    cerr << get_line() << ": internal error: attempt to delete "
		 << "signal ``" << name() << "'' which has "
		 << "assign references." << endl;
	    dump_net(cerr, 4);
      }
      assert(lref_count_ == 0);
      if (mref_ != 0) {
	    cerr << get_line() << ": internal error: attempt to delete "
		 << "signal ``" << name() << "'' which has "
		 << "memory explode references." << endl;
	    dump_net(cerr, 4);
      }
      assert(mref_ == 0);

      if (scope())
	    scope()->rem_signal(this);

	/* Detach me from all the NetRelease objects that refer to me. */
      while (release_list_) {
	    NetRelease*tmp = release_list_;
	    release_list_ = tmp->release_next_;
	    assert(tmp->lval_ == this);
	    tmp->lval_ = 0;
	    tmp->release_next_ = 0;
      }
}

NetNet::Type NetNet::type() const
{
      return type_;
}

void NetNet::type(NetNet::Type t)
{
      if (type_ == t)
	    return;

      Link::DIR dir = Link::PASSIVE;
      switch (t) {
	  case REG:
	  case IMPLICIT_REG:
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY0:
	    dir = Link::OUTPUT;
	    break;
	  case SUPPLY1:
	    dir = Link::OUTPUT;
	    break;
	  default:
	    break;
      }

      type_ = t;
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(dir);
      }

}


NetNet::PortType NetNet::port_type() const
{
      return port_type_;
}

void NetNet::port_type(NetNet::PortType t)
{
      port_type_ = t;
}

bool NetNet::get_signed() const
{
      return signed_;
}

void NetNet::set_signed(bool flag)
{
      signed_ = flag;
}

bool NetNet::get_isint() const
{
      return isint_;
}

void NetNet::set_isint(bool flag)
{
      isint_ = flag;
}

long NetNet::lsb() const
{
      return lsb_;
}

long NetNet::msb() const
{
      return msb_;
}

bool NetNet::sb_is_valid(long sb) const
{
      if (msb_ >= lsb_)
	    return (sb <= msb_) && (sb >= lsb_);
      else
	    return (sb <= lsb_) && (sb >= msb_);
}

unsigned NetNet::sb_to_idx(long sb) const
{
      if (msb_ >= lsb_)
	    return sb - lsb_;
      else
	    return lsb_ - sb;
}

void NetNet::incr_eref()
{
      eref_count_ += 1;
}

void NetNet::decr_eref()
{
      assert(eref_count_ > 0);
      eref_count_ -= 1;
}

unsigned NetNet::peek_eref() const
{
      return eref_count_;
}

void NetNet::incr_lref()
{
      lref_count_ += 1;
}

void NetNet::decr_lref()
{
      assert(lref_count_ > 0);
      lref_count_ -= 1;
}

unsigned NetNet::peek_lref() const
{
      return lref_count_;
}

unsigned NetNet::get_refs() const
{
      return lref_count_ + eref_count_;
}

void NetNet::mref(NetMemory*mem)
{
      assert(mref_ == 0);
      assert(mem != 0);
      mref_ = mem;
}

NetMemory*NetNet::mref()
{
      return mref_;
}

const NetMemory* NetNet::mref() const
{
      return mref_;
}


NetSubnet::NetSubnet(NetNet*sig, unsigned off, unsigned wid)
: NetNet(sig->scope(), sig->scope()->local_symbol(), sig->type(), wid)
{
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
	    connect(sig->pin(idx+off), pin(idx));

      port_type(sig->port_type());
      local_flag(true);
      set_line(*sig);
}

NetProc::NetProc()
: next_(0)
{
}

NetProc::~NetProc()
{
}

NetProcTop::NetProcTop(NetScope*s, Type t, NetProc*st)
: type_(t), statement_(st), scope_(s)
{
}

NetProcTop::~NetProcTop()
{
      delete statement_;
}

NetProc* NetProcTop::statement()
{
      return statement_;
}

const NetProc* NetProcTop::statement() const
{
      return statement_;
}

NetScope* NetProcTop::scope()
{
      return scope_;
}

const NetScope* NetProcTop::scope() const
{
      return scope_;
}

/*
 * The NetFF class represents an LPM_FF device. The pinout is assigned
 * like so:
 *    0  -- Clock
 *    1  -- Enable
 *    2  -- Aset
 *    3  -- Aclr
 *    4  -- Sset
 *    5  -- Sclr
 *
 *    6  -- Data[0]
 *    7  -- Q[0]
 *     ...
 */

NetFF::NetFF(NetScope*s, perm_string n, unsigned wid)
: NetNode(s, n, 6 + 2*wid)
{
      demux_ = 0;

      pin_Clock().set_dir(Link::INPUT);
      pin_Clock().set_name(perm_string::literal("Clock"), 0);
      pin_Enable().set_dir(Link::INPUT);
      pin_Enable().set_name(perm_string::literal("Enable"), 0);
      pin_Aset().set_dir(Link::INPUT);
      pin_Aset().set_name(perm_string::literal("Aset"), 0);
      pin_Aclr().set_dir(Link::INPUT);
      pin_Aclr().set_name(perm_string::literal("Aclr"), 0);
      pin_Sset().set_dir(Link::INPUT);
      pin_Sset().set_name(perm_string::literal("Sset"), 0);
      pin_Sclr().set_dir(Link::INPUT);
      pin_Sclr().set_name(perm_string::literal("Sclr"), 0);
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT);
	    pin_Data(idx).set_name(perm_string::literal("Data"), idx);
	    pin_Q(idx).set_dir(Link::OUTPUT);
	    pin_Q(idx).set_name(perm_string::literal("Q"), idx);
      }
}

NetFF::~NetFF()
{
}

unsigned NetFF::width() const
{
      return (pin_count() - 6) / 2;
}

Link& NetFF::pin_Clock()
{
      return pin(0);
}

const Link& NetFF::pin_Clock() const
{
      return pin(0);
}

Link& NetFF::pin_Enable()
{
      return pin(1);
}

const Link& NetFF::pin_Enable() const
{
      return pin(1);
}

Link& NetFF::pin_Aset()
{
      return pin(2);
}

const Link& NetFF::pin_Aset() const
{
      return pin(2);
}

Link& NetFF::pin_Aclr()
{
      return pin(3);
}

const Link& NetFF::pin_Aclr() const
{
      return pin(3);
}

Link& NetFF::pin_Sset()
{
      return pin(4);
}

const Link& NetFF::pin_Sset() const
{
      return pin(4);
}

Link& NetFF::pin_Sclr()
{
      return pin(5);
}

const Link& NetFF::pin_Sclr() const
{
      return pin(5);
}

Link& NetFF::pin_Data(unsigned w)
{
      unsigned pn = 6 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

const Link& NetFF::pin_Data(unsigned w) const
{
      unsigned pn = 6 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

Link& NetFF::pin_Q(unsigned w)
{
      unsigned pn = 7 + w*2;
      assert(pn < pin_count());
      return pin(pn);
}

const Link& NetFF::pin_Q(unsigned w) const
{
      unsigned pn = 7 + w*2;
      assert(pn < pin_count());
      return pin(pn);
}

void NetFF::aset_value(const verinum&val)
{
      aset_value_ = val;
}

const verinum& NetFF::aset_value() const
{
      return aset_value_;
}

void NetFF::sset_value(const verinum&val)
{
      sset_value_ = val;
}

const verinum& NetFF::sset_value() const
{
      return sset_value_;
}

unsigned NetDecode::width() const
{
      return width_;
}

unsigned NetDecode::awidth() const
{
      return pin_count();
}

/*
 * The NetLatch class represents an LPM_LATCH device. The pinout is assigned
 * like so:
 *    0  -- Clock (Gate)
 *    1  -- Aset
 *    2  -- Aclr
 *
 *    3  -- Data[0]
 *    4  -- Q[0]
 *     ...
 */
NetLatch::NetLatch(NetScope*s, perm_string n, unsigned wid)
: NetNode(s, n, 3 + 2*wid)
{
      pin_Clock().set_dir(Link::INPUT);
      pin_Clock().set_name(perm_string::literal("Clock"), 0);
      pin_Aset().set_dir(Link::INPUT);
      pin_Aset().set_name(perm_string::literal("Aset"), 0);
      pin_Aclr().set_dir(Link::INPUT);
      pin_Aclr().set_name(perm_string::literal("Aclr"), 0);
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT);
	    pin_Data(idx).set_name(perm_string::literal("Data"), idx);
	    pin_Q(idx).set_dir(Link::OUTPUT);
	    pin_Q(idx).set_name(perm_string::literal("Q"), idx);
      }
}

NetLatch::~NetLatch()
{
}

unsigned NetLatch::width() const
{
      return (pin_count() - 3) / 2;
}

Link& NetLatch::pin_Clock()
{
      return pin(0);
}

const Link& NetLatch::pin_Clock() const
{
      return pin(0);
}

Link& NetLatch::pin_Aset()
{
      return pin(1);
}

const Link& NetLatch::pin_Aset() const
{
      return pin(1);
}

Link& NetLatch::pin_Aclr()
{
      return pin(2);
}

const Link& NetLatch::pin_Aclr() const
{
      return pin(2);
}

Link& NetLatch::pin_Data(unsigned w)
{
      unsigned pn = 3 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

const Link& NetLatch::pin_Data(unsigned w) const
{
      unsigned pn = 3 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

Link& NetLatch::pin_Q(unsigned w)
{
      unsigned pn = 4 + w*2;
      assert(pn < pin_count());
      return pin(pn);
}

const Link& NetLatch::pin_Q(unsigned w) const
{
      unsigned pn = 4 + w*2;
      assert(pn < pin_count());
      return pin(pn);
}

void NetLatch::aset_value(const verinum&val)
{
      aset_value_ = val;
}

const verinum& NetLatch::aset_value() const
{
      return aset_value_;
}

NetDemux::NetDemux(NetScope*s, perm_string name,
		   unsigned bus_width, unsigned address_width, unsigned size)
: NetNode(s, name, bus_width*2+address_width+bus_width/size)
{
      width_ = bus_width;
      awidth_ = address_width;
      size_ = size;

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin_Q(idx).set_dir(Link::OUTPUT);
	    pin_Q(idx).set_name(perm_string::literal("Q"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT);
	    pin_Data(idx).set_name(perm_string::literal("Data"), idx);
      }
      for (unsigned idx = 0 ;  idx < awidth_ ;  idx += 1) {
	    pin_Address(idx).set_dir(Link::INPUT);
	    pin_Address(idx).set_name(perm_string::literal("Address"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_/size_ ;  idx += 1) {
	    pin_WriteData(idx).set_dir(Link::INPUT);
	    pin_WriteData(idx).set_name(perm_string::literal("Writedata"), idx);
      }
}

NetDemux::~NetDemux()
{
}

unsigned NetDemux::width() const
{
      return width_;
}

unsigned NetDemux::awidth() const
{
      return awidth_;
}

unsigned NetDemux::size() const
{
      return size_;
}

Link& NetDemux::pin_Q(unsigned idx)
{
      assert(idx < width_);
      return pin(idx);
}

const Link& NetDemux::pin_Q(unsigned idx) const
{
      assert(idx < width_);
      return pin(idx);
}

Link& NetDemux::pin_Data(unsigned idx)
{
      assert(idx < width_);
      return pin(width_+idx);
}

const Link& NetDemux::pin_Data(unsigned idx) const
{
      assert(idx < width_);
      return pin(width_+idx);
}

Link& NetDemux::pin_Address(unsigned idx)
{
      assert(idx < awidth_);
      return pin(width_+width_+idx);
}

const Link& NetDemux::pin_Address(unsigned idx) const
{
      assert(idx < awidth_);
      return pin(width_+width_+idx);
}

Link& NetDemux::pin_WriteData(unsigned idx)
{
      assert(idx < width_/size_);
      return pin(width_+width_+awidth_+idx);
}

const Link& NetDemux::pin_WriteData(unsigned idx) const
{
      assert(idx < width_/size_);
      return pin(width_+width_+awidth_+idx);
}

NetDecode* NetFF::get_demux()
{
      return demux_;
}

const NetDecode* NetFF::get_demux() const
{
      return demux_;
}

/*
 * The NetAddSub class represents an LPM_ADD_SUB device. The pinout is
 * assigned like so:
 *    0  -- Add_Sub
 *    1  -- Aclr
 *    2  -- Clock
 *    3  -- Cin
 *    4  -- Cout
 *    5  -- Overflow
 *    6  -- DataA[0]
 *    7  -- DataB[0]
 *    8  -- Result[0]
 */
NetAddSub::NetAddSub(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, w*3+6)
{
      pin(0).set_dir(Link::INPUT); pin(0).set_name(
				 perm_string::literal("Add_Sub"), 0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(
				 perm_string::literal("Aclr"), 0);
      pin(2).set_dir(Link::INPUT); pin(2).set_name(
				 perm_string::literal("Clock"), 0);
      pin(3).set_dir(Link::INPUT); pin(3).set_name(
				 perm_string::literal("Cin"), 0);
      pin(4).set_dir(Link::OUTPUT); pin(4).set_name(
				 perm_string::literal("Cout"), 0);
      pin(5).set_dir(Link::OUTPUT); pin(5).set_name(
				 perm_string::literal("Overflow"), 0);
      for (unsigned idx = 0 ;  idx < w ;  idx += 1) {
	    pin_DataA(idx).set_dir(Link::INPUT);
	    pin_DataB(idx).set_dir(Link::INPUT);
	    pin_Result(idx).set_dir(Link::OUTPUT);
	    pin_DataA(idx).set_name(perm_string::literal("DataA"), idx);
	    pin_DataB(idx).set_name(perm_string::literal("DataB"), idx);
	    pin_Result(idx).set_name(perm_string::literal("Result"), idx);
      }
}

NetAddSub::~NetAddSub()
{
}

unsigned NetAddSub::width()const
{
      return (pin_count() - 6) / 3;
}

Link& NetAddSub::pin_Cout()
{
      return pin(4);
}

const Link& NetAddSub::pin_Cout() const
{
      return pin(4);
}

Link& NetAddSub::pin_DataA(unsigned idx)
{
      idx = 6 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const Link& NetAddSub::pin_DataA(unsigned idx) const
{
      idx = 6 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

Link& NetAddSub::pin_DataB(unsigned idx)
{
      idx = 7 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const Link& NetAddSub::pin_DataB(unsigned idx) const
{
      idx = 7 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

Link& NetAddSub::pin_Result(unsigned idx)
{
      idx = 8 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const Link& NetAddSub::pin_Result(unsigned idx) const
{
      idx = 8 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

/*
 * The pinout for the NetCLShift is:
 *    0   -- Direction
 *    1   -- Underflow
 *    2   -- Overflow
 *    3   -- Data(0)
 *    3+W -- Result(0)
 *    3+2W -- Distance(0)
 */
NetCLShift::NetCLShift(NetScope*s, perm_string n,
		       unsigned width, unsigned width_dist,
		       bool right_flag, bool signed_flag)
: NetNode(s, n, 3+2*width+width_dist),
  width_(width), width_dist_(width_dist),
    right_flag_(right_flag), signed_flag_(signed_flag)
{
      pin(0).set_dir(Link::INPUT); pin(0).set_name(
				     perm_string::literal("Direction"), 0);
      pin(1).set_dir(Link::OUTPUT); pin(1).set_name(
				     perm_string::literal("Underflow"), 0);
      pin(2).set_dir(Link::OUTPUT); pin(2).set_name(
				     perm_string::literal("Overflow"), 0);

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin(3+idx).set_dir(Link::INPUT);
	    pin(3+idx).set_name(perm_string::literal("Data"), idx);

	    pin(3+width_+idx).set_dir(Link::OUTPUT);
	    pin(3+width_+idx).set_name(perm_string::literal("Result"), idx);
      }

      for (unsigned idx = 0 ;  idx < width_dist_ ;  idx += 1) {
	    pin(3+2*width_+idx).set_dir(Link::INPUT);
	    pin(3+2*width_+idx).set_name(perm_string::literal("Distance"), idx);
      }
}

NetCLShift::~NetCLShift()
{
}

unsigned NetCLShift::width() const
{
      return width_;
}

unsigned NetCLShift::width_dist() const
{
      return width_dist_;
}

bool NetCLShift::right_flag() const
{
      return right_flag_;
}

bool NetCLShift::signed_flag() const
{
      return signed_flag_;
}

#if 0
Link& NetCLShift::pin_Direction()
{
      return pin(0);
}

const Link& NetCLShift::pin_Direction() const
{
      return pin(0);
}
#endif

Link& NetCLShift::pin_Underflow()
{
      return pin(1);
}

const Link& NetCLShift::pin_Underflow() const
{
      return pin(1);
}

Link& NetCLShift::pin_Overflow()
{
      return pin(2);
}

const Link& NetCLShift::pin_Overflow() const
{
      return pin(2);
}

Link& NetCLShift::pin_Data(unsigned idx)
{
      assert(idx < width_);
      return pin(3+idx);
}

const Link& NetCLShift::pin_Data(unsigned idx) const
{
      assert(idx < width_);
      return pin(3+idx);
}

Link& NetCLShift::pin_Result(unsigned idx)
{
      assert(idx < width_);
      return pin(3+width_+idx);
}

const Link& NetCLShift::pin_Result(unsigned idx) const
{
      assert(idx < width_);
      return pin(3+width_+idx);
}

Link& NetCLShift::pin_Distance(unsigned idx)
{
      assert(idx < width_dist_);
      return pin(3+2*width_+idx);
}

const Link& NetCLShift::pin_Distance(unsigned idx) const
{
      assert(idx < width_dist_);
      return pin(3+2*width_+idx);
}

NetCompare::NetCompare(NetScope*s, perm_string n, unsigned wi)
: NetNode(s, n, 8+2*wi), width_(wi)
{
      signed_flag_ = false;
      pin(0).set_dir(Link::INPUT); pin(0).set_name(
				perm_string::literal("Aclr"));
      pin(1).set_dir(Link::INPUT); pin(1).set_name(
				perm_string::literal("Clock"));
      pin(2).set_dir(Link::OUTPUT); pin(2).set_name(
				perm_string::literal("AGB"));
      pin(3).set_dir(Link::OUTPUT); pin(3).set_name(
				perm_string::literal("AGEB"));
      pin(4).set_dir(Link::OUTPUT); pin(4).set_name(
				perm_string::literal("AEB"));
      pin(5).set_dir(Link::OUTPUT); pin(5).set_name(
				perm_string::literal("ANEB"));
      pin(6).set_dir(Link::OUTPUT); pin(6).set_name(
				perm_string::literal("ALB"));
      pin(7).set_dir(Link::OUTPUT); pin(7).set_name(
				perm_string::literal("ALEB"));
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin(8+idx).set_dir(Link::INPUT);
	    pin(8+idx).set_name(perm_string::literal("DataA"), idx);
	    pin(8+width_+idx).set_dir(Link::INPUT);
	    pin(8+width_+idx).set_name(perm_string::literal("DataB"), idx);
      }
}

NetCompare::~NetCompare()
{
}

unsigned NetCompare::width() const
{
      return width_;
}

bool NetCompare::get_signed() const
{
      return signed_flag_;
}

void NetCompare::set_signed(bool flag)
{
      signed_flag_ = flag;
}

Link& NetCompare::pin_Aclr()
{
      return pin(0);
}

const Link& NetCompare::pin_Aclr() const
{
      return pin(0);
}

Link& NetCompare::pin_Clock()
{
      return pin(1);
}

const Link& NetCompare::pin_Clock() const
{
      return pin(1);
}

Link& NetCompare::pin_AGB()
{
      return pin(2);
}

const Link& NetCompare::pin_AGB() const
{
      return pin(2);
}

Link& NetCompare::pin_AGEB()
{
      return pin(3);
}

const Link& NetCompare::pin_AGEB() const
{
      return pin(3);
}

Link& NetCompare::pin_AEB()
{
      return pin(4);
}

const Link& NetCompare::pin_AEB() const
{
      return pin(4);
}

Link& NetCompare::pin_ANEB()
{
      return pin(5);
}

const Link& NetCompare::pin_ANEB() const
{
      return pin(5);
}

Link& NetCompare::pin_ALB()
{
      return pin(6);
}

const Link& NetCompare::pin_ALB() const
{
      return pin(6);
}

Link& NetCompare::pin_ALEB()
{
      return pin(7);
}

const Link& NetCompare::pin_ALEB() const
{
      return pin(7);
}

Link& NetCompare::pin_DataA(unsigned idx)
{
      return pin(8+idx);
}

const Link& NetCompare::pin_DataA(unsigned idx) const
{
      return pin(8+idx);
}

Link& NetCompare::pin_DataB(unsigned idx)
{
      return pin(8+width_+idx);
}

const Link& NetCompare::pin_DataB(unsigned idx) const
{
      return pin(8+width_+idx);
}

NetDecode::NetDecode(NetScope*s, perm_string name, NetFF*mem,
		     unsigned awid, unsigned word_width)
: NetNode(s, name, awid)
{
      width_ = word_width;
      assert( mem->width() % width_ == 0 );

      ff_ = mem;
      ff_->demux_ = this;
      make_pins_(awid);
}

NetDecode::~NetDecode()
{
}

void NetDecode::make_pins_(unsigned awid)
{
      for (unsigned idx = 0 ;  idx < awid ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("Address"), idx);
      }
}

Link& NetDecode::pin_Address(unsigned idx)
{
      return pin(idx);
}

const Link& NetDecode::pin_Address(unsigned idx) const
{
      return pin(idx);
}

NetDivide::NetDivide(NetScope*sc, perm_string n, unsigned wr,
		     unsigned wa, unsigned wb)
: NetNode(sc, n, wr+wa+wb),
    width_r_(wr), width_a_(wa), width_b_(wb), signed_flag_(false)
{
      unsigned p = 0;
      for (unsigned idx = 0 ;  idx < width_r_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::OUTPUT);
	    pin(p).set_name(perm_string::literal("Result"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_a_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataA"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_b_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataB"), idx);
      }
}

NetDivide::~NetDivide()
{
}

unsigned NetDivide::width_r() const
{
      return width_r_;
}

unsigned NetDivide::width_a() const
{
      return width_a_;
}

unsigned NetDivide::width_b() const
{
      return width_b_;
}

void NetDivide::set_signed(bool flag)
{
      signed_flag_ = flag;
}

bool NetDivide::get_signed() const
{
      return signed_flag_;
}

Link& NetDivide::pin_Result(unsigned idx)
{
      assert(idx < width_r_);
      return pin(idx);
}

const Link& NetDivide::pin_Result(unsigned idx) const
{
      assert(idx < width_r_);
      return pin(idx);
}

Link& NetDivide::pin_DataA(unsigned idx)
{
      assert(idx < width_a_);
      return pin(idx+width_r_);
}

const Link& NetDivide::pin_DataA(unsigned idx) const
{
      assert(idx < width_a_);
      return pin(idx+width_r_);
}

Link& NetDivide::pin_DataB(unsigned idx)
{
      assert(idx < width_b_);
      return pin(idx+width_r_+width_a_);
}

const Link& NetDivide::pin_DataB(unsigned idx) const
{
      assert(idx < width_b_);
      return pin(idx+width_r_+width_a_);
}

NetMult::NetMult(NetScope*sc, perm_string n, unsigned wr,
		 unsigned wa, unsigned wb, unsigned ws)
: NetNode(sc, n, 2+wr+wa+wb+ws),
  signed_(false), width_r_(wr), width_a_(wa), width_b_(wb), width_s_(ws)
{
      pin(0).set_dir(Link::INPUT); pin(0).set_name(perm_string::literal("Aclr"), 0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(perm_string::literal("Clock"), 0);


      unsigned p = 2;
      for (unsigned idx = 0 ;  idx < width_r_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::OUTPUT);
	    pin(p).set_name(perm_string::literal("Result"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_a_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataA"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_b_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("DataB"), idx);
      }
      for (unsigned idx = 0 ;  idx < width_s_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(Link::INPUT);
	    pin(p).set_name(perm_string::literal("Sum"), idx);
      }
}

NetMult::~NetMult()
{
}

void NetMult::set_signed(bool flag)
{
      signed_ = flag;
}

bool NetMult::get_signed() const
{
      return signed_;
}

unsigned NetMult::width_r() const
{
      return width_r_;
}

unsigned NetMult::width_a() const
{
      return width_a_;
}

unsigned NetMult::width_b() const
{
      return width_b_;
}

unsigned NetMult::width_s() const
{
      return width_s_;
}

Link& NetMult::pin_Aclr()
{
      return pin(0);
}

const Link& NetMult::pin_Aclr() const
{
      return pin(0);
}

Link& NetMult::pin_Clock()
{
      return pin(1);
}

const Link& NetMult::pin_Clock() const
{
      return pin(1);
}

Link& NetMult::pin_Result(unsigned idx)
{
      assert(idx < width_r_);
      return pin(idx+2);
}

const Link& NetMult::pin_Result(unsigned idx) const
{
      assert(idx < width_r_);
      return pin(idx+2);
}

Link& NetMult::pin_DataA(unsigned idx)
{
      assert(idx < width_a_);
      return pin(idx+2+width_r_);
}

const Link& NetMult::pin_DataA(unsigned idx) const
{
      assert(idx < width_a_);
      return pin(idx+2+width_r_);
}

Link& NetMult::pin_DataB(unsigned idx)
{
      assert(idx < width_b_);
      return pin(idx+2+width_r_+width_a_);
}

const Link& NetMult::pin_DataB(unsigned idx) const
{
      assert(idx < width_b_);
      return pin(idx+2+width_r_+width_a_);
}

Link& NetMult::pin_Sum(unsigned idx)
{
      assert(idx < width_s_);
      return pin(idx+2+width_r_+width_a_+width_b_);
}

const Link& NetMult::pin_Sum(unsigned idx) const
{
      assert(idx < width_s_);
      return pin(idx+2+width_r_+width_a_+width_b_);
}

/*
 * The NetMux class represents an LPM_MUX device. The pinout is assigned
 * like so:
 *    0   -- Aclr (optional)
 *    1   -- Clock (optional)
 *    2   -- Result[0]
 *    2+N -- Result[N]
 */

NetMux::NetMux(NetScope*s, perm_string n,
	       unsigned wi, unsigned si, unsigned sw)
: NetNode(s, n, 2+wi+sw+wi*si),
  width_(wi), size_(si), swidth_(sw)
{
      pin(0).set_dir(Link::INPUT); pin(0).set_name(perm_string::literal("Aclr"),  0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(perm_string::literal("Clock"), 0);

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin_Result(idx).set_dir(Link::OUTPUT);
	    pin_Result(idx).set_name(perm_string::literal("Result"), idx);

	    for (unsigned jdx = 0 ;  jdx < size_ ;  jdx += 1) {
		  pin_Data(idx,jdx).set_dir(Link::INPUT);
		  pin_Data(idx,jdx).set_name(perm_string::literal("Data"), jdx*width_+idx);
	    }
      }

      for (unsigned idx = 0 ;  idx < swidth_ ;  idx += 1) {
	    pin_Sel(idx).set_dir(Link::INPUT);
	    pin_Sel(idx).set_name(perm_string::literal("Sel"), idx);
      }
}

NetMux::~NetMux()
{
}

unsigned NetMux::width()const
{
      return width_;
}

unsigned NetMux::size() const
{
      return size_;
}

unsigned NetMux::sel_width() const
{
      return swidth_;
}

Link& NetMux::pin_Aclr()
{
      return pin(0);
}

const Link& NetMux::pin_Aclr() const
{
      return pin(0);
}

Link& NetMux::pin_Clock()
{
      return pin(1);
}

const Link& NetMux::pin_Clock() const
{
      return pin(1);
}

Link& NetMux::pin_Result(unsigned w)
{
      assert(w < width_);
      return pin(2+w);
}

const Link& NetMux::pin_Result(unsigned w) const
{
      assert(w < width_);
      return pin(2+w);
}

Link& NetMux::pin_Sel(unsigned w)
{
      assert(w < swidth_);
      return pin(2+width_+w);
}

const Link& NetMux::pin_Sel(unsigned w) const
{
      assert(w < swidth_);
      return pin(2+width_+w);
}

Link& NetMux::pin_Data(unsigned w, unsigned s)
{
      assert(w < width_);
      assert(s < size_);
      return pin(2+width_+swidth_+s*width_+w);
}

const Link& NetMux::pin_Data(unsigned w, unsigned s) const
{
      assert(w < width_);
      assert(s < size_);
      return pin(2+width_+swidth_+s*width_+w);
}

void NetRamDq::make_pins_(unsigned wid)
{
      pin(0).set_dir(Link::INPUT); pin(0).set_name(perm_string::literal("InClock"), 0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(perm_string::literal("OutClock"), 0);
      pin(2).set_dir(Link::INPUT); pin(2).set_name(perm_string::literal("WE"), 0);

      for (unsigned idx = 0 ;  idx < awidth_ ;  idx += 1) {
	    pin(3+idx).set_dir(Link::INPUT);
	    pin(3+idx).set_name(perm_string::literal("Address"), idx);
      }

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    pin(3+awidth_+idx).set_dir(Link::INPUT);
	    pin(3+awidth_+idx).set_name(perm_string::literal("Data"), idx);
      }

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    pin(3+awidth_+wid+idx).set_dir(Link::OUTPUT);
	    pin(3+awidth_+wid+idx).set_name(perm_string::literal("Q"), idx);
      }
}


NetRamDq::NetRamDq(NetScope*s, perm_string n, NetMemory*mem, unsigned awid)
: NetNode(s, n, 3+2*mem->width()+awid),
  mem_(mem), awidth_(awid)
{
      make_pins_(mem->width());
      next_ = mem_->ram_list_;
      mem_->ram_list_ = this;
}

NetRamDq::~NetRamDq()
{
      if (mem_) {
	    if (mem_->ram_list_ == this) {
		  mem_->ram_list_ = next_;

	    } else {
		  NetRamDq*cur = mem_->ram_list_;
		  while (cur->next_ != this) {
			assert(cur->next_);
			cur = cur->next_;
		  }
		  assert(cur->next_ == this);
		  cur->next_ = next_;
	    }
      }
}

unsigned NetRamDq::width() const
{
      if (mem_) return mem_->width();
      return 0;
}

unsigned NetRamDq::awidth() const
{
      return awidth_;
}

unsigned NetRamDq::size() const
{
      if (mem_) return mem_->count();
      return 0;
}

NetMemory* NetRamDq::mem()
{
      return mem_;
}

const NetMemory* NetRamDq::mem() const
{
      return mem_;
}

unsigned NetRamDq::count_partners() const
{
      unsigned count = 0;
      for (NetRamDq*cur = mem_->ram_list_ ;  cur ;  cur = cur->next_)
	    count += 1;

      return count;
}

void NetRamDq::absorb_partners()
{
      NetRamDq*cur, *tmp;
      for (cur = mem_->ram_list_, tmp = 0
		 ;  cur||tmp ;  cur = cur? cur->next_ : tmp) {
	    tmp = 0;
	    if (cur == this) continue;

	    bool ok_flag = true;
	    for (unsigned idx = 0 ;  idx < awidth() ;  idx += 1)
		  ok_flag &= pin_Address(idx).is_linked(cur->pin_Address(idx));

	    if (!ok_flag) continue;

	    if (pin_InClock().is_linked()
		&& cur->pin_InClock().is_linked()
		&& ! pin_InClock().is_linked(cur->pin_InClock()))
		  continue;

	    if (pin_OutClock().is_linked()
		&& cur->pin_OutClock().is_linked()
		&& ! pin_OutClock().is_linked(cur->pin_OutClock()))
		  continue;

	    if (pin_WE().is_linked()
		&& cur->pin_WE().is_linked()
		&& ! pin_WE().is_linked(cur->pin_WE()))
		  continue;

	    for (unsigned idx = 0 ;  idx < width() ;  idx += 1) {
		  if (!pin_Data(idx).is_linked()) continue;
		  if (! cur->pin_Data(idx).is_linked()) continue;

		  ok_flag &= pin_Data(idx).is_linked(cur->pin_Data(idx));
	    }

	    if (! ok_flag) continue;

	    for (unsigned idx = 0 ;  idx < width() ;  idx += 1) {
		  if (!pin_Q(idx).is_linked()) continue;
		  if (! cur->pin_Q(idx).is_linked()) continue;

		  ok_flag &= pin_Q(idx).is_linked(cur->pin_Q(idx));
	    }

	    if (! ok_flag) continue;

	      // I see no other reason to reject cur, so link up all
	      // my pins and delete it.
	    connect(pin_InClock(), cur->pin_InClock());
	    connect(pin_OutClock(), cur->pin_OutClock());
	    connect(pin_WE(), cur->pin_WE());

	    for (unsigned idx = 0 ;  idx < awidth() ;  idx += 1)
		  connect(pin_Address(idx), cur->pin_Address(idx));

	    for (unsigned idx = 0 ;  idx < width() ;  idx += 1) {
		  connect(pin_Data(idx), cur->pin_Data(idx));
		  connect(pin_Q(idx), cur->pin_Q(idx));
	    }

	    tmp = cur->next_;
	    delete cur;
	    cur = 0;
      }
}

Link& NetRamDq::pin_InClock()
{
      return pin(0);
}

const Link& NetRamDq::pin_InClock() const
{
      return pin(0);
}

Link& NetRamDq::pin_OutClock()
{
      return pin(1);
}

const Link& NetRamDq::pin_OutClock() const
{
      return pin(1);
}

Link& NetRamDq::pin_WE()
{
      return pin(2);
}

const Link& NetRamDq::pin_WE() const
{
      return pin(2);
}

Link& NetRamDq::pin_Address(unsigned idx)
{
      assert(idx < awidth_);
      return pin(3+idx);
}

const Link& NetRamDq::pin_Address(unsigned idx) const
{
      assert(idx < awidth_);
      return pin(3+idx);
}

Link& NetRamDq::pin_Data(unsigned idx)
{
      assert(idx < width());
      return pin(3+awidth_+idx);
}

const Link& NetRamDq::pin_Data(unsigned idx) const
{
      assert(idx < width());
      return pin(3+awidth_+idx);
}

Link& NetRamDq::pin_Q(unsigned idx)
{
      assert(idx < width());
      return pin(3+awidth_+width()+idx);
}

const Link& NetRamDq::pin_Q(unsigned idx) const
{
      assert(idx < width());
      return pin(3+awidth_+width()+idx);
}

NetBUFZ::NetBUFZ(NetScope*s, perm_string n)
: NetNode(s, n, 2)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      pin(1).set_name(perm_string::literal("I"), 0);
}

NetBUFZ::~NetBUFZ()
{
}


NetCaseCmp::NetCaseCmp(NetScope*s, perm_string n)
: NetNode(s, n, 3)
{
      pin(0).set_dir(Link::OUTPUT); pin(0).set_name(perm_string::literal("O"),0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(perm_string::literal("I"),0);
      pin(2).set_dir(Link::INPUT); pin(2).set_name(perm_string::literal("I"),1);
}

NetCaseCmp::~NetCaseCmp()
{
}

NetCondit::NetCondit(NetExpr*ex, NetProc*i, NetProc*e)
: expr_(ex), if_(i), else_(e)
{
}

NetCondit::~NetCondit()
{
      delete expr_;
      if (if_) delete if_;
      if (else_) delete else_;
}

const NetExpr* NetCondit::expr() const
{
      return expr_;
}

NetExpr* NetCondit::expr()
{
      return expr_;
}

void NetCondit::set_expr(NetExpr*ex)
{
      delete expr_;
      expr_ = ex;
}

NetProc* NetCondit::if_clause()
{
      return if_;
}

NetProc* NetCondit::else_clause()
{
      return else_;
}

NetConst::NetConst(NetScope*s, perm_string n, verinum::V v)
: NetNode(s, n, 1)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      value_ = new verinum::V[1];
      value_[0] = v;
}

NetConst::NetConst(NetScope*s, perm_string n, const verinum&val)
: NetNode(s, n, val.len())
{
      value_ = new verinum::V[pin_count()];
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::OUTPUT);
	    pin(idx).set_name(perm_string::literal("O"), idx);
	    value_[idx] = val.get(idx);
      }
}

NetConst::~NetConst()
{
      delete[]value_;
}

verinum::V NetConst::value(unsigned idx) const
{
      assert(idx < pin_count());
      return value_[idx];
}

NetFuncDef::NetFuncDef(NetScope*s, NetNet*result, const svector<NetNet*>&po)
: scope_(s), statement_(0), result_sig_(result), result_var_(0), ports_(po)
{
}

NetFuncDef::NetFuncDef(NetScope*s, NetVariable*result, const svector<NetNet*>&po)
: scope_(s), statement_(0), result_sig_(0), result_var_(result), ports_(po)
{
}

NetFuncDef::~NetFuncDef()
{
}

const string NetFuncDef::name() const
{
      return scope_->name();
}

void NetFuncDef::set_proc(NetProc*st)
{
      assert(statement_ == 0);
      assert(st != 0);
      statement_ = st;
}

const NetProc* NetFuncDef::proc() const
{
      return statement_;
}

NetScope*NetFuncDef::scope()
{
      return scope_;
}

unsigned NetFuncDef::port_count() const
{
      return ports_.count();
}

const NetNet* NetFuncDef::port(unsigned idx) const
{
      assert(idx < ports_.count());
      return ports_[idx];
}

const NetNet* NetFuncDef::return_sig() const
{
      return result_sig_;
}

const NetVariable* NetFuncDef::return_var() const
{
      return result_var_;
}

NetSTask::NetSTask(const char*na, const svector<NetExpr*>&pa)
: name_(0), parms_(pa)
{
      name_ = lex_strings.add(na);
      assert(name_[0] == '$');
}

NetSTask::~NetSTask()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];

	/* The name_ string is perm-allocated in lex_strings. */
}

const char*NetSTask::name() const
{
      return name_;
}

unsigned NetSTask::nparms() const
{
      return parms_.count();
}

const NetExpr* NetSTask::parm(unsigned idx) const
{
      return parms_[idx];
}

NetEUFunc::NetEUFunc(NetScope*def, NetESignal*res, svector<NetExpr*>&p)
: func_(def), result_sig_(res), result_var_(0), parms_(p)
{
      expr_width(result_sig_->expr_width());
}

NetEUFunc::NetEUFunc(NetScope*def, NetEVariable*res, svector<NetExpr*>&p)
: func_(def), result_sig_(0), result_var_(res), parms_(p)
{
}

NetEUFunc::~NetEUFunc()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];
}

const string NetEUFunc::name() const
{
      return func_->name();
}

const NetESignal*NetEUFunc::result_sig() const
{
      return result_sig_;
}

const NetEVariable*NetEUFunc::result_var() const
{
      return result_var_;
}

unsigned NetEUFunc::parm_count() const
{
      return parms_.count();
}

const NetExpr* NetEUFunc::parm(unsigned idx) const
{
      assert(idx < parms_.count());
      return parms_[idx];
}

const NetScope* NetEUFunc::func() const
{
      return func_;
}

NetExpr::TYPE NetEUFunc::expr_type() const
{
      if (result_sig_)
	    return result_sig_->expr_type();
      if (result_var_)
	    return result_var_->expr_type();

      return ET_VOID;
}

NetUTask::NetUTask(NetScope*def)
: task_(def)
{
}

NetUTask::~NetUTask()
{
}

const string NetUTask::name() const
{
      return task_->name();
}

const NetScope* NetUTask::task() const
{
      return task_;
}

NetExpr::NetExpr(unsigned w)
: width_(w), signed_flag_(false)
{
}

NetExpr::~NetExpr()
{
}

bool NetExpr::has_sign() const
{
      return signed_flag_;
}

void NetExpr::cast_signed(bool flag)
{
      signed_flag_ = flag;
}

bool NetExpr::has_width() const
{
      return true;
}

/*
 * Create a bitwise operator node from the opcode and the left and
 * right expressions. Don't worry about the width of the expression
 * yet, we'll get that from the l-value, whatever that turns out to
 * be. However, if we don't, our default will be the width of the
 * largest operand.
 */
NetEBBits::NetEBBits(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      if (r->expr_width() > l->expr_width())
	    expr_width(r->expr_width());
      else
	    expr_width(l->expr_width());
}

NetEBBits::~NetEBBits()
{
}

NetEBBits* NetEBBits::dup_expr() const
{
      NetEBBits*result = new NetEBBits(op_, left_->dup_expr(),
				       right_->dup_expr());
      return result;
}

NetEBinary::NetEBinary(char op, NetExpr*l, NetExpr*r)
: op_(op), left_(l), right_(r)
{
}

NetEBinary::~NetEBinary()
{
      delete left_;
      delete right_;
}

bool NetEBinary::has_width() const
{
      return left_->has_width() && right_->has_width();
}

NetEBinary* NetEBinary::dup_expr() const
{
      assert(0);
      return 0;
}

NetEBLogic::NetEBLogic(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(1);
}

NetEBLogic::~NetEBLogic()
{
}

NetEBLogic* NetEBLogic::dup_expr() const
{
      NetEBLogic*result = new NetEBLogic(op_, left_->dup_expr(),
					 right_->dup_expr());
      return result;
}

NetEConst::NetEConst(const verinum&val)
: NetExpr(val.len()), value_(val)
{
      cast_signed(value_.has_sign());
}

NetEConst::~NetEConst()
{
}

const verinum& NetEConst::value() const
{
      return value_;
}

bool NetEConst::has_width() const
{
      return value_.has_len();
}

NetEConstParam::NetEConstParam(NetScope*s, perm_string n, const verinum&v)
: NetEConst(v), scope_(s), name_(n)
{
}

NetEConstParam::~NetEConstParam()
{
}

perm_string NetEConstParam::name() const
{
      return name_;
}

const NetScope* NetEConstParam::scope() const
{
      return scope_;
}


NetEMemory::NetEMemory(NetMemory*m, NetExpr*i)
: NetExpr(m->width()), mem_(m), idx_(i)
{
}

NetEMemory::~NetEMemory()
{
}

perm_string NetEMemory::name() const
{
      return mem_->name();
}

const NetExpr* NetEMemory::index() const
{
      return idx_;
}

NetMemory::NetMemory(NetScope*sc, perm_string n, long w, long s, long e)
: width_(w), idxh_(s), idxl_(e), ram_list_(0), scope_(sc)
{
      name_ = n;
      explode_ = 0;
      scope_->add_memory(this);
}

NetMemory::~NetMemory()
{
      assert(scope_);
      scope_->rem_memory(this);
}

unsigned NetMemory::count() const
{
      if (idxh_ < idxl_)
	    return idxl_ - idxh_ + 1;
      else
	    return idxh_ - idxl_ + 1;
}

perm_string NetMemory::name() const
{
      return name_;
}

unsigned NetMemory::index_to_address(long idx) const
{
      if (idxh_ < idxl_)
	    return idx - idxh_;
      else
	    return idx - idxl_;
}

NetNet* NetMemory::explode_to_reg()
{
      if (explode_)
	    return explode_;

      explode_ = new NetNet(scope_, name_, NetNet::REG, count()*width_);
      explode_->mref(this);
	//explode_->incr_lref();
      return explode_;
}

NetNet* NetMemory::reg_from_explode()
{
      return explode_;
}

const NetNet* NetMemory::reg_from_explode() const
{
      return explode_;
}

NetEMemory* NetEMemory::dup_expr() const
{
      assert(0);
      return 0;
}

NetEEvent::NetEEvent(NetEvent*e)
: event_(e)
{
    e->exprref_ += 1;
}

NetEEvent::~NetEEvent()
{
}

const NetEvent* NetEEvent::event() const
{
      return event_;
}

NetEScope::NetEScope(NetScope*s)
: scope_(s)
{
}

NetEScope::~NetEScope()
{
}

const NetScope* NetEScope::scope() const
{
      return scope_;
}

NetESignal::NetESignal(NetNet*n)
: NetExpr(n->pin_count()), net_(n)
{
      msi_ = n->pin_count() - 1;
      lsi_ = 0;
      net_->incr_eref();
      set_line(*n);
      cast_signed(net_->get_signed());
}

NetESignal::NetESignal(NetNet*n, unsigned m, unsigned l)
: NetExpr(m - l + 1), net_(n)
{
      assert(m >= l);
      msi_ = m;
      lsi_ = l;
      net_->incr_eref();
      set_line(*n);
      cast_signed(net_->get_signed());
}

NetESignal::~NetESignal()
{
      net_->decr_eref();
}

perm_string NetESignal::name() const
{
      return net_->name();
}

unsigned NetESignal::bit_count() const
{
      return msi_ - lsi_ + 1;
}

Link& NetESignal::bit(unsigned idx)
{
      assert(idx <= (msi_ - lsi_));
      return net_->pin(idx + lsi_);
}

const NetNet* NetESignal::sig() const
{
      return net_;
}

unsigned NetESignal::lsi() const
{
      return lsi_;
}

unsigned NetESignal::msi() const
{
      return msi_;
}

NetEBitSel::NetEBitSel(NetESignal*sig, NetExpr*ex)
: sig_(sig), idx_(ex)
{
	// This supports mux type indexing of an expression, so the
	// with is by definition 1 bit.
      expr_width(1);
}

NetEBitSel::~NetEBitSel()
{
      delete idx_;
}

perm_string NetEBitSel::name() const
{
      return sig_->name();
}

const NetNet* NetEBitSel::sig() const
{
      return sig_->sig();
}

NetEBitSel* NetEBitSel::dup_expr() const
{
      assert(0);
      return 0;
}

NetETernary::NetETernary(NetExpr*c, NetExpr*t, NetExpr*f)
: cond_(c), true_val_(t), false_val_(f)
{
	// use widest result
      if (true_val_->expr_width() > false_val_->expr_width())
            expr_width(true_val_->expr_width());
      else
            expr_width(false_val_->expr_width());
      cast_signed(c->has_sign() && t->has_sign() && f->has_sign());
}

NetETernary::~NetETernary()
{
      delete cond_;
      delete true_val_;
      delete false_val_;
}

const NetExpr* NetETernary::cond_expr() const
{
      return cond_;
}

const NetExpr* NetETernary::true_expr() const
{
      return true_val_;
}

const NetExpr* NetETernary::false_expr() const
{
      return false_val_;
}

NetEUnary::NetEUnary(char op, NetExpr*ex)
: NetExpr(ex->expr_width()), op_(op), expr_(ex)
{
      switch (op_) {
	  case '!':
	    expr_width(1);
	    break;
      }
      switch (op_) {
	  case '-':
	  case '+':
	    cast_signed(ex->has_sign());
	    break;
	  default:
	    ;
      }
}

NetEUnary::~NetEUnary()
{
      delete expr_;
}

NetEUBits::NetEUBits(char op, NetExpr*ex)
: NetEUnary(op, ex)
{
}

NetEUBits::~NetEUBits()
{
}

NetEUReduce::NetEUReduce(char op, NetExpr*ex)
: NetEUnary(op, ex)
{
      expr_width(1);
}

NetEUReduce::~NetEUReduce()
{
}

NetLogic::NetLogic(NetScope*s, perm_string n, unsigned pins, TYPE t)
: NetNode(s, n, pins), type_(t)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx-1);
      }
}

NetTaskDef::NetTaskDef(const string&n, const svector<NetNet*>&po)
: name_(n), proc_(0), ports_(po)
{
}

NetTaskDef::~NetTaskDef()
{
      delete proc_;
}

void NetTaskDef::set_proc(NetProc*p)
{
      assert(proc_ == 0);
      proc_ = p;
}

unsigned NetTaskDef::port_count() const
{
      return ports_.count();
}

NetNet* NetTaskDef::port(unsigned idx)
{
      assert(idx < ports_.count());
      return ports_[idx];
}

const string& NetTaskDef::name() const
{
      return name_;
}

const NetProc*NetTaskDef::proc() const
{
      return proc_;
}
