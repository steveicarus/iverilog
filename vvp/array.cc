/*
 * Copyright (c) 2007-2008 Stephen Williams (steve@icarus.com)
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

# include  "array.h"
#include  "symbols.h"
#include  "schedule.h"
#include  "vpi_priv.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>
# include  <iostream>
# include  "compile.h"
# include  <assert.h>

static symbol_table_t array_table =0;

class vvp_fun_arrayport;
static void array_attach_port(vvp_array_t, vvp_fun_arrayport*);

vvp_array_t array_find(char*label)
{
      if (array_table == 0)
	    return 0;

      symbol_value_t v = sym_get_value(array_table, label);
      return (vvp_array_t)v.ptr;
}

/*
* The vpiArray object holds an array of vpi objects that themselves
* represent the words of the array. The vpi_array_t is a pointer to this.
*/
struct __vpiArray {
      struct __vpiHandle base;
      struct __vpiScope*scope;
      const char*name; /* Permanently allocated string */
      unsigned array_count;
      struct __vpiDecConst first_addr;
      struct __vpiDecConst last_addr;
	// If this is a net array, nets lists the handles.
      vpiHandle*nets;
	// If this is a var array, then these are used instead of nets.
      vvp_vector4_t        *vals;
      unsigned              vals_width;
      struct __vpiArrayWord*vals_words;

      class vvp_fun_arrayport*ports_;
};

struct __vpiArrayIterator {
      struct __vpiHandle base;
      struct __vpiArray*array;
      unsigned next;
};

struct __vpiArrayIndex {
      struct __vpiHandle base;
      struct __vpiDecConst *index;
      unsigned done;
};

/*
 * The vpiArrayWord is magic. It is used as the handle to return when
 * vpi code tries to index or scan an array of variable words. The
 * array word handle contains no actual data. It is just a hook for
 * the vpi methods and to point to the parent.
 *
 * How the point to the parent works is tricky. The vpiArrayWord
 * objects for an array are themselves allocated as an array. All the
 * ArrayWord objects in the array have a word0 that points to the base
 * of the array. Thus, the position into the array (and the index into
 * the memory) is calculated by subtracting word0 from the ArrayWord
 * pointer.
 *
 * To then get to the parent, use word0[-1].parent.
 */
struct __vpiArrayWord {
      struct __vpiHandle base;
      union {
	    struct __vpiArray*parent;
	    struct __vpiArrayWord*word0;
      };
};


static int vpi_array_get(int code, vpiHandle ref);
static char*vpi_array_get_str(int code, vpiHandle ref);
static vpiHandle vpi_array_get_handle(int code, vpiHandle ref);
static vpiHandle vpi_array_iterate(int code, vpiHandle ref);
static vpiHandle vpi_array_index(vpiHandle ref, int index);

static vpiHandle array_iterator_scan(vpiHandle ref, int);
static int array_iterator_free_object(vpiHandle ref);

static vpiHandle array_index_scan(vpiHandle ref, int);
static int array_index_free_object(vpiHandle ref);

static int vpi_array_var_word_get(int code, vpiHandle);
static void vpi_array_var_word_get_value(vpiHandle, p_vpi_value);
static vpiHandle vpi_array_var_word_put_value(vpiHandle, p_vpi_value, int);

static const struct __vpirt vpip_arraymem_rt = {
      vpiMemory,
      vpi_array_get,
      vpi_array_get_str,
      0,
      0,
      vpi_array_get_handle,
      vpi_array_iterate,
      vpi_array_index,
};

static const struct __vpirt vpip_array_iterator_rt = {
      vpiIterator,
      0,
      0,
      0,
      0,
      0,
      0,
      array_iterator_scan,
      &array_iterator_free_object
};

/* This should look a bit odd since it provides a fake iteration on
 * this object. This trickery is used to implement the two forms of
 * index access, simple handle access and iteration access. */
static const struct __vpirt vpip_array_index_rt = {
      vpiIterator,
      0,
      0,
      0,
      0,
      0,
      array_index_iterate,
      array_index_scan,
      array_index_free_object
};

