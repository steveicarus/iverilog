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
#ident "$Id: vvm_gates.h,v 1.29 1999/12/02 04:54:11 steve Exp $"
#endif

# include  "vvm.h"
# include  <assert.h>

extern vpip_bit_t compute_nand(const vpip_bit_t*inp, unsigned count);
extern vpip_bit_t compute_and(const vpip_bit_t*inp, unsigned count);
extern vpip_bit_t compute_nor(const vpip_bit_t*inp, unsigned count);
extern vpip_bit_t compute_or(const vpip_bit_t*inp, unsigned count);
extern vpip_bit_t compute_xor(const vpip_bit_t*inp, unsigned count);
extern vpip_bit_t compute_xnor(const vpip_bit_t*inp, unsigned count);

extern void compute_mux(vpip_bit_t*out, unsigned wid,
			const vpip_bit_t*sel, unsigned swid,
			const vpip_bit_t*dat, unsigned size);

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

      vvm_out_event(vvm_simulation*s, vpip_bit_t v, action_t o);
      ~vvm_out_event();

      void event_function();

    private:
      const action_t output_;
      vvm_simulation*const sim_;
      const vpip_bit_t val_;
};

class vvm_1bit_out {

    public:
      vvm_1bit_out(vvm_out_event::action_t, unsigned delay);
      ~vvm_1bit_out();
      void output(vvm_simulation*sim, vpip_bit_t);

    private:
      vvm_out_event::action_t output_;
      unsigned delay_;
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

template <unsigned WIDTH, unsigned long DELAY>
class vvm_and  : private vvm_1bit_out {

