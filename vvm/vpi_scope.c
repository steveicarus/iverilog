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
#ident "$Id: vpi_scope.c,v 1.4 1999/12/15 18:21:20 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static vpiHandle module_iter(int code, vpiHandle obj)
{
      struct __vpiScope*ref = (struct __vpiScope*)obj;
      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiNamedBegin));

      switch (code) {
	  case vpiInternalScope:
	    return vpip_make_iterator(ref->nintern, ref->intern);
      }
      return 0;
}

static const struct __vpirt vpip_module_rt = {
      vpiModule,
      0,
      0,
      0,
      0,
      0,
      module_iter
};

static const struct __vpirt vpip_named_begin_rt = {
      vpiNamedBegin,
      0,
      0,
      0,
      0,
      0,
      module_iter
};

vpiHandle vpip_make_scope(struct __vpiScope*ref, int type, const char*name)
{
      ref->intern = 0;
      ref->nintern = 0;

      switch (type) {
	  case vpiModule:
	    ref->base.vpi_type = &vpip_module_rt;
	    break;
	  case vpiNamedBegin:
	    ref->base.vpi_type = &vpip_named_begin_rt;
	    break;
	  default:
	    assert(0);
      }

      return &ref->base;
}

void vpip_attach_to_scope(struct __vpiScope*ref, vpiHandle obj)
{
      unsigned idx = ref->nintern++;

      if (ref->intern == 0)
	    ref->intern = malloc(sizeof(vpiHandle));
      else
	    ref->intern = realloc(ref->intern, sizeof(vpiHandle)*ref->nintern);

      ref->intern[idx] = obj;
}

/*
 * $Log: vpi_scope.c,v $
 * Revision 1.4  1999/12/15 18:21:20  steve
 *  Support named begin scope at run time.
 *
 * Revision 1.3  1999/12/15 04:01:14  steve
 *  Add the VPI implementation of $readmemh.
 *
 * Revision 1.2  1999/11/28 00:56:08  steve
 *  Build up the lists in the scope of a module,
 *  and get $dumpvars to scan the scope for items.
 *
 * Revision 1.1  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 */

