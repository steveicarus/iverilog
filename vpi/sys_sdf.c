/*
 * Copyright (c) 2007-2026 Stephen Williams (steve@icarus.com)
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

# include  "sys_priv.h"
# include  "sdf_priv.h"
# include  <math.h>
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

/*
 * These are static context
 */

int sdf_flag_warning = 0;
int sdf_flag_inform = 0;
int sdf_min_typ_max;

  /* Scope of the $sdf_annotate call. Annotation starts here. */
static vpiHandle sdf_scope;
static vpiHandle sdf_callh = 0;
  /* The cell in process. */
static vpiHandle sdf_cur_cell;
/* New timing checks require the selected instance to match the SDF CELLTYPE. Keep
   existing annotation kinds' historical mismatch behavior unchanged. */
static int sdf_cur_cell_type_matches;

/* Timescale from the SDF file header, in seconds. IEEE 1497 makes 1ns the
   default when the header carries no (TIMESCALE ...) entry. */
double sdf_timescale = 1.0e-9;

/* SDF delay values are expressed in the SDF file's own timescale, but
   vpi_put_delays() with vpiScaledRealTime interprets them in the time units
   of the module that contains the annotated object, so convert between the
   two. That module is the cell currently being processed, which is not
   necessarily the scope passed to $sdf_annotate: a cell library routinely
   declares a different `timescale from the design that instantiates it. */
static double sdf_delay_scale(vpiHandle scope)
{
      if (scope == 0) scope = sdf_scope;
      int unit = vpi_get(vpiTimeUnit, scope);
      return sdf_timescale / pow(10.0, (double)unit);
}
static char* sdf_fname = NULL;

/* Hierarchy separators have been normalized to dots by the parser. Split
   before decoding escapes so a literal dot remains part of its local name.
   The lexer guarantees that a backslash is followed by a character. */
static char* next_component(char**cursor)
{
      char*name = *cursor;
      char*src = name;
      char*dst = name;
      while (*src && *src != '.') {
	    if (*src == '\\') src += 1;
	    *dst++ = *src++;
      }
      *cursor = *src ? src + 1 : 0;
      *dst = 0;
      return name;
}

