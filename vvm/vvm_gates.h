#ifndef __vvm_gates_H
#define __vvm_gates_H
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvm_gates.h,v 1.46 2000/03/18 01:27:00 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_signal.h"
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
      typedef void (*action_t)(vpip_bit_t); // XXXX Remove me!

      vvm_out_event(vpip_bit_t v, vvm_nexus::drive_t*o);
      ~vvm_out_event();

      void event_function();

    private:
      vvm_nexus::drive_t*output_;
      const vpip_bit_t val_;
};

class vvm_1bit_out  : public vvm_nexus::drive_t  {

    public:
      vvm_1bit_out(unsigned delay);
      ~vvm_1bit_out();
      void output(vpip_bit_t);

    private:
      unsigned delay_;
};

/*
 * This class implements the LPM_ADD_SUB device type. The width of
 * the device is a constructor parameter. The device handles addition and
 * subtraction, selectable by the Add_Sub input. When configured as a
 * subtractor, the device works by adding the 2s complement of
 * DataB.
 */
class vvm_add_sub : public vvm_nexus::recvr_t {

    public:
      explicit vvm_add_sub(unsigned width);
      ~vvm_add_sub();

      vvm_nexus::drive_t* config_rout(unsigned idx);
      vvm_nexus::drive_t* config_cout();

      unsigned key_DataA(unsigned idx) const;
      unsigned key_DataB(unsigned idx) const;

      void init_DataA(unsigned idx, vpip_bit_t val);
      void init_DataB(unsigned idx, vpip_bit_t val);
      void init_Add_Sub(unsigned, vpip_bit_t val);

      void start();

    private:
      void take_value(unsigned key, vpip_bit_t val);

      unsigned width_;
      vpip_bit_t*ibits_;
      vpip_bit_t c_;

	// this is the inverse of the Add_Sub port. It is 0 for add,
	// and 1 for subtract.
      vpip_bit_t ndir_;

      vvm_nexus::drive_t*ro_;
      vvm_nexus::drive_t co_;

      void compute_();

    private: // not implemented
      vvm_add_sub(const vvm_add_sub&);
      vvm_add_sub& operator= (const vvm_add_sub&);
};

/*
 * These are implementations of reduction AND. the vvm_and class is
 * the parameterized form, that takes the width of the gate input as a
 * parameter. The vvm_andN classes are versions that have specific
 * widths. The latter should be preferred over the generic form.
 */
