/*
 * Copyright (c) 1998-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: netlist.cc,v 1.256 2007/03/02 06:13:22 steve Exp $"
#endif

# include "config.h"

# include <iostream>

# include  <typeinfo>
# include  "compiler.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  "ivl_assert.h"


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
	  case NetNet::WONE:
	    o << "wone";
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

NetBus::NetBus(NetScope*s, unsigned pin_count)
: NetObj(s, perm_string::literal(""), pin_count)
{
}

NetBus::~NetBus()
{
}

NetDelaySrc::NetDelaySrc(NetScope*s, perm_string n,
			 unsigned npins, bool condit_src)
: NetObj(s, n, npins + (condit_src?1:0))
{
      condit_flag_ = false;
      posedge_ = false;
      negedge_ = false;
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    pin(idx).set_name(perm_string::literal("I"), idx);
	    pin(idx).set_dir(Link::INPUT);
      }

      if (condit_src) {
	    condit_flag_ = true;
	    pin(npins).set_name(perm_string::literal("COND"), 0);
	    pin(npins).set_dir(Link::INPUT);
      }
}

NetDelaySrc::~NetDelaySrc()
{
}

void NetDelaySrc::set_delays(uint64_t del)
{
      for (unsigned idx = 0 ;  idx < 12 ;  idx += 1)
	    transition_delays_[idx] = del;
}

void NetDelaySrc::set_delays(uint64_t trise, uint64_t tfall)
{
      transition_delays_[IVL_PE_01] = trise;
      transition_delays_[IVL_PE_10] = tfall;
      transition_delays_[IVL_PE_0z] = trise;
      transition_delays_[IVL_PE_z1] = trise;
      transition_delays_[IVL_PE_1z] = tfall;
      transition_delays_[IVL_PE_z0] = tfall;
      transition_delays_[IVL_PE_0x] = trise;
      transition_delays_[IVL_PE_x1] = trise;
      transition_delays_[IVL_PE_1x] = tfall;
      transition_delays_[IVL_PE_x0] = tfall;
      transition_delays_[IVL_PE_xz] = max(trise,tfall);
      transition_delays_[IVL_PE_zx] = min(trise,tfall);
}

void NetDelaySrc::set_delays(uint64_t trise, uint64_t tfall, uint64_t tz)
{
      transition_delays_[IVL_PE_01] = trise;
      transition_delays_[IVL_PE_10] = tfall;
      transition_delays_[IVL_PE_0z] = tz;
      transition_delays_[IVL_PE_z1] = trise;
      transition_delays_[IVL_PE_1z] = tz;
      transition_delays_[IVL_PE_z0] = tfall;
      transition_delays_[IVL_PE_0x] = min(trise,tz);
      transition_delays_[IVL_PE_x1] = trise;
      transition_delays_[IVL_PE_1x] = min(tfall,tz);
      transition_delays_[IVL_PE_x0] = tfall;
      transition_delays_[IVL_PE_xz] = tz;
      transition_delays_[IVL_PE_zx] = min(trise,tfall);
}

void NetDelaySrc::set_delays(uint64_t t01, uint64_t t10, uint64_t t0z,
			     uint64_t tz1, uint64_t t1z, uint64_t tz0)
{
      transition_delays_[IVL_PE_01] = t01;
      transition_delays_[IVL_PE_10] = t10;
      transition_delays_[IVL_PE_0z] = t0z;
      transition_delays_[IVL_PE_z1] = tz1;
      transition_delays_[IVL_PE_1z] = t1z;
      transition_delays_[IVL_PE_z0] = tz0;
      transition_delays_[IVL_PE_0x] = min(t01,t0z);
      transition_delays_[IVL_PE_x1] = max(t01,tz1);
      transition_delays_[IVL_PE_1x] = min(t10,t1z);
      transition_delays_[IVL_PE_x0] = max(t10,tz0);
      transition_delays_[IVL_PE_xz] = max(t1z,t0z);
      transition_delays_[IVL_PE_zx] = min(tz1,tz0);
}

void NetDelaySrc::set_delays(uint64_t t01, uint64_t t10, uint64_t t0z,
			     uint64_t tz1, uint64_t t1z, uint64_t tz0,
			     uint64_t t0x, uint64_t tx1, uint64_t t1x,
			     uint64_t tx0, uint64_t txz, uint64_t tzx)
{
      transition_delays_[IVL_PE_01] = t01;
      transition_delays_[IVL_PE_10] = t10;
      transition_delays_[IVL_PE_0z] = t0z;
      transition_delays_[IVL_PE_z1] = tz1;
      transition_delays_[IVL_PE_1z] = t1z;
      transition_delays_[IVL_PE_z0] = tz0;
      transition_delays_[IVL_PE_0x] = t0x;
      transition_delays_[IVL_PE_x1] = tx1;
      transition_delays_[IVL_PE_1x] = t1x;
      transition_delays_[IVL_PE_x0] = tx0;
      transition_delays_[IVL_PE_xz] = txz;
      transition_delays_[IVL_PE_zx] = tzx;
}

uint64_t NetDelaySrc::get_delay(unsigned idx) const
{
      assert(idx < 12);
      return transition_delays_[idx];
}

void NetDelaySrc::set_posedge()
{
      posedge_ = true;
}

void NetDelaySrc::set_negedge()
{
      negedge_ = true;
}

bool NetDelaySrc::is_posedge() const
{
      return posedge_;
}

bool NetDelaySrc::is_negedge() const
{
      return negedge_;
}

unsigned NetDelaySrc::src_count() const
{
      if (condit_flag_)
	    return pin_count() - 1;
      else
	    return pin_count();
}

Link& NetDelaySrc::src_pin(unsigned idx)
{
      ivl_assert(*this, idx < src_count());
      return pin(idx);
}

const Link& NetDelaySrc::src_pin(unsigned idx) const
{
      ivl_assert(*this, idx < src_count());
      return pin(idx);
}

bool NetDelaySrc::is_condit() const
{
      return condit_flag_;
}

Link& NetDelaySrc::condit_pin()
{
      ivl_assert(*this, condit_flag_);
      return pin(pin_count()-1);
}

const Link& NetDelaySrc::condit_pin() const
{
      ivl_assert(*this, condit_flag_);
      return pin(pin_count()-1);
}

NetNet::NetNet(NetScope*s, perm_string n, Type t, unsigned npins)
: NetObj(s, n, 1), sig_next_(0), sig_prev_(0),
    type_(t), port_type_(NOT_A_PORT), data_type_(IVL_VT_NO_TYPE),
    signed_(false), msb_(npins-1), lsb_(0), s0_(0), e0_(0),
    local_flag_(false), eref_count_(0), lref_count_(0)
{
      assert(s);

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

      pin(0).set_name(perm_string::literal("P"), 0);
      pin(0).set_dir(dir);
      pin(0).set_init(init_value);

      s->add_signal(this);
}

static unsigned calculate_count(long s, long e)
{
      if (s >= e)
	    return s - e + 1;
      else
	    return e - s + 1;
}

NetNet::NetNet(NetScope*s, perm_string n, Type t,
	       long ms, long ls, long array_s, long array_e)
: NetObj(s, n, calculate_count(array_s, array_e)),
    sig_next_(0), sig_prev_(0), type_(t),
    port_type_(NOT_A_PORT), data_type_(IVL_VT_NO_TYPE), signed_(false),
    msb_(ms), lsb_(ls), s0_(array_s), e0_(array_e),
    local_flag_(false), eref_count_(0), lref_count_(0)
{
      assert(s);

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
      if (scope())
	    scope()->rem_signal(this);

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

ivl_variable_type_t NetNet::data_type() const
{
      return data_type_;
}

void NetNet::data_type(ivl_variable_type_t t)
{
      data_type_ = t;
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

unsigned long NetNet::vector_width() const
{
      if (msb_ > lsb_)
	    return msb_ - lsb_ + 1;
      else
	    return lsb_ - msb_ + 1;
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

unsigned NetNet::array_dimensions() const
{
      if (s0_ == e0_)
	    return 0;
      return 1;
}

long NetNet::array_first() const
{
      if (s0_ < e0_)
	    return s0_;
      else
	    return e0_;
}

unsigned NetNet::array_count() const
{
      return calculate_count(s0_, e0_);
}

bool NetNet::array_index_is_valid(long sb) const
{
      if (sb < s0_ && sb < e0_)
	    return false;
      if (sb > e0_ && sb > s0_)
	    return false;
      return true;
}

unsigned NetNet::array_index_to_address(long sb) const
{
      if (s0_ <= e0_)
	    return sb - s0_;
      else
	    return sb - e0_;
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

void NetNet::add_delay_path(NetDelaySrc*path)
{
      delay_paths_.push_back(path);
}

unsigned NetNet::delay_paths(void)const
{
      return delay_paths_.size();
}

const NetDelaySrc* NetNet::delay_path(unsigned idx) const
{
      assert(idx < delay_paths_.size());
      return delay_paths_[idx];
}

NetPartSelect::NetPartSelect(NetNet*sig, unsigned off, unsigned wid,
			     NetPartSelect::dir_t dir)
: NetNode(sig->scope(), sig->scope()->local_symbol(), 2),
    off_(off), wid_(wid), dir_(dir)
{
      connect(pin(1), sig->pin(0));
      set_line(*sig);

      switch (dir_) {
	  case NetPartSelect::VP:
	    pin(0).set_dir(Link::OUTPUT);
	    pin(1).set_dir(Link::INPUT);
	    break;
	  case NetPartSelect::PV:
	    pin(0).set_dir(Link::INPUT);
	    pin(1).set_dir(Link::OUTPUT);
	    break;
	  case NetPartSelect::BI:
	    pin(0).set_dir(Link::OUTPUT);
	    pin(1).set_dir(Link::OUTPUT);
	    break;
      }
      pin(0).set_name(perm_string::literal("Part"), 0);
      pin(1).set_name(perm_string::literal("Vect"), 0);
}

NetPartSelect::NetPartSelect(NetNet*sig, NetNet*sel,
			     unsigned wid)
: NetNode(sig->scope(), sig->scope()->local_symbol(), 3),
    off_(0), wid_(wid), dir_(VP)
{
      connect(pin(1), sig->pin(0));
      connect(pin(2), sel->pin(0));

      switch (dir_) {
	  case NetPartSelect::VP:
	    pin(0).set_dir(Link::OUTPUT);
	    pin(1).set_dir(Link::INPUT);
	    break;
	  case NetPartSelect::PV:
	    pin(0).set_dir(Link::INPUT);
	    pin(1).set_dir(Link::OUTPUT);
	    break;
	  case NetPartSelect::BI:
	    pin(0).set_dir(Link::PASSIVE);
	    pin(1).set_dir(Link::PASSIVE);
	    break;
      }
      pin(2).set_dir(Link::INPUT);

      pin(0).set_name(perm_string::literal("Part"), 0);
      pin(1).set_name(perm_string::literal("Vect"), 0);
      pin(2).set_name(perm_string::literal("Select"), 0);
}

NetPartSelect::~NetPartSelect()
{
}

unsigned NetPartSelect::width() const
{
      return wid_;
}

unsigned NetPartSelect::base() const
{
      return off_;
}

NetPartSelect::dir_t NetPartSelect::dir() const
{
      return dir_;
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

NetConcat::NetConcat(NetScope*scope, perm_string n, unsigned wid, unsigned cnt)
: NetNode(scope, n, cnt+1), width_(wid)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      for (unsigned idx = 1 ;  idx < cnt+1 ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx-1);
      }
}

NetConcat::~NetConcat()
{
}

unsigned NetConcat::width() const
{
      return width_;
}

NetReplicate::NetReplicate(NetScope*scope, perm_string n,
			   unsigned wid, unsigned rpt)
: NetNode(scope, n, 2), width_(wid), repeat_(rpt)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("I"), 0);
}

NetReplicate::~NetReplicate()
{
}

unsigned NetReplicate::width() const
{
      return width_;
}

unsigned NetReplicate::repeat() const
{
      return repeat_;
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
 *    6  -- Data
 *    7  -- Q
 *     ...
 */

