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
#ident "$Id: vpi_priv.cc,v 1.46 2004/05/19 03:26:24 steve Exp $"
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
# include  <math.h>

vpi_mode_t vpi_mode_flag = VPI_MODE_NONE;
FILE*vpi_trace = 0;

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

static unsigned hash_string(const char*text)
{
      unsigned h = 0;

      while (*text) {
	    h = (h << 4) ^ (h >> 28) ^ *text;
	    text += 1;
      }
      return h;
}

const char* vpip_name_string(const char*text)
{
      const unsigned HASH_SIZE = 4096;
      static const char*hash_table[HASH_SIZE] = {0};

      unsigned hash_value = hash_string(text) % HASH_SIZE;

	/* If we easily find the string in the hash table, then return
	   that and be done. */
      if (hash_table[hash_value]
	  && (strcmp(hash_table[hash_value], text) == 0)) {
	    return hash_table[hash_value];
      }

	/* The existing hash entry is not a match. Replace it with the
	   newly allocated value, and return the new pointer as the
	   result to the add. */
      const char*res = vpip_string(text);
      hash_table[hash_value] = res;

      return res;
}
PLI_INT32 vpi_chk_error(p_vpi_error_info info)
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

PLI_INT32 vpi_free_object(vpiHandle ref)
{
      int rtn;

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_free_object(%p)", ref);
	    fflush(vpi_trace);
      }

      assert(ref);
      if (ref->vpi_type->vpi_free_object_ == 0)
	    rtn = 1;
      else
	    rtn = ref->vpi_type->vpi_free_object_(ref);

      if (vpi_trace)
	    fprintf(vpi_trace, " --> %d\n", rtn);

      return rtn;
}

static int vpip_get_global(int property)
{
      switch (property) {

	  case vpiTimePrecision:
	    return vpip_get_time_precision();

	  default:
	    fprintf(stderr, "vpi error: bad global property: %d\n", property);
	    assert(0);
	    return vpiUndefined;
      }
}

static const char* vpi_property_str(PLI_INT32 code)
{
      static char buf[32];
      switch (code) {
	  case vpiConstType:
	    return "vpiConstType";
	  case vpiName:
	    return "vpiName";
	  case vpiFullName:
	    return "vpiFullName";
	  case vpiTimeUnit:
	    return "vpiTimeUnit";
	  default:
	    sprintf(buf, "%d", code);
      }
      return buf;
}

static const char* vpi_type_values(PLI_INT32 code)
{
      static char buf[32];
      switch (code) {
	  case vpiConstant:
	    return "vpiConstant";
	  case vpiModule:
	    return "vpiModule";
	  case vpiNet:
	    return "vpiNet";
	  case vpiReg:
	    return "vpiReg";
	  default:
	    sprintf(buf, "%d", code);
      }
      return buf;
}

PLI_INT32 vpi_get(int property, vpiHandle ref)
{
      if (ref == 0)
	    return vpip_get_global(property);

      if (property == vpiType) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(vpiType, %p) --> %s\n",
			  ref, vpi_type_values(ref->vpi_type->type_code));
	    }

	    struct __vpiSignal*rfp = (struct __vpiSignal*)ref;
	    if (ref->vpi_type->type_code == vpiReg && rfp->isint_)
		  return vpiIntegerVar;
	    else
		  return ref->vpi_type->type_code;
      }

      if (ref->vpi_type->vpi_get_ == 0) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get(%s, %p) --X\n",
			  vpi_property_str(property), ref);
	    }

	    return vpiUndefined;
      }

      int res = (ref->vpi_type->vpi_get_)(property, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get(%s, %p) --> %d\n",
		    vpi_property_str(property), ref, res);
      }

      return res;
}

