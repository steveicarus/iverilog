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
#ident "$Id: vvm_signal.cc,v 1.6 2000/12/12 03:30:25 steve Exp $"
#endif

# include  "vvm_signal.h"
# include  <iostream.h>

vvm_bitset_t::vvm_bitset_t(vpip_bit_t*b, unsigned nb)
: bits(b), nbits(nb)
{
}

vvm_bitset_t::~vvm_bitset_t()
{
}

vpip_bit_t&vvm_bitset_t::operator[] (unsigned idx)
{
      assert(idx < nbits);
      return bits[idx];
}

vpip_bit_t vvm_bitset_t::operator[] (unsigned idx) const
{
      assert(idx < nbits);
      return bits[idx];
}

vpip_bit_t vvm_bitset_t::get_bit(unsigned idx) const
{
      assert(idx < nbits);
      return bits[idx];
}

unsigned vvm_bitset_t::as_unsigned() const
{
      unsigned result = 0;
      unsigned width = get_width();
      for (unsigned idx = width ;  idx > 0 ;  idx -= 1) {
	    result <<= 1;

	    if (B_IS1(get_bit(idx-1)))
		  result |= 1;
      }
      return result;
}

ostream& operator << (ostream&os, const vvm_bitset_t&str)
{
      os << str.get_width() << "b'";
      for (unsigned idx = str.get_width() ;  idx > 0 ;  idx -= 1)
	    b_output(os, str.get_bit(idx));

      return os;
}

vvm_signal_t::vvm_signal_t()
{
      bits = 0;
      nbits = 0;
}

vvm_signal_t::~vvm_signal_t()
{
}

void vvm_signal_t::init_P(unsigned idx, vpip_bit_t val)
{
      assert(idx < nbits);
      bits[idx] = val;
}

void vvm_signal_t::take_value(unsigned key, vpip_bit_t val)
{
      assert(key < nbits);
      bits[key] = val;
      vpip_run_value_changes(this);
}

vvm_ram_callback::vvm_ram_callback()
{
}

vvm_ram_callback::~vvm_ram_callback()
{
}

/*
 * $Log: vvm_signal.cc,v $
 * Revision 1.6  2000/12/12 03:30:25  steve
 *  out-line vvm_bitset_t methods.
 *
 * Revision 1.5  2000/04/26 18:35:12  steve
 *  Handle assigning small values to big registers.
 *
 * Revision 1.4  2000/03/26 16:55:41  steve
 *  Remove the vvm_bits_t abstract class.
 *
 * Revision 1.3  2000/03/25 05:02:25  steve
 *  signal bits are referenced at run time by the vpiSignal struct.
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

