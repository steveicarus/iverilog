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
#ident "$Id: vpi_systask.c,v 1.5 2002/08/12 01:35:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static vpiHandle systask_handle(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      switch (type) {
	  case vpiScope:
	    return &rfp->scope->base;
	  default:
	    assert(0);
	    return 0;
      }
}

/*
 * the iter function only supports getting an iterator of the
 * arguments. This works equally well for tasks and functions.
 */
static vpiHandle systask_iter(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert((ref->vpi_type->type_code == vpiSysTaskCall)
	     || (ref->vpi_type->type_code == vpiSysFuncCall));

      if (rfp->nargs == 0)
	    return 0;

      return vpip_make_iterator(rfp->nargs, rfp->args);
}

static const struct __vpirt vpip_systask_rt = {
      vpiSysTaskCall,
      0,
      0,
      0,
      0,
      systask_handle,
      systask_iter
};

const struct __vpirt *vpip_get_systask_rt(void)
{
    return &vpip_systask_rt;
}

/*
 * A value *can* be put to a vpiSysFuncCall object. This is how the
 * return value is set. The value that is given should be converted to
 * bits and set into the return value bit array.
 */
static vpiHandle sysfunc_put_value(vpiHandle ref, p_vpi_value val,
				   p_vpi_time t, int flag)
{
      long tmp;
      int idx;

      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert(ref->vpi_type->type_code == vpiSysFuncCall);

	/* There *must* be a return value array. */
      assert(rfp->res);
      assert(rfp->nres > 0);

	/* XXXX For now, only support very specific formats. */
      assert(val->format == vpiIntVal);

	/* This fills the result bits with the signed value of the
	   integer. This automagically extends the sign bit by nature
	   of how the >> works in C. */
      tmp = val->value.integer;
      for (idx = 0 ;  idx < rfp->nres ;  idx += 1) {
	    rfp->res[idx] =  (tmp&1) ? St1 : St0;
	    tmp >>= 1;
      }

      return 0;
}

static const struct __vpirt vpip_sysfunc_rt = {
      vpiSysFuncCall,
      0,
      0,
      0,
      sysfunc_put_value,
      0,
      systask_iter
};

const struct __vpirt *vpip_get_sysfunc_rt(void)
{
    return &vpip_sysfunc_rt;
}

/*
 * $Log: vpi_systask.c,v $
 * Revision 1.5  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2001/09/30 16:45:10  steve
 *  Fix some Cygwin DLL handling. (Venkat Iyer)
 *
 * Revision 1.3  2001/05/22 02:14:47  steve
 *  Update the mingw build to not require cygwin files.
 *
 * Revision 1.2  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.9  2000/11/01 03:19:36  steve
 *  Add the general $time system function.
 *
 * Revision 1.8  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.7  2000/10/06 23:11:39  steve
 *  Replace data references with function calls. (Venkat)
 *
 * Revision 1.6  2000/09/30 03:20:48  steve
 *  Cygwin port changes from Venkat
 *
 * Revision 1.5  2000/05/07 18:20:08  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.4  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
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

