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
#ident "$Id: vvm_gates.h,v 1.20 1999/11/14 18:22:12 steve Exp $"
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
      typedef void (*action_t)(vvm_simulation*, vpip_bit_t);
      vvm_out_event(vvm_simulation*s, vpip_bit_t v, action_t o)
      : output_(o), sim_(s), val_(v) { }

      void event_function()
      { output_(sim_, val_); }

    private:
      const action_t output_;
      vvm_simulation*const sim_;
      const vpip_bit_t val_;
};

/*
 * This template implements the LPM_ADD_SUB device type. The width of
 * the device is a template parameter. The device handles addition and
 * subtraction, selectable by the Add_Sub input. When configured as a
 * subtractor, the device works by adding the 2s complement of
 * DataB.
 */
template <unsigned WIDTH> class vvm_add_sub {

    public:
      vvm_add_sub() : ndir_(V0) { }

      void config_rout(unsigned idx, vvm_out_event::action_t a)
	    { o_[idx] = a;
	    }

      void init_DataA(unsigned idx, vpip_bit_t val)
	    { a_[idx] = val;
	    }

      void init_DataB(unsigned idx, vpip_bit_t val)
	    { b_[idx] = val;
	    }

      void init_Add_Sub(unsigned, vpip_bit_t val)
	    { ndir_ = not(val);
	    }

      void set_DataA(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { a_[idx] = val;
	      compute_(sim);
	    }
      void set_DataB(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { b_[idx] = val;
	      compute_(sim);
	    }

    private:
      vpip_bit_t a_[WIDTH];
      vpip_bit_t b_[WIDTH];
      vpip_bit_t r_[WIDTH];

	// this is the inverse of the Add_Sub port. It is 0 for add,
	// and 1 for subtract.
      vpip_bit_t ndir_;

      vvm_out_event::action_t o_[WIDTH];

      void compute_(vvm_simulation*sim)
	    { vpip_bit_t carry = ndir_;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1) {
		    vpip_bit_t val;
		    val = add_with_carry(a_[idx], b_[idx] ^ ndir_, carry);
		    if (val == r_[idx]) continue;
		    r_[idx] = val;
		    if (o_[idx] == 0) continue;
		    vvm_event*ev = new vvm_out_event(sim, val, o_[idx]);
		    sim->insert_event(0, ev);
	      }
	    }
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_and {

    public:
      explicit vvm_and(vvm_out_event::action_t o)
      : output_(o) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;

	      vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval & input_[i];
	      return outval;
	    }

      vpip_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

/*
 * This class simulates the LPM flip-flop device.
 * XXXX Inverted clock not yet supported.
 */
template <unsigned WIDTH> class vvm_ff {

    public:
      explicit vvm_ff()
	    { clk_ = Vx;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    q_[idx] = Vx;
	    }
      ~vvm_ff() { }

      void init_Data(unsigned idx, vpip_bit_t val) { d_[idx] = val; }
      void init_Clock(unsigned, vpip_bit_t val) { clk_ = val; }

      void set_Clock(vvm_simulation*sim, unsigned, vpip_bit_t val)
	    { if (val == clk_) return;
	      bool flag = posedge(clk_, val);
	      clk_ = val;
	      if (flag) latch_(sim);
	    }

      void set_Data(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { d_[idx] = val;
	    }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o;
	    }

    private:
      vpip_bit_t d_[WIDTH];
      vpip_bit_t q_[WIDTH];
      vpip_bit_t clk_;

      vvm_out_event::action_t out_[WIDTH];

      void latch_(vvm_simulation*sim)
	    { q_ = d_;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    if (out_[idx]) {
			  vvm_event*ev = new vvm_out_event(sim, q_[idx],
							   out_[idx]);
			  sim->active_event(ev);
		    }
	    }
};

/*
 * This class supports mux devices. The width is the width of the data
 * (or bus) path, SIZE is the number of alternative inputs and SELWID
 * is the size (in bits) of the selector input.
 */
template <unsigned WIDTH, unsigned SIZE, unsigned SELWID> class vvm_mux {

    public:
      explicit vvm_mux()
	    { sel_val_ = SIZE;
	      for (unsigned idx = 0;idx < WIDTH; idx += 1)
		    out_[idx] = 0;
	    }

      void init_Sel(unsigned idx, vpip_bit_t val)
	    { sel_[idx] = val; }

      void init_Data(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void set_Sel(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (sel_[idx] == val) return;
	      sel_[idx] = val;
	      evaluate_(sim);
	    }

      void set_Data(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val) return;
	      input_[idx] = val;
	      unsigned sel = idx / WIDTH;
	      if (sel != sel_val_) return;
	      unsigned off = idx % WIDTH;
	      vvm_event*ev = new vvm_out_event(sim, val, out_[off]);
	      sim->active_event(ev);
	    }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o; }

    private:
      vpip_bit_t sel_[SELWID];
      vpip_bit_t input_[WIDTH * SIZE];
      vvm_out_event::action_t out_[WIDTH];

      unsigned sel_val_;
      void evaluate_(vvm_simulation*sim)
	    { unsigned tmp = 0;
	      for (unsigned idx = 0 ;  idx < SELWID ;  idx += 1)
		    switch (sel_[idx]) {
			case V0:
			  break;
			case V1:
			  tmp |= (1<<idx);
			  break;
			default:
			  tmp = SIZE;
			  break;
		    }
	      if (tmp > SIZE) tmp = SIZE;
	      if (tmp == sel_val_) return;
	      sel_val_ = tmp;
	      if (sel_val_ == SIZE) {
		    for (unsigned idx = 0; idx < WIDTH ;  idx += 1) {
			  vvm_event*ev = new vvm_out_event(sim, Vx, out_[idx]);
			  sim->active_event(ev);
		    }
	      } else {
		    unsigned b = sel_val_ * WIDTH;
		    for (unsigned idx = 0; idx < WIDTH ;  idx += 1) {
			  vvm_event*ev = new vvm_out_event(sim,
							   input_[idx+b],
							   out_[idx]);
			  sim->active_event(ev);
		    }
	      }
	    }
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_or {

    public:
      explicit vvm_or(vvm_out_event::action_t o)
      : output_(o) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;

	      vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval | input_[i];
	      return outval;
	    }

      vpip_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_nor {

    public:
      explicit vvm_nor(vvm_out_event::action_t o)
      : output_(o) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;

	      vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval | input_[i];
	      return not(outval);
	    }

      vpip_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

template <unsigned long DELAY> class vvm_bufif1 {

    public:
      explicit vvm_bufif1(vvm_out_event::action_t o)
      : output_(o)
	    { input_[0] = Vx;
	      input_[1] = Vx;
	    }

      void set(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx-1] == val)
		  return;
	      input_[idx-1] = val;
	      vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void start(vvm_simulation*sim)
	    {
	    }

    private:
      vpip_bit_t input_[2];
      vvm_out_event::action_t output_;

      vpip_bit_t compute_() const
	    { if (input_[1] != V1) return Vz;
	      if (input_[0] == Vz) return Vx;
	      return input_[0];
	    }
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_nand {

    public:
      explicit vvm_nand(vvm_out_event::action_t o)
      : output_(o)
	    { for (unsigned idx = 0 ; idx < WIDTH ;  idx += 1)
		  input_[idx] = V0;
	    }

      void init_I(unsigned idx, vpip_bit_t val) { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

	// Set an input of the NAND gate causes a new output value to
	// be calculated and an event generated to make the output
	// happen. The input pins are numbered from 1 - WIDTH.
      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;

	      vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval & input_[i];
	      outval = not(outval);
	      return outval;
	    }

      vpip_bit_t input_[WIDTH];
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

      void init_I(unsigned, vpip_bit_t) { }
      void start(vvm_simulation*) { }

      void set_I(vvm_simulation*sim, unsigned, vpip_bit_t val)
	    { vpip_bit_t outval = not(val);
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

      void init_I(unsigned idx, vpip_bit_t val) { input_[idx] = val; }
      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }



      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      vpip_bit_t outval = compute_();
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval ^ input_[i];

	      outval = not(outval);
	      return outval;
	    }
      vpip_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

template <unsigned WIDTH, unsigned long DELAY> class vvm_xor {

    public:
      explicit vvm_xor(vvm_out_event::action_t o)
      : output_(o) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      start(sim);
	    }

    private:

      vpip_bit_t compute_() const
	    { vpip_bit_t outval = input_[0];
	      for (unsigned i = 1 ;  i < WIDTH ;  i += 1)
		    outval = outval ^ input_[i];
	      return outval;
	    }

      vpip_bit_t input_[WIDTH];
      vvm_out_event::action_t output_;
};

/*
 * This gate has only 3 pins, the output at pin 0 and two inputs. The
 * output is 1 or 0 if the two inputs are exactly equal or not.
 */
template <unsigned long DELAY> class vvm_eeq {

    public:
      explicit vvm_eeq(vvm_out_event::action_t o)
      : output_(o) { }

      void init(unsigned idx, vpip_bit_t val)
	    { input_[idx-1] = val; }

      void start(vvm_simulation*sim)
	    { vvm_event*ev = new vvm_out_event(sim, compute_(), output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

      void set(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx-1] == val)
		    return;
	      input_[idx-1] = val;
	      start(sim);
	    }

    private:

      vpip_bit_t compute_() const
	    { vpip_bit_t outval = V0;
	      if (input_[0] == input_[1])
		    outval = V1;
	      return outval;
	    }

      vpip_bit_t input_[2];
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
      explicit vvm_udp_ssequ(vvm_out_event::action_t o, vpip_bit_t i,
			    const vvm_u32*tab)
      : output_(o), table_(tab)
      { state_[0] = i;
        for (unsigned idx = 1; idx < WIDTH+1 ;  idx += 1)
	      state_[idx] = Vx;
      }

      void init(unsigned pin, vpip_bit_t val)
	    { state_[pin] = val; }

      void set(vvm_simulation*sim, unsigned pin, vpip_bit_t val)
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
		// Convert the code to a vpip_bit_t and run with it.
	      vpip_bit_t outval = (code == 0)? V0 : (code == 1)? V1 : Vx;
	      state_[0] = outval;
	      state_[pin] = val;
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      sim->insert_event(1, ev); // XXXX Delay not supported.
	    }

    private:
      vvm_out_event::action_t output_;
      const vvm_u32*const table_;
      vpip_bit_t state_[WIDTH+1];
};

