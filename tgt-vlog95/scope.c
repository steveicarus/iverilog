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
 *
 *
 * This is the vlog95 target module. It generates a 1364-1995 compliant
 * netlist from the input netlist. The generated netlist is expected to
 * be simulation equivalent to the original.
 */

# include <inttypes.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"

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
	    fprintf(stderr, "Invalid time constant %d\n", time_value);
	    assert(0);
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

int emit_scope(ivl_scope_t scope, ivl_scope_t parent)
{
      ivl_scope_type_t sc_type = ivl_scope_type(scope);
      unsigned is_auto = ivl_scope_is_auto(scope);
      unsigned idx, count, start = 0;

	/* Output the scope definition. */
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(!is_auto);
	    assert(indent == 0);
	      /* Set the time scale for this scope. */
	    fprintf(vlog_out, "\n`timescale %s/%s\n",
	                      get_time_const(ivl_scope_time_units(scope)),
	                      get_time_const(ivl_scope_time_precision(scope)));
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`celldefine\n");
	    }
	    fprintf(vlog_out, "module %s", ivl_scope_tname(scope));
// HERE: Still need to add port information.
	    break;
	case IVL_SCT_FUNCTION:
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
	    fprintf(vlog_out, "endmodule\n");
	    if (ivl_scope_is_cell(scope)) {
		  fprintf(vlog_out, "`endcelldefine\n");
	    }
	    break;
	case IVL_SCT_FUNCTION:
	    fprintf(vlog_out, "%*cendfunction\n", indent, ' ');
	    break;
	case IVL_SCT_TASK:
	    fprintf(vlog_out, "%*cendtask\n", indent, ' ');
	    break;
	default:
	    assert(0);
	    break;
      }
      return 0;
}
