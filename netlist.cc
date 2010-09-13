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

# include  <typeinfo>
# include  <cstdlib>
# include  <climits>
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
	  case NetNet::UWIRE:
	    o << "uwire";
      }
      return o;
}


unsigned count_inputs(const Link&pin)
{
      unsigned count = 0;

      const Nexus*nex = pin.nexus();
      for (const Link*clnk = nex->first_nlink()
		 ; clnk ; clnk = clnk->next_nlink()) {
	    const NetPins*cur;
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
	    const NetPins*cur;
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
	    const NetPins*cur;
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

	    const NetPins*cur;
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

void NetPins::devirtualize_pins(void)
{
      if (pins_) return;
      if (npins_ > array_size_limit) {
	    cerr << get_fileline() << ": error: pin count " << npins_ <<
		" exceeds " << array_size_limit <<
		" (set by -pARRAY_SIZE_LIMIT)" << endl;
	    assert(0);
      }
      if (debug_optimizer && npins_ > 1000) cerr << "debug: devirtualizing " << npins_ << " pins." << endl;

      pins_ = new Link[npins_];
      pins_[0].pin_zero_ = true;
      pins_[0].node_ = this;
      pins_[0].dir_  = default_dir_;
      pins_[0].init_ = default_init_;

      for (unsigned idx = 1 ;  idx < npins_ ;  idx += 1) {
	    pins_[idx].pin_zero_ = false;
	    pins_[idx].pin_      = idx;
	    pins_[idx].dir_      = default_dir_;
	    pins_[idx].init_     = default_init_;
      }
}

bool NetPins::pins_are_virtual(void) const
{
      return pins_ == NULL;
}

NetPins::NetPins(unsigned npins)
: npins_(npins)
{
      pins_ = NULL;  // Wait until someone asks.
      if (disable_virtual_pins) devirtualize_pins();  // Ask.  Bummer.
}

NetPins::~NetPins()
{
      if (pins_) delete[]pins_;
}

Link& NetPins::pin(unsigned idx)
{
      if (!pins_) devirtualize_pins();
      if (idx >= npins_) {
	    cerr << get_fileline() << ": internal error: pin("<<idx<<")"
		 << " out of bounds("<<npins_<<")" << endl;
	    cerr << get_fileline() << ":               : typeid="
		 << typeid(*this).name() << endl;
      }

      assert(idx < npins_);
      assert(idx == 0? pins_[0].pin_zero_ : pins_[idx].pin_==idx);

      return pins_[idx];
}

const Link& NetPins::pin(unsigned idx) const
{
      if (!pins_ && !disable_virtual_pins) {
	    cerr << get_fileline() << ": internal error: pin is unexpectedly"
	      " virtual, try again with -pDISABLE_VIRTUAL_PINS=true" << endl;
	    assert(0);
      }
      assert(pins_);
      assert(idx < npins_);
      assert(idx == 0? pins_[0].pin_zero_ : pins_[idx].pin_==idx);
      return pins_[idx];
}

void NetPins::set_default_dir(Link::DIR d)
{
       default_dir_ = d;
}

void NetPins::set_default_init(verinum::V val)
{
       default_init_ = val;
}

bool NetPins::is_linked(void)
{
      bool linked_flag = false;
      if (pins_ == NULL) return false;
      for (unsigned u = 0; u < npins_; u++) {
	    if (pins_[u].is_linked()) {
		  linked_flag = true;
		  break;
	    }
      }
      return linked_flag;
}

NetObj::NetObj(NetScope*s, perm_string n, unsigned np)
: NetPins(np), scope_(s), name_(n), delay1_(0), delay2_(0), delay3_(0)
{
      /* Don't
      ivl_assert(*this, np > 0);
       * because it would happen before we get to print a useful
       * message in the NetNet constructor
       */
}

NetObj::~NetObj()
{
}

NetScope* NetObj::scope()
{
      return scope_;
}

const NetScope* NetObj::scope() const
{
      return scope_;
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

NetBranch::NetBranch(ivl_discipline_t dis)
: NetPins(2), IslandBranch(dis)
{
      pin(0).set_dir(Link::PASSIVE);
      pin(1).set_dir(Link::PASSIVE);
}

NetBranch::~NetBranch()
{
}

NetBus::NetBus(NetScope*s, unsigned pin_count__)
: NetObj(s, perm_string::literal(""), pin_count__)
{
      for (unsigned idx = 0 ;  idx <pin_count__ ;  idx += 1) {
	    pin(idx).set_dir(Link::PASSIVE);
      }
}

NetBus::~NetBus()
{
}

NetDelaySrc::NetDelaySrc(NetScope*s, perm_string n, unsigned npins,
                         bool condit_src, bool conditional)
: NetObj(s, n, npins + (condit_src?1:0))
{
      condit_flag_ = false;
      conditional_ = conditional;
      posedge_ = false;
      negedge_ = false;
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
      }

      if (condit_src) {
	    condit_flag_ = true;
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
      return conditional_;
}

bool NetDelaySrc::has_condit() const
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
: NetObj(s, n, 1),
    type_(t), port_type_(NOT_A_PORT), data_type_(IVL_VT_NO_TYPE),
    signed_(false), isint_(false), is_scalar_(false),
    discipline_(0), msb_(npins-1), lsb_(0), dimensions_(0),
    s0_(0), e0_(0), local_flag_(false), eref_count_(0), lref_count_(0)
{
      assert(s);
      assert(npins>0);

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

      pin(0).set_dir(dir);
      pin(0).set_init(init_value);

      s->add_signal(this);
}

void NetNet::initialize_value_and_dir(verinum::V init_value, Link::DIR dir)
{
      if (pins_are_virtual()) {
	    if (0) cerr << "NetNet setting Link default value and dir" << endl;
	    set_default_init(init_value);
	    set_default_dir(dir);
      } else {
	    for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
		  pin(idx).set_dir(dir);
		  pin(idx).set_init(init_value);
	    }
      }
}

NetNet::NetNet(NetScope*s, perm_string n, Type t,
	       long ms, long ls)
: NetObj(s, n, 1), type_(t),
    port_type_(NOT_A_PORT), data_type_(IVL_VT_NO_TYPE), signed_(false),
    isint_(false), is_scalar_(false), discipline_(0), msb_(ms), lsb_(ls),
    dimensions_(0), s0_(0), e0_(0),
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

      initialize_value_and_dir(init_value, dir);

      s->add_signal(this);
}

static unsigned calculate_count(long s, long e)
{
      unsigned long r;
      if (s >= e) {
	    r = s - e;
      } else {
	    r = e - s;
      }
      if (r >= UINT_MAX) {
	    return 0;
      }
      return r + 1;
}

NetNet::NetNet(NetScope*s, perm_string n, Type t,
	       long ms, long ls, long array_s, long array_e)
: NetObj(s, n, calculate_count(array_s, array_e)),
    type_(t), port_type_(NOT_A_PORT),
    data_type_(IVL_VT_NO_TYPE), signed_(false), isint_(false),
    is_scalar_(false), discipline_(0), msb_(ms), lsb_(ls),
    dimensions_(1), s0_(array_s), e0_(array_e),
    local_flag_(false), eref_count_(0), lref_count_(0)
{
      ivl_assert(*this, s);
      if (pin_count() == 0) {
	    cerr << "Array too big [" << array_s << ":" << array_e << "]" << endl;
	    ivl_assert(*this, 0);
      }

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

      initialize_value_and_dir(init_value, dir);

      s->add_signal(this);
}

NetNet::~NetNet()
{
      if (eref_count_ > 0) {
	    cerr << get_fileline() << ": internal error: attempt to delete "
		 << "signal ``" << name() << "'' which has "
		 << "expression references." << endl;
	    dump_net(cerr, 4);
      }
      assert(eref_count_ == 0);
      if (lref_count_ > 0) {
	    cerr << get_fileline() << ": internal error: attempt to delete "
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
      if (data_type_ == IVL_VT_REAL)
	    return true;
      else
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

bool NetNet::get_scalar() const
{
      return is_scalar_;
}

void NetNet::set_scalar(bool flag)
{
      is_scalar_ = flag;
}

ivl_discipline_t NetNet::get_discipline() const
{
      return discipline_;
}

void NetNet::set_discipline(ivl_discipline_t dis)
{
      ivl_assert(*this, discipline_ == 0);
      discipline_ = dis;
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

long NetNet::sb_to_idx(long sb) const
{
      if (msb_ >= lsb_)
	    return sb - lsb_;
      else
	    return lsb_ - sb;
}

unsigned NetNet::array_dimensions() const
{
      return dimensions_;
}

long NetNet::array_first() const
{
      if (s0_ <= e0_)
	    return s0_;
      else
	    return e0_;
}

bool NetNet::array_addr_swapped() const
{
      if (s0_ <= e0_)
	     return false;
      else
	     return true;
}

unsigned NetNet::array_count() const
{
      unsigned c = calculate_count(s0_, e0_);
      ivl_assert(*this, c > 0);
      return c;
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
			     NetPartSelect::dir_t dir__)
: NetNode(sig->scope(), sig->scope()->local_symbol(), 2),
    off_(off), wid_(wid), dir_(dir__), signed_flag_(false)
{
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
      }

      connect(pin(1), sig->pin(0));
}

NetPartSelect::NetPartSelect(NetNet*sig, NetNet*sel,
			     unsigned wid, bool signed_flag__)
: NetNode(sig->scope(), sig->scope()->local_symbol(), 3),
    off_(0), wid_(wid), dir_(VP), signed_flag_(signed_flag__)
{
      switch (dir_) {
	  case NetPartSelect::VP:
	    pin(0).set_dir(Link::OUTPUT);
	    pin(1).set_dir(Link::INPUT);
	    break;
	  case NetPartSelect::PV:
	      /* Only a vector to part can be a variable select. */
	    assert(0);
      }
      pin(2).set_dir(Link::INPUT);

      connect(pin(1), sig->pin(0));
      connect(pin(2), sel->pin(0));
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

NetProcTop::NetProcTop(NetScope*s, ivl_process_type_t t, NetProc*st)
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

NetAnalogTop::NetAnalogTop(NetScope*scope__, ivl_process_type_t t, NetProc*st)
: type_(t), statement_(st), scope_(scope__)
{
      next_ = 0;
}

NetAnalogTop::~NetAnalogTop()
{
}

NetProc* NetAnalogTop::statement()
{
      return statement_;
}

const NetProc* NetAnalogTop::statement() const
{
      return statement_;
}

NetScope* NetAnalogTop::scope()
{
      return scope_;
}

const NetScope* NetAnalogTop::scope() const
{
      return scope_;
}

NetCastInt::NetCastInt(NetScope*scope__, perm_string n, unsigned width__)
: NetNode(scope__, n, 2), width_(width__)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
}

NetCastReal::NetCastReal(NetScope*scope__, perm_string n, bool signed_flag__)
: NetNode(scope__, n, 2), signed_flag_(signed_flag__)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
}

NetConcat::NetConcat(NetScope*scope__, perm_string n, unsigned wid, unsigned cnt)
: NetNode(scope__, n, cnt+1), width_(wid)
{
      pin(0).set_dir(Link::OUTPUT);
      for (unsigned idx = 1 ;  idx < cnt+1 ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
      }
}

NetConcat::~NetConcat()
{
}

unsigned NetConcat::width() const
{
      return width_;
}

NetReplicate::NetReplicate(NetScope*scope__, perm_string n,
			   unsigned wid, unsigned rpt)
: NetNode(scope__, n, 2), width_(wid), repeat_(rpt)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
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

NetFF::NetFF(NetScope*s, perm_string n, unsigned width__)
: NetNode(s, n, 8), width_(width__)
{
      pin_Clock().set_dir(Link::INPUT);
      pin_Enable().set_dir(Link::INPUT);
      pin_Aset().set_dir(Link::INPUT);
      pin_Aclr().set_dir(Link::INPUT);
      pin_Sset().set_dir(Link::INPUT);
      pin_Sclr().set_dir(Link::INPUT);
      pin_Data().set_dir(Link::INPUT);
      pin_Q().set_dir(Link::OUTPUT);
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


NetAbs::NetAbs(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, 2), width_(w)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
}

NetAbs::~NetAbs()
{
}

unsigned NetAbs::width() const
{
      return width_;
}

/*
 * The NetAddSub class represents an LPM_ADD_SUB device. The pinout is
 * assigned like so:
 *    0  -- Cout
 *    1  -- DataA (normally a vector)
 *    2  -- DataB (normally a vector)
 *    3  -- Result (normally a vector)
 */
NetAddSub::NetAddSub(NetScope*s, perm_string n, unsigned w)
: NetNode(s, n, 4), width_(w)
{
      pin(0).set_dir(Link::OUTPUT); // Cout
      pin(1).set_dir(Link::INPUT);  // DataA
      pin(2).set_dir(Link::INPUT);  // DataB
      pin(3).set_dir(Link::OUTPUT); // Result
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
      return pin(0);
}

const Link& NetAddSub::pin_Cout() const
{
      return pin(0);
}

Link& NetAddSub::pin_DataA()
{
      return pin(1);
}

const Link& NetAddSub::pin_DataA() const
{
      return pin(1);
}

Link& NetAddSub::pin_DataB()
{
      return pin(2);
}

const Link& NetAddSub::pin_DataB() const
{
      return pin(2);
}

Link& NetAddSub::pin_Result()
{
      return pin(3);
}

const Link& NetAddSub::pin_Result() const
{
      return pin(3);
}

NetArrayDq::NetArrayDq(NetScope*s, perm_string n, NetNet*mem__, unsigned awid)
: NetNode(s, n, 2),
  mem_(mem__), awidth_(awid)
{
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // Address
	// Increment the expression reference count for the target
	// memory so that it is not deleted underneath me.
      mem_->incr_eref();
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
		       unsigned width__, unsigned width_dist__,
		       bool right_flag__, bool signed_flag__)
: NetNode(s, n, 3),
  width_(width__), width_dist_(width_dist__),
    right_flag_(right_flag__), signed_flag_(signed_flag__)
{
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // Data
      pin(2).set_dir(Link::INPUT);  // Distance
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
: NetNode(s, n, 8), width_(wi)
{
      signed_flag_ = false;
      pin(0).set_dir(Link::OUTPUT); // AGB
      pin(1).set_dir(Link::OUTPUT); // AGEB
      pin(2).set_dir(Link::OUTPUT); // AEB
      pin(3).set_dir(Link::OUTPUT); // ANEB
      pin(4).set_dir(Link::OUTPUT); // ALB
      pin(5).set_dir(Link::OUTPUT); // ALEB
      pin(6).set_dir(Link::INPUT);  // DataA
      pin(7).set_dir(Link::INPUT);  // DataB
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


Link& NetCompare::pin_AGB()
{
      return pin(0);
}

const Link& NetCompare::pin_AGB() const
{
      return pin(0);
}

Link& NetCompare::pin_AGEB()
{
      return pin(1);
}

const Link& NetCompare::pin_AGEB() const
{
      return pin(1);
}

Link& NetCompare::pin_AEB()
{
      return pin(2);
}

const Link& NetCompare::pin_AEB() const
{
      return pin(2);
}

Link& NetCompare::pin_ANEB()
{
      return pin(3);
}

const Link& NetCompare::pin_ANEB() const
{
      return pin(3);
}

Link& NetCompare::pin_ALB()
{
      return pin(4);
}

const Link& NetCompare::pin_ALB() const
{
      return pin(4);
}

Link& NetCompare::pin_ALEB()
{
      return pin(5);
}

const Link& NetCompare::pin_ALEB() const
{
      return pin(5);
}

Link& NetCompare::pin_DataA()
{
      return pin(6);
}

const Link& NetCompare::pin_DataA() const
{
      return pin(6);
}

Link& NetCompare::pin_DataB()
{
      return pin(7);
}

const Link& NetCompare::pin_DataB() const
{
      return pin(7);
}

NetDivide::NetDivide(NetScope*sc, perm_string n, unsigned wr,
		     unsigned wa, unsigned wb)
: NetNode(sc, n, 3),
    width_r_(wr), width_a_(wa), width_b_(wb), signed_flag_(false)
{
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // DataA
      pin(2).set_dir(Link::INPUT);  // DataB
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
}

NetLiteral::~NetLiteral()
{
}

ivl_variable_type_t NetLiteral::data_type() const
{
      return IVL_VT_REAL;
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
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // DataA
      pin(2).set_dir(Link::INPUT);  // DataB
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

NetPow::NetPow(NetScope*sc, perm_string n, unsigned wr,
		 unsigned wa, unsigned wb)
: NetNode(sc, n, 3),
  signed_(false), width_r_(wr), width_a_(wa), width_b_(wb)
{
      pin(0).set_dir(Link::OUTPUT); // Result
      pin(1).set_dir(Link::INPUT);  // DataA
      pin(2).set_dir(Link::INPUT);  // DataB
}

NetPow::~NetPow()
{
}

void NetPow::set_signed(bool flag)
{
      signed_ = flag;
}

bool NetPow::get_signed() const
{
      return signed_;
}

unsigned NetPow::width_r() const
{
      return width_r_;
}

unsigned NetPow::width_a() const
{
      return width_a_;
}

unsigned NetPow::width_b() const
{
      return width_b_;
}

Link& NetPow::pin_Result()
{
      return pin(0);
}

const Link& NetPow::pin_Result() const
{
      return pin(0);
}

Link& NetPow::pin_DataA()
{
      return pin(1);
}

const Link& NetPow::pin_DataA() const
{
      return pin(1);
}

Link& NetPow::pin_DataB()
{
      return pin(2);
}

const Link& NetPow::pin_DataB() const
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
      pin(0).set_dir(Link::OUTPUT); // Q
      pin(1).set_dir(Link::INPUT);  // Sel

      for (unsigned idx = 0 ;  idx < size_ ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT); // Data[idx]
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
}

NetBUFZ::~NetBUFZ()
{
}

unsigned NetBUFZ::width() const
{
      return width_;
}

NetCaseCmp::NetCaseCmp(NetScope*s, perm_string n, unsigned wid, bool eeq__)
: NetNode(s, n, 3), width_(wid), eeq_(eeq__)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
      pin(2).set_dir(Link::INPUT);
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
      value_ = new verinum::V[1];
      value_[0] = v;
}

NetConst::NetConst(NetScope*s, perm_string n, const verinum&val)
: NetNode(s, n, 1), width_(val.len())
{
      pin(0).set_dir(Link::OUTPUT);
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

const NetScope* NetFuncDef::scope() const
{
      return scope_;
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

NetEUFunc::NetEUFunc(NetScope*scope, NetScope*def, NetESignal*res,
                     svector<NetExpr*>&p)
: scope_(scope), func_(def), result_sig_(res), parms_(p)
{
      expr_width(result_sig_->expr_width());
}

NetEUFunc::~NetEUFunc()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];
}
#if 0
const string NetEUFunc::name() const
{
      return func_->name();
}
#endif
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

const NetScope* NetUTask::task() const
{
      return task_;
}

NetAlloc::NetAlloc(NetScope*scope__)
: scope_(scope__)
{
}

NetAlloc::~NetAlloc()
{
}
#if 0
const string NetAlloc::name() const
{
      return scope_->name();
}
#endif
const NetScope* NetAlloc::scope() const
{
      return scope_;
}

NetFree::NetFree(NetScope*scope__)
: scope_(scope__)
{
}

NetFree::~NetFree()
{
}
#if 0
const string NetFree::name() const
{
      return scope_->name();
}
#endif
const NetScope* NetFree::scope() const
{
      return scope_;
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

void NetExpr::expr_width(unsigned w)
{
	// Catch underflow wrap errors.
      ivl_assert(*this, w < (UINT_MAX - 256));
      width_ = w;
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
NetEBBits::NetEBBits(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
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

NetEBinary::NetEBinary(char op__, NetExpr*l, NetExpr*r)
: op_(op__), left_(l), right_(r)
{
	// Binary expressions of all sorts are signed if both the
	// arguments are signed.
      cast_signed_base_( left_->has_sign() && right_->has_sign());
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

NetEBLogic::NetEBLogic(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r)
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
      cast_signed_base_(value_.has_sign());
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
      cast_signed_base_(v.has_sign());
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

long NetESignal::lsi() const
{
      return net_->lsb();
}

long NetESignal::msi() const
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
      if (type_is_vectorable(expr_type())) {
	      // use widest result
	    if (true_val_->expr_width() > false_val_->expr_width())
		  expr_width(true_val_->expr_width());
	    else
		  expr_width(false_val_->expr_width());
      } else {
	    expr_width(1);
      }

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
      ivl_assert(*this, true_val_);
      ivl_assert(*this, false_val_);
      ivl_variable_type_t tru = true_val_->expr_type();
      ivl_variable_type_t fal = false_val_->expr_type();
      if (tru == IVL_VT_LOGIC && fal == IVL_VT_BOOL)
	    return IVL_VT_LOGIC;
      if (tru == IVL_VT_BOOL && fal == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      if (tru == IVL_VT_REAL && (fal == IVL_VT_LOGIC || fal == IVL_VT_BOOL))
	    return IVL_VT_REAL;
      if (fal == IVL_VT_REAL && (tru == IVL_VT_LOGIC || tru == IVL_VT_BOOL))
	    return IVL_VT_REAL;

      if (tru != fal) {
	    cerr << get_fileline() << ": internal error:"
		 << " Unexpected ?: type clash:"
		 << " tru=" << tru << ", fal=" << fal << endl;
      }
      ivl_assert(*this, tru == fal);
      return tru;
}

NetEUnary::NetEUnary(char op__, NetExpr*ex)
: NetExpr(ex->expr_width()), op_(op__), expr_(ex)
{
      switch (op_) {
	  case '!':
	    expr_width(1);
	    break;
      }
      switch (op_) {
	  case '-':
	  case '+':
	  case 'm': // abs()
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

NetEUBits::NetEUBits(char op__, NetExpr*ex)
: NetEUnary(op__, ex)
{
}

NetEUBits::~NetEUBits()
{
}

ivl_variable_type_t NetEUBits::expr_type() const
{
      return expr_->expr_type();
}

NetEUReduce::NetEUReduce(char op__, NetExpr*ex)
: NetEUnary(op__, ex)
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
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
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

NetUReduce::NetUReduce(NetScope*scope__, perm_string n,
		       NetUReduce::TYPE t, unsigned wid)
: NetNode(scope__, n, 2), type_(t), width_(wid)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
}

NetUReduce::TYPE NetUReduce::type() const
{
      return type_;
}

unsigned NetUReduce::width() const
{
      return width_;
}

NetTaskDef::NetTaskDef(NetScope*n, const svector<NetNet*>&po)
: scope_(n), proc_(0), ports_(po)
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
#if 0
const string& NetTaskDef::name() const
{
      return name_;
}
#endif
const NetScope* NetTaskDef::scope() const
{
      return scope_;
}

const NetProc*NetTaskDef::proc() const
{
      return proc_;
}

/*
 * These are the delay_type() functions. They are used to determine
 * the type of delay for the given object.
 */

/*
 * This function implements the following table:
 *
 * in_A  in_B   out
 *   NO    NO    NO
 *   NO  ZERO  ZERO
 *   NO   POS   POS
 *   NO   DEF   POS
 * ZERO    NO  ZERO
 * ZERO  ZERO  ZERO
 * ZERO   POS   POS
 * ZERO   DEF   POS
 *  POS    NO   POS
 *  POS  ZERO   POS
 *  POS   POS   POS
 *  POS   DEF   POS
 *  DEF    NO   POS
 *  DEF  ZERO   POS
 *  DEF   POS   POS
 *  DEF   DEF   DEF
 *
 * It is used to combine two delay values.
 */
static DelayType combine_delays(const DelayType a, const DelayType b)
{
	/* The default is POSSIBLE_DELAY. */
      DelayType result = POSSIBLE_DELAY;

	/* If both are no or zero delay then we return ZERO_DELAY. */
      if ((a == NO_DELAY || a == ZERO_DELAY) &&
          (b == NO_DELAY || b == ZERO_DELAY)) {
	    result = ZERO_DELAY;
      }

	/* Except if both are no delay then we return NO_DELAY. */
      if (a == NO_DELAY && b == NO_DELAY) {
	    result = NO_DELAY;
      }

	/* If both are definite delay then we return DEFINITE_DELAY. */
      if (a == DEFINITE_DELAY && b == DEFINITE_DELAY) {
	    result = DEFINITE_DELAY;
      }

      return result;
}

/*
 * This is used to see what we can find out about the delay when it
 * is given as an expression. We also use this for loop expressions.
 */
static DelayType delay_type_from_expr(const NetExpr*expr)
{
      DelayType result = POSSIBLE_DELAY;

      if (const NetEConst*e = dynamic_cast<const NetEConst*>(expr)) {
	    if (e->value().is_zero()) result = ZERO_DELAY;
	    else result = DEFINITE_DELAY;
      }

      if (const NetECReal*e = dynamic_cast<const NetECReal*>(expr)) {
	    if (e->value().as_double() == 0.0) result = ZERO_DELAY;
	    else result = DEFINITE_DELAY;
      }

      return result;
}

/*
 * The looping structures can use the same basic code so put it here
 * instead of duplicating it for each one (repeat and while).
 */
static DelayType get_loop_delay_type(const NetExpr*expr, const NetProc*proc)
{
      DelayType result;

      switch (delay_type_from_expr(expr)) {
	    /* We have a constant false expression so the body never runs. */
	  case ZERO_DELAY:
	    result = NO_DELAY;
	    break;
	    /* We have a constant true expression so the body always runs. */
	  case DEFINITE_DELAY:
	    result = proc->delay_type();
	    break;
	    /* We don't know if the body will run so reduce a DEFINITE_DELAY
	     * to a POSSIBLE_DELAY. All other stay the same. */
	  case POSSIBLE_DELAY:
	    result = combine_delays(NO_DELAY, proc->delay_type());
	    break;
	    /* This should never happen since delay_type_from_expr() only
	     * returns three different values. */
	  default:
	    result = NO_DELAY;
	    assert(0);
      }

      return result;
}

/* The default object does not have any delay. */
DelayType NetProc::delay_type() const
{
      return NO_DELAY;
}

DelayType NetBlock::delay_type() const
{
      DelayType result = NO_DELAY;

      for (const NetProc*cur = proc_first(); cur; cur = proc_next(cur)) {
	    DelayType dt = cur->delay_type();
            if (dt > result) result = dt;
            if (dt == DEFINITE_DELAY) break;
      }

      return result;
}

DelayType NetCase::delay_type() const
{
      DelayType result = NO_DELAY;
      bool def_stmt = false;
      unsigned nstmts = nitems();

      for (unsigned idx = 0; idx < nstmts; idx += 1) {
	    if (!expr(idx)) def_stmt = true;
            if (idx == 0) {
		  result = stat(idx)->delay_type();
            } else {
		  result = combine_delays(result, stat(idx)->delay_type());
            }
      }

	/* If we don't have a default statement we don't know for sure
	 * that we have a delay. */
      if (!def_stmt) result = combine_delays(NO_DELAY, result);

      return result;
}

DelayType NetCondit::delay_type() const
{
      DelayType result;

      if (else_) {
	    result = combine_delays(if_->delay_type(), else_->delay_type());
      } else {
	      /* Because of the indeterminate conditional value the
	       * best we can have for this case is a possible delay. */
	    if (if_) {
		  result = combine_delays(if_->delay_type(), NO_DELAY);
	    } else {
		  result = NO_DELAY;
	    }
      }

      return result;
}

DelayType NetEvWait::delay_type() const
{
      return DEFINITE_DELAY;
}

DelayType NetForever::delay_type() const
{
      return statement_->delay_type();
}

DelayType NetPDelay::delay_type() const
{
      if (expr_) {
	    return delay_type_from_expr(expr_);
      } else {
	    if (delay() > 0) {
		  return DEFINITE_DELAY;
	    } else {
		  if (statement_) {
			return statement_->delay_type();
		  } else {
			return NO_DELAY;
		  }
	    }
      }
}

DelayType NetRepeat::delay_type() const
{
      return get_loop_delay_type(expr_, statement_);
}

DelayType NetTaskDef::delay_type() const
{
      return proc_->delay_type();
}

DelayType NetUTask::delay_type() const
{
      return task()->task_def()->delay_type();
}

DelayType NetWhile::delay_type() const
{
      return get_loop_delay_type(cond_, proc_);
}