static const struct __vpirt vpip_array_var_word_rt = {
      vpiMemoryWord,
      &vpi_array_var_word_get,
      0,
      &vpi_array_var_word_get_value,
      &vpi_array_var_word_put_value,
      0,
      0,
      0,
      0
};

# define ARRAY_HANDLE(ref) (assert(ref->vpi_type->type_code==vpiMemory), \
			    (struct __vpiArray*)ref)

# define ARRAY_VAR_WORD(ref) (assert(ref->vpi_type->type_code==vpiMemoryWord), \
			      (struct __vpiArrayWord*)ref)

static void array_make_vals_words(struct __vpiArray*parent)
{
      assert(parent->vals_words == 0);
      parent->vals_words = new struct __vpiArrayWord[parent->array_count + 1];

	// Make word[-1] point to the parent.
      parent->vals_words->parent = parent;
	// Now point to word-0
      parent->vals_words += 1;

      struct __vpiArrayWord*words = parent->vals_words;
      for (unsigned idx = 0 ; idx < parent->array_count ; idx += 1) {
	    words[idx].base.vpi_type = &vpip_array_var_word_rt;
	    words[idx].word0 = words;
      }
}

static unsigned decode_array_word_pointer(struct __vpiArrayWord*word,
					  struct __vpiArray*&parent)
{
      struct __vpiArrayWord*word0 = word->word0;
      parent = (word0 - 1) -> parent;
      return word - word0;
}

static int vpi_array_get(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      switch (code) {
	  case vpiLineNo:
	    return 0; // Not implemented for now!

	  case vpiSize:
	    return (int) obj->array_count;

	  default:
	    return 0;
      }
}

static char*vpi_array_get_str(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      if (code == vpiFile) {  // Not implemented for now!
            return simple_set_rbuf_str(file_names[0]);
      }

      return generic_get_str(code, &obj->scope->base, obj->name, NULL);
}

static vpiHandle vpi_array_get_handle(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      switch (code) {

	  case vpiLeftRange:
	    return &(obj->first_addr.base);

	  case vpiRightRange:
	    return &(obj->last_addr.base);

	  case vpiScope:
	    return &obj->scope->base;
      }

      return 0;
}

static vpiHandle vpi_array_iterate(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      switch (code) {

	  case vpiMemoryWord: {
		struct __vpiArrayIterator*res;
		res = (struct __vpiArrayIterator*) calloc(1, sizeof (*res));
		res->base.vpi_type = &vpip_array_iterator_rt;
		res->array = obj;
		res->next = 0;
		return &res->base;
	  }

      }

      return 0;
}

/*
* VPI code passes indices that are not yet converted to canonical
* form, so this index function does it here.
*/
static vpiHandle vpi_array_index(vpiHandle ref, int index)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      index -= obj->first_addr.value;
      if (index >= (long)obj->array_count)
	    return 0;
      if (index < 0)
	    return 0;

      if (obj->nets != 0) {
	    return obj->nets[index];
      }

      if (obj->vals_words == 0)
	    array_make_vals_words(obj);

      return &(obj->vals_words[index].base);
}

static int vpi_array_var_word_get(int code, vpiHandle ref)
{
      struct __vpiArrayWord*obj = ARRAY_VAR_WORD(ref);
      struct __vpiArray*parent;

      decode_array_word_pointer(obj, parent);

      switch (code) {
	  case vpiSize:
	    return (int) parent->vals_width;

	  default:
	    return 0;
      }
}

static void vpi_array_var_word_get_value(vpiHandle ref, p_vpi_value value)
{
      struct __vpiArrayWord*obj = ARRAY_VAR_WORD(ref);
      struct __vpiArray*parent;

      unsigned index = decode_array_word_pointer(obj, parent);

      assert(0);
}

static vpiHandle vpi_array_var_word_put_value(vpiHandle ref, p_vpi_value vp, int flags)
{
      struct __vpiArrayWord*obj = ARRAY_VAR_WORD(ref);
      struct __vpiArray*parent;

      unsigned index = decode_array_word_pointer(obj, parent);

      vvp_vector4_t val = vec4_from_vpi_value(vp, parent->vals_width);
      array_set_word(parent, index, 0, val);
      return ref;
}

