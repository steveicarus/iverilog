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
#ident "$Id: vvm_signal.h,v 1.11 2000/12/15 20:05:16 steve Exp $"
#endif

# include  "vvm.h"
# include  "vvm_nexus.h"
class ostream;

/*
 * The vvm_bitset_t is a reference to an array of vpip_bit_t
 * values. The space for the value is actually managed elsewhere, this
 * object just references it, and attaches operations to it.
 *
 * The vvm_bitset_t is useful in behavioral situations, to operate on
 * vpip_bit_t data vectors.
 */
class vvm_bitset_t  {

    public:
      explicit vvm_bitset_t(vpip_bit_t*b, unsigned nb);
      ~vvm_bitset_t();

      vpip_bit_t operator[] (unsigned idx) const;
      vpip_bit_t&operator[] (unsigned idx);

      unsigned get_width() const { return nbits; }
      vpip_bit_t get_bit(unsigned idx) const;

      unsigned as_unsigned() const;

    public:
      vpip_bit_t*bits;
      unsigned nbits;

    private: // not implemented
      vvm_bitset_t(const vvm_bitset_t&);
      vvm_bitset_t& operator= (const vvm_bitset_t&);
};

extern ostream& operator << (ostream&os, const vvm_bitset_t&str);

/*
 * The vvm_signal_t template is the real object that handles the
 * receiving of assignments and doing whatever is done. It also
 * connects VPI to the C++/vvm design. The actual bits are referenced
 * by the base vpiSignal structure.
 */
class vvm_signal_t  : public __vpiSignal, public vvm_nexus::recvr_t  {

    public:
      vvm_signal_t();
      ~vvm_signal_t();

      void init_P(unsigned idx, vpip_bit_t val);
      void take_value(unsigned key, vpip_bit_t val);

    private: // not implemented
      vvm_signal_t(const vvm_signal_t&);
      vvm_signal_t& operator= (const vvm_signal_t&);
};

struct vvm_ram_callback {
      vvm_ram_callback();
      virtual ~vvm_ram_callback();
      virtual void handle_write(unsigned idx) =0;
      vvm_ram_callback*next_;
};

class vvm_memory_t : public __vpiMemory {

    public:
      vvm_memory_t();

      void set_word(unsigned addr, const vvm_bitset_t&val);

      void set_word(unsigned addr, const vpip_bit_t*val);

      void get_word(unsigned addr, vvm_bitset_t&val) const;

      void set_callback(vvm_ram_callback*ram);

      class assign_nb  : public vvm_event {
	  public:
	    assign_nb(vvm_memory_t&m, unsigned i, const vvm_bitset_t&v);
	    ~assign_nb();

	    void event_function();

	  private:
	    vvm_memory_t&mem_;
	    unsigned index_;
	    vpip_bit_t*bits_;
	    vvm_bitset_t val_;
      };

    private:
      vvm_ram_callback*cb_list_;
      void call_list_(unsigned idx);
};


/*
 * $Log: vvm_signal.h,v $
 * Revision 1.11  2000/12/15 20:05:16  steve
 *  Fix memory access in vvm. (PR#70)
 *
 * Revision 1.10  2000/12/12 03:30:25  steve
 *  out-line vvm_bitset_t methods.
 *
 * Revision 1.9  2000/04/26 18:35:12  steve
 *  Handle assigning small values to big registers.
 *
 * Revision 1.8  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.7  2000/03/26 16:28:31  steve
 *  vvm_bitset_t is no longer a template.
 *
 * Revision 1.6  2000/03/25 05:02:25  steve
 *  signal bits are referenced at run time by the vpiSignal struct.
 *
 * Revision 1.5  2000/03/25 02:43:57  steve
 *  Remove all remain vvm_bitset_t return values,
 *  and disallow vvm_bitset_t copying.
 *
 * Revision 1.4  2000/03/24 02:43:37  steve
 *  vvm_unop and vvm_binop pass result by reference
 *  instead of returning a value.
 *
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
