#ifndef __vvm_vvm_signal_H
#define __vvm_vvm_signal_H
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
#ident "$Id: vvm_signal.h,v 1.3 2000/03/22 04:26:41 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_nexus.h"

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
		  bits[idx] = HiZ;
	    }

      vvm_bitset_t(const vvm_bits_t&that)
	    { unsigned wid = WIDTH;
	      if (that.get_width() < WIDTH)
		    wid = that.get_width();
	      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		    bits[idx] = that.get_bit(idx);
	      for (unsigned idx = wid ;  idx < WIDTH ;  idx += 1)
		    bits[idx] = St0;
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
 * The vvm_signal_t template is the real object that handles the
 * receiving of assignments and doing whatever is done. It also
 * connects VPI to the C++/vvm design. The vvm_bitset_t stores the
 * actual bits, this just attaches the name and vpiSignal stuff to the
 * set. 
 */
class vvm_signal_t  : public __vpiSignal, public vvm_nexus::recvr_t  {

    public:
      vvm_signal_t(vpip_bit_t*b, unsigned nb);
      ~vvm_signal_t();

      void init_P(unsigned idx, vpip_bit_t val);

      void take_value(unsigned key, vpip_bit_t val);
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
 * $Log: vvm_signal.h,v $
 * Revision 1.3  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.2  2000/03/17 20:21:14  steve
 *  Detemplatize the vvm_signal_t class.
 *
 * Revision 1.1  2000/03/16 19:03:04  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 */
#endif
