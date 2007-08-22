/*
 * Copyright (c) 1999-2005 Stephen Williams (steve@icarus.com>
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
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
#ident "$Id: vpi_memory.cc,v 1.33 2006/03/06 05:43:15 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "memory.h"
# include  "statistics.h"
# include  <iostream>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

# include  <stdio.h>

extern const char hex_digits[256];

static void memory_make_word_handles(struct __vpiMemory*rfp);

struct __vpiMemoryWord {
      struct __vpiHandle base;
      struct __vpiMemory*mem;
      struct __vpiDecConst index;
};

struct __vpiMemory {
      struct __vpiHandle base;
      struct __vpiScope* scope;
      struct __vpiMemoryWord*words;
      vvp_memory_t mem;
      const char*name; /* Permanently allocated string. */
      struct __vpiDecConst left_range;
      struct __vpiDecConst right_range;
      struct __vpiDecConst word_left_range;
      struct __vpiDecConst word_right_range;

      struct __vpiCallback*value_change_cb;
};

struct __vpiMemWordIterator {
      struct __vpiHandle base;
      struct __vpiMemory*mem;
      unsigned next;
};

static vpiHandle memory_get_handle(int code, vpiHandle obj)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)obj;

      assert(obj->vpi_type->type_code==vpiMemory);

      switch(code){
	  case vpiLeftRange:
	    return &(rfp->left_range.base);

	  case vpiRightRange:
	    return &(rfp->right_range.base);

	  case vpiScope:
	    return &rfp->scope->base;

      }

      return 0;
}

static int vpi_memory_get(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;

      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiSize:
	    return (int)memory_word_count(rfp->mem);

	  default:
	    return 0;
      }
}

static char* memory_get_str(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code==vpiMemory);

      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;

      char *bn = strdup(vpi_get_str(vpiFullName, &rfp->scope->base));

      char *rbuf = need_result_buf(strlen(bn)+strlen(rfp->name)+2, RBUF_STR);

      switch (code) {
	  case vpiFullName:
	    sprintf(rbuf, "%s.%s", bn, rfp->name);
	    free(bn);
	    return rbuf;
	  case vpiName:
	    strcpy(rbuf, rfp->name);
	    free(bn);
	    return rbuf;
      }

      free(bn);
      return 0;
}

static vpiHandle memory_scan(vpiHandle ref, int)
{
      struct __vpiMemWordIterator*obj = (struct __vpiMemWordIterator*)ref;
      assert(ref->vpi_type->type_code == vpiIterator);

      if (obj->next >= memory_word_count(obj->mem->mem)) {
	    vpi_free_object(ref);
	    return 0;
      }

      return &obj->mem->words[obj->next++].base;
}

static int mem_iter_free_object(vpiHandle ref)
{
      free(ref);
      return 1;
}

static const struct __vpirt vpip_mem_iter_rt = {
      vpiIterator,
      0,
      0,
      0,
      0,
      0,
      0,
      memory_scan,
      &mem_iter_free_object
};


static vpiHandle memory_iterate(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiMemoryWord: {
		memory_make_word_handles(rfp);

		struct __vpiMemWordIterator*res =
		      (struct __vpiMemWordIterator*)
		      calloc(1, sizeof(struct __vpiMemWordIterator));
		assert(res);
		res->base.vpi_type = &vpip_mem_iter_rt;
		res->mem  = rfp;
		res->next = 0;
		return &(res->base);
	  }
      }

      return 0;
}

static vpiHandle memory_index(vpiHandle ref, int index)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      if (index >= (int)memory_word_count(rfp->mem))
	    return 0;
      if (index < 0)
	    return 0;

      memory_make_word_handles(rfp);
      return &(rfp->words[index].base);
}

//==============================

static vpiHandle memory_word_get_handle(int code, vpiHandle obj)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)obj;
      assert(obj->vpi_type->type_code==vpiMemoryWord);

      switch(code){
	  case vpiLeftRange:
	    return &(rfp->mem->word_left_range.base);

	  case vpiRightRange:
	    return &(rfp->mem->word_right_range.base);

	  case vpiIndex:
	    return &(rfp->index.base);
      }


      return 0;
}

