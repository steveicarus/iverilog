/*
 * Copyright (c) 1999-2008 Stephen Williams (steve@icarus.com)
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

# include "sys_priv.h"

/*
 * This file contains the implementations of the VCD related
 * funcitons.
 */

# include  "vpi_user.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <time.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  "vcd_priv.h"

static char*dump_path = 0;
static FILE*dump_file = 0;

static const char*units_names[] = {
      "s",
      "ms",
      "us",
      "ns",
      "ps",
      "fs"
};

struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct t_vpi_time time;
      const char*ident;
      struct vcd_info* next;
      struct vcd_info* dmp_next;
      int scheduled;
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
                 break;
           }
      }
}

static struct vcd_info *vcd_list = 0;
static struct vcd_info *vcd_dmp_list = 0;
PLI_UINT64 vcd_cur_time = 0;
static int dump_is_off = 0;
static long dump_limit = 0;
static int dump_is_full = 0;
static int finish_status = 0;

static char *truncate_bitvec(char *s)
{
      char l, r;

      r=*s;
      if(r=='1') return s;
      else s += 1;

      for(;;s++) {
	    l=r; r=*s;
	    if(!r) return (s-1);
	    if(l!=r) return(((l=='0')&&(r=='1'))?s:s-1);
      }
}

static void show_this_item(struct vcd_info*info)
{
      s_vpi_value value;

      if (vpi_get(vpiType, info->item) == vpiRealVar) {
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


static void show_this_item_x(struct vcd_info*info)
{
      if (vpi_get(vpiType, info->item) == vpiRealVar) {
	      /* Some tools dump nothing here...? */
	    fprintf(dump_file, "rNaN %s\n", info->ident);
      } else if (vpi_get(vpiSize, info->item) == 1) {
	    fprintf(dump_file, "x%s\n", info->ident);
      } else {
	    fprintf(dump_file, "bx %s\n", info->ident);
      }
}


/*
 * managed qsorted list of scope names/variables for duplicates bsearching
 */

struct vcd_names_list_s vcd_tab = { 0 };
struct vcd_names_list_s vcd_var = { 0 };


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
static void vcd_checkpoint()
{
      struct vcd_info*cur;

      for (cur = vcd_list ;  cur ;  cur = cur->next)
	    show_this_item(cur);
}

static void vcd_checkpoint_x()
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

      fprintf(dump_file, "$enddefinitions $end\n");

      if (!dump_is_off) {
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);
	    fprintf(dump_file, "$dumpvars\n");
	    vcd_checkpoint();
	    fprintf(dump_file, "$end\n");
      }

      return 0;
}

static PLI_INT32 finish_cb(p_cb_data cause)
{
      if (finish_status != 0) return 0;

      finish_status = 1;

      dumpvars_time = timerec_to_time64(cause->time);

      if (!dump_is_off && !dump_is_full && dumpvars_time != vcd_cur_time) {
	    fprintf(dump_file, "#%" PLI_UINT64_FMT "\n", dumpvars_time);
      }

      return 0;
}

__inline__ static int install_dumpvars_callback(void)
{
      struct t_cb_data cb;
      static struct t_vpi_time now;

      if (dumpvars_status == 1) return 0;

      if (dumpvars_status == 2) {
	    vpi_printf("VCD warning: $dumpvars ignored, previously"
	               " called at simtime %" PLI_UINT64_FMT "\n",
	               dumpvars_time);
	    return 1;
      }

      now.type = vpiSimTime;
      cb.time = &now;
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

static PLI_INT32 sys_dumpoff_calltf(PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

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
      vcd_checkpoint_x();
      fprintf(dump_file, "$end\n");

      return 0;
}

static PLI_INT32 sys_dumpon_calltf(PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

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
      vcd_checkpoint();
      fprintf(dump_file, "$end\n");

      return 0;
}

static PLI_INT32 sys_dumpall_calltf(PLI_BYTE8*name)
{
      s_vpi_time now;
      PLI_UINT64 now64;

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
      vcd_checkpoint();
      fprintf(dump_file, "$end\n");

      return 0;
}

static void open_dumpfile(vpiHandle callh)
{
      if (dump_path == 0) dump_path = strdup("dump.vcd");

      dump_file = fopen(dump_path, "w");

      if (dump_file == 0) {
	    vpi_printf("VCD Error: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open %s for output.\n", dump_path);
	    vpi_control(vpiFinish, 1);
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);
	    unsigned scale = 1;
	    unsigned udx = 0;
	    time_t walltime;

	    vpi_printf("VCD info: dumpfile %s opened for output.\n", dump_path);

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

	    fprintf(dump_file, "$date\n");
	    fprintf(dump_file, "\t%s",asctime(localtime(&walltime)));
	    fprintf(dump_file, "$end\n");
	    fprintf(dump_file, "$version\n");
	    fprintf(dump_file, "\tIcarus Verilog\n");
	    fprintf(dump_file, "$end\n");
	    fprintf(dump_file, "$timescale\n");
	    fprintf(dump_file, "\t%u%s\n", scale, units_names[udx]);
	    fprintf(dump_file, "$end\n");
      }
}

