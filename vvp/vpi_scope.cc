/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_scope.cc,v 1.4 2001/04/18 04:21:23 steve Exp $"
#endif

# include  "compile.h"
# include  "vpi_priv.h"
# include  "symbols.h"
# include  <malloc.h>
# include  <assert.h>

static char* scope_get_str(int code, vpiHandle obj)
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
	    return 0;
      }
      return 0;
}

static const struct __vpirt vpip_scope_rt = {
      vpiModule,
      0,
      scope_get_str,
      0,
      0,
      0,
      module_iter
};

static struct __vpiScope*current_scope = 0;

static void attach_to_scope_(struct __vpiScope*scope, vpiHandle obj)
{
      assert(scope);
      unsigned idx = scope->nintern++;

      if (scope->intern == 0)
	    scope->intern = (vpiHandle*)
		  malloc(sizeof(vpiHandle));
      else
	    scope->intern = (vpiHandle*)
		  realloc(scope->intern, sizeof(vpiHandle)*scope->nintern);

      scope->intern[idx] = obj;
}

/*
 * When the compiler encounters a scope declaration, this function
 * creates and initializes a __vpiScope object with the requested name
 * and within the addressed parent. The label is used as a key in the
 * symbol table and the name is used to construct the actual object.
 */
void compile_scope_decl(char*label, char*name, char*parent)
{
      struct __vpiScope*scope = new struct __vpiScope;
      scope->base.vpi_type = &vpip_scope_rt;
      scope->name = name;
      scope->intern = 0;
      scope->nintern = 0;
      scope->threads = 0;

      current_scope = scope;

      compile_vpi_symbol(label, &scope->base);
      free(label);

      if (parent) {
	    vpiHandle obj = compile_vpi_lookup(parent);
	    struct __vpiScope*sp = (struct __vpiScope*) obj;
	    attach_to_scope_(sp, &scope->base);
	    free(parent);
      }
}

void compile_scope_recall(char*symbol)
{
      vpiHandle obj = compile_vpi_lookup(symbol);
      current_scope = (struct __vpiScope*)obj;
      free(symbol);
}

struct __vpiScope* vpip_peek_current_scope(void)
{
      return current_scope;
}

void vpip_attach_to_current_scope(vpiHandle obj)
{
      attach_to_scope_(current_scope, obj);
}


/*
 * $Log: vpi_scope.cc,v $
 * Revision 1.4  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.3  2001/04/03 03:46:14  steve
 *  VPI access time as a decimal string, and
 *  stub vpi access to the scopes.
 *
 * Revision 1.2  2001/03/21 05:13:03  steve
 *  Allow var objects as vpiHandle arguments to %vpi_call.
 *
 * Revision 1.1  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 */

