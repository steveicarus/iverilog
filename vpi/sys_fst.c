/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "sys_priv.h"
# include "vcd_priv.h"
# include "fstapi.h"

/*
 * This file contains the implementations of the FST related functions.
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
# include  "ivl_alloc.h"

static char *dump_path = NULL;
static struct fstContext *dump_file = NULL;

static struct t_vpi_time zero_delay = { vpiSimTime, 0, 0, 0.0 };

struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct t_vpi_time time;
      struct vcd_info *next;
      struct vcd_info *dmp_next;
      fstHandle handle;
      int scheduled;
};


static struct vcd_info *vcd_list = NULL;
static struct vcd_info *vcd_dmp_list = NULL;
static PLI_UINT64 vcd_cur_time = 0;
static int dump_is_off = 0;
static long dump_limit = 0;
static int dump_is_full = 0;
static int finish_status = 0;


static enum lxm_optimum_mode_e {
      LXM_NONE  = 0,
      LXM_SPACE = 1,
      LXM_SPEED = 2,
      LXM_BOTH = 3
} lxm_optimum_mode = LXM_NONE;

static const char*units_names[] = {
      "s",
      "ms",
      "us",
      "ns",
      "ps",
      "fs"
};

static void show_this_item(struct vcd_info*info)
{
      s_vpi_value value;
      PLI_INT32 type = vpi_get(vpiType, info->item);

      if (type == vpiRealVar) {
	    value.format = vpiRealVal;
	    vpi_get_value(info->item, &value);
	    fstWriterEmitValueChange(dump_file, info->handle, &value.value.real);
      } else {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    fstWriterEmitValueChange(dump_file, info->handle, (type != vpiNamedEvent) ? value.value.str : "1");
      }
}

/* Dump values for a $dumpoff. */
static void show_this_item_x(struct vcd_info*info)
{
      PLI_INT32 type = vpi_get(vpiType, info->item);

      if (type == vpiRealVar) {
	      /* Some tools dump nothing here...? */
            double mynan = strtod("NaN", NULL);
	    fstWriterEmitValueChange(dump_file, info->handle, &mynan);
      } else if (type == vpiNamedEvent) {
	    /* Do nothing for named events. */
      } else {
	    int siz = vpi_get(vpiSize, info->item);
	    char *xmem = malloc(siz);
	    memset(xmem, 'x', siz);
	    fstWriterEmitValueChange(dump_file, info->handle, xmem);
	    free(xmem);
      }
}


/*
 * managed qsorted list of scope names/variables for duplicates bsearching
 */

struct vcd_names_list_s fst_tab = { 0, 0, 0, 0 };
struct vcd_names_list_s fst_var = { 0, 0, 0, 0 };


static int dumpvars_status = 0; /* 0:fresh 1:cb installed, 2:callback done */
static PLI_UINT64 dumpvars_time;
__inline__ static int dump_header_pending(void)
{
      return dumpvars_status != 2;
}

/*
 * This function writes out all the traced variables, whether they
 * changed or not.
 */
static void vcd_checkpoint(void)
{
      struct vcd_info*cur;

      for (cur = vcd_list ;  cur ;  cur = cur->next)
	    show_this_item(cur);
}

static void vcd_checkpoint_x(void)
{
      struct vcd_info*cur;

      for (cur = vcd_list ;  cur ;  cur = cur->next)
	    show_this_item_x(cur);
}

static PLI_INT32 variable_cb_2(p_cb_data cause)
{
      struct vcd_info* info = vcd_dmp_list;
      PLI_UINT64 now = timerec_to_time64(cause->time);

      if (now != vcd_cur_time) {
	    fstWriterEmitTimeChange(dump_file, now);
	    vcd_cur_time = now;
      }

      do {
           show_this_item(info);
           info->scheduled = 0;
      } while ((info = info->dmp_next) != 0);

      vcd_dmp_list = 0;

      return 0;
}