# define ARRAY_ITERATOR(ref) (assert(ref->vpi_type->type_code==vpiIterator), \
			      (struct __vpiArrayIterator*)ref)

static vpiHandle array_iterator_scan(vpiHandle ref, int)
{
      struct __vpiArrayIterator*obj = ARRAY_ITERATOR(ref);

      if (obj->next >= obj->array->array_count) {
	    vpi_free_object(ref);
	    return 0;
      }

      assert(obj->array->nets != 0);

      vpiHandle res = obj->array->nets[obj->next];
      obj->next += 1;
      return res;
}

static int array_iterator_free_object(vpiHandle ref)
{
      struct __vpiArrayIterator*obj = ARRAY_ITERATOR(ref);
      free(obj);
      return 1;
}

# define ARRAY_INDEX(ref) (assert(ref->vpi_type->type_code==vpiIterator), \
			   (struct __vpiArrayIndex*)ref)

vpiHandle array_index_iterate(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code == vpiConstant);
      struct __vpiDecConst*obj = (struct __vpiDecConst*)ref;

      if (code == vpiIndex) {
	    struct __vpiArrayIndex*res;
	    res = (struct __vpiArrayIndex*) calloc(1, sizeof (*res));
	    res->base.vpi_type = &vpip_array_index_rt;
	    res->index = obj;
	    res->done = 0;
	    return &res->base;
      }
      return 0;
}

static vpiHandle array_index_scan(vpiHandle ref, int)
{
      struct __vpiArrayIndex*obj = ARRAY_INDEX(ref);

      if (obj->done == 0) {
            obj->done = 1;
            return &obj->index->base;
      }

      vpi_free_object(ref);
      return 0;
}

static int array_index_free_object(vpiHandle ref)
{
      struct __vpiArrayIndex*obj = ARRAY_INDEX(ref);
      free(obj);
      return 1;
}

void array_set_word(vvp_array_t arr,
		    unsigned address,
		    unsigned part_off,
		    vvp_vector4_t val)
{
      if (address >= arr->array_count)
	    return;

      if (arr->vals) {
	    assert(arr->nets == 0);
	    if (part_off != 0) {
		  if (arr->vals[address].size() == 0)
			arr->vals[address] = vvp_vector4_t(arr->vals_width, BIT4_X);
		  if ((part_off + val.size()) > arr->vals[address].size()) {
			cerr << "part_off=" << part_off
			     << " val.size()=" << val.size()
			     << " arr->vals[address].size()=" << arr->vals[address].size()
			     << " arr->vals_width=" << arr->vals_width << endl;
			assert(0);
		  }
		  arr->vals[address].set_vec(part_off, val);
	    } else {
		  arr->vals[address] = val;
	    }
	    array_word_change(arr, address);
	    return;
      }

      assert(arr->nets != 0);

	// Select the word of the array that we affect.
      vpiHandle word = arr->nets[address];
      struct __vpiSignal*vsig = vpip_signal_from_handle(word);
      assert(vsig);

      vvp_net_ptr_t ptr (vsig->node, 0);
      vvp_send_vec4_pv(ptr, val, part_off, val.size(), vpip_size(vsig));
      array_word_change(arr, address);
}

vvp_vector4_t array_get_word(vvp_array_t arr, unsigned address)
{
      if (arr->vals) {
	    assert(arr->nets == 0);

	    vvp_vector4_t tmp;
	    if (address < arr->array_count)
		  tmp = arr->vals[address];

	    if (tmp.size() == 0)
		  tmp = vvp_vector4_t(arr->vals_width, BIT4_X);

	    return tmp;
      }

      assert(arr->vals == 0);
      assert(arr->nets != 0);

      if (address >= arr->array_count) {
	      // Reading outside the array. Return X's but get the
	      // width by looking at a word that we know is present.
	    assert(arr->array_count > 0);
	    vpiHandle word = arr->nets[0];
	    struct __vpiSignal*vsig = vpip_signal_from_handle(word);
	    assert(vsig);
	    vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (vsig->node->fun);
	    assert(sig);
	    return vvp_vector4_t(sig->size(), BIT4_X);
      }

      vpiHandle word = arr->nets[address];
      struct __vpiSignal*vsig = vpip_signal_from_handle(word);
      assert(vsig);
      vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (vsig->node->fun);
      assert(sig);

      vvp_vector4_t val = sig->vec4_value();
      return val;
}

