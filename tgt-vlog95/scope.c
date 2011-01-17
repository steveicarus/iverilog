/*
 * Copyright (C) 2010-2011 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

# include <inttypes.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"
# include "ivl_alloc.h"

static char*get_time_const(int time_value)
{
      switch (time_value) {
	case   2: return "100s";
	case   1: return "10s";
	case   0: return "1s";
	case  -1: return "100ms";
	case  -2: return "10ms";
	case  -3: return "1ms";
	case  -4: return "100us";
	case  -5: return "10us";
	case  -6: return "1us";
	case  -7: return "100ns";
	case  -8: return "10ns";
	case  -9: return "1ns";
	case -10: return "100ps";
	case -11: return "10ps";
	case -12: return "1ps";
	case -13: return "100fs";
	case -14: return "10fs";
	case -15: return "1fs";
	default:
	    fprintf(stderr, "Invalid time constant value %d.\n", time_value);
	    return "N/A";
      }
}

void emit_func_return(ivl_signal_t sig)
{
      if (ivl_signal_dimensions(sig) > 0) {
	    fprintf(stderr, "%s:%u: vlog95 error: A function cannot return "
	                    "an array.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_integer(sig)) {
	    fprintf(vlog_out, " integer");
      } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    fprintf(vlog_out, " real");
      } else {
	    int msb = ivl_signal_msb(sig);
	    int lsb = ivl_signal_lsb(sig);
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
      }
}

void emit_var_def(ivl_signal_t sig)
{
      fprintf(vlog_out, "%*c", indent, ' ');
      if (ivl_signal_integer(sig)) {
	    fprintf(vlog_out, "integer %s;\n", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Integer arrays are "
		                  "not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig));
		  vlog_errors += 1;
	    }
      } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    fprintf(vlog_out, "real %s;\n", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real arrays are "
		                  "not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig));
		  vlog_errors += 1;
	    }
      } else {
	    int msb = ivl_signal_msb(sig);
	    int lsb = ivl_signal_lsb(sig);
	    fprintf(vlog_out, "reg");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s", ivl_signal_basename(sig));
	    if (ivl_signal_dimensions(sig) > 0) {
		  unsigned wd_count = ivl_signal_array_count(sig);
		  int first = ivl_signal_array_base(sig);
		  int last = first + wd_count - 1;
		  if (ivl_signal_array_addr_swapped(sig)) {
			fprintf(vlog_out, " [%d:%d]", last, first);
		  } else {
			fprintf(vlog_out, " [%d:%d]", first, last);
		  }
	    }
	    fprintf(vlog_out, ";\n");
	    if (ivl_signal_signed(sig)) {
		  fprintf(stderr, "%s:%u: vlog95 error: Signed registers are "
		                  "not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig));
		  vlog_errors += 1;
	    }
      }
}

void emit_net_def(ivl_signal_t sig)
{
      fprintf(vlog_out, "%*c", indent, ' ');
      int msb = ivl_signal_msb(sig);
      int lsb = ivl_signal_lsb(sig);
      if (ivl_signal_data_type(sig) == IVL_VT_REAL){
	    fprintf(vlog_out, "wire %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Real nets are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_signed(sig)) {
	    fprintf(vlog_out, "wire");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Signed nets are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_dimensions(sig) > 0) {
	    fprintf(vlog_out, "wire");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
	    fprintf(stderr, "%s:%u: vlog95 error: Array nets are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig));
	    vlog_errors += 1;
      } else {
	    switch(ivl_signal_type(sig)) {
	      case IVL_SIT_TRI:
	      case IVL_SIT_UWIRE:
// HERE: Need to add support for supply nets. Probably supply strength
//       with a constant 0/1 driver for all the bits.
		  fprintf(vlog_out, "wire");
		  break;
	      case IVL_SIT_TRI0:
		  fprintf(vlog_out, "tri0");
		  break;
	      case IVL_SIT_TRI1:
		  fprintf(vlog_out, "tri1");
		  break;
	      case IVL_SIT_TRIAND:
		  fprintf(vlog_out, "wand");
		  break;
	      case IVL_SIT_TRIOR:
		  fprintf(vlog_out, "wor");
		  break;
	      default:
		  fprintf(vlog_out, "<unknown>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unknown net type "
	                    "(%d).\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), (int)ivl_signal_type(sig));
		  vlog_errors += 1;
		  break;
	    }
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(sig));
      }
}

static char *get_mangled_name(ivl_scope_t scope, unsigned root)
{
      char *name;
	/* If the module has parameters and it's not a root module than it
	 * may not be unique so we create a mangled name version instead. */
      if (ivl_scope_params(scope) && ! root) {
	    unsigned idx;
	    size_t len = strlen(ivl_scope_name(scope)) +
	                 strlen(ivl_scope_tname(scope)) + 2;
	    name = (char *)malloc(len);
	    (void) strcpy(name, ivl_scope_tname(scope));
	    (void) strcat(name, "_");
	    (void) strcat(name, ivl_scope_name(scope));
	    assert(name[len-1] == 0);
	    for (idx = 0; idx < len; idx += 1) {
		  if (name[idx] == '.') name[idx] = '_';
	    }
      } else {
	    name = strdup(ivl_scope_tname(scope));
      }
      return name;
}

