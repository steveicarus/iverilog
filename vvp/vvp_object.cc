/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "vvp_object.h"
# include  "vvp_net.h"
# include  <iostream>
# include  <typeinfo>

using namespace std;

vvp_object::~vvp_object()
{
}

vvp_darray::~vvp_darray()
{
}

void vvp_darray::set_word(unsigned, const vvp_vector4_t&)
{
      cerr << "XXXX set_word not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::get_word(unsigned, vvp_vector4_t&)
{
      cerr << "XXXX get_word not implemented for " << typeid(*this).name() << endl;
}

template <class TYPE> vvp_darray_atom<TYPE>::~vvp_darray_atom()
{
}

template <class TYPE> void vvp_darray_atom<TYPE>::set_word(unsigned adr, const vvp_vector4_t&value)
{
      if (adr >= array_.size())
	    return;
      int32_t tmp;
      vector4_to_value(value, tmp, true, false);
      array_[adr] = tmp;
}

template <class TYPE> void vvp_darray_atom<TYPE>::get_word(unsigned adr, vvp_vector4_t&value)
{
      if (adr >= array_.size()) {
	    value = vvp_vector4_t(8*sizeof(TYPE), BIT4_X);
	    return;
      }

      TYPE word = array_[adr];
      vvp_vector4_t tmp (8*sizeof(TYPE), BIT4_0);
      for (unsigned idx = 0 ; idx < tmp.size() ; idx += 1) {
	    if (word&1) tmp.set_bit(idx, BIT4_1);
	    word >>= 1;
      }
      value = tmp;
}

template class vvp_darray_atom<uint8_t>;
template class vvp_darray_atom<uint16_t>;
template class vvp_darray_atom<uint32_t>;
template class vvp_darray_atom<int8_t>;
template class vvp_darray_atom<int16_t>;
template class vvp_darray_atom<int32_t>;
