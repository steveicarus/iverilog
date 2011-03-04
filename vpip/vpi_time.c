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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_time.c,v 1.2 2002/08/12 01:35:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <stdio.h>

/*
 * IEEE-1364 VPI pretty much mandates the existence of this sort of
 * thing. (Either this or a huge memory leak.) Sorry.
 */
static char buf_obj[128];

static void timevar_get_value(vpiHandle ref, s_vpi_value*vp)
{
      struct __vpiTimeVar*rfp = (struct __vpiTimeVar*)ref;
      assert(ref->vpi_type->type_code == vpiTimeVar);

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiTimeVal:
	    rfp->time_obj.low = rfp->time;
	    vp->value.time = &rfp->time_obj;
	    vp->format = vpiTimeVal;
	    break;

	  case vpiDecStrVal:
	    sprintf(buf_obj, "%lu", rfp->time);
	    vp->value.str = buf_obj;
	    break;

	  default:
	    vp->format = vpiSuppressVal;
	    vp->value.str = 0;
	    break;
      }
}


static const struct __vpirt vpip_time_var_rt = {
      vpiTimeVar,
      0,
      0,
      timevar_get_value,
      0,
      0,
      0
};

vpiHandle vpip_make_time_var(struct __vpiTimeVar*ref, const char*val)
{
      ref->base.vpi_type = &vpip_time_var_rt;
      ref->name = val;
      return &(ref->base);
}

/*
 * $Log: vpi_time.c,v $
 * Revision 1.2  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.3  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