static void emit_gate_drive(ivl_net_logic_t nlogic, ivl_drive_t drive)
{
      switch (drive) {
	case IVL_DR_HiZ:
	    fprintf(vlog_out, "highz");
	    break;
	case IVL_DR_WEAK:
	    fprintf(vlog_out, "weak");
	    break;
	case IVL_DR_PULL:
	    fprintf(vlog_out, "pull");
	    break;
	case IVL_DR_STRONG:
	    fprintf(vlog_out, "strong");
	    break;
	case IVL_DR_SUPPLY:
	    fprintf(vlog_out, "supply");
	    break;
	default:
	    fprintf(vlog_out, "<invalid>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported gate "
	                    "drive (%d)\n", ivl_logic_file(nlogic),
	                    ivl_logic_lineno(nlogic), (int)drive);
	    vlog_errors += 1;
	    break;
      }
}

static void emit_gate_strength(ivl_net_logic_t nlogic)
{
      ivl_drive_t drive1 = ivl_logic_drive1(nlogic);
      ivl_drive_t drive0 = ivl_logic_drive0(nlogic);
      if ((drive1 != IVL_DR_STRONG) || (drive0 != IVL_DR_STRONG)) {
	    fprintf(vlog_out, " (");
	    emit_gate_drive(nlogic, drive1);
	    fprintf(vlog_out, "1, ");
	    emit_gate_drive(nlogic, drive0);
	    fprintf(vlog_out, "0)");
      }
}

static void emit_gate_delay(ivl_scope_t scope, ivl_net_logic_t nlogic,
                            unsigned dly_count)
{
      ivl_expr_t rise = ivl_logic_delay(nlogic, 0);
      ivl_expr_t fall = ivl_logic_delay(nlogic, 1);
      ivl_expr_t decay = ivl_logic_delay(nlogic, 2);
      assert((dly_count >= 2) && (dly_count <= 3));

	/* No delays. */
      if (! rise) {
	    assert(! fall);
	    assert(! decay);
	    return;
      }
	/* If all three delays match then we only have a single delay. */
      if ((rise == fall) && (rise == decay)) {
	    fprintf(vlog_out, " #");
	    emit_scaled_delayx(scope, rise);
	    return;
      }
	/* If we have a gate that only supports two delays then print them. */
      if (dly_count == 2) {
	    fprintf(vlog_out, " #(");
	    emit_scaled_delayx(scope, rise);
	    fprintf(vlog_out, ",");
	    emit_scaled_delayx(scope, fall);
	    fprintf(vlog_out, ")");
	    return;
      }

	/* What's left is a gate that supports three delays. */
      fprintf(vlog_out, " #(");
      emit_scaled_delayx(scope, rise);
      fprintf(vlog_out, ",");
      emit_scaled_delayx(scope, fall);
      if (decay) {
	    fprintf(vlog_out, ",");
	    emit_scaled_delayx(scope, decay);
      }
      fprintf(vlog_out, ")");
}

