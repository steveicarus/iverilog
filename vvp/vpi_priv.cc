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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vpi_priv.cc,v 1.31 2003/02/21 03:40:35 steve Exp $"
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

static s_vpi_vlog_info  vpi_vlog_info;
static s_vpi_error_info vpip_last_error = { 0, 0, 0, 0, 0, 0, 0 };

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

int vpi_chk_error(p_vpi_error_info info)
{
      if (vpip_last_error.state == 0)
	    return 0;

      info->state = vpip_last_error.state;
      info->level = vpip_last_error.level;
      info->message = vpip_last_error.message;
      info->product = vpi_vlog_info.product;
      info->code = "";
      info->file = 0;
      info->line = 0;

      return info->level;
}

/*
 * When a task is called, this value is set so that vpi_handle can
 * fathom the vpi_handle(vpiSysTfCall,0) function.
 */
struct __vpiSysTaskCall*vpip_cur_task = 0;

int vpi_free_object(vpiHandle ref)
{
      assert(ref);
      if (ref->vpi_type->vpi_free_object_ == 0)
	    return 1;

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
      if (ref == 0) {
	    fprintf(stderr, "vpi error: vpi_get_str(%d, ...) called "
		    "with null vpiHandle.\n", property);
	    return 0;
      }

      assert(ref);
      if (ref->vpi_type->vpi_get_str_ == 0)
	    return 0;

      return (char*)(ref->vpi_type->vpi_get_str_)(property, ref);
}

void vpi_get_time(vpiHandle obj, s_vpi_time*vp)
{
      assert(vp);

	// Only vpiSimTime is supported, for now.
      assert(vp->type == vpiSimTime);
      vp->high = 0;
      vp->low = schedule_simtime();
}

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

      if (ref->vpi_type->handle_ == 0)
	    return 0;

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

      if (ref->vpi_type->iterate_)
	  return (ref->vpi_type->iterate_)(type, ref);
      else
	  return 0;
}

vpiHandle vpi_handle_by_index(vpiHandle ref, int idx)
{
      assert(ref);

      if (ref->vpi_type->index_ == 0)
	    return 0;

      assert(ref->vpi_type->index_);
      return (ref->vpi_type->index_)(ref, idx);
}

static vpiHandle find_name(char *name, vpiHandle handle)
{
      vpiHandle rtn = 0;
      struct __vpiScope*ref = (struct __vpiScope*)handle;

      /* check module names */
      if (!strcmp(name, vpi_get_str(vpiName, handle)))
	    rtn = handle;

      /* brute force search for the name in all objects in this scope */
      for (unsigned i = 0 ;  i < ref->nintern ;  i += 1) {
	    char *nm = vpi_get_str(vpiName, ref->intern[i]);
	    if (!strcmp(name, nm)) {
		  rtn = ref->intern[i];
		  break;
	    } else if (vpi_get(vpiType, ref->intern[i]) == vpiMemory) {
		  /* We need to iterate on the words */
		  vpiHandle word_i, word_h;
		  word_i = vpi_iterate(vpiMemoryWord, ref->intern[i]);
		  while (word_i && (word_h = vpi_scan(word_i))) {
			nm = vpi_get_str(vpiName, word_h);
			if (!strcmp(name, nm)) {
			      rtn = word_h;
			      break;
			}
		  }
	    }
	    /* found it yet? */
	    if (rtn) break;
      }

      return rtn;
}

static vpiHandle find_scope(char *name, vpiHandle handle, int depth)
{
      vpiHandle iter, hand, rtn = 0;

      iter = !handle ? vpi_iterate(vpiModule, NULL) :
		       vpi_iterate(vpiInternalScope, handle);

      while (iter && (hand = vpi_scan(iter))) {
	    char *nm = vpi_get_str(vpiName, hand);
	    int len = strlen(nm);
	    char *cp = name + len;	/* hier separator */

	    if (!handle && !strcmp(name, nm)) {
		  /* root module */
		  rtn = hand;
	    } else if (!strncmp(name, nm, len) && *(cp) == '.')
		  /* recurse deeper */
		  rtn = find_scope(cp+1, hand, depth + 1);

	    /* found it yet ? */
	    if (rtn) break;
      }

      /* matched up to here */
      if (!rtn) rtn = handle;

      return rtn;
}

vpiHandle vpi_handle_by_name(char *name, vpiHandle scope)
{
      vpiHandle hand;
      char *nm, *cp;
      int len;

      /* If scope provided, look in corresponding module; otherwise
       * traverse the hierarchy specified in name to find the leaf module
       * and try finding it there.
       */
      if (scope)
	    hand = vpi_handle(vpiModule, scope);
      else
	    hand = find_scope(name, NULL, 0);

      if (hand) {
	    /* remove hierarchical portion of name */
	    nm = vpi_get_str(vpiFullName, hand);
	    len = strlen(nm);
	    cp = name + len;
	    if (!strncmp(name, nm, len) && *cp == '.') name = cp + 1;

	    /* Ok, time to burn some cycles */
	    return find_name(name, hand);
      }

      return 0;
}

extern "C" void vpi_vprintf(const char*fmt, va_list ap)
{
      vprintf(fmt, ap);
}

extern "C" void vpi_printf(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      vpi_vprintf(fmt, ap);
      va_end(ap);
}


extern "C" void vpi_sim_vcontrol(int operation, va_list ap)
{
      switch (operation) {
	  case vpiFinish:
	    schedule_finish(0);
	    break;

	  case vpiStop:
	    schedule_stop(0);
	    break;

	  default:
	    assert(0);
      }
}

extern "C" void vpi_sim_control(int operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

extern "C" void vpi_control(int operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

/*
 * $Log: vpi_priv.cc,v $
 * Revision 1.31  2003/02/21 03:40:35  steve
 *  Add vpiStop and interactive mode.
 *
 * Revision 1.30  2003/02/09 23:33:26  steve
 *  Spelling fixes.
 *
 * Revision 1.29  2003/02/02 01:40:24  steve
 *  Five vpi_free_object a default behavior.
 *
 * Revision 1.28  2003/01/10 19:02:21  steve
 *  Add missing vpi entry points.
 *
 * Revision 1.27  2003/01/10 03:06:32  steve
 *  Remove vpithunk, and move libvpi to vvp directory.
 *
 * Revision 1.26  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.25  2002/12/11 23:55:22  steve
 *  Add vpi_handle_by_name to the VPI interface,
 *  and bump the vpithunk magic number.
 *
 * Revision 1.24  2002/08/24 05:02:58  steve
 *  Fix = vs == error.
 *
 * Revision 1.23  2002/08/12 01:35:09  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.22  2002/07/19 01:57:26  steve
 *  Add vpi_chk_error and vpi_control functions.
 *
 * Revision 1.21  2002/07/19 01:12:50  steve
 *  vpi_iterate returns 0 on error.
 */
