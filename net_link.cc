/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: net_link.cc,v 1.2 2000/07/14 06:12:57 steve Exp $"
#endif

# include  "netlist.h"
# include  <strstream>
# include  <string>
# include  <typeinfo>

void connect(Link&l, Link&r)
{
      assert(&l != &r);
      assert(l.nexus_);
      assert(r.nexus_);

	/* If the links are already connected, then we're done. */
      if (l.nexus_ == r.nexus_)
	    return;

      Nexus*tmp = r.nexus_;
      while (Link*cur = tmp->first_nlink()) {
	    tmp->unlink(cur);
	    l.nexus_->relink(cur);
      }

      assert(tmp->list_ == 0);
      assert(l.is_linked(r));
      delete tmp;
}

Link::Link()
: dir_(PASSIVE), drive0_(STRONG), drive1_(STRONG), init_(verinum::Vx),
  inst_(0), next_(0), nexus_(0)
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

void Link::set_name(const string&n, unsigned i)
{
      name_ = n;
      inst_ = i;
}

const string& Link::get_name() const
{
      return name_;
}

unsigned Link::get_inst() const
{
      return inst_;
}

Nexus::Nexus()
{
      list_ = 0;
}

Nexus::~Nexus()
{
      assert(list_ == 0);
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

void Nexus::unlink(Link*that)
{
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

string Nexus::name() const
{
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

      }
      assert(sig);
      ostrstream tmp;
      tmp << sig->name();
      if (sig->pin_count() > 1)
	    tmp << "<" << pin << ">";
      tmp << ends;

      return tmp.str();
}

/*
 * $Log: net_link.cc,v $
 * Revision 1.2  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.1  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 */