    public:
      explicit vvm_and(vvm_out_event::action_t o)
      : vvm_1bit_out(o, DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim, compute_and(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val) return;
	      input_[idx] = val;
	      output(sim, compute_and(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH, unsigned WDIST> class vvm_clshift {

    public:
      explicit vvm_clshift()
	    { dir_ = V0;
	      dist_val_ = WIDTH;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    data_[idx] = Vx;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    out_[idx] = 0;
	      for (unsigned idx = 0 ;  idx < WDIST ;  idx += 1)
		    dist_[idx] = Vx;
	    }

      ~vvm_clshift() { }

      void init_Data(unsigned idx, vpip_bit_t val)
	    { data_[idx] = val;
	    }
      void init_Distance(unsigned idx, vpip_bit_t val)
	    { dist_[idx] = val;
	      calculate_dist_();
	    }
      void init_Direction(vpip_bit_t val)
	    { dir_ = val; }

      void set_Data(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (data_[idx] == val) return;
	      data_[idx] = val;
	      if ((dist_val_ + idx) >= WIDTH) return;
	      if ((dist_val_ + idx) < 0) return;
	      vvm_out_event::action_t out = out_[dist_val_+idx];
	      if (out == 0) return;
	      vvm_event*ev = new vvm_out_event(sim, val, out);
	      sim->active_event(ev);
	    }

      void set_Distance(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (dist_[idx] == val) return;
	      dist_[idx] = val;
	      calculate_dist_();
	      compute_(sim);
	    }

      void set_Direction(vvm_simulation*sim, unsigned, vpip_bit_t val)
	    { if (dir_ == val) return;
	      dir_ = val;
	      calculate_dist_();
	      compute_(sim);
	    }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o;
	    }

    private:
      vpip_bit_t dir_;
      vpip_bit_t data_[WIDTH];
      vpip_bit_t dist_[WDIST];
      vvm_out_event::action_t out_[WIDTH];
      int dist_val_;

      void calculate_dist_()
	    { int tmp = 0;
	      for (unsigned idx = 0 ;  idx < WDIST ;  idx += 1)
		    switch (dist_[idx]) {
			case V0:
			  break;
			case V1:
			  tmp |= 1<<idx;
			  break;
			default:
			  tmp = WIDTH;
		    }
	      if (tmp > WIDTH)
		    tmp = WIDTH;
	      else if (dir_ == V1)
		    tmp = -tmp;
	      dist_val_ = tmp;
	    }

      void compute_(vvm_simulation*sim)
	    { vvm_event*ev;
	      if (dist_val_ == WIDTH) {
		    for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1) {
			  if (out_[idx] == 0) continue;
			  ev = new vvm_out_event(sim, Vx, out_[idx]);
			  sim->active_event(ev);
		    }
		    return;
	      }
	      for (int idx = 0 ;  idx < WIDTH ;  idx += 1) {
		    if (out_[idx] == 0) continue;
		    vpip_bit_t val;
		    if ((idx-dist_val_) >= WIDTH) val = V0;
		    else if ((idx-dist_val_) < 0) val = V0;
		    else val = data_[idx-dist_val_];
		    ev = new vvm_out_event(sim, val, out_[idx]);
		    sim->active_event(ev);
	      }
	    }
};

template <unsigned WIDTH> class vvm_compare {

    public:
      explicit vvm_compare()
	    { out_lt_ = 0;
	      out_le_ = 0;
	      gt_ = Vx;
	      lt_ = Vx;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1) {
		    a_[idx] = Vx;
		    b_[idx] = Vx;
	      }
	    }
      ~vvm_compare() { }

      void init_DataA(unsigned idx, vpip_bit_t val)
	    { a_[idx] = val; }
      void init_DataB(unsigned idx, vpip_bit_t val)
	    { b_[idx] = val; }

      void set_DataA(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (a_[idx] == val) return;
	      a_[idx] = val;
	      compute_(sim);
	    }

      void set_DataB(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (b_[idx] == val) return;
	      b_[idx] = val;
	      compute_(sim);
	    }

      void config_ALB_out(vvm_out_event::action_t o)
	    { out_lt_ = o; }

      void config_ALEB_out(vvm_out_event::action_t o)
	    { out_le_ = o; }

      void config_AGB_out(vvm_out_event::action_t o)
	    { out_gt_ = o; }

      void config_AGEB_out(vvm_out_event::action_t o)
	    { out_ge_ = o; }

    private:
      vpip_bit_t a_[WIDTH];
      vpip_bit_t b_[WIDTH];

      vpip_bit_t gt_;
      vpip_bit_t lt_;

      vvm_out_event::action_t out_lt_;
      vvm_out_event::action_t out_le_;
      vvm_out_event::action_t out_gt_;
      vvm_out_event::action_t out_ge_;

      void compute_(vvm_simulation*sim)
	    { vpip_bit_t gt = V0;
	      vpip_bit_t lt = V0;
	      vvm_event*ev;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1) {
		    gt = greater_with_cascade(a_[idx], b_[idx], gt);
		    lt = less_with_cascade(a_[idx], b_[idx], lt);
	      }

	      if ((gt_ == gt) && (lt_ == lt)) return;
	      gt_ = gt;
	      lt_ = lt;
	      if (out_lt_) {
		    ev = new vvm_out_event(sim, lt_, out_lt_);
		    sim->active_event(ev);
	      }
	      if (out_le_) {
		    ev = new vvm_out_event(sim, not(gt_), out_le_);
		    sim->active_event(ev);
	      }
	      if (out_gt_) {
		    ev = new vvm_out_event(sim, gt_, out_gt_);
		    sim->active_event(ev);
	      }
	      if (out_ge_) {
		    ev = new vvm_out_event(sim, not(lt_), out_ge_);
		    sim->active_event(ev);
	      }
	    }
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
	    { unsigned idx;
	      for (idx = 0 ;  idx < WIDTH ;  idx += 1) out_[idx] = 0;
	      for (idx = 0 ;  idx < WIDTH ;  idx += 1) output_[idx] = Vx;
	      for (idx = 0 ;  idx < SELWID;  idx += 1) sel_[idx] = Vx;
	      for (idx = 0 ;  idx < WIDTH*SIZE;  idx += 1) input_[idx] = Vx;
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
	      evaluate_(sim);
	    }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o; }

    private:
      vpip_bit_t sel_[SELWID];
      vpip_bit_t input_[WIDTH * SIZE];
      vpip_bit_t output_[WIDTH];

      vvm_out_event::action_t out_[WIDTH];

      void evaluate_(vvm_simulation*sim)
	    { vpip_bit_t tmp[WIDTH];
	      compute_mux(tmp, WIDTH, sel_, SELWID, input_, SIZE);
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    if (tmp[idx] != output_[idx]) {
			  output_[idx] = tmp[idx];
			  vvm_event*ev = new vvm_out_event(sim,
							   output_[idx],
							   out_[idx]);
			  sim->active_event(ev);
		    }
	    }
};

template <unsigned WIDTH, unsigned long DELAY> 
class vvm_or : private vvm_1bit_out {