static PLI_INT32 variable_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

      if (dump_is_full) return 0;
      if (dump_is_off) return 0;
      if (dump_header_pending()) return 0;
      if (info->scheduled) return 0;

      if ((dump_limit > 0) && fstWriterGetDumpSizeLimitReached(dump_file)) {
            dump_is_full = 1;
            vpi_printf("WARNING: Dump file limit (%ld bytes) "
                               "exceeded.\n", dump_limit);
            return 0;
      }

      if (!vcd_dmp_list) {
          cb = *cause;
	  cb.time = &zero_delay;
          cb.reason = cbReadOnlySynch;
          cb.cb_rtn = variable_cb_2;
          vpi_register_cb(&cb);
      }

      info->scheduled = 1;
      info->dmp_next  = vcd_dmp_list;
      vcd_dmp_list    = info;

      return 0;
}

static PLI_INT32 dumpvars_cb(p_cb_data cause)
{
      if (dumpvars_status != 1) return 0;

      dumpvars_status = 2;

      dumpvars_time = timerec_to_time64(cause->time);
      vcd_cur_time = dumpvars_time;

      /* nothing to do for $enddefinitions $end */

      if (!dump_is_off) {
	    fstWriterEmitTimeChange(dump_file, dumpvars_time);
	    /* nothing to do for  $dumpvars... */
	    vcd_checkpoint();
	    /* ...nothing to do for $end */
      }

      return 0;
}

static PLI_INT32 finish_cb(p_cb_data cause)
{
      struct vcd_info *cur, *next;

      if (finish_status != 0) return 0;

      finish_status = 1;

      dumpvars_time = timerec_to_time64(cause->time);

      if (!dump_is_off && !dump_is_full && dumpvars_time != vcd_cur_time) {
	    fstWriterEmitTimeChange(dump_file, dumpvars_time);
      }

      fstWriterClose(dump_file);

      for (cur = vcd_list ;  cur ;  cur = next) {
	    next = cur->next;
	    free(cur);
      }
      vcd_list = 0;
      vcd_names_delete(&fst_tab);
      vcd_names_delete(&fst_var);
      nexus_ident_delete();
      free(dump_path);
      dump_path = 0;

      return 0;
}

__inline__ static int install_dumpvars_callback(void)
{
      struct t_cb_data cb;

      if (dumpvars_status == 1) return 0;

      if (dumpvars_status == 2) {
	    vpi_printf("FST warning: $dumpvars ignored, previously"
	               " called at simtime %" PLI_UINT64_FMT "\n",
	               dumpvars_time);
	    return 1;
      }

      cb.time = &zero_delay;
      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = dumpvars_cb;
      cb.user_data = 0x0;
      cb.obj = 0x0;

      vpi_register_cb(&cb);

      cb.reason = cbEndOfSimulation;
      cb.cb_rtn = finish_cb;

      vpi_register_cb(&cb);

      dumpvars_status = 1;
      return 0;
}

static PLI_INT32 sys_dumpoff_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (dump_is_off) return 0;

      dump_is_off = 1;

      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    fstWriterEmitTimeChange(dump_file, now64);
	    vcd_cur_time = now64;
      }

      fstWriterEmitDumpActive(dump_file, 0); /* $dumpoff */
      vcd_checkpoint_x();

      return 0;
}

static PLI_INT32 sys_dumpon_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (!dump_is_off) return 0;

      dump_is_off = 0;

      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    fstWriterEmitTimeChange(dump_file, now64);
	    vcd_cur_time = now64;
      }

      fstWriterEmitDumpActive(dump_file, 1); /* $dumpon */
      vcd_checkpoint();

      return 0;
}

