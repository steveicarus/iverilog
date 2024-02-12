/*
 * Copyright (c) 2007-2024 Stephen Williams (steve@icarus.com)
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

# include  "array_common.h"
# include  "array.h"
# include  "symbols.h"
# include  "schedule.h"
# include  "vpi_priv.h"
# include  "vvp_net_sig.h"
# include  "vvp_darray.h"
# include  "config.h"
#ifdef CHECK_WITH_VALGRIND
#include  "vvp_cleanup.h"
#endif
# include  <cstdlib>
# include  <cstring>
# include  <climits>
# include  <iostream>
# include  "compile.h"
# include  <cassert>
# include  "ivl_alloc.h"

using namespace std;

unsigned long count_net_arrays = 0;
unsigned long count_net_array_words = 0;
unsigned long count_var_arrays = 0;
unsigned long count_var_array_words = 0;
unsigned long count_real_arrays = 0;
unsigned long count_real_array_words = 0;

static symbol_map_s<struct __vpiArray>* array_table =0;

class vvp_fun_arrayport;
static void array_attach_port(vvp_array_t, vvp_fun_arrayport*);

vvp_array_t array_find(const char*label)
{
      if (array_table == 0)
	    return 0;

      vvp_array_t v = array_table->sym_get_value(label);
      return v;
}

struct __vpiArrayVthrA : public __vpiHandle {
      int get_type_code(void) const { return vpiMemoryWord; }
      int vpi_get(int code);
      char* vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_put_value(p_vpi_value val, int flags);
      vpiHandle vpi_handle(int code);

      struct __vpiArray*array;
	// If this is set, then use it to get the index value.
      vpiHandle address_handle;
	// If wid==0, then address is the address into the array.
      unsigned address;

      unsigned get_address() const
      {
	    if (address_handle) {
		  s_vpi_value vp;
		    /* Check to see if the value is defined. */
		  vp.format = vpiVectorVal;
		  address_handle->vpi_get_value(&vp);
		  int words = (address_handle->vpi_get(vpiSize)-1)/32 + 1;
		  for(int idx = 0; idx < words; idx += 1) {
			  /* Return UINT_MAX to indicate an X base. */
			if (vp.value.vector[idx].bval != 0) return UINT_MAX;
		  }
		    /* The value is defined so get and return it. */
		  vp.format = vpiIntVal;
		  address_handle->vpi_get_value(&vp);
		  return vp.value.integer;
	    }

	    return address;
      }
};


struct __vpiArrayVthrAPV : public __vpiHandle {
      int get_type_code(void) const { return vpiPartSelect; }
      int vpi_get(int code);
      char* vpi_get_str(int code);
      void vpi_get_value(p_vpi_value val);
      vpiHandle vpi_handle(int code);

      struct __vpiArray*array;
      unsigned word_sel;
      unsigned part_bit;
      unsigned part_wid;
};

bool is_net_array(vpiHandle obj)
{
      struct __vpiArray*rfp = dynamic_cast<__vpiArray*> (obj);
      assert(rfp);

      if (rfp->nets != 0) return true;
      return false;
}

// This function return true if the underlying array words are real.
static bool vpi_array_is_real(const vvp_array_t arr)
{
	// Check to see if this is a variable/register array.
      if (arr->vals4 != 0)  // A bit based variable/register array.
	    return false;

      if (dynamic_cast<vvp_darray_real*> (arr->vals))
	    return true;

      if (arr->vals != 0)
	    return false;

	// This must be a net array so look at element 0 to find the type.
      assert(arr->nets != 0);
      assert(arr->get_size() > 0);
      struct __vpiRealVar*rsig = dynamic_cast<__vpiRealVar*>(arr->nets[0]);
      if (rsig) {
	    return true;
      }

      return false;
}

static bool vpi_array_is_string(const vvp_array_t arr)
{
	// Check to see if this is a variable/register array.
      if (arr->vals4 != 0)  // A bit based variable/register array.
	    return false;

      if (dynamic_cast<vvp_darray_string*> (arr->vals))
	    return true;

      return false;
}

int __vpiArray::get_word_size() const
{
      unsigned width;

      assert(get_size() > 0);
	/* For a net array we need to get the width from the first element. */
      if (nets) {
	    assert(vals4 == 0 && vals == 0);
	    struct __vpiSignal*vsig = dynamic_cast<__vpiSignal*>(nets[0]);
	    assert(vsig);
	    width = vpip_size(vsig);
	/* For a variable array we can get the width from vals_width. */
      } else {
	    assert(vals4 || vals);
	    width = vals_width;
      }

      return width;
}

char*__vpiArray::get_word_str(struct __vpiArrayWord*word, int code)
{
      unsigned index = word->get_index();

      if (code == vpiFile) {  // Not implemented for now!
	    return simple_set_rbuf_str(file_names[0]);
      }

      char sidx [64];
      snprintf(sidx, 63, "%d", (int)index + first_addr.get_value());
      return generic_get_str(code, get_scope(), name, sidx);
}

void __vpiArray::get_word_value(struct __vpiArrayWord*word, p_vpi_value vp)
{
      unsigned index = word->get_index();

    // Determine the appropriate format (The Verilog PLI Handbook 5.2.10)
      if(vp->format == vpiObjTypeVal) {
          if(vpi_array_is_real(this))
              vp->format = vpiRealVal;
          else if(vpi_array_is_string(this))
              vp->format = vpiStringVal;
          else
              vp->format = vpiIntVal;
      }

      if(vals4) {
          vpip_vec4_get_value(vals4->get_word(index),
                              vals4->width(), signed_flag, vp);
      } else if(vals) {
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
                vals->get_word(index, v);
                vpip_vec4_get_value(v, vals_width, signed_flag, vp);
            }
            break;

            case vpiRealVal:
            {
                double d;
                vals->get_word(index, d);
                vpip_real_get_value(d, vp);
            }
            break;

            case vpiStringVal:
            {
                string s;
                vals->get_word(index, s);
                vpip_string_get_value(s, vp);
            }
            break;

            default:
                fprintf(stderr, "vpi sorry: format is not implemented\n");
                assert(false);
          }
      }
}