static int memory_word_get(int code, vpiHandle ref)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      switch (code) {
	  case vpiSize:
	    return memory_word_width(rfp->mem->mem);

	  default:
	    return 0;
      }
}

static vpiHandle memory_word_put(vpiHandle ref, p_vpi_value val)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);


	/* Get the width of the memory, and the byte index of the
	   first byte of the word. */
      unsigned width = memory_word_width(rfp->mem->mem);
      unsigned word_addr = rfp->index.value;

	/* Addresses are converted to canonical form by offsetting the
	   address by the lowest index. */
      unsigned addr_off = memory_left_range(rfp->mem->mem, 0);
      if (memory_right_range(rfp->mem->mem, 0) < addr_off)
	    addr_off = memory_right_range(rfp->mem->mem, 0);

      word_addr -= addr_off;

	/* Build up the word value from whatever format the user
	   supplies. */
      vvp_vector4_t put_val (width);

      switch (val->format) {
	  case vpiVectorVal:
	    for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  p_vpi_vecval cur = val->value.vector + (idx/32);
		  int aval = (cur->aval >> (idx%32)) & 1;
		  int bval = (cur->bval >> (idx%32)) & 1;

		    /* Check this bit value conversion. This is
		       specifically defined by the IEEE1364 standard. */
		  vvp_bit4_t bit;
		  if (bval) {
			bit = aval? BIT4_Z : BIT4_X;
		  } else {
			bit = aval? BIT4_1 : BIT4_0;
		  }
		  put_val.set_bit(idx, bit);
	    }
	    break;

	  case vpiIntVal: {
		int cur = val->value.integer;
		for (unsigned idx = 0;  idx < width;  idx += 1) {
		      vvp_bit4_t bit = (cur&1)? BIT4_1 : BIT4_0;
		      put_val.set_bit(idx, bit);
		      cur >>= 1;
		}
		break;
	  }

	  case vpiBinStrVal: {
		char*str = val->value.str;
		for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		      switch (str[width-idx-1]) {
			  case '0':
			    put_val.set_bit(idx, BIT4_0);
			    break;
			  case '1':
			    put_val.set_bit(idx, BIT4_1);
			    break;
			  case 'x':
			  case 'X':
			    put_val.set_bit(idx, BIT4_X);
			    break;
			  case 'z':
			  case 'Z':
			    put_val.set_bit(idx, BIT4_Z);
			    break;
			  default:
			    fprintf(stderr, "Unsupported value %c(%d).\n",
			            str[width-idx-1], str[width-idx-1]);
			    assert(0);
		      }
		}
		break;
	  }

	  case vpiOctStrVal: {
		char*str = val->value.str;
		vpip_oct_str_to_vec4(put_val, str);
		break;
	  }

	  case vpiDecStrVal: {
		char*str = val->value.str;
		vpip_dec_str_to_vec4(put_val, str, false);
		break;
	  }

	  case vpiHexStrVal: {
		char*str = val->value.str;
		vpip_hex_str_to_vec4(put_val, str);
		break;
	  }

	  default:
	    cerr << "internal error: memory_word put_value format="
		 << val->format << endl;
	    assert(0);
      }

      memory_set_word(rfp->mem->mem, word_addr, 0, put_val);
      return 0;
}

static char* memory_word_get_str(int code, vpiHandle ref)
{
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;

      char *bn = strdup(vpi_get_str(vpiFullName, &rfp->mem->scope->base));
      const char *nm = rfp->mem->name;

      char *rbuf = need_result_buf(strlen(bn) + strlen(nm) + 10 + 4, RBUF_STR);

      switch (code) {
	  case vpiFullName:
	    sprintf(rbuf, "%s.%s[%d]", bn, nm, rfp->index.value);
	    free(bn);
	    return rbuf;
	    break;
	  case vpiName: {
	    sprintf(rbuf, "%s[%d]", nm, rfp->index.value);
	    free(bn);
	    return rbuf;
	    break;
	  }
      }

      free(bn);
      return 0;
}