static vpiHandle vpip_make_array(char*label, const char*name,
				 int first_addr, int last_addr)
{
      struct __vpiArray*obj = (struct __vpiArray*)
	    malloc(sizeof(struct __vpiArray));

	// Assume increasing addresses.
      assert(last_addr >= first_addr);
      unsigned array_count = last_addr+1-first_addr;

	// For now, treat all arrays as memories. This is not quite
	// correct, as arrays are arrays with memories a special case.
      obj->base.vpi_type = &vpip_arraymem_rt;
      obj->scope = vpip_peek_current_scope();
      obj->name  = vpip_name_string(name);
      obj->array_count = array_count;

      vpip_make_dec_const(&obj->first_addr, first_addr);
      vpip_make_dec_const(&obj->last_addr, last_addr);

	// Start off now knowing if we are nets or variables.
      obj->nets = 0;
      obj->vals = 0;
      obj->vals_width = 0;
      obj->vals_words = 0;

	// Initialize (clear) the read-ports list.
      obj->ports_ = 0;

	/* Add this symbol to the array_symbols table for later lookup. */
      if (!array_table)
	    array_table = new_symbol_table();

      assert(!array_find(label));
      symbol_value_t v;
      v.ptr = obj;
      sym_set_value(array_table, label, v);

	/* Add this into the table of VPI objects. This is used for
	   contexts that try to look up VPI objects in
	   general. (i.e. arguments to vpi_task calls.) */
      compile_vpi_symbol(label, &(obj->base));

	/* Blindly attach to the scope as an object. */
      vpip_attach_to_current_scope(&(obj->base));

      return &(obj->base);
}

void array_alias_word(vvp_array_t array, unsigned long addr, vpiHandle word)
{
      assert(addr < array->array_count);
      assert(array->nets);
      array->nets[addr] = word;
}

void array_attach_word(vvp_array_t array, unsigned long addr, vpiHandle word)
{
      assert(addr < array->array_count);
      assert(array->nets);
      array->nets[addr] = word;

      if (struct __vpiSignal*sig = vpip_signal_from_handle(word)) {
	    vvp_net_t*net = sig->node;
	    assert(net);
	    vvp_fun_signal_base*fun = dynamic_cast<vvp_fun_signal_base*>(net->fun);
	    assert(fun);
	    fun->attach_as_word(array, addr);
	    sig->parent = &array->base;
	    sig->id.index = vpip_make_dec_const(addr + array->first_addr.value);
      }
}

void compile_var_array(char*label, char*name, int last, int first,
		   int msb, int lsb, char signed_flag)
{
      vpiHandle obj = vpip_make_array(label, name, first, last);

      struct __vpiArray*arr = ARRAY_HANDLE(obj);

	/* Make the words. */
      arr->vals = new vvp_vector4_t[arr->array_count];
      arr->vals_width = labs(msb-lsb) + 1;

      free(label);
      free(name);
}

void compile_real_array(char*label, char*name, int last, int first,
			int msb, int lsb)
{
      vpiHandle obj = vpip_make_array(label, name, first, last);

      struct __vpiArray*arr = ARRAY_HANDLE(obj);
      vvp_array_t array = array_find(label);

	/* Make the words. */
      for (unsigned idx = 0 ;  idx < arr->array_count ;  idx += 1) {
	    char buf[64];
	    snprintf(buf, sizeof buf, "%s_%u", label, idx);
	    compile_varw_real(strdup(buf), array, idx, msb, lsb);
      }

      free(label);
      free(name);
}

