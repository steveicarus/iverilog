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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvm_memory.cc,v 1.4 2002/08/12 01:35:07 steve Exp $"
#endif

# include "config.h"

# include  "vvm_signal.h"

vvm_memory_t::vvm_memory_t()
{
      cb_list_ = 0;
}

void vvm_memory_t::set_word(unsigned addr, const vvm_bitset_t&val)
{
      if (addr >= size)
	    return;

      unsigned base = width * addr;
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    bits[base+idx] = val[idx];

      call_list_(addr);
}

void vvm_memory_t::set_word(unsigned addr, const vpip_bit_t*val)
{
      if (addr >= size)
	    return;

      unsigned base = width * addr;
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    bits[base+idx] = val[idx];

      call_list_(addr);
}

void vvm_memory_t::get_word(unsigned addr, vvm_bitset_t&val) const
{
      if (addr >= size) {
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1)
		  val[idx] = StX;
	    return;
      }

      unsigned base = width * addr;
      for (unsigned idx = 0 ;  idx < width ;  idx += 1)
	    val[idx] = bits[base+idx];
}

void vvm_memory_t::set_callback(vvm_ram_callback*ram)
{
      ram->next_ = cb_list_;
      cb_list_ = ram;
}

void vvm_memory_t::call_list_(unsigned idx)
{
      for (vvm_ram_callback*cur = cb_list_; cur; cur = cur->next_)
	    cur->handle_write(idx);
}

vvm_memory_t::assign_nb::assign_nb(vvm_memory_t&m, unsigned i,
				   const vvm_bitset_t&v)
: mem_(m), index_(i), bits_(new vpip_bit_t[m.width]), val_(bits_, m.width)
{
      unsigned top = m.width;
      if (top > v.nbits)
	    top = v.nbits;

      for (unsigned idx = 0 ;  idx < top ;  idx += 1)
	    val_[idx] = v[idx];
      for (unsigned idx = top ;  idx < m.width ;  idx += 1)
	    val_[idx] = St0;
}


vvm_memory_t::assign_nb::~assign_nb()
{
      delete[]bits_;
}

void vvm_memory_t::assign_nb::event_function()
{
      mem_.set_word(index_, val_);
}

/*
 * $Log: vvm_memory.cc,v $
 * Revision 1.4  2002/08/12 01:35:07  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2000/12/15 21:54:43  steve
 *  Allow non-blocking assign to pad memory word with zeros.
 *
 * Revision 1.1  2000/12/15 20:05:16  steve
 *  Fix memory access in vvm. (PR#70)
 *
 */

