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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: vpi_systask.c,v 1.3 2000/02/23 02:56:56 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static vpiHandle systask_iter(int type, vpiHandle ref)
{
      struct __vpiSysTaskCall*rfp = (struct __vpiSysTaskCall*)ref;
      assert(ref->vpi_type->type_code == vpiSysTaskCall);

      if (rfp->nargs == 0)
	    return 0;

      return vpip_make_iterator(rfp->nargs, rfp->args);
}

const struct __vpirt vpip_systask_rt = {
      vpiSysTaskCall,
      0,
      0,
      0,
      0,
      0,
      systask_iter
};

/*
 * $Log: vpi_systask.c,v $
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