static void emit_logic(ivl_scope_t scope, ivl_net_logic_t nlogic)
{
      unsigned idx, count, dly_count;
      fprintf(vlog_out, "%*c", indent, ' ');
      switch (ivl_logic_type(nlogic)) {
	case IVL_LO_AND:
            fprintf(vlog_out, "and");
            dly_count = 2;
	    break;
	case IVL_LO_BUF:
            fprintf(vlog_out, "buf");
            dly_count = 2;
	    break;
	case IVL_LO_BUFIF0:
            fprintf(vlog_out, "bufif0");
            dly_count = 3;
	    break;
	case IVL_LO_BUFIF1:
            fprintf(vlog_out, "bufif1");
            dly_count = 3;
	    break;
	case IVL_LO_CMOS:
            fprintf(vlog_out, "cmos");
            dly_count = 3;
	    break;
	case IVL_LO_NAND:
            fprintf(vlog_out, "nand");
            dly_count = 2;
	    break;
	case IVL_LO_NMOS:
            fprintf(vlog_out, "nmos");
            dly_count = 3;
	    break;
	case IVL_LO_NOR:
            fprintf(vlog_out, "nor");
            dly_count = 2;
	    break;
	case IVL_LO_NOT:
            fprintf(vlog_out, "not");
            dly_count = 2;
	    break;
	case IVL_LO_NOTIF0:
            fprintf(vlog_out, "notif0");
            dly_count = 3;
	    break;
	case IVL_LO_NOTIF1:
            fprintf(vlog_out, "notif1");
            dly_count = 3;
	    break;
	case IVL_LO_OR:
            fprintf(vlog_out, "or");
            dly_count = 2;
	    break;
	case IVL_LO_PMOS:
            fprintf(vlog_out, "pmos");
            dly_count = 3;
	    break;
	case IVL_LO_PULLDOWN:
            fprintf(vlog_out, "pulldown");
            dly_count = 0;
	    break;
	case IVL_LO_PULLUP:
            fprintf(vlog_out, "pullup");
            dly_count = 0;
	    break;
	case IVL_LO_RCMOS:
            fprintf(vlog_out, "rcmos");
            dly_count = 3;
	    break;
	case IVL_LO_RNMOS:
            fprintf(vlog_out, "rnmos");
            dly_count = 3;
	    break;
	case IVL_LO_RPMOS:
            fprintf(vlog_out, "rpmos");
            dly_count = 3;
	    break;
	case IVL_LO_XNOR:
            fprintf(vlog_out, "xnor");
            dly_count = 2;
	    break;
	case IVL_LO_XOR:
            fprintf(vlog_out, "xor");
            dly_count = 2;
	    break;
	default:
// HERE: Missing support for BUFT, BUFZ, UDP.
            fprintf(vlog_out, "<unknown>(");
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported logic type "
	                    "(%d) named: %s.\n", ivl_logic_file(nlogic),
	                    ivl_logic_lineno(nlogic), ivl_logic_type(nlogic),
	                    ivl_logic_basename(nlogic));
            vlog_errors += 1;
            dly_count = 0;
	    break;
      }
      emit_gate_strength(nlogic);
      if (dly_count) emit_gate_delay(scope, nlogic, dly_count);
      fprintf(vlog_out, " %s(", ivl_logic_basename(nlogic));
      count = ivl_logic_pins(nlogic);
      count -= 1;
      for (idx = 0; idx < count; idx += 1) {
	    emit_name_of_nexus(ivl_logic_pin(nlogic, idx));
	    fprintf(vlog_out, ", ");
      }
      emit_name_of_nexus(ivl_logic_pin(nlogic, count));
      fprintf(vlog_out, ");\n");
}

/*
 * This function is called for each process in the design so that we
 * can extract the processes for the given scope.
 */