NetFF::NetFF(NetScope*s, perm_string n, unsigned width)
: NetNode(s, n, 8), width_(width)
{
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
      pin_Data().set_dir(Link::INPUT);
      pin_Data().set_name(perm_string::literal("Data"), 0);
      pin_Q().set_dir(Link::OUTPUT);
      pin_Q().set_name(perm_string::literal("Q"), 0);
}

NetFF::~NetFF()
{
}

unsigned NetFF::width() const
{
      return width_;
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

Link& NetFF::pin_Data()
{
      return pin(6);
}

const Link& NetFF::pin_Data() const
{
      return pin(6);
}

Link& NetFF::pin_Q()
{
      return pin(7);
}

const Link& NetFF::pin_Q() const
{
      return pin(7);
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


/*
 * The NetAddSub class represents an LPM_ADD_SUB device. The pinout is
 * assigned like so:
 *    0  -- Add_Sub
 *    1  -- Aclr
 *    2  -- Clock
 *    3  -- Cin
 *    4  -- Cout
 *    5  -- Overflow
 *    6  -- DataA (normally a vector)
 *    7  -- DataB (normally a vector)
 *    8  -- Result (normally a vector)
 */
NetAddSub::NetAddSub(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, 9), width_(w)
{
      pin(0).set_dir(Link::INPUT);
      pin(0).set_name(perm_string::literal("Add_Sub"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("Aclr"), 0);
      pin(2).set_dir(Link::INPUT);
      pin(2).set_name(perm_string::literal("Clock"), 0);
      pin(3).set_dir(Link::INPUT);
      pin(3).set_name(perm_string::literal("Cin"), 0);
      pin(4).set_dir(Link::OUTPUT);
      pin(4).set_name(perm_string::literal("Cout"), 0);
      pin(5).set_dir(Link::OUTPUT);
      pin(5).set_name(perm_string::literal("Overflow"), 0);
      pin(6).set_dir(Link::INPUT);
      pin(6).set_name(perm_string::literal("DataA"), 0);
      pin(7).set_dir(Link::INPUT);
      pin(7).set_name(perm_string::literal("DataB"), 0);
      pin(8).set_dir(Link::OUTPUT);
      pin(8).set_name(perm_string::literal("Result"), 0);
}

NetAddSub::~NetAddSub()
{
}

unsigned NetAddSub::width()const
{
      return width_;
}

Link& NetAddSub::pin_Cout()
{
      return pin(4);
}

const Link& NetAddSub::pin_Cout() const
{
      return pin(4);
}

Link& NetAddSub::pin_DataA()
{
      return pin(6);
}

const Link& NetAddSub::pin_DataA() const
{
      return pin(6);
}

Link& NetAddSub::pin_DataB()
{
      return pin(7);
}

const Link& NetAddSub::pin_DataB() const
{
      return pin(7);
}

Link& NetAddSub::pin_Result()
{
      return pin(8);
}

const Link& NetAddSub::pin_Result() const
{
      return pin(8);
}

NetArrayDq::NetArrayDq(NetScope*s, perm_string n, NetNet*mem, unsigned awid)
: NetNode(s, n, 2),
  mem_(mem), awidth_(awid)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Result"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("Address"), 0);
}

NetArrayDq::~NetArrayDq()
{
}

unsigned NetArrayDq::width() const
{
      return mem_->vector_width();
}

unsigned NetArrayDq::awidth() const
{
      return awidth_;
}

const NetNet* NetArrayDq::mem() const
{
      return mem_;
}

Link& NetArrayDq::pin_Result()
{
      return pin(0);
}

Link& NetArrayDq::pin_Address()
{
      return pin(1);
}

const Link& NetArrayDq::pin_Result() const
{
      return pin(0);
}

const Link& NetArrayDq::pin_Address() const
{
      return pin(1);
}

/*
 * The pinout for the NetCLShift is:
 *    0  -- Result
 *    1  -- Data
 *    2  -- Distance
 */
NetCLShift::NetCLShift(NetScope*s, perm_string n,
		       unsigned width, unsigned width_dist,
		       bool right_flag, bool signed_flag)
: NetNode(s, n, 3),
  width_(width), width_dist_(width_dist),
    right_flag_(right_flag), signed_flag_(signed_flag)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Result"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("Data"), 0);
      pin(2).set_dir(Link::INPUT);
      pin(2).set_name(perm_string::literal("Distance"), 0);
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

Link& NetCLShift::pin_Data()
{
      return pin(1);
}

const Link& NetCLShift::pin_Data() const
{
      return pin(1);
}

Link& NetCLShift::pin_Result()
{
      return pin(0);
}

const Link& NetCLShift::pin_Result() const
{
      return pin(0);
}

Link& NetCLShift::pin_Distance()
{
      return pin(2);
}

const Link& NetCLShift::pin_Distance() const
{
      return pin(2);
}

NetCompare::NetCompare(NetScope*s, perm_string n, unsigned wi)
: NetNode(s, n, 10), width_(wi)
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
      pin(8).set_dir(Link::INPUT);
      pin(8).set_name(perm_string::literal("DataA"));
      pin(9).set_dir(Link::INPUT);
      pin(9).set_name(perm_string::literal("DataB"));
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

Link& NetCompare::pin_DataA()
{
      return pin(8);
}

const Link& NetCompare::pin_DataA() const
{
      return pin(8);
}

Link& NetCompare::pin_DataB()
{
      return pin(9);
}

const Link& NetCompare::pin_DataB() const
{
      return pin(9);
}

NetDivide::NetDivide(NetScope*sc, perm_string n, unsigned wr,
		     unsigned wa, unsigned wb)
: NetNode(sc, n, 3),
    width_r_(wr), width_a_(wa), width_b_(wb), signed_flag_(false)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Result"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("DataA"), 0);
      pin(2).set_dir(Link::INPUT);
      pin(2).set_name(perm_string::literal("DataB"), 0);
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

Link& NetDivide::pin_Result()
{
      return pin(0);
}

const Link& NetDivide::pin_Result() const
{
      return pin(0);
}

Link& NetDivide::pin_DataA()
{
      return pin(1);
}

const Link& NetDivide::pin_DataA() const
{
      return pin(1);
}

Link& NetDivide::pin_DataB()
{
      return pin(2);
}

const Link& NetDivide::pin_DataB() const
{
      return pin(2);
}

NetLiteral::NetLiteral(NetScope*sc, perm_string n, const verireal&val)
: NetNode(sc, n, 1), real_(val)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
}

NetLiteral::~NetLiteral()
{
}

const verireal& NetLiteral::value_real() const
{
      return real_;
}

NetMult::NetMult(NetScope*sc, perm_string n, unsigned wr,
		 unsigned wa, unsigned wb)
: NetNode(sc, n, 3),
  signed_(false), width_r_(wr), width_a_(wa), width_b_(wb)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Result"), 0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("DataA"), 0);
      pin(2).set_dir(Link::INPUT);
      pin(2).set_name(perm_string::literal("DataB"), 0);
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

Link& NetMult::pin_Result()
{
      return pin(0);
}

const Link& NetMult::pin_Result() const
{
      return pin(0);
}

Link& NetMult::pin_DataA()
{
      return pin(1);
}

const Link& NetMult::pin_DataA() const
{
      return pin(1);
}

Link& NetMult::pin_DataB()
{
      return pin(2);
}

const Link& NetMult::pin_DataB() const
{
      return pin(2);
}

/*
 * The NetMux class represents an LPM_MUX device. The pinout is assigned
 * like so:
 *    0   -- Result
 *    1   -- Sel
 *    2+N -- Data[N]  (N is the size of the mux)
 */

NetMux::NetMux(NetScope*s, perm_string n,
	       unsigned wi, unsigned si, unsigned sw)
: NetNode(s, n, 2+si),
  width_(wi), size_(si), swidth_(sw)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("Q"),  0);
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("Sel"), 0);

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT);
	    pin_Data(idx).set_name(perm_string::literal("D"), idx);
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