static PLI_INT32 sys_dumpfile_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value value;

      char*path;

        /* $dumpfile must be called before $dumpvars starts! */
      if (dumpvars_status != 0) {
	    vpi_printf("VCD warning: %s called after $dumpvars started,\n"
	               "             using existing file (%s).\n",
	               name, dump_path);
	    return 0;
      }

      assert(argv);
      value.format = vpiStringVal;
      vpi_get_value(vpi_scan(argv), &value);
      path = strdup(value.value.str);

      if (dump_path) {
	    vpi_printf("VCD warning: Overriding dump file %s with %s\n",
	               dump_path, path);
	    free(dump_path);
      }
      dump_path = path;

      vpi_free_object(argv);
      return 0;
}

static PLI_INT32 sys_dumpflush_calltf(PLI_BYTE8*name)
{
      if (dump_file) fflush(dump_file);

      return 0;
}

static PLI_INT32 sys_dumplimit_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      s_vpi_value val;

      /* Get the value and set the dump limit. */
      assert(argv);
      val.format = vpiIntVal;
      vpi_get_value(vpi_scan(argv), &val);
      dump_limit = val.value.integer;

      vpi_free_object(argv);
      return 0;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      struct t_cb_data cb;
      struct vcd_info* info;

      const char* type;
      const char* name;
      const char* prefix;
      const char* ident;
      int nexus_id;

      /* list of types to iterate upon */
      int i;
      static int types[] = {
	    /* Value */
	    vpiNet,
	    vpiReg,
	    vpiVariables,
	    /* Scope */
	    vpiFunction,
	    vpiModule,
	    vpiNamedBegin,
	    vpiNamedFork,
	    vpiTask,
	    -1
      };

      switch (vpi_get(vpiType, item)) {

	  case vpiNet:  type = "wire";    if(0){
	  case vpiIntegerVar:
	  case vpiMemoryWord:
	  case vpiTimeVar:
	  case vpiReg:  type = "reg";    }

	      /* Skip this signal if it has already been included. */
	    if (vcd_names_search(&vcd_var, vpi_get_str(vpiFullName, item))) {
		  vpi_printf("VCD warning: skipping signal %s, it was "
		             "previously included.\n",
		             vpi_get_str(vpiFullName, item));
		  break;
	    }

            if (skip || vpi_get(vpiAutomatic, item)) break;

	    name = vpi_get_str(vpiName, item);
	    prefix = is_escaped_id(name) ? "\\" : "";

	    nexus_id = vpi_get(_vpiNexusId, item);

	    if (nexus_id) {
		  ident = find_nexus_ident(nexus_id);
	    } else {
		  ident = 0;
	    }

	    if (!ident) {
		  ident = strdup(vcdid);
		  gen_new_vcd_id();

		  if (nexus_id) set_nexus_ident(nexus_id, ident);

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


		  info->next      = vcd_list;
		  info->dmp_next  = 0;
		  vcd_list    = info;

		  info->cb    = vpi_register_cb(&cb);
	    }

	    fprintf(dump_file, "$var %s %u %s %s%s",
		    type, vpi_get(vpiSize, item), ident,
		    prefix, name);
      /* FIXME
	    if (vpi_get(vpiVector, item)
      */
	    if (vpi_get(vpiSize, item) > 1
		|| vpi_get(vpiLeftRange, item) != 0) {
		  fprintf(dump_file, " [%i:%i]", vpi_get(vpiLeftRange, item),
			  vpi_get(vpiRightRange, item));
	    }
	    fprintf(dump_file, " $end\n");
	    break;

	  case vpiRealVar:

	      /* Skip this signal if it has already been included. */
	    if (vcd_names_search(&vcd_var, vpi_get_str(vpiFullName, item))) {
		  vpi_printf("VCD warning: skipping signal %s, it was "
		             "previously included.\n",
		             vpi_get_str(vpiFullName, item));
		  break;
	    }

            if (skip || vpi_get(vpiAutomatic, item)) break;

	      /* Declare the variable in the VCD file. */
	    name = vpi_get_str(vpiName, item);
	    prefix = is_escaped_id(name) ? "\\" : "";
	    ident = strdup(vcdid);
	    gen_new_vcd_id();
	    fprintf(dump_file, "$var real 1 %s %s%s $end\n",
		    ident, prefix, name);

	      /* Add a callback for the variable. */
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

	    info->next  = vcd_list;
	    info->dmp_next  = 0;
	    vcd_list    = info;

	    info->cb    = vpi_register_cb(&cb);

	    break;

	  case vpiModule:      type = "module";      if(0){
	  case vpiNamedBegin:  type = "begin";      }if(0){
	  case vpiTask:        type = "task";       }if(0){
	  case vpiFunction:    type = "function";   }if(0){
	  case vpiNamedFork:   type = "fork";       }

	    if (depth > 0) {
		  int nskip;
		  vpiHandle argv;

		  const char* fullname =
			vpi_get_str(vpiFullName, item);

#if 0
		  vpi_printf("VCD info: scanning scope %s, %u levels\n",
		             fullname, depth);
#endif
		  nskip = 0 != vcd_names_search(&vcd_tab, fullname);

		  if (!nskip)
			vcd_names_add(&vcd_tab, fullname);
		  else
		    vpi_printf("VCD warning: ignoring signals in "
		               "previously scanned scope %s\n", fullname);

		  name = vpi_get_str(vpiName, item);

		  fprintf(dump_file, "$scope %s %s $end\n", type, name);

		  for (i=0; types[i]>0; i++) {
			vpiHandle hand;
			argv = vpi_iterate(types[i], item);
			while (argv && (hand = vpi_scan(argv))) {
			      scan_item(depth-1, hand, nskip);
			}
		  }

		  fprintf(dump_file, "$upscope $end\n");
	    }
	    break;

	  default:
	    vpi_printf("VCD warning: $dumpvars: Unsupported parameter "
	               "type (%s)\n", vpi_get_str(vpiType, item));
      }
}

