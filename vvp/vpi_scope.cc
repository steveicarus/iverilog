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
#ident "$Id: vpi_scope.cc,v 1.16 2002/07/05 17:14:15 steve Exp $"
#endif

# include  "compile.h"
# include  "vpi_priv.h"
# include  "symbols.h"
# include  "functor.h"
# include  "statistics.h"
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <assert.h>

static vpiHandle *vpip_root_table_ptr = 0;
static unsigned   vpip_root_table_cnt = 0;

vpiHandle vpip_make_root_iterator(void)
{
      assert(vpip_root_table_ptr);
      assert(vpip_root_table_cnt);
      return vpip_make_iterator(vpip_root_table_cnt,
				vpip_root_table_ptr, false);
}

static char* scope_get_str(int code, vpiHandle obj)
{
      struct __vpiScope*ref = (struct __vpiScope*)obj;

      char *n, *nn;

      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiFunction)
	     || (obj->vpi_type->type_code == vpiTask)
	     || (obj->vpi_type->type_code == vpiNamedBegin)
	     || (obj->vpi_type->type_code == vpiNamedFork));

      switch (code) {
	  case vpiFullName:
	    return const_cast<char*>(ref->name);

	  case vpiName:
	    nn = n = const_cast<char*>(ref->name);
	    while (*n)
		  if (*n=='\\' && *++n)
			++n;
		  else if (*n=='.')
			nn = ++n;
		  else 
			++n;
	    return nn;
		  
	  default:
	    assert(0);
	    return 0;
      }
}

static vpiHandle scope_get_handle(int code, vpiHandle obj)
{
      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiFunction)
	     || (obj->vpi_type->type_code == vpiTask)
	     || (obj->vpi_type->type_code == vpiNamedBegin)
	     || (obj->vpi_type->type_code == vpiNamedFork));

      struct __vpiScope*rfp = (struct __vpiScope*)obj;

      switch (code) {

	  case vpiScope:
	    return &rfp->scope->base;
      }

      return 0;
}

static vpiHandle module_iter_subset(int code, struct __vpiScope*ref)
{
      unsigned mcnt = 0, ncnt = 0;
      vpiHandle*args;

      for (unsigned idx = 0 ;  idx < ref->nintern ;  idx += 1)
	    if (ref->intern[idx]->vpi_type->type_code == code)
		  mcnt += 1;

      if (mcnt == 0)
	    return 0;

      args = (vpiHandle*)calloc(mcnt, sizeof(vpiHandle));
      for (unsigned idx = 0 ;  idx < ref->nintern ;  idx += 1)
	    if (ref->intern[idx]->vpi_type->type_code == code)
		  args[ncnt++] = ref->intern[idx];

      assert(ncnt == mcnt);

      return vpip_make_iterator(mcnt, args, true);
}

/*
 * This function implements the vpi_iterate method for vpiModule and
 * similar objects. The vpi_iterate allows the user to iterate over
 * things that are contained in the scope object, by generating an
 * iterator for the requested set of items.
 */
static vpiHandle module_iter(int code, vpiHandle obj)
{
      struct __vpiScope*ref = (struct __vpiScope*)obj;
      assert((obj->vpi_type->type_code == vpiModule)
	     || (obj->vpi_type->type_code == vpiFunction)
	     || (obj->vpi_type->type_code == vpiTask)
	     || (obj->vpi_type->type_code == vpiNamedBegin)
	     || (obj->vpi_type->type_code == vpiNamedFork));

      switch (code) {
	  case vpiInternalScope:
	    return vpip_make_iterator(ref->nintern, ref->intern, false);

	      /* Generate and iterator for all the modules contained
		 in this scope. */
	  case vpiMemory:
	  case vpiModule:
	  case vpiNamedEvent:
	  case vpiNet:
	  case vpiReg:
	    return module_iter_subset(code, ref);

      }
      return 0;
}

/*
**  Keeping track of functor scope.  When the scope changes during
**  compilation, we record the current number of functors in a list.
**
**  Why are we doing this?  The SDF annotator needs this for
**  INTERCONNECT delays.  The INTERCONNECT delay is specified between
**  a source modules output port and a target module input port, which
**  are connected with a wire.  The vpiSignal for both ports point to
**  the same functor output, together with all other ports that may be
**  connected to the same wire.  The SDF annotator need to find those
**  functors which are inside the scope of the target module, which
**  are driven by the source functor.  And even this is only an
**  aproximation, in case the wire is connected to multiple inputs of
**  the same module.  But those should have the same delays anyway.
**
*/

struct functor_scope_s {
      vpiHandle scope;
      unsigned start;
};

static struct functor_scope_s * functor_scopes = 0;
static unsigned n_functor_scopes = 0;
static unsigned a_functor_scopes = 0;

void functor_set_scope(vpiHandle scope)
{
      unsigned nfun = functor_limit();

      if (n_functor_scopes) {
	    functor_scope_s *last = &functor_scopes[n_functor_scopes - 1];

	    if (last->scope == scope) {
		  return;
	    }
	    
	    if (last->start == nfun) {
		  last->scope = scope;
		  return;
	    }
      }
      
      n_functor_scopes += 1;
      if (n_functor_scopes >= a_functor_scopes) {
	    a_functor_scopes += 512;
	    functor_scopes = (struct functor_scope_s *)
		  realloc(functor_scopes, 
			  a_functor_scopes*sizeof(struct functor_scope_s));
	    assert(functor_scopes);
      }

      functor_scope_s *last = &functor_scopes[n_functor_scopes - 1];
      last->start = nfun;
      last->scope = scope;
}