static void memory_word_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(rfp->base.vpi_type->type_code==vpiMemoryWord);

      unsigned width = memory_word_width(rfp->mem->mem);
      unsigned word_address = rfp->index.value;

      vvp_vector4_t word_val = memory_get_word(rfp->mem->mem, word_address);

      vpip_vec4_get_value(word_val, width, false /* never signed */, vp);
}

static const struct __vpirt vpip_memory_rt = {
      vpiMemory,
      vpi_memory_get,
      memory_get_str,
      0,
      0,
      memory_get_handle,
      memory_iterate,
      memory_index,
};

static const struct __vpirt vpip_memory_word_rt = {
      vpiMemoryWord,
      memory_word_get,
      memory_word_get_str,
      memory_word_get_value,
      memory_word_put,
      memory_word_get_handle,
      0,
      0,
};

static void memory_make_word_handles(struct __vpiMemory*rfp)
{
      if (rfp->words != 0)
	    return;

      unsigned word_count = memory_word_count(rfp->mem);

      rfp->words = (struct __vpiMemoryWord*)
	    calloc(word_count, sizeof (struct __vpiMemoryWord));

      for (unsigned idx = 0 ;  idx < word_count ;  idx += 1) {
	    struct __vpiMemoryWord*cur = rfp->words + idx;
	    cur->base.vpi_type = &vpip_memory_word_rt;
	    cur->mem = rfp;
	    vpip_make_dec_const(&cur->index, idx);
      }
}

/*
 * Run the callbacks for a memory value change. The memory.cc methods
 * call this method with the canonical address of the word that
 * changed, and we here run through all the callbacks for the memory,
 * passing the translated index through.
 */
void vpip_run_memory_value_change(vpiHandle ref, unsigned addr)
{
      struct __vpiMemory*obj = reinterpret_cast<struct __vpiMemory*>(ref);

      vvp_vector4_t word_val = memory_get_word(obj->mem, addr);
      unsigned width = memory_word_width(obj->mem);

      for (struct __vpiCallback*cur=obj->value_change_cb;
	   cur != 0 ;  cur = cur->next) {

	    if (cur->cb_data.cb_rtn == 0)
		  continue;

	    if (cur->cb_data.value)
		  vpip_vec4_get_value(word_val, width, false, cur->cb_data.value);

	    cur->cb_data.index = addr;
	    vpi_mode_flag = VPI_MODE_RWSYNC;
	    (cur->cb_data.cb_rtn) (&cur->cb_data);
	    vpi_mode_flag = VPI_MODE_NONE;
      }
}

/*
 * Attach the callback to the memory.
 */
void vpip_memory_value_change(struct __vpiCallback*cbh, vpiHandle ref)
{
      struct __vpiMemory*obj = reinterpret_cast<struct __vpiMemory*>(ref);
      cbh->next = obj->value_change_cb;
      obj->value_change_cb = cbh;
}

vpiHandle vpip_make_memory(vvp_memory_t mem, const char*name)
{
      struct __vpiMemory*obj = (struct __vpiMemory*)
	    malloc(sizeof(struct __vpiMemory));
      count_vpi_memories += 1;

      obj->base.vpi_type = &vpip_memory_rt;
      obj->scope = vpip_peek_current_scope();
      obj->mem = mem;
      obj->name = vpip_name_string(name);
      obj->value_change_cb = 0;

      memory_attach_self(mem, &(obj->base));
      vpip_make_dec_const(&obj->left_range, memory_left_range(mem, 0));
      vpip_make_dec_const(&obj->right_range, memory_right_range(mem, 0));
      vpip_make_dec_const(&obj->word_left_range, memory_word_left_range(mem));
      vpip_make_dec_const(&obj->word_right_range,memory_word_right_range(mem));

      obj->words = 0;

      return &(obj->base);
}