    public:
      explicit vvm_or(vvm_out_event::action_t o)
      : vvm_1bit_out(o,DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim, compute_or(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;
	      output(sim, compute_or(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH, unsigned long DELAY>
class vvm_nor  : private vvm_1bit_out {

    public:
      explicit vvm_nor(vvm_out_event::action_t o)
      : vvm_1bit_out(o, DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim, compute_nor(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;
	      output(sim, compute_nor(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH, unsigned AWIDTH, unsigned SIZE>
class vvm_ram_dq  : protected vvm_ram_callback {

    public:
      vvm_ram_dq(vvm_memory_t<WIDTH,SIZE>*mem)
      : mem_(mem)
	    { mem->set_callback(this);
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    out_[idx] = 0;
	      for (unsigned idx = 0 ;  idx < AWIDTH ;  idx += 1)
		    addr_[idx] = Vx;
	    }

      void init_Address(unsigned idx, vpip_bit_t val)
	    { addr_[idx] = val; }

      void set_Address(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (addr_[idx] == val) return;
	      addr_[idx] = val;
	      compute_();
	      send_out_(sim);
	    }

      void handle_write(vvm_simulation*sim, unsigned idx)
	    { if (idx == addr_val_) send_out_(sim); }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o; }

    private:
      vvm_memory_t<WIDTH,SIZE>*mem_;
      vpip_bit_t addr_[AWIDTH];
      vvm_out_event::action_t out_[WIDTH];

      unsigned long addr_val_;

      void compute_()
	    { unsigned bit;
	      unsigned mask;
	      addr_val_ = 0;
	      for (bit = 0, mask = 1 ;  bit < AWIDTH ;  bit += 1, mask <<= 1)
		    if (addr_[bit] == V1) addr_val_ |= mask;
	    }

      void send_out_(vvm_simulation*sim)
	    { vvm_bitset_t<WIDTH>ov = mem_->get_word(addr_val_);
	      for (unsigned bit = 0 ;  bit < WIDTH ;  bit += 1)
		    if (out_[bit]) {
			  vvm_event*ev = new vvm_out_event(sim,
							   ov[bit],
							   out_[bit]);
			  sim->active_event(ev);
		    }
	    }
};

template <unsigned long DELAY> class vvm_buf {

    public:
      explicit vvm_buf(vvm_out_event::action_t o)
      : output_(o)
      { }

      void init_I(unsigned, vpip_bit_t) { }
      void start(vvm_simulation*) { }

      void set_I(vvm_simulation*sim, unsigned, vpip_bit_t val)
	    { vpip_bit_t outval = val;
	      if (val == Vz) val = Vx;
	      vvm_event*ev = new vvm_out_event(sim, outval, output_);
	      if (DELAY > 0)
		    sim->insert_event(DELAY, ev);
	      else
		    sim->active_event(ev);
	    }

    private:
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

template <unsigned WIDTH, unsigned long DELAY> 
class vvm_nand : private vvm_1bit_out {

    public:
      explicit vvm_nand(vvm_out_event::action_t o)
      : vvm_1bit_out(o, DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim, compute_nand(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val) return;
	      input_[idx] = val;
	      output(sim, compute_nand(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

/*
 * Simple inverter buffer.
 */
template <unsigned long DELAY> class vvm_not  : private vvm_1bit_out {

    public:
      explicit vvm_not(vvm_out_event::action_t o)
      : vvm_1bit_out(o, DELAY) { }

      void init_I(unsigned, vpip_bit_t) { }
      void start(vvm_simulation*) { }

      void set_I(vvm_simulation*sim, unsigned, vpip_bit_t val)
	    { output(sim, not(val)); }

    private:
};

template <unsigned WIDTH, unsigned long DELAY> 
class vvm_xnor : private vvm_1bit_out {

    public:
      explicit vvm_xnor(vvm_out_event::action_t o)
      : vvm_1bit_out(o,DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val) { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim,compute_xnor(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      output(sim,compute_xnor(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH, unsigned long DELAY> 
class vvm_xor : private vvm_1bit_out {

    public:
      explicit vvm_xor(vvm_out_event::action_t o)
      : vvm_1bit_out(o,DELAY) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start(vvm_simulation*sim)
	    { output(sim,compute_xor(input_,WIDTH)); }

      void set_I(vvm_simulation*sim, unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      output(sim,compute_xor(input_,WIDTH)); }

    private:
      vpip_bit_t input_[WIDTH];
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
 * Revision 1.29  1999/12/02 04:54:11  steve
 *  Handle mux sel of X, if inputs are equal.
 *
 * Revision 1.28  1999/11/25 01:34:04  steve
 *  Reduce more gate templates to use vvm_1bit_out (Eric Aardoom)
 *
 * Revision 1.27  1999/11/24 04:38:49  steve
 *  LT and GT fixes from Eric Aardoom.
 *
 * Revision 1.26  1999/11/22 00:30:52  steve
 *  Detemplate some and, or and nor methods.
 *
 * Revision 1.25  1999/11/21 01:16:51  steve
 *  Fix coding errors handling names of logic devices,
 *  and add support for buf device in vvm.
 *
 * Revision 1.24  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.23  1999/11/15 00:42:31  steve
 *  Fixup to include right shift support.
 *
 * Revision 1.22  1999/11/14 23:43:46  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.21  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
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
 */
#endif
