/*
 * Copyright (c) 1999-2024 Stephen Williams (steve@icarus.com)
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

/*
 * This file contains the implementations of the VCD related functions.
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
# include  "ivl_alloc.h"

static FILE *dump_file = NULL;
static int   dump_no_date = 0;

static struct t_vpi_time zero_delay = { vpiSimTime, 0, 0, 0.0 };

/*
 * The vcd_list is the list of all the objects that are tracked for
 * dumping. The vcd_checkpoint goes through the list to dump the current
 * values for everything. When the item has a value change, it is added to the
 * vcd_dmp_list for dumping in the current time step.
 *
 * The vcd_const_list is a list of all of the parameters that are being
 * dumped. This list is scanned less often, since parameters do not change
 * values.
 */
DECLARE_VCD_INFO(vcd_info, const char*);
static struct vcd_info *vcd_const_list = NULL;
static struct vcd_info *vcd_list = NULL;
static struct vcd_info *vcd_dmp_list = NULL;

static PLI_UINT64 vcd_cur_time = 0;
static int dump_is_off = 0;
static long dump_limit = 0;
static int dump_is_full = 0;
static int finish_status = 0;


static const char*units_names[] = {
      "s",
      "ms",
      "us",
      "ns",
      "ps",
      "fs"
};

static char vcdid[8] = "!";

static void gen_new_vcd_id(void)
{
      static unsigned value = 0;
      unsigned v = ++value;
      unsigned int i;

      for (i=0; i < sizeof(vcdid)-1; i++) {
           vcdid[i] = (char)((v%94)+33); /* for range 33..126 */
           v /= 94;
           if(!v) {
                 vcdid[i+1] = '\0';
                 return;
           }
      }
	// This should never happen since 94**7 is a lot if identifiers!
      assert(0);
}

static char *truncate_bitvec(char *s)
{
      char r;

      r=*s;
      if(r=='1') return s;
      else s += 1;

      for(;;s++) {
	    char l;
	    l=r; r=*s;
	    if(!r) return (s-1);
	    if(l!=r) return(((l=='0')&&(r=='1'))?s:s-1);
      }
}

static void show_this_item(struct vcd_info*info)
{
      s_vpi_value value;
      PLI_INT32 type = vpi_get(vpiType, info->item);

      if (type == vpiRealVar) {
	    value.format = vpiRealVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "r%.16g %s\n", value.value.real, info->ident);
      } else if (type == vpiNamedEvent) {
	    fprintf(dump_file, "1%s\n", info->ident);
      } else if (type == vpiParameter && vpi_get(vpiConstType, info->item) == vpiRealConst) {

	    value.format = vpiRealVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "r%.16g %s\n", value.value.real, info->ident);
      } else if (vpi_get(vpiSize, info->item) == 1) {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "%s%s\n", value.value.str, info->ident);
      } else {
	    value.format = vpiBinStrVal;
	    vpi_get_value(info->item, &value);
	    fprintf(dump_file, "b%s %s\n", truncate_bitvec(value.value.str),
		    info->ident);
      }
}

/* Dump values for a $dumpoff. */
static void show_this_item_x(struct vcd_info*info)
{
      PLI_INT32 type = vpi_get(vpiType, info->item);

      if (type == vpiRealVar) {
	      /* Some tools dump nothing here...? */
	    fprintf(dump_file, "rNaN %s\n", info->ident);
      } else if (type == vpiNamedEvent) {
	    /* Do nothing for named events. */
      } else if (vpi_get(vpiSize, info->item) == 1) {
	    fprintf(dump_file, "x%s\n", info->ident);
      } else {
	    fprintf(dump_file, "bx %s\n", info->ident);
      }
}


/*
 * managed qsorted list of scope names/variables for duplicates bsearching
 */

struct vcd_names_list_s vcd_tab = { 0, 0, 0, 0 };
struct vcd_names_list_s vcd_var = { 0, 0, 0, 0 };


static PLI_UINT64 dumpvars_time;
__inline__ static int dump_header_pending(void)
{
      return dumpvars_status != 2;
}


