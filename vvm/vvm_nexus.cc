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
#ident "$Id: vvm_nexus.cc,v 1.1 2000/03/16 19:03:04 steve Exp $"
#endif

# include  "vvm_nexus.h"
# include  <assert.h>

vvm_nexus::vvm_nexus()
{
      drivers_ = 0;
      recvrs_ = 0;
      ival_ = 0;
      nival_ = 0;
}

vvm_nexus::~vvm_nexus()
{
      assert(drivers_ == 0);

	/* assert(recvrs_ == 0); XXXX I really should make a point to
	   guarantee that all the receivers that I refer to are gone,
	   but since I know for now that this destructor is only
	   called when the simulation exist, nevermind. */
}

/*
 * Connect a driver to this nexus. The driver needs to know who I am
 * (and can only be connected to one nexus) and also I need to keep a
 * list of all the drivers connected to me. Arranging the list
 * pointers accordingly. Also, keep track of the number of drivers I
 * have connected to me, and maintain the ival_ array so that I do not
 * need to manage the array at run time.
 */
void vvm_nexus::connect(vvm_nexus::drive_t*drv)
{
	// Link the driver into my list of drivers.
      assert(drv->nexus_ == 0);
      drv->nexus_ = this;
      drv->next_ = drivers_;
      drivers_ = drv;

      nival_ += 1;
      if (ival_) delete[]ival_;
      ival_ = new vpip_bit_t[nival_];
}

/*
 * Connect this receiver to me. Receivers are handled a little
 * differently from drivers, a receiver is fully specified by the
 * pointer to the receiver AS WELL AS a magic key value. This allows a
 * gate to me a receiver with many different input pins.
 */
void vvm_nexus::connect(vvm_nexus::recvr_t*rcv, unsigned key)
{
      recvr_cell*cur = new recvr_cell;
      cur->dev = rcv;
      cur->key = key;
      cur->next = recvrs_;
      recvrs_ = cur;
}

void vvm_nexus::disconnect(vvm_nexus::drive_t*drv)
{
      drive_t*cur = drivers_;
      drivers_ = 0;
      while (cur) {
	    drive_t*tmp = cur;
	    cur = cur->next_;
	    if (tmp != drv) {
		  tmp->next_ = drivers_;
		  drivers_ = tmp;
	    }
      }
}

/*
 * This method disconnects a receiver completely by removing every
 * access to it. (Remember that a recvr_t object may be connected many
 * times with different keys.)
 */
void vvm_nexus::disconnect(vvm_nexus::recvr_t*rcv)
{
      recvr_cell*cur = recvrs_;
      recvrs_ = 0;
      while (cur) {
	    recvr_cell*tmp = cur;
	    cur = tmp->next;
	    if (tmp->dev == rcv) {
		  delete tmp;
	    } else {
		  tmp->next = recvrs_;
		  recvrs_ = tmp;
	    }
      }
}

/*
 * Reg assign (or procedural assignment) does not use the concept of
 * drivers. Instead, I skip the resolution function, save the value
 * for myself and pass the value to the receivers.
 */
void vvm_nexus::reg_assign(vpip_bit_t val)
{
      assert(drivers_ == 0);
      value_ = val;
      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

/*
 * This method is invoked when something interesting happens at one of
 * the drivers. It collects all the driver values, resolves them into
 * a signal value, and passed them to all the connected
 * receivers. This is the workhorse of the nexus object.
 */
void vvm_nexus::run_values()
{
	/* Collect the values from all the signal drivers. We should
	   have exactly the right size ival_ array for the number of
	   drives. */

      unsigned idx = 0;
      for (drive_t*cur = drivers_; cur ;  cur = cur->next_) {
	    assert(idx < nival_);
	    ival_[idx++] = cur->peek_value();
      }
      assert(idx == nival_);


	/* Use the resolution function to generate the settled value
	   for this nexus. */

      vpip_bit_t val = resolution_function(ival_, nival_);
      if (value_ == val) return;
      value_ = val;

	/* Now deliver that output value to all the receivers
	   connected to this nexus. */
      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

vvm_nexus::drive_t::drive_t()
{
      nexus_ = 0;
}

vvm_nexus::drive_t::~drive_t()
{
      if (nexus_) nexus_->disconnect(this);
}

vpip_bit_t vvm_nexus::drive_t::peek_value() const
{
      return value_;
}

void vvm_nexus::drive_t::set_value(vpip_bit_t val)
{
      if (val == value_) return;
      value_ = val;
      if (nexus_) nexus_->run_values();
}


vvm_nexus::recvr_t::recvr_t()
{
}

vvm_nexus::recvr_t::~recvr_t()
{
}

vvm_nexus_wire::vvm_nexus_wire()
{
}

vvm_nexus_wire::~vvm_nexus_wire()
{
}

vpip_bit_t vvm_nexus_wire::resolution_function(const vpip_bit_t*bits,
					       unsigned nbits) const
{
      if (nbits == 0) return Vz;
      return bits[0];
}

class delayed_assign_event  : public vvm_event {
    public:
      delayed_assign_event(vvm_nexus&l, vpip_bit_t r)
      : l_val_(l), r_val_(r) { }

      void event_function() { l_val_.reg_assign(r_val_); }

    private:
      vvm_nexus& l_val_;
      vpip_bit_t r_val_;
};

void vvm_delayed_assign(vvm_nexus&l_val, vpip_bit_t r_val,
			unsigned long delay)
{
      delayed_assign_event*ev = new delayed_assign_event(l_val, r_val);
      ev->schedule(delay);
}

/*
 * $Log: vvm_nexus.cc,v $
 * Revision 1.1  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 */

