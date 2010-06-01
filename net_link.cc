/*
 * Copyright (c) 2000-2008 Stephen Williams (steve@icarus.com)
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

# include  <iostream>

# include  "netlist.h"
# include  <sstream>
# include  <cstring>
# include  <string>
# include  <typeinfo>
# include  <cstdlib>

void connect(Nexus*l, Link&r)
{
      assert(l);

      if (l == r.nexus_)
	    return;

	// Special case: The "r" link is connected to nothing. The
	// connect becomes trivially easy.
      if (r.nexus_ == 0) {
	    l->relink(&r);
	    return;
      }

      Nexus*tmp = r.nexus_;
      while (Link*cur = tmp->list_) {
	    tmp->list_ = cur->next_;
	    cur->nexus_ = 0;
	    cur->next_  = 0;
	    l->relink(cur);
      }

      l->driven_ = Nexus::NO_GUESS;

      assert(tmp->list_ == 0);
      delete tmp;
}

void connect(Link&l, Link&r)
{
      assert(&l != &r);
      if (l.nexus_ != 0) {
	    connect(l.nexus_, r);
      } else if (r.nexus_ != 0) {
	    connect(r.nexus_, l);
      } else {
	    Nexus*tmp = new Nexus;
	    connect(tmp, l);
	    connect(tmp, r);
      }
}

Link::Link()
: dir_(PASSIVE), drive0_(STRONG), drive1_(STRONG), init_(verinum::Vx),
  next_(0), nexus_(0)
{
}

Link::~Link()
{
      if (Nexus*tmp = nexus_) {
	    nexus_->unlink(this);
	    if (tmp->list_ == 0)
		  delete tmp;
      }
}

Nexus* Link::nexus()
{
      if (nexus_ == 0)
	    (new Nexus()) ->relink(this);
      return nexus_;
}

const Nexus* Link::nexus() const
{
      return nexus_;
}

void Link::set_dir(DIR d)
{
      dir_ = d;
}

Link::DIR Link::get_dir() const
{
      return dir_;
}

void Link::drivers_delays(NetExpr*rise, NetExpr*fall, NetExpr*decay)
{
      nexus_->drivers_delays(rise, fall, decay);
}

void Link::drivers_drive(strength_t drive0__, strength_t drive1__)
{
      nexus_->drivers_drive(drive0__, drive1__);
}

void Link::drive0(Link::strength_t str)
{
      drive0_ = str;
}

void Link::drive1(Link::strength_t str)
{
      drive1_ = str;
}

Link::strength_t Link::drive0() const
{
      return drive0_;
}

Link::strength_t Link::drive1() const
{
      return drive1_;
}

void Link::set_init(verinum::V val)
{
      init_ = val;
}

verinum::V Link::get_init() const
{
      return init_;
}


void Link::cur_link(NetPins*&net, unsigned &pin)
{
      net = get_obj();
      pin = get_pin();
}

void Link::cur_link(const NetPins*&net, unsigned &pin) const
{
      net = get_obj();
      pin = get_pin();
}

void Link::unlink()
{
      assert(nexus_);
      if (! is_linked())
	    return;

      nexus_->unlink(this);
}

bool Link::is_equal(const Link&that) const
{
      return (get_obj() == that.get_obj()) && (get_pin() == that.get_pin());
}

bool Link::is_linked() const
{
      if (nexus_ == 0)
	    return false;
      if (next_)
	    return true;
      if (nexus_->first_nlink() != this)
	    return true;

      return false;
}

bool Link::is_linked(const Link&that) const
{
      if (nexus_ == 0)
	    return false;
      return nexus_ == that.nexus_;
}

Link* Link::next_nlink()
{
      return next_;
}

const Link* Link::next_nlink() const
{
      return next_;
}

const NetPins*Link::get_obj() const
{
      if (pin_zero_)
	    return node_;
      const Link*tmp = this - pin_;
      assert(tmp->pin_zero_);
      return tmp->node_;
}

NetPins*Link::get_obj()
{
      if (pin_zero_)
	    return node_;
      Link*tmp = this - pin_;
      assert(tmp->pin_zero_);
      return tmp->node_;
}

unsigned Link::get_pin() const
{
      if (pin_zero_)
	    return 0;
      else
	    return pin_;
}

Nexus::Nexus()
{
      name_ = 0;
      list_ = 0;
      driven_ = NO_GUESS;
      t_cookie_ = 0;
}

Nexus::~Nexus()
{
      assert(list_ == 0);
      if (name_)
	    delete[]name_;
}

verinum::V Nexus::get_init() const
{
      assert(list_);
      for (Link*cur = list_ ;  cur ;  cur = cur->next_) {
	    if (cur->get_dir() == Link::OUTPUT)
		  return verinum::Vx;

	    if ((cur->get_dir() == Link::PASSIVE)
		&& (cur->get_init() != verinum::Vz))
		  return cur->get_init();
      }

      return verinum::Vz;
}

bool Nexus::assign_lval() const
{
      assert(list_);
      for (Link*cur = list_ ; cur ; cur = cur->next_) {

	    const NetPins*obj;
	    unsigned pin;
	    cur->cur_link(obj, pin);
	    const NetNet*net = dynamic_cast<const NetNet*> (obj);
	    if (net == 0)
		  continue;

	    if (net->peek_lref())
		  return true;
      }

      return false;
}

bool Nexus::drivers_present() const
{
      assert(list_);
      for (Link*cur = list_ ;  cur ; cur = cur->next_) {
	    if (cur->get_dir() == Link::OUTPUT)
		  return true;

	    if (cur->get_dir() == Link::INPUT)
		  continue;

	      // Must be PASSIVE, so if it is some kind of net, see if
	      // it is the sort that might drive the nexus.
	    const NetPins*obj;
	    unsigned pin;
	    cur->cur_link(obj, pin);
	    if (const NetNet*net = dynamic_cast<const NetNet*>(obj))
		  switch (net->type()) {
		      case NetNet::SUPPLY0:
		      case NetNet::SUPPLY1:
		      case NetNet::TRI0:
		      case NetNet::TRI1:
		      case NetNet::WAND:
		      case NetNet::WOR:
		      case NetNet::TRIAND:
		      case NetNet::TRIOR:
		      case NetNet::REG:
			return true;
		      default:
			break;
		  }
      }

      return false;
}

void Nexus::drivers_delays(NetExpr*rise, NetExpr*fall, NetExpr*decay)
{
      for (Link*cur = list_ ; cur ; cur = cur->next_) {
	    if (cur->get_dir() != Link::OUTPUT)
		  continue;

	    NetObj*obj = dynamic_cast<NetObj*>(cur->get_obj());
	    if (obj == 0)
		  continue;

	    obj->rise_time(rise);
	    obj->fall_time(fall);
	    obj->decay_time(decay);
      }
}

void Nexus::drivers_drive(Link::strength_t drive0, Link::strength_t drive1)
{
      for (Link*cur = list_ ; cur ; cur = cur->next_) {
	    if (cur->get_dir() != Link::OUTPUT)
		  continue;

	    cur->drive0(drive0);
	    cur->drive1(drive1);
      }
}

void Nexus::unlink(Link*that)
{
      if (name_) {
	    delete[] name_;
	    name_ = 0;
      }

	/* If the link I'm removing was a driver for this nexus, then
	   cancel my guess of the driven value. */
      if (that->get_dir() != Link::INPUT)
	    driven_ = NO_GUESS;

      assert(that);
      if (list_ == that) {
	    list_ = that->next_;
	    that->next_ = 0;
	    that->nexus_ = 0;
	    return;
      }

      Link*cur = list_;
      while (cur->next_ != that) {
	    assert(cur->next_);
	    cur = cur->next_;
      }

      cur->next_ = that->next_;
      that->nexus_ = 0;
      that->next_ = 0;
}

