#ifndef __vvm_vvm_nexus_H
#define __vvm_vvm_nexus_H
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
#ident "$Id: vvm_nexus.h,v 1.6 2000/11/20 00:58:41 steve Exp $"
#endif

# include  "vvm.h"


class vvm_force;

/*
 * The nexus class represents a connection point for drivers and
 * receivers of signals. The signal carries a single bit, has drivers,
 * has fan_out and has a current value. Classes derive from the nexus
 * in order to provide specific kinds of N-driver resolution.
 *
 * The driver_t and recvr_t classes are the means to connect to a
 * nexus. In general, other then to connect the drivers and receivers,
 * there should be no cause to manipulate the nexus object directly.
 *
 * The driver_t class represents a driver on the nexus. There can be
 * any number of drivers on a nexus, so long as the
 * recolution_function can handle the situation. The drivers are
 * normally members of some gate object somewhere. When the driver
 * receives a value, it saves the value in itself and tells the
 * connected nexus that something has changed.
 *
 * The nexus object, when it notices that one of its drivers changed,
 * collects the values of the drivers and passes the set to the
 * resolution function. The resolution function calculates what the
 * value of the nexus should be given the values driving it.
 *
 * When the value of the nexus is ready, it scans the list of
 * connected receivers and passes the new value out. The receiver is
 * actually a pointer to the recvr_t object and a key. This is so the
 * receiver object can receive many different pins worth of data. The
 * idea is that a gate can be a single recvr_t object, and the key can
 * be used and the number of the affected pin.
 */
class vvm_nexus {

    public:
      class drive_t {
	    friend class ::vvm_nexus;
	  public:
	    drive_t();
	    ~drive_t();
	    vpip_bit_t peek_value() const;
	    void set_value(vpip_bit_t);
	  private:
	    vpip_bit_t value_;
	    vvm_nexus*nexus_;
	    drive_t*next_;
	  private: // not implemented
	    drive_t(const drive_t&);
	    drive_t& operator= (const drive_t&);
      };

      class recvr_t {
	    friend class ::vvm_nexus;
	  public:
	    recvr_t();
	    virtual ~recvr_t() =0;
	    virtual void take_value(unsigned key, vpip_bit_t val) =0;
	  private: // not implemented
	    recvr_t(const recvr_t&);
	    recvr_t& operator= (const recvr_t&);
      };

    public:
      vvm_nexus();
      ~vvm_nexus();

	// These methods support connecting the receiver and driver to
	// the nexus.
      void connect(drive_t*drv);
      void connect(recvr_t*rcv, unsigned key);

      void disconnect(drive_t*drv);
      void disconnect(recvr_t*rcv);


	// If there are no drivers to this nexus, this can be used to
	// to procedural assignments to the node, as if it where a reg.
      void reg_assign(vpip_bit_t val);

	// These methods support the procedural continuous assign. The
	// vvm_force oject will set itself as an assigner, then will
	// periodically call the cassign method to do the assign. The
	// procedural deassign will call the deassign method to detach
	// the vvm_force object.
      void cassign_set(class vvm_force*frc, unsigned key);
      void cassign(vpip_bit_t val);
      void deassign();

	// This method causes the specified value to be forced onto
	// the nexus. This overrides all drivers that are attached.
      void force_set(class vvm_force*frc, unsigned key);
      void force_assign(vpip_bit_t val);
      void release();

	// The run_values() method collects all the current driver
	// values and, with the aid of the resolution_function,
	// generates the current value for the nexus. It also passes
	// that value on to the receuvers.
      void run_values();
      vpip_bit_t (*resolution_function)(const vpip_bit_t*, unsigned);

    private:
      vpip_bit_t value_;
      drive_t*drivers_;
      struct recvr_cell {
	    recvr_t *dev;
	    unsigned key;
	    recvr_cell*next;
      } *recvrs_;

      vpip_bit_t*ival_;
      unsigned  nival_;

      vvm_force *assigner_;
      unsigned   assigner_key_;

      vpip_bit_t force_;
      vvm_force *forcer_;
      unsigned   forcer_key_;

    private: // not implemented
      vvm_nexus(const vvm_nexus&);
      vvm_nexus& operator= (const vvm_nexus&);
};


extern vpip_bit_t vvm_resolution_wire(const vpip_bit_t*bits, unsigned nbits);
extern vpip_bit_t vvm_resolution_sup0(const vpip_bit_t*bits, unsigned nbits);
extern vpip_bit_t vvm_resolution_sup1(const vpip_bit_t*bits, unsigned nbits);
extern vpip_bit_t vvm_resolution_tri0(const vpip_bit_t*bits, unsigned nbits);
extern vpip_bit_t vvm_resolution_tri1(const vpip_bit_t*bits, unsigned nbits);

/*
 * This function arranges for a non-blocking reg_assign to a nexus. It
 * creates all the events needed to make it happen after the specified
 * delay.
 */
extern void vvm_delayed_assign(vvm_nexus&l_val, vpip_bit_t r_val,
			       unsigned long delay);

/*
 * $Log: vvm_nexus.h,v $
 * Revision 1.6  2000/11/20 00:58:41  steve
 *  Add support for supply nets (PR#17)
 *
 * Revision 1.5  2000/08/02 00:57:03  steve
 *  tri01 support in vvm.
 *
 * Revision 1.4  2000/05/11 23:37:28  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.3  2000/04/23 03:45:25  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.2  2000/04/22 04:20:20  steve
 *  Add support for force assignment.
 *
 * Revision 1.1  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 */
#endif