static PLI_INT32 sys_dumpall_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

      (void)name; /* Parameter is not used. */

      if (dump_is_off) return 0;
      if (dump_file == 0) return 0;
      if (dump_header_pending()) return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      now64 = timerec_to_time64(&now);

      if (now64 > vcd_cur_time) {
	    fstWriterEmitTimeChange(dump_file, now64);
	    vcd_cur_time = now64;
      }

      /* nothing to do for $dumpall... */
      vcd_checkpoint();

      return 0;
}

static void open_dumpfile(vpiHandle callh)
{
      if (dump_path == 0) dump_path = strdup("dump.fst");

      dump_file = fstWriterCreate(dump_path, 1);

      if (dump_file == 0) {
	    vpi_printf("FST Error: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open %s for output.\n", dump_path);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    free(dump_path);
	    dump_path = 0;
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);
	    unsigned scale = 1;
	    unsigned udx = 0;
	    time_t walltime;
	    char scale_buf[65];

	    vpi_printf("FST info: dumpfile %s opened for output.\n",
	               dump_path);

	    time(&walltime);

	    assert(prec >= -15);
	    while (prec < 0) {
		  udx += 1;
		  prec += 3;
	    }
	    while (prec > 0) {
		  scale *= 10;
		  prec -= 1;
	    }

	    fstWriterSetDate(dump_file, asctime(localtime(&walltime)));
	    fstWriterSetVersion(dump_file, "Icarus Verilog");
	    sprintf(scale_buf, "\t%u%s\n", scale, units_names[udx]);
	    fstWriterSetTimescaleFromString(dump_file, scale_buf);
	      /* Set the faster dump type when requested. */
	    if ((lxm_optimum_mode == LXM_SPEED) ||
	        (lxm_optimum_mode == LXM_BOTH)) {
		  fstWriterSetPackType(dump_file, 1);
	    }
	      /* Set the most effective compression when requested. */
	    if ((lxm_optimum_mode == LXM_SPACE) ||
	        (lxm_optimum_mode == LXM_BOTH)) {
		  fstWriterSetRepackOnClose(dump_file, 1);
	    }
      }
}

static PLI_INT32 sys_dumpfile_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      char *path;

        /* $dumpfile must be called before $dumpvars starts! */
      if (dumpvars_status != 0) {
	    char msg[64];
	    snprintf(msg, sizeof(msg), "FST warning: %s:%d:",
	             vpi_get_str(vpiFile, callh),
	             (int)vpi_get(vpiLineNo, callh));
	    msg[sizeof(msg)-1] = 0;
	    vpi_printf("%s %s called after $dumpvars started,\n", msg, name);
	    vpi_printf("%*s using existing file (%s).\n",
	               (int) strlen(msg), " ", dump_path);
	    vpi_free_object(argv);
	    return 0;
      }

      path = get_filename_with_suffix(callh, name, vpi_scan(argv), "fst");
      vpi_free_object(argv);
      if (! path) return 0;

      if (dump_path) {
	    vpi_printf("FST warning: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Overriding dump file %s with %s.\n", dump_path, path);
	    free(dump_path);
      }
      dump_path = path;

      return 0;
}

static PLI_INT32 sys_dumpflush_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      if (dump_file) fstWriterFlushContext(dump_file);

      return 0;
}

