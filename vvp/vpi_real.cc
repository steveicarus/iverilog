/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_real.cc,v 1.1 2003/01/25 23:48:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdio.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <assert.h>


struct __vpiRealVar {
      struct __vpiHandle base;
      const char*name;
      double value;
};

static void real_var_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      switch (vp->format) {
	  case vpiObjTypeVal:
	    vp->format = vpiRealVal;
	  case vpiRealVal:
	    vp->value.real = rfp->value;
	    break;
	  default:
	    fprintf(stderr, "ERROR: Unsupported format code: %d\n",
		    vp->format);
	    assert(0);
      }
}

static vpiHandle real_var_put_value(vpiHandle ref, p_vpi_value vp,
				    p_vpi_time, int)
{
      assert(ref->vpi_type->type_code == vpiRealVar);

      struct __vpiRealVar*rfp = (struct __vpiRealVar*)ref;

      assert(vp->format == vpiRealVal);
      rfp->value = vp->value.real;
      return 0;
}

static int real_var_free_object(vpiHandle)
{
      return 0;
}

static const struct __vpirt vpip_real_var_rt = {
      vpiRealVar,

      0,
      0,
      &real_var_get_value,
      &real_var_put_value,

      0,
      0,
      0,

      real_var_free_object
};

vpiHandle vpip_make_real_var(const char*name)
{
      struct __vpiRealVar*obj = (struct __vpiRealVar*)
	    malloc(sizeof(struct __vpiRealVar));

      obj->base.vpi_type = &vpip_real_var_rt;
      obj->name = vpip_string(name);
      obj->value = 0.0;

      return &obj->base;
}

/*
 * $Log: vpi_real.cc,v $
 * Revision 1.1  2003/01/25 23:48:06  steve
 *  Add thread word array, and add the instructions,
 *  %add/wr, %cmp/wr, %load/wr, %mul/wr and %set/wr.
 *
 */