Link& NetMux::pin_Result()
{
      return pin(0);
}

const Link& NetMux::pin_Result() const
{
      return pin(0);
}

Link& NetMux::pin_Sel()
{
      return pin(1);
}

const Link& NetMux::pin_Sel() const
{
      return pin(1);
}

Link& NetMux::pin_Data(unsigned s)
{
      assert(s < size_);
      return pin(2+s);
}

const Link& NetMux::pin_Data(unsigned s) const
{
      assert(s < size_);
      return pin(2+s);
}

NetSignExtend::NetSignExtend(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, 2), width_(w)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      pin(1).set_name(perm_string::literal("I"), 0);
}

NetSignExtend::~NetSignExtend()
{
}

unsigned NetSignExtend::width() const
{
      return width_;
}

NetBUFZ::NetBUFZ(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, 2), width_(w)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      pin(1).set_name(perm_string::literal("I"), 0);
}

NetBUFZ::~NetBUFZ()
{
}

unsigned NetBUFZ::width() const
{
      return width_;
}

NetCaseCmp::NetCaseCmp(NetScope*s, perm_string n, unsigned wid, bool eeq)
: NetNode(s, n, 3), width_(wid), eeq_(eeq)
{
      pin(0).set_dir(Link::OUTPUT); pin(0).set_name(perm_string::literal("O"),0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name(perm_string::literal("I"),0);
      pin(2).set_dir(Link::INPUT); pin(2).set_name(perm_string::literal("I"),1);
}

NetCaseCmp::~NetCaseCmp()
{
}

unsigned NetCaseCmp::width() const
{
      return width_;
}

bool NetCaseCmp::eeq() const
{
      return eeq_;
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
: NetNode(s, n, 1), width_(1)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      value_ = new verinum::V[1];
      value_[0] = v;
}

NetConst::NetConst(NetScope*s, perm_string n, const verinum&val)
: NetNode(s, n, 1), width_(val.len())
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      value_ = new verinum::V[width_];
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    value_[idx] = val.get(idx);
      }
}

