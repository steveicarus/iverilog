#ifndef __vvm_H
#define __vvm_H
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
#ident "$Id: vvm.h,v 1.32 2000/03/13 00:02:34 steve Exp $"
#endif

# include  <cassert>
# include  "vpi_priv.h"

/*
 * The Verilog Virtual Machine are definitions for the virtual machine
 * that executes models that the simulation generator makes.
 */

typedef unsigned vvm_u32;

class vvm_event;
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

inline vpip_bit_t v_not(vpip_bit_t l)
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

    public:
      vvm_event();
      virtual ~vvm_event() =0;
      virtual void event_function() =0;

      void schedule(unsigned long delay =0);

    private:
      struct vpip_event*event_;

      static void callback_(void*);

    private: // not implemented
      vvm_event(const vvm_event&);
      vvm_event& operator= (const vvm_event&);
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

      void set_P(unsigned idx, vpip_bit_t val)
	    { bits[idx] = val;
	      vpip_run_value_changes(this);
	    }

      void set_P(const vvm_bitset_t<WIDTH>&val)
	    { for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		  set(sim, idx, val[idx]);
	    }
};

struct vvm_ram_callback {
      vvm_ram_callback();
      virtual ~vvm_ram_callback();
      virtual void handle_write(unsigned idx) =0;
      vvm_ram_callback*next_;
};

template <unsigned WIDTH, unsigned SIZE>
class vvm_memory_t : public __vpiMemory {

    public:
      vvm_memory_t()
	    { cb_list_ = 0;
	    }

      void set_word(unsigned addr,
		    const vvm_bitset_t<WIDTH>&val)
	    { unsigned base = WIDTH * addr;
	      assert(addr < size);
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    bits[base+idx] = val[idx];
	      call_list_(addr);
	    }

      void set_word(unsigned addr,
		    const vpip_bit_t val[WIDTH])
	    { unsigned base = WIDTH * addr;
	      assert(addr < size);
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    bits[base+idx] = val[idx];
	      call_list_(addr);
	    }

      vvm_bitset_t<WIDTH> get_word(unsigned addr) const
	    { vvm_bitset_t<WIDTH> val;
	      unsigned base = WIDTH * addr;
	      assert(addr < size);
	      for (unsigned idx = 0 ;  idx < WIDTH ;  idx += 1)
		    val[idx] = bits[base+idx];
	      return val;
	    }

      void set_callback(vvm_ram_callback*ram)
	    { ram->next_ = cb_list_;
	      cb_list_ = ram;
	    }

      class assign_nb  : public vvm_event {
	  public:
	    assign_nb(vvm_memory_t<WIDTH,SIZE>&m, unsigned i,
		      const vvm_bitset_t<WIDTH>&v)
	    : mem_(m), index_(i), val_(v) { }

	    void event_function() { mem_.set_word(index_, val_); }

	  private:
	    vvm_memory_t<WIDTH,SIZE>&mem_;
	    unsigned index_;
	    vvm_bitset_t<WIDTH> val_;
      };

    private:
      vvm_ram_callback*cb_list_;
      void call_list_(unsigned idx)
	    { for (vvm_ram_callback*cur = cb_list_; cur; cur = cur->next_)
		    cur->handle_write(idx);
	    }
};


/*
 * $Log: vvm.h,v $
 * Revision 1.32  2000/03/13 00:02:34  steve
 *  Remove unneeded templates.
 *
 * Revision 1.31  2000/02/23 04:43:43  steve
 *  Some compilers do not accept the not symbol.
 *
 * Revision 1.30  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.29  2000/01/08 03:09:14  steve
 *  Non-blocking memory writes.
 */
#endif