void __vpiArray::put_word_value(struct __vpiArrayWord*word, p_vpi_value vp, int)
{
      unsigned index = word->get_index();
      vvp_vector4_t val = vec4_from_vpi_value(vp, vals_width);
      set_word(index, 0, val);
}

vpiHandle __vpiArray::get_iter_index(struct __vpiArrayIterator*, int idx)
{
      if (nets) return nets[idx];

      assert(vals4 || vals);

      if (vals_words == 0) make_vals_words();

      return &(vals_words[idx].as_word);
}

int __vpiArray::vpi_get(int code)
{
      switch (code) {
	  case vpiLineNo:
	    return 0; // Not implemented for now!

	  case vpiSize:
	    return get_size();

	  case vpiAutomatic:
	    return scope->is_automatic()? 1 : 0;

	  default:
	    return 0;
      }
}

char* __vpiArray::vpi_get_str(int code)
{
      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      return generic_get_str(code, scope, name, NULL);
}

vpiHandle __vpiArray::vpi_handle(int code)
{
      switch (code) {
          case vpiLeftRange:
            if (swap_addr) return &last_addr;
            else return &first_addr;

          case vpiRightRange:
            if (swap_addr) return &first_addr;
            else return &last_addr;

	  case vpiScope:
	    return scope;

	  case vpiModule:
	    return vpip_module(scope);
      }

      return 0;
}

/*
* VPI code passes indices that are not yet converted to canonical
* form, so this index function does it here.
*/
vpiHandle __vpiArray::vpi_index(int index)
{
      index -= first_addr.get_value();
      if (index >= (long) get_size())
	    return 0;
      if (index < 0)
	    return 0;

      if (nets != 0) {
	    return nets[index];
      }

      if (vals_words == 0)
	    make_vals_words();

      return &(vals_words[index].as_word);
}

int __vpiArrayWord::as_word_t::vpi_get(int code)
{
      struct __vpiArrayWord*obj = array_var_word_from_handle(this);
      assert(obj);
      struct __vpiArrayBase*my_parent = obj->get_parent();
      t_vpi_value val;

      switch (code) {
	  case vpiLineNo:
	    return 0; // Not implemented for now!

	  case vpiSize:
	    return my_parent->get_word_size();

	  case vpiLeftRange:
            val.format = vpiIntVal;
	    my_parent->get_left_range()->vpi_get_value(&val);
            assert(val.format == vpiIntVal);
            return val.value.integer;

	  case vpiRightRange:
            val.format = vpiIntVal;
	    my_parent->get_right_range()->vpi_get_value(&val);
            assert(val.format == vpiIntVal);
            return val.value.integer;

	  case vpiIndex:
	    {
		  int base_offset = 0;
		  struct __vpiArray*base = dynamic_cast<__vpiArray*> (my_parent);
		  if (base) {
			val.format = vpiIntVal;
			base->first_addr.vpi_get_value(&val);
			base_offset += val.value.integer;
		  }
		  val.format = vpiIntVal;
		  obj->as_index.vpi_get_value(&val);
		  assert(val.format == vpiIntVal);
		  return val.value.integer + base_offset;
	    }

	  case vpiAutomatic:
	    return my_parent->get_scope()->is_automatic()? 1 : 0;

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
	  case _vpiFromThr:
	    return _vpiNoThr;
#endif

	  default:
	    return 0;
      }
}


int __vpiArrayVthrA::vpi_get(int code)
{
      switch (code) {
	  case vpiLineNo:
	    return 0; // Not implemented for now!

	  case vpiSize:
	    return array->get_word_size();

	  case vpiLeftRange:
	    return array->msb.get_value();

	  case vpiRightRange:
	    return array->lsb.get_value();

	  case vpiIndex:
	    return (int)get_address() + array->first_addr.get_value();

	  case vpiAutomatic:
	    return array->get_scope()->is_automatic() ? 1 : 0;

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
	  case _vpiFromThr:
	    return _vpi_at_A;
#endif

	  // If address_handle is not zero we definitely have a
	  // variable.
	  case vpiConstantSelect:
	    return address_handle == 0;

	  default:
	    return 0;
      }
}

char* __vpiArrayVthrA::vpi_get_str(int code)
{
      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      char sidx [64];
      snprintf(sidx, 63, "%d", (int)get_address() + array->first_addr.get_value());
      return generic_get_str(code, array->get_scope(), array->name, sidx);
}

void __vpiArrayVthrA::vpi_get_value(p_vpi_value vp)
{
      assert(array);

      unsigned index = get_address();
      if (vpi_array_is_real(array)) {
	    double tmp = array->get_word_r(index);
	    vpip_real_get_value(tmp, vp);
      } else if (vpi_array_is_string(array)) {
	    string tmp = array->get_word_str(index);
	    vpip_string_get_value(tmp, vp);
      } else {
	    vvp_vector4_t tmp = array->get_word(index);
	    unsigned width = array->get_word_size();
	    vpip_vec4_get_value(tmp, width, array->signed_flag, vp);
      }
}

vpiHandle __vpiArrayVthrA::vpi_put_value(p_vpi_value vp, int)
{
      unsigned index = get_address();

      assert(array);
      assert(index < array->get_size());

      if (vpi_array_is_real(array)) {
	    double val = real_from_vpi_value(vp);
	    array->set_word(index, val);
      } else {
	    unsigned width = array->get_word_size();
	    vvp_vector4_t val = vec4_from_vpi_value(vp, width);
	    array->set_word(index, 0, val);
      }

      return this;
}

vpiHandle __vpiArrayVthrA::vpi_handle(int code)
{
      switch (code) {
	  case vpiIndex:
	    break;  // Not implemented!

	  case vpiLeftRange:
	    return &array->msb;

	  case vpiRightRange:
	    return &array->lsb;

	  case vpiParent:
	  case vpiArray:
	    return array;

	  case vpiScope:
	    return array->get_scope();

	  case vpiModule:
	    return vpip_module(array->get_scope());
      }

      return 0;
}


