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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: sys_vcd.c,v 1.30 2002/06/21 04:59:36 steve Exp $"
#endif

# include "config.h"

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
      struct vcd_info*next;
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

static struct vcd_info*vcd_list = 0;
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

      if (vpi_get(vpiSize, info->item) == 1) {
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
      if (vpi_get(vpiSize, info->item) == 1) {
	    fprintf(dump_file, "x%s\n", info->ident);
      } else {
	    fprintf(dump_file, "bx %s\n", info->ident);
      }
}


/*
 * managed qsorted list of scope names for duplicates bsearching
 */

struct vcd_names_s {
      const char *name;
      struct vcd_names_s *next;
};

static struct vcd_names_s *vcd_names_list;
static const char **vcd_names_sorted;
static int listed_names, sorted_names;

inline static void vcd_names_add(const char *name)
{
      struct vcd_names_s *nl = (struct vcd_names_s *)
	    malloc(sizeof(struct vcd_names_s));
      assert(nl);
      nl->name = name;
      nl->next = vcd_names_list;
      vcd_names_list = nl;
      listed_names ++;
}

static int vcd_names_compare(const void *s1, const void *s2)
{
      const char *v1 = *(const char **) s1;
      const char *v2 = *(const char **) s2;

      return strcmp(v1, v2);
}

static const char *vcd_names_search(const char *key)
{
      const char **v = (const char **)
	    bsearch(&key, 
		    vcd_names_sorted, sorted_names,
		    sizeof(const char *), vcd_names_compare );
      
      return(v ? *v : NULL);
}

void vcd_names_sort(void)
{
      if (listed_names) {
	    struct vcd_names_s *r; 
	    const char **l;
	    
	    sorted_names += listed_names;
	    vcd_names_sorted = (const char **) 
		  realloc(vcd_names_sorted, 
			  sorted_names*(sizeof(const char *)));
	    assert(vcd_names_sorted);
	    
	    l = vcd_names_sorted + sorted_names - listed_names;
	    listed_names = 0;

	    r = vcd_names_list;
	    vcd_names_list = 0x0;

	    while (r) {
		  struct vcd_names_s *rr = r;
		  r = rr->next;
		  *(l++) = rr->name;
		  free(rr);
	    }
	    
	    qsort(vcd_names_sorted, sorted_names, 
		  sizeof(const char **), vcd_names_compare);
      }
}


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