static int draw_scope(vpiHandle item, vpiHandle callh)
{
      int depth;
      const char *name;
      char *type;

      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope) return 0;

      depth = 1 + draw_scope(scope, callh);
      name = vpi_get_str(vpiName, scope);

      switch (vpi_get(vpiType, scope)) {
	  case vpiNamedBegin:  type = "begin";      break;
	  case vpiTask:        type = "task";       break;
	  case vpiFunction:    type = "function";   break;
	  case vpiNamedFork:   type = "fork";       break;
	  case vpiModule:      type = "module";     break;
	  default:
	    vpi_printf("VCD Error: %s:%d: $dumpvars: Unsupported scope "
	               "type (%d)\n", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh), vpi_get(vpiType, item));
            assert(0);
      }

      fprintf(dump_file, "$scope %s %s $end\n", type, name);

      return depth;
}

static PLI_INT32 sys_dumpvars_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle item;
      s_vpi_value value;
      unsigned depth = 0;

      if (dump_file == 0) {
	    open_dumpfile(callh);
	    if (dump_file == 0) return 0;
      }

      if (install_dumpvars_callback()) return 0;

        /* Get the depth if it exists. */
      if (argv) {
	    value.format = vpiIntVal;
	    vpi_get_value(vpi_scan(argv), &value);
	    depth = value.value.integer;
      }
      if (!depth) depth = 10000;

        /* This dumps all the modules in the design if none are given. */
      if (!argv || !(item = vpi_scan(argv))) {
	    argv = vpi_iterate(vpiModule, 0x0);
	    assert(argv);  /* There must be at least one top level module. */
	    item = vpi_scan(argv);
      }

      for ( ; item; item = vpi_scan(argv)) {
	    const char *scname;
	    int add_var = 0;
	    int dep;

	    vcd_names_sort(&vcd_tab);

	      /* If this is a signal make sure it has not already
	       * been included. */
	    switch (vpi_get(vpiType, item)) {
	        case vpiIntegerVar:
	        case vpiMemoryWord:
	        case vpiNet:
	        case vpiRealVar:
	        case vpiReg:
	        case vpiTimeVar:
		  scname = vpi_get_str(vpiFullName, vpi_handle(vpiScope, item));
		  if (vcd_names_search(&vcd_tab, scname)) {
		        vpi_printf("VCD warning: skipping signal %s, "
		                   "it was previously included.\n",
		                   vpi_get_str(vpiFullName, item));
		        continue;
		  } else {
		        add_var = 1;
		  }
	    }

	    dep = draw_scope(item, callh);

	    scan_item(depth, item, 0);

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

void sys_vcd_register()
{
      s_vpi_systf_data tf_data;

      /* All the compiletf routines are located in vcd_priv.c. */

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpall";
      tf_data.calltf    = sys_dumpall_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpall";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = sys_one_string_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpflush";
      tf_data.calltf    = sys_dumpflush_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpflush";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumplimit";
      tf_data.calltf    = sys_dumplimit_calltf;
      tf_data.compiletf = sys_one_numeric_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumplimit";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpoff";
      tf_data.calltf    = sys_dumpoff_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpoff";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpon";
      tf_data.calltf    = sys_dumpon_calltf;
      tf_data.compiletf = sys_no_arg_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpon";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = sys_dumpvars_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      vpi_register_systf(&tf_data);
}
