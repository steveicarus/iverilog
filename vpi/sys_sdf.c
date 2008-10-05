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

	    vpiHandle tmp_scope = find_scope(scope, buffer);
	    if (tmp_scope == 0) {
		  vpi_printf("SDF WARNING: Cannot find %s in %s?\n",
			     buffer, vpi_get_str(vpiName,scope));
		  break;
	    }
	    assert(tmp_scope);
	    scope = tmp_scope;

	    src = dp + 1;
      }

	/* Now find the cell. */
      if (src[0] == 0)
	    sdf_cur_cell = sdf_scope;
      else
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

static const char*edge_str(int vpi_edge)
{
      if (vpi_edge == vpiNoEdge)
	    return "";
      if (vpi_edge == vpiPosedge)
	    return "posedge ";
      if (vpi_edge == vpiNegedge)
	    return "negedge ";
      return "edge.. ";
}

void sdf_iopath_delays(int vpi_edge, const char*src, const char*dst,
		       const struct sdf_delval_list_s*delval_list)
{
      s_vpi_delay delays;
      if (sdf_cur_cell == 0)
	    return;

      vpiHandle iter = vpi_iterate(vpiModPath, sdf_cur_cell);
      assert(iter);

      struct t_vpi_time delay_vals[12];
      int idx;
      for (idx = 0 ; idx < delval_list->count ; idx += 1) {
	    delay_vals[idx].type = vpiScaledRealTime;
	    delay_vals[idx].real = delval_list->val[idx];
      }

      delays.da = delay_vals;
      delays.no_of_delays = delval_list->count;
      delays.time_type = vpiScaledRealTime;
      delays.mtm_flag = 0;
      delays.append_flag = 0;
      delays.plusere_flag = 0;


	/* Search for the modpath that matches the IOPATH by looking
	   for the modpath that uses the same ports as the ports that
	   the parser has found. */
      vpiHandle path;
      int match_count = 0;
      while ( (path = vpi_scan(iter)) ) {
	    vpiHandle path_t_in = vpi_handle(vpiModPathIn,path);
	    vpiHandle path_t_out = vpi_handle(vpiModPathOut,path);

	    vpiHandle path_in = vpi_handle(vpiExpr,path_t_in);
	    vpiHandle path_out = vpi_handle(vpiExpr,path_t_out);

	      /* The expressions for the path terms must be signals,
	         vpiNet or vpiReg. */
	    assert(vpi_get(vpiType,path_in) == vpiNet);
	    assert(vpi_get(vpiType,path_out) == vpiNet
		   || vpi_get(vpiType,path_out) == vpiReg);

	      /* If the src name doesn't match, go on. */
	    if (strcmp(src,vpi_get_str(vpiName,path_in)) != 0)
		  continue;
	      /* The edge type must match too. But note that if this
	         IOPATH has no edge, then it matches with all edges of
	         the modpath object. */
	    if (vpi_edge != vpiNoEdge && vpi_get(vpiEdge,path_t_in) != vpi_edge)
		  continue;

	      /* If the dst name doesn't match, go on. */
	    if (strcmp(dst,vpi_get_str(vpiName,path_out)) != 0)
		  continue;

	      /* Ah, this must be a match! */
	    vpi_put_delays(path, &delays);
	    match_count += 1;
      }

      if (match_count == 0) {
	    vpi_printf("SDF WARNING: Unable to match ModPath %s%s -> %s in %s\n",
		       edge_str(vpi_edge), src, dst, vpi_get_str(vpiName,sdf_cur_cell));
      }
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

      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

      vpiHandle path = vpi_scan(argv);
      if (path == 0) {
	    vpi_printf("SDF ERROR: First argument of %s is required.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      assert(path);

      vpiHandle scope = vpi_scan(argv);
      if (scope == 0)
	    return 0;

      if (vpi_get(vpiType,scope) != vpiModule) {
	    vpi_printf("SDF ERROR: The second argument of %s"
		       " must be a module instance.\n", name);
	    vpi_control(vpiFinish, 1);
      }

      vpi_free_object(argv);

      return 0;
}

static PLI_INT32 sys_sdf_annotate_calltf(PLI_BYTE8*name)
{
      s_vpi_value value;
      vpiHandle sys = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, sys);

	/* The first argument is the path to the SDF file. */
      vpiHandle path = vpi_scan(argv);
      assert(path);

      value.format = vpiStringVal;
      vpi_get_value(path, &value);

      if ((value.format != vpiStringVal) || !value.value.str) {
	    vpi_printf("ERROR: %s: File name argument (type=%d)"
		       " does not have a string value.\n",
		       name, vpi_get(vpiType, path));
	    vpi_control(vpiFinish, 1);
	    return 0;
      }

      char*path_str = strdup(value.value.str);
      FILE*sdf_fd = fopen(path_str, "r");
      if (sdf_fd == 0) {
	    vpi_printf("ERROR: %s: Unable to open SDF file `%s'."
		       " Skipping annotation.\n", name, path_str);
	    return 0;
      }

	/* The optional second argument is the scope to annotate. */
      sdf_scope = vpi_scan(argv);
      if (sdf_scope)
	    vpi_free_object(argv);
      if (sdf_scope == 0) {
	    sdf_scope = vpi_handle(vpiScope,sys);
      }

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
