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
#ident "$Id: sys_vcd.c,v 1.9 2000/04/09 04:18:16 steve Exp $"
#endif

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

static FILE*dump_file = 0;

struct vcd_info {
      vpiHandle item;
      vpiHandle cb;
      struct t_vpi_time time;
      char*ident;
      char*fullname;
      struct vcd_info*next;
};


static char vcdid[8]={'!',0,0,0,0,0,0,0};
static void gen_new_vcd_id(void)
{
      int i;
      for(i=0;i<8;i++)        /* increment vcd id for next fac */
	    {
		  vcdid[i]++;
		  if(vcdid[i]!=127) break;
		  vcdid[i]='!';
		  if(vcdid[i+1]==0x00)
			{
			      vcdid[i+1]='!';
			      break;
			}
	    }
}

static struct vcd_info*vcd_list = 0;
unsigned long vcd_cur_time = 0;

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


/*
 * managed qsorted list of vcd_info structs for duplicates bsearching
 */
static int nident=0, old_nident=0;
static struct vcd_info **vcd_info_name_sorted=NULL;

static int vcd_info_name_bsearch_compare(const void *s1, const void *s2)
{
      char *v1;
      struct vcd_info *v2;

      v1=(char *)s1;
      v2=*((struct vcd_info **)s2);

      return(strcmp(v1, v2->fullname));
}

static struct vcd_info *bsearch_vcd_info(char *key)
{
      struct vcd_info **v;

      v=(struct vcd_info **)bsearch(key, vcd_info_name_sorted, old_nident,
				    sizeof(struct vcd_info *), vcd_info_name_bsearch_compare);

      return(v ? (*v) : NULL);
}

static int vcd_info_name_compare(const void *s1, const void *s2)
{
      struct vcd_info *v1, *v2;

      v1=*((struct vcd_info **)s1);
      v2=*((struct vcd_info **)s2);

      return(strcmp(v1->fullname, v2->fullname));
}  

void vcd_info_post_process(void)
{
      if(nident) {
	    struct vcd_info **l, *r;

	    if (vcd_info_name_sorted) free(vcd_info_name_sorted);
	
	    old_nident+=nident;
	    nident=0;
	    l=vcd_info_name_sorted=(struct vcd_info **)malloc(old_nident*(sizeof(struct vcd_info *)));
	    r=vcd_list;
	    while(r) {
		  *(l++)=r;
		  r=r->next;
	    }

	    qsort(vcd_info_name_sorted, old_nident, sizeof(struct vcd_info *), vcd_info_name_compare);
      }
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

static int variable_cb(p_cb_data cause)
{
      unsigned long now = cause->time->low;
      struct t_cb_data cb;
      struct vcd_info*info = (struct vcd_info*)cause->user_data;

	/* Reschedule this event so that it happens for the next
	   trigger on this variable. */
      cb = *cause;
      vpi_register_cb(&cb);

      if (now != vcd_cur_time) {
	    fprintf(dump_file, "#%lu\n", now);
	    vcd_cur_time = now;
      }

      show_this_item(info);

      return 0;
}

static int sys_dumpall_calltf(char*name)
{
      s_vpi_time now;
      vpi_get_time(0, &now);
      fprintf(dump_file, "#%u\n", now.low);
      vcd_cur_time = now.low;
      vcd_checkpoint();

      return 0;
}

static int sys_dumpfile_calltf(char*name)
{
      char*path;

      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      if (argv) {
	    vpiHandle item = vpi_scan(argv);
	    s_vpi_value value;

	    if (vpi_get(vpiType, item) != vpiConstant) {
		  vpi_printf("ERROR: %s parameter must be a constant\n", name);
		  return 0;
	    }

	    if (vpi_get(vpiConstType, item) != vpiStringConst) {
		  vpi_printf("ERROR: %s parameter must be a constant\n", name);
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
      dump_file = fopen(path, "w");
      if (dump_file == 0) {
	    vpi_printf("ERROR: Unable to open %s for output.\n", path);
	    return 0;
      } else {
	    time_t walltime;

	    time(&walltime);

	    fprintf(dump_file, "$date\n");
	    fprintf(dump_file, "\t%s",asctime(localtime(&walltime)));
	    fprintf(dump_file, "$end\n");
	    fprintf(dump_file, "$version\n");
	    fprintf(dump_file, "\tIcarus Verilog\n");
	    fprintf(dump_file, "$end\n");
	    fprintf(dump_file, "$timescale\n");
	    fprintf(dump_file, "\t1ps\n");
	    fprintf(dump_file, "$end\n");
      }

      free(path);

      return 0;
}

static void scan_scope(unsigned depth, vpiHandle argv)
{
      struct t_cb_data cb;
      struct vcd_info*info;

      vpiHandle item;
      vpiHandle sublist;

      cb.reason = cbValueChange;
      cb.cb_rtn = variable_cb;

      for (item = vpi_scan(argv) ;  item ;  item = vpi_scan(argv)) {
	    const char*type;
	    char*fullname;

	    switch (vpi_get(vpiType, item)) {
		case vpiNet:
		case vpiReg:
		  fullname=vpi_get_str(vpiFullName, item);
		  if((old_nident)&&(fullname)&&(bsearch_vcd_info(fullname)))
			continue;

		  type = "wire";
		  if (vpi_get(vpiType, item) == vpiReg)
			type = "reg";

		  info = malloc(sizeof(*info));
		  info->time.type = vpiSimTime;
		  cb.time = &info->time;
		  cb.user_data = (char*)info;
		  cb.obj = item;
		  info->item  = item;
		  info->ident = strdup(vcdid);
		  info->fullname = fullname;
		  info->cb    = vpi_register_cb(&cb);
		  info->next = vcd_list;
		  vcd_list   = info;
		  fprintf(dump_file, "$var %s %u %s %s $end\n",
			  type, vpi_get(vpiSize, item), info->ident,
			  info->fullname);
		  gen_new_vcd_id();
		  nident += 1;
		  break;

		case vpiModule:
		  sublist = vpi_iterate(vpiInternalScope, item);
		  if (sublist && (depth > 0)) {
			vcd_info_post_process();
			scan_scope(depth-1, sublist);
		  }
		  break;

		default:
		  vpi_printf("ERROR: $dumpvars: Unsupported parameter type\n");
	    }

      }

	/* close + sort this level so parent collisions are found */
      vcd_info_post_process();
}

static int sys_dumpvars_calltf(char*name)
{
      s_vpi_time now;
      vpiHandle item;
      vpiHandle sys = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      if (argv == 0) {
	    vpi_printf("SORRY: %s requires arguments\n", name);
	    return 0;
      }

      item = vpi_scan(argv);

      assert(dump_file);

      scan_scope(99, argv);

      fprintf(dump_file, "$enddefinitions $end\n");

      vpi_get_time(0, &now);
      fprintf(dump_file, "#%u\n", now.low);

      vcd_checkpoint();

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