template <unsigned WIDTH>
class vvm_and  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_and(unsigned d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start()
	    { output(compute_and(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val) return;
	      input_[idx] = val;
	      output(compute_and(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

class vvm_and2  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_and2(unsigned long d);
      ~vvm_and2();

      void init_I(unsigned idx, vpip_bit_t val);
      void start();

    private:
      void take_value(unsigned key, vpip_bit_t val);

      vpip_bit_t input_[2];
};

/*
 * This class implements LPM_CLSHIFT devices with specified data width
 * and selector input width. The direction bit is a single bit input.
 */
class vvm_clshift  : public vvm_nexus::recvr_t {

    public:
      vvm_clshift(unsigned wid, unsigned dwid);
      ~vvm_clshift();

      void init_Data(unsigned idx, vpip_bit_t val);
      void init_Distance(unsigned idx, vpip_bit_t val);
      void init_Direction(unsigned, vpip_bit_t val);

      vvm_nexus::drive_t* config_rout(unsigned idx);

      unsigned key_Data(unsigned idx) const;
      unsigned key_Distance(unsigned idx) const;
      unsigned key_Direction(unsigned) const;

    private:
      void take_value(unsigned key, vpip_bit_t val);

    private:
      unsigned width_, wdist_;
      vpip_bit_t dir_;
      int dist_val_;

      vpip_bit_t*ibits_;
      vvm_nexus::drive_t*out_;

      void calculate_dist_();
      void compute_();
};


/*
 * This class implements structural comparators, specifically the
 * LPM_COMPARE device type.
 */
class vvm_compare  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_compare(unsigned w);
      ~vvm_compare();

      void init_DataA(unsigned idx, vpip_bit_t val);
      void init_DataB(unsigned idx, vpip_bit_t val);

      unsigned key_DataA(unsigned idx) const;
      unsigned key_DataB(unsigned idx) const;

      vvm_nexus::drive_t* config_ALB_out();
      vvm_nexus::drive_t* config_ALEB_out();
      vvm_nexus::drive_t* config_AGB_out();
      vvm_nexus::drive_t* config_AGEB_out();

    private:
      void take_value(unsigned key, vpip_bit_t val);

      unsigned width_;
      vpip_bit_t*ibits_;

      vpip_bit_t gt_;
      vpip_bit_t lt_;

      vvm_nexus::drive_t out_lt_;
      vvm_nexus::drive_t out_le_;
      vvm_nexus::drive_t out_gt_;
      vvm_nexus::drive_t out_ge_;

      void compute_();

    private: // not implemented
      vvm_compare(const vvm_compare&);
      vvm_compare& operator= (const vvm_compare&);
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

      void set_Clock(unsigned, vpip_bit_t val)
	    { if (val == clk_) return;
	      bool flag = posedge(clk_, val);
	      clk_ = val;
	      if (flag) latch_();
	    }

      void set_Data(unsigned idx, vpip_bit_t val)
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

      void latch_()
	    { q_ = d_;
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    if (out_[idx]) {
			  vvm_event*ev = new vvm_out_event(q_[idx], out_[idx]);
			  ev->schedule();
		    }
	    }
};

/*
 * This class behaves like a combinational multiplier. The device
 * behaves like the LPM_MULT device.
 */
class vvm_mult  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_mult(unsigned rwid, unsigned awid,
			unsigned bwid, unsigned swid);
      ~vvm_mult();

      void init_DataA(unsigned idx, vpip_bit_t val);
      void init_DataB(unsigned idx, vpip_bit_t val);
      void init_Sum(unsigned idx, vpip_bit_t val);

      vvm_nexus::drive_t* config_rout(unsigned idx);
      unsigned key_DataA(unsigned idx) const;
      unsigned key_DataB(unsigned idx) const;
      unsigned key_Sum(unsigned idx) const;

    private:
      void take_value(unsigned key, vpip_bit_t val);

      unsigned rwid_;
      unsigned awid_;
      unsigned bwid_;
      unsigned swid_;
      vpip_bit_t*bits_;
      vvm_nexus::drive_t*out_;
};

/*
 * This class supports mux devices. The width is the width of the data
 * (or bus) path, SIZE is the number of alternative inputs and SELWID
 * is the size (in bits) of the selector input. The device passes to
 * the output the bits of the input selected by the selector.
 */
class vvm_mux  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_mux(unsigned width, unsigned size, unsigned selwid);
      ~vvm_mux();

      void init_Sel(unsigned idx, vpip_bit_t val);
      void init_Data(unsigned idx, vpip_bit_t val);

      vvm_nexus::drive_t* config_rout(unsigned idx);

      unsigned key_Sel(unsigned idx) const;
      unsigned key_Data(unsigned wi, unsigned si) const;

    private:
      void take_value(unsigned key, vpip_bit_t val);

      unsigned width_, size_, selwid_;
      vpip_bit_t*bits_;

      vvm_nexus::drive_t*out_;

      void evaluate_();

    private: // not implemented
      vvm_mux(const vvm_mux&);
      vvm_mux& operator= (vvm_mux&);
};

template <unsigned WIDTH> 
class vvm_or : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_or(unsigned long d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start()
	    { output(compute_or(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;
	      output(compute_or(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH>
class vvm_nor  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_nor(unsigned long d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start()
	    { output(compute_nor(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		  return;
	      input_[idx] = val;
	      output(compute_nor(input_,WIDTH));
	    }

      vpip_bit_t input_[WIDTH];
};

class vvm_nor2  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_nor2(unsigned long d);
      ~vvm_nor2();

      void init_I(unsigned idx, vpip_bit_t val);
      void start();

    private:
      void take_value(unsigned key, vpip_bit_t val);
      vpip_bit_t input_[2];
};

/*
 * This object implements a LPM_RAM_DQ device, except for the actual
 * contents of the memory, which are stored in a vvm_memory_t
 * object. Note that there may be many vvm_ram_dq items referencing
 * the same memory.
 *
 * XXXX Only asynchronous reads are supported.
 * XXXX Only *synchronous* writes with WE are supported.
 */
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

      void init_Data(unsigned idx, vpip_bit_t val)
	    { data_[idx] = val; }

      void init_WE(unsigned, vpip_bit_t val)
	    { we_ = val; }

      void init_InClock(unsigned, vpip_bit_t val)
	    { iclk_ = val; }

      void set_Address(unsigned idx, vpip_bit_t val)
	    { if (addr_[idx] == val) return;
	      addr_[idx] = val;
	      compute_();
	      send_out_();
	    }

      void set_Data(unsigned idx, vpip_bit_t val)
	    { data_[idx] = val; }

      void set_WE(unsigned, vpip_bit_t val)
	    { we_ = val; }

      void set_InClock(unsigned, vpip_bit_t val)
	    { if (val == iclk_) return;
	      vpip_bit_t tmp = iclk_;
	      iclk_ = val;
	      if (we_ != V1) return;
	      if (posedge(tmp, val)) mem_->set_word(addr_val_, data_);
	    }

      void handle_write(unsigned idx)
	    { if (idx == addr_val_) send_out_(); }

      void config_rout(unsigned idx, vvm_out_event::action_t o)
	    { out_[idx] = o; }

    private:
      vvm_memory_t<WIDTH,SIZE>*mem_;

      vpip_bit_t addr_[AWIDTH];
      vpip_bit_t data_[WIDTH];
      vpip_bit_t we_;
      vpip_bit_t iclk_;

      vvm_out_event::action_t out_[WIDTH];

      unsigned long addr_val_;

      void compute_()
	    { unsigned bit;
	      unsigned mask;
	      addr_val_ = 0;
	      for (bit = 0, mask = 1 ;  bit < AWIDTH ;  bit += 1, mask <<= 1)
		    if (addr_[bit] == V1) addr_val_ |= mask;
	    }

      void send_out_()
	    { vvm_bitset_t<WIDTH>ov = mem_->get_word(addr_val_);
	      for (unsigned bit = 0 ;  bit < WIDTH ;  bit += 1)
		    if (out_[bit]) {
			  vvm_event*ev = new vvm_out_event(ov[bit],
							   out_[bit]);
			  ev->schedule();
		    }
	    }
};

class vvm_buf  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_buf(unsigned long d);
      ~vvm_buf();

      void init_I(unsigned, vpip_bit_t);
      void start() { }
    private:
      void take_value(unsigned, vpip_bit_t val);
};

class vvm_bufif1  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_bufif1(unsigned long d);
      ~vvm_bufif1();

      void init_I(unsigned, vpip_bit_t);
      void start() { }

    private:
      vpip_bit_t input_[2];
      void take_value(unsigned key, vpip_bit_t val);
};

template <unsigned WIDTH> 
class vvm_nand : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_nand(unsigned long d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start()
	    { output(compute_nand(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val) return;
	      input_[idx] = val;
	      output(compute_nand(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

/*
 * Simple inverter buffer.
 */
class vvm_not  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_not(unsigned long d);
      ~vvm_not();

      void init_I(unsigned, vpip_bit_t);
      void start();
    private:
      void take_value(unsigned key, vpip_bit_t val);
};


template <unsigned WIDTH> 
class vvm_xnor : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_xnor(unsigned long d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val) { input_[idx] = val; }

      void start()
	    { output(compute_xnor(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      output(compute_xnor(input_,WIDTH));
	    }

    private:
      vpip_bit_t input_[WIDTH];
};

template <unsigned WIDTH> 
class vvm_xor : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_xor(unsigned long d)
      : vvm_1bit_out(d) { }

      void init_I(unsigned idx, vpip_bit_t val)
	    { input_[idx] = val; }

      void start()
	    { output(compute_xor(input_,WIDTH)); }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_I(key, val); }

      void set_I(unsigned idx, vpip_bit_t val)
	    { if (input_[idx] == val)
		    return;
	      input_[idx] = val;
	      output(compute_xor(input_,WIDTH)); }

    private:
      vpip_bit_t input_[WIDTH];
};

/*
 * This gate has only 3 pins, the output at pin 0 and two inputs. The
 * output is 1 or 0 if the two inputs are exactly equal or not.
 */
class vvm_eeq  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_eeq(unsigned long d);
      ~vvm_eeq();

      void init_I(unsigned idx, vpip_bit_t val);

      void start();

    private:
      void take_value(unsigned key, vpip_bit_t val);

      vpip_bit_t compute_() const;

      vpip_bit_t input_[2];
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

      void set(unsigned pin, vpip_bit_t val)
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
	      vvm_event*ev = new vvm_out_event(outval, output_);
	      ev->schedule(1); // XXXX Delay not supported.
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

      void set(unsigned idx, vpip_bit_t val)
	    { output_(val); }

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
      void wakeup();

    private:
      vvm_thread*hold_;

    private: // not implemented
      vvm_sync(const vvm_sync&);
      vvm_sync& operator= (const vvm_sync&);
};

template <unsigned WIDTH> class vvm_pevent : public vvm_nexus::recvr_t {
    public:
      enum EDGE { ANYEDGE, POSEDGE, NEGEDGE };

      explicit vvm_pevent(vvm_sync*tgt, EDGE e)
      : target_(tgt), edge_(e)
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  value_[idx] = Vz;
	    }

      vvm_bitset_t<WIDTH> get() const { return value_; }

      void init_P(int idx, vpip_bit_t val)
	    { assert(idx < WIDTH);
	      value_[idx] = val;
	    }

    private:
      void take_value(unsigned key, vpip_bit_t val) { set_P(key, val); }

      void set_P(unsigned idx, vpip_bit_t val)
	    { if (value_[idx] == val) return;
	      switch (edge_) {
		  case ANYEDGE:
		    target_->wakeup();
		    break;
		  case POSEDGE:
		    if (val == V1)
			  target_->wakeup();
		    break;
		  case NEGEDGE:
		    if (val == V0)
			  target_->wakeup();
		    break;
	      }
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
 * Revision 1.46  2000/03/18 01:27:00  steve
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
 * Revision 1.45  2000/03/17 19:24:00  steve
 *  nor2 and and2 optimized gates.
 *
 * Revision 1.44  2000/03/17 17:25:53  steve
 *  Adder and comparator in nexus style.
 *
 * Revision 1.43  2000/03/17 03:36:07  steve
 *  Remove some useless template parameters.
 *
 * Revision 1.42  2000/03/17 03:05:13  steve
 *  Update vvm_mult to nexus style.
 *
 * Revision 1.41  2000/03/17 02:22:03  steve
 *  vvm_clshift implementation without templates.
 *
 * Revision 1.40  2000/03/16 23:13:49  steve
 *  Update LPM_MUX to nexus style.
 *
 * Revision 1.39  2000/03/16 21:47:27  steve
 *  Update LMP_CLSHIFT to use nexus interface.
 *
 * Revision 1.38  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 * Revision 1.37  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.36  2000/02/23 02:56:57  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.35  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.34  1999/12/19 20:57:07  steve
 *  Proper init_ method prototype.
 *
 * Revision 1.33  1999/12/16 02:42:15  steve
 *  Simulate carry output on adders.
 *
 * Revision 1.32  1999/12/12 19:47:54  steve
 *  Remove the useless vvm_simulation class.
 *
 * Revision 1.31  1999/12/05 02:24:09  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 */
#endif