char* vpi_get_str(PLI_INT32 property, vpiHandle ref)
{
      if (ref == 0) {
	    fprintf(stderr, "vpi error: vpi_get_str(%s, 0) called "
		    "with null vpiHandle.\n", vpi_property_str(property));
	    return 0;
      }

      assert(ref);
      if (ref->vpi_type->vpi_get_str_ == 0) {
	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_get_str(%s, %p) --X\n",
			  vpi_property_str(property), ref);
	    }
	    return 0;
      }

      char*res = (char*)(ref->vpi_type->vpi_get_str_)(property, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get_str(%s, %p) --> %s\n",
		    vpi_property_str(property), ref, res);
      }

      return res;
}

int vpip_time_units_from_handle(vpiHandle obj)
{
      struct __vpiSysTaskCall*task;
      struct __vpiScope*scope;
      struct __vpiSignal*signal;

      if (obj == 0)
	    return vpip_get_time_precision();

      switch (obj->vpi_type->type_code) {
	  case vpiSysTaskCall:
	    task = (struct __vpiSysTaskCall*)obj;
	    return task->scope->time_units;

	  case vpiModule:
	    scope = (struct __vpiScope*)obj;
	    return scope->time_units;

	  case vpiNet:
	  case vpiReg:
	    signal = (struct __vpiSignal*)obj;
 	    return signal->scope->time_units;

	  default:
	    fprintf(stderr, "ERROR: vpi_get_time called with object "
		    "handle type=%u\n", obj->vpi_type->type_code);
	    assert(0);
	    return 0;
      }
}

void vpi_get_time(vpiHandle obj, s_vpi_time*vp)
{
      int units;
      vvp_time64_t time;

      assert(vp);

      time = schedule_simtime();

      switch (vp->type) {
          case vpiSimTime:
	    vp->high = (time >> 32) & 0xffffffff;
	    vp->low = time & 0xffffffff;
	    break;

          case vpiScaledRealTime:
	    units = vpip_time_units_from_handle(obj);
            vp->real = pow(10, vpip_get_time_precision() - units);
            vp->real *= time;
	    break;

          default:
            fprintf(stderr, "vpi_get_time: unknown type: %d\n", vp->type);
            assert(0);
	    break;
      }
}

PLI_INT32 vpi_get_vlog_info(p_vpi_vlog_info vlog_info_p)
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

    static char trace_buf[1024];

    if (const char*path = getenv("VPI_TRACE")) {
	  if (!strcmp(path,"-"))
		vpi_trace = stdout;
	  else {
		vpi_trace = fopen(path, "w");
		if (!vpi_trace) {
		      perror(path);
		      exit(1);
		}
		setvbuf(vpi_trace, trace_buf, _IOLBF, sizeof(trace_buf));
	  }
    }
}

void vpi_get_value(vpiHandle expr, s_vpi_value*vp)
{
      assert(expr);
      assert(vp);
      if (expr->vpi_type->vpi_get_value_) {
	    (expr->vpi_type->vpi_get_value_)(expr, vp);

	    if (vpi_trace) switch (vp->format) {
		case vpiStringVal:
		  fprintf(vpi_trace,"vpi_get_value(%p=<%d>) -> string=\"%s\"\n",
			  expr, expr->vpi_type->type_code, vp->value.str);
		  break;

		case vpiBinStrVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> binstr=%s\n",
			  expr->vpi_type->type_code, vp->value.str);
		  break;

		case vpiIntVal:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> int=%d\n",
			  expr->vpi_type->type_code, vp->value.integer);
		  break;

		default:
		  fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <%d>=?\n",
			  expr->vpi_type->type_code, vp->format);
	    }
	    return;
      }

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_get_value(<%d>...) -> <suppress>\n",
		    expr->vpi_type->type_code);
      }

      vp->format = vpiSuppressVal;
}

struct vpip_put_value_event : vvp_gen_event_s {
      vpiHandle handle;
      s_vpi_value value;
};

