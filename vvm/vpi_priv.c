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
#ident "$Id: vpi_priv.c,v 1.1 1999/10/28 00:47:25 steve Exp $"
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

static struct systf_entry*systf_list = 0;

/* This is the handle of the task currently being called. */
static struct __vpiSysTaskCall*vpip_cur_task;


void vpip_calltask(const char*fname, unsigned nparms, vpiHandle*parms)
{
      struct systf_entry*idx;
      struct __vpiSysTaskCall cur_task;
      cur_task.base.vpi_type = &vpip_systask_rt;
      cur_task.args = parms;
      cur_task.nargs = nparms;

      vpip_cur_task = &cur_task;

	/* Look for a systf function to invoke. */
      for (idx = systf_list ;  idx ;  idx = idx->next)
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


int vpi_free_object(vpiHandle ref)
{
      free(ref);
      return 0;
}

int vpi_get(int property, vpiHandle ref)
{
      if (property == vpiType)
	    return ref->vpi_type->type_code;

      if (ref->vpi_type->vpi_get_ == 0)
	    return -1;

      return (ref->vpi_type->vpi_get_)(property, ref);
}

char* vpi_get_str(int property, vpiHandle ref)
{
      if (ref->vpi_type->vpi_get_str_ == 0)
	    return 0;

      return (ref->vpi_type->vpi_get_str_)(property, ref);
}

void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      if (expr->vpi_type->vpi_get_value_) {
	    (expr->vpi_type->vpi_get_value_)(expr, vp);
	    return;
      }

      vp->format = vpiSuppressVal;
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

void vpi_printf(const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      vprintf(fmt, ap);
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
      cur->next = systf_list;
      systf_list = cur;
}

/*
 * $Log: vpi_priv.c,v $
 * Revision 1.1  1999/10/28 00:47:25  steve
 *  Rewrite vvm VPI support to make objects more
 *  persistent, rewrite the simulation scheduler
 *  in C (to interface with VPI) and add VPI support
 *  for callbacks.
 *
 */

