#ifndef __vvm_gates_H
#define __vvm_gates_H
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
#ident "$Id: vvm_gates.h,v 1.4 1998/12/20 02:05:41 steve Exp $"
#endif

# include  "vvm.h"
# include  <assert.h>

/*
 * A vvm gate is constructed with an input width and an output
 * function. The input width represents all the input signals that go
 * into generating a single output value. The output value is passed
 * to the output function, which may fan the result however makes
 * sense. The output is scheduled as an event.
 */

class vvm_out_event  : public vvm_event {

    public:
      typedef void (*action_t)(vvm_simulation*, vvm_bit_t);
      vvm_out_event(vvm_simulation*s, vvm_bit_t v, action_t o)
      : output_(o), sim_(s), val_(v) { }

      void event_function()
      { output_(sim_, val_); }

    private:
      const action_t output_;
      vvm_simulation*const sim_;
      const vvm_bit_t val_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_and {

    public:
      explicit vvm_and(vvm_out_event::action_t o)
      : output_(o) { }

      void init(unsigned idx, vvm_bit_t val) { input_[idx-1] = val; }

      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { if (input_[idx-1] == val)
		  return;
	      input_[idx-1] = val;
	      vvm_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval & input_[i];

	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vvm_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_nand {

    public:
      explicit vvm_nand(vvm_out_event::action_t o)
      : output_(o)
	    { for (unsigned idx = 0 ; idx < WIDTH ;  idx += 1)
		  input_[idx] = V0;
	    }

      void init(unsigned idx, vvm_bit_t val) { input_[idx-1] = val; }

	// Set an input of the NAND gate causes a new output value to
	// be calculated and an event generated to make the output
	// happen. The input pins are numbered from 1 - WIDTH.
      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { if (input_[idx-1] == val)
		    return;
	      input_[idx-1] = val;
	      vvm_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval & input_[i];

	      vvm_event*ev = new vvm_out_event(sim, not(outval), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vvm_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

/*
 * Simple inverter buffer.
 */
template <unsigned long DELAY> class vvm_not {

    public:
      explicit vvm_not(vvm_out_event::action_t o)
      : output_(o)
      { }

      void init(unsigned, vvm_bit_t) { }

      void set(vvm_simulation*sim, unsigned, vvm_bit_t val)
	    { vvm_bit_t outval = not(val);
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vvm_out_event::action_t output_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_xnor {

    public:
      explicit vvm_xnor(vvm_out_event::action_t o)
      : output_(o) { }

      void init(unsigned idx, vvm_bit_t val) { input_[idx-1] = val; }

      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { if (input_[idx-1] == val)
		    return;
	      input_[idx-1] = val;
	      vvm_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval ^ input_[i];

	      outval = not(outval);
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vvm_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_xor {

    public:
      explicit vvm_xor(vvm_out_event::action_t o)
      : output_(o) { }

      void init(unsigned idx, vvm_bit_t val) { input_[idx-1] = val; }

      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { if (input_[idx-1] == val)
		    return;
	      input_[idx-1] = val;
	      vvm_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval ^ input_[i];

	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vvm_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

/*
 * A Sequential UDP has a more complex truth table, and handles
 * edges. Pin 0 is an output, and all the remaining pins are
 * input. The WIDTH is the number of input pins.
 *
 * See vvm.txt for a description of the gate transition table.
 */
template <unsigned WIDTH> class vvm_udp_ssequ {

    public:
      explicit vvm_udp_ssequ(vvm_out_event::action_t o, vvm_bit_t i,
			    const vvm_u32*tab)
      : output_(o), table_(tab)
      { state_[0] = i;
        for (unsigned idx = 1; idx < WIDTH+1 ;  idx += 1)
	      state_[idx] = Vx;
      }

      void init(unsigned pin, vvm_bit_t val)
	    { state_[pin] = val; }

      void set(vvm_simulation*sim, unsigned pin, vvm_bit_t val)
	    { assert(pin > 0);
	      assert(pin < WIDTH+1);
	      if (val == Vz) val = Vx;
	      if (state_[pin] == val) return;
		// Convert the current state into a table index.
	      unsigned entry = 0;
	      for (unsigned idx = 0 ;  idx < WIDTH+1 ;  idx += 1) {
		    entry *= 3;
		    entry += state_[idx];
	      }
		// Get the table entry, and the 4bits that encode
		// activity on this pin.
	      vvm_u32 code = table_[entry];
	      code >>= 4 * (WIDTH-pin);
	      switch (state_[pin]*4 + val) {
		  case (V0*4 + V1):
		  case (V1*4 + V0):
		  case (Vx*4 + V0):
		    code = (code>>2) & 3;
		    break;
		  case (V0*4 + Vx):
		  case (V1*4 + Vx):
		  case (Vx*4 + V1):
		    code &= 3;
		    break;
	      }
		// Convert the code to a vvm_bit_t and run with it.
	      vvm_bit_t outval = (code == 0)? V0 : (code == 1)? V1 : Vx;
	      state_[0] = outval;
	      state_[pin] = val;
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      sim->insert_event(1, ev); // XXXX Delay not supported.
	    }

    private:
      vvm_out_event::action_t output_;
      const vvm_u32*const table_;
      vvm_bit_t state_[WIDTH+1];
};

class vvm_bufz {
    public:
      explicit vvm_bufz(vvm_out_event::action_t o)
      : output_(o)
      { }

      void init(unsigned idx, vvm_bit_t val) { }

      void set(vvm_simulation*sim, unsigned idx, vvm_bit_t val)
	    { output_(sim, val); }

    private:
      vvm_out_event::action_t output_;
};

class vvm_pevent {
    public:
      enum EDGE { ANYEDGE, POSEDGE, NEGEDGE };

      explicit vvm_pevent();
      void wait(EDGE, vvm_thread*);

      void set(vvm_simulation*sim, unsigned, vvm_bit_t val);
      vvm_bit_t get() const { return value_; }

    private:
      vvm_bit_t value_;
      vvm_thread*hold_;
      EDGE hold_edge_;

    private: // not implemented
      vvm_pevent(const vvm_pevent&);
      vvm_pevent& operator= (const vvm_pevent&);
};

/*
 * $Log: vvm_gates.h,v $
 * Revision 1.4  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.3  1998/12/17 23:54:58  steve
 *  VVM support for small sequential UDP objects.
 *
 * Revision 1.2  1998/11/10 00:48:31  steve
 *  Add support it vvm target for level-sensitive
 *  triggers (i.e. the Verilog wait).
 *  Fix display of $time is format strings.
 *
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */
#endif
