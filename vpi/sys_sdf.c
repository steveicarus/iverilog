/*
 * Copyright (c) 2007 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"

# include  "vpi_user.h"
# include  "sdf_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

/*
 * These are static context
 */

int sdf_flag_warning = 0;
int sdf_flag_inform = 0;

  /* Scope of the $sdf_annotate call. Annotation starts here. */
static vpiHandle sdf_scope;
  /* The cell in process. */
static vpiHandle sdf_cur_cell;

static vpiHandle find_scope(vpiHandle scope, const char*name)
{
      vpiHandle idx = vpi_iterate(vpiModule, scope);
      assert(idx);

      vpiHandle cur;
      while ( (cur = vpi_scan(idx)) ) {

	    if ( strcmp(name, vpi_get_str(vpiName,cur)) == 0) {
		  vpi_free_object(idx);
		  return cur;
	    }
      }

      return 0;
}

/*
 * These functions are called by the SDF parser during parsing to
 * handling items discovered in the parse.
 */

void sdf_select_instance(const char*celltype, const char*cellinst)
{
      char buffer[128];

	/* First follow the hierarchical parts of the cellinst name to
	   get to the cell that I'm looking for. */
      vpiHandle scope = sdf_scope;
      const char*src = cellinst;
      const char*dp;
      while ( (dp=strchr(src, '.')) ) {
	    int len = dp - src;
	    assert(len < sizeof buffer);
	    strncpy(buffer, src, len);
	    buffer[len] = 0;

	    scope = find_scope(scope, buffer);
	    assert(scope);

	    src = dp + 1;
      }

	/* Now find the cell. */
      sdf_cur_cell = find_scope(scope, src);
      if (sdf_cur_cell == 0) {
	    vpi_printf("SDF WARNING: Unable to find %s in current scope\n",
		       cellinst);
	    return;
      }

	/* The scope that matches should be a module. */
      if (vpi_get(vpiType,sdf_cur_cell) != vpiModule) {
	    vpi_printf("SDF ERROR: Scope %s in %s is not a module.\n",
		       cellinst, vpi_get_str(vpiName,sdf_scope));
      }

	/* The matching scope (a module) should have the expected type. */
      if (strcmp(celltype,vpi_get_str(vpiDefName,sdf_cur_cell)) != 0) {
	    vpi_printf("SDF ERROR: Module %s in %s is not a %s; "
		       "it is an %s\n", cellinst,
		       vpi_get_str(vpiName,sdf_scope), celltype,
		       vpi_get_str(vpiDefName,sdf_cur_cell));
      }

}

void sdf_iopath_delays(const char*src, const char*dst,
		       const struct sdf_delval_list_s*delval_list)
{
      if (sdf_cur_cell == 0)
	    return;

      vpiHandle iter = vpi_iterate(vpiModPath, sdf_cur_cell);
      assert(iter);

	/* Search for the modpath that matches the IOPATH by looking
	   for the modpath that uses the same ports as the ports that
	   the parser has found. */
      vpiHandle path;
      while ( (path = vpi_scan(iter)) ) {
	    vpiHandle path_in = vpi_handle(vpiModPathIn,path);
	    vpiHandle path_out = vpi_handle(vpiModPathOut,path);

	    path_in = vpi_handle(vpiExpr,path_in);
	    path_out = vpi_handle(vpiExpr,path_out);
	    assert(vpi_get(vpiType,path_in) == vpiNet);
	    assert(vpi_get(vpiType,path_out) == vpiNet);

	      /* If the src name doesn't match, go on. */
	    if (strcmp(src,vpi_get_str(vpiName,path_in)) != 0)
		  continue;

	      /* If the dst name doesn't match, go on. */
	    if (strcmp(dst,vpi_get_str(vpiName,path_out)) != 0)
		  continue;

	      /* Ah, this must be a match! */
	    break;
      }

      if (path == 0) {
	    vpi_printf("SDF ERROR: Unable to find ModPath %s -> %s in %s\n",
		       src, dst, vpi_get_str(vpiName,sdf_cur_cell));
	    return;
      }

        /* No longer need the iterator. */
      vpi_free_object(iter);

      struct t_vpi_time delay_vals[12];
      int idx;
      for (idx = 0 ; idx < delval_list->count ; idx += 1) {
	    delay_vals[idx].type = vpiScaledRealTime;
	    delay_vals[idx].real = delval_list->val[idx];
      }

      s_vpi_delay delays;
      delays.da = delay_vals;
      delays.no_of_delays = delval_list->count;
      delays.time_type = vpiScaledRealTime;
      delays.mtm_flag = 0;
      delays.append_flag = 0;
      delays.plusere_flag = 0;

      vpi_put_delays(path, &delays);
}

static void check_command_line_args(void)
{
      struct t_vpi_vlog_info vlog_info;
      int idx;

      static int sdf_command_line_done = 0;
      if (sdf_command_line_done)
	    return;

      vpi_get_vlog_info(&vlog_info);

      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {
	    if (strcmp(vlog_info.argv[idx],"-sdf-warn") == 0) {
		  sdf_flag_warning = 1;

	    } else if (strcmp(vlog_info.argv[idx],"-sdf-info") == 0) {
		  sdf_flag_inform = 1;

	    } else if (strcmp(vlog_info.argv[idx],"-sdf-verbose") == 0) {
		  sdf_flag_warning = 1;
		  sdf_flag_inform = 1;
	    }
      }

      sdf_command_line_done = 1;
}

static PLI_INT32 sys_sdf_annotate_compiletf(PLI_BYTE8*name)
{
      check_command_line_args();
      return 0;
}

static PLI_INT32 sys_sdf_annotate_calltf(PLI_BYTE8*name)
{
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      vpiHandle path = vpi_scan(argv);
      assert(path);

      vpi_free_object(argv);

      value.format = vpiStringVal;
      vpi_get_value(path, &value);

      if ((value.format != vpiStringVal) || !value.value.str) {
	    vpi_printf("ERROR: %s: File name argument (type=%d)"
		       " does not have a string value\n",
		       name, vpi_get(vpiType, path));
	    return 0;
      }

      char*path_str = strdup(value.value.str);
      FILE*sdf_fd = fopen(path_str, "r");
      assert(sdf_fd);

      sdf_scope = vpi_handle(vpiScope,sys);
      sdf_cur_cell = 0;

      sdf_process_file(sdf_fd, path_str);

      fclose(sdf_fd);
      free(path_str);
      return 0;
}

void sys_sdf_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$sdf_annotate";
      tf_data.calltf    = sys_sdf_annotate_calltf;
      tf_data.compiletf = sys_sdf_annotate_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$sdf_annotate";
      vpi_register_systf(&tf_data);
}
