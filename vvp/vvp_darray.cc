/*
 * Copyright (c) 2012-2015 Stephen Williams (steve@icarus.com)
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

# include  "vvp_darray.h"
# include  <iostream>
# include  <typeinfo>

using namespace std;

vvp_darray::~vvp_darray()
{
}

void vvp_darray::set_word(unsigned, const vvp_vector4_t&)
{
      cerr << "XXXX set_word(vvp_vector4_t) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::set_word(unsigned, double)
{
      cerr << "XXXX set_word(double) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::set_word(unsigned, const string&)
{
      cerr << "XXXX set_word(string) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::set_word(unsigned, const vvp_object_t&)
{
      cerr << "XXXX set_word(vvp_object_t) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::get_word(unsigned, vvp_vector4_t&)
{
      cerr << "XXXX get_word(vvp_vector4_t) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::get_word(unsigned, double&)
{
      cerr << "XXXX get_word(double) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::get_word(unsigned, string&)
{
      cerr << "XXXX get_word(string) not implemented for " << typeid(*this).name() << endl;
}

void vvp_darray::get_word(unsigned, vvp_object_t&)
{
      cerr << "XXXX get_word(vvp_object_t) not implemented for " << typeid(*this).name() << endl;
}

template <class TYPE> vvp_darray_atom<TYPE>::~vvp_darray_atom()
{
}

template <class TYPE> size_t vvp_darray_atom<TYPE>::get_size() const
{
      return array_.size();
}

template <class TYPE> void vvp_darray_atom<TYPE>::set_word(unsigned adr, const vvp_vector4_t&value)
{
      if (adr >= array_.size())
	    return;
      TYPE tmp;
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
template class vvp_darray_atom<uint64_t>;
template class vvp_darray_atom<int8_t>;
template class vvp_darray_atom<int16_t>;
template class vvp_darray_atom<int32_t>;
template class vvp_darray_atom<int64_t>;

vvp_darray_vec4::~vvp_darray_vec4()
{
}

size_t vvp_darray_vec4::get_size(void) const
{
      return array_.size();
}

void vvp_darray_vec4::set_word(unsigned adr, const vvp_vector4_t&value)
{
      if (adr >= array_.size()) return;
      assert(value.size() == word_wid_);
      array_[adr] = value;
}

void vvp_darray_vec4::get_word(unsigned adr, vvp_vector4_t&value)
{
	/*
	 * Return an undefined value for an out of range address or if the
	 * value has not been written yet (has a size of zero).
	 */
      if ((adr >= array_.size()) || (array_[adr].size() == 0)) {
	    value = vvp_vector4_t(word_wid_, BIT4_X);
	    return;
      }
      value = array_[adr];
      assert(value.size() == word_wid_);
}

vvp_darray_vec2::~vvp_darray_vec2()
{
}

size_t vvp_darray_vec2::get_size(void) const
{
      return array_.size();
}

void vvp_darray_vec2::set_word(unsigned adr, const vvp_vector4_t&value)
{
      if (adr >= array_.size()) return;
      assert(value.size() == word_wid_);
      array_[adr] = value;
}

void vvp_darray_vec2::get_word(unsigned adr, vvp_vector4_t&value)
{
	/*
	 * Return a zero value for an out of range address or if the
	 * value has not been written yet (has a size of zero).
	 */
      if ((adr >= array_.size()) || (array_[adr].size() == 0)) {
	    value = vvp_vector4_t(word_wid_, BIT4_0);
	    return;
      }
      assert(array_[adr].size() == word_wid_);
      value.resize(word_wid_);
      for (unsigned idx = 0; idx < word_wid_; idx += 1) {
	    value.set_bit(idx, array_[adr].value4(idx));
      }
}


vvp_darray_object::~vvp_darray_object()
{
}

size_t vvp_darray_object::get_size() const
{
      return array_.size();
}

void vvp_darray_object::set_word(unsigned adr, const vvp_object_t&value)
{
      if (adr >= array_.size())
	    return;
      array_[adr] = value;
}

void vvp_darray_object::get_word(unsigned adr, vvp_object_t&value)
{
      if (adr >= array_.size()) {
	    value = vvp_object_t();
	    return;
      }

      value = array_[adr];
}

