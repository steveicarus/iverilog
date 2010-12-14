/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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
# include  <string>
# include  <cstring>
# include  <typeinfo>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif

void connect(Nexus*l, Link&r)
{
      assert(l);
      assert(r.nexus_);

      if (l == r.nexus_)
	    return;

      Nexus*tmp = r.nexus_;

      while (Link*cur = tmp->list_) {
	    tmp->list_ = cur->next_;
	    cur->nexus_ = 0;
	    cur->next_  = 0;
	    cur->prev_  = 0;
	    l->relink(cur);
      }

      delete tmp;
}

void connect(Link&l, Link&r)
{
      assert(&l != &r);
      if (r.is_linked() && !l.is_linked())
	    connect(r.nexus_, l);
      else if (r.nexus_->list_len_ > l.nexus_->list_len_)
	    connect(r.nexus_, l);
      else
	    connect(l.nexus_, r);
}

Link::Link()
: dir_(PASSIVE), drive0_(STRONG), drive1_(STRONG), init_(verinum::Vx),
  inst_(0), next_(0), prev_(0), nexus_(0)
{
      (new Nexus()) -> relink(this);
}

Link::~Link()
{
      assert(nexus_);
      Nexus*tmp = nexus_;
      nexus_->unlink(this);
      if (tmp->list_ == 0)
	    delete tmp;
}

Nexus* Link::nexus()
{
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


void Link::cur_link(NetObj*&net, unsigned &pin)
{
      net = node_;
      pin = pin_;
}

void Link::cur_link(const NetObj*&net, unsigned &pin) const
{
      net = node_;
      pin = pin_;
}

void Link::unlink()
{
      assert(nexus_);
      if (! is_linked())
	    return;

      nexus_->unlink(this);
      (new Nexus()) -> relink(this);
}

bool Link::is_equal(const Link&that) const
{
      return (node_ == that.node_) && (pin_ == that.pin_);
}

bool Link::is_linked() const
{
      if (next_)
	    return true;
      if (nexus_->first_nlink() != this)
	    return true;

      return false;
}

bool Link::is_linked(const Link&that) const
{
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

const NetObj*Link::get_obj() const
{
      return node_;
}

NetObj*Link::get_obj()
{
      return node_;
}

unsigned Link::get_pin() const
{
      return pin_;
}

void Link::set_name(perm_string n, unsigned i)
{
      name_ = n;
      inst_ = i;
}

perm_string Link::get_name() const
{
      return name_;
}

unsigned Link::get_inst() const
{
      return inst_;
}

Nexus::Nexus()
{
      name_ = 0;
      list_ = 0;
      list_len_ = 0;
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

int Nexus::is_driven() const
{
      int count = 0;

      for (Link*cur = list_ ; cur ;  cur = cur->next_) {
	    if (cur->get_dir() == Link::OUTPUT)
		  count += 1;
      }

      return count;
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
      assert(that->nexus_ == this);
      if (list_ == that) {
	    assert(that->prev_ == 0);
	      // Move the second item the front of the list.
	    list_ = that->next_;
	    if (list_)
		  list_->prev_ = 0;
	    list_len_ -= 1;
	      // Clear the pointers for the link.
	    that->next_ = 0;
	    that->prev_ = 0;
	    that->nexus_ = 0;
	    return;
      }


      Link*cur = that->prev_;
      assert(cur->nexus_ == this);

      cur->next_ = that->next_;
      if (cur->next_)
	    cur->next_->prev_ = cur;
      that->nexus_ = 0;
      that->next_ = 0;
      that->prev_ = 0;
      list_len_ -= 1;
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
      that->prev_ = 0;
      if (that->next_)
	    that->next_->prev_ = that;

      that->nexus_ = this;
      list_ = that;
      list_len_ += 1;
}

Link* Nexus::first_nlink()
{
      return list_;
}

const Link* Nexus::first_nlink() const
{
      return list_;
}

void* Nexus::t_cookie() const
{
      return t_cookie_;
}

void* Nexus::t_cookie(void*val)const
{
      void*tmp = t_cookie_;
      t_cookie_ = val;
      return tmp;
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
	    const NetObj*obj = lnk->get_obj();
	    pin = lnk->get_pin();
	    cerr << "internal error: No signal for nexus of " <<
		  obj->name() << " pin " << pin << "(" <<
		  lnk->get_name() << "<" << lnk->get_inst() << ">)"
		  " type=" << typeid(*obj).name() << "?" << endl;
	    return 0;
      }

      assert(sig);
      ostringstream tmp;
      tmp << sig->name();
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
      index_ = 0;
      nitems_ = 0;
}

NexusSet::~NexusSet()
{
      if (nitems_ > 0) {
	    assert(items_ != 0);
	    delete[] items_;
	    assert(index_ != 0);
	    delete[] index_;
      } else {
	    assert(items_ == 0);
	    assert(index_ == 0);
      }
}

unsigned NexusSet::count() const
{
      return nitems_;
}

/*
 * Add the Nexus to the nexus set at the *end* of the array. This
 * preserves order, which is used in a few cases by the
 * synthesizer. But for efficiency, also create a sorted index.
 */
void NexusSet::add(Nexus*that)
{
	/* Handle the special case that the set is empty. */
      if (nitems_ == 0) {
	    assert(items_ == 0);
	    assert(index_ == 0);
	    items_ = (Nexus**)malloc(sizeof(Nexus*));
	    items_[0] = that;
	    index_ = (unsigned*)malloc(sizeof(unsigned));
	    index_[0] = 0;
	    nitems_ = 1;
	    return;
      }

      unsigned ptr = bsearch_(that);
      if (ptr < nitems_ && items_[index_[ptr]] == that) {
	    assert(items_[index_[ptr]] == that);
	    return;
      }

      items_ = (Nexus**)  realloc(items_, (nitems_+1) * sizeof(Nexus*));
      index_ = (unsigned*)realloc(index_, (nitems_+1) * sizeof(unsigned));

      items_[nitems_] = that;

      unsigned dest = nitems_;
      for (unsigned idx = ptr ;  idx < nitems_ ;  idx += 1) {
	    unsigned tmp = index_[idx];
	    index_[idx] = dest;
	    dest = tmp;
      }
      index_[nitems_] = dest;

      nitems_ += 1;
}

void NexusSet::add(const NexusSet&that)
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1)
	    add(that.items_[idx]);
}

