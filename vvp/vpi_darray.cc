/*
 * Copyright (c) 2012-2014 Stephen Williams (steve@icarus.com)
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


# include  "compile.h"
# include  "vpi_priv.h"
# include  "vvp_net_sig.h"
# include  "vvp_darray.h"
# include  "array_common.h"
# include  "schedule.h"
#ifdef CHECK_WITH_VALGRIND
# include  "vvp_cleanup.h"
#endif
# include  <cstdio>
# include  <cstdlib>
# include  <cstring>
# include  <cassert>
# include  "ivl_alloc.h"

using namespace std;

__vpiDarrayVar::__vpiDarrayVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: __vpiBaseVar(sc, na, ne)
{
}

unsigned __vpiDarrayVar::get_size() const
{
      vvp_fun_signal_object*fun = dynamic_cast<vvp_fun_signal_object*> (get_net()->fun);
      if(!fun)
        return 0;

      vvp_object_t val = fun->get_object();
      vvp_darray*aval = val.peek<vvp_darray>();

      if(!aval)
        return 0;

      return aval->get_size();
}

vpiHandle __vpiDarrayVar::get_left_range()
{
      left_range_.set_value(0);
      return &left_range_;
}

vpiHandle __vpiDarrayVar::get_right_range()
{
      right_range_.set_value(get_size() - 1);
      return &right_range_;
}

int __vpiDarrayVar::get_word_size() const
{
      return get_vvp_darray()->get_size();
}

char*__vpiDarrayVar::get_word_str(struct __vpiArrayWord*word, int code)
{
      return NULL;
}

void __vpiDarrayVar::get_word_value(struct __vpiArrayWord*word, p_vpi_value vp)
{
      unsigned index = word->get_index();
      vvp_darray*aobj = get_vvp_darray();

      switch(vp->format) {
      case vpiIntVal:
      case vpiVectorVal:
      {
          vvp_vector4_t v;
          aobj->get_word(index, v);
          vpip_vec2_get_value(v, get_word_size(), true, vp);
      }
      break;

      case vpiRealVal:
      {
          double d;
          aobj->get_word(index, d);
          vpip_real_get_value(d, vp);
      }
      break;

      case vpiStringVal:
      {
          string s;
          aobj->get_word(index, s);
          vpip_string_get_value(s, vp);
      }
      break;

      default:
          fprintf(stderr, "vpi sorry: format is not implemented");
          assert(false);
      }
}

void __vpiDarrayVar::put_word_value(struct __vpiArrayWord*word, p_vpi_value vp, int flags)
{
}

vpiHandle __vpiDarrayVar::get_iter_index(struct __vpiArrayIterator*iter, int idx)
{
    return NULL;
}

int __vpiDarrayVar::vpi_get(int code)
{
      switch (code) {
	  case vpiArrayType:
	    return vpiDynamicArray;
	  case vpiSize:
            return get_size();

	  default:
	    return 0;
      }
}

char* __vpiDarrayVar::vpi_get_str(int code)
{
    return NULL;
}

vpiHandle __vpiDarrayVar::vpi_handle(int code)
{
      switch (code) {
          case vpiLeftRange:
            return get_left_range();

          case vpiRightRange:
            return get_right_range();

	  //case vpiModule:
	    //return vpip_module(scope_);
     }

     return 0;
}

vpiHandle __vpiDarrayVar::vpi_index(int index)
{
      if (index >= (long) get_size())
	    return 0;
      if (index < 0)
	    return 0;

      if (vals_words == 0)
	    make_vals_words();

      return &(vals_words[index].as_word);
}

void __vpiDarrayVar::vpi_get_value(p_vpi_value val)
{
      val->format = vpiSuppressVal;
}

vvp_darray*__vpiDarrayVar::get_vvp_darray() const
{
      vvp_fun_signal_object*fun = dynamic_cast<vvp_fun_signal_object*> (get_net()->fun);
      assert(fun);
      vvp_object_t obj = fun->get_object();

      return obj.peek<vvp_darray>();
}

vpiHandle vpip_make_darray_var(const char*name, vvp_net_t*net)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : 0;

      class __vpiDarrayVar*obj = new __vpiDarrayVar(scope, use_name, net);

      return obj;
}

__vpiQueueVar::__vpiQueueVar(__vpiScope*sc, const char*na, vvp_net_t*ne)
: __vpiBaseVar(sc, na, ne)
{
}

int __vpiQueueVar::get_type_code(void) const
{ return vpiArrayVar; }


int __vpiQueueVar::vpi_get(int code)
{
      vvp_fun_signal_object*fun = dynamic_cast<vvp_fun_signal_object*> (get_net()->fun);
      assert(fun);
      vvp_object_t val = fun->get_object();
      vvp_queue*aval = val.peek<vvp_queue>();

      switch (code) {
	  case vpiArrayType:
	    return vpiQueueArray;
	  case vpiSize:
	    if (aval == 0)
		  return 0;
	    else
		  return aval->get_size();

	  default:
	    return 0;
      }
}

void __vpiQueueVar::vpi_get_value(p_vpi_value val)
{
      val->format = vpiSuppressVal;
}


vpiHandle vpip_make_queue_var(const char*name, vvp_net_t*net)
{
      struct __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : 0;

      class __vpiQueueVar*obj = new __vpiQueueVar(scope, use_name, net);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void darray_delete(vpiHandle item)
{
      class __vpiDarrayVar*obj = dynamic_cast<__vpiDarrayVar*>(item);
      delete obj;
}

void queue_delete(vpiHandle item)
{
      class __vpiQueueVar*obj = dynamic_cast<__vpiQueueVar*>(item);
      delete obj;
}
#endif