class vvm_bufz {
    public:
      explicit vvm_bufz(vvm_out_event::action_t o)
      : output_(o)
      { }

      void init(unsigned idx, vpip_bit_t val) { }

      void set(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { output_(sim, val); }

    private:
      vvm_out_event::action_t output_;
};

/*
 * Threads use the vvm_sync to wait for something to happen. This
 * class cooperates with the vvm_pevent class that is the actual gates
 * that receive signals. By handling the suspension and the awakening
 * separately, I can trivially handle event OR expressions.
 *
 * When there is an event expression in the source, the elaborator
 * makes NetNEvent objects, which are approximately represented by the
 * vvm_pevent class.
 */
class vvm_sync {

    public:
      vvm_sync();

      void wait(vvm_thread*);
      void wakeup(vvm_simulation*sim);

    private:
      vvm_thread*hold_;

    private: // not implemented
      vvm_sync(const vvm_sync&);
      vvm_sync& operator= (const vvm_sync&);
};

template <unsigned WIDTH> class vvm_pevent {
    public:
      enum EDGE { ANYEDGE, POSEDGE, NEGEDGE };

      explicit vvm_pevent(vvm_sync*tgt, EDGE e)
      : target_(tgt), edge_(e)
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  value_[idx] = Vz;
	    }

