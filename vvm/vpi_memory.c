/*
 * Copyright (c) 1999 Picture Elements, Inc.
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
#if !defined(WINNT)
#ident "$Id: vpi_memory.c,v 1.1 1999/11/10 02:52:24 steve Exp $"
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

static char* memory_get_str(int code, vpiHandle ref)
{
      struct __vpiMemory*rfp = (struct __vpiMemory*)ref;

      assert(ref->vpi_type->type_code==vpiMemory);

      switch (code) {

	  case vpiFullName:
	    return (char*)rfp->name;
      }

      return 0;
}

static const struct __vpirt vpip_memory_rt = {
      vpiMemory,
      memory_get,
      memory_get_str,
      0,
      0,
      0
};

vpiHandle vpip_make_memory(struct __vpiMemory*ref, const char*name,
			   unsigned wid, unsigned siz)
{
      ref->base.vpi_type = &vpip_memory_rt;
      ref->name = name;
      ref->bits = calloc(wid*siz, sizeof(enum vpip_bit_t));
      ref->width = wid;
      ref->size  = siz;
      return &(ref->base);
}
/*
 * $Log: vpi_memory.c,v $
 * Revision 1.1  1999/11/10 02:52:24  steve
 *  Create the vpiMemory handle type.
 *
 */

