/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: netlist.cc,v 1.109 2000/03/29 04:37:11 steve Exp $"
#endif

# include  <cassert>
# include  <typeinfo>
# include  "netlist.h"
# include  "netmisc.h"

ostream& operator<< (ostream&o, NetNet::Type t)
{
      switch (t) {
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


void connect(NetObj::Link&l, NetObj::Link&r)
{
      assert(&l != &r);
      assert(l.next_->prev_ == &l);
      assert(l.prev_->next_ == &l);
      assert(r.next_->prev_ == &r);
      assert(r.prev_->next_ == &r);

      NetObj::Link* cur = &l;
      do {
	    NetObj::Link*tmp = cur->next_;

	      // If I stumble on r in the nexus, then stop now because
	      // we are already connected.
	    if (tmp == &r) break;

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

      assert(l.next_->prev_ == &l);
      assert(l.prev_->next_ == &l);
      assert(r.next_->prev_ == &r);
      assert(r.prev_->next_ == &r);
}

NetObj::Link::Link()
: dir_(PASSIVE), inst_(0), next_(this), prev_(this)
{
}

NetObj::Link::~Link()
{
      unlink();
}

void NetObj::Link::unlink()
{
      next_->prev_ = prev_;
      prev_->next_ = next_;
      next_ = prev_ = this;
}

bool NetObj::Link::is_linked() const
{
      return next_ != this;
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
	    if (idx->is_equal(that))
		  return true;

      return false;
}

void NetObj::Link::next_link(NetObj*&net, unsigned&pin)
{
      assert(next_->prev_ == this);
      assert(prev_->next_ == this);
      net = next_->node_;
      pin = next_->pin_;
}

void NetObj::Link::next_link(const NetObj*&net, unsigned&pin) const
{
      assert(next_->prev_ == this);
      assert(prev_->next_ == this);
      net = next_->node_;
      pin = next_->pin_;
}

NetObj::Link* NetObj::Link::next_link()
{
      assert(next_->prev_ == this);
      assert(prev_->next_ == this);
      return next_;
}

const NetObj::Link* NetObj::Link::next_link() const
{
      assert(next_->prev_ == this);
      assert(prev_->next_ == this);
      return next_;
}

const NetObj*NetObj::Link::get_obj() const
{
      return node_;
}

NetObj*NetObj::Link::get_obj()
{
      return node_;
}

unsigned NetObj::Link::get_pin() const
{
      return pin_;
}

void NetObj::Link::set_name(const string&n, unsigned i)
{
      name_ = n;
      inst_ = i;
}

const string& NetObj::Link::get_name() const
{
      return name_;
}

unsigned NetObj::Link::get_inst() const
{
      return inst_;
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

NetObj::Link* find_next_output(NetObj::Link*lnk)
{
      for (NetObj::Link*cur = lnk->next_link()
		 ;  cur != lnk ;  cur = cur->next_link())
	    if (cur->get_dir() == NetObj::Link::OUTPUT)
		  return cur;

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

NetObj::Link& NetObj::pin(unsigned idx)
{
      assert(idx < npins_);
      return pins_[idx];
}

const NetObj::Link& NetObj::pin(unsigned idx) const
{
      assert(idx < npins_);
      return pins_[idx];
}

NetNode::~NetNode()
{
      if (design_)
	    design_->del_node(this);
}

NetNet::NetNet(NetScope*s, const string&n, Type t, unsigned npins)
: NetObj(n, npins), sig_next_(0), sig_prev_(0), design_(0), scope_(s),
    type_(t), port_type_(NOT_A_PORT), msb_(npins-1), lsb_(0),
    local_flag_(false), eref_count_(0)
{
      ivalue_ = new verinum::V[npins];
      for (unsigned idx = 0 ;  idx < npins ;  idx += 1) {
	    pin(idx).set_name("P", idx);
	    ivalue_[idx] = verinum::Vz;
      }
}

NetNet::NetNet(NetScope*s, const string&n, Type t, long ms, long ls)
: NetObj(n, ((ms>ls)?ms-ls:ls-ms) + 1), sig_next_(0),
    sig_prev_(0), design_(0), scope_(s), type_(t),
    port_type_(NOT_A_PORT), msb_(ms), lsb_(ls), local_flag_(false),
    eref_count_(0)
{
      ivalue_ = new verinum::V[pin_count()];
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_name("P", idx);
	    ivalue_[idx] = verinum::Vz;
      }
}

NetNet::~NetNet()
{
      assert(eref_count_ == 0);
      if (design_)
	    design_->del_signal(this);
}

NetScope* NetNet::scope()
{
      return scope_;
}

const NetScope* NetNet::scope() const
{
      return scope_;
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

unsigned NetNet::get_eref() const
{
      return eref_count_;
}

NetTmp::NetTmp(const string&name, unsigned npins)
: NetNet(0, name, IMPLICIT, npins)
{
      local_flag(true);
}

NetProc::NetProc()
: next_(0)
{
}

NetProc::~NetProc()
{
}

NetProcTop::NetProcTop(Type t, NetProc*st)
: type_(t), statement_(st)
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

/*
 * The NetFF class represents an LPM_FF device. The pinout is assigned
 * like so:
 *    0  -- Clock
 *    1  -- Enable
 *    2  -- Aload
 *    3  -- Aset
 *    4  -- Aclr
 *    5  -- Sload
 *    6  -- Sset
 *    7  -- Sclr
 *
 *    8  -- Data[0]
 *    9  -- Q[0]
 *     ...
 */

NetFF::NetFF(const string&n, unsigned wid)
: NetNode(n, 8 + 2*wid)
{
      pin_Clock().set_dir(Link::INPUT);
      pin_Clock().set_name("Clock", 0);
      pin_Enable().set_dir(Link::INPUT);
      pin_Enable().set_name("Enable", 0);
      pin_Aload().set_dir(Link::INPUT);
      pin_Aload().set_name("Aload", 0);
      pin_Aset().set_dir(Link::INPUT);
      pin_Aset().set_name("Aset", 0);
      pin_Aclr().set_dir(Link::INPUT);
      pin_Aclr().set_name("Aclr", 0);
      pin_Sload().set_dir(Link::INPUT);
      pin_Sload().set_name("Sload", 0);
      pin_Sset().set_dir(Link::INPUT);
      pin_Sset().set_name("Sset", 0);
      pin_Sclr().set_dir(Link::INPUT);
      pin_Sclr().set_name("Sclr", 0);
      for (unsigned idx = 0 ;  idx < wid ;  idx += 1) {
	    pin_Data(idx).set_dir(Link::INPUT);
	    pin_Data(idx).set_name("Data", idx);
	    pin_Q(idx).set_dir(Link::OUTPUT);
	    pin_Q(idx).set_name("Q", idx);
      }
}

NetFF::~NetFF()
{
}

unsigned NetFF::width() const
{
      return (pin_count() - 8) / 2;
}

NetObj::Link& NetFF::pin_Clock()
{
      return pin(0);
}

const NetObj::Link& NetFF::pin_Clock() const
{
      return pin(0);
}

NetObj::Link& NetFF::pin_Enable()
{
      return pin(1);
}

const NetObj::Link& NetFF::pin_Enable() const
{
      return pin(1);
}

NetObj::Link& NetFF::pin_Aload()
{
      return pin(2);
}

NetObj::Link& NetFF::pin_Aset()
{
      return pin(3);
}

NetObj::Link& NetFF::pin_Aclr()
{
      return pin(4);
}

NetObj::Link& NetFF::pin_Sload()
{
      return pin(5);
}

NetObj::Link& NetFF::pin_Sset()
{
      return pin(6);
}

NetObj::Link& NetFF::pin_Sclr()
{
      return pin(7);
}

NetObj::Link& NetFF::pin_Data(unsigned w)
{
      unsigned pn = 8 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

const NetObj::Link& NetFF::pin_Data(unsigned w) const
{
      unsigned pn = 8 + 2*w;
      assert(pn < pin_count());
      return pin(pn);
}

NetObj::Link& NetFF::pin_Q(unsigned w)
{
      unsigned pn = 9 + w*2;
      assert(pn < pin_count());
      return pin(pn);
}

const NetObj::Link& NetFF::pin_Q(unsigned w) const
{
      unsigned pn = 9 + w*2;
      assert(pn < pin_count());
      return pin(pn);
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
NetAddSub::NetAddSub(const string&n, unsigned w)
: NetNode(n, w*3+6)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("Add_Sub", 0);
      pin(1).set_dir(NetObj::Link::INPUT); pin(1).set_name("Aclr", 0);
      pin(2).set_dir(NetObj::Link::INPUT); pin(2).set_name("Clock", 0);
      pin(3).set_dir(NetObj::Link::INPUT); pin(3).set_name("Cin", 0);
      pin(4).set_dir(NetObj::Link::OUTPUT); pin(4).set_name("Cout", 0);
      pin(5).set_dir(NetObj::Link::OUTPUT); pin(5).set_name("Overflow", 0);
      for (unsigned idx = 0 ;  idx < w ;  idx += 1) {
	    pin_DataA(idx).set_dir(NetObj::Link::INPUT);
	    pin_DataB(idx).set_dir(NetObj::Link::INPUT);
	    pin_Result(idx).set_dir(NetObj::Link::OUTPUT);
	    pin_DataA(idx).set_name("DataA", idx);
	    pin_DataB(idx).set_name("DataB", idx);
	    pin_Result(idx).set_name("Result", idx);
      }
}

NetAddSub::~NetAddSub()
{
}

unsigned NetAddSub::width()const
{
      return (pin_count() - 6) / 3;
}

NetObj::Link& NetAddSub::pin_Cout()
{
      return pin(4);
}

const NetObj::Link& NetAddSub::pin_Cout() const
{
      return pin(4);
}

NetObj::Link& NetAddSub::pin_DataA(unsigned idx)
{
      idx = 6 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const NetObj::Link& NetAddSub::pin_DataA(unsigned idx) const
{
      idx = 6 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

NetObj::Link& NetAddSub::pin_DataB(unsigned idx)
{
      idx = 7 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const NetObj::Link& NetAddSub::pin_DataB(unsigned idx) const
{
      idx = 7 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

NetObj::Link& NetAddSub::pin_Result(unsigned idx)
{
      idx = 8 + idx*3;
      assert(idx < pin_count());
      return pin(idx);
}

const NetObj::Link& NetAddSub::pin_Result(unsigned idx) const
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
NetCLShift::NetCLShift(const string&n, unsigned width, unsigned width_dist)
: NetNode(n, 3+2*width+width_dist), width_(width), width_dist_(width_dist)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("Direction", 0);
      pin(1).set_dir(NetObj::Link::OUTPUT); pin(1).set_name("Underflow", 0);
      pin(2).set_dir(NetObj::Link::OUTPUT); pin(2).set_name("Overflow", 0);

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin(3+idx).set_dir(NetObj::Link::INPUT);
	    pin(3+idx).set_name("Data", idx);

	    pin(3+width_+idx).set_dir(NetObj::Link::OUTPUT);
	    pin(3+width_+idx).set_name("Result", idx);
      }

      for (unsigned idx = 0 ;  idx < width_dist_ ;  idx += 1) {
	    pin(3+2*width_+idx).set_dir(NetObj::Link::INPUT);
	    pin(3+2*width_+idx).set_name("Distance", idx);
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

NetObj::Link& NetCLShift::pin_Direction()
{
      return pin(0);
}

const NetObj::Link& NetCLShift::pin_Direction() const
{
      return pin(0);
}

NetObj::Link& NetCLShift::pin_Underflow()
{
      return pin(1);
}

const NetObj::Link& NetCLShift::pin_Underflow() const
{
      return pin(1);
}

NetObj::Link& NetCLShift::pin_Overflow()
{
      return pin(2);
}

const NetObj::Link& NetCLShift::pin_Overflow() const
{
      return pin(2);
}

NetObj::Link& NetCLShift::pin_Data(unsigned idx)
{
      assert(idx < width_);
      return pin(3+idx);
}

const NetObj::Link& NetCLShift::pin_Data(unsigned idx) const
{
      assert(idx < width_);
      return pin(3+idx);
}

NetObj::Link& NetCLShift::pin_Result(unsigned idx)
{
      assert(idx < width_);
      return pin(3+width_+idx);
}

const NetObj::Link& NetCLShift::pin_Result(unsigned idx) const
{
      assert(idx < width_);
      return pin(3+width_+idx);
}

NetObj::Link& NetCLShift::pin_Distance(unsigned idx)
{
      assert(idx < width_dist_);
      return pin(3+2*width_+idx);
}

const NetObj::Link& NetCLShift::pin_Distance(unsigned idx) const
{
      assert(idx < width_dist_);
      return pin(3+2*width_+idx);
}

NetCompare::NetCompare(const string&n, unsigned wi)
: NetNode(n, 8+2*wi), width_(wi)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("Aclr");
      pin(1).set_dir(NetObj::Link::INPUT); pin(1).set_name("Clock");
      pin(2).set_dir(NetObj::Link::OUTPUT); pin(2).set_name("AGB");
      pin(3).set_dir(NetObj::Link::OUTPUT); pin(3).set_name("AGEB");
      pin(4).set_dir(NetObj::Link::OUTPUT); pin(4).set_name("AEB");
      pin(5).set_dir(NetObj::Link::OUTPUT); pin(5).set_name("ANEB");
      pin(6).set_dir(NetObj::Link::OUTPUT); pin(6).set_name("ALB");
      pin(7).set_dir(NetObj::Link::OUTPUT); pin(7).set_name("ALEB");
      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin(8+idx).set_dir(NetObj::Link::INPUT);
	    pin(8+idx).set_name("DataA", idx);
	    pin(8+width_+idx).set_dir(NetObj::Link::INPUT);
	    pin(8+width_+idx).set_name("DataB", idx);
      }
}

NetCompare::~NetCompare()
{
}

unsigned NetCompare::width() const
{
      return width_;
}

NetObj::Link& NetCompare::pin_Aclr()
{
      return pin(0);
}

const NetObj::Link& NetCompare::pin_Aclr() const
{
      return pin(0);
}

NetObj::Link& NetCompare::pin_Clock()
{
      return pin(1);
}

const NetObj::Link& NetCompare::pin_Clock() const
{
      return pin(1);
}

NetObj::Link& NetCompare::pin_AGB()
{
      return pin(2);
}

const NetObj::Link& NetCompare::pin_AGB() const
{
      return pin(2);
}

NetObj::Link& NetCompare::pin_AGEB()
{
      return pin(3);
}

const NetObj::Link& NetCompare::pin_AGEB() const
{
      return pin(3);
}

NetObj::Link& NetCompare::pin_AEB()
{
      return pin(4);
}

const NetObj::Link& NetCompare::pin_AEB() const
{
      return pin(4);
}

NetObj::Link& NetCompare::pin_ANEB()
{
      return pin(5);
}

const NetObj::Link& NetCompare::pin_ANEB() const
{
      return pin(5);
}

NetObj::Link& NetCompare::pin_ALB()
{
      return pin(6);
}

const NetObj::Link& NetCompare::pin_ALB() const
{
      return pin(6);
}

NetObj::Link& NetCompare::pin_ALEB()
{
      return pin(7);
}

const NetObj::Link& NetCompare::pin_ALEB() const
{
      return pin(7);
}

NetObj::Link& NetCompare::pin_DataA(unsigned idx)
{
      return pin(8+idx);
}

const NetObj::Link& NetCompare::pin_DataA(unsigned idx) const
{
      return pin(8+idx);
}

NetObj::Link& NetCompare::pin_DataB(unsigned idx)
{
      return pin(8+width_+idx);
}

const NetObj::Link& NetCompare::pin_DataB(unsigned idx) const
{
      return pin(8+width_+idx);
}

NetMult::NetMult(const string&n, unsigned wr, unsigned wa, unsigned wb,
		 unsigned ws)
: NetNode(n, 2+wr+wa+wb+ws), width_r_(wr), width_a_(wa), width_b_(wb),
    width_s_(ws)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("Aclr", 0);
      pin(1).set_dir(NetObj::Link::INPUT); pin(1).set_name("Clock", 0);


      unsigned p = 2;
      for (unsigned idx = 0 ;  idx < width_r_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(NetObj::Link::OUTPUT);
	    pin(p).set_name("Result", idx);
      }
      for (unsigned idx = 0 ;  idx < width_a_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(NetObj::Link::INPUT);
	    pin(p).set_name("DataA", idx);
      }
      for (unsigned idx = 0 ;  idx < width_b_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(NetObj::Link::INPUT);
	    pin(p).set_name("DataB", idx);
      }
      for (unsigned idx = 0 ;  idx < width_s_ ;  idx += 1, p += 1) {
	    pin(p).set_dir(NetObj::Link::INPUT);
	    pin(p).set_name("Sum", idx);
      }
}

NetMult::~NetMult()
{
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

NetObj::Link& NetMult::pin_Aclr()
{
      return pin(0);
}

const NetObj::Link& NetMult::pin_Aclr() const
{
      return pin(0);
}

NetObj::Link& NetMult::pin_Clock()
{
      return pin(1);
}

const NetObj::Link& NetMult::pin_Clock() const
{
      return pin(1);
}

NetObj::Link& NetMult::pin_Result(unsigned idx)
{
      assert(idx < width_r_);
      return pin(idx+2);
}

const NetObj::Link& NetMult::pin_Result(unsigned idx) const
{
      assert(idx < width_r_);
      return pin(idx+2);
}

NetObj::Link& NetMult::pin_DataA(unsigned idx)
{
      assert(idx < width_a_);
      return pin(idx+2+width_r_);
}

const NetObj::Link& NetMult::pin_DataA(unsigned idx) const
{
      assert(idx < width_a_);
      return pin(idx+2+width_r_);
}

NetObj::Link& NetMult::pin_DataB(unsigned idx)
{
      assert(idx < width_b_);
      return pin(idx+2+width_r_+width_a_);
}

const NetObj::Link& NetMult::pin_DataB(unsigned idx) const
{
      assert(idx < width_b_);
      return pin(idx+2+width_r_+width_a_);
}

NetObj::Link& NetMult::pin_Sum(unsigned idx)
{
      assert(idx < width_s_);
      return pin(idx+2+width_r_+width_a_+width_b_);
}

const NetObj::Link& NetMult::pin_Sum(unsigned idx) const
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

NetMux::NetMux(const string&n, unsigned wi, unsigned si, unsigned sw)
: NetNode(n, 2+wi+sw+wi*si), width_(wi), size_(si), swidth_(sw)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("Aclr",  0);
      pin(1).set_dir(NetObj::Link::INPUT); pin(1).set_name("Clock", 0);

      for (unsigned idx = 0 ;  idx < width_ ;  idx += 1) {
	    pin_Result(idx).set_dir(NetObj::Link::OUTPUT);
	    pin_Result(idx).set_name("Result", idx);

	    for (unsigned jdx = 0 ;  jdx < size_ ;  jdx += 1) {
		  pin_Data(idx,jdx).set_dir(Link::INPUT);
		  pin_Data(idx,jdx).set_name("Data", jdx*width_+idx);
	    }
      }

      for (unsigned idx = 0 ;  idx < swidth_ ;  idx += 1) {
	    pin_Sel(idx).set_dir(Link::INPUT);
	    pin_Sel(idx).set_name("Sel", idx);
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

NetObj::Link& NetMux::pin_Aclr()
{
      return pin(0);
}

const NetObj::Link& NetMux::pin_Aclr() const
{
      return pin(0);
}

NetObj::Link& NetMux::pin_Clock()
{
      return pin(1);
}

const NetObj::Link& NetMux::pin_Clock() const
{
      return pin(1);
}

NetObj::Link& NetMux::pin_Result(unsigned w)
{
      assert(w < width_);
      return pin(2+w);
}

const NetObj::Link& NetMux::pin_Result(unsigned w) const
{
      assert(w < width_);
      return pin(2+w);
}

NetObj::Link& NetMux::pin_Sel(unsigned w)
{
      assert(w < swidth_);
      return pin(2+width_+w);
}

const NetObj::Link& NetMux::pin_Sel(unsigned w) const
{
      assert(w < swidth_);
      return pin(2+width_+w);
}

NetObj::Link& NetMux::pin_Data(unsigned w, unsigned s)
{
      assert(w < width_);
      assert(s < size_);
      return pin(2+width_+swidth_+s*width_+w);
}

const NetObj::Link& NetMux::pin_Data(unsigned w, unsigned s) const
{
      assert(w < width_);
      assert(s < size_);
      return pin(2+width_+swidth_+s*width_+w);
}


NetRamDq::NetRamDq(const string&n, NetMemory*mem, unsigned awid)
: NetNode(n, 3+2*mem->width()+awid), mem_(mem), awidth_(awid)
{
      pin(0).set_dir(NetObj::Link::INPUT); pin(0).set_name("InClock", 0);
      pin(1).set_dir(NetObj::Link::INPUT); pin(1).set_name("OutClock", 0);
      pin(2).set_dir(NetObj::Link::INPUT); pin(2).set_name("WE", 0);

      for (unsigned idx = 0 ;  idx < awidth_ ;  idx += 1) {
	    pin(3+idx).set_dir(NetObj::Link::INPUT);
	    pin(3+idx).set_name("Address", idx);
      }

      for (unsigned idx = 0 ;  idx < width() ;  idx += 1) {
	    pin(3+awidth_+idx).set_dir(NetObj::Link::INPUT);
	    pin(3+awidth_+idx).set_name("Data", idx);
      }

      for (unsigned idx = 0 ;  idx < width() ;  idx += 1) {
	    pin(3+awidth_+width()+idx).set_dir(NetObj::Link::OUTPUT);
	    pin(3+awidth_+width()+idx).set_name("Q", idx);
      }

      next_ = mem_->ram_list_;
      mem_->ram_list_ = this;
}

NetRamDq::~NetRamDq()
{
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

unsigned NetRamDq::width() const
{
      return mem_->width();
}

unsigned NetRamDq::awidth() const
{
      return awidth_;
}

unsigned NetRamDq::size() const
{
      return mem_->count();
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

NetObj::Link& NetRamDq::pin_InClock()
{
      return pin(0);
}

const NetObj::Link& NetRamDq::pin_InClock() const
{
      return pin(0);
}

NetObj::Link& NetRamDq::pin_OutClock()
{
      return pin(1);
}

const NetObj::Link& NetRamDq::pin_OutClock() const
{
      return pin(1);
}

NetObj::Link& NetRamDq::pin_WE()
{
      return pin(2);
}

const NetObj::Link& NetRamDq::pin_WE() const
{
      return pin(2);
}

NetObj::Link& NetRamDq::pin_Address(unsigned idx)
{
      assert(idx < awidth_);
      return pin(3+idx);
}

const NetObj::Link& NetRamDq::pin_Address(unsigned idx) const
{
      assert(idx < awidth_);
      return pin(3+idx);
}

NetObj::Link& NetRamDq::pin_Data(unsigned idx)
{
      assert(idx < width());
      return pin(3+awidth_+idx);
}

const NetObj::Link& NetRamDq::pin_Data(unsigned idx) const
{
      assert(idx < width());
      return pin(3+awidth_+idx);
}

NetObj::Link& NetRamDq::pin_Q(unsigned idx)
{
      assert(idx < width());
      return pin(3+awidth_+width()+idx);
}

const NetObj::Link& NetRamDq::pin_Q(unsigned idx) const
{
      assert(idx < width());
      return pin(3+awidth_+width()+idx);
}

/*
 * NetAssign
 */

NetAssign_::NetAssign_(const string&n, unsigned w)
: NetNode(n, w), rval_(0), bmux_(0)
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(NetObj::Link::OUTPUT);
	    pin(idx).set_name("P", idx);
      }

}

NetAssign_::~NetAssign_()
{
      if (rval_) delete rval_;
      if (bmux_) delete bmux_;
}

void NetAssign_::set_rval(NetExpr*r)
{
      assert(rval_ == 0);
      rval_ = r;
}

void NetAssign_::set_bmux(NetExpr*r)
{
      assert(bmux_ == 0);
      bmux_ = r;
}

NetExpr* NetAssign_::rval()
{
      return rval_;
}

const NetExpr* NetAssign_::rval() const
{
      return rval_;
}

const NetExpr* NetAssign_::bmux() const
{
      return bmux_;
}

NetAssign::NetAssign(const string&n, Design*des, unsigned w, NetExpr*rv)
: NetAssign_(n, w)
{
      set_rval(rv);
}

NetAssign::NetAssign(const string&n, Design*des, unsigned w,
		     NetExpr*mu, NetExpr*rv)
: NetAssign_(n, w)
{
      bool flag = rv->set_width(1);
      if (flag == false) {
	    cerr << rv->get_line() << ": Expression bit width" <<
		  " conflicts with l-value bit width." << endl;
	    des->errors += 1;
      }

      set_rval(rv);
      set_bmux(mu);
}

NetAssign::~NetAssign()
{
}

NetAssignNB::NetAssignNB(const string&n, Design*des, unsigned w, NetExpr*rv)
: NetAssign_(n, w)
{
      if (rv->expr_width() < w) {
	    cerr << rv->get_line() << ": Expression bit width (" <<
		  rv->expr_width() << ") conflicts with l-value "
		  "bit width (" << w << ")." << endl;
	    des->errors += 1;
      }

      set_rval(rv);
}

NetAssignNB::NetAssignNB(const string&n, Design*des, unsigned w,
			 NetExpr*mu, NetExpr*rv)
: NetAssign_(n, w)
{
      bool flag = rv->set_width(1);
      if (flag == false) {
	    cerr << rv->get_line() << ": Expression bit width" <<
		  " conflicts with l-value bit width." << endl;
	    des->errors += 1;
      }

      set_rval(rv);
      set_bmux(mu);
}

NetAssignNB::~NetAssignNB()
{
}


NetAssignMem_::NetAssignMem_(NetMemory*m, NetNet*i, NetExpr*r)
: mem_(m), index_(i), rval_(r)
{
      index_->incr_eref();
}

NetAssignMem_::~NetAssignMem_()
{
      index_->decr_eref();
}

NetAssignMem::NetAssignMem(NetMemory*m, NetNet*i, NetExpr*r)
: NetAssignMem_(m, i, r)
{
}

NetAssignMem::~NetAssignMem()
{
}

NetAssignMemNB::NetAssignMemNB(NetMemory*m, NetNet*i, NetExpr*r)
: NetAssignMem_(m, i, r)
{
}

NetAssignMemNB::~NetAssignMemNB()
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

NetBUFZ::NetBUFZ(const string&n)
: NetNode(n, 2)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(1).set_dir(Link::INPUT);
      pin(0).set_name("O", 0);
      pin(1).set_name("I", 0);
}

NetBUFZ::~NetBUFZ()
{
}


NetCase::NetCase(NetCase::TYPE c, NetExpr*ex, unsigned cnt)
: type_(c), expr_(ex), nitems_(cnt)
{
      assert(expr_);
      items_ = new Item[nitems_];
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    items_[idx].statement = 0;
      }
}

NetCase::~NetCase()
{
      delete expr_;
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    delete items_[idx].guard;
	    if (items_[idx].statement) delete items_[idx].statement;
      }
      delete[]items_;
}

NetCase::TYPE NetCase::type() const
{
      return type_;
}

void NetCase::set_case(unsigned idx, NetExpr*e, NetProc*p)
{
      assert(idx < nitems_);
      items_[idx].guard = e;
      items_[idx].statement = p;
      if (items_[idx].guard)
	    items_[idx].guard->set_width(expr_->expr_width());
}

NetCaseCmp::NetCaseCmp(const string&n)
: NetNode(n, 3)
{
      pin(0).set_dir(Link::OUTPUT); pin(0).set_name("O",0);
      pin(1).set_dir(Link::INPUT); pin(1).set_name("I",0);
      pin(2).set_dir(Link::INPUT); pin(2).set_name("I",1);
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

NetProc* NetCondit::if_clause()
{
      return if_;
}

NetProc* NetCondit::else_clause()
{
      return else_;
}

NetConst::NetConst(const string&n, verinum::V v)
: NetNode(n, 1)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name("O", 0);
      value_ = new verinum::V[1];
      value_[0] = v;
}

NetConst::NetConst(const string&n, const verinum&val)
: NetNode(n, val.len())
{
      value_ = new verinum::V[pin_count()];
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    pin(idx).set_dir(Link::OUTPUT);
	    pin(idx).set_name("O", idx);
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

NetFuncDef::NetFuncDef(NetScope*s, const svector<NetNet*>&po)
: scope_(s), statement_(0), ports_(po)
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

NetNEvent::NetNEvent(const string&ev, unsigned wid, Type e, NetPEvent*pe)
: NetNode(ev, wid), sref<NetPEvent,NetNEvent>(pe), edge_(e)
{
      for (unsigned idx = 0 ;  idx < wid ; idx += 1) {
	    pin(idx).set_name("P", idx);
      }
}

NetNEvent::~NetNEvent()
{
}

NetPEvent::NetPEvent(const string&n, NetProc*st)
: name_(n), statement_(st)
{
}

NetPEvent::~NetPEvent()
{
      svector<NetNEvent*>*back = back_list();
      if (back) {
	    for (unsigned idx = 0 ;  idx < back->count() ;  idx += 1) {
		  NetNEvent*ne = (*back)[idx];
		  delete ne;
	    }
	    delete back;
      }

      delete statement_;
}

NetProc* NetPEvent::statement()
{
      return statement_;
}

const NetProc* NetPEvent::statement() const
{
      return statement_;
}

NetSTask::NetSTask(const string&na, const svector<NetExpr*>&pa)
: name_(na), parms_(pa)
{
      assert(name_[0] == '$');
}

NetSTask::~NetSTask()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];

}

const NetExpr* NetSTask::parm(unsigned idx) const
{
      return parms_[idx];
}

NetEUFunc::NetEUFunc(NetFuncDef*def, NetESignal*res, svector<NetExpr*>&p)
: func_(def), result_(res), parms_(p)
{
      expr_width(result_->expr_width());
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

const NetESignal*NetEUFunc::result() const
{
      return result_;
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

const NetFuncDef* NetEUFunc::definition() const
{
      return func_;
}

NetEUFunc* NetEUFunc::dup_expr() const
{
      assert(0);
      return 0;
}

NetUTask::NetUTask(NetTaskDef*def)
: task_(def)
{
}

NetUTask::~NetUTask()
{
}

NetExpr::NetExpr(unsigned w)
: width_(w)
{
}

NetExpr::~NetExpr()
{
}

NetEBAdd::NetEBAdd(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      if (l->expr_width() > r->expr_width())
	    r->set_width(l->expr_width());

      if (r->expr_width() > l->expr_width())
	    l->set_width(r->expr_width());

      if (l->expr_width() < r->expr_width())
	    r->set_width(l->expr_width());

      if (r->expr_width() < l->expr_width())
	    l->set_width(r->expr_width());

      if (r->expr_width() != l->expr_width())
	    expr_width(0);
      else
	    expr_width(l->expr_width());
}

NetEBAdd::~NetEBAdd()
{
}

NetEBAdd* NetEBAdd::dup_expr() const
{
      NetEBAdd*result = new NetEBAdd(op_, left_->dup_expr(),
				     right_->dup_expr());
      return result;
}

NetEBBits::NetEBBits(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
	/* First try to naturally adjust the size of the
	   expressions to match. */
      if (l->expr_width() > r->expr_width())
	    r->set_width(l->expr_width());

      if (r->expr_width() > l->expr_width())
	    l->set_width(r->expr_width());

      if (l->expr_width() < r->expr_width())
	    r->set_width(l->expr_width());

      if (r->expr_width() < l->expr_width())
	    l->set_width(r->expr_width());

	/* If the expressions cannot be matched, pad them to fit. */
      if (l->expr_width() > r->expr_width())
	    right_ = pad_to_width(r, l->expr_width());

      if (r->expr_width() > l->expr_width())
	    left_ = pad_to_width(l, r->expr_width());

      assert(left_->expr_width() == right_->expr_width());
      expr_width(left_->expr_width());
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

NetEBComp::NetEBComp(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(1);
}

NetEBComp::~NetEBComp()
{
}

NetEBComp* NetEBComp::dup_expr() const
{
      NetEBComp*result = new NetEBComp(op_, left_->dup_expr(),
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

NetEBinary* NetEBinary::dup_expr() const
{
      assert(0);
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

NetEBMult::NetEBMult(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(l->expr_width() + r->expr_width());
}

NetEBMult::~NetEBMult()
{
}

NetEBMult* NetEBMult::dup_expr() const
{
      NetEBMult*result = new NetEBMult(op_, left_->dup_expr(),
				       right_->dup_expr());
      return result;
}

NetEBShift::NetEBShift(char op, NetExpr*l, NetExpr*r)
: NetEBinary(op, l, r)
{
      expr_width(l->expr_width());
}

NetEBShift::~NetEBShift()
{
}

NetEBShift* NetEBShift::dup_expr() const
{
      NetEBShift*result = new NetEBShift(op_, left_->dup_expr(),
					 right_->dup_expr());
      return result;
}

NetEConcat::NetEConcat(unsigned cnt, unsigned r)
: parms_(cnt), repeat_(r)
{
      expr_width(0);
}

NetEConcat::~NetEConcat()
{
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    delete parms_[idx];
}

void NetEConcat::set(unsigned idx, NetExpr*e)
{
      assert(idx < parms_.count());
      assert(parms_[idx] == 0);
      parms_[idx] = e;
      expr_width( expr_width() + repeat_*e->expr_width() );
}

NetEConcat* NetEConcat::dup_expr() const
{
      NetEConcat*dup = new NetEConcat(parms_.count(), repeat_);
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    if (parms_[idx]) {
		  assert(parms_[idx]->dup_expr());
		  dup->parms_[idx] = parms_[idx]->dup_expr();
	    }


      dup->expr_width(expr_width());
      return dup;
}

NetEConst::NetEConst(const verinum&val)
: NetExpr(val.len()), value_(val)
{
}

NetEConst::~NetEConst()
{
}

NetEConst* NetEConst::dup_expr() const
{
      NetEConst*tmp = new NetEConst(value_);
      tmp->set_line(*this);
      return tmp;
}

NetEIdent* NetEIdent::dup_expr() const
{
      assert(0);
}

NetEMemory::NetEMemory(NetMemory*m, NetExpr*i)
: NetExpr(m->width()), mem_(m), idx_(i)
{
}

NetEMemory::~NetEMemory()
{
}

NetMemory::NetMemory(const string&n, long w, long s, long e)
: name_(n), width_(w), idxh_(s), idxl_(e), ram_list_(0)
{
}

unsigned NetMemory::count() const
{
      if (idxh_ < idxl_)
	    return idxl_ - idxh_ + 1;
      else
	    return idxh_ - idxl_ + 1;
}

unsigned NetMemory::index_to_address(long idx) const
{
      if (idxh_ < idxl_)
	    return idx - idxh_;
      else
	    return idx - idxl_;
}


void NetMemory::set_attributes(const map<string,string>&attr)
{
      assert(attributes_.size() == 0);
      attributes_ = attr;
}

NetEMemory* NetEMemory::dup_expr() const
{
      assert(0);
}

NetEParam::NetEParam()
: des_(0)
{
}

NetEParam::NetEParam(Design*d, NetScope*s, const string&n)
: des_(d), scope_(s), name_(n)
{
}

NetEParam::~NetEParam()
{
}

NetEParam* NetEParam::dup_expr() const
{
      return 0;
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
      net_->incr_eref();
      set_line(*n);
}

NetESignal::~NetESignal()
{
      net_->decr_eref();
}

const string& NetESignal::name() const
{
      return net_->name();
}

unsigned NetESignal::pin_count() const
{
      return net_->pin_count();
}

NetObj::Link& NetESignal::pin(unsigned idx)
{
      return net_->pin(idx);
}

NetESignal* NetESignal::dup_expr() const
{
      assert(0);
}

NetESubSignal::NetESubSignal(NetESignal*sig, NetExpr*ex)
: sig_(sig), idx_(ex)
{
	// This supports mux type indexing of an expression, so the
	// with is by definition 1 bit.
      expr_width(1);
}

NetESubSignal::~NetESubSignal()
{
      delete idx_;
}

NetESubSignal* NetESubSignal::dup_expr() const
{
      assert(0);
}

NetETernary::NetETernary(NetExpr*c, NetExpr*t, NetExpr*f)
: cond_(c), true_val_(t), false_val_(f)
{
      expr_width(true_val_->expr_width());
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

NetETernary* NetETernary::dup_expr() const
{
      assert(0);
}

NetEUnary::NetEUnary(char op, NetExpr*ex)
: NetExpr(ex->expr_width()), op_(op), expr_(ex)
{
      switch (op_) {
	  case '!': // Logical not
	  case '&': // Reduction and
	  case '|': // Reduction or
	  case '^': // Reduction XOR
	  case 'A': // Reduction NAND (~&)
	  case 'N': // Reduction NOR (~|)
	  case 'X': // Reduction NXOR (~^)
	    expr_width(1);
	    break;
      }
}

NetEUnary::~NetEUnary()
{
      delete expr_;
}

NetEUnary* NetEUnary::dup_expr() const
{
      assert(0);
}

NetEUBits::NetEUBits(char op, NetExpr*ex)
: NetEUnary(op, ex)
{
}

NetEUBits::~NetEUBits()
{
}

NetForever::NetForever(NetProc*p)
: statement_(p)
{
}

NetForever::~NetForever()
{
      delete statement_;
}

NetLogic::NetLogic(const string&n, unsigned pins, TYPE t)
: NetNode(n, pins), type_(t)
{
      pin(0).set_dir(Link::OUTPUT);
      pin(0).set_name("O", 0);
      for (unsigned idx = 1 ;  idx < pins ;  idx += 1) {
	    pin(idx).set_dir(Link::INPUT);
	    pin(idx).set_name("I", idx-1);
      }
}

NetRepeat::NetRepeat(NetExpr*e, NetProc*p)
: expr_(e), statement_(p)
{
}

NetRepeat::~NetRepeat()
{
      delete expr_;
      delete statement_;
}

const NetExpr* NetRepeat::expr() const
{
      return expr_;
}

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
}

NetScope::NetScope(NetScope*up, const string&n, NetScope::TYPE t)
: type_(t), name_(n), up_(up), sib_(0), sub_(0)
{
      sib_ = up_->sub_;
      up_->sub_ = this;
}

NetScope::~NetScope()
{
      assert(sib_ == 0);
      assert(sub_ == 0);
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

string NetScope::name() const
{
      if (up_)
	    return up_->name() + "." + name_;
      else
	    return name_;
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

NetNet* NetTaskDef::port(unsigned idx)
{
      assert(idx < ports_.count());
      return ports_[idx];
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

/*
 * $Log: netlist.cc,v $
 * Revision 1.109  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.108  2000/03/12 17:09:41  steve
 *  Support localparam.
 *
 * Revision 1.107  2000/03/10 06:20:48  steve
 *  Handle defparam to partial hierarchical names.
 *
 * Revision 1.106  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.105  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.104  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.103  2000/01/10 01:35:24  steve
 *  Elaborate parameters afer binding of overrides.
 *
 * Revision 1.102  1999/12/30 04:19:12  steve
 *  Propogate constant 0 in low bits of adders.
 */

