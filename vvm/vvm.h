#ifndef __vvm_H
#define __vvm_H
/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm.h,v 1.26 1999/12/09 06:00:19 steve Exp $"
#endif

# include  <cassert>
# include  "vpi_priv.h"

/*
 * The Verilog Virtual Machine are definitions for the virtual machine
 * that executes models that the simulation generator makes.
 */

typedef unsigned vvm_u32;

class vvm_event;
class vvm_simulation;
class vvm_simulation_cycle;
class vvm_thread;
class ostream;


inline vpip_bit_t operator & (vpip_bit_t l, vpip_bit_t r)
{
      if (l == V0) return V0;
      if (r == V0) return V0;
      if ((l == V1) && (r == V1)) return V1;
      return Vx;
}

inline vpip_bit_t operator | (vpip_bit_t l, vpip_bit_t r)
{
      if (l == V1) return V1;
      if (r == V1) return V1;
      if ((l == V0) && (r == V0)) return V0;
      return Vx;
}

inline vpip_bit_t operator ^ (vpip_bit_t l, vpip_bit_t r)
{
      if (l == Vx) return Vx;
      if (l == Vz) return Vx;
      if (r == Vx) return Vx;
      if (r == Vz) return Vx;
      if (l == V0) return r;
      return (r == V0)? V1 : V0;
}

inline vpip_bit_t less_with_cascade(vpip_bit_t l, vpip_bit_t r, vpip_bit_t c)
{
      if (l == Vx) return Vx;
      if (r == Vx) return Vx;
      if (l > r) return V0;
      if (l < r) return V1;
      return c;
}

inline vpip_bit_t greater_with_cascade(vpip_bit_t l, vpip_bit_t r, vpip_bit_t c)
{
      if (l == Vx) return Vx;
      if (r == Vx) return Vx;
      if (l > r) return V1;
      if (l < r) return V0;
      return c;
}

extern vpip_bit_t add_with_carry(vpip_bit_t l, vpip_bit_t r, vpip_bit_t&carry);

inline vpip_bit_t not(vpip_bit_t l)
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

extern bool posedge(vpip_bit_t from, vpip_bit_t to);


class vvm_bits_t {
    public:
      virtual ~vvm_bits_t() =0;
      virtual unsigned get_width() const =0;
      virtual vpip_bit_t get_bit(unsigned idx) const =0;

      unsigned as_unsigned() const;
};

extern ostream& operator << (ostream&os, vpip_bit_t);
extern ostream& operator << (ostream&os, const vvm_bits_t&str);

/*
 * The vvm_bitset_t is a fixed width array-like set of vpip_bit_t
 * items. A number is often times made up of bit sets instead of
 * single bits. The fixed array is used when possible because of the
 * more thorough type checking and (hopefully) better optimization.
 */
template <unsigned WIDTH> class vvm_bitset_t  : public vvm_bits_t {

    public:
      vvm_bitset_t()
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  bits[idx] = Vz;
	    }

      vvm_bitset_t(const vvm_bits_t&that)
	    { unsigned wid = WIDTH;
	      if (that.get_width() < WIDTH)
		    wid = that.get_width();
	      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		    bits[idx] = that.get_bit(idx);
	      for (unsigned idx = wid ;  idx < WIDTH ;  idx += 1)
		    bits[idx] = V0;
	    }
	    

      vvm_bitset_t(const vvm_bitset_t<WIDTH>&that)
	    { for (unsigned idx = 0; idx < WIDTH; idx += 1)
		    bits[idx] = that.bits[idx];
	    }

      vpip_bit_t operator[] (unsigned idx) const { return bits[idx]; }
      vpip_bit_t&operator[] (unsigned idx) { return bits[idx]; }

      unsigned get_width() const { return WIDTH; }
      vpip_bit_t get_bit(unsigned idx) const { return bits[idx]; }

    public:
      vpip_bit_t bits[WIDTH];
};

/*
 * Verilog events (update events and nonblocking assign) are derived
 * from this abstract class so that the simulation engine can treat
 * all of them identically.
 */
class vvm_event {

      friend class vvm_simulation;

    public:
      vvm_event();
      virtual ~vvm_event() =0;
      virtual void event_function() =0;

      static void callback_(void*);

    private:
      struct vpip_event*event_;

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

      bool finished() const;