static int variable_cb(p_cb_data cause)
{
      unsigned long now = cause->time->low;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

      if (dump_is_off)
	    return 0;

      if (dump_header_pending())
	    return 0;

      if (now != vcd_cur_time) {
	    fprintf(dump_file, "#%lu\n", now);
	    vcd_cur_time = now;
      }

      show_this_item(info);

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
	    vpi_mcd_printf(6, "VCD Error:"
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
	    vpi_mcd_printf(6, 
			   "VCD Error: Unable to open %s for output.\n", 
			   path);
	    return;
      } else {
	    int prec = vpi_get(vpiTimePrecision, 0);
	    unsigned scale = 1;
	    unsigned udx = 0;
	    time_t walltime;

	    vpi_mcd_printf(4, 
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
		  vpi_mcd_printf(6, 
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

      assert(dump_file == 0);
      open_dumpfile(path);

      free(path);

      return 0;
}

/* 
   Nexus Id cache

   In structural models, many signals refer to the same nexus.
   Some structural models also have very many signals.  This cache
   saves nexus_id - vcd_id pairs, and reuses the vcd_id when a signal 
   refers to a nexus that is already dumped.

   The new signal will be listed as a $var, but no callback 
   will be installed.  This saves considerable CPU time and leads 
   to smalle VCD files.

   The _vpiNexusId is a private (int) property of IVL simulators.
*/

struct vcd_id_s 
{
  const char *id;
  struct vcd_id_s *next;
  int nex;
};

static inline unsigned ihash(int nex)
{
  unsigned a = nex;
  a ^= a>>16;
  a ^= a>>8;
  return a & 0xff;
}

static struct vcd_id_s **vcd_ids;

inline static const char *find_nexus_ident(int nex)
{
      struct vcd_id_s *bucket;
      
      if (!vcd_ids) {
	    vcd_ids = (struct vcd_id_s **)
		  calloc(256, sizeof(struct vcd_id_s*));
	    assert(vcd_ids);
      }

      bucket = vcd_ids[ihash(nex)];
      while (bucket) {
	    if (nex == bucket->nex)
		  return bucket->id;
	    bucket = bucket->next;
      }

      return 0;
}

inline static void set_nexus_ident(int nex, const char *id)
{
      struct vcd_id_s *bucket;

      assert(vcd_ids);

      bucket = (struct vcd_id_s *) malloc(sizeof(struct vcd_id_s));
      bucket->next = vcd_ids[ihash(nex)];
      bucket->id = id;
      bucket->nex = nex;
      vcd_ids[ihash(nex)] = bucket;
}

static void scan_item(unsigned depth, vpiHandle item, int skip)
{
      struct t_cb_data cb;
      struct vcd_info* info;

      const char* type;
      const char* name;
      const char* ident;
      int nexus_id;

      switch (vpi_get(vpiType, item)) {

	  case vpiMemory:
	      /* don't know how to watch memories. */
	    break;

	  case vpiNamedEvent:
	      /* There is nothing in named events to dump. */
	    break;

	  case vpiNet:  type = "wire";    if(0){
	  case vpiIntegerVar:
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

		  cb.time      = &info->time;
		  cb.user_data = (char*)info;
		  cb.value     = NULL;
		  cb.obj       = item;
		  cb.reason    = cbValueChange;
		  cb.cb_rtn    = variable_cb;

		  info->next  = vcd_list;
		  vcd_list    = info;

		  info->cb    = vpi_register_cb(&cb);
	    }
	    
	    fprintf(dump_file, "$var %s %u %s %s $end\n",
		    type, vpi_get(vpiSize, item), ident,
		    name);
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

		  vpi_mcd_printf(4, 
				 "VCD info:"
				 " scanning scope %s, %u levels\n",
				 fullname, depth);

		  nskip = sorted_names && fullname
			&& vcd_names_search(fullname);
		  
		  if (!nskip) 
			vcd_names_add(fullname);
		  else 
		    vpi_mcd_printf(6,
				   "VCD warning:"
				   " ignoring signals"
				   " in previously scanned scope %s\n",
				   fullname);

		  name = vpi_get_str(vpiName, item);

		  fprintf(dump_file, "$scope %s %s $end\n", type, name);
		  
		  argv = vpi_iterate(vpiInternalScope, item);
		  if (argv) {
			for (item = vpi_scan(argv) ;  
			     item ;  
			     item = vpi_scan(argv)) {
			      
			      scan_item(depth-1, item, nskip);
			}
		  }
		  
		  fprintf(dump_file, "$upscope $end\n");
	    }
	    break;
	    
	  default:
	    vpi_mcd_printf(6,
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

	    vcd_names_sort();
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
      tf_data.compiletf = 0;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$dumpvars";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_vcd.c,v $
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
 *
 * Revision 1.13  2000/11/01 06:05:44  steve
 *  VCD scans tasks (PR#35)
 *
 * Revision 1.12  2000/07/31 03:34:31  steve
 *  Report error when dumpfile is missing.
 *
 * Revision 1.11  2000/07/26 04:07:59  steve
 *  Get VCD timescale from design precision.
 *
 * Revision 1.10  2000/06/03 02:22:15  steve
 *  Interpret the depth paramter of dumpvars.
 *
 * Revision 1.9  2000/04/09 04:18:16  steve
 *  Catch duplicate $dumpvars of symbols (ajb)
 *
 * Revision 1.8  2000/04/08 05:28:39  steve
 *  Revamped VCD id generation and duplicates removal. (ajb)
 *
 * Revision 1.8  2000/04/06 21:00:00  ajb
 *  Revamped VCD id generation and duplicates removal.
 *
 * Revision 1.7  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.6  2000/02/17 06:04:30  steve
 *  Fix overlap of identifiers when multiple modules used.
 *
 * Revision 1.5  2000/01/23 23:54:36  steve
 *  Compile time problems with vpi_user.h
 *
 * Revision 1.4  2000/01/20 06:04:55  steve
 *  $dumpall checkpointing in VCD dump.
 *
 * Revision 1.3  2000/01/13 04:48:50  steve
 *  Catch some parameter problems.
 *
 * Revision 1.2  1999/11/28 00:56:08  steve
 *  Build up the lists in the scope of a module,
 *  and get $dumpvars to scan the scope for items.
 *
 * Revision 1.1  1999/11/07 20:33:30  steve
 *  Add VCD output and related system tasks.
 *
 */