static int find_process(ivl_process_t proc, ivl_scope_t scope)
{
      if (scope == ivl_process_scope(proc)) emit_process(scope, proc);
      return 0;
}

/*
 * This search method may be slow for a large structural design with a
 * large number of gate types. That's not what this converter was built
 * for so this is probably OK. If this becomes an issue then we need a
 * better method/data structure.
*/
static const char **scopes_emitted = 0;
static unsigned num_scopes_emitted = 0;

static unsigned scope_has_been_emitted(ivl_scope_t scope)
{
      unsigned idx;
      for (idx = 0; idx < num_scopes_emitted; idx += 1) {
	    if (! strcmp(ivl_scope_tname(scope), scopes_emitted[idx])) return 1;
      }
      return 0;
}

static void add_scope_to_list(ivl_scope_t scope)
{
      num_scopes_emitted += 1;
      scopes_emitted = realloc(scopes_emitted, num_scopes_emitted *
                                               sizeof(char *));
      scopes_emitted[num_scopes_emitted-1] = ivl_scope_tname(scope);
}

void free_emitted_scope_list()
{
      free(scopes_emitted);
      scopes_emitted = 0;
      num_scopes_emitted = 0;
}

/*
 * A list of module scopes that need to have their definition emitted when
 * the current root scope (module) is finished is kept here.
 */
static ivl_scope_t *scopes_to_emit = 0;
static unsigned num_scopes_to_emit = 0;
static unsigned emitting_scopes = 0;

