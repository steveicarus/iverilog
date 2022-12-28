/*
 * Copyright (c) 2012-2022 Stephen Williams (steve@icarus.com)
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
      vvp_vector4_t new_vec;
      vvp_darray*aobj = get_vvp_darray();
      aobj->get_word(0, new_vec);
      return new_vec.size();
}

char*__vpiDarrayVar::get_word_str(struct __vpiArrayWord*word, int code)
{
      unsigned index = word->get_index();

      if (code == vpiFile) {  // Not implemented for now!
	    return simple_set_rbuf_str(file_names[0]);
      }

      char sidx [64];
      snprintf(sidx, 63, "%d", (int)index);
      return generic_get_str(code, scope_, name_, sidx);
}

void __vpiDarrayVar::get_word_value(struct __vpiArrayWord*word, p_vpi_value vp)
{
      unsigned index = word->get_index();
      vvp_darray*aobj = get_vvp_darray();

      if(vp->format == vpiObjTypeVal) {
          if(dynamic_cast<vvp_darray_real*>(aobj))
              vp->format = vpiRealVal;
          else if(dynamic_cast<vvp_darray_string*>(aobj))
              vp->format = vpiStringVal;
          else
              vp->format = vpiVectorVal;
      }

      switch(vp->format) {
      case vpiBinStrVal:
      case vpiOctStrVal:
      case vpiDecStrVal:
      case vpiHexStrVal:
      case vpiScalarVal:
      case vpiIntVal:
      case vpiVectorVal:
      {
          vvp_vector4_t v;
          aobj->get_word(index, v);
          vpip_vec4_get_value(v, v.size(), false, vp);  // TODO sign?
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
          fprintf(stderr, "vpi sorry: format is not implemented\n");
          assert(false);
      }
}

void __vpiDarrayVar::put_word_value(struct __vpiArrayWord*word, p_vpi_value vp, int)
{
      unsigned index = word->get_index();
      vvp_darray*aobj = get_vvp_darray();

      switch(vp->format) {
      case vpiScalarVal:
      {
          vvp_vector4_t vec(1, vp->value.scalar);
          aobj->set_word(index, vec);
      }
      break;

      case vpiIntVal:
      {
          vvp_vector4_t vec;
	  unsigned long val = vp->value.integer;
          vec.setarray(0, 8 * sizeof(vp->value.integer), &val);
          aobj->set_word(index, vec);
      }
      break;

      case vpiVectorVal:        // 2 vs 4 state logic?
      {
          int size = get_word_size();
          PLI_INT32 a = 0, b = 0;
          vvp_vector4_t new_vec(size);
          p_vpi_vecval vec = vp->value.vector;
          vec--; // it will be increased in the first loop iteration

          for(int i = 0; i < size; ++i) {
            int new_bit;
            if(i % (8 * sizeof(vec->aval)) == 0) {
                ++vec;
                a = vec->aval;
                b = vec->bval;
            }

            // convert to vvp_bit4_t
            new_bit = ((b & 1) << 2) | (a & 1);
            new_vec.set_bit(i, (vvp_bit4_t) new_bit);

            a >>= 1;
            b >>= 1;
          }
          aobj->set_word(index, new_vec);
      }
      break;

      case vpiRealVal:
        aobj->set_word(index, vp->value.real);
        break;

      case vpiStringVal:
        aobj->set_word(index, std::string(vp->value.str));
        break;

      default:
          fprintf(stderr, "vpi sorry: format is not implemented");
          assert(false);
      }
}

vpiHandle __vpiDarrayVar::get_iter_index(struct __vpiArrayIterator*, int idx)
{
      if (vals_words == 0) make_vals_words();

      return &(vals_words[idx].as_word);
}

int __vpiDarrayVar::vpi_get(int code)
{
      switch (code) {
	  case vpiArrayType:
	    return vpiDynamicArray;
	  case vpiLeftRange:
	    return 0;
	  case vpiRightRange:
	    return get_size() - 1;
	  case vpiSize:
            return get_size();

	  default:
	    fprintf(stderr, "vpi sorry: property is not implemented");
	    assert(false);
	    return 0;
      }
}

char* __vpiDarrayVar::vpi_get_str(int code)
{
      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      return generic_get_str(code, scope_, name_, NULL);
}

vpiHandle __vpiDarrayVar::vpi_handle(int code)
{
      switch (code) {
          case vpiLeftRange:
            return get_left_range();

          case vpiRightRange:
            return get_right_range();

          case vpiScope:
            return scope_;

          case vpiModule:
            return vpip_module(scope_);
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
      __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : NULL;

      __vpiDarrayVar*obj = new __vpiDarrayVar(scope, use_name, net);

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
      __vpiScope*scope = vpip_peek_current_scope();
      const char*use_name = name ? vpip_name_string(name) : NULL;

      __vpiQueueVar*obj = new __vpiQueueVar(scope, use_name, net);

      return obj;
}

#ifdef CHECK_WITH_VALGRIND
void darray_delete(vpiHandle item)
{
      __vpiDarrayVar*obj = dynamic_cast<__vpiDarrayVar*>(item);
      if (obj->vals_words) delete [] (obj->vals_words-1);
      delete obj;
}

void queue_delete(vpiHandle item)
{
      __vpiQueueVar*obj = dynamic_cast<__vpiQueueVar*>(item);
      delete obj;
}
#endif