void compile_net_array(char*label, char*name, int last, int first)
{
      vpiHandle obj = vpip_make_array(label, name, first, last);

      struct __vpiArray*arr = ARRAY_HANDLE(obj);
      arr->nets = (vpiHandle*)calloc(arr->array_count, sizeof(vpiHandle));

      free(label);
      free(name);
}

class vvp_fun_arrayport  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net);
      explicit vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net, long addr);
      ~vvp_fun_arrayport();

      void check_word_change(unsigned long addr);

      void recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit);

    private:
      vvp_array_t arr_;
      vvp_net_t  *net_;
      unsigned long addr_;

      friend void array_attach_port(vvp_array_t, vvp_fun_arrayport*);
      friend void array_word_change(vvp_array_t, unsigned long);
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

void vvp_fun_arrayport::recv_vec4(vvp_net_ptr_t port, const vvp_vector4_t&bit)
{
      bool addr_valid_flag;

      switch (port.port()) {

	  case 0: // Address input
	    addr_valid_flag = vector4_to_value(bit, addr_);
	    if (! addr_valid_flag)
		  addr_ = arr_->array_count;
	    vvp_send_vec4(port.ptr()->out, array_get_word(arr_,addr_));
	    break;

	  default:
	    fprintf(stdout, "XXXX write ports not implemented.\n");
	    assert(0);
      }
}

void vvp_fun_arrayport::check_word_change(unsigned long addr)
{
      if (addr != addr_)
	    return;

      vvp_vector4_t bit = array_get_word(arr_, addr_);
      vvp_send_vec4(net_->out, bit);
}

static void array_attach_port(vvp_array_t array, vvp_fun_arrayport*fun)
{
      assert(fun->next_ == 0);
      fun->next_ = array->ports_;
      array->ports_ = fun;
}

void array_word_change(vvp_array_t array, unsigned long addr)
{
      for (vvp_fun_arrayport*cur = array->ports_; cur; cur = cur->next_)
	    cur->check_word_change(addr);
}

void compile_array_port(char*label, char*array, char*addr)
{
      vvp_array_t mem = array_find(array);
      assert(mem);

      vvp_net_t*ptr = new vvp_net_t;
      vvp_fun_arrayport*fun = new vvp_fun_arrayport(mem, ptr);
      ptr->fun = fun;

      define_functor_symbol(label, ptr);
	// Connect the port-0 input as the address.
      input_connect(ptr, 0, addr);

      array_attach_port(mem, fun);

      free(label);
      free(array);
	// The input_connect arranges for the array string to be free'ed.
}

void compile_array_port(char*label, char*array, long addr)
{
      vvp_array_t mem = array_find(array);
      assert(mem);

      vvp_net_t*ptr = new vvp_net_t;
      vvp_fun_arrayport*fun = new vvp_fun_arrayport(mem, ptr, addr);
      ptr->fun = fun;

      define_functor_symbol(label, ptr);

	// Other then the array itself, this kind of array port has no
	// inputs.
      array_attach_port(mem, fun);

      free(label);
      free(array);
}

void compile_array_alias(char*label, char*name, char*src)
{
      vvp_array_t mem = array_find(src);
      assert(mem);

      struct __vpiArray*obj = (struct __vpiArray*)
	    malloc(sizeof (struct __vpiArray));

      obj->base.vpi_type = &vpip_arraymem_rt;
      obj->scope = vpip_peek_current_scope();
      obj->name  = vpip_name_string(name);
      obj->array_count = mem->array_count;

	// XXXX Need to set an accurate range of addresses.
      vpip_make_dec_const(&obj->first_addr, mem->first_addr.value);
      vpip_make_dec_const(&obj->last_addr, mem->last_addr.value);

	// Share the words with the source array.
      obj->nets = mem->nets;
      obj->vals = mem->vals;

      obj->ports_ = 0;

      assert(array_table);
      assert(!array_find(label));
      symbol_value_t v;
      v.ptr = obj;
      sym_set_value(array_table, label, v);

      compile_vpi_symbol(label, &obj->base);
      vpip_attach_to_current_scope(&obj->base);

      free(label);
      free(name);
      free(src);
}
