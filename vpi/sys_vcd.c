/*
 * Copyright (c) 1999-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sys_vcd.c,v 1.44 2003/05/15 16:51:09 steve Exp $"
#endif

# include "config.h"
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
      int i;

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
unsigned long vcd_cur_time = 0;
static int dump_is_off = 0;

static char *truncate_bitvec(char *s)
{
      char l, r;

      r=*s;
      if(r=='1')
	    return s;
      else
	    s += 1;
            
      for(;;s++) { 
	    l=r; r=*s;
	    if(!r) return (s-1);
                 
	    if(l!=r)
		  return(((l=='0')&&(r='1'))?s:s-1);

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
	    fprintf(dump_file, "b%s %s\n",
		    truncate_bitvec(value.value.str),
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
 * managed qsorted list of scope names for duplicates bsearching
 */

struct vcd_names_list_s vcd_tab = { 0 };


static int dumpvars_status = 0; /* 0:fresh 1:cb installed, 2:callback done */
static unsigned long dumpvars_time;
inline static int dump_header_pending(void)
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

static int variable_cb_2(p_cb_data cause)
{
      struct vcd_info* info = vcd_dmp_list;
      unsigned long now = cause->time->low;

      if (now != vcd_cur_time) {
	    fprintf(dump_file, "#%lu\n", now);
	    vcd_cur_time = now;
      }

      do {
           show_this_item(info);
           info->scheduled = 0;
      } while ((info = info->dmp_next) != 0);

      vcd_dmp_list = 0;

      return 0;
}