int __vpiArrayVthrAPV::vpi_get(int code)
{
      switch (code) {
	  case vpiLineNo:
	    return 0; // Not implemented for now!

	  case vpiSize:
	    return part_wid;

	  case vpiLeftRange:
	    return part_bit + part_wid - 1;

	  case vpiRightRange:
	    return part_bit;

	  case vpiAutomatic:
	    return array->get_scope()->is_automatic() ? 1 : 0;

#if defined(CHECK_WITH_VALGRIND) || defined(BR916_STOPGAP_FIX)
	  case _vpiFromThr:
	    return _vpi_at_APV;
#endif

	  case vpiConstantSelect:
	    return 1;

	  default:
	    return 0;
      }
}

char* __vpiArrayVthrAPV::vpi_get_str(int code)
{
      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      char sidx [64];
      snprintf(sidx, 63, "%u", word_sel + array->first_addr.get_value());
      return generic_get_str(code, array->get_scope(), array->name, sidx);
}

void __vpiArrayVthrAPV::vpi_get_value(p_vpi_value vp)
{
      assert(array);

      unsigned index = word_sel;
      if (vpi_array_is_real(array)) {
	    double tmp = array->get_word_r(index);
	    vpip_real_get_value(tmp, vp);
      } else {
	    vvp_vector4_t tmp = array->get_word(index);
	    tmp = tmp.subvalue(part_bit, part_wid);
	    vpip_vec4_get_value(tmp, part_wid, array->signed_flag, vp);
      }
}

vpiHandle __vpiArrayVthrAPV::vpi_handle(int code)
{
      switch (code) {
            // Not currently implemented. We would need to create a VPI
            // object for the memory word.
	  case vpiParent:
	    return 0;

            // Not part of the Verilog standard. We use this internally.
	  case vpiArray:
	    return array;

	  case vpiScope:
	    return array->get_scope();

	  case vpiModule:
	    return vpip_module(array->get_scope());
      }

      return 0;
}

void __vpiArray::set_word(unsigned address, unsigned part_off, const vvp_vector4_t&val)
{
      if (address >= get_size())
	    return;

      if (vals4) {
	    assert(nets == 0);
	    if (part_off != 0 || val.size() != vals_width) {
		  vvp_vector4_t tmp = vals4->get_word(address);
		  if ((part_off + val.size()) > tmp.size()) {
			cerr << "part_off=" << part_off
			     << " val.size()=" << val.size()
			     << " vals[address].size()=" << tmp.size()
			     << " vals_width=" << vals_width << endl;
			assert(0);
		  }
		  tmp.set_vec(part_off, val);
		  vals4->set_word(address, tmp);
	    } else {
		  vals4->set_word(address, val);
	    }
	    word_change(address);
	    return;
      }

      if (vals) {
	    assert(nets == 0);
	    if (part_off != 0 || val.size() != vals_width) {
		  vvp_vector4_t tmp;
		  vals->get_word(address, tmp);
		  if ((part_off + val.size()) > tmp.size()) {
			cerr << "part_off=" << part_off
			     << " val.size()=" << val.size()
			     << " vals[address].size()=" << tmp.size()
			     << " vals_width=" << vals_width << endl;
			assert(0);
		  }
		  tmp.set_vec(part_off, val);
		  vals->set_word(address, tmp);
	    } else {
		  vals->set_word(address, val);
	    }
	    word_change(address);
	    return;
      }

      assert(nets != 0);

	// Select the word of the array that we affect.
      vpiHandle word = nets[address];
      struct __vpiSignal*vsig = dynamic_cast<__vpiSignal*>(word);
      assert(vsig);

      vsig->node->send_vec4_pv(val, part_off, vpip_size(vsig), 0);
      word_change(address);
}

void __vpiArray::set_word(unsigned address, double val)
{
      assert(vals != 0);
      assert(nets == 0);

      if (address >= vals->get_size())
	    return;

      vals->set_word(address, val);
      word_change(address);
}

void __vpiArray::set_word(unsigned address, const string&val)
{
      assert(vals != 0);
      assert(nets == 0);

      if (address >= vals->get_size())
	    return;

      vals->set_word(address, val);
      word_change(address);
}

void __vpiArray::set_word(unsigned address, const vvp_object_t&val)
{
      assert(vals != 0);
      assert(nets == 0);

      if (address >= vals->get_size())
	    return;

      vals->set_word(address, val);
      word_change(address);
}

vvp_vector4_t __vpiArray::get_word(unsigned address)
{
      if (vals4) {
	    assert(nets == 0);
	    assert(vals == 0);
	    return vals4->get_word(address);
      }

      if (vals) {
	    assert(nets == 0);
	    assert(vals4== 0);
	    if (address >= vals->get_size())
		  return vvp_vector4_t(vals_width, BIT4_X);

	    vvp_vector4_t val;
	    vals->get_word(address, val);
	    return val;
      }

      assert(vals4 == 0);
      assert(vals == 0);
      assert(nets != 0);

      if (address >= get_size()) {
	      // Reading outside the array. Return X's but get the
	      // width by looking at a word that we know is present.
	    assert(get_size() > 0);
	    vpiHandle word = nets[0];
	    assert(word);
	    struct __vpiSignal*vsig = dynamic_cast<__vpiSignal*>(word);
	    assert(vsig);
	    vvp_signal_value*sig = dynamic_cast<vvp_signal_value*> (vsig->node->fil);
	    assert(sig);
	    return vvp_vector4_t(sig->value_size(), BIT4_X);
      }

      vpiHandle word = nets[address];
      struct __vpiSignal*vsig = dynamic_cast<__vpiSignal*>(word);
      assert(vsig);
      vvp_signal_value*sig = dynamic_cast<vvp_signal_value*> (vsig->node->fil);
      assert(sig);

      vvp_vector4_t val;
      sig->vec4_value(val);
      return val;
}

