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
#ident "$Id: vvm_nexus.cc,v 1.11 2000/11/20 00:58:40 steve Exp $"
#endif

# include  "vvm_nexus.h"
# include  "vvm_gates.h"
# include  <assert.h>

vvm_nexus::vvm_nexus()
{
      drivers_ = 0;
      recvrs_ = 0;
      ival_ = 0;
      nival_ = 0;
      value_ = 0;
      force_ = 0;
      forcer_ = 0;
      forcer_key_ = 0;
      assigner_ = 0;
      assigner_key_ = 0;
      resolution_function = &vvm_resolution_wire;
}

vvm_nexus::~vvm_nexus()
{
      while (drivers_) {
	    drive_t*cur = drivers_;
	    drivers_ = cur->next_;
	    assert(cur->nexus_ == this);
	    cur->nexus_ = 0;
	    cur->next_ = 0;
      }

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
      if (forcer_)
	    return;

      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

void vvm_nexus::force_set(vvm_force*f, unsigned k)
{
      if (forcer_)
	    forcer_->release(forcer_key_);

      forcer_ = f;
      forcer_key_ = k;
}

void vvm_nexus::cassign_set(vvm_force*f, unsigned k)
{
      assert(drivers_ == 0);
      if (assigner_)
	    assigner_->release(assigner_key_);

      assigner_ = f;
      assigner_key_ = k;
}

void vvm_nexus::force_assign(vpip_bit_t val)
{
      assert(forcer_);
      force_ = val;
      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, force_);
}

void vvm_nexus::cassign(vpip_bit_t val)
{
      assert(assigner_);
      value_ = val;

      if (forcer_) return;

      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

void vvm_nexus::release()
{
      if (forcer_) {
	    forcer_->release(forcer_key_);
	    forcer_ = 0;
      }

	/* Now deliver that output value to all the receivers
	   connected to this nexus. */
      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

void vvm_nexus::deassign()
{
      assert(drivers_ == 0);
      if (assigner_) {
	    assigner_->release(assigner_key_);
	    assigner_ = 0;
      }
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

      if (forcer_)
	    return;

	/* Now deliver that output value to all the receivers
	   connected to this nexus. */
      for (recvr_cell*cur = recvrs_;  cur ;  cur = cur->next)
	    cur->dev->take_value(cur->key, value_);
}

vvm_nexus::drive_t::drive_t()
{
      value_ = StX;
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

vpip_bit_t vvm_resolution_wire(const vpip_bit_t*bits, unsigned nbits)
{
      if (nbits == 0) return HiZ;
      return vpip_bits_resolve(bits, nbits);
}

vpip_bit_t vvm_resolution_sup0(const vpip_bit_t*, unsigned)
{
      return Su0;
}

vpip_bit_t vvm_resolution_sup1(const vpip_bit_t*, unsigned)
{
      return Su1;
}

vpip_bit_t vvm_resolution_tri0(const vpip_bit_t*bits, unsigned nbits)
{
      if (nbits == 0) return Pu0;

      vpip_bit_t res = vpip_bits_resolve(bits, nbits);
      if (B_ISZ(res)) return Pu0;

      return res;
}

vpip_bit_t vvm_resolution_tri1(const vpip_bit_t*bits, unsigned nbits)
{
      if (nbits == 0) return Pu1;

      vpip_bit_t res = vpip_bits_resolve(bits, nbits);
      if (B_ISZ(res)) return Pu1;

      return res;
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
 * Revision 1.11  2000/11/20 00:58:40  steve
 *  Add support for supply nets (PR#17)
 *
 * Revision 1.10  2000/10/23 00:32:48  steve
 *  Nexus value is initially unknown so that it propogates for sure.
 *
 * Revision 1.9  2000/08/02 00:57:03  steve
 *  tri01 support in vvm.
 *
 * Revision 1.8  2000/05/11 23:37:28  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.7  2000/04/23 03:45:25  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.6  2000/04/22 04:20:20  steve
 *  Add support for force assignment.
 *
 * Revision 1.5  2000/03/22 05:16:38  steve
 *  Integrate drive resolution function.
 *
 * Revision 1.4  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.3  2000/03/18 01:27:00  steve
 *  Generate references into a table of nexus objects instead of
 *  generating lots of isolated nexus objects. Easier on linkers
 *  and compilers,
 *
 *  Add missing nexus support for l-value bit selects,
 *
 *  Detemplatize the vvm_mux type.
 *
 *  Fix up the vvm_nexus destructor to disconnect from drivers.
 *
 * Revision 1.2  2000/03/16 21:45:07  steve
 *  Properly initialize driver and nexus values.
 *
 * Revision 1.1  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 */