static int variable_cb_1(p_cb_data cause)
{
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

      if (dump_is_off) 		 return 0;
      if (dump_header_pending()) return 0;
      if (info->scheduled)       return 0;

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

static int dumpvars_cb(p_cb_data cause)
{
      if (dumpvars_status != 1)
	    return 0;

      dumpvars_status = 2;

      dumpvars_time = cause->time->low;
      vcd_cur_time = dumpvars_time;

      fprintf(dump_file, "$enddefinitions $end\n");

      if (!dump_is_off) {
	    fprintf(dump_file, "#%lu\n", dumpvars_time);
	    fprintf(dump_file, "$dumpvars\n");
	    vcd_checkpoint();
	    fprintf(dump_file, "$end\n");
      }

      return 0;
}

inline static int install_dumpvars_callback(void)
{
      struct t_cb_data cb;
      static struct t_vpi_time time;

      if (dumpvars_status == 1)
	    return 0;

      if (dumpvars_status == 2) {
	    vpi_mcd_printf(1, "VCD Error:"
			   " $dumpvars ignored,"
			   " previously called at simtime %lu\n",
			   dumpvars_time);
	    return 1;
      }

      time.type = vpiSimTime;
      cb.time = &time;
      cb.reason = cbReadOnlySynch;
      cb.cb_rtn = dumpvars_cb;
      cb.user_data = 0x0;
      cb.obj = 0x0;

      vpi_register_cb(&cb);

      dumpvars_status = 1;
      return 0;
}

static int sys_dumpoff_calltf(char*name)
{
      s_vpi_time now;

      if (dump_is_off)
	    return 0;

      dump_is_off = 1;

      if (dump_file == 0)
	    return 0;

      if (dump_header_pending())
	    return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      if (now.low > vcd_cur_time)
	    fprintf(dump_file, "#%u\n", now.low);
      vcd_cur_time = now.low;

      fprintf(dump_file, "$dumpoff\n");
      vcd_checkpoint_x();
      fprintf(dump_file, "$end\n");

      return 0;
}

static int sys_dumpon_calltf(char*name)
{
      s_vpi_time now;

      if (!dump_is_off)
	    return 0;

      dump_is_off = 0;

      if (dump_file == 0)
	    return 0;

      if (dump_header_pending())
	    return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      if (now.low > vcd_cur_time)
	    fprintf(dump_file, "#%u\n", now.low);
      vcd_cur_time = now.low;

      fprintf(dump_file, "$dumpon\n");
      vcd_checkpoint();
      fprintf(dump_file, "$end\n");

      return 0;
}

static int sys_dumpall_calltf(char*name)
{
      s_vpi_time now;

      if (dump_file == 0)
	    return 0;

      if (dump_header_pending())
	    return 0;

      now.type = vpiSimTime;
      vpi_get_time(0, &now);
      if (now.low > vcd_cur_time)
	    fprintf(dump_file, "#%u\n", now.low);
      vcd_cur_time = now.low;

      fprintf(dump_file, "$dumpall\n");
      vcd_checkpoint();
      fprintf(dump_file, "$end\n");

      return 0;
}

static void open_dumpfile(const char*path)
{
      dump_file = fopen(path, "w");

      if (dump_file == 0) {
	    vpi_mcd_printf(1, 
			   "VCD Error: Unable to open %s for output.\n", 
			   path);
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);
	    unsigned scale = 1;
	    unsigned udx = 0;
	    time_t walltime;

	    vpi_mcd_printf(1, 
			   "VCD info: dumpfile %s opened for output.\n", 
			   path);
	    
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

static int sys_dumpfile_calltf(char*name)
{
      char*path;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);
      vpiHandle item;

      if (argv && (item = vpi_scan(argv))) {
	    s_vpi_value value;

	    if (vpi_get(vpiType, item) != vpiConstant
		|| vpi_get(vpiConstType, item) != vpiStringConst) {
		  vpi_mcd_printf(1, 
				 "VCD Error:"
				 " %s parameter must be a string constant\n", 
				 name);
		  return 0;
	    }

	    value.format = vpiStringVal;
	    vpi_get_value(item, &value);
	    path = strdup(value.value.str);

	    vpi_free_object(argv);

      } else {
	    path = strdup("dumpfile.vcd");
      }

      if (dump_file) {
	    fclose(dump_file);
	    dump_file = 0;
      }

      assert(dump_file == 0);
      open_dumpfile(path);

      free(path);

      return 0;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      struct t_cb_data cb;
      struct vcd_info* info;

      const char* type;
      const char* name;
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

	  case vpiMemory:
	      /* don't know how to watch memories. */
	    break;

	  case vpiNamedEvent:
	      /* There is nothing in named events to dump. */
	    break;

	  case vpiNet:  type = "wire";    if(0){
	  case vpiIntegerVar:
	  case vpiTimeVar:
	  case vpiReg:  type = "reg";    }

	    if (skip)
		  break;
	    
	    name = vpi_get_str(vpiName, item);
	    
	    nexus_id = vpi_get(_vpiNexusId, item);

	    if (nexus_id) {
		  ident = find_nexus_ident(nexus_id);
	    } else {
		  ident = 0;
	    }
	    
	    if (!ident) {
		  ident = strdup(vcdid);
		  gen_new_vcd_id();
		  
		  if (nexus_id)
			set_nexus_ident(nexus_id, ident);
		  
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
	    
	    fprintf(dump_file, "$var %s %u %s %s $end\n",
		    type, vpi_get(vpiSize, item), ident,
		    name);
	    break;

	  case vpiRealVar:

	    if (skip)
		  break;

	      /* Declare the variable in the VCD file. */
	    name = vpi_get_str(vpiName, item);
	    ident = strdup(vcdid);
	    gen_new_vcd_id();
	    fprintf(dump_file, "$var real 1 %s %s $end\n",
		    ident, name);

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
		  vpi_mcd_printf(1, 
				 "VCD info:"
				 " scanning scope %s, %u levels\n",
				 fullname, depth);
#endif
		  nskip = 0 != vcd_names_search(&vcd_tab, fullname);
		  
		  if (!nskip) 
			vcd_names_add(&vcd_tab, fullname);
		  else 
		    vpi_mcd_printf(1,
				   "VCD warning:"
				   " ignoring signals"
				   " in previously scanned scope %s\n",
				   fullname);

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
	    vpi_mcd_printf(1,
			   "VCD Error: $dumpvars: Unsupported parameter "
			   "type (%d)\n", vpi_get(vpiType, item));
      }
}

static int draw_scope(vpiHandle item)
{
      int depth;
      const char *name;
      char *type;

      vpiHandle scope = vpi_handle(vpiScope, item);
      if (!scope)
	    return 0;
      
      depth = 1 + draw_scope(scope);
      name = vpi_get_str(vpiName, scope);

      switch (vpi_get(vpiType, item)) {
	  case vpiNamedBegin:  type = "begin";      break;
	  case vpiTask:        type = "task";       break;
	  case vpiFunction:    type = "function";   break;
	  case vpiNamedFork:   type = "fork";       break;
      	  default:             type = "module";     break;
      }
      
      fprintf(dump_file, "$scope %s %s $end\n", type, name);

      return depth;
}

/*
 * This function is also used in sys_lxt to check the arguments of the
 * lxt variant of $dumpvars.
 */
int sys_vcd_dumpvars_compiletf(char*name)
{
      vpiHandle sys   = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv  = vpi_iterate(vpiArgument, sys);
      vpiHandle tmp;

      if (argv == 0)
	    return 0;

      tmp = vpi_scan(argv);
      assert(tmp);

      switch (vpi_get(vpiType, tmp)) {
	  case vpiConstant:
	    if (vpi_get(vpiConstType, tmp) == vpiStringConst) {
		  vpi_printf("ERROR: %s argument must be "
			     "a number constant.\n", name);
		  vpi_control(vpiFinish, 1);
	    }
	    break;

	  case vpiNet:
	  case vpiReg:
	  case vpiIntegerVar:
	  case vpiMemoryWord:
	    break;

	  default:
	    vpi_printf("ERROR: %s argument must be "
		       "a number constant.\n", name);
	    vpi_control(vpiFinish, 1);
	    break;
      }

      vpi_free_object(argv);
      return 0;
}

static int sys_dumpvars_calltf(char*name)
{
      unsigned depth;
      s_vpi_value value;
      vpiHandle item = 0;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv;

      if (dump_file == 0) {
	    open_dumpfile("dumpfile.vcd");
	    if (dump_file == 0)
		  return 0;
      }

      if (install_dumpvars_callback()) {
	    return 0;
      }

      argv = vpi_iterate(vpiArgument, sys);

      depth = 0;
      if (argv && (item = vpi_scan(argv)))
	    switch (vpi_get(vpiType, item)) {
		case vpiConstant:
		case vpiNet:
		case vpiReg:
		case vpiIntegerVar:
		case vpiMemoryWord:
		  value.format = vpiIntVal;
		  vpi_get_value(item, &value);
		  depth = value.value.integer;
		  break;
	    }

      if (!depth)
	    depth = 10000;

      if (!argv) {
	    // $dumpvars;
	    // search for the toplevel module
	    vpiHandle parent = vpi_handle(vpiScope, sys);
	    while (parent) {
		  item = parent;
		  parent = vpi_handle(vpiScope, item);
	    }

      } else if (!item  ||  !(item = vpi_scan(argv))) {
	    // $dumpvars(level);
	    // $dumpvars();
	    // dump the current scope
	    item = vpi_handle(vpiScope, sys);
	    argv = 0x0;
      }

      for ( ; item; item = argv ? vpi_scan(argv) : 0x0) {

	    int dep = draw_scope(item);

	    vcd_names_sort(&vcd_tab);
	    scan_item(depth, item, 0);
	    
	    while (dep--) {
		  fprintf(dump_file, "$upscope $end\n");
	    }
      }

      return 0;
}

void sys_vcd_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpall";
      tf_data.calltf    = sys_dumpall_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpall";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpoff";
      tf_data.calltf    = sys_dumpoff_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpoff";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpon";
      tf_data.calltf    = sys_dumpon_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpon";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpfile";
      tf_data.calltf    = sys_dumpfile_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpfile";
      vpi_register_systf(&tf_data);

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$dumpvars";
      tf_data.calltf    = sys_dumpvars_calltf;
      tf_data.compiletf = sys_vcd_dumpvars_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_vcd.c,v $
 * Revision 1.44  2003/05/15 16:51:09  steve
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
 * Revision 1.43  2003/04/27 02:22:28  steve
 *  Capture VCD dump value in the rosync time period.
 *
 * Revision 1.42  2003/02/12 05:28:01  steve
 *  Set dumpoff of real variables to NaN.
 *
 * Revision 1.41  2003/02/11 05:21:33  steve
 *  Support dump of vpiRealVar objects.
 *
 * Revision 1.40  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.39  2002/11/17 22:28:42  steve
 *  Close old file if $dumpfile is called again.
 *
 * Revision 1.38  2002/11/14 22:43:58  steve
 *  Save vpiFullName results.
 *
 * Revision 1.37  2002/08/15 02:12:20  steve
 *  add dumpvars_compiletf to check first argument.
 *
 * Revision 1.36  2002/08/12 01:35:05  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.35  2002/07/17 05:13:43  steve
 *  Implementation of vpi_handle_by_name, and
 *  add the vpiVariables iterator.
 *
 * Revision 1.34  2002/07/12 17:09:21  steve
 *  Remember to scan IntegerVars.
 *
 * Revision 1.33  2002/07/12 17:02:38  steve
 *  Scan scope objects before subscopes.
 *
 * Revision 1.32  2002/07/12 02:10:20  steve
 *  Make types array static, not on stack.
 *
 * Revision 1.31  2002/07/12 02:08:10  steve
 *  Eliminate use of vpiInternalScope.
 *
 * Revision 1.30  2002/06/21 04:59:36  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.29  2002/05/23 01:07:26  steve
 *  Ignore Named events in vcd signal scan.
 *
 * Revision 1.28  2002/05/10 16:00:16  steve
 *  ignore vpiMemory objects in vcd dumper.
 *
 * Revision 1.27  2002/04/06 20:25:45  steve
 *  cbValueChange automatically replays.
 *
 * Revision 1.26  2001/10/26 02:29:10  steve
 *  const/non-const warnings. (Stephan Boettcher)
 *
 * Revision 1.25  2001/10/25 04:19:53  steve
 *  VPI support for callback to return values.
 *
 * Revision 1.24  2001/10/15 01:50:23  steve
 *  Include scope information in VCD output.
 *
 * Revision 1.23  2001/10/14 18:32:06  steve
 *  More coverage of $dump related commands.
 *
 * Revision 1.22  2001/10/08 23:33:00  steve
 *  Fix pr283: signal values before enddefinitions in vcd. (Stephan Boettcher)
 *
 * Revision 1.21  2001/09/30 05:18:46  steve
 *  Reduce VCD output by removing duplicates. (Stephan Boettcher)
 *
 * Revision 1.20  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.19  2001/07/16 18:53:16  steve
 *  Cut off scope iteration when depth runs out.
 *
 * Revision 1.18  2001/06/29 00:42:39  steve
 *  Get a private copy of the object name.
 *
 * Revision 1.17  2001/06/21 04:15:22  steve
 *  Add dumpon and dumpoff (Stephan Boettcher)
 *
 * Revision 1.16  2001/01/23 18:50:26  steve
 *  Forgot to actually *open* the VCD output.
 *
 * Revision 1.15  2001/01/22 20:58:31  steve
 *  Support default dumpfiles.
 *
 * Revision 1.14  2001/01/01 08:10:35  steve
 *  Handle function scopes in dumpvars scn (PR#95)
 */