vvp_darray_real::~vvp_darray_real()
{
}

size_t vvp_darray_real::get_size() const
{
      return array_.size();
}

void vvp_darray_real::set_word(unsigned adr, double value)
{
      if (adr >= array_.size())
	    return;
      array_[adr] = value;
}

void vvp_darray_real::get_word(unsigned adr, double&value)
{
      if (adr >= array_.size()) {
	    value = 0.0;
	    return;
      }

      value = array_[adr];
}

vvp_darray_string::~vvp_darray_string()
{
}

size_t vvp_darray_string::get_size() const
{
      return array_.size();
}

void vvp_darray_string::set_word(unsigned adr, const string&value)
{
      if (adr >= array_.size())
	    return;
      array_[adr] = value;
}

void vvp_darray_string::get_word(unsigned adr, string&value)
{
      if (adr >= array_.size()) {
	    value = "";
	    return;
      }

      value = array_[adr];
}

vvp_queue::~vvp_queue()
{
}

void vvp_queue::push_back(const vvp_vector4_t&)
{
      cerr << "XXXX push_back(vvp_vector4_t) not implemented for " << typeid(*this).name() << endl;
}

void vvp_queue::push_front(const vvp_vector4_t&)
{
      cerr << "XXXX push_front(vvp_vector4_t) not implemented for " << typeid(*this).name() << endl;
}

void vvp_queue::push_back(double)
{
      cerr << "XXXX push_back(double) not implemented for " << typeid(*this).name() << endl;
}

void vvp_queue::push_front(double)
{
      cerr << "XXXX push_front(double) not implemented for " << typeid(*this).name() << endl;
}

void vvp_queue::push_back(const string&)
{
      cerr << "XXXX push_back(string) not implemented for " << typeid(*this).name() << endl;
}

void vvp_queue::push_front(const string&)
{
      cerr << "XXXX push_front(string) not implemented for " << typeid(*this).name() << endl;
}

vvp_queue_string::~vvp_queue_string()
{
}

size_t vvp_queue_string::get_size() const
{
      return array_.size();
}

void vvp_queue_string::push_back(const string&val)
{
      array_.push_back(val);
}

void vvp_queue_string::set_word(unsigned adr, const string&value)
{
      if (adr >= array_.size())
	    return;

      list<string>::iterator cur = array_.begin();
      while (adr > 0) {
	    ++ cur;
	    adr -= 1;
      }

      *cur = value;
}

void vvp_queue_string::get_word(unsigned adr, string&value)
{
      if (adr >= array_.size()) {
	    value = "";
	    return;
      }

      list<string>::const_iterator cur = array_.begin();
      while (adr > 0) {
	    ++ cur;
	    adr -= 1;
      }

      value = *cur;
}

void vvp_queue_string::pop_back(void)
{
      array_.pop_back();
}

void vvp_queue_string::pop_front(void)
{
      array_.pop_front();
}

vvp_queue_vec4::~vvp_queue_vec4()
{
}

size_t vvp_queue_vec4::get_size() const
{
      return array_.size();
}

void vvp_queue_vec4::set_word(unsigned adr, const vvp_vector4_t&value)
{
      if (adr >= array_.size())
	    return;

      list<vvp_vector4_t>::iterator cur = array_.begin();
      while (adr > 0) {
	    ++ cur;
	    adr -= 1;
      }

      *cur = value;
}

void vvp_queue_vec4::get_word(unsigned adr, vvp_vector4_t&value)
{
      if (adr >= array_.size()) {
	    value = vvp_vector4_t();
	    return;
      }

      list<vvp_vector4_t>::const_iterator cur = array_.begin();
      while (adr > 0) {
	    ++ cur;
	    adr -= 1;
      }

      value = *cur;
}

void vvp_queue_vec4::push_back(const vvp_vector4_t&val)
{
      array_.push_back(val);
}

void vvp_queue_vec4::push_front(const vvp_vector4_t&val)
{
      array_.push_front(val);
}

void vvp_queue_vec4::pop_back(void)
{
      array_.pop_back();
}

void vvp_queue_vec4::pop_front(void)
{
      array_.pop_front();
}