/*
**  Lockup the scope of a functor.
**
**  Cannot use bserach, since we are not looking for an exact match
*/
vpiHandle ipoint_get_scope(vvp_ipoint_t ipt)
{
      if (n_functor_scopes == 0)
	    return NULL;

      unsigned fidx = ipt/4;
      
      unsigned first = 0;
      unsigned last = n_functor_scopes;
      while (first < last) {
	    unsigned next = (first+last)/2;
	    functor_scope_s *cur = &functor_scopes[next];

	    if (cur->start > fidx)
		  last = next;
	    else if (next == first)
		  break;
	    else 
		  first = next;
      }

      functor_scope_s *cur = &functor_scopes[first];
      return cur->scope;
}

static const struct __vpirt vpip_scope_module_rt = {
      vpiModule,
      0,
      scope_get_str,
      0,
      0,
      scope_get_handle,
      module_iter
};

static const struct __vpirt vpip_scope_task_rt = {
      vpiTask,
      0,
      scope_get_str,
      0,
      0,
      scope_get_handle,
      module_iter
};

static const struct __vpirt vpip_scope_function_rt = {
      vpiFunction,
      0,
      scope_get_str,
      0,
      0,
      scope_get_handle,
      module_iter
};

static const struct __vpirt vpip_scope_begin_rt = {
      vpiNamedBegin,
      0,
      scope_get_str,
      0,
      0,
      scope_get_handle,
      module_iter
};

static const struct __vpirt vpip_scope_fork_rt = {
      vpiNamedFork,
      0,
      scope_get_str,
      0,
      0,
      scope_get_handle,
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
void compile_scope_decl(char*label, char*type, char*name, char*parent)
{
      struct __vpiScope*scope = new struct __vpiScope;
      count_vpi_scopes += 1;

      switch(type[2]) {
	  case 'd': /* type == moDule */
	    scope->base.vpi_type = &vpip_scope_module_rt;
	    break;
	  case 'n': /* type == fuNction */
	    scope->base.vpi_type = &vpip_scope_function_rt;
	    break;
	  case 's': /* type == taSk */
	    scope->base.vpi_type = &vpip_scope_task_rt;
	    break;
	  case 'r': /* type == foRk */
	    scope->base.vpi_type = &vpip_scope_fork_rt;
	    break;
	  case 'g': /* type == beGin */
	    scope->base.vpi_type = &vpip_scope_begin_rt;
	    break;
	  default:
	    scope->base.vpi_type = &vpip_scope_module_rt;
	    assert(0);
      }

      assert(scope->base.vpi_type);

      scope->name = vpip_string(name);
      scope->intern = 0;
      scope->nintern = 0;
      scope->threads = 0;

      current_scope = scope;

      compile_vpi_symbol(label, &scope->base);

      free(label);
      free(type);
      free(name);

      if (parent) {
	    static vpiHandle obj;
	    compile_vpi_lookup(&obj, parent);
	    assert(obj);
	    struct __vpiScope*sp = (struct __vpiScope*) obj;
	    attach_to_scope_(sp, &scope->base);
	    scope->scope = (struct __vpiScope*)obj;
      } else {
	    scope->scope = 0x0;

	    unsigned cnt = vpip_root_table_cnt + 1;
	    vpip_root_table_ptr = (vpiHandle*)
		  realloc(vpip_root_table_ptr, cnt * sizeof(vpiHandle));
	    vpip_root_table_ptr[vpip_root_table_cnt] = &scope->base;
	    vpip_root_table_cnt = cnt;
      }

      functor_set_scope(&current_scope->base);
}

void compile_scope_recall(char*symbol)
{
      compile_vpi_lookup((vpiHandle*)&current_scope, symbol);
      assert(current_scope);
      functor_set_scope(&current_scope->base);
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
 * Revision 1.16  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.15  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.14  2002/05/10 16:00:57  steve
 *  Support scope iterate over vpiNet,vpiReg/vpiMemory.
 *
 * Revision 1.13  2002/05/03 15:44:11  steve
 *  Add vpiModule iterator to vpiScope objects.
 *
 * Revision 1.12  2002/01/06 17:50:50  steve
 *  Support scope for functors. (Stephan Boettcher)
 *
 * Revision 1.11  2002/01/06 00:48:39  steve
 *  VPI access to root module scopes.
 *
 * Revision 1.10  2001/11/02 05:43:11  steve
 *  Comment the scope type parser.
 *
 * Revision 1.9  2001/10/15 02:58:27  steve
 *  Carry the type of the scope (Stephan Boettcher)
 *
 * Revision 1.8  2001/10/15 01:49:50  steve
 *  Support getting scope of scope, and scope of signals.
 *
 * Revision 1.7  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.6  2001/07/11 04:43:57  steve
 *  support postpone of $systask parameters. (Stephan Boettcher)
 */