double __vpiArray::get_word_r(unsigned address)
{
      if (vals) {
	    assert(vals4 == 0);
	    assert(nets  == 0);
	      // In this context, address out of bounds returns 0.0
	      // instead of an error.
	    if (address >= vals->get_size())
		  return 0.0;

	    double val;
	    vals->get_word(address, val);
	    return val;
      }

      if (address >= get_size())
	    return 0.0;

      assert(nets);
      vpiHandle word = nets[address];
      struct __vpiRealVar*vsig = dynamic_cast<__vpiRealVar*>(word);
      assert(vsig);
      vvp_signal_value*sig = dynamic_cast<vvp_signal_value*> (vsig->net->fil);
      assert(sig);

      double val = sig->real_value();
      return val;
}

void __vpiArray::get_word_obj(unsigned address, vvp_object_t&val)
{
      if (vals) {
	    assert(vals4 == 0);
	    assert(nets  == 0);
	      // In this context, address out of bounds returns 0.0
	      // instead of an error.
	    if (address >= vals->get_size()) {
		  val = vvp_object_t();
		  return;
	    }

	    vals->get_word(address, val);
	    return;
      }

      assert(nets);
	// Arrays of string nets not implemented!
      assert(0);
      return;
}

string __vpiArray::get_word_str(unsigned address)
{
      if (vals) {
	    assert(vals4 == 0);
	    assert(nets  == 0);
	      // In this context, address out of bounds returns 0.0
	      // instead of an error.
	    if (address >= vals->get_size())
		  return "";

	    string val;
	    vals->get_word(address, val);
	    return val;
      }

      assert(nets);
	// Arrays of string nets not implemented!
      assert(0);
      return "";
}

vpiHandle vpip_make_array(char*label, const char*name,
				 int first_addr, int last_addr,
				 bool signed_flag)
{
      struct __vpiArray*obj = new __vpiArray;

      obj->signed_flag = signed_flag;

	// Assume increasing addresses.
      if (last_addr >= first_addr) {
	    obj->swap_addr = false;
      } else {
	    obj->swap_addr = true;
	    int tmp = last_addr;
	    last_addr = first_addr;
	    first_addr = tmp;
      }
      assert(last_addr >= first_addr);
      unsigned array_count = last_addr+1-first_addr;

	// For now, treat all arrays as memories. This is not quite
	// correct, as arrays are arrays with memories a special case.
      obj->scope = vpip_peek_current_scope();
      obj->name  = vpip_name_string(name);
      obj->array_count = array_count;

      obj->first_addr.set_value(first_addr);
      obj->last_addr .set_value(last_addr);

	// Start off now knowing if we are nets or variables.
      obj->nets = 0;
      obj->vals4 = 0;
      obj->vals  = 0;
      obj->vals_width = 0;
      obj->vals_words = 0;

	// Initialize (clear) the read-ports list.
      obj->ports_ = 0;
      obj->vpi_callbacks = 0;

	/* Add this symbol to the array_symbols table for later lookup. */
      if (!array_table)
	    array_table = new symbol_map_s<struct __vpiArray>;

      assert(!array_find(label));
      array_table->sym_set_value(label, obj);

	/* Add this into the table of VPI objects. This is used for
	   contexts that try to look up VPI objects in
	   general. (i.e. arguments to vpi_task calls.) */
      compile_vpi_symbol(label, obj);

	/* Blindly attach to the scope as an object. */
      vpip_attach_to_current_scope(obj);

      return obj;
}

void __vpiArray::alias_word(unsigned long addr, vpiHandle word, int msb_, int lsb_)
{
      assert(msb.get_value() == msb_);
      assert(lsb.get_value() == lsb_);
      assert(addr < get_size());
      assert(nets);
      nets[addr] = word;
}

void __vpiArray::attach_word(unsigned addr, vpiHandle word)
{
      assert(addr < get_size());
      assert(nets);
      nets[addr] = word;

      if (struct __vpiSignal*sig = dynamic_cast<__vpiSignal*>(word)) {
	    vvp_net_t*net = sig->node;
	    assert(net);
	    vvp_vpi_callback*fun = dynamic_cast<vvp_vpi_callback*>(net->fil);
	    assert(fun);
	    fun->attach_as_word(this, addr);
	    sig->is_netarray = 1;
	    sig->within.parent = this;
	    sig->id.index = new __vpiDecConst(addr + first_addr.get_value());
	      // Now we know the data type, update the array signed_flag.
	    signed_flag = sig->signed_flag;
	    return;
      }

      if (struct __vpiRealVar*sig = dynamic_cast<__vpiRealVar*>(word)) {
	    vvp_net_t*net = sig->net;
	    assert(net);
	    vvp_vpi_callback*fun = dynamic_cast<vvp_vpi_callback*>(net->fil);
	    assert(fun);
	    fun->attach_as_word(this, addr);
	    sig->is_netarray = 1;
	    sig->within.parent = this;
	    sig->id.index = new __vpiDecConst(addr + first_addr.get_value());
	      // Now we know the data type, update the array signed_flag.
	    signed_flag = true;
	    return;
      }
}

void compile_var_array(char*label, char*name, int last, int first,
		   int msb, int lsb, char signed_flag)
{
      vpiHandle obj = vpip_make_array(label, name, first, last,
                                      signed_flag != 0);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);

	/* Make the words. */
      arr->vals_width = labs(msb-lsb) + 1;
      if (vpip_peek_current_scope()->is_automatic()) {
            arr->vals4 = new vvp_vector4array_aa(arr->vals_width,
						 arr->get_size());
      } else {
            arr->vals4 = new vvp_vector4array_sa(arr->vals_width,
						 arr->get_size());
      }
      arr->msb.set_value(msb);
      arr->lsb.set_value(lsb);

      count_var_arrays += 1;
      count_var_array_words += arr->get_size();

      free(label);
      delete[] name;
}

