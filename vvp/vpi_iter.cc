/*
 * Copyright (c) 1999-2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_iter.cc,v 1.2 2001/05/08 23:59:33 steve Exp $"
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
      struct __vpiIterator*res = (struct __vpiIterator*)
	    calloc(1, sizeof(struct __vpiIterator));
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

      if (ref->vpi_type->index_)
	    return (ref->vpi_type->index_(ref, 0));

      if (hp->next == hp->nargs) {
	    vpi_free_object(ref);
	    return 0;
      }

      return hp->args[hp->next++];
}

/*
 * $Log: vpi_iter.cc,v $
 * Revision 1.2  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