      vvm_bitset_t<WIDTH> get() const { return value_; }

      void set_P(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (value_[idx] == val) return;
	      switch (edge_) {
		  case ANYEDGE:
		    target_->wakeup(sim);
		    break;
		  case POSEDGE:
		    if (val == V1)
			  target_->wakeup(sim);
		    break;
		  case NEGEDGE:
		    if (val == V0)
			  target_->wakeup(sim);
		    break;
	      }
	      value_[idx] = val;
	    }

      void init_P(int idx, vpip_bit_t val)
	    { assert(idx < WIDTH);
	      value_[idx] = val;
	    }

    private:
      vvm_sync*target_;
      vvm_bitset_t<WIDTH> value_;
      EDGE edge_;

    private: // not implemented
      vvm_pevent(const vvm_pevent&);
      vvm_pevent& operator= (const vvm_pevent&);
};

/*
 * $Log: vvm_gates.h,v $
 * Revision 1.20  1999/11/14 18:22:12  steve
 *  Fix NAND gate support to use named pins.
 *
 * Revision 1.19  1999/11/13 03:46:52  steve
 *  Support the LPM_MUX in vvm.
 *
 * Revision 1.18  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.17  1999/10/31 20:08:24  steve
 *  Include subtraction in LPM_ADD_SUB device.
 *
 * Revision 1.16  1999/10/31 04:11:28  steve
 *  Add to netlist links pin name and instance number,
 *  and arrange in vvm for pin connections by name
 *  and instance number.
 *
 * Revision 1.15  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 * Revision 1.14  1999/10/10 01:59:55  steve
 *  Structural case equals device.
 *
 * Revision 1.13  1999/10/09 19:24:36  steve
 *  NOR device.
 *
 * Revision 1.12  1999/07/17 03:07:27  steve
 *  pevent objects have initial values.
 *
 * Revision 1.11  1999/06/09 00:58:29  steve
 *  Support for binary | (Stephen Tell)
 *
 * Revision 1.10  1999/05/03 01:51:29  steve
 *  Restore support for wait event control.
 *
 * Revision 1.9  1999/05/01 20:43:55  steve
 *  Handle wide events, such as @(a) where a has
 *  many bits in it.
 *
 *  Add to vvm the binary ^ and unary & operators.
 *
 *  Dump events a bit more completely.
 *
 * Revision 1.8  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.7  1999/02/15 05:52:50  steve
 *  Mangle that handles device instance numbers.
 *
 * Revision 1.6  1999/01/31 18:15:55  steve
 *  Missing start methods.
 *
 * Revision 1.5  1999/01/01 01:44:56  steve
 *  Support the start() method.
 *
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
