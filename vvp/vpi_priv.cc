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
#ident "$Id: vpi_priv.cc,v 1.18 2002/07/05 17:14:15 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "schedule.h"
# include  <stdio.h>
# include  <stdarg.h>
# include  <string.h>
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>

vpi_mode_t vpi_mode_flag = VPI_MODE_NONE;

/*
 * The vpip_string function creates a constant string from the pass
 * input. This constant string is permanently allocate from an
 * efficient string buffer store.
 */
struct vpip_string_chunk {
      struct vpip_string_chunk*next;
      char data[64*1024 - sizeof (struct vpip_string_chunk*)];
};

const char *vpip_string(const char*str)
{
      static vpip_string_chunk first_chunk = {0, {0}};
      static vpip_string_chunk*chunk_list = &first_chunk;
      static unsigned chunk_fill = 0;

      unsigned len = strlen(str);
      assert( (len+1) <= sizeof chunk_list->data );

      if ( (len+1) > (sizeof chunk_list->data - chunk_fill) ) {
	    vpip_string_chunk*tmp = new vpip_string_chunk;
	    tmp->next = chunk_list;
	    chunk_list = tmp;
	    chunk_fill = 0;
      }

      char*res = chunk_list->data + chunk_fill;
      chunk_fill += len + 1;

      strcpy(res, str);
      return res;
}

/*
 * When a task is called, this value is set so that vpi_handle can
 * fathom the vpi_handle(vpiSysTfCall,0) function.
 */
struct __vpiSysTaskCall*vpip_cur_task = 0;

int vpi_free_object(vpiHandle ref)
{
      assert(ref);
      assert(ref->vpi_type->vpi_free_object_);
      return ref->vpi_type->vpi_free_object_(ref);
}

static int vpip_get_global(int property)
{
      switch (property) {

	  case vpiTimePrecision:
	    return vpip_get_time_precision();

	  default:
	    fprintf(stderr, "vpi error: bad global property: %d\n", property);
	    assert(0);
	    return -1;
      }
}

int vpi_get(int property, vpiHandle ref)
{
      if (ref == 0)
	    return vpip_get_global(property);

      if (property == vpiType) {
	    struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
	    if (ref->vpi_type->type_code == vpiReg && rfp->isint_)
		  return vpiIntegerVar;
	    else
		  return ref->vpi_type->type_code;
      }

      if (ref->vpi_type->vpi_get_ == 0)
	    return -1;

      return (ref->vpi_type->vpi_get_)(property, ref);
}

char* vpi_get_str(int property, vpiHandle ref)
{
      assert(ref);
      if (ref->vpi_type->vpi_get_str_ == 0)
	    return 0;

      return (char*)(ref->vpi_type->vpi_get_str_)(property, ref);
}

void vpi_get_time(vpiHandle obj, s_vpi_time*vp)
{
      assert(vp);

	// XXXX Cheat. Ignore timescale for the scope.
      vp->type = vpiSimTime;
      vp->high = 0;
      vp->low = schedule_simtime();
}

static s_vpi_vlog_info vpi_vlog_info;

int vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p)
{
    if (vlog_info_p != 0) {
	  *vlog_info_p = vpi_vlog_info;
	  return 1;
    } else {
	  return 0;
    }
}

void vpi_set_vlog_info(int argc, char** argv)
{
    vpi_vlog_info.product = "Icarus Verilog";
    vpi_vlog_info.version = "$Name:  $";
    vpi_vlog_info.argc    = argc;
    vpi_vlog_info.argv    = argv;
}

void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      assert(expr);
      assert(vp);
      if (expr->vpi_type->vpi_get_value_) {
	    (expr->vpi_type->vpi_get_value_)(expr, vp);
	    return;
      }

      vp->format = vpiSuppressVal;
}

vpiHandle vpi_put_value(vpiHandle obj, s_vpi_value*vp,
			s_vpi_time*tp, int flags)
{
      assert(obj);
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

      assert(ref);
      assert(ref->vpi_type->handle_);
      return (ref->vpi_type->handle_)(type, ref);
}

/*
 * This function asks the object to return an iterator for
 * the specified reference. It is up to the iterate_ method to
 * allocate a properly formed iterator.
 */
static vpiHandle vpi_iterate_global(int type)
{
      switch (type) {
	  case vpiModule:
	    return vpip_make_root_iterator();

	  default:
	    assert(0);
      }

      return 0;
}

vpiHandle vpi_iterate(int type, vpiHandle ref)
{
      assert(vpi_mode_flag != VPI_MODE_NONE);
      if (vpi_mode_flag == VPI_MODE_REGISTER) {
	    fprintf(stderr, "vpi error: vpi_iterate called during "
		    "vpi_register_systf. You can't do that!\n");
	    return 0;
      }
	    
      if (ref == 0)
	    return vpi_iterate_global(type);

      assert(ref->vpi_type->iterate_);
      return (ref->vpi_type->iterate_)(type, ref);
}

vpiHandle vpi_handle_by_index(vpiHandle ref, int idx)
{
      assert(ref);
      assert(ref->vpi_type->index_);
      return (ref->vpi_type->index_)(ref, idx);
}

extern "C" void vpi_vprintf(const char*fmt, va_list ap)
{
      vprintf(fmt, ap);
}


extern "C" void vpi_sim_vcontrol(int operation, va_list ap)
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
 * Revision 1.18  2002/07/05 17:14:15  steve
 *  Names of vpi objects allocated as vpip_strings.
 *
 * Revision 1.17  2002/06/21 04:58:55  steve
 *  Add support for special integer vectors.
 *
 * Revision 1.16  2002/06/02 19:05:50  steve
 *  Check for null pointers from users.
 *
 * Revision 1.15  2002/05/18 02:34:11  steve
 *  Add vpi support for named events.
 *
 *  Add vpi_mode_flag to track the mode of the
 *  vpi engine. This is for error checking.
 *
 * Revision 1.14  2002/05/03 15:44:11  steve
 *  Add vpiModule iterator to vpiScope objects.
 *
 * Revision 1.13  2002/04/07 00:46:21  steve
 *  minor cleanup of formatting.
 *
 * Revision 1.12  2002/01/09 03:15:23  steve
 *  Add vpi_get_vlog_info support.
 *
 * Revision 1.11  2002/01/06 00:48:39  steve
 *  VPI access to root module scopes.
 *
 * Revision 1.10  2001/10/31 04:27:47  steve
 *  Rewrite the functor type to have fewer functor modes,
 *  and use objects to manage the different types.
 *  (Stephan Boettcher)
 *
 * Revision 1.9  2001/09/15 18:27:05  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.8  2001/07/11 02:27:21  steve
 *  Add support for REadOnlySync and monitors.
 *
 * Revision 1.7  2001/06/30 23:03:17  steve
 *  support fast programming by only writing the bits
 *  that are listed in the input file.
 *
 * Revision 1.6  2001/06/21 22:54:12  steve
 *  Support cbValueChange callbacks.
 */

