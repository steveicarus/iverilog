/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vpi_priv.c,v 1.5 2002/08/12 01:35:05 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  <assert.h>
# include  <stdarg.h>
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>

/*
 * Keep a list of vpi_systf_data structures. This list is searched
 * forward whenever a function is invoked by name, and items are
 * pushed in front of the list whenever they are registered. This
 * allows entries to override older entries.
 */
struct systf_entry {
      struct systf_entry* next;
      s_vpi_systf_data systf_data;
};

static struct systf_entry*systf_func_list = 0;
static struct systf_entry*systf_task_list = 0;

/* This is the handle of the task currently being called. */
static struct __vpiSysTaskCall*vpip_cur_task;

void vpip_calltask(struct __vpiScope*scope, const char*fname,
		   unsigned nparms, vpiHandle*parms)
{
      struct systf_entry*idx;
      struct __vpiSysTaskCall cur_task;
      cur_task.base.vpi_type = vpip_get_systask_rt();
      cur_task.scope = scope;
      cur_task.args  = parms;
      cur_task.nargs = nparms;
      cur_task.res   = 0;
      cur_task.nres  = 0;

      vpip_cur_task = &cur_task;

	/* Look for a systf function to invoke. */
      for (idx = systf_task_list ;  idx ;  idx = idx->next)
	    if (strcmp(fname, idx->systf_data.tfname) == 0) {
		  cur_task.info = &idx->systf_data;
		  idx->systf_data.calltf(idx->systf_data.user_data);
		  return;
	    }


	/* Finally, if nothing is found then something is not
	   right. Print out the function name all the parameters
	   passed, so that someone can deal with it. */
      vpi_printf("Call %s\n", fname);
}

/*
 * System functions are kept in the same sort of table as the system
 * tasks, and we call them in a similar manner.
 */
void vpip_callfunc(const char*fname, unsigned nres, vpip_bit_t*res,
		   unsigned nparms, vpiHandle*parms)
{
      struct systf_entry*idx;
      struct __vpiSysTaskCall cur_task;
      cur_task.base.vpi_type = vpip_get_sysfunc_rt();
      cur_task.args  = parms;
      cur_task.nargs = nparms;
      cur_task.res = res;
      cur_task.nres = nres;

      vpip_cur_task = &cur_task;

	/* Look for a systf function to invoke. */
      for (idx = systf_func_list ;  idx ;  idx = idx->next)
	    if (strcmp(fname, idx->systf_data.tfname) == 0) {
		  cur_task.info = &idx->systf_data;
		  idx->systf_data.calltf(idx->systf_data.user_data);
		  return;
	    }


	/* Finally, if nothing is found then something is not
	   right. Print out the function name all the parameters
	   passed, so that someone can deal with it. */
      vpi_printf("Call %s with width==%u\n", fname, nres);
}


int vpi_free_object(vpiHandle ref)
{
      free(ref);
      return 0;
}

static int vpip_get_global(int property)
{
      switch (property) {
	  case vpiTimePrecision:
	    return vpip_get_simulation_obj()->time_precision;

	  default:
	    assert(0);
	    return -1;
      }
}

int vpi_get(int property, vpiHandle ref)
{
      if (property == vpiType)
	    return ref->vpi_type->type_code;

      if (ref == 0)
	    return vpip_get_global(property);

      if (ref->vpi_type->vpi_get_ == 0)
	    return -1;

      return (ref->vpi_type->vpi_get_)(property, ref);
}

char* vpi_get_str(int property, vpiHandle ref)
{
      if (ref->vpi_type->vpi_get_str_ == 0)
	    return 0;

      return (char*)(ref->vpi_type->vpi_get_str_)(property, ref);
}

void vpi_get_time(vpiHandle obj, s_vpi_time*t)
{
      s_vpi_value value;
      vpiHandle tm = vpip_sim_time();
      value.format = vpiTimeVal;
      vpi_get_value(tm, &value);
      memcpy(t, value.value.time, sizeof (*t));
}

void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      if (expr->vpi_type->vpi_get_value_) {
	    (expr->vpi_type->vpi_get_value_)(expr, vp);
	    return;
      }

      vp->format = vpiSuppressVal;
}

vpiHandle vpi_put_value(vpiHandle obj, s_vpi_value*vp,
			s_vpi_time*tp, int flags)
{
      if (obj->vpi_type->vpi_put_value_)
	    return (obj->vpi_type->vpi_put_value_)(obj, vp, tp, flags);
      else
	    return 0;
}

vpiHandle vpi_handle(int type, vpiHandle ref)
{
      if (type == vpiSysTfCall) {
	    assert(ref == 0);
	    return &vpip_cur_task->base;
      }

      assert(ref->vpi_type->handle_);
      return (ref->vpi_type->handle_)(type, ref);
}

/*
 * This function asks the object to return an iterator for
 * the specified reference. It is up to the iterate_ method to
 * allocate a properly formed iterator.
 */
vpiHandle vpi_iterate(int type, vpiHandle ref)
{
      assert(ref->vpi_type->iterate_);
      return (ref->vpi_type->iterate_)(type, ref);
}

vpiHandle vpi_handle_by_index(vpiHandle ref, int idx)
{
      assert(ref->vpi_type->index_);
      return (ref->vpi_type->index_)(ref, idx);
}

extern void vpi_vprintf(const char*fmt, va_list ap)
{
      vprintf(fmt, ap);
}

extern void vpi_printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap,fmt);
  vpi_vprintf(fmt,ap);
  va_end(ap);
}


/*
 * This function adds the information that the user supplies to a list
 * that I keep.
 */
void vpi_register_systf(const struct t_vpi_systf_data*systf)
{
      struct systf_entry*cur = calloc(1, sizeof(struct systf_entry));
      cur->systf_data = *systf;
      cur->systf_data.tfname = strdup(systf->tfname);
      switch (systf->type) {
	  case vpiSysFunc:
	    cur->next = systf_func_list;
	    systf_func_list = cur;
	    break;
	  case vpiSysTask:
	    cur->next = systf_task_list;
	    systf_task_list = cur;
	    break;
	  default:
	    assert(0);
      }
}

/*
 * $Log: vpi_priv.c,v $
 * Revision 1.5  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2001/10/26 02:29:10  steve
 *  const/non-const warnings. (Stephan Boettcher)
 *
 * Revision 1.3  2001/06/19 14:57:10  steve
 *  Get va_start arguments in right order.
 *
 * Revision 1.2  2001/06/12 03:53:10  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.1  2001/03/14 19:27:44  steve
 *  Rearrange VPI support libraries.
 *
 * Revision 1.11  2000/10/28 00:51:42  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.10  2000/10/06 23:11:39  steve
 *  Replace data references with function calls. (Venkat)
 *
 * Revision 1.9  2000/08/20 17:49:05  steve
 *  Clean up warnings and portability issues.
 *
 * Revision 1.8  2000/07/26 03:53:12  steve
 *  Make simulation precision available to VPI.
 *
 * Revision 1.7  2000/05/07 18:20:08  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.6  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.5  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.4  2000/02/13 19:18:28  steve
 *  Accept memory words as parameter to $display.
 *
 * Revision 1.3  2000/01/20 06:04:55  steve
 *  $dumpall checkpointing in VCD dump.
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

