/*
 * Copyright (c) 1999-2000 Picture Elements, Inc.
 *    Stephen Williams (steve@picturel.com)
 * Copyright (c) 2001 Stephan Boettcher <stephan@nevis.columbia.edu>
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version. In order to redistribute the software in
 *    binary form, you will need a Picture Elements Binary Software
 *    License.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *  ---
 *    You should also have recieved a copy of the Picture Elements
 *    Binary Software License offer along with the source. This offer
 *    allows you to obtain the right to redistribute the software in
 *    binary (compiled) form. If you have not received it, contact
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpi_memory.cc,v 1.1 2001/05/08 23:59:33 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "memory.h"
# include  <stdlib.h>
# include  <assert.h>


struct __vpiMemoryWord {
      struct __vpiHandle base;
      struct __vpiMemory*mem;
      int index;
};

struct __vpiMemory {
      struct __vpiHandle base;
	/* The signal has a name (this points to static memory.) */
      struct __vpiMemoryWord word;
      vvp_memory_t mem;
};

static int vpi_memory_get(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;

      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiSize:
	    return (int)memory_size(rfp->mem);

	  default:
	    return 0;
      }
}

static char* memory_get_str(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiFullName:
	    return memory_name(rfp->mem);
      }
      
      return 0;
}


static vpiHandle memory_scan(vpiHandle ref, int)
{
      struct __vpiIterator*hp = (struct __vpiIterator*)ref;
      assert(ref->vpi_type->type_code == vpiIterator);

      struct __vpiMemory*rfp = (struct __vpiMemory*)hp->args;
      assert(rfp->base.vpi_type->type_code==vpiMemory);

      if (hp->next >= hp->nargs) {
	    vpi_free_object(ref);
	    return 0;
      }

      rfp->word.index = hp->next++;
      return &rfp->word.base;
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
};

static vpiHandle memory_iterate(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiMemoryWord: {
		struct __vpiIterator*res = (struct __vpiIterator*)
		      calloc(1, sizeof(struct __vpiIterator));
		assert(res);
		res->base.vpi_type = &vpip_mem_iter_rt;
		res->args = (vpiHandle *)&rfp->base;
		res->nargs = memory_size(rfp->mem);
		res->next  = 0;
		return &(res->base);
	  }
      }

      return 0;
}

static vpiHandle memory_index(vpiHandle ref, int index)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      index -= memory_root(rfp->mem);
      if (index >= (int)memory_size(rfp->mem)) return 0;
      if (index < 0) return 0;
      rfp->word.index = index;
      return &rfp->word.base;
}

static int memory_word_get(int code, vpiHandle ref)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      switch (code) {
	  case vpiSize:
	    return memory_data_width(rfp->mem->mem);

	  default:
	    return 0;
      }
}

static vpiHandle memory_word_put(vpiHandle ref, p_vpi_value val,
				 p_vpi_time tim, int flags)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      assert(val->format == vpiVectorVal);

      unsigned width = memory_data_width(rfp->mem->mem);
      unsigned bidx = rfp->index * ((width+3)&~3);

      for (unsigned widx = 0;  widx < width;  widx += 32) {
	    p_vpi_vecval cur = val->value.vector + (widx/32);
	    for (unsigned idx = widx;  idx < width && idx < widx+32;  idx++) {
		  int aval = (cur->aval >> (idx%32)) & 1;
		  int bval = (cur->bval >> (idx%32)) & 1;
		  unsigned char val = (bval<<1) | (aval^bval);
		  memory_set(rfp->mem->mem, bidx+idx, val);
	    }
      }
      return 0;
}

static void memory_word_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(rfp->base.vpi_type->type_code==vpiMemoryWord);
      
      assert(0 && "sorry, not yet");
}

static const struct __vpirt vpip_memory_rt = {
      vpiMemory,
      vpi_memory_get,
      memory_get_str,
      0,
      0,
      0,
      memory_iterate,
      memory_index,
};

static const struct __vpirt vpip_memory_word_rt = {
      vpiMemoryWord,
      memory_word_get,
      0,
      memory_word_get_value,
      memory_word_put,
      0,
      0,
      0,
};

vpiHandle vpip_make_memory(vvp_memory_t mem)
{
      struct __vpiMemory*obj = (struct __vpiMemory*)
	    malloc(sizeof(struct __vpiMemory));

      obj->base.vpi_type = &vpip_memory_rt;
      obj->word.base.vpi_type = &vpip_memory_word_rt;
      obj->mem = mem;
      obj->word.mem = obj;

      return &(obj->base);
}

/*
 * $Log: vpi_memory.cc,v $
 * Revision 1.1  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 */