void Nexus::relink(Link*that)
{
      if (name_) {
	    delete[] name_;
	    name_ = 0;
      }

	/* If the link I'm adding is a driver for this nexus, then
	   cancel my guess of the driven value. */
      if (that->get_dir() != Link::INPUT)
	    driven_ = NO_GUESS;

      assert(that->nexus_ == 0);
      assert(that->next_ == 0);
      that->next_ = list_;
      that->nexus_ = this;
      list_ = that;
}

Link* Nexus::first_nlink()
{
      return list_;
}

const Link* Nexus::first_nlink() const
{
      return list_;
}

ivl_nexus_t Nexus::t_cookie() const
{
      return t_cookie_;
}

ivl_nexus_t Nexus::t_cookie(ivl_nexus_t val)const
{
      ivl_nexus_t tmp = t_cookie_;
      t_cookie_ = val;
      return tmp;
}

unsigned Nexus::vector_width() const
{
      for (const Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {
	    const NetNet*sig = dynamic_cast<const NetNet*>(cur->get_obj());
	    if (sig == 0)
		  continue;

	    return sig->vector_width();
      }

      return 0;
}

NetNet* Nexus::pick_any_net()
{
      for (Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {
	    NetNet*sig = dynamic_cast<NetNet*>(cur->get_obj());
	    if (sig != 0)
		  return sig;
      }

      return 0;
}

const char* Nexus::name() const
{
      if (name_)
	    return name_;

      const NetNet*sig = 0;
      unsigned pin = 0;
      for (const Link*cur = first_nlink()
		 ;  cur  ;  cur = cur->next_nlink()) {

	    const NetNet*cursig = dynamic_cast<const NetNet*>(cur->get_obj());
	    if (cursig == 0)
		  continue;

	    if (sig == 0) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if ((cursig->pin_count() == 1) && (sig->pin_count() > 1))
		  continue;

	    if ((cursig->pin_count() > 1) && (sig->pin_count() == 1)) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if (cursig->local_flag() && !sig->local_flag())
		  continue;

	    if (cursig->name() < sig->name())
		  continue;

	    sig = cursig;
	    pin = cur->get_pin();
      }

      if (sig == 0) {
	    const Link*lnk = first_nlink();
	    const NetObj*obj = dynamic_cast<const NetObj*>(lnk->get_obj());
	    pin = lnk->get_pin();
	    cerr << "internal error: No signal for nexus of "
		 << obj->name() << " pin " << pin
		 << " type=" << typeid(*obj).name() << "?" << endl;

      }
      assert(sig);
      ostringstream tmp;
      tmp << scope_path(sig->scope()) << "." << sig->name();
      if (sig->pin_count() > 1)
	    tmp << "<" << pin << ">";

      const string tmps = tmp.str();
      name_ = new char[strlen(tmps.c_str()) + 1];
      strcpy(name_, tmps.c_str());
      return name_;
}


NexusSet::NexusSet()
{
      items_ = 0;
      nitems_ = 0;
}

NexusSet::~NexusSet()
{
      if (nitems_ > 0) {
	    assert(items_ != 0);
	    free(items_);
      } else {
	    assert(items_ == 0);
      }
}

unsigned NexusSet::count() const
{
      return nitems_;
}

void NexusSet::add(Nexus*that)
{
      assert(that);
      if (nitems_ == 0) {
	    assert(items_ == 0);
	    items_ = (Nexus**)malloc(sizeof(Nexus*));
	    items_[0] = that;
	    nitems_ = 1;
	    return;
      }

      unsigned ptr = bsearch_(that);
      if (ptr < nitems_) {
	    assert(items_[ptr] == that);
	    return;
      }

      assert(ptr == nitems_);

      items_ = (Nexus**)realloc(items_, (nitems_+1) * sizeof(Nexus*));
      items_[ptr] = that;
      nitems_ += 1;
}

void NexusSet::add(const NexusSet&that)
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1)
	    add(that.items_[idx]);
}

