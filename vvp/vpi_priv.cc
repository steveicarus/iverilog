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
#ident "$Id: vpi_priv.cc,v 1.2 2001/03/19 01:55:38 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <stdarg.h>
# include  <assert.h>
# include  <malloc.h>

/*
 * When a task is called, this value is set so that vpi_handle can
 * fathom the vpi_handle(vpiSysTfCall,0) function.
 */
struct __vpiSysTaskCall*vpip_cur_task = 0;

int vpi_free_object(vpiHandle ref)
{
      free(ref);
      return 0;
}

static int vpip_get_global(int property)
{
      switch (property) {

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

      return (ref->vpi_type->vpi_get_str_)(property, ref);
}

void vpi_get_time(vpiHandle obj, s_vpi_time*t)
{
      assert(0);
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

void vpi_printf(const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      vprintf(fmt, ap);
      va_end(ap);
}


/* STUBS */

vpiHandle vpi_register_cb(p_cb_data data)
{
      assert(0);
      return 0;
}


int vpi_remove_cb(vpiHandle ref)
{
      assert(0);
      return 0;
}

void vpi_sim_control(int operation, ...)
{
      switch (operation) {
	  case vpiFinish:
	    schedule_finish(0);
	    break;

	  default:
	    assert(0);
      }
}

/*
 * $Log: vpi_priv.cc,v $
 * Revision 1.2  2001/03/19 01:55:38  steve
 *  Add support for the vpiReset sim control.
 *
 * Revision 1.1  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 */