static PLI_INT32 variable_cb_2(p_cb_data cause)
{
      struct vcd_info* info = vcd_dmp_list;
      PLI_UINT64 now = timerec_to_time64(cause->time);

      if (now != vcd_cur_time) {
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", now);
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

      if ((dump_limit > 0) && (ftell(dump_file) > dump_limit)) {
            dump_is_full = 1;
            vpi_printf("WARNING: Dump file limit (%ld bytes) "
                               "exceeded.\n", dump_limit);
            fprintf(dump_file, "$comment Dump file limit (%ld bytes) "
                               "exceeded. $end\n", dump_limit);
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

/*
 * This is called at the end of the timestep where the $dumpvars task is
 * called. This allows for values to settle for the timestep, so that the
 * checkpoint gets the current values.
 */
static PLI_INT32 dumpvars_cb(p_cb_data cause)
{
      if (dumpvars_status != 1) return 0;

      dumpvars_status = 2;

      dumpvars_time = timerec_to_time64(cause->time);
      vcd_cur_time = dumpvars_time;

      fprintf(dump_file, "$enddefinitions $end\n");

      if (!dump_is_off) {
	    fprintf(dump_file, "$comment Show the parameter values. $end\n");
	    fprintf(dump_file, "$dumpall\n");
	    ITERATE_VCD_INFO(vcd_const_list, vcd_info, next, show_this_item);
	    fprintf(dump_file, "$end\n");

	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);

	    fprintf(dump_file, "$dumpvars\n");
	    ITERATE_VCD_INFO(vcd_list, vcd_info, next, show_this_item);
	    fprintf(dump_file, "$end\n");
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
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);
      }

      fclose(dump_file);

      for (cur = vcd_list ;  cur ;  cur = next) {
	    next = cur->next;
	    free((char *)cur->ident);
	    free(cur);
      }
      vcd_list = 0;
      for (cur = vcd_const_list ; cur ; cur = next) {
	    next = cur->next;
	    free((char *)cur->ident);
	    free(cur);
      }
      vcd_const_list = 0;
      vcd_names_delete(&vcd_tab);
      vcd_names_delete(&vcd_var);
      nexus_ident_delete();
      vcd_free_dump_path();

      return 0;
}

__inline__ static int install_dumpvars_callback(void)
{
      struct t_cb_data cb;

      if (dumpvars_status == 1) return 0;

      if (dumpvars_status == 2) {
	    vpi_printf("VCD warning: $dumpvars ignored, previously"
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
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", now64);
	    vcd_cur_time = now64;
      }

      fprintf(dump_file, "$dumpoff\n");
      ITERATE_VCD_INFO(vcd_list, vcd_info, next, show_this_item_x);
      fprintf(dump_file, "$end\n");

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
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", now64);
	    vcd_cur_time = now64;
      }

      fprintf(dump_file, "$dumpon\n");
      ITERATE_VCD_INFO(vcd_list, vcd_info, next, show_this_item);
      fprintf(dump_file, "$end\n");

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
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", now64);
	    vcd_cur_time = now64;
      }

      fprintf(dump_file, "$dumpall\n");
      ITERATE_VCD_INFO(vcd_list, vcd_info, next, show_this_item);
      fprintf(dump_file, "$end\n");

      return 0;
}

static void open_dumpfile(vpiHandle callh)
{
      char* use_dump_path = vcd_get_dump_path("vcd");

      dump_file = fopen(use_dump_path, "w");

      if (dump_file == 0) {
	    vpi_printf("VCD Error: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open %s for output.\n", use_dump_path);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    vcd_free_dump_path();
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);
	    unsigned scale = 1;
	    unsigned udx = 0;
	    time_t walltime;

	    vpi_printf("VCD info: dumpfile %s opened for output.\n",
	               use_dump_path);

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

		if (!dump_no_date) {
			fprintf(dump_file, "$date\n");
			fprintf(dump_file, "\t%s",asctime(localtime(&walltime)));
			fprintf(dump_file, "$end\n");
		}
	    fprintf(dump_file, "$version\n");
	    fprintf(dump_file, "\tIcarus Verilog\n");
	    fprintf(dump_file, "$end\n");
	    fprintf(dump_file, "$timescale\n");
	    fprintf(dump_file, "\t%u%s\n", scale, units_names[udx]);
	    fprintf(dump_file, "$end\n");
      }
}

static PLI_INT32 sys_dumpfile_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void) name;
      return sys_dumpfile_common("VCD", "vcd");
}