void compile_var2_array(char*label, char*name, int last, int first,
		   int msb, int lsb, bool signed_flag)
{
      vpiHandle obj = vpip_make_array(label, name, first, last, signed_flag);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);

	/* Make the words. */
      arr->msb.set_value(msb);
      arr->lsb.set_value(lsb);
      arr->vals_width = labs(msb-lsb) + 1;

      assert(! arr->nets);
      if (lsb == 0 && msb == 7 && signed_flag) {
	    arr->vals = new vvp_darray_atom<int8_t>(arr->get_size());
      } else if (lsb == 0 && msb == 7 && !signed_flag) {
	    arr->vals = new vvp_darray_atom<uint8_t>(arr->get_size());
      } else if (lsb == 0 && msb == 15 && signed_flag) {
	    arr->vals = new vvp_darray_atom<int16_t>(arr->get_size());
      } else if (lsb == 0 && msb == 15 && !signed_flag) {
	    arr->vals = new vvp_darray_atom<uint16_t>(arr->get_size());
      } else if (lsb == 0 && msb == 31 && signed_flag) {
	    arr->vals = new vvp_darray_atom<int32_t>(arr->get_size());
      } else if (lsb == 0 && msb == 31 && !signed_flag) {
	    arr->vals = new vvp_darray_atom<uint32_t>(arr->get_size());
      } else if (lsb == 0 && msb == 63 && signed_flag) {
	    arr->vals = new vvp_darray_atom<int64_t>(arr->get_size());
      } else if (lsb == 0 && msb == 63 && !signed_flag) {
	    arr->vals = new vvp_darray_atom<uint64_t>(arr->get_size());
      } else {
	    arr->vals = new vvp_darray_vec2(arr->get_size(), arr->vals_width);
      }
      count_var_arrays += 1;
      count_var_array_words += arr->get_size();

      free(label);
      delete[] name;
}

void compile_real_array(char*label, char*name, int last, int first)
{
      vpiHandle obj = vpip_make_array(label, name, first, last, true);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);

	/* Make the words. */
      arr->vals = new vvp_darray_real(arr->get_size());
      arr->vals_width = 1;

      count_real_arrays += 1;
      count_real_array_words += arr->get_size();

      free(label);
      delete[] name;
}

void compile_string_array(char*label, char*name, int last, int first)
{
      vpiHandle obj = vpip_make_array(label, name, first, last, true);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);

	/* Make the words. */
      arr->vals = new vvp_darray_string(arr->get_size());
      arr->vals_width = 1;

      count_real_arrays += 1;
      count_real_array_words += arr->get_size();

      free(label);
      delete[] name;
}

void compile_object_array(char*label, char*name, int last, int first)
{
      vpiHandle obj = vpip_make_array(label, name, first, last, true);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);

	/* Make the words. */
      arr->vals = new vvp_darray_object(arr->get_size());
      arr->vals_width = 1;

      count_real_arrays += 1;
      count_real_array_words += arr->get_size();

      free(label);
      delete[] name;
}

void compile_net_array(char*label, char*name, int last, int first)
{
	// At this point we don't know the array data type, so we
	// initialise signed_flag to false. This will be corrected
	// (if necessary) when we attach words to the array.
      vpiHandle obj = vpip_make_array(label, name, first, last, false);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(obj);
      arr->nets = (vpiHandle*)calloc(arr->get_size(), sizeof(vpiHandle));

      count_net_arrays += 1;
      count_net_array_words += arr->get_size();

      free(label);
      delete[] name;
}

class vvp_fun_arrayport  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net);
      explicit vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net, long addr);
      ~vvp_fun_arrayport();

      virtual void check_word_change(unsigned long addr) = 0;

    protected:
      vvp_array_t arr_;
      vvp_net_t  *net_;
      unsigned long addr_;

      friend void array_attach_port(vvp_array_t, vvp_fun_arrayport*);
      friend void __vpiArray::word_change(unsigned long);
      vvp_fun_arrayport*next_;
};

vvp_fun_arrayport::vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net)
: arr_(mem), net_(net), addr_(0)
{
      next_ = 0;
}

vvp_fun_arrayport::vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net, long addr)
: arr_(mem), net_(net), addr_(addr)
{
      next_ = 0;
}

vvp_fun_arrayport::~vvp_fun_arrayport()
{
}

class vvp_fun_arrayport_sa  : public vvp_fun_arrayport {

    public:
      explicit vvp_fun_arrayport_sa(vvp_array_t mem, vvp_net_t*net);
      explicit vvp_fun_arrayport_sa(vvp_array_t mem, vvp_net_t*net, long addr);
      ~vvp_fun_arrayport_sa();

      void check_word_change(unsigned long addr);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t);

    private:
};

vvp_fun_arrayport_sa::vvp_fun_arrayport_sa(vvp_array_t mem, vvp_net_t*net)
: vvp_fun_arrayport(mem, net)
{
}

vvp_fun_arrayport_sa::vvp_fun_arrayport_sa(vvp_array_t mem, vvp_net_t*net, long addr)
: vvp_fun_arrayport(mem, net, addr)
{
}

vvp_fun_arrayport_sa::~vvp_fun_arrayport_sa()
{
}

void vvp_fun_arrayport_sa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                     vvp_context_t)
{
      bool addr_valid_flag;

      switch (port.port()) {

	  case 0: // Address input
	    addr_valid_flag = vector4_to_value(bit, addr_);
	    if (! addr_valid_flag)
		  addr_ = arr_->get_size();
	    if (vpi_array_is_real(arr_))
		  port.ptr()->send_real(arr_->get_word_r(addr_), 0);
	    else
		  port.ptr()->send_vec4(arr_->get_word(addr_), 0);

	    break;

	  default:
	    fprintf(stdout, "XXXX write ports not implemented.\n");
	    assert(0);
      }
}

void vvp_fun_arrayport_sa::check_word_change(unsigned long addr)
{
      if (addr != addr_) return;

      if (vpi_array_is_real(arr_)) {
	    net_->send_real(arr_->get_word_r(addr_), 0);
      } else {
	    net_->send_vec4(arr_->get_word(addr_), 0);
      }
}

class vvp_fun_arrayport_aa  : public vvp_fun_arrayport, public automatic_hooks_s {

    public:
      explicit vvp_fun_arrayport_aa(__vpiScope*context_scope, vvp_array_t mem, vvp_net_t*net);
      explicit vvp_fun_arrayport_aa(__vpiScope*context_scope, vvp_array_t mem, vvp_net_t*net, long addr);
      ~vvp_fun_arrayport_aa();