void NexusSet::rem(Nexus*that)
{
      if (nitems_ == 0)
	    return;

      unsigned ptr = bsearch_(that);
      if (ptr >= nitems_ || items_[index_[ptr]] != that)
	    return;

      if (nitems_ == 1) {
	    free(items_);
	    free(index_);
	    items_ = 0;
	    index_ = 0;
	    nitems_ = 0;
	    return;
      }

      unsigned index_ptr = index_[ptr];
      for (unsigned idx = index_ptr ;  idx < (nitems_-1) ;  idx += 1)
	    items_[idx] = items_[idx+1];

      for (unsigned idx = ptr ;  idx < (nitems_-1) ;  idx += 1)
	    index_[idx] = index_[idx+1];

      for (unsigned idx = 0 ;  idx < (nitems_-1) ;  idx += 1)
	    if (index_[idx] > index_ptr)
		  index_[idx] -= 1;

      items_ = (Nexus**)  realloc(items_, (nitems_-1) * sizeof(Nexus*));
      index_ = (unsigned*)realloc(index_, (nitems_-1) * sizeof(unsigned));
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

/*
 * This method uses binary search to locate the item in the list of
 * nexus pointers. If the item is in the set, then this method returns
 * the index where it exists in the *index* array. If the item is not
 * in the set, the index points to where in the array the item should go.
 */
unsigned NexusSet::bsearch_(Nexus*that) const
{

      unsigned low = 0, hig = nitems_;

      while (low < hig) {
	    unsigned mid = (low + hig) / 2;
	    if (mid == hig) mid -= 1;
	    assert(mid >= low);
	    assert(mid < hig);

	    if (items_[index_[mid]] == that) {
		  return mid;

	    } else if (items_[index_[mid]] > that) {
		  hig = mid;

	    } else {
		  low = mid+1;
	    }
      }

      assert(low == hig);
      assert(low == nitems_ || items_[index_[low]] >= that);
      return low;
}

bool NexusSet::contains(const NexusSet&that) const
{
      for (unsigned idx = 0 ;  idx < that.nitems_ ;  idx += 1) {
	    unsigned where = bsearch_(that[idx]);
	    if (where == nitems_)
		  return false;
	    if (items_[index_[where]] != that[idx])
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
	    if (items_[index_[where]] == that[idx])
		  return true;
      }

      return false;
}
