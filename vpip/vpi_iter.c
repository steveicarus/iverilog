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
#ident "$Id: vpi_iter.c,v 1.2 2002/08/12 01:35:05 steve Exp $"
#endif

/*
 * Find here the methods functions in support of iterator objects.
 */

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static const struct __vpirt vpip_iterator_rt = {
      vpiIterator,
      0,
      0,
      0,
      0,
      0,
      0
};

vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args)
{
      struct __vpiIterator*res = calloc(1, sizeof(struct __vpiIterator));
      res->base.vpi_type = &vpip_iterator_rt;
      res->args = args;
      res->nargs = nargs;
      res->next  = 0;

      return &(res->base);
}

/*
 * The vpi_scan function only applies to iterators. It returns the
 * next vpiHandle in the iterated list.
 */
vpiHandle vpi_scan(vpiHandle ref)
{
      struct __vpiIterator*hp = (struct __vpiIterator*)ref;
      assert(ref->vpi_type->type_code == vpiIterator);

      if (hp->next == hp->nargs) {
	    vpi_free_object(ref);
	    return 0;
      }

      return hp->args[hp->next++];
}

/*
 * $Log: vpi_iter.c,v $
 * Revision 1.2  2002/08/12 01:35:05  steve
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