static PLI_INT32 sys_dumplimit_calltf(ICARUS_VPI_CONST PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;

      (void)name; /* Parameter is not used. */

      /* Get the value and set the dump limit. */
      val.format = vpiIntVal;
      vpi_get_value(vpi_scan(argv), &val);
      dump_limit = val.value.integer;
      fstWriterSetDumpSizeLimit(dump_file, dump_limit);

      vpi_free_object(argv);
      return 0;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      struct t_cb_data cb;
      struct vcd_info* info;

      enum fstVarType type = FST_VT_MAX;
      enum fstScopeType stype = FST_ST_MAX;
      enum fstVarDir dir;
      const char *name;
      const char *fullname;
      char *escname;
      const char *ident;
      fstHandle new_ident;
      int nexus_id;
      unsigned size;
      PLI_INT32 item_type;

	/* Get the displayed type for the various $var and $scope types. */
	/* Not all of these are supported now, but they should be in a
	 * future development version. */
      item_type = vpi_get(vpiType, item);
      switch (item_type) {
	  case vpiNamedEvent: type = FST_VT_VCD_EVENT; break;
	  case vpiIntVar:
	  case vpiIntegerVar: type = FST_VT_VCD_INTEGER; break;
	  case vpiParameter:  type = FST_VT_VCD_PARAMETER; break;
	    /* Icarus converts realtime to real. */
	  case vpiRealVar:    type = FST_VT_VCD_REAL; break;
	  case vpiMemoryWord:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiLongIntVar:
	  case vpiReg:        type = FST_VT_VCD_REG; break;
	    /* Icarus converts a time to a plain register. */
	  case vpiTimeVar:    type = FST_VT_VCD_TIME; break;
	  case vpiNet:
	    switch (vpi_get(vpiNetType, item)) {
		case vpiWand:    type = FST_VT_VCD_WAND; break;
		case vpiWor:     type = FST_VT_VCD_WOR; break;
		case vpiTri:     type = FST_VT_VCD_TRI; break;
		case vpiTri0:    type = FST_VT_VCD_TRI0; break;
		case vpiTri1:    type = FST_VT_VCD_TRI1; break;
		case vpiTriReg:  type = FST_VT_VCD_TRIREG; break;
		case vpiTriAnd:  type = FST_VT_VCD_TRIAND; break;
		case vpiTriOr:   type = FST_VT_VCD_TRIOR; break;
		case vpiSupply1: type = FST_VT_VCD_SUPPLY1; break;
		case vpiSupply0: type = FST_VT_VCD_SUPPLY0; break;
		default:         type = FST_VT_VCD_WIRE; break;
	    }
	    break;

	  case vpiNamedBegin: stype = FST_ST_VCD_BEGIN; break;
	  case vpiNamedFork:  stype = FST_ST_VCD_FORK; break;
	  case vpiFunction:   stype = FST_ST_VCD_FUNCTION; break;
	  case vpiGenScope:   stype = FST_ST_VCD_GENERATE; break;
	  case vpiModule:     stype = FST_ST_VCD_MODULE; break;
	  case vpiPackage:    stype = FST_ST_VCD_PACKAGE; break;
	  case vpiTask:       stype = FST_ST_VCD_TASK; break;

	  default:
	    vpi_printf("FST warning: $dumpvars: Unsupported argument "
	               "type (%s).\n", vpi_get_str(vpiType, item));
	    return;
      }

	/* Do some special processing/checking on array words. Dumping
	 * array words is an Icarus extension. */
      if (item_type == vpiMemoryWord) {
	      /* Turn a non-constant array word select into a constant
	       * word select. */
	    if (vpi_get(vpiConstantSelect, item) == 0) {
		  vpiHandle array = vpi_handle(vpiParent, item);
		  PLI_INT32 idx = vpi_get(vpiIndex, item);
		  item = vpi_handle_by_index(array, idx);
	    }

	      /* An array word is implicitly escaped so look for an
	       * escaped identifier that this could conflict with. */
	      /* This does not work as expected since we always find at
	       * least the array word. We likely need a custom routine. */
            if (vpi_get(vpiType, item) == vpiMemoryWord &&
                vpi_handle_by_name(vpi_get_str(vpiFullName, item), 0)) {
		  vpi_printf("FST warning: array word %s will conflict "
		             "with an escaped identifier.\n",
		             vpi_get_str(vpiFullName, item));
            }
      }

      fullname = vpi_get_str(vpiFullName, item);

	/* Generate the $var or $scope commands. */
      switch (item_type) {
	  case vpiParameter:
	    vpi_printf("FST sorry: $dumpvars: can not dump parameters.\n");
	    break;

	  case vpiNamedEvent:
	  case vpiIntegerVar:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiIntVar:
	  case vpiLongIntVar:
	  case vpiRealVar:
	  case vpiMemoryWord:
	  case vpiReg:
	  case vpiTimeVar:
	  case vpiNet:

	      /* If we are skipping all signal or this is in an automatic
	       * scope then just return. */
            if (skip || vpi_get(vpiAutomatic, item)) return;

	      /* Skip this signal if it has already been included.
	       * This can only happen for implicitly given signals. */
	    if (vcd_names_search(&fst_var, fullname)) return;

	      /* Declare the variable in the FST file. */
	    name = vpi_get_str(vpiName, item);
	    if (is_escaped_id(name)) {
		  escname = malloc(strlen(name) + 2);
		  sprintf(escname, "\\%s", name);
	    } else escname = strdup(name);

	      /* Some signals can have an alias so handle that. */
	    nexus_id = vpi_get(_vpiNexusId, item);

	    ident = 0;
	    if (nexus_id) ident = find_nexus_ident(nexus_id);

	      /* Named events do not have a size, but other tools use
	       * a size of 1 and some viewers do not accept a width of
	       * zero so we will also use a width of one for events. */
	    if (item_type == vpiNamedEvent) size = 1;
	    else size = vpi_get(vpiSize, item);
	      /* The FST format supports a port direction so if the net
	       * is a port set the direction to one of the following:
	       *   FST_VD_INPUT, FST_VD_OUTPUT or FST_VD_INOUT */
	    dir = FST_VD_IMPLICIT;

	    if (size > 1 || vpi_get(vpiLeftRange, item) != 0) {
		  char *buf = malloc(strlen(escname) + 65);
		  sprintf(buf, "%s [%i:%i]", escname,
                            (int)vpi_get(vpiLeftRange, item),
                            (int)vpi_get(vpiRightRange, item));

		  new_ident = fstWriterCreateVar(dump_file, type,
		                                 dir, size, buf,
		                                 (fstHandle)(intptr_t)ident);
		  free(buf);
	    } else {
		  new_ident = fstWriterCreateVar(dump_file, type,
		                                 dir, size, escname,
		                                 (fstHandle)(intptr_t)ident);
	    }
	    free(escname);

	    if (!ident) {
		  if (nexus_id) set_nexus_ident(nexus_id,
		                                (const char *)(intptr_t)new_ident);

		    /* Add a callback for the signal. */
		  info = malloc(sizeof(*info));

		  info->time.type = vpiSimTime;
		  info->item  = item;
		  info->handle = new_ident;
		  info->scheduled = 0;

		  cb.time      = &info->time;
		  cb.user_data = (char*)info;
		  cb.value     = NULL;
		  cb.obj       = item;
		  cb.reason    = cbValueChange;
		  cb.cb_rtn    = variable_cb_1;

		  info->dmp_next = 0;
		  info->next  = vcd_list;
		  vcd_list    = info;

		  info->cb    = vpi_register_cb(&cb);
	    }

	    break;

	  case vpiModule:
	  case vpiPackage:
	  case vpiGenScope:
	  case vpiFunction:
	  case vpiTask:
	  case vpiNamedBegin:
	  case vpiNamedFork:

	    if (depth > 0) {
		  char *instname;
		  char *defname = NULL;
		  /* list of types to iterate upon */
		  static int types[] = {
			/* Value */
			vpiNamedEvent,
			vpiNet,
			/* vpiParameter, */
			vpiReg,
			vpiVariables,
			/* Scope */
			vpiFunction,
			vpiGenScope,
			vpiModule,
			vpiNamedBegin,
			vpiNamedFork,
			vpiTask,
			-1
		  };
		  int i;
		  int nskip = (vcd_names_search(&fst_tab, fullname) != 0);

		    /* We have to always scan the scope because the
		     * depth could be different for this call. */
		  if (nskip) {
			vpi_printf("FST warning: ignoring signals in "
			           "previously scanned scope %s.\n", fullname);
		  } else {
			vcd_names_add(&fst_tab, fullname);
		  }

		    /* Set the file and line information for this scope.
		     * Everything has instance information. Only a module
		     * has separate definition information. */
		  instname = vpi_get_str(vpiFile, item);
		  fstWriterSetSourceInstantiationStem(dump_file, instname,
		      (int)vpi_get(vpiLineNo, item), 0);
		  if (item_type == vpiModule) {
			fstWriterSetSourceStem(dump_file,
			    vpi_get_str(vpiDefFile, item),
			    (int)vpi_get(vpiDefLineNo, item), 0);
		  } else {
			fstWriterSetSourceStem(dump_file, instname,
			    (int)vpi_get(vpiLineNo, item), 0);
		  }

		    /* This must be done before the other name is fetched
		     * and the string must always be freed */
		  if (item_type == vpiModule) {
			defname = strdup(vpi_get_str(vpiDefName, item));
		  }
		  name = vpi_get_str(vpiName, item);
		    /* If the two names match only use the vpiName. */
		  if (defname && (strcmp(defname, name) == 0)) {
			free(defname);
			defname = NULL;
		  }
		  fstWriterSetScope(dump_file, stype, name, defname);
		  free(defname);

		  for (i=0; types[i]>0; i++) {
			vpiHandle hand;
			vpiHandle argv = vpi_iterate(types[i], item);
			while (argv && (hand = vpi_scan(argv))) {
			      scan_item(depth-1, hand, nskip);
			}
		  }

		    /* Sort any signals that we added above. */
		  fstWriterSetUpscope(dump_file);
	    }
	    break;
      }
}

