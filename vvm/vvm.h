#ifndef __vvm_H
#define __vvm_H
/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vvm.h,v 1.7 1999/05/03 01:51:29 steve Exp $"
#endif

# include  <vector>
# include  <string>
# include  <cassert>

/*
 * The Verilog Virtual Machine are definitions for the virtual machine
 * that executes models that the simulation generator makes.
 */

typedef unsigned vvm_u32;

class vvm_event;
class vvm_simulation;
class vvm_simulation_cycle;
class vvm_thread;

/* The vvm_bit_t is the basic unit of value for a scalar signal in
   Verilog. It represents all the possible values. The vvm_bitstring_t
   is a vector of vvm_bit_t and is used when variable-length bit
   arrays are needed. */
enum vvm_bit_t { V0 = 0, V1, Vx, Vz };


inline vvm_bit_t operator & (vvm_bit_t l, vvm_bit_t r)
{
      if (l == V0) return V0;
      if (r == V0) return V0;
      if ((l == V1) && (r == V1)) return V1;
      return Vx;
}

inline vvm_bit_t operator | (vvm_bit_t l, vvm_bit_t r)
{
      if (l == V1) return V1;
      if (r == V1) return V1;
      if ((l == V0) && (r == V0)) return V0;
      return Vx;
}

inline vvm_bit_t operator ^ (vvm_bit_t l, vvm_bit_t r)
{
      if (l == Vx) return Vx;
      if (l == Vz) return Vx;
      if (r == Vx) return Vx;
      if (r == Vz) return Vx;
      if (l == V0) return r;
      return (r == V0)? V1 : V0;
}

extern vvm_bit_t add_with_carry(vvm_bit_t l, vvm_bit_t r, vvm_bit_t&carry);

inline vvm_bit_t not(vvm_bit_t l)
{
      switch (l) {
	  case V0:
	    return V1;
	  case V1:
	    return V0;
	  default:
	    return Vx;
      }
}

class vvm_bits_t {
    public:
      virtual ~vvm_bits_t() =0;
      virtual unsigned get_width() const =0;
      virtual vvm_bit_t get_bit(unsigned idx) const =0;
};

extern ostream& operator << (ostream&os, vvm_bit_t);
extern ostream& operator << (ostream&os, const vvm_bits_t&str);

/*
 * The vvm_bitset_t is a fixed width array-like set of vvm_bit_t
 * items. A number is often times made up of bit sets instead of
 * single bits. The fixed array is used when possible because of the
 * more thorough type checking and (hopefully) better optimization.
 */
template <unsigned WIDTH> class vvm_bitset_t  : public vvm_bits_t {

    public:
      vvm_bitset_t()
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  bits_[idx] = Vz;
	    }

      vvm_bit_t operator[] (unsigned idx) const { return bits_[idx]; }
      vvm_bit_t&operator[] (unsigned idx) { return bits_[idx]; }

      unsigned get_width() const { return WIDTH; }
      vvm_bit_t get_bit(unsigned idx) const { return bits_[idx]; }

      bool eequal(const vvm_bitset_t<WIDTH>&that) const
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  if (bits_[idx] != that.bits_[idx])
			return false;
	      return true;
	    }

      unsigned as_unsigned() const
	    { unsigned result = 0;
	      for (unsigned idx = WIDTH ;  idx > 0 ;  idx -= 1) {
		    result <<= 1;
		    if (bits_[idx-1]) result |= 1;
	      }
	      return result;
	    }

    private:
      vvm_bit_t bits_[WIDTH];
};

/*
 * Verilog events (update events and nonblocking assign) are derived
 * from this abstract class so that the simulation engine can treat
 * all of them identically.
 */
class vvm_event {

      friend class vvm_simulation;

    public:
      vvm_event() { }
      virtual ~vvm_event() =0;
      virtual void event_function() =0;

    private:
      vvm_event*next_;

