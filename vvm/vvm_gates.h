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
#ident "$Id: vvm_gates.h,v 1.1 1998/11/09 23:44:11 steve Exp $"
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
 * XXXX The WIDTH parameter is useless?
 */
template <unsigned long DELAY> class vvm_not {

    public:
      explicit vvm_not(vvm_out_event::action_t o)
      : output_(o)
      { }

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

class vvm_bufz {
    public:
      explicit vvm_bufz(vvm_out_event::action_t o)
      : output_(o)
      { }

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
 * Revision 1.1  1998/11/09 23:44:11  steve
 *  Add vvm library.
 *
 */
#endif
