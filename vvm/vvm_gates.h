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
#ident "$Id: vvm_gates.h,v 1.56 2000/04/10 05:26:07 steve Exp $"
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
 * This class simulates the LPM flip-flop device. The vvm_ff class
 * supports arbitrary width devices. For each output Q, there is a
 * unique input D. The CLK input is common for all the bit lanes.
 */
class vvm_ff  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_ff(unsigned wid);
      ~vvm_ff();

      vvm_nexus::drive_t* config_rout(unsigned idx);

      unsigned key_Data(unsigned idx) const;
      unsigned key_Clock() const;

      void init_Data(unsigned idx, vpip_bit_t val);
      void init_Clock(unsigned, vpip_bit_t val);

    private:
      void take_value(unsigned key, vpip_bit_t val);
      unsigned width_;
      vpip_bit_t*bits_;
      vpip_bit_t clk_;

      vvm_nexus::drive_t* out_;

      void latch_();

    private: // not implemeneted
      vvm_ff(const vvm_ff&);
      vvm_ff& operator= (const vvm_ff&);
};

/*
 * This class behaves like a combinational divider. There isn't really
 * such a practical device, but this is useful for simulating code
 * that includes a / operator in structural contexts.
 */
class vvm_idiv  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_idiv(unsigned rwid, unsigned awid, unsigned bwid);
      ~vvm_idiv();

      void init_DataA(unsigned idx, vpip_bit_t val);
      void init_DataB(unsigned idx, vpip_bit_t val);

      vvm_nexus::drive_t* config_rout(unsigned idx);
      unsigned key_DataA(unsigned idx) const;
      unsigned key_DataB(unsigned idx) const;

    private:
      void take_value(unsigned key, vpip_bit_t val);

      unsigned rwid_;
      unsigned awid_;
      unsigned bwid_;
      vpip_bit_t*bits_;
      vvm_nexus::drive_t*out_;
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
class vvm_ram_dq  : protected vvm_ram_callback,  public vvm_nexus::recvr_t {

    public:
      vvm_ram_dq(vvm_memory_t<WIDTH,SIZE>*mem)
      : mem_(mem)
	    { mem->set_callback(this);
	      for (unsigned idx = 0 ;  idx < AWIDTH+WIDTH+2 ;  idx += 1)
		    ibits_[idx] = StX;
	    }

      void init_Address(unsigned idx, vpip_bit_t val)
	    { ibits_[idx] = val; }

      void init_Data(unsigned idx, vpip_bit_t val)
	    { ibits_[AWIDTH+idx] = val; }

      void init_WE(unsigned, vpip_bit_t val)
	    { ibits_[AWIDTH+WIDTH] = val; }

      void init_InClock(unsigned, vpip_bit_t val)
	    { ibits_[AWIDTH+WIDTH+1] = val; }

      unsigned key_Address(unsigned idx) const { return idx; }
      unsigned key_Data(unsigned idx) const { return AWIDTH+idx; }
      unsigned key_WE() const { return AWIDTH+WIDTH + 0; }
      unsigned key_InClock() const { return AWIDTH+WIDTH + 1; }

      vvm_nexus::drive_t* config_rout(unsigned idx) { return out_+idx; }

      void take_value(unsigned key, vpip_bit_t val)
      { if (ibits_[key] == val) return;
        if (key == key_InClock()) {
	      vpip_bit_t tmp = ibits_[key];
	      ibits_[key] = val;
	      if (B_IS1(ibits_[key_WE()])) return;
	      if (posedge(tmp, val)) mem_->set_word(addr_val_, ibits_+AWIDTH);
	      return;
	} else {
	      ibits_[key] = val;
	      if (key < AWIDTH) {
		    compute_();
		    send_out_();
	      }
	}
      }


      void handle_write(unsigned idx) { if (idx == addr_val_) send_out_(); }

    private:
      vvm_memory_t<WIDTH,SIZE>*mem_;

      vpip_bit_t ibits_[AWIDTH+WIDTH+2];
      vvm_nexus::drive_t out_[WIDTH];

      unsigned long addr_val_;

      void compute_()
	    { unsigned bit;
	      unsigned mask;
	      addr_val_ = 0;
	      for (bit = 0, mask = 1 ;  bit < AWIDTH ;  bit += 1, mask <<= 1)
		    if (B_IS1(ibits_[bit])) addr_val_ |= mask;
	    }