static PLI_INT32 sys_dumpflush_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      (void)name; /* Parameter is not used. */
      if (dump_file) fflush(dump_file);

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

      vpi_free_object(argv);
      return 0;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      static int dumpable_types[] = {
            /* Value */
            vpiNamedEvent,
            vpiNet,
            vpiParameter,
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

      struct t_cb_data cb;
      struct vcd_info* info;

      const char *type;
      const char *name;
      const char *fullname;
      const char *prefix;
      const char *ident;
      int nexus_id;
      unsigned size;
      PLI_INT32 item_type;

	/* Get the displayed type for the various $var and $scope types. */
	/* Not all of these are supported now, but they should be in a
	 * future development version. */
      item_type = vpi_get(vpiType, item);
      switch (item_type) {
	  case vpiNamedEvent: type = "event"; break;
	  case vpiIntVar:
	  case vpiIntegerVar: type = "integer"; break;
	    /* VCD doesn't support real parameters, so lie. */
	  case vpiParameter:
	    switch (vpi_get(vpiConstType, item)) {
		case vpiRealConst: type = "real"; break;
		default: type = "parameter"; break;
	    }
	    break;
	    /* Icarus converts realtime to real. */
	  case vpiRealVar:    type = "real"; break;
	  case vpiMemoryWord:
	  case vpiBitVar:
	  case vpiByteVar:
	  case vpiShortIntVar:
	  case vpiLongIntVar:
	  case vpiReg:        type = "reg"; break;
	    /* Icarus converts a time to a plain register. */
	  case vpiTimeVar:    type = "time"; break;
	  case vpiNet:
	    switch (vpi_get(vpiNetType, item)) {
		case vpiWand:    type = "wand"; break;
		case vpiWor:     type = "wor"; break;
		case vpiTri:     type = "tri"; break;
		case vpiTri0:    type = "tri0"; break;
		case vpiTri1:    type = "tri1"; break;
		case vpiTriReg:  type = "trireg"; break;
		case vpiTriAnd:  type = "triand"; break;
		case vpiTriOr:   type = "trior"; break;
		case vpiSupply1: type = "supply1"; break;
		case vpiSupply0: type = "supply0"; break;
		default:         type = "wire"; break;
	    }
	    break;

	  case vpiNamedBegin: type = "begin"; break;
	  case vpiGenScope:   type = "begin"; break;
	  case vpiNamedFork:  type = "fork"; break;
	  case vpiFunction:   type = "function"; break;
	  case vpiModule:     type = "module"; break;
	  case vpiPackage:    type = "package"; break;
	  case vpiTask:       type = "task"; break;

	  default:
	    vpi_printf("VCD warning: $dumpvars: Unsupported argument "
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
		  vpi_printf("VCD warning: array word %s will conflict "
		             "with an escaped identifier.\n",
		             vpi_get_str(vpiFullName, item));
            }
      }

      fullname = vpi_get_str(vpiFullName, item);

	/* Generate the $var or $scope commands. */
      switch (item_type) {
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
	    if (vcd_names_search(&vcd_var, fullname)) return;

	      /* Declare the variable in the VCD file. */
	    name = vpi_get_str(vpiName, item);
	    prefix = is_escaped_id(name) ? "\\" : "";

	      /* Some signals can have an alias so handle that. */
	    nexus_id = vpi_get(_vpiNexusId, item);

	    ident = 0;
	    if (nexus_id) ident = find_nexus_ident(nexus_id);

	    if (!ident) {
		  ident = strdup(vcdid);
		  gen_new_vcd_id();

		  if (nexus_id) set_nexus_ident(nexus_id, ident);

		    /* Add a callback for the signal. */
		  info = malloc(sizeof(*info));

		  info->time.type = vpiSimTime;
		  info->item  = item;
		  info->ident = ident;
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

	      /* Named events do not have a size, but other tools use
	       * a size of 1 and some viewers do not accept a width of
	       * zero so we will also use a width of one for events. */
	    if (item_type == vpiNamedEvent) size = 1;
	    else size = vpi_get(vpiSize, item);

	    fprintf(dump_file, "$var %s %u %s %s%s",
		    type, size, ident, prefix, name);

	      /* Add a range for vectored values. */
	    if (size > 1 || vpi_get(vpiLeftRange, item) != 0) {
		  fprintf(dump_file, " [%i:%i]",
			  (int)vpi_get(vpiLeftRange, item),
			  (int)vpi_get(vpiRightRange, item));
	    }

	    fprintf(dump_file, " $end\n");
	    break;

	  case vpiParameter:

	    /* If we are skipping all pamaeters then just return. */
            if (skip) return;

	    size = vpi_get(vpiSize, item);

	    /* Declare the parameter in the VCD file. */
	    name = vpi_get_str(vpiName, item);
	    prefix = is_escaped_id(name) ? "\\" : "";

	    ident = strdup(vcdid);
	    gen_new_vcd_id();

	    /* Make an info item to go in the vcd_const_list. */
	    info = malloc(sizeof(*info));
	    info->item = item;
	    info->ident = ident;
	    info->scheduled = 0;
	    info->dmp_next = 0;
	    info->next = vcd_const_list;
	    vcd_const_list = info;
	    info->cb = NULL;

	    /* Generate the $var record. Now the parameter is declared. */
	    fprintf(dump_file, "$var %s %u %s %s%s $end\n",
		    type, size, ident, prefix, name);
	    break;

	  case vpiModule:
	  case vpiGenScope:
	  case vpiFunction:
	  case vpiTask:
	  case vpiNamedBegin:
	  case vpiNamedFork:

	    if (depth > 0) {
		  int i;
		  int nskip = (vcd_names_search(&vcd_tab, fullname) != 0);

		    /* We have to always scan the scope because the
		     * depth could be different for this call. */
		  if (nskip) {
			vpi_printf("VCD warning: ignoring signals in "
			           "previously scanned scope %s.\n", fullname);
		  } else {
			vcd_names_add(&vcd_tab, fullname);
		  }

		  name = vpi_get_str(vpiName, item);
		  fprintf(dump_file, "$scope %s %s $end\n", type, name);

		  for (i=0; dumpable_types[i]>0; i++) {
			vpiHandle hand;
			vpiHandle argv = vpi_iterate(dumpable_types[i], item);
			while (argv && (hand = vpi_scan(argv))) {
			      scan_item(depth-1, hand, nskip);
			}
		  }

		    /* Sort any signals that we added above. */
		  fprintf(dump_file, "$upscope $end\n");
	    }
	    break;

	  case vpiPackage:
	      // Don't print a warning for empty packages.
	    if (vcd_instance_contains_dumpable_items(dumpable_types, item))
		  vpi_printf("VCD warning: $dumpvars: Package (%s) is not dumpable "
			     "with VCD.\n", vpi_get_str(vpiFullName, item));
	    break;
      }
}

static int draw_scope(vpiHandle item, vpiHandle callh)
{
      int depth;
      const char *name;
      const char *type;

      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope) return 0;

      depth = 1 + draw_scope(scope, callh);
      name = vpi_get_str(vpiName, scope);

      switch (vpi_get(vpiType, scope)) {
	  case vpiNamedBegin:  type = "begin";      break;
	  case vpiGenScope:    type = "begin";      break;
	  case vpiTask:        type = "task";       break;
	  case vpiFunction:    type = "function";   break;
	  case vpiNamedFork:   type = "fork";       break;
	  case vpiModule:      type = "module";     break;
	  default:
	    type = "invalid";
	    vpi_printf("VCD Error: %s:%d: $dumpvars: Unsupported scope "
	               "type (%d)\n", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh),
	               (int)vpi_get(vpiType, item));
            assert(0);
      }

      fprintf(dump_file, "$scope %s %s $end\n", type, name);

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
		       vcd_names_search(&vcd_tab, scname)) ||
		      vcd_names_search(&vcd_var, fullname)) {
		        vpi_printf("VCD warning: skipping signal %s, "
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
	    vcd_names_sort(&vcd_tab);

	    while (dep--) fprintf(dump_file, "$upscope $end\n");

	      /* Add this signal to the variable list so we can verify it
	       * is not included twice. This must be done after it has
	       * been added */
	    if (add_var) {
		  vcd_names_add(&vcd_var, vpi_get_str(vpiFullName, item));
		  vcd_names_sort(&vcd_var);
	    }
      }

      return 0;
}

void sys_vcd_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;
      int idx;
      struct t_vpi_vlog_info vlog_info;

      /* Scan the extended arguments */
      vpi_get_vlog_info(&vlog_info);

      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
        if (strcmp(vlog_info.argv[idx],"-no-date") == 0) {
          dump_no_date = 1;
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