static int draw_scope(vpiHandle item, vpiHandle callh)
{
      int depth;
      const char *name;
      char *defname = NULL;
      PLI_INT32 scope_type, type;

      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope) return 0;

      depth = 1 + draw_scope(scope, callh);

      scope_type = vpi_get(vpiType, scope);
	/* This must be done before the other name is fetched
	 * and the string must always be freed */
      if (scope_type == vpiModule) {
	    defname = strdup(vpi_get_str(vpiDefName, scope));
      }
      name = vpi_get_str(vpiName, scope);
	/* If the two names match only use the vpiName. */
      if (defname && (strcmp(defname, name) == 0)) {
	    free(defname);
	    defname = NULL;
      }

      switch (scope_type) {
	  case vpiNamedBegin:  type = FST_ST_VCD_BEGIN;      break;
	  case vpiFunction:    type = FST_ST_VCD_FUNCTION;   break;
	  case vpiNamedFork:   type = FST_ST_VCD_FORK;       break;
	  case vpiGenScope:    type = FST_ST_VCD_GENERATE;   break;
	  case vpiModule:      type = FST_ST_VCD_MODULE;     break;
	  case vpiTask:        type = FST_ST_VCD_TASK;       break;
	  default:
	    type = FST_ST_VCD_MODULE;
	    vpi_printf("FST Error: %s:%d: $dumpvars: Unsupported scope "
	               "type (%d)\n", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh),
	               (int)vpi_get(vpiType, item));
            assert(0);
      }

      fstWriterSetScope(dump_file, type, name, defname);
      free(defname);

      return depth;
}