      void send_out_()
	    { vpip_bit_t ov_bits[WIDTH];
	      vvm_bitset_t ov(ov_bits, WIDTH);
	      mem_->get_word(addr_val_, ov);
	      for (unsigned bit = 0 ;  bit < WIDTH ;  bit += 1) {
		    vvm_out_event*ev = new vvm_out_event(ov[bit], out_+bit);
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
 * This class allows programmers to define combinational primitives at
 * truth tables. The device has a single bit output, and any fixed
 * width.
 *
 * The truth table is specified as a string of characters organized as
 * a table. Every width+1 characters represents one row, including the
 * set of inputs and the output that they generated. There can be any
 * number of rows in the table, which is terminated by a nul byte. The
 * table is passed to the constructor as a constant string.
 *
 * This is a simple example of a truth table for an inverter. The
 * width is 1, so there are two characters in each row. The last
 * character in each row is the output if all the other characters in
 * the row match the input. As you can see, this table inverts its
 * input and outputs 'x' if the input is unknown.
 *
 *  const char*invert_table = 
 *          "01"
 *          "10"
 *          "xx";
 *
 * The valid input characters are '0', '1' and 'x'. The valid output
 * characters are '0', '1', 'x' and 'z'. (The last is not supported by
 * Verilog primitives, so ivl will never generate it.)
 */
class vvm_udp_comb  : public vvm_1bit_out, public vvm_nexus::recvr_t {

    public:
      explicit vvm_udp_comb(unsigned w, const char*t);
      ~vvm_udp_comb();

      void init_I(unsigned idx, vpip_bit_t val);
      void start();

    private:
      void take_value(unsigned key, vpip_bit_t val);
      vpip_bit_t*ibits_;

      unsigned width_;
      const char*table_;
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

/*
 * The bufz is a trivial device that simply passes its input to its
 * output. Unlike a buf devince, this does not change Vz values to Vx,
 * it instead passes the Vz unaltered.
 *
 * This device is useful for isolating nets.
 */
class vvm_bufz  : public vvm_nexus::recvr_t, public vvm_nexus::drive_t {
    public:
      explicit vvm_bufz();
      ~vvm_bufz();

      void init(unsigned idx, vpip_bit_t val);

    private:
      void take_value(unsigned, vpip_bit_t val);
};

/*
 * Threads use the vvm_sync to wait for something to happen. This
 * class cooperates with the various event source classes that receive
 * events and trigger the associated vvm_sync object.
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

class vvm_anyedge  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_anyedge(vvm_sync*tgt, unsigned n);
      ~vvm_anyedge();

      void init_P(unsigned idx, vpip_bit_t val);

    private:
      void take_value(unsigned key, vpip_bit_t val);

      vpip_bit_t*val_;
      unsigned nval_;

      vvm_sync*sync_;

    private: // not implemented
      vvm_anyedge(const vvm_anyedge&);
      vvm_anyedge& operator= (const vvm_anyedge&);
};

class vvm_negedge  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_negedge(vvm_sync*tgt);
      ~vvm_negedge();

      void init_P(int idx, vpip_bit_t val);

    private:
      void take_value(unsigned key, vpip_bit_t val);
      vpip_bit_t val_;

      vvm_sync*sync_;

    private: // not implemented
      vvm_negedge(const vvm_negedge&);
      vvm_negedge& operator= (const vvm_negedge&);
};

class vvm_posedge  : public vvm_nexus::recvr_t {

    public:
      explicit vvm_posedge(vvm_sync*tgt);
      ~vvm_posedge();

      void init_P(int idx, vpip_bit_t val);

    private:
      void take_value(unsigned key, vpip_bit_t val);
      vpip_bit_t val_;

      vvm_sync*sync_;

    private: // not implemented
      vvm_posedge(const vvm_posedge&);
      vvm_posedge& operator= (const vvm_posedge&);
};

/*
 * $Log: vvm_gates.h,v $
 * Revision 1.56  2000/04/10 05:26:07  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.55  2000/04/08 05:49:59  steve
 *  Fix memory object compile problems.
 *
 * Revision 1.54  2000/04/01 21:40:23  steve
 *  Add support for integer division.
 *
 * Revision 1.53  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.52  2000/03/26 16:28:31  steve
 *  vvm_bitset_t is no longer a template.
 *
 * Revision 1.51  2000/03/25 02:43:57  steve
 *  Remove all remain vvm_bitset_t return values,
 *  and disallow vvm_bitset_t copying.
 *
 * Revision 1.50  2000/03/24 03:47:01  steve
 *  Update vvm_ram_dq to nexus style.
 *
 * Revision 1.49  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 */
#endif