int emit_scope(ivl_scope_t scope, ivl_scope_t parent)
{
      ivl_scope_type_t sc_type = ivl_scope_type(scope);
      unsigned is_auto = ivl_scope_is_auto(scope);
      unsigned idx, count, start = 0;
      char *name;

	/* Output the scope definition. */
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(!is_auto);
	    name = get_mangled_name(scope, !parent && !emitting_scopes);
	      /* This is an instantiation. */
	    if (parent) {
		  assert(indent != 0);
		    /* If the module has parameters than it may not be unique
		     * so we create a mangled name version instead. */
		  fprintf(vlog_out, "\n%*c%s %s(", indent, ' ', name,
		                    ivl_scope_basename(scope));
// HERE: Still need to add port information.
		  fprintf(vlog_out, ");\n");
		  free(name);
		  num_scopes_to_emit += 1;
		  scopes_to_emit = realloc(scopes_to_emit, num_scopes_to_emit *
		                                           sizeof(ivl_scope_t));
		  scopes_to_emit[num_scopes_to_emit-1] = scope;
		  return 0;
	    }
	    assert(indent == 0);
	      /* Set the time scale for this scope. */
	    fprintf(vlog_out, "\n`timescale %s/%s\n",
	                      get_time_const(ivl_scope_time_units(scope)),
	                      get_time_const(ivl_scope_time_precision(scope)));
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`celldefine\n");
	    }
	    fprintf(vlog_out, "module %s", name);
	    free(name);
// HERE: Still need to add port information.
	    break;
	case IVL_SCT_FUNCTION:
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*cfunction", indent, ' ');
	    assert(ivl_scope_ports(scope) >= 2);
	      /* The function return information is the zero port. */
	    emit_func_return(ivl_scope_port(scope, 0));
	    start = 1;
	    fprintf(vlog_out, " %s", ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic function is "
	                    "not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_TASK:
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*ctask %s", indent, ' ',
	                      ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic task is "
	                    "not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_BEGIN:
	case IVL_SCT_FORK:
	    assert(indent != 0);
	    return 0; /* A named begin/fork is handled in line. */
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported scope type "
	                    "(%d) named: %s.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), sc_type,
	                    ivl_scope_tname(scope));
	    vlog_errors += 1;
	    return 0;
      }
      fprintf(vlog_out, ";\n");
      indent += indent_incr;

	/* Output the scope ports. */
      count = ivl_scope_ports(scope);
      for (idx = start; idx < count; idx += 1) {
	    fprintf(vlog_out, "%*c", indent, ' ');
	    ivl_signal_t port = ivl_scope_port(scope, idx);
	    switch (ivl_signal_port(port)) {
	      case IVL_SIP_INPUT:
		  fprintf(vlog_out, "input");
		  break;
	      case IVL_SIP_OUTPUT:
		  fprintf(vlog_out, "output");
		  break;
	      case IVL_SIP_INOUT:
		  fprintf(vlog_out, "inout");
		  break;
	      default:
		  fprintf(vlog_out, "<unknown>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unknown port "
	                    "direction (%d).\n", ivl_signal_file(port),
	                    ivl_signal_lineno(port),
	                    (int)ivl_signal_port(port));
		  vlog_errors += 1;
		  break;
	    }
	    fprintf(vlog_out, " %s;\n", ivl_signal_basename(port));
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the scope parameters. */
      count = ivl_scope_params(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	    ivl_expr_t pex = ivl_parameter_expr(par);
	    fprintf(vlog_out, "%*cparameter %s = ", indent, ' ',
	                      ivl_parameter_basename(par));
	    emit_expr(scope, pex, 0);
	    fprintf(vlog_out, ";\n");
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the scope signals. */
      count = ivl_scope_sigs(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    if (ivl_signal_type(sig) == IVL_SIT_REG) {
		    /* Do not output the implicit function return register. */
		  if (sc_type == IVL_SCT_FUNCTION &&
                      strcmp(ivl_signal_basename(sig),
		              ivl_scope_tname(scope)) == 0) continue;
		  emit_var_def(sig);
	    } else {
		  emit_net_def(sig);
	    }
      }

// HERE: Need to find and print any continuous assignments.

      if (sc_type == IVL_SCT_MODULE) {
	      /* Output any logic devices. */
	    count = ivl_scope_logs(scope);
	    for (idx = 0; idx < count; idx += 1) {
		  emit_logic(scope, ivl_scope_log(scope, idx));
	    }

	      /* Output the initial/always blocks for this module. */
	    ivl_design_process(design, (ivl_process_f)find_process, scope);
      }

	/* Output the function/task body. */
      if (sc_type == IVL_SCT_TASK || sc_type == IVL_SCT_FUNCTION) {
	    emit_stmt(scope, ivl_scope_def(scope));
      }

	/* Now print out any sub-scopes. */
      ivl_scope_children(scope, (ivl_scope_f*) emit_scope, scope);

	/* Output the scope ending. */
      assert(indent >= indent_incr);
      indent -= indent_incr;
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(indent == 0);
	    fprintf(vlog_out, "endmodule  /* %s */\n", ivl_scope_tname(scope));
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`endcelldefine\n");
	    }
	      /* If this is a root scope then emit any saved instance scopes.
	       * Save any scope that does not have parameters/a mangled name
	       * to a list so we don't print duplicate module definitions. */
	    if (!emitting_scopes) {
		  emitting_scopes = 1;
		  for (idx =0; idx < num_scopes_to_emit; idx += 1) {
			ivl_scope_t scope_to_emit = scopes_to_emit[idx];
			if (scope_has_been_emitted(scope_to_emit)) continue;
			(void) emit_scope(scope_to_emit, 0);
			  /* If we used a mangled name then the instance is
			   * unique so don't add it to the list. */
			if (ivl_scope_params(scope_to_emit)) continue;
			add_scope_to_list(scope_to_emit);
		  }
		  free(scopes_to_emit);
		  scopes_to_emit = 0;
		  num_scopes_to_emit = 0;
		  emitting_scopes = 0;
	    }
	    break;
	case IVL_SCT_FUNCTION:
	    fprintf(vlog_out, "%*cendfunction  /* %s */\n", indent, ' ',
	                      ivl_scope_tname(scope));
	    break;
	case IVL_SCT_TASK:
	    fprintf(vlog_out, "%*cendtask  /* %s */\n", indent, ' ',
	                      ivl_scope_tname(scope));
	    break;
	default:
	    assert(0);
	    break;
      }
      return 0;
}