static vpiHandle find_scope(vpiHandle scope, const char*name, int type)
{
      vpiHandle idx = vpi_iterate(type, scope);
	/* If this scope has no children of this type, it cannot contain
	 * the requested name. */
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

void sdf_warn_file_line(const int sdf_lineno)
{
      vpi_printf("SDF WARNING: %s:%d: loaded from %s:%d: ",
                 sdf_fname, sdf_lineno,
                 vpi_get_str(vpiFile, sdf_callh),
                 (int)vpi_get(vpiLineNo, sdf_callh));
}

/*
 * These functions are called by the SDF parser during parsing to
 * handling items discovered in the parse.
 */

void sdf_select_instance(const char*celltype, const char*cellinst, const int sdf_lineno)
{
      sdf_cur_cell_type_matches = 0;
	/* Test for wildcard character */
      if (cellinst == NULL) {
	    if (sdf_flag_warning) {
		  vpi_printf("SDF WARNING: %s:%d: sorry: "
		             "Wildcard cell instance specification (*) currently not supported.\n",
		             sdf_fname, sdf_lineno);
	    }
	    sdf_cur_cell = 0;
	    return;
      }

	/* First follow the hierarchical parts of the cellinst name to
	   get to the cell that I'm looking for. */
      vpiHandle scope = sdf_scope;
      char*buffer = strdup(cellinst);
      char*cursor = cellinst[0] ? buffer : 0;
      const char*src = cellinst;
      while (cursor) {
	    src = next_component(&cursor);
	    vpiHandle tmp_scope = find_scope(scope, src, vpiModule);
	    if (tmp_scope == 0) {
		  vpi_printf("SDF ERROR: %s:%d: Cannot find %s in scope %s.\n",
		             sdf_fname, sdf_lineno, src, vpi_get_str(vpiFullName, scope));
		  scope = 0;
		  break;
	    }
	    assert(tmp_scope);
	    scope = tmp_scope;
      }

      sdf_cur_cell = scope;
      if (sdf_cur_cell == 0) {
	    vpi_printf("SDF ERROR: %s:%d: Unable to find %s in scope %s.\n",
	               sdf_fname, sdf_lineno, cellinst, vpi_get_str(vpiFullName, sdf_scope));
	    free(buffer);
	    return;
      }

	/* The scope that matches should be a module. */
      if (vpi_get(vpiType,sdf_cur_cell) != vpiModule) {
	    vpi_printf("SDF ERROR: %s:%d: Scope %s in %s is not a module.\n",
	               sdf_fname, sdf_lineno, src, vpi_get_str(vpiFullName, scope));
      }

	/* The matching scope (a module) should have the expected type. */
      if (strcmp(celltype,vpi_get_str(vpiDefName,sdf_cur_cell)) != 0) {
	    vpi_printf("SDF ERROR: %s:%d: Module %s in %s is not a %s; it is a ",
	               sdf_fname, sdf_lineno, src, vpi_get_str(vpiFullName, scope), celltype);
	    vpi_printf("%s\n", vpi_get_str(vpiDefName, sdf_cur_cell));
      } else {
            sdf_cur_cell_type_matches =
                  vpi_get(vpiType, sdf_cur_cell) == vpiModule;
      }
      free(buffer);
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

static vpiHandle get_port_handle(char* port_name, const int sdf_lineno)
{
      vpiHandle scope = sdf_cur_cell;
      if (!scope) return 0;
      char*cursor = port_name;
      char*token = next_component(&cursor);

	// Resolve exact local scope names without parsing literal dots again.
      while (cursor) {
	    scope = find_scope(scope, token, vpiInternalScope);
	    if (!scope) {
		  vpi_printf("SDF ERROR: %s:%d: Submodule %s in port path not found!\n", sdf_fname, sdf_lineno, token);
		  return 0;
	    }
	    token = next_component(&cursor);
      }

	// Iterate over ports
      vpiHandle port_i = vpi_iterate(vpiPort, scope) ;
      vpiHandle port;
      vpiHandle port_handle = NULL;

      if (!port_i) return 0;

      while ((port=vpi_scan(port_i)) != NULL) {
	    const char *port_name_ = vpi_get_str(vpiName, port) ;

	    if (strcmp(port_name_, token) == 0) {
		  if (port_handle != NULL) {
			if (sdf_flag_warning) vpi_printf("SDF WARNING: %s:%d: Found multiple matching ports for %s !\n", sdf_fname, sdf_lineno, token);
		  }
		  port_handle = port;
	    }
      }

      return port_handle;
}


void sdf_interconnect_delays(struct interconnect_port_s port1, struct interconnect_port_s port2,
                             const struct sdf_delval_list_s*delval_list,
                             const int sdf_lineno)
{

	// Get handles for both ports
	// After calling get_port_handle, the name is invalid
      vpiHandle port1_handle = get_port_handle(port1.name, sdf_lineno);
      vpiHandle port2_handle = get_port_handle(port2.name, sdf_lineno);
      if (!port1_handle || !port2_handle) {
	    vpi_printf("SDF ERROR: %s:%d: Could not find handles for both ports!\n", sdf_fname, sdf_lineno);
	    return;
      }

	// Check whether we have a single bit of a port for port1
      if (port1.has_index) {
	    vpiHandle iter, vpi_port_bit;
	    iter = vpi_iterate(vpiBit, port1_handle);

	    if (!iter) {
		  vpi_printf("SDF ERROR: %s:%d: Could not find vpiBit iterator for port1!\n", sdf_fname, sdf_lineno);
		  return;
	    }

	    while ((vpi_port_bit = vpi_scan(iter))) {
		  int bit = vpi_get(vpiBit, vpi_port_bit);

		    // If we found the correct vpiPortBit, replace the port with it
		  if (port1.index == bit) {
			if (sdf_flag_inform) vpi_printf("SDF INFO: %s:%d: Substituting vpiPort with vpiPortBit for port1\n", sdf_fname, sdf_lineno);
			port1_handle = vpi_port_bit;
			vpi_release_handle(iter); // Free the iterator
			break;
		  }
	    }
      }

	// Check whether we have a single bit of a port for port2
      if (port2.has_index) {
	    vpiHandle iter, vpi_port_bit;
	    iter = vpi_iterate(vpiBit, port2_handle);

	    if (!iter) {
		  vpi_printf("SDF ERROR: %s:%d: Could not find vpiBit iterator for port2!\n", sdf_fname, sdf_lineno);
		  return;
	    }

	    while ((vpi_port_bit = vpi_scan(iter))) {
		  int bit = vpi_get(vpiBit, vpi_port_bit);

		    // If we found the correct vpiPortBit, replace the port with it
		  if (port2.index == bit) {
			if (sdf_flag_inform) vpi_printf("SDF INFO: %s:%d: Substituting vpiPort with vpiPortBit for port2\n", sdf_fname, sdf_lineno);
			port2_handle = vpi_port_bit;
			vpi_release_handle(iter); // Free the iterator
			break;
		  }
	    }
      }

      if (port1_handle && port2_handle) {
	      // Get interModPath for the two ports
	    vpiHandle intermodpath = vpi_handle_multi(vpiInterModPath, port1_handle, port2_handle);

	    if (intermodpath) {
		  if (sdf_flag_inform) vpi_printf("SDF INFO: %s:%d: Created a vpiInterModPath\n", sdf_fname, sdf_lineno);

		  s_vpi_delay delays;
		  struct t_vpi_time delay_vals[12];

		    // Initialize delay structure
		  delays.da = delay_vals;
		  delays.no_of_delays = delval_list->count;
		  delays.time_type = vpiScaledRealTime;
		  delays.mtm_flag = 0;
		  delays.append_flag = 0;
		  delays.pulsere_flag = 0;
		  vpi_get_delays(intermodpath, &delays);

		  for (int idx = 0 ; idx < delval_list->count ; idx += 1) {
			delay_vals[idx].type = vpiScaledRealTime;
			if (delval_list->val[idx].defined) {
			      double val = delval_list->val[idx].value * sdf_delay_scale(sdf_cur_cell);
			      if (sdf_flag_inform) vpi_printf("SDF INFO: %s:%d: Putting delay: %f for index %d\n",
			                                      sdf_fname, sdf_lineno, val, idx);
			      delay_vals[idx].real = val;
			}
		  }

		    // Put the new delays
		  vpi_put_delays(intermodpath, &delays);
	    } else {
		  vpi_printf("SDF ERROR: %s:%d: Could not find intermodpath!\n", sdf_fname, sdf_lineno);
	    }
      } else {
	    vpi_printf("SDF ERROR: %s:%d: Could not find handles for both ports!\n", sdf_fname, sdf_lineno);
      }
}

void sdf_cond_iopath_delays(const char*key, int vpi_edge, const char*src, const char*dst,
                       const struct sdf_delval_list_s*delval_list,
                       const int sdf_lineno)
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

            /* A COND names the original source condition, independently of
               its current value. Never fall back to an endpoint-only match. */
            if (key) {
                  const char*condition = vpi_get_str(_vpiModPathCondition, path);
                  if (!condition || strcmp(key, condition) != 0) continue;
            }

	      /* Ah, this must be a match! */
	    delays.da = delay_vals;
	    delays.no_of_delays = delval_list->count;
	    delays.time_type = vpiScaledRealTime;
	    delays.mtm_flag = 0;
	    delays.append_flag = 0;
	    delays.pulsere_flag = 0;
	    vpi_get_delays(path, &delays);

	    for (idx = 0 ; idx < delval_list->count ; idx += 1) {
		  delay_vals[idx].type = vpiScaledRealTime;
		  if (delval_list->val[idx].defined) {
			delay_vals[idx].real = delval_list->val[idx].value
			                       * sdf_delay_scale(sdf_cur_cell);
		  }
	    }

	    vpi_put_delays(path, &delays);
	    match_count += 1;
      }

      if (match_count == 0) {
	    vpi_printf("SDF ERROR: %s:%d: Unable to match ModPath %s%s -> %s in %s\n",
	               sdf_fname, sdf_lineno,
	               edge_str(vpi_edge), src, dst,
	               vpi_get_str(vpiFullName, sdf_cur_cell));
      }
}