static void vpip_put_value_callback(vvp_gen_event_t eobj, unsigned char)
{
      vpip_put_value_event*put = (vpip_put_value_event*)eobj;

      put->handle->vpi_type->vpi_put_value_ (put->handle, &put->value);
      switch (put->value.format) {
	    /* Free the copied string. */
	  case vpiBinStrVal:
	  case vpiOctStrVal:
	  case vpiDecStrVal:
	  case vpiHexStrVal:
	  case vpiStringVal:
	    free(put->value.value.str);
	    break;
	    /* If these are every copied then free them too. */
	  case vpiTimeVal:
	  case vpiVectorVal:
	  case vpiStrengthVal:
	  default:
	    break;
      }
}

vpiHandle vpi_put_value(vpiHandle obj, s_vpi_value*vp,
			s_vpi_time*when, PLI_INT32 flags)
{
      assert(obj);

      if (obj->vpi_type->vpi_put_value_ == 0)
	    return 0;

      if (flags != vpiNoDelay) {
	    vvp_time64_t dly;

 	    switch (when->type) {
 		case vpiScaledRealTime:
 		  dly = (vvp_time64_t)(when->real *
				       (pow(10,
					    vpip_time_units_from_handle(obj) -
					    vpip_get_time_precision())));
 		  break;
 		case vpiSimTime:
		  dly = vpip_timestruct_to_time(when);
 		  break;
 		default:
		  dly = 0;
		  break;
 	    }

	    vpip_put_value_event*put = new vpip_put_value_event;
	    put->handle = obj;
	    put->value = *vp;
 	    switch (put->value.format) {
		  /* If this is scheduled make a copy of the string. */
 		case vpiBinStrVal:
 		case vpiOctStrVal:
 		case vpiDecStrVal:
 		case vpiHexStrVal:
 		case vpiStringVal:
		  put->value.value.str = strdup(put->value.value.str);
		  break;
		  /* Do these also need to be copied? */
 		case vpiTimeVal:
 		case vpiVectorVal:
 		case vpiStrengthVal:
 		default:
		  break;
 	    }
	    put->run = &vpip_put_value_callback;
	    schedule_generic(put, 0, dly, false);
	    return 0;
      }

      (obj->vpi_type->vpi_put_value_)(obj, vp);

      return 0;
}

vpiHandle vpi_handle(PLI_INT32 type, vpiHandle ref)
{
      if (type == vpiSysTfCall) {
	    assert(ref == 0);

	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_handle(vpiSysTfCall, 0) "
			  "-> %p (%s)\n", &vpip_cur_task->base,
			  vpip_cur_task->defn->info.tfname);
	    }

	    return &vpip_cur_task->base;
      }

      if (ref == 0) {
	    fprintf(stderr, "internal error: vpi_handle(type=%d, ref=0)\n",
		    type);
      }
      assert(ref);

      if (ref->vpi_type->handle_ == 0) {

	    if (vpi_trace) {
		  fprintf(vpi_trace, "vpi_handle(%d, %p) -X\n",
			  type, ref);
	    }

	    return 0;
      }

      assert(ref->vpi_type->handle_);
      vpiHandle res = (ref->vpi_type->handle_)(type, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle(%d, %p) -> %p\n",
		    type, ref, res);
      }

      return res;
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

vpiHandle vpi_iterate(PLI_INT32 type, vpiHandle ref)
{
      vpiHandle rtn = 0;

      assert(vpi_mode_flag != VPI_MODE_NONE);
      if (vpi_mode_flag == VPI_MODE_REGISTER) {
	    fprintf(stderr, "vpi error: vpi_iterate called during "
		    "vpi_register_systf. You can't do that!\n");
	    return 0;
      }

      if (ref == 0)
	    rtn = vpi_iterate_global(type);
      else if (ref->vpi_type->iterate_)
	    rtn = (ref->vpi_type->iterate_)(type, ref);

      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_iterate(%d, %p) ->%s\n",
	    type, ref, rtn ? "" : " (null)");
      }

      return rtn;
}