static PLI_INT32 sys_dumpvars_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      s_vpi_value value;
      unsigned depth = 0;

      (void)name; /* Parameter is not used. */

      if (dump_file == 0) {
	    open_dumpfile(callh);
	    if (dump_file == 0) {
		  if (argv) vpi_free_object(argv);
		  return 0;
	    }
      }

      if (install_dumpvars_callback()) {
	    if (argv) vpi_free_object(argv);
	    return 0;
      }

        /* Get the depth if it exists. */
      if (argv) {
	    value.format = vpiIntVal;
	    vpi_get_value(vpi_scan(argv), &value);
	    depth = value.value.integer;
      }
      if (!depth) depth = 10000;

        /* This dumps all the instances in the design if none are given. */
      if (!argv || !(item = vpi_scan(argv))) {
	    argv = vpi_iterate(vpiInstance, 0x0);
	    assert(argv);  /* There must be at least one top level instance. */
	    item = vpi_scan(argv);
      }

      for ( ; item; item = vpi_scan(argv)) {
	    char *scname;
	    const char *fullname;
	    int add_var = 0;
	    int dep;
	    PLI_INT32 item_type = vpi_get(vpiType, item);

	      /* If this is a signal make sure it has not already
	       * been included. */
	    switch (item_type) {
	        case vpiIntegerVar:
		case vpiBitVar:
		case vpiByteVar:
		case vpiShortIntVar:
		case vpiIntVar:
		case vpiLongIntVar:
	        case vpiMemoryWord:
	        case vpiNamedEvent:
	        case vpiNet:
	        case vpiParameter:
	        case vpiRealVar:
	        case vpiReg:
	        case vpiTimeVar:
		    /* Warn if the variables scope (which includes the
		     * variable) or the variable itself was already
		     * included. A scope does not automatically include
		     * memory words so do not check the scope for them.  */
		  scname = strdup(vpi_get_str(vpiFullName,
		                              vpi_handle(vpiScope, item)));
		  fullname = vpi_get_str(vpiFullName, item);
		  if (((item_type != vpiMemoryWord) &&
		       vcd_names_search(&fst_tab, scname)) ||
		      vcd_names_search(&fst_var, fullname)) {
		        vpi_printf("FST warning: skipping signal %s, "
		                   "it was previously included.\n",
		                   fullname);
		        free(scname);
		        continue;
		  } else {
		        add_var = 1;
		  }
		  free(scname);
	    }

	    dep = draw_scope(item, callh);

	    scan_item(depth, item, 0);
	      /* The scope list must be sorted after we scan an item.  */
	    vcd_names_sort(&fst_tab);

	    while (dep--) fstWriterSetUpscope(dump_file);

	      /* Add this signal to the variable list so we can verify it
	       * is not included twice. This must be done after it has
	       * been added */
	    if (add_var) {
		  vcd_names_add(&fst_var, vpi_get_str(vpiFullName, item));
		  vcd_names_sort(&fst_var);
	    }
      }

      return 0;
}

