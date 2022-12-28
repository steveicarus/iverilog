/*
 * Copyright (c) 2000-2022 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <iostream>

# include  "netlist.h"
# include  <sstream>
# include  <cstring>
# include  <string>
# include  <typeinfo>
# include  <cstdlib>
# include  "ivl_alloc.h"

using namespace std;

void Nexus::connect(Link&r)
{
      Nexus*r_nexus = r.next_? r.find_nexus_() : NULL;
      if (this == r_nexus)
	    return;

      delete[] name_;
      name_ = 0;

	// Special case: This nexus is empty. Simply copy all the
	// links of the other nexus to this one, and delete the old
	// nexus.
      if (list_ == 0) {
	    if (r.next_ == 0) {
		  list_ = &r;
		  r.next_ = &r;
		  r.nexus_ = this;
		  driven_ = NO_GUESS;
	    } else {
		  driven_ = r_nexus->driven_;
		  list_ = r_nexus->list_;
		  list_->nexus_ = this;
		  r_nexus->list_ = 0;
		  delete r_nexus;
	    }
	    return;
      }

	// Special case: The Link is unconnected. Put it at the end of
	// the current list and move the list_ pointer and nexus_ back
	// pointer to suit.
      if (r.next_ == 0) {
	    if (r.get_dir() != Link::INPUT)
		  driven_ = NO_GUESS;

	    r.nexus_ = this;
	    r.next_ = list_->next_;
	    list_->next_ = &r;
	    list_->nexus_ = 0;
	    list_ = &r;
	    return;
      }

      if (r_nexus->driven_ != Vz)
	    driven_ = NO_GUESS;

	// Splice the list of links from the "tmp" nexus to the end of
	// this nexus. Adjust the nexus pointers as needed.
      Link*save_first = list_->next_;
      list_->next_ = r_nexus->list_->next_;
      r_nexus->list_->next_ = save_first;
      list_->nexus_ = 0;
      list_ = r_nexus->list_;
      list_->nexus_ = this;

      r_nexus->list_ = 0;
      delete r_nexus;
}

void connect(Link&l, Link&r)
{
      Nexus*tmp;
      assert(&l != &r);
	// If either the l or r link already are part of a Nexus, then
	// re-use that nexus. Go through some effort so that we are
	// not gratuitously creating Nexus object.
      if (l.next_ && (tmp=l.find_nexus_())) {
	    connect(tmp, r);
      } else if (r.next_ && (tmp=r.find_nexus_())) {
	    connect(tmp, l);
      } else {
	      // No existing Nexus (both links are so far unconnected)
	      // so start one.
	    tmp = new Nexus(l);
	    tmp->connect(r);
      }
}

Link::Link()
: dir_(PASSIVE), drive0_(IVL_DR_STRONG), drive1_(IVL_DR_STRONG),
  next_(0), nexus_(0)
{
      node_ = 0;
      pin_zero_ = true;
}

Link::~Link()
{
      if (next_) {
	    Nexus*tmp = nexus();
	    tmp->unlink(this);
	    if (tmp->list_ == 0)
		  delete tmp;
      }
}

Nexus* Link::find_nexus_() const
{
      assert(next_);
      if (nexus_) return nexus_;
      for (Link*cur = next_ ; cur != this ; cur = cur->next_) {
	    if (cur->nexus_) return cur->nexus_;
      }
      return 0;
}

Nexus* Link::nexus()
{
      if (next_ == 0) {
	    assert(nexus_ == 0);
	    Nexus*tmp = new Nexus(*this);
	    return tmp;
      }

      return find_nexus_();
}

const Nexus* Link::nexus() const
{
      if (next_ == 0) return 0;
      return find_nexus_();
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
      find_nexus_()->drivers_delays(rise, fall, decay);
}

void Link::drivers_drive(ivl_drive_t drive0__, ivl_drive_t drive1__)
{
      find_nexus_()->drivers_drive(drive0__, drive1__);
}


void Link::drive0(ivl_drive_t str)
{
      drive0_ = str;
}

void Link::drive1(ivl_drive_t str)
{
      drive1_ = str;
}

ivl_drive_t Link::drive0() const
{
      return drive0_;
}

ivl_drive_t Link::drive1() const
{
      return drive1_;
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
      if (! is_linked())
	    return;

      find_nexus_()->unlink(this);
}

bool Link::is_equal(const Link&that) const
{
      return (get_obj() == that.get_obj()) && (get_pin() == that.get_pin());
}

bool Link::is_linked() const
{
      if (next_ == 0)
	    return false;
      if (next_ == this)
	    return false;

      return true;
}

bool Link::is_linked(const Link&that) const
{
	// If this or that link is linked to nothing, then they cannot
	// be linked to each other.
      if (! this->is_linked())
	    return false;
      if (! that.is_linked())
	    return false;

      const Link*cur = next_;
      while (cur != this) {
	    if (cur == &that) return true;
	    cur = cur->next_;
      }

      return false;
}

Nexus::Nexus(Link&that)
{
      name_ = 0;
      driven_ = NO_GUESS;
      t_cookie_ = 0;

      if (that.next_ == 0) {
	    list_ = &that;
	    that.next_ = &that;
	    that.nexus_ = this;
	    driven_ = NO_GUESS;

      } else {
	    Nexus*tmp = that.find_nexus_();
	    list_ = tmp->list_;
	    list_->nexus_ = this;
	    driven_ = tmp->driven_;
	    name_ = tmp->name_;

	    tmp->list_ = 0;
	    tmp->name_ = 0;
	    delete tmp;
      }
}

Nexus::~Nexus()
{
      assert(list_ == 0);
      delete[] name_;
}

bool Nexus::assign_lval() const
{
      for (const Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {

	    const NetPins*obj;
	    unsigned pin;
	    cur->cur_link(obj, pin);
	    const NetNet*net = dynamic_cast<const NetNet*> (obj);
	    if (net == 0)
		  continue;

	    if (net->peek_lref() > 0)
		  return true;
      }

      return false;
}

void Nexus::count_io(unsigned&inp, unsigned&out) const
{
      for (const Link*cur = first_nlink() ;  cur ; cur = cur->next_nlink()) {
	    switch (cur->get_dir()) {
		case Link::INPUT:
		  inp += 1;
		  break;
		case Link::OUTPUT:
		  out += 1;
		  break;
		default:
		  break;
	    }
      }
}

bool Nexus::has_floating_input() const
{
      bool found_input = false;
      for (const Link*cur = first_nlink() ;  cur ; cur = cur->next_nlink()) {
	    if (cur->get_dir() == Link::OUTPUT)
		  return false;

	    if (cur->get_dir() == Link::INPUT)
		  found_input = true;
      }

      return found_input;
}

bool Nexus::drivers_present() const
{
      for (const Link*cur = first_nlink() ;  cur ; cur = cur->next_nlink()) {
	    if (cur->get_dir() == Link::OUTPUT)
		  return true;

	    if (cur->get_dir() == Link::INPUT)
		  continue;

	      // Must be PASSIVE, so if it is some kind of net, see if
	      // it is the sort that might drive the nexus. Note that
	      // supply0/1 and tri0/1 nets are classified as OUTPUT.
	    const NetPins*obj;
	    unsigned pin;
	    cur->cur_link(obj, pin);
	    if (const NetNet*net = dynamic_cast<const NetNet*>(obj))
		  switch (net->type()) {
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
      for (Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {
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

void Nexus::drivers_drive(ivl_drive_t drive0, ivl_drive_t drive1)
{
      for (Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {
	    if (cur->get_dir() != Link::OUTPUT)
		  continue;

	    cur->drive0(drive0);
	    cur->drive1(drive1);
      }
}

void Nexus::unlink(Link*that)
{
      delete[] name_;
      name_ = 0;

      assert(that);

	// Special case: the Link is the only link in the nexus. In
	// this case, the unlink is trivial. Also clear the Nexus
	// pointers.
      if (that->next_ == that) {
	    assert(that->nexus_ == this);
	    assert(list_ == that);
	    list_ = 0;
	    driven_ = NO_GUESS;
	    that->nexus_ = 0;
	    that->next_ = 0;
	    return;
      }

	// If the link I'm removing was a driver for this nexus, then
	// cancel my guess of the driven value.
      if (that->get_dir() != Link::INPUT)
	    driven_ = NO_GUESS;

	// Look for the Link that points to "that". We know that there
	// will be one because the list is a circle. When we find the
	// prev pointer, then remove that from the list.
      Link*prev = list_;
      while (prev->next_ != that)
	    prev = prev->next_;

      prev->next_ = that->next_;

	// If "that" was the last item in the list, then change the
	// list_ pointer to point to the new end of the list.
      if (list_ == that) {
	    assert(that->nexus_ == this);
	    list_ = prev;
	    list_->nexus_ = this;
      }

      that->nexus_ = 0;
      that->next_ = 0;
}

Link* Nexus::first_nlink()
{
      if (list_) return list_->next_;
      else return 0;
}

const Link* Nexus::first_nlink() const
{
      if (list_) return list_->next_;
      else return 0;
}

/*
 * The t_cookie can be set exactly once. This attaches an ivl_nexus_t
 * object to the Nexus, and causes the Link list to be marked up for
 * efficient use by the code generator. The change is to give all the
 * links a valid nexus_ pointer. This breaks most of the other
 * methods, but they are not used during code generation.
*/
void Nexus::t_cookie(ivl_nexus_t val) const
{
      assert(val && !t_cookie_);
      t_cookie_ = val;

      for (Link*cur = list_->next_ ; cur->nexus_ == 0 ; cur = cur->next_)
	    cur->nexus_ = const_cast<Nexus*> (this);
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

NetNode* Nexus::pick_any_node()
{
      for (Link*cur = first_nlink() ; cur ; cur = cur->next_nlink()) {
	    NetNode*node = dynamic_cast<NetNode*>(cur->get_obj());
	    if (node != 0)
		  return node;
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

	    ostringstream tmp;
	    tmp << "nex=" << this << ends;
	    const string tmps = tmp.str();
	    name_ = new char[strlen(tmps.c_str()) + 1];
	    strcpy(name_, tmps.c_str());
      } else {
	    assert(sig);
	    ostringstream tmp;
	    tmp << scope_path(sig->scope()) << "." << sig->name();
	    if (sig->pin_count() > 1)
		  tmp << "<" << pin << ">";
	    tmp << ends;

	    const string tmps = tmp.str();
	    name_ = new char[strlen(tmps.c_str()) + 1];
	    strcpy(name_, tmps.c_str());
      }

      return name_;
}


NexusSet::NexusSet()
{
}

NexusSet::~NexusSet()
{
      for (size_t idx = 0 ; idx < items_.size() ; idx += 1)
	    delete items_[idx];
}

size_t NexusSet::size() const
{
      return items_.size();
}

void NexusSet::add(Nexus*that, unsigned base, unsigned wid)
{
      assert(that);
      elem_t*cur = new elem_t(that, base, wid);

      if (items_.size() == 0) {
	    items_.resize(1);
	    items_[0] = cur;
	    return;
      }

      unsigned ptr = bsearch_(*cur);
      if (ptr < items_.size()) {
	    delete cur;
	    return;
      }

      assert(ptr == items_.size());

      items_.push_back(cur);
}

void NexusSet::add(NexusSet&that)
{
      for (size_t idx = 0 ;  idx < that.items_.size() ;  idx += 1)
	    add(that.items_[idx]->lnk.nexus(), that.items_[idx]->base, that.items_[idx]->wid);
}

void NexusSet::rem_(const NexusSet::elem_t*that)
{
      if (items_.empty())
	    return;

      unsigned ptr = bsearch_(*that);
      if (ptr >= items_.size())
	    return;

      if (items_.size() == 1) {
	    delete items_[0];
	    items_.clear();
	    return;
      }

      delete items_[ptr];
      for (unsigned idx = ptr ;  idx < (items_.size()-1) ;  idx += 1)
	    items_[idx] = items_[idx+1];

      items_.pop_back();
}

void NexusSet::rem(const NexusSet&that)
{
      for (size_t idx = 0 ;  idx < that.items_.size() ;  idx += 1)
	    rem_(that.items_[idx]);
}

unsigned NexusSet::find_nexus(const NexusSet::elem_t&that) const
{
      return bsearch_(that);
}

NexusSet::elem_t& NexusSet::at (unsigned idx)
{
      assert(idx <  items_.size());
      return *items_[idx];
}

size_t NexusSet::bsearch_(const NexusSet::elem_t&that) const
{
      for (unsigned idx = 0 ;  idx < items_.size() ;  idx += 1) {
	    if (*items_[idx] == that)
		  return idx;
      }

      return items_.size();
}

bool NexusSet::elem_t::contains(const struct elem_t&that) const
{
      if (! lnk.is_linked(that.lnk))
	    return false;
      if (that.base < base)
	    return false;
      if ((that.base+that.wid) > (base+wid))
	    return false;

      return true;
}

bool NexusSet::contains_(const NexusSet::elem_t&that) const
{
      for (unsigned idx = 0 ; idx < items_.size() ; idx += 1) {
	    if (items_[idx]->contains(that))
		  return true;
      }
      return false;
}

bool NexusSet::contains(const NexusSet&that) const
{
      for (size_t idx = 0 ;  idx < that.items_.size() ;  idx += 1) {
	    if (! contains_(*that.items_[idx]))
		return false;
      }

      return true;
}

bool NexusSet::intersect(const NexusSet&that) const
{
      for (size_t idx = 0 ;  idx < that.items_.size() ;  idx += 1) {
	    size_t where = bsearch_(*that.items_[idx]);
	    if (where == items_.size())
		  continue;

	    return true;
      }

      return false;
}