vpiHandle vpi_handle_by_index(vpiHandle ref, PLI_INT32 idx)
{
      assert(ref);

      if (ref->vpi_type->index_ == 0)
	    return 0;

      assert(ref->vpi_type->index_);
      return (ref->vpi_type->index_)(ref, idx);
}

static vpiHandle find_name(const char *name, vpiHandle handle)
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

static vpiHandle find_scope(const char *name, vpiHandle handle, int depth)
{
      vpiHandle iter, hand, rtn = 0;

      iter = !handle ? vpi_iterate(vpiModule, NULL) :
		       vpi_iterate(vpiInternalScope, handle);

      while (iter && (hand = vpi_scan(iter))) {
	    char *nm = vpi_get_str(vpiName, hand);
	    int len = strlen(nm);
	    const char *cp = name + len;	/* hier separator */

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

vpiHandle vpi_handle_by_name(const char *name, vpiHandle scope)
{
      vpiHandle hand;
      const char *nm, *cp;
      int len;


      if (vpi_trace) {
	    fprintf(vpi_trace, "vpi_handle_by_name(%s, %p) -->\n",
		    name, scope);
      }

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
	    vpiHandle out = find_name(name, hand);
	    return out;
      }

      return 0;
}

extern "C" PLI_INT32 vpi_vprintf(const char*fmt, va_list ap)
{
      return vpi_mcd_vprintf(1, fmt, ap);
}

extern "C" PLI_INT32 vpi_printf(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      int r = vpi_mcd_vprintf(1, fmt, ap);
      va_end(ap);
      return r;
}

extern "C" PLI_INT32 vpi_flush(void)
{
      return vpi_mcd_flush(1);
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

extern "C" void vpi_sim_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

extern "C" void vpi_control(PLI_INT32 operation, ...)
{
      va_list ap;
      va_start(ap, operation);
      vpi_sim_vcontrol(operation, ap);
      va_end(ap);
}

/*
 * $Log: vpi_priv.cc,v $
 * Revision 1.46  2004/05/19 03:26:24  steve
 *  Support delayed/non-blocking assignment to reals and others.
 *
 * Revision 1.45  2004/02/18 17:52:00  steve
 *  PRototypes match the standard.
 *
 * Revision 1.44  2004/02/18 02:51:59  steve
 *  Fix type mismatches of various VPI functions.
 *
 * Revision 1.43  2003/06/25 04:04:19  steve
 *  Fix mingw portability problems.
 *
 * Revision 1.42  2003/06/22 04:19:26  steve
 *  vpi_handle diagnostic message.
 *
 * Revision 1.41  2003/06/17 16:55:08  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.40  2003/05/30 04:08:28  steve
 *  vpi_trace of vpi_free_object.
 *
 * Revision 1.39  2003/05/15 16:51:09  steve
 *  Arrange for mcd id=00_00_00_01 to go to stdout
 *  as well as a user specified log file, set log
 *  file to buffer lines.
 *
 *  Add vpi_flush function, and clear up some cunfused
 *  return codes from other vpi functions.
 *
 *  Adjust $display and vcd/lxt messages to use the
 *  standard output/log file.
 *
 * Revision 1.38  2003/05/15 01:24:46  steve
 *  Return all 64bits of time in vpi_get_time.
 *
 * Revision 1.37  2003/05/02 04:29:57  steve
 *  Add put_value with transport delay.
 *
 * Revision 1.36  2003/04/27 04:19:24  steve
 *  Support vpiScaledRealTime.
 *
 * Revision 1.35  2003/03/14 05:02:34  steve
 *  More detail in vpi tracing.
 *
 * Revision 1.34  2003/03/13 04:34:18  steve
 *  Add VPI_TRACE tracing of VPI calls.
 *  vpi_handle_by_name takes a const char*.
 *
 * Revision 1.33  2003/03/12 02:50:32  steve
 *  Add VPI tracing.
 *
 * Revision 1.32  2003/03/06 04:32:00  steve
 *  Use hashed name strings for identifiers.
 *
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