NetConst::~NetConst()
{
      delete[]value_;
}

verinum::V NetConst::value(unsigned idx) const
{
      assert(idx < width_);
      return value_[idx];
}

unsigned NetConst::width() const
{
      return width_;
}

NetFuncDef::NetFuncDef(NetScope*s, NetNet*result, const svector<NetNet*>&po)
: scope_(s), statement_(0), result_sig_(result), ports_(po)
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
: func_(def), result_sig_(res), parms_(p)
{
      expr_width(result_sig_->expr_width());
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

ivl_variable_type_t NetEUFunc::expr_type() const
{
      if (result_sig_)
	    return result_sig_->expr_type();

      return IVL_VT_VOID;
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

ivl_variable_type_t NetEConst::expr_type() const
{
      if (value_.len() == 0)
	    return IVL_VT_LOGIC;
      if (value_.is_string())
	    return IVL_VT_BOOL;
      if (value_.is_defined())
	    return IVL_VT_BOOL;

      return IVL_VT_LOGIC;
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
: NetExpr(n->vector_width()), net_(n), word_(0)
{
      net_->incr_eref();
      set_line(*n);
      cast_signed(net_->get_signed());
}

NetESignal::NetESignal(NetNet*n, NetExpr*w)
: NetExpr(n->vector_width()), net_(n), word_(w)
{
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

const NetExpr* NetESignal::word_index() const
{
      return word_;
}

unsigned NetESignal::vector_width() const
{
      return net_->vector_width();
}

const NetNet* NetESignal::sig() const
{
      return net_;
}

NetNet* NetESignal::sig()
{
      return net_;
}

unsigned NetESignal::lsi() const
{
      return net_->lsb();
}

unsigned NetESignal::msi() const
{
      return net_->msb();
}

ivl_variable_type_t NetESignal::expr_type() const
{
      return net_->data_type();
}

/*
* Make a ternary operator from all the sub-expressions. The condition
* expression is self-determined, but the true and false expressions
* should have the same width. NOTE: This matching of the widths really
* has to be done in elaboration.
*/
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

ivl_variable_type_t NetETernary::expr_type() const
{
      ivl_variable_type_t tru = true_val_->expr_type();
      ivl_variable_type_t fal = false_val_->expr_type();
      if (tru == IVL_VT_LOGIC && fal == IVL_VT_BOOL)
	    return IVL_VT_LOGIC;
      if (tru == IVL_VT_BOOL && fal == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      ivl_assert(*this, tru == fal);
      return tru;
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

ivl_variable_type_t NetEUnary::expr_type() const
{
      return expr_->expr_type();
}

NetEUBits::NetEUBits(char op, NetExpr*ex)
: NetEUnary(op, ex)
{
}

NetEUBits::~NetEUBits()
{
}

ivl_variable_type_t NetEUBits::expr_type() const
{
      return expr_->expr_type();
}

NetEUReduce::NetEUReduce(char op, NetExpr*ex)
: NetEUnary(op, ex)
{
      expr_width(1);
}

NetEUReduce::~NetEUReduce()
{
}

ivl_variable_type_t NetEUReduce::expr_type() const
{
      return expr_->expr_type();
}

NetLogic::NetLogic(NetScope*s, perm_string n, unsigned pins,
		   TYPE t, unsigned wid)
: NetNode(s, n, pins), type_(t), width_(wid)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"), 0);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name(perm_string::literal("I"), idx-1);
      }
}

NetLogic::TYPE NetLogic::type() const
{
      return type_;
}

unsigned NetLogic::width() const
{
      return width_;
}

NetUReduce::NetUReduce(NetScope*scope, perm_string n,
		       NetUReduce::TYPE t, unsigned wid)
: NetNode(scope, n, 2), type_(t), width_(wid)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name(perm_string::literal("O"));
      pin(1).set_dir(Link::INPUT);
      pin(1).set_name(perm_string::literal("I"));
}

NetUReduce::TYPE NetUReduce::type() const
{
      return type_;
}

unsigned NetUReduce::width() const
{
      return width_;
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

/*
 * $Log: netlist.cc,v $
 * Revision 1.256  2007/03/02 06:13:22  steve
 *  Add support for edge sensitive spec paths.
 *
 * Revision 1.255  2007/03/01 06:19:38  steve
 *  Add support for conditional specify delay paths.
 *
 * Revision 1.254  2007/02/20 05:58:36  steve
 *  Handle unary minus of real valued expressions.
 *
 * Revision 1.253  2007/02/14 05:59:46  steve
 *  Handle type of ternary expressions properly.
 *
 * Revision 1.252  2007/01/19 04:25:37  steve
 *  Fix missing passive setting for array word pins.
 *
 * Revision 1.251  2007/01/16 05:44:15  steve
 *  Major rework of array handling. Memories are replaced with the
 *  more general concept of arrays. The NetMemory and NetEMemory
 *  classes are removed from the ivl core program, and the IVL_LPM_RAM
 *  lpm type is removed from the ivl_target API.
 *
 * Revision 1.250  2006/11/10 04:54:26  steve
 *  Add test_width methods for PETernary and PEString.
 *
 * Revision 1.249  2006/09/23 04:57:19  steve
 *  Basic support for specify timing.
 *
 * Revision 1.248  2005/09/14 02:53:14  steve
 *  Support bool expressions and compares handle them optimally.
 *
 * Revision 1.247  2005/08/06 17:58:16  steve
 *  Implement bi-directional part selects.
 *
 * Revision 1.246  2005/07/11 16:56:50  steve
 *  Remove NetVariable and ivl_variable_t structures.
 *
 * Revision 1.245  2005/07/07 16:22:49  steve
 *  Generalize signals to carry types.
 *
 * Revision 1.244  2005/05/24 01:44:28  steve
 *  Do sign extension of structuran nets.
 *
 * Revision 1.243  2005/05/08 23:44:08  steve
 *  Add support for variable part select.
 *
 * Revision 1.242  2005/04/24 23:44:02  steve
 *  Update DFF support to new data flow.
 *
 * Revision 1.241  2005/04/08 04:51:16  steve
 *  All memory addresses are signed.
 *
 * Revision 1.240  2005/04/06 05:29:08  steve
 *  Rework NetRamDq and IVL_LPM_RAM nodes.
 *
 * Revision 1.239  2005/03/09 05:52:04  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.238  2005/02/19 02:43:38  steve
 *  Support shifts and divide.
 *
 * Revision 1.237  2005/02/12 06:25:40  steve
 *  Restructure NetMux devices to pass vectors.
 *  Generate NetMux devices from ternary expressions,
 *  Reduce NetMux devices to bufif when appropriate.
 *
 * Revision 1.236  2005/02/08 00:12:36  steve
 *  Add the NetRepeat node, and code generator support.
 *
 * Revision 1.235  2005/02/03 04:56:20  steve
 *  laborate reduction gates into LPM_RED_ nodes.
 *
 * Revision 1.234  2005/01/28 05:39:33  steve
 *  Simplified NetMult and IVL_LPM_MULT.
 *
 * Revision 1.233  2005/01/24 05:28:30  steve
 *  Remove the NetEBitSel and combine all bit/part select
 *  behavior into the NetESelect node and IVL_EX_SELECT
 *  ivl_target expression type.
 *
 * Revision 1.232  2005/01/22 18:16:01  steve
 *  Remove obsolete NetSubnet class.
 *
 * Revision 1.231  2005/01/22 01:06:55  steve
 *  Change case compare from logic to an LPM node.
 *
 * Revision 1.230  2005/01/16 04:20:32  steve
 *  Implement LPM_COMPARE nodes as two-input vector functors.
 *
 * Revision 1.229  2005/01/09 20:16:01  steve
 *  Use PartSelect/PV and VP to handle part selects through ports.
 *
 * Revision 1.228  2004/12/29 23:55:43  steve
 *  Unify elaboration of l-values for all proceedural assignments,
 *  including assing, cassign and force.
 *
 *  Generate NetConcat devices for gate outputs that feed into a
 *  vector results. Use this to hande gate arrays. Also let gate
 *  arrays handle vectors of gates when the outputs allow for it.
 *
 * Revision 1.227  2004/12/11 02:31:26  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.226  2004/10/04 01:10:54  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.225  2004/06/30 02:16:26  steve
 *  Implement signed divide and signed right shift in nets.
 *
 * Revision 1.224  2004/06/13 04:56:54  steve
 *  Add support for the default_nettype directive.
 *
 * Revision 1.223  2004/05/31 23:34:37  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.222  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.221  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.220  2003/11/10 19:44:30  steve
 *  Fix return value warnings.
 *
 * Revision 1.219  2003/09/03 23:32:10  steve
 *  Oops, missing pin_Sset implementation.
 *
 * Revision 1.218  2003/08/15 02:23:52  steve
 *  Add synthesis support for synchronous reset.
 *
 * Revision 1.217  2003/07/05 20:42:08  steve
 *  Fix some enumeration warnings.
 *
 * Revision 1.216  2003/06/18 03:55:18  steve
 *  Add arithmetic shift operators.
 *
 * Revision 1.215  2003/05/01 01:13:57  steve
 *  More complete bit range internal error message,
 *  Better test of part select ranges on non-zero
 *  signal ranges.
 */

