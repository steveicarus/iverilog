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
#ident "$Id: vpi_memory.cc,v 1.6 2002/05/03 15:44:11 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "memory.h"
# include  <stdlib.h>
# include  <assert.h>

extern const char hex_digits[256];
static char buf[4096];

struct __vpiMemoryWord {
      struct __vpiHandle base;
      struct __vpiMemory*mem;
      int index;
      struct __vpiDecConst*left_range;
      struct __vpiDecConst*right_range;
};

struct __vpiMemory {
      struct __vpiHandle base;
	/* The signal has a name (this points to static memory.) */
      struct __vpiMemoryWord word;
      vvp_memory_t mem;
      struct __vpiDecConst*left_range;
      struct __vpiDecConst*right_range;
    
};

static vpiHandle memory_get_handle(int code, vpiHandle obj)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)obj;

      assert(obj->vpi_type->type_code==vpiMemory);

      switch(code){
      case vpiLeftRange:
	  return &(rfp->left_range->base);

      case vpiRightRange:
	  return &(rfp->right_range->base);
      }

      return 0;
}

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

static int mem_iter_free_object(vpiHandle ref)
{
      free(ref);
      return 0;
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

//==============================

static vpiHandle memory_word_get_handle(int code, vpiHandle obj)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)obj;

      assert(obj->vpi_type->type_code==vpiMemoryWord);

      switch(code){
      case vpiLeftRange:
	  return &(rfp->left_range->base);

      case vpiRightRange:
	  return &(rfp->right_range->base);
      }

      return 0;
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


	/* Get the width of the memory, and the byte index of the
	   first byte of the word. */
      unsigned width = memory_data_width(rfp->mem->mem);
      unsigned bidx = rfp->index * ((width+3)&~3);

      switch (val->format) {

	  case vpiVectorVal:
	    for (unsigned widx = 0;  widx < width;  widx += 32) {
		  p_vpi_vecval cur = val->value.vector + (widx/32);
		  for (unsigned idx = widx
			     ; idx < width && idx < widx+32
			     ; idx += 1) {
			int aval = (cur->aval >> (idx%32)) & 1;
			int bval = (cur->bval >> (idx%32)) & 1;
			unsigned char val = (bval<<1) | (aval^bval);
			memory_set(rfp->mem->mem, bidx+idx, val);
		  }
	    }
	    break;

	  case vpiIntVal:
	    for (unsigned widx = 0;  widx < width;  widx += 32) {
		  int cur = val->value.integer;
		  for (unsigned idx = widx
			     ; idx < width && idx < widx+32
			     ; idx += 1) {
			unsigned char val = (cur&1)? 1 : 0;
			memory_set(rfp->mem->mem, bidx+idx, val);
			cur >>= 1;
		  }
	    }
	    break;

	  default:
	    assert(0);
      }

      return 0;
}

static void memory_word_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(rfp->base.vpi_type->type_code==vpiMemoryWord);
      
      unsigned width = memory_data_width(rfp->mem->mem);
      unsigned bidx = rfp->index * ((width+3)&~3);

      switch (vp->format) {
	  default:
	    assert(!"not implemented");

          case vpiBinStrVal:
	      assert(width < sizeof(buf));
	      for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		  unsigned bit = memory_get(rfp->mem->mem, bidx+idx);
		  
		  buf[width-idx-1] = "01xz"[bit];
	      }
	      buf[width] = 0;
	      vp->value.str = buf;
	      break;

	  case vpiHexStrVal: {
		unsigned hval, hwid;
		hwid = (width + 3) / 4;

		assert(hwid < sizeof(buf) );
		buf[hwid] = 0;

		hval = 0;
		for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
		    unsigned bit = memory_get(rfp->mem->mem, bidx+idx);
		    hval = hval | (bit << 2*(idx % 4));

		    if (idx%4 == 3) {
			hwid -= 1;
			buf[hwid] = hex_digits[hval];
			hval = 0;
		    }
		}

		if (hwid > 0) {
		    unsigned padd = 0;

		    hwid -= 1;
		    buf[hwid] = hex_digits[hval];
		    switch(buf[hwid]) {
		    case 'X': padd = 2; break;
		    case 'Z': padd = 3; break;
		    }
		    if (padd) {
			for (unsigned idx = width % 4; idx < 4; idx += 1) {
			    hval = hval | padd << 2*idx;
			}
			buf[hwid] = hex_digits[hval];
		    }
		}
		vp->value.str = buf;
		break;
	  }

      	  case vpiIntVal:
	    vp->value.integer = 0;
	    for (unsigned idx = 0;  idx < width;  idx += 1) {

		  unsigned bit = memory_get(rfp->mem->mem, bidx+idx);
		  if (bit>1) {
			vp->value.integer = 0;
			break;
		  }
			
		  vp->value.integer |= bit << idx;
	    }
      }
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
      0,
      memory_word_get_value,
      memory_word_put,
      memory_word_get_handle,
      0,
      0,
};

vpiHandle vpip_make_memory(vvp_memory_t mem)
{
      struct __vpiMemory*obj = (struct __vpiMemory*)
	    malloc(sizeof(struct __vpiMemory));

      obj->base.vpi_type = &vpip_memory_rt;
      obj->mem = mem;
      obj->left_range = (struct __vpiDecConst*)vpip_make_dec_const(memory_left_range(mem));
      obj->right_range = (struct __vpiDecConst*)vpip_make_dec_const(memory_right_range(mem));

      obj->word.base.vpi_type = &vpip_memory_word_rt;
      obj->word.mem = obj;
      obj->word.left_range = (struct __vpiDecConst*)vpip_make_dec_const(memory_word_left_range(mem));
      obj->word.right_range = (struct __vpiDecConst*)vpip_make_dec_const(memory_word_right_range(mem));

      return &(obj->base);
}

/*
 * $Log: vpi_memory.cc,v $
 * Revision 1.6  2002/05/03 15:44:11  steve
 *  Add vpiModule iterator to vpiScope objects.
 *
 * Revision 1.5  2002/02/06 04:48:34  steve
 *  get bin or hex string values of memory words.
 *
 * Revision 1.4  2002/01/31 04:28:17  steve
 *  Full support for $readmem ranges (Tom Verbeure)
 *
 * Revision 1.3  2001/12/07 23:23:05  steve
 *  vpi_put_value of vpiIntVal for memory words.
 *
 * Revision 1.2  2001/11/09 03:39:07  steve
 *  Support vpiIntVal from memory.
 *
 * Revision 1.1  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 */

