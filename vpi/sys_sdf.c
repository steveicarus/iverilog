/*
 * Copyright (c) 2007-2010 Stephen Williams (steve@icarus.com)
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

# include  "sys_priv.h"
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
static vpiHandle sdf_callh = 0;
  /* The cell in process. */
static vpiHandle sdf_cur_cell;

static vpiHandle find_scope(vpiHandle scope, const char*name)
{
      vpiHandle idx = vpi_iterate(vpiModule, scope);
	/* If this scope has no modules then it can't have the one we
	 * are looking for so just return 0. */
      if (idx == 0) return 0;

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
	    unsigned len = dp - src;
	    assert(dp >= src);
	    assert(len < sizeof buffer);
	    strncpy(buffer, src, len);
	    buffer[len] = 0;

	    vpiHandle tmp_scope = find_scope(scope, buffer);
	    if (tmp_scope == 0) {
		  vpi_printf("SDF WARNING: %s:%d: ",
		             vpi_get_str(vpiFile, sdf_callh),
		             (int)vpi_get(vpiLineNo, sdf_callh));
		  vpi_printf("Cannot find %s in scope %s.\n",
			     buffer, vpi_get_str(vpiFullName, scope));
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
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, sdf_callh),
	               (int)vpi_get(vpiLineNo, sdf_callh));
	    vpi_printf("Unable to find %s in scope %s.\n",
		       cellinst, vpi_get_str(vpiFullName, scope));
	    return;
      }

	/* The scope that matches should be a module. */
      if (vpi_get(vpiType,sdf_cur_cell) != vpiModule) {
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, sdf_callh),
	               (int)vpi_get(vpiLineNo, sdf_callh));
	    vpi_printf("Scope %s in %s is not a module.\n",
		       cellinst, vpi_get_str(vpiFullName, sdf_scope));
      }

	/* The matching scope (a module) should have the expected type. */
      if (strcmp(celltype,vpi_get_str(vpiDefName,sdf_cur_cell)) != 0) {
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, sdf_callh),
	               (int)vpi_get(vpiLineNo, sdf_callh));
	    vpi_printf("Module %s in %s is not a %s; it is an %s\n", cellinst,
		       vpi_get_str(vpiFullName, sdf_scope), celltype,
		       vpi_get_str(vpiDefName, sdf_cur_cell));
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
      vpiHandle iter, path;
      int match_count = 0;

      if (sdf_cur_cell == 0)
	    return;

      iter = vpi_iterate(vpiModPath, sdf_cur_cell);

	/* Search for the modpath that matches the IOPATH by looking
	   for the modpath that uses the same ports as the ports that
	   the parser has found. */
      if (iter) while ( (path = vpi_scan(iter)) ) {
	    s_vpi_delay delays;
	    struct t_vpi_time delay_vals[12];
	    int idx;

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
/* --> Is this correct in the context of the 10, 01, etc. edges? */
	    if (vpi_edge != vpiNoEdge && vpi_get(vpiEdge,path_t_in) != vpi_edge)
		  continue;

	      /* If the dst name doesn't match, go on. */
	    if (strcmp(dst,vpi_get_str(vpiName,path_out)) != 0)
		  continue;

	      /* Ah, this must be a match! */
	    delays.da = delay_vals;
	    delays.no_of_delays = delval_list->count;
	    delays.time_type = vpiScaledRealTime;
	    delays.mtm_flag = 0;
	    delays.append_flag = 0;
	    delays.plusere_flag = 0;
	    vpi_get_delays(path, &delays);

	    for (idx = 0 ; idx < delval_list->count ; idx += 1) {
		  delay_vals[idx].type = vpiScaledRealTime;
		  if (delval_list->val[idx].defined) {
			delay_vals[idx].real = delval_list->val[idx].value;
			  /* Simulation cannot support negative delays. */
			if (delay_vals[idx].real < 0.0)  {
			      delay_vals[idx].real = 0.0;
			}
		  }
	    }

	    vpi_put_delays(path, &delays);
	    match_count += 1;
      }

      if (match_count == 0) {
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, sdf_callh),
	               (int)vpi_get(vpiLineNo, sdf_callh));
	    vpi_printf("Unable to match ModPath %s%s -> %s in %s\n",
		       edge_str(vpi_edge), src, dst,
		       vpi_get_str(vpiFullName, sdf_cur_cell));
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
      vpiHandle callh = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle module;

      check_command_line_args();

	/* Check that we have a file name argument. */
      if (argv == 0) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a file name argument.\n", name);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name must be a string.\n", name);
	    vpi_control(vpiFinish, 1);
      }

	/* The module argument is optional. */
      module = vpi_scan(argv);
      if (module == 0) return 0;
      if (vpi_get(vpiType, module) != vpiModule) {
	    vpi_printf("ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be a module instance.\n",
	               name);
	    vpi_control(vpiFinish, 1);
      }

	/* Warn the user that we only use the first two arguments. */
      if (vpi_scan(argv) != 0) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s currently only uses the first two argument.\n",
	               name);
	    vpi_free_object(argv);
      }

      return 0;
}

static PLI_INT32 sys_sdf_annotate_calltf(PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      FILE *sdf_fd;
      char *fname = get_filename(callh, name, vpi_scan(argv));

      if (fname == 0) return 0;

      sdf_fd = fopen(fname, "r");
      if (sdf_fd == 0) {
	    vpi_printf("WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open SDF file \"%s\"."
		       " Skipping this annotation.\n", fname);
	    return 0;
      }

	/* The optional second argument is the scope to annotate. */
      sdf_scope = vpi_scan(argv);
      if (sdf_scope) vpi_free_object(argv);
      else sdf_scope = vpi_handle(vpiScope, callh);

      sdf_cur_cell = 0;
      sdf_callh = callh;
      sdf_process_file(sdf_fd, fname);
      sdf_callh = 0;

      fclose(sdf_fd);
      free(fname);
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
