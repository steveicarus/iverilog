/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vpi_const.c,v 1.2 1999/11/06 16:00:18 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>


static void string_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiStringVal:
	    vp->value.str = rfp->value;
	    vp->format = vpiStringVal;
	    break;

	  default:
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static void number_value(vpiHandle ref, p_vpi_value vp)
{
      struct __vpiStringConst*rfp = (struct __vpiStringConst*)ref;
      assert(ref->vpi_type->type_code == vpiConstant);

      switch (vp->format) {

	  default:
	    vp->format = vpiSuppressVal;
	    break;
      }
}

static const struct __vpirt vpip_string_rt = {
      vpiConstant,
      0,
      0,
      string_value,
      0,
      0
};

static const struct __vpirt vpip_number_rt = {
      vpiConstant,
      0,
      0,
      number_value,
      0,
      0
};

vpiHandle vpip_make_string_const(struct __vpiStringConst*ref, const char*val)
{
      ref->base.vpi_type = &vpip_string_rt;
      ref->value = val;
      return &(ref->base);
}

vpiHandle vpip_make_number_const(struct __vpiNumberConst*ref,
				 const enum vpip_bit_t*bits,
				 unsigned nbits)
{
      ref->base.vpi_type = &vpip_number_rt;
      ref->bits = bits;
      ref->nbits = nbits;
      return &(ref->base);
}

/*
 * $Log: vpi_const.c,v $
 * Revision 1.2  1999/11/06 16:00:18  steve
 *  Put number constants into a static table.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

