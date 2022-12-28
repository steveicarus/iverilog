/*
 * Copyright (c) 2014-2022 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2014 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
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

#include "array_common.h"

vpiHandle __vpiArrayBase::vpi_array_base_iterate(int code)
{
    switch (code) {
        case vpiReg:
        case vpiMemoryWord: {
                struct __vpiArrayIterator*res;
                res = new __vpiArrayIterator;
                res->array = this;
                res->next = 0;
                return res;
        }
    }

    return 0;
}

void __vpiArrayBase::make_vals_words()
{
    assert(vals_words == 0);
    vals_words = new struct __vpiArrayWord[get_size() + 1];

    // Make word[-1] point to the parent.
    vals_words->parent = this;
    // Now point to word-0
    vals_words += 1;

    struct __vpiArrayWord*words = vals_words;
    for (unsigned idx = 0 ; idx < get_size() ; idx += 1) {
            words[idx].word0 = words;
    }
}

vpiHandle __vpiArrayIterator::vpi_index(int)
{
      if (next >= array->get_size()) {
	    vpi_free_object(this);
	    return 0;
      }

      unsigned use_index = next;
      next += 1;

      return array->get_iter_index(this, use_index);
}

static int array_iterator_free_object(vpiHandle ref)
{
      struct __vpiArrayIterator*obj = dynamic_cast<__vpiArrayIterator*>(ref);
      delete obj;
      return 1;
}

__vpiHandle::free_object_fun_t __vpiArrayIterator::free_object_fun(void)
{ return &array_iterator_free_object; }

vpiHandle __vpiArrayIndex::vpi_iterate(int code)
{
    // TODO originally, __vpiArrayIndex is casted to __vpiDecConst and assigned
    // to res->index - seems like a bug to me

      if (code == vpiIndex) {
	    struct __vpiArrayIndex*res;
	    res = new __vpiArrayIndex;
	    res->index = index;         // see the comment above
	    res->done = 0;
	    return res;
      }
      return 0;
}

vpiHandle __vpiArrayIndex::vpi_index(int)
{
      if (done == 0) {
            done = 1;
            return index;
      }

      vpi_free_object(this);
      return 0;
}

static int array_index_free_object(vpiHandle ref)
{
      struct __vpiArrayIndex*obj = dynamic_cast<__vpiArrayIndex*>(ref);
      delete obj;
      return 1;
}

__vpiHandle::free_object_fun_t __vpiArrayIndex::free_object_fun(void)
{ return &array_index_free_object; }

char* __vpiArrayWord::as_word_t::vpi_get_str(int code)
{
      struct __vpiArrayWord*obj = array_var_word_from_handle(this);
      assert(obj);
      struct __vpiArrayBase*my_parent = obj->get_parent();
      return my_parent->get_word_str(obj, code);
}

void __vpiArrayWord::as_word_t::vpi_get_value(p_vpi_value vp)
{
      struct __vpiArrayWord*obj = array_var_word_from_handle(this);
      assert(obj);
      struct __vpiArrayBase*my_parent = obj->get_parent();
      return my_parent->get_word_value(obj, vp);
}

vpiHandle __vpiArrayWord::as_word_t::vpi_put_value(p_vpi_value vp, int flags)
{
      struct __vpiArrayWord*obj = array_var_word_from_handle(this);
      assert(obj);
      struct __vpiArrayBase*my_parent = obj->get_parent();
      my_parent->put_word_value(obj, vp, flags);
      return this;
}

vpiHandle __vpiArrayWord::as_word_t::vpi_handle(int code)
{
      struct __vpiArrayWord*obj = array_var_word_from_handle(this);
      assert(obj);
      struct __vpiArrayBase*my_parent = obj->get_parent();

      switch (code) {

	  case vpiIndex:
	    return &(obj->as_index);

	  case vpiLeftRange:
	    return my_parent->get_left_range();

	  case vpiRightRange:
	    return my_parent->get_right_range();

	  case vpiParent:
	    return dynamic_cast<vpiHandle>(my_parent);

	  case vpiScope:
	    return my_parent->get_scope();

	  case vpiModule:
	    return vpip_module(my_parent->get_scope());
      }

      return 0;
}

void __vpiArrayWord::as_index_t::vpi_get_value(p_vpi_value vp)
{
      struct __vpiArrayWord*obj = array_var_index_from_handle(this);
      assert(obj);
      unsigned index = obj->get_index();

      assert(vp->format == vpiIntVal);
      vp->value.integer = index;
}

struct __vpiArrayWord*array_var_word_from_handle(vpiHandle ref)
{
      if (ref == 0)
	    return 0;
      __vpiArrayWord::as_word_t*ptr = dynamic_cast<__vpiArrayWord::as_word_t*> (ref);
      if (ptr == 0)
	    return 0;

      return (struct __vpiArrayWord*) ref;
}

struct __vpiArrayWord* array_var_index_from_handle(vpiHandle ref)
{
      if (ref == 0)
	    return 0;
      __vpiArrayWord::as_index_t*ptr = dynamic_cast<__vpiArrayWord::as_index_t*> (ref);
      if (ptr == 0)
	    return 0;

      assert(sizeof(__vpiHandle) == sizeof(__vpiArrayWord::as_index_t));
      assert(sizeof(__vpiHandle) == sizeof(__vpiArrayWord::as_word_t));
      return (struct __vpiArrayWord*) (ref-1);
}

