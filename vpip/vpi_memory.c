/*
 * Copyright (c) 1999-2000 Picture Elements, Inc.
 *    Stephen Williams (steve@picturel.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_memory.c,v 1.3 2002/08/12 01:35:05 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static int memory_get(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;

      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiSize:
	    return rfp->size;

	  default:
	    return 0;
      }
}

static const char* memory_get_str(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {

	  case vpiFullName:
	    return (char*)rfp->name;
      }

      return 0;
}

static vpiHandle memory_iterate(int code, vpiHandle ref)
{
      unsigned idx;
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {
	  case vpiMemoryWord:
	    if (rfp->args == 0) {
		  rfp->args = calloc(rfp->size, sizeof(vpiHandle));
		  for (idx = 0 ;  idx < rfp->size ;  idx += 1)
			rfp->args[idx] = &rfp->words[idx].base;
	    }
	    return vpip_make_iterator(rfp->size, rfp->args);

	  default:
	    return 0;
      }
}

static vpiHandle memory_index(vpiHandle ref, int index)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;
      assert(ref->vpi_type->type_code==vpiMemory);

      if (rfp->args == 0) {
	    unsigned idx;
	    rfp->args = calloc(rfp->size, sizeof(vpiHandle));
	    for (idx = 0 ;  idx < rfp->size ;  idx += 1)
		  rfp->args[idx] = &rfp->words[idx].base;
      }

      if (index > rfp->size) return 0;
      if (index < 0) return 0;
      return &(rfp->words[index].base);
}

static int memory_word_get(int code, vpiHandle ref)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      switch (code) {
	  case vpiSize:
	    return rfp->mem->width;

	  default:
	    return 0;
      }
}

static vpiHandle memory_word_put(vpiHandle ref, p_vpi_value val,
				 p_vpi_time tim, int flags)
{
      unsigned idx;
      vpip_bit_t*base;
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      base = rfp->mem->bits + rfp->index*rfp->mem->width;

      assert(val->format == vpiVectorVal);
      for (idx = 0 ;  idx < rfp->mem->width ;  idx += 1) {
	    p_vpi_vecval cur = val->value.vector + (idx/32);
	    int aval = cur->aval >> (idx%32);
	    int bval = cur->bval >> (idx%32);

	    if (bval & 1) {
		  if (aval & 1)
			*base = StX;
		  else
			*base = HiZ;
	    } else {
		  if (aval & 1)
			*base = St1;
		  else
			*base = St0;
	    }
	    base += 1;
      }
      return 0;
}

static void memory_word_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiMemoryWord*rfp = (struct __vpiMemoryWord*)ref;
      assert(ref->vpi_type->type_code==vpiMemoryWord);

      vpip_bits_get_value(rfp->mem->bits+rfp->index*rfp->mem->width,
			  rfp->mem->width, vp, 0);
}

static const struct __vpirt vpip_memory_rt = {
      vpiMemory,
      memory_get,
      memory_get_str,
      0,
      0,
      0,
      memory_iterate,
      memory_index
};

static const struct __vpirt vpip_memory_word_rt = {
      vpiMemoryWord,
      memory_word_get,
      0,
      memory_word_get_value,
      memory_word_put,
      0,
      0,
      0
};

vpiHandle vpip_make_memory(struct __vpiMemory*ref, const char*name,
			   unsigned wid, unsigned siz)
{
      unsigned idx;

      ref->base.vpi_type = &vpip_memory_rt;
      ref->name = name;
      ref->bits = calloc(wid*siz, sizeof(vpip_bit_t));
      for (idx = 0 ;  idx < wid*siz ;  idx += 1)
	    ref->bits[idx] = StX;
      ref->words = calloc(siz, sizeof(struct __vpiMemoryWord));
      ref->args = 0;
      ref->width = wid;
      ref->size  = siz;

      for (idx = 0 ;  idx < siz ;  idx += 1) {
	    ref->words[idx].base.vpi_type = &vpip_memory_word_rt;
	    ref->words[idx].mem = ref;
	    ref->words[idx].index = idx;
      }

      return &(ref->base);
}
/*
 * $Log: vpi_memory.c,v $
 * Revision 1.3  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/10/26 02:29:10  steve
 *  const/non-const warnings. (Stephan Boettcher)
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.9  2001/01/06 22:22:17  steve
 *  Support signed decimal display of variables.
 *
 * Revision 1.8  2000/06/28 18:38:00  steve
 *  Initialize memories as they are create.
 *
 * Revision 1.7  2000/03/22 04:26:41  steve
 *  Replace the vpip_bit_t with a typedef and
 *  define values for all the different bit
 *  values, including strengths.
 *
 * Revision 1.6  2000/02/29 01:41:32  steve
 *  Fix warning and typo.
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.4  2000/02/13 19:18:28  steve
 *  Accept memory words as parameter to $display.
 *
 * Revision 1.3  1999/12/15 04:15:17  steve
 *  Implement vpi_put_value for memory words.
 *
 * Revision 1.2  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.1  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 */