void sdf_iopath_delays(int vpi_edge, const char*src, const char*dst,
                       const struct sdf_delval_list_s*delval_list,
                       const int sdf_lineno)
{
      sdf_cond_iopath_delays(0, vpi_edge, src, dst, delval_list, sdf_lineno);
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

static PLI_INT32 sys_sdf_annotate_compiletf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall,0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle module;

      check_command_line_args();

	/* Check that we have a file name argument. */
      if (argv == 0) {
	    vpi_printf("SDF ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s requires a file name argument.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
	    return 0;
      }
      if (! is_string_obj(vpi_scan(argv))) {
	    vpi_printf("SDF ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's file name must be a string.\n", name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* The module argument is optional. */
      module = vpi_scan(argv);
      if (module == 0) return 0;
      if (vpi_get(vpiType, module) != vpiModule) {
	    vpi_printf("SDF ERROR: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s's second argument must be a module instance.\n",
	               name);
	    vpip_set_return_value(1);
	    vpi_control(vpiFinish, 1);
      }

	/* Warn the user that we only use the first two arguments. */
      if (vpi_scan(argv) != 0) {
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("%s currently only uses the first two argument.\n",
	               name);
	    vpi_free_object(argv);
      }

      return 0;
}

/* Check the same scope-precision rounding and global tick range used by
 * the single-limit timing object's vpi_put_delays implementation. The VPI setter returns
 * void, so reject an unrepresentable SDF value before reporting annotation. */
static int sdf_tchk_limit_valid(double value, vpiHandle scope)
{
      if (!scope || !isfinite(value) || value < 0) return 0;
      int precision = vpi_get(vpiTimePrecision, scope);
      int local_shift = vpi_get(vpiTimeUnit, scope) - precision;
      int global_shift = precision - vpi_get(vpiTimePrecision, 0);
      if (local_shift < 0 || global_shift < 0) return 0;
      double scaled = value * pow(10.0, local_shift);
      if (!isfinite(scaled)) return 0;
      double rounded = floor(scaled + 0.5);
      long double total = (long double)rounded * powl(10.0L, global_shift);
      return isfinite(total) && total < ldexpl(1.0L, 64);
}

/* Single-limit matching follows the timing-check object/terminal scheme
 * introduced by Leo Moser's timingcheck-width work. Values use the SDF header
 * timescale; object type prevents WIDTH/PERIOD cross-binding. */
static void sdf_single_limit(int check_type, const char* check_name,
                             int edge, const char* reference,
                             struct sdf_delay_s limit, int lineno)
{
      if (!sdf_cur_cell || !limit.defined) return;
      if (!isfinite(limit.value) || limit.value < 0) {
            vpi_printf("SDF ERROR: %s:%d: invalid %s limit\n", sdf_fname, lineno, check_name);
            return;
      }
      vpiHandle checks = vpi_iterate(vpiTchk, sdf_cur_cell);
      vpiHandle check;
      unsigned matches = 0;
      while (checks && (check = vpi_scan(checks))) {
            if (vpi_get(vpiTchkType, check) != check_type) continue;
            vpiHandle term = vpi_handle(vpiTchkRefTerm, check);
            if (!term || (edge != vpiNoEdge && vpi_get(vpiEdge, term) != edge)) continue;
            vpiHandle signal = vpi_handle(vpiExpr, term);
            if (!signal || strcmp(reference, vpi_get_str(vpiName, signal))) continue;
            /* Count object bindings, not successful value updates. */
            matches += 1;
            vpiHandle scope = vpi_handle(vpiScope, check);
            double scaled_limit = limit.value * sdf_delay_scale(scope);
            if (!sdf_tchk_limit_valid(scaled_limit, scope)) {
                  vpi_printf("SDF ERROR: %s:%d: unrepresentable %s limit for %s\n",
                             sdf_fname, lineno, check_name, reference);
                  continue;
            }
            s_vpi_time value;
            s_vpi_delay delays;
            memset(&value, 0, sizeof value);
            memset(&delays, 0, sizeof delays);
            value.type = vpiScaledRealTime;
            value.real = scaled_limit;
            delays.da = &value;
            delays.no_of_delays = 1;
            delays.time_type = vpiScaledRealTime;
            vpi_put_delays(check, &delays);
            if (sdf_flag_inform)
                  vpi_printf("SDF INFO: %s:%d: %s %s limit %g\n",
                             sdf_fname, lineno, check_name, reference, value.real);
      }
      if (!matches)
            vpi_printf("SDF ERROR: %s:%d: Unable to match %s %s%s in %s\n",
                       sdf_fname, lineno, check_name, edge_str(edge), reference,
                       vpi_get_str(vpiFullName, sdf_cur_cell));
}

void sdf_width_limits(int edge, const char* reference,
                      struct sdf_delay_s limit, int lineno)
{
      sdf_single_limit(vpiWidth, "WIDTH", edge, reference, limit, lineno);
}

void sdf_period_limits(int edge, const char* reference,
                       struct sdf_delay_s limit, int lineno)
{
      if (!sdf_cur_cell_type_matches) return;
      sdf_single_limit(vpiPeriod, "PERIOD", edge, reference, limit, lineno);
}

/* Match the supported source expression, not its value at annotation time.
 * An absent SDF condition is a wildcard; an explicit one requires the same
 * local scalar signal and expression form on that timing-check terminal. */
static int sdf_tchk_condition_matches(vpiHandle term,
                                     struct sdf_tchk_condition_s condition)
{
      if (condition.kind == SDF_TCHK_COND_NONE) return 1;
      vpiHandle expression = vpi_handle(vpiCondition, term);
      if (!expression) return 0;
      vpiHandle signal = expression;
      if (condition.kind == SDF_TCHK_COND_EQ0 ||
          condition.kind == SDF_TCHK_COND_EQ1) {
            if (vpi_get(vpiType, expression) != vpiOperation ||
                vpi_get(vpiOpType, expression) != vpiCaseEqOp) return 0;
            vpiHandle operands = vpi_iterate(vpiOperand, expression);
            vpiHandle operand, pair[2];
            unsigned count = 0;
            while (operands && (operand = vpi_scan(operands))) {
                  if (count == 2) {
                        vpi_free_object(operands);
                        return 0;
                  }
                  pair[count++] = operand;
            }
            if (count != 2 || vpi_get(vpiType, pair[1]) != vpiConstant ||
                vpi_get(vpiSize, pair[1]) != 1) return 0;
            s_vpi_value value;
            value.format = vpiScalarVal;
            vpi_get_value(pair[1], &value);
            int wanted = condition.kind == SDF_TCHK_COND_EQ0 ? vpi0 : vpi1;
            if (value.value.scalar != wanted) return 0;
            signal = pair[0];
      } else if (condition.kind != SDF_TCHK_COND_BARE) return 0;
      int type = vpi_get(vpiType, signal);
      if ((type != vpiNet && type != vpiReg) || vpi_get(vpiSize, signal) != 1)
            return 0;
      const char* name = vpi_get_str(vpiName, signal);
      return name && !strcmp(condition.name, name);
}

static int sdf_tchk_terminal_matches(vpiHandle check, int role,
                                    struct sdf_tchk_port_s event)
{
      struct port_with_edge_s port = event.port;
      vpiHandle term = vpi_handle(role, check);
      if (!term || (port.vpi_edge != vpiNoEdge &&
                    vpi_get(vpiEdge, term) != port.vpi_edge)) return 0;
      vpiHandle signal = vpi_handle(vpiExpr, term);
      return signal && !strcmp(port.string_val, vpi_get_str(vpiName, signal)) &&
            sdf_tchk_condition_matches(term, event.condition);
}

/* SETUP/HOLD SDF records list data before reference. RECOVERY/REMOVAL
 * records list the asynchronous reference before the clock/data event.
 * Match object roles explicitly; an unqualified SDF port matches either edge. */
void sdf_two_event_limits(int check_type, struct sdf_tchk_port_s first,
                          struct sdf_tchk_port_s second,
                          struct sdf_delay_s limit, int lineno)
{
      const char* name;
      int reset_check;
      switch (check_type) {
          case vpiSetup: name = "SETUP"; reset_check = 0; break;
          case vpiHold: name = "HOLD"; reset_check = 0; break;
          case vpiRecovery: name = "RECOVERY"; reset_check = 1; break;
          case vpiRemoval: name = "REMOVAL"; reset_check = 1; break;
          default: assert(0); return;
      }
      struct sdf_tchk_port_s reference = reset_check ? first : second;
      struct sdf_tchk_port_s data = reset_check ? second : first;
      if (!data.port.string_val || !reference.port.string_val ||
          data.condition.kind == SDF_TCHK_COND_UNSUPPORTED ||
          reference.condition.kind == SDF_TCHK_COND_UNSUPPORTED) {
            vpi_printf("SDF WARNING: %s:%d: conditional or hierarchical %s event not supported.\n",
                       sdf_fname, lineno, name);
            return;
      }
      if (!sdf_cur_cell || !sdf_cur_cell_type_matches || !limit.defined) return;
      if (!isfinite(limit.value) || limit.value < 0) {
            vpi_printf("SDF ERROR: %s:%d: invalid %s limit\n",
                       sdf_fname, lineno, name);
            return;
      }
      vpiHandle checks = vpi_iterate(vpiTchk, sdf_cur_cell);
      vpiHandle check;
      unsigned matches = 0;
      while (checks && (check = vpi_scan(checks))) {
            if (vpi_get(vpiTchkType, check) != check_type ||
                !sdf_tchk_terminal_matches(check, vpiTchkDataTerm, data) ||
                !sdf_tchk_terminal_matches(check, vpiTchkRefTerm, reference))
                  continue;
            matches += 1;
            vpiHandle scope = vpi_handle(vpiScope, check);
            double scaled_limit = limit.value * sdf_delay_scale(scope);
            if (!sdf_tchk_limit_valid(scaled_limit, scope)) {
                  vpi_printf("SDF ERROR: %s:%d: unrepresentable %s limit\n",
                             sdf_fname, lineno, name);
                  continue;
            }
            s_vpi_time value;
            s_vpi_delay delays;
            memset(&value, 0, sizeof value);
            memset(&delays, 0, sizeof delays);
            value.type = vpiScaledRealTime;
            value.real = scaled_limit;
            delays.da = &value;
            delays.no_of_delays = 1;
            delays.time_type = vpiScaledRealTime;
            vpi_put_delays(check, &delays);
            if (sdf_flag_inform)
                  vpi_printf("SDF INFO: %s:%d: %s %s -> %s limit %g\n",
                             sdf_fname, lineno, name, first.port.string_val,
                             second.port.string_val, value.real);
      }
      if (!matches)
            vpi_printf("SDF ERROR: %s:%d: Unable to match %s %s -> %s in %s\n",
                       sdf_fname, lineno, name, first.port.string_val,
                       second.port.string_val, vpi_get_str(vpiFullName, sdf_cur_cell));
}

static PLI_INT32 sys_sdf_annotate_calltf(ICARUS_VPI_CONST PLI_BYTE8*name)
{
	/* IEEE 1497: a file with no (TIMESCALE ...) entry defaults to 1ns. Reset
	   here so a second $sdf_annotate cannot inherit the previous file's
	   timescale. */
      sdf_timescale = 1.0e-9;
      sdf_cur_cell_type_matches = 0;

      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      FILE *sdf_fd;
      char *fname = get_filename(callh, name, vpi_scan(argv));

      if (fname == 0) {
	    vpi_free_object(argv);
	    return 0;
      }

      if (sdf_flag_inform) {
	    vpi_printf("SDF INFO: Loading %s from %s:%d\n",
	               fname,
	               vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
      }

      sdf_fd = fopen(fname, "r");
      if (sdf_fd == 0) {
	    vpi_printf("SDF WARNING: %s:%d: ", vpi_get_str(vpiFile, callh),
	               (int)vpi_get(vpiLineNo, callh));
	    vpi_printf("Unable to open SDF file \"%s\"."
		       " Skipping this annotation.\n", fname);
	    vpi_free_object(argv);
	    free(fname);
	    return 0;
      }
      sdf_fname = fname;

	/* The optional second argument is the scope to annotate. */
      sdf_scope = vpi_scan(argv);
      if (sdf_scope) vpi_free_object(argv);
      else sdf_scope = vpi_handle(vpiScope, callh);

	/* Select which delay to use. */
      sdf_min_typ_max = vpi_get(_vpiDelaySelection, 0);

      sdf_cur_cell = 0;
      sdf_callh = callh;
      sdf_process_file(sdf_fd, fname);
      sdf_callh = 0;

      fclose(sdf_fd);
      sdf_fname = NULL;
      free(fname);
      return 0;
}

void sys_sdf_register(void)
{
      s_vpi_systf_data tf_data;
      vpiHandle res;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$sdf_annotate";
      tf_data.calltf    = sys_sdf_annotate_calltf;
      tf_data.compiletf = sys_sdf_annotate_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$sdf_annotate";
      res = vpi_register_systf(&tf_data);
      vpip_make_systf_system_defined(res);
}
