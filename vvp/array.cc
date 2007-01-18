/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
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
#ident "$Id: array.cc,v 1.1 2007/01/18 00:24:10 steve Exp $"
#endif

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
      vpiHandle*words;

      class vvp_fun_arrayport*ports_;
};

struct __vpiArrayIterator {
      struct __vpiHandle base;
      struct __vpiArray*array;
      unsigned next;
};

static int vpi_array_get(int code, vpiHandle ref);
static char*vpi_array_get_str(int code, vpiHandle ref);
static vpiHandle vpi_array_get_handle(int code, vpiHandle ref);
static vpiHandle vpi_array_iterate(int code, vpiHandle ref);
static vpiHandle vpi_array_index(vpiHandle ref, int index);

static vpiHandle array_iterator_scan(vpiHandle ref, int);
static int array_iterator_free_object(vpiHandle ref);

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

# define ARRAY_HANDLE(ref) (assert(ref->vpi_type->type_code==vpiMemory), \
			    (struct __vpiArray*)ref)

static int vpi_array_get(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      switch (code) {
	  case vpiSize:
	    return (int) obj->array_count;

	  default:
	    return 0;
      }
}

static char*vpi_array_get_str(int code, vpiHandle ref)
{
      struct __vpiArray*obj = ARRAY_HANDLE(ref);

      char*bn;
      char*rbuf;
      size_t len;

      switch (code) {
	  case vpiFullName:
	    bn = strdup(vpi_get_str(vpiFullName, &obj->scope->base));
	    len = strlen(bn)+strlen(obj->name)+2;
	    rbuf = need_result_buf(len, RBUF_STR);
	    snprintf(rbuf, len, "%s.%s", bn, obj->name);
	    free(bn);
	    return rbuf;

	  case vpiName:
	    rbuf = need_result_buf(strlen(obj->name)+1, RBUF_STR);
	    strcpy(rbuf, obj->name);
	    return rbuf;
      }

      return 0;
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

      return obj->words[index];
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

      vpiHandle res = obj->array->words[obj->next];
      obj->next += 1;
      return res;
}

static int array_iterator_free_object(vpiHandle ref)
{
      struct __vpiArrayIterator*obj = ARRAY_ITERATOR(ref);
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

	// Select the word of the array that we affect.
      vpiHandle word = arr->words[address];
      struct __vpiSignal*vsig = vpip_signal_from_handle(word);
      assert(vsig);

      vvp_net_ptr_t ptr (vsig->node, 0);
      vvp_send_vec4(ptr, val);
}

vvp_vector4_t array_get_word(vvp_array_t arr, unsigned address)
{
      if (address >= arr->array_count) {
	      // Reading outside the array. Return X's but get the
	      // width by looking at a word that we know is present.
	    assert(arr->array_count > 0);
	    vpiHandle word = arr->words[0];
	    struct __vpiSignal*vsig = vpip_signal_from_handle(word);
	    assert(vsig);
	    vvp_fun_signal_vec*sig = dynamic_cast<vvp_fun_signal_vec*> (vsig->node->fun);
	    assert(sig);
	    return vvp_vector4_t(sig->size(), BIT4_X);
      }

      vpiHandle word = arr->words[address];
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

      obj->words = (vpiHandle*)calloc(array_count, sizeof(vpiHandle));

	// Initialize (clear) the read-ports list.
      obj->ports_ = 0;

	/* Add this symbol to the array_symbols table for later lookup. */
      if (!array_table)
	    array_table = new_symbol_table();

      assert(!array_find(label));
      symbol_value_t v;
      v.ptr = obj;
      sym_set_value(array_table, label, v);

      return &(obj->base);
}

void array_attach_word(vvp_array_t array, vpiHandle word)
{
      unsigned idx;
      for (idx = 0 ;  idx < array->array_count ;  idx += 1) {
	    if (array->words[idx] == 0) {
		  array->words[idx] = word;
		  break;
	    }
      }

      assert(idx < array->array_count);

      if (struct __vpiSignal*sig = vpip_signal_from_handle(word)) {
	    vvp_net_t*net = sig->node;
	    assert(net);
	    vvp_fun_signal_base*fun = dynamic_cast<vvp_fun_signal_base*>(net->fun);
	    assert(fun);
	    fun->attach_as_word(array, idx);
      }
}

void compile_array(char*label, char*name, int last, int first)
{
      vpiHandle obj = vpip_make_array(label, name, first, last);
	/* Add this into the table of VPI objects. This is used for
	   contexts that try to look up VPI objects in
	   general. (i.e. arguments to vpi_task calls.) */
      compile_vpi_symbol(label, obj);
	/* Blindly attach to the scope as an object. */
      vpip_attach_to_current_scope(obj);

      free(label);
      free(name);
}

class vvp_fun_arrayport  : public vvp_net_fun_t {

    public:
      explicit vvp_fun_arrayport(vvp_array_t mem, vvp_net_t*net);
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

/*
 * $Log: array.cc,v $
 * Revision 1.1  2007/01/18 00:24:10  steve
 *  Add missing array source files to CVS.
 *
 */