      void alloc_instance(vvp_context_t context);
      void reset_instance(vvp_context_t context);
#ifdef CHECK_WITH_VALGRIND
      void free_instance(vvp_context_t context);
#endif

      void check_word_change(unsigned long addr);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                     vvp_context_t context);

    private:
      void check_word_change_(unsigned long addr, vvp_context_t context);

      __vpiScope*context_scope_;
      unsigned context_idx_;
};

vvp_fun_arrayport_aa::vvp_fun_arrayport_aa(__vpiScope*context_scope, vvp_array_t mem, vvp_net_t*net)
: vvp_fun_arrayport(mem, net), context_scope_(context_scope)
{
      context_idx_ = vpip_add_item_to_context(this, context_scope);
}

vvp_fun_arrayport_aa::vvp_fun_arrayport_aa(__vpiScope*context_scope, vvp_array_t mem, vvp_net_t*net, long addr)
: vvp_fun_arrayport(mem, net, addr), context_scope_(context_scope)
{
      context_idx_ = vpip_add_item_to_context(this, context_scope);
}

vvp_fun_arrayport_aa::~vvp_fun_arrayport_aa()
{
}

void vvp_fun_arrayport_aa::alloc_instance(vvp_context_t context)
{
      unsigned long*addr = new unsigned long;
      vvp_set_context_item(context, context_idx_, addr);

      *addr = addr_;
}

void vvp_fun_arrayport_aa::reset_instance(vvp_context_t context)
{
      unsigned long*addr = static_cast<unsigned long*>
            (vvp_get_context_item(context, context_idx_));

      *addr = addr_;
}

#ifdef CHECK_WITH_VALGRIND
void vvp_fun_arrayport_aa::free_instance(vvp_context_t context)
{
      unsigned long*addr = static_cast<unsigned long*>
            (vvp_get_context_item(context, context_idx_));
      delete addr;
}
#endif

void vvp_fun_arrayport_aa::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit,
                                     vvp_context_t context)
{
      if (context) {
            unsigned long*addr = static_cast<unsigned long*>
                  (vvp_get_context_item(context, context_idx_));

            bool addr_valid_flag;

            switch (port.port()) {

                case 0: // Address input
                  addr_valid_flag = vector4_to_value(bit, *addr);
                  if (! addr_valid_flag) *addr = arr_->get_size();
                  if (vpi_array_is_real(arr_)) {
			port.ptr()->send_real(arr_->get_word_r(*addr),
					      context);
                  } else {
			port.ptr()->send_vec4(arr_->get_word(*addr),
					      context);
                  }
                  break;

                default:
                  fprintf(stdout, "XXXX write ports not implemented.\n");
                  assert(0);
            }
      } else {
            context = context_scope_->live_contexts;
            while (context) {
                  recv_vec4(port, bit, context);
                  context = vvp_get_next_context(context);
            }
      }
}

void vvp_fun_arrayport_aa::check_word_change_(unsigned long addr,
                                              vvp_context_t context)
{
      unsigned long*port_addr = static_cast<unsigned long*>
            (vvp_get_context_item(context, context_idx_));

      if (addr != *port_addr)
	    return;

      if (vpi_array_is_real(arr_)) {
	    net_->send_real(arr_->get_word_r(addr), context);
      } else {
	    net_->send_vec4(arr_->get_word(addr), context);
      }
}

void vvp_fun_arrayport_aa::check_word_change(unsigned long addr)
{
      if (arr_->get_scope()->is_automatic()) {
            assert(vthread_get_wt_context());
            check_word_change_(addr, vthread_get_wt_context());
      } else {
            vvp_context_t context = context_scope_->live_contexts;
            while (context) {
                  check_word_change_(addr, context);
                  context = vvp_get_next_context(context);
            }
      }
}

static void array_attach_port(vvp_array_t array, vvp_fun_arrayport*fun)
{
      assert(fun->next_ == 0);
      fun->next_ = array->ports_;
      array->ports_ = fun;
      if (!array->get_scope()->is_automatic() &&
          (array->vals4 || array->vals)) {
              /* propagate initial values for variable arrays */
            if (!vpi_array_is_real(array)) {
                  vvp_bit4_t init;
                  if (array->vals4)
                      init = BIT4_X;
                  else
                      init = BIT4_0;
                  vvp_vector4_t tmp(array->vals_width, init);
                  schedule_init_propagate(fun->net_, tmp);
            } else {
                  schedule_init_propagate(fun->net_, 0.0);
            }
      }
}

class array_word_value_callback : public value_callback {
    public:
      inline explicit array_word_value_callback(p_cb_data data)
      : value_callback(data)
      { }

    public:
      long word_addr;
};

void __vpiArray::word_change(unsigned long addr)
{
      for (vvp_fun_arrayport*cur = ports_; cur; cur = cur->next_)
	    cur->check_word_change(addr);

	// Run callbacks attached to the array itself.
      struct __vpiCallback *next = vpi_callbacks;
      struct __vpiCallback *prev = 0;

      while (next) {
	    array_word_value_callback*cur = dynamic_cast<array_word_value_callback*>(next);
	    next = cur->next;

	      // Skip callbacks that are not for me. -1 is for every element.
	    if (cur->word_addr != (long)addr && cur->word_addr != -1) {
		  prev = cur;
		  continue;
	    }

	      // For whole array callbacks we need to set the index.
	    if (cur->word_addr == -1) {
		  cur->cb_data.index = (PLI_INT32) ((int)addr +
						    first_addr.get_value());
	    }

	    if (cur->cb_data.cb_rtn != 0) {
		  if (cur->test_value_callback_ready()) {
			if (cur->cb_data.value) {
			      if (vpi_array_is_real(this)) {
				    double val = 0.0;
				    if (addr < vals->get_size())
					  vals->get_word(addr, val);
				    vpip_real_get_value(val, cur->cb_data.value);
			      } else if (vals4) {
				    vpip_vec4_get_value(vals4->get_word(addr),
							vals_width,
							signed_flag,
							cur->cb_data.value);
			      } else if (dynamic_cast<vvp_darray_atom<int8_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<int16_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<int32_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<int64_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<uint8_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<uint16_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<uint32_t>*>(vals)
				      || dynamic_cast<vvp_darray_atom<uint64_t>*>(vals)
				      || dynamic_cast<vvp_darray_vec2*>(vals)) {
				    vvp_vector4_t val;
				    if (addr < vals->get_size())
					  vals->get_word(addr, val);
				    vpip_vec4_get_value(val,
							vals_width,
							signed_flag,
							cur->cb_data.value);
			      } else {
			            assert(0);
			      }
			}

			callback_execute(cur);
		  }

		  prev = cur;

	    } else if (prev == 0) {

		  vpi_callbacks = next;
		  cur->next = 0;
		  delete cur;

	    } else {
		  assert(prev->next == cur);
		  prev->next = next;
		  cur->next = 0;
		  delete cur;
	    }
      }
}