    private: // not implemented
      vvm_simulation(const vvm_simulation&);
      vvm_simulation& operator= (const vvm_simulation&);
};

/*
 * The vvm_signal_t template is the real object that handles the
 * receiving of assignments and doing whatever is done. It also
 * connects VPI to the C++/vvm design.
 */
template <unsigned WIDTH> class vvm_signal_t  : public __vpiSignal  {

    public:
      vvm_signal_t(vvm_bitset_t<WIDTH>*b)
	    { bits = b->bits;
	      nbits = WIDTH;
	    }
      ~vvm_signal_t() { }

      void init_P(unsigned idx, vpip_bit_t val)
	    { bits[idx] = val; }

      void set_P(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { bits[idx] = val;
	      vpip_run_value_changes(this);
	    }

      void set_P(vvm_simulation*sim, const vvm_bitset_t<WIDTH>&val)
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  set(sim, idx, val[idx]);
	    }
};

struct vvm_ram_callback {
      vvm_ram_callback();
      virtual ~vvm_ram_callback();
      virtual void handle_write(vvm_simulation*sim, unsigned idx) =0;
      vvm_ram_callback*next_;
};

template <unsigned WIDTH, unsigned SIZE>
class vvm_memory_t : public __vpiMemory {

    public:
      vvm_memory_t()
	    { cb_list_ = 0;
	    }

      void set_word(vvm_simulation*sim, unsigned addr,
		    const vvm_bitset_t<WIDTH>&val)
	    { unsigned base = WIDTH * addr;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    bits[base+idx] = val[idx];
	      call_list_(sim, addr);
	    }

      void set_word(vvm_simulation*sim, unsigned addr,
		    const vpip_bit_t val[WIDTH])
	    { unsigned base = WIDTH * addr;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    bits[base+idx] = val[idx];
	      call_list_(sim, addr);
	    }

      vvm_bitset_t<WIDTH> get_word(unsigned addr) const
	    { vvm_bitset_t<WIDTH> val;
	      unsigned base = WIDTH * addr;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    val[idx] = bits[base+idx];
	      return val;
	    }

      void set_callback(vvm_ram_callback*ram)
	    { ram->next_ = cb_list_;
	      cb_list_ = ram;
	    }

    private:
      vvm_ram_callback*cb_list_;
      void call_list_(vvm_simulation*sim, unsigned idx)
	    { for (vvm_ram_callback*cur = cb_list_; cur; cur = cur->next_)
		    cur->handle_write(sim, idx);
	    }
};

/*
 * $Log: vvm.h,v $
 * Revision 1.26  1999/12/09 06:00:19  steve
 *  Fix const/non-const errors.
 *
 * Revision 1.25  1999/12/05 02:24:09  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 *
 * Revision 1.24  1999/12/02 03:36:01  steve
 *  shiftl and shiftr take unsized second parameter.
 *
 * Revision 1.23  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 * Revision 1.22  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.21  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 * Revision 1.20  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.19  1999/10/31 04:11:28  steve
 *  Add to netlist links pin name and instance number,
 *  and arrange in vvm for pin connections by name
 *  and instance number.
 *
 * Revision 1.18  1999/10/29 03:37:22  steve
 *  Support vpiValueChance callbacks.
 *
 * Revision 1.17  1999/10/28 21:36:00  steve
 *  Get rid of monitor_t and fold __vpiSignal into signal.
 *
 * Revision 1.16  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.15  1999/10/13 03:15:51  steve
 *  Remove useless operator=.
 *
 * Revision 1.14  1999/10/06 01:28:18  steve
 *  The $finish task should work immediately.
 *
 * Revision 1.13  1999/10/05 04:02:10  steve
 *  Relaxed width handling for <= assignment.
 *
 * Revision 1.12  1999/09/29 18:36:04  steve
 *  Full case support
 *
 * Revision 1.11  1999/09/28 01:13:15  steve
 *  Support in vvm > and >= behavioral operators.
 *
 * Revision 1.10  1999/08/15 01:23:56  steve
 *  Convert vvm to implement system tasks with vpi.
 *
 * Revision 1.9  1999/06/21 01:02:34  steve
 *  Add init to vvm_signal_t.
 *
 * Revision 1.8  1999/06/07 03:40:22  steve
 *  Implement the < binary operator.
 *
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