void sys_fst_register(void)
{
      int idx;
      struct t_vpi_vlog_info vlog_info;
      s_vpi_systf_data tf_data;
      vpiHandle res;

	/* Scan the extended arguments, looking for fst optimization flags. */
      vpi_get_vlog_info(&vlog_info);

	/* The "speed" option is not used in this dumper. */
      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
	    if (strcmp(vlog_info.argv[idx],"-fst-space") == 0) {
		  lxm_optimum_mode = LXM_SPACE;

	    } else if (strcmp(vlog_info.argv[idx],"-fst-speed") == 0) {
		  lxm_optimum_mode = LXM_SPEED;

	    } else if (strcmp(vlog_info.argv[idx],"-fst-space-speed") == 0) {
		  lxm_optimum_mode = LXM_BOTH;
	    } else if (strcmp(vlog_info.argv[idx],"-fst-speed-space") == 0) {
		  lxm_optimum_mode = LXM_BOTH;
	    }
      }

      /* All the compiletf routines are located in vcd_priv.c. */

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpall";
      tf_data.calltf    = sys_dumpall_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpall";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = sys_one_string_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpflush";
      tf_data.calltf    = sys_dumpflush_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpflush";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumplimit";
      tf_data.calltf    = sys_dumplimit_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumplimit";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpoff";
      tf_data.calltf    = sys_dumpoff_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpoff";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpon";
      tf_data.calltf    = sys_dumpon_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpon";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = sys_dumpvars_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