void NexusSet::rem(Nexus*that)
{
      assert(that);
      if (nitems_ == 0)
	    return;

      unsigned ptr = bsearch_(that);
      if (ptr >= nitems_)
	    return;

      if (nitems_ == 1) {
	    free(items_);
	    items_ = 0;
	    nitems_ = 0;
	    return;
      }

      for (unsigned idx = ptr ;  idx < (nitems_-1) ;  idx += 1)
	    items_[idx] = items_[idx+1];

      items_ = (Nexus**)realloc(items_, (nitems_-1) * sizeof(Nexus*));
      nitems_ -= 1;
}

void NexusSet::rem(const NexusSet&that)
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1)
	    rem(that.items_[idx]);
}

Nexus* NexusSet::operator[] (unsigned idx) const
{
      assert(idx < nitems_);
      return items_[idx];
}

unsigned NexusSet::bsearch_(Nexus*that) const
{
      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    if (items_[idx] == that)
		  return idx;
      }

      return nitems_;
}

bool NexusSet::contains(const NexusSet&that) const
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1) {
	    unsigned where = bsearch_(that[idx]);
	    if (where == nitems_)
		  return false;
	    if (items_[where] != that[idx])
		  return false;
      }

      return true;
}

bool NexusSet::intersect(const NexusSet&that) const
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1) {
	    unsigned where = bsearch_(that[idx]);
	    if (where == nitems_)
		  continue;
	    if (items_[where] == that[idx])
		  return true;
      }

      return false;
}
