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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_iter.cc,v 1.7 2003/02/25 01:17:28 steve Exp $"
#endif

/*
 * Find here the methods functions in support of iterator objects.
 */

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static int iterator_free_object(vpiHandle ref)
{
      struct __vpiIterator*hp = (struct __vpiIterator*)ref;
      assert(ref->vpi_type->type_code == vpiIterator);

      if (hp->free_args_flag)
	    free(hp->args);

      free(hp);
      return 1;
}

static const struct __vpirt vpip_iterator_rt = {
      vpiIterator,
      0, // vpi_get_
      0, // vpi_get_str_
      0, // vpi_get_value_
      0, // vpi_put_value_
      0, // handle_
      0, // iterate_
      0, // index_
      &iterator_free_object
};

vpiHandle vpip_make_iterator(unsigned nargs, vpiHandle*args,
			     bool free_args_flag)
{
      struct __vpiIterator*res = (struct __vpiIterator*)
	    calloc(1, sizeof(struct __vpiIterator));
      res->base.vpi_type = &vpip_iterator_rt;
      res->args = args;
      res->nargs = nargs;
      res->next  = 0;

      res->free_args_flag = free_args_flag;

      return &(res->base);
}

/*
 * The vpi_scan function only applies to iterators. It returns the
 * next vpiHandle in the iterated list.
 */
vpiHandle vpi_scan(vpiHandle ref)
{
      if (ref == 0) {
	    vpi_printf("ERROR: NULL handle passed to vpi_scan.\n");
	    assert(0);
	    return 0;
      }

      if (ref->vpi_type->type_code != vpiIterator) {
	    vpi_printf("ERROR: vpi_scan argument is "
		       "inappropriate vpiType code %d\n",
		       ref->vpi_type->type_code);
	    assert(0);
	    return 0;
      }

      struct __vpiIterator*hp = (struct __vpiIterator*)ref;
      assert(ref);
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
 * Revision 1.7  2003/02/25 01:17:28  steve
 *  Some error messages around asserts.
 *
 * Revision 1.6  2003/02/17 00:58:38  steve
 *  Strict correctness of vpi_free_object results.
 *
 * Revision 1.5  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2002/06/02 19:05:50  steve
 *  Check for null pointers from users.
 *
 * Revision 1.3  2002/05/03 15:44:11  steve
 *  Add vpiModule iterator to vpiScope objects.
 *
 * Revision 1.2  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