class array_resolv_list_t : public resolv_list_s {

    public:
      explicit array_resolv_list_t(char*lab) : resolv_list_s(lab) {
	    array = 0;
      }

      vvp_array_t*array;
      bool resolve(bool mes);

    private:
};

bool array_resolv_list_t::resolve(bool mes)
{
      *array = array_find(label());
      if (*array == 0) {
	    assert(!mes);
	    return false;
      }
      return true;
}

class array_port_resolv_list_t : public resolv_list_s {

    public:
      explicit array_port_resolv_list_t(char* lab, bool use_addr__,
                                        long addr__);

      __vpiScope*context_scope;
      vvp_net_t*ptr;
      bool use_addr;
      long addr;
      bool resolve(bool mes);

    private:
};

array_port_resolv_list_t::array_port_resolv_list_t(char *lab, bool use_addr__,
                                                   long addr__)
: resolv_list_s(lab), use_addr(use_addr__), addr(addr__)
{
      if (vpip_peek_current_scope()->is_automatic())
            context_scope = vpip_peek_context_scope();
      else
            context_scope = 0;
      ptr = new vvp_net_t;
}

bool array_port_resolv_list_t::resolve(bool mes)
{
      vvp_array_t mem = array_find(label());
      if (mem == 0) {
	    assert(mem || !mes);
	    return false;
      }

      vvp_fun_arrayport*fun;
      if (use_addr)
            if (context_scope)
                  fun = new vvp_fun_arrayport_aa(context_scope, mem, ptr, addr);
            else
                  fun = new vvp_fun_arrayport_sa(mem, ptr, addr);
      else
            if (context_scope)
                  fun = new vvp_fun_arrayport_aa(context_scope, mem, ptr);
            else
                  fun = new vvp_fun_arrayport_sa(mem, ptr);
      ptr->fun = fun;

      array_attach_port(mem, fun);

      return true;
}

class array_word_part_callback : public array_word_value_callback {
    public:
      explicit array_word_part_callback(p_cb_data data);
      ~array_word_part_callback();

      bool test_value_callback_ready(void);

    private:
      char*value_bits_;
};

array_word_part_callback::array_word_part_callback(p_cb_data data)
: array_word_value_callback(data)
{
	// Get the initial value of the part, to use as a reference.
      struct __vpiArrayVthrAPV*apvword = dynamic_cast<__vpiArrayVthrAPV*>(data->obj);
      s_vpi_value tmp_value;
      tmp_value.format = vpiBinStrVal;
      apvword->vpi_get_value(&tmp_value);

      value_bits_ = new char[apvword->part_wid+1];

      memcpy(value_bits_, tmp_value.value.str, apvword->part_wid);
      value_bits_[apvword->part_wid] = 0;
}

array_word_part_callback::~array_word_part_callback()
{
      delete[]value_bits_;
}

bool array_word_part_callback::test_value_callback_ready(void)
{
      struct __vpiArrayVthrAPV*apvword = dynamic_cast<__vpiArrayVthrAPV*>(cb_data.obj);
      assert(apvword);

	// Get a reference value that can be used to compare with an
	// updated value.
      s_vpi_value tmp_value;
      tmp_value.format = vpiBinStrVal;
      apvword->vpi_get_value(&tmp_value);

      if (memcmp(value_bits_, tmp_value.value.str, apvword->part_wid) == 0)
	    return false;

      memcpy(value_bits_, tmp_value.value.str, apvword->part_wid);
      return true;

}

value_callback*vpip_array_word_change(p_cb_data data)
{
      struct __vpiArray*parent = 0;
      array_word_value_callback*cbh = 0;
      if (struct __vpiArrayWord*word = array_var_word_from_handle(data->obj)) {
	    parent = static_cast<__vpiArray*>(word->get_parent());
	    unsigned addr = word->get_index();
	    cbh = new array_word_value_callback(data);
	    cbh->word_addr = addr;

      } else if (struct __vpiArrayVthrA*tword = dynamic_cast<__vpiArrayVthrA*>(data->obj)) {
	    parent = tword->array;
	    cbh = new array_word_value_callback(data);
	    cbh->word_addr = tword->address;

      } else if (struct __vpiArrayVthrAPV*apvword = dynamic_cast<__vpiArrayVthrAPV*>(data->obj)) {
	    parent = apvword->array;
	    cbh = new array_word_part_callback(data);
	    cbh->word_addr = apvword->word_sel;
      }

      assert(cbh);
      assert(parent);
      cbh->next = parent->vpi_callbacks;
      parent->vpi_callbacks = cbh;

      return cbh;
}

value_callback* vpip_array_change(p_cb_data data)
{
      array_word_value_callback*cbh = new array_word_value_callback(data);
      assert(data->obj);

      struct __vpiArray*arr = dynamic_cast<__vpiArray*>(data->obj);
      cbh->word_addr = -1; // This is a callback for every element.
      cbh->next = arr->vpi_callbacks;
      arr->vpi_callbacks = cbh;
      return cbh;
}

void compile_array_port(char*label, char*array, char*addr)
{
      array_port_resolv_list_t*resolv_mem
	    = new array_port_resolv_list_t(array, false, 0);

      define_functor_symbol(label, resolv_mem->ptr);
      free(label);
	// Connect the port-0 input as the address.
      input_connect(resolv_mem->ptr, 0, addr);

      resolv_submit(resolv_mem);
}