    private: // not implemented
      vvm_event(const vvm_event&);
      vvm_event& operator= (const vvm_event&);
};


/*
 * This class is the main simulation engine. Object of this type are
 * self-contained simulations. Generally, only one is needed.
 */
class vvm_simulation {

    public:
      vvm_simulation();
      ~vvm_simulation();

	// Take a simulation that has been primed with some initial
	// events, and run it. Continue running it until the
	// simulation stops. The sim parameter becomes the new list,
	// or 0 if the events run out. The simulation clock is
	// advanced for the first cycle in sim.
      void run();

	// Add an event to an existing simulation cycle list. If there
	// is not a cycle for the exact delay of the event, create one
	// and insert it into the cycle list. Add the event to the
	// list of events for the cycle time.
      void insert_event(unsigned long delay, vvm_event*event);

	// This puts the event in the current active list. No delay.
      void active_event(vvm_event*event);

	// These are versions of the *_event methods that take
	// vvm_thread objects instead.
      void thread_delay(unsigned long delay, vvm_thread*);
      void thread_active(vvm_thread*);

	// Trigger an event as a monitor event causes it to be
	// scheduled and executed when the time cycle is
	// complete. Unlike other events, the execution of a event so
	// scheduled will not cause the event to be deleted. Also,
	// only one event can be a monitor.
      void monitor_event(vvm_event*);

      unsigned long get_sim_time() const { return time_; }

      void s_finish();

    private:
      bool going_;
      vvm_simulation_cycle*sim_;

	// Triggered monitor event.
      vvm_event*mon_;

      unsigned long time_;

    private: // not implemented
      vvm_simulation(const vvm_simulation&);
      vvm_simulation& operator= (const vvm_simulation&);
};

/*
 * The vvm_monitor_t is usually associated with a vvm_bitset_t that
 * represents a signal. The VVM code generator generates calls to the
 * trigger method whenever an assignment or output value is set on
 * the associated signal. The trigger, if enabled, then causes the
 * monitor event to be scheduled in the simulation.
 *
 * This object also carries the canonical signal name, for the use of
 * %m display patterns.
 */
class vvm_monitor_t {
    public:
      vvm_monitor_t(const string&);

      void trigger(vvm_simulation*sim)
	    { if (event_) sim->monitor_event(event_); }

      const string& name() const { return name_; }

      void enable(vvm_event*e) { event_ = e; }

    private:
      string name_;
      vvm_event*event_;

    private: // not implemented
      vvm_monitor_t(const vvm_monitor_t&);
      vvm_monitor_t& operator= (const vvm_monitor_t&);
};


template <unsigned WIDTH> class vvm_signal_t  : public vvm_monitor_t {

    public:
      vvm_signal_t(const string&n, vvm_bitset_t<WIDTH>*b)
      : vvm_monitor_t(n), bits_(b)
	    { }

      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { (*bits_)[idx] = val;
	      trigger(sim);
	    }

      void set(vvm_simulation*sim, const vvm_bitset_t<WIDTH>&val)
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  set(sim, idx, val[idx]);
	    }

    private:
      vvm_bitset_t<WIDTH>*bits_;
};

/*
 * $Log: vvm.h,v $
 * Revision 1.7  1999/05/03 01:51:29  steve
 *  Restore support for wait event control.
 *
 * Revision 1.6  1999/04/22 04:56:58  steve
 *  Add to vvm proceedural memory references.
 *
 * Revision 1.5  1999/03/16 04:43:46  steve
 *  Add some logical operators.
 *
 * Revision 1.4  1999/02/08 03:55:55  steve
 *  Do not generate code for signals,
 *  instead use the NetESignal node to
 *  generate gate-like signal devices.
 *
 * Revision 1.3  1998/12/17 23:54:58  steve
 *  VVM support for small sequential UDP objects.
 *
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:10  steve
 *  Add vvm library.
 *
 */
#endif
