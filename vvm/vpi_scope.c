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
#ident "$Id: vpi_scope.c,v 1.1 1999/11/27 19:07:58 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>

static const struct __vpirt vpip_module_rt = {
      vpiModule,
      0,
      0,
      0,
      0,
      0
};

vpiHandle vpip_make_scope(struct __vpiScope*ref, int type, const char*name)
{
      switch (type) {
	  case vpiModule:
	    ref->base.vpi_type = &vpip_module_rt;
	    break;
	  default:
	    assert(0);
      }

      return &ref->base;
}

/*
 * $Log: vpi_scope.c,v $
 * Revision 1.1  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 */

