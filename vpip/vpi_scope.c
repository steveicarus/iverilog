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
#ident "$Id: vpi_scope.c,v 1.3 2002/08/12 01:35:06 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <stdlib.h>
# include  <assert.h>

static const char* scope_get_str(int code, vpiHandle obj)
{
      struct __vpiScope*ref = (struct __vpiScope*)obj;


      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiNamedBegin)
	     || (obj->vpi_type->type_code == vpiTask));

      switch (code) {
	  case vpiFullName:
	    return ref->name;
	  default:
	    assert(0);
	    return 0;
      }
}

static vpiHandle module_iter(int code, vpiHandle obj)
{
      struct __vpiScope*ref = (struct __vpiScope*)obj;
      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiNamedBegin)
	     || (obj->vpi_type->type_code == vpiTask)
	     || (obj->vpi_type->type_code == vpiFunction));

      switch (code) {
	  case vpiInternalScope:
	    return vpip_make_iterator(ref->nintern, ref->intern);
      }
      return 0;
}

static const struct __vpirt vpip_module_rt = {
      vpiModule,
      0,
      scope_get_str,
      0,
      0,
      0,
      module_iter
};

static const struct __vpirt vpip_task_rt = {
      vpiTask,
      0,
      scope_get_str,
      0,
      0,
      0,
      module_iter
};

static const struct __vpirt vpip_function_rt = {
      vpiFunction,
      0,
      scope_get_str,
      0,
      0,
      0,
      module_iter
};

static const struct __vpirt vpip_named_begin_rt = {
      vpiNamedBegin,
      0,
      scope_get_str,
      0,
      0,
      0,
      module_iter
};

static const struct __vpirt vpip_named_fork_rt = {
      vpiNamedFork,
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
      ref->name = name;

      switch (type) {
	  case vpiModule:
	    ref->base.vpi_type = &vpip_module_rt;
	    break;
	  case vpiNamedBegin:
	    ref->base.vpi_type = &vpip_named_begin_rt;
	    break;
	  case vpiNamedFork:
	    ref->base.vpi_type = &vpip_named_fork_rt;
	    break;
	  case vpiTask:
	    ref->base.vpi_type = &vpip_task_rt;
	    break;
	  case vpiFunction:
	    ref->base.vpi_type = &vpip_function_rt;
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
 * Revision 1.3  2002/08/12 01:35:06  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/10/21 23:37:49  steve
 *  Kill const-nonconst warning.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.11  2001/01/01 08:10:35  steve
 *  Handle function scopes in dumpvars scn (PR#95)
 *
 * Revision 1.10  2000/11/01 06:05:44  steve
 *  VCD scans tasks (PR#35)
 *
 * Revision 1.9  2000/10/29 17:10:02  steve
 *  task threads ned their scope initialized. (PR#32)
 *
 * Revision 1.8  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.7  2000/05/03 05:03:26  steve
 *  Support named for in VPI.
 *
 * Revision 1.6  2000/03/08 04:36:54  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
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