void compile_array_port(char*label, char*array, long addr)
{
      array_port_resolv_list_t*resolv_mem
	    = new array_port_resolv_list_t(array, true, addr);

      define_functor_symbol(label, resolv_mem->ptr);
      free(label);

      resolv_submit(resolv_mem);
}

void compile_array_alias(char*label, char*name, char*src)
{
      vvp_array_t mem = array_find(src);
      assert(mem);

      struct __vpiArray*obj = new __vpiArray;

      obj->scope = vpip_peek_current_scope();
      obj->name  = vpip_name_string(name);
      obj->array_count = mem->array_count;
      obj->signed_flag = mem->signed_flag;

	// Need to set an accurate range of addresses.
      obj->first_addr = mem->first_addr;
      obj->last_addr  = mem->last_addr;
      obj->swap_addr = mem->swap_addr;

      obj->msb = mem->msb;
      obj->lsb = mem->lsb;

	// Share the words with the source array.
      obj->nets = mem->nets;
      obj->vals4 = mem->vals4;
      obj->vals  = mem->vals;
      obj->vals_width = mem->vals_width;
      obj->vals_words = mem->vals_words;

      obj->ports_ = 0;
      obj->vpi_callbacks = 0;

      assert(array_table);
      assert(!array_find(label));
      array_table->sym_set_value(label, obj);

      compile_vpi_symbol(label, obj);
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
      free(src);
}

/*
 * &A<label,addr>
 * This represents a VPI handle for an addressed array. This comes
 * from expressions like "label[addr]" where "label" is the array and
 * "addr" is the canonical address of the desired word.
 */
vpiHandle vpip_make_vthr_A(char*label, unsigned addr)
{
      struct __vpiArrayVthrA*obj = new __vpiArrayVthrA;

      array_resolv_list_t*resolv_mem
	    = new array_resolv_list_t(label);

      resolv_mem->array = &obj->array;
      resolv_submit(resolv_mem);

      obj->address_handle = 0;
      obj->address = addr;

      return obj;
}

/*
 * &A<label,symbol>
 * This represents a VPI handle for an addressed word, where the
 * word address is calculated from the VPI object that symbol
 * represents. The expression that leads to this looks like label[symbol].
 */
vpiHandle vpip_make_vthr_A(char*label, char*symbol)
{
      struct __vpiArrayVthrA*obj = new __vpiArrayVthrA;

      array_resolv_list_t*resolv_mem
	    = new array_resolv_list_t(label);

      resolv_mem->array = &obj->array;
      resolv_submit(resolv_mem);

      obj->address_handle = 0;
      compile_vpi_lookup(&obj->address_handle, symbol);
      obj->address = 0;

      return obj;
}

vpiHandle vpip_make_vthr_A(char*label, vpiHandle handle)
{
      struct __vpiArrayVthrA*obj = new __vpiArrayVthrA;

      array_resolv_list_t*resolv_mem
	    = new array_resolv_list_t(label);

      resolv_mem->array = &obj->array;
      resolv_submit(resolv_mem);

      obj->address_handle = handle;
      obj->address = 0;

      return obj;
}

vpiHandle vpip_make_vthr_APV(char*label, unsigned index, unsigned bit, unsigned wid)
{
      struct __vpiArrayVthrAPV*obj = new __vpiArrayVthrAPV;

      array_resolv_list_t*resolv_mem
	    = new array_resolv_list_t(label);

      resolv_mem->array = &obj->array;
      resolv_submit(resolv_mem);

      obj->word_sel = index;
      obj->part_bit = bit;
      obj->part_wid = wid;

      return obj;
}

void compile_array_cleanup(void)
{
      delete array_table;
      array_table = 0;
}

#ifdef CHECK_WITH_VALGRIND
void memory_delete(vpiHandle item)
{
      struct __vpiArray*arr = (struct __vpiArray*) item;
      if (arr->vals_words) delete [] (arr->vals_words-1);

//      if (arr->vals4) {}
// Delete the individual words?
// constant_delete(handle)?
      delete arr->vals4;

//      if (arr->vals) {}
// Delete the individual words?
// constant_delete(handle)?
      delete arr->vals;

      if (arr->nets) {
	    for (unsigned idx = 0; idx < arr->get_size(); idx += 1) {
		  if (struct __vpiSignal*sig =
		      dynamic_cast<__vpiSignal*>(arr->nets[idx])) {
// Delete the individual words?
			constant_delete(sig->id.index);
		    /* These should only be the real words. */
		  } else {
			assert(arr->nets[idx]->get_type_code() ==
			       vpiRealVar);
			struct __vpiRealVar *sigr = (struct __vpiRealVar *)
			                            arr->nets[idx];
			constant_delete(sigr->id.index);
// Why are only the real words still here?
			delete arr->nets[idx];
		  }
	    }
	    free(arr->nets);
      }

      while (arr->vpi_callbacks) {
	    struct __vpiCallback*tmp = arr->vpi_callbacks->next;
	    delete arr->vpi_callbacks;
	    arr->vpi_callbacks = tmp;
      }

      delete arr;
}

void A_delete(vpiHandle item)
{
      struct __vpiArrayVthrA*obj = (struct __vpiArrayVthrA*) item;
      if (obj->address_handle) {
	    switch (obj->address_handle->get_type_code()) {
		case vpiMemoryWord:
		  if (vpi_get(_vpiFromThr, obj->address_handle) == _vpi_at_A) {
			A_delete(obj->address_handle);
		  }
		  break;
		case vpiPartSelect:
		  assert(vpi_get(_vpiFromThr, obj->address_handle) ==
		         _vpi_at_PV);
		  PV_delete(obj->address_handle);
		  break;
	    }
      }

      delete obj;
}

void APV_delete(vpiHandle item)
{
      struct __vpiArrayVthrAPV*obj = (struct __vpiArrayVthrAPV*) item;
      delete obj;
}
#endif
