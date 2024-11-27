/*
 * Copyright (C) 2010-2024 Cary R. (cygcary@yahoo.com)
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

const char *func_rtn_name = 0;

static void emit_func_return(ivl_signal_t sig)
{
        // Handle SV void functions.
      if (sig == 0)
            return;

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
	    if (ivl_signal_signed(sig)) {
		  if (allow_signed) {
			fprintf(vlog_out, " signed");
		  } else {
			fprintf(stderr, "%s:%u: vlog95 error: Signed return "
			                "value for function `%s` is not "
					"supported.\n",
			                ivl_signal_file(sig),
			                ivl_signal_lineno(sig),
			                ivl_signal_basename(sig));
			vlog_errors += 1;
		  }
	    }
	    int msb, lsb;
	    get_sig_msb_lsb(sig, &msb, &lsb);
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, " [%d:%d]", msb, lsb);
      }
}

void emit_sig_file_line(ivl_signal_t sig)
{
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_signal_file(sig),
	                      ivl_signal_lineno(sig));
      }
}

static void emit_sig_id(ivl_signal_t sig)
{
      emit_id(ivl_signal_basename(sig));
      fprintf(vlog_out, ";");
      emit_sig_file_line(sig);
      fprintf(vlog_out, "\n");
}

static void emit_var_def(ivl_signal_t sig)
{
      if (ivl_signal_local(sig)) return;
      fprintf(vlog_out, "%*c", indent, ' ');
      if (ivl_signal_integer(sig)) {
	    fprintf(vlog_out, "integer ");
	    emit_sig_id(sig);
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Integer arrays (%s) "
		                  "are not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    }
      } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
	    fprintf(vlog_out, "real ");
	    emit_sig_id(sig);
	    if (ivl_signal_dimensions(sig) > 0) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real arrays (%s) "
		                  "are not supported.\n", ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    }
      } else if (ivl_signal_data_type(sig) == IVL_VT_STRING) {
	    fprintf(vlog_out, "string ");
	    emit_sig_id(sig);
	    fprintf(stderr, "%s:%u: vlog95 error: SystemVerilog strings (%s) "
	                    "are not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_data_type(sig) == IVL_VT_DARRAY) {
	    fprintf(vlog_out, "<dynamic array> ");
	    emit_sig_id(sig);
	    fprintf(stderr, "%s:%u: vlog95 error: SystemVerilog dynamic "
	                    "arrays (%s) are not supported.\n",
	                    ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_data_type(sig) == IVL_VT_QUEUE) {
	    fprintf(vlog_out, "<queue> ");
	    emit_sig_id(sig);
	    fprintf(stderr, "%s:%u: vlog95 error: SystemVerilog queues "
	                    "(%s) are not supported.\n",
	                    ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else {
	    int msb, lsb;
	    get_sig_msb_lsb(sig, &msb, &lsb);
	    fprintf(vlog_out, "reg ");
	    if (ivl_signal_signed(sig)) {
		  if (allow_signed) {
			fprintf(vlog_out, "signed ");
		  } else {
			fprintf(stderr, "%s:%u: vlog95 error: Signed registers "
			                "(%s) are not supported.\n",
			                ivl_signal_file(sig),
			                ivl_signal_lineno(sig),
			                ivl_signal_basename(sig));
			vlog_errors += 1;
		  }
	    }
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, "[%d:%d] ", msb, lsb);
	    emit_id(ivl_signal_basename(sig));
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
	    fprintf(vlog_out, ";");
	    emit_sig_file_line(sig);
	    fprintf(vlog_out, "\n");
      }
}

/*
 * Keep a list of constants that drive nets and need to be emitted as
 * a continuous assignment.
*/
typedef struct {
            ivl_signal_t    sig;
            ivl_nexus_ptr_t nex_ptr;
        } net_const_t;

static net_const_t *net_consts = 0;
static unsigned num_net_consts = 0;

static void add_net_const_to_list(ivl_signal_t sig, ivl_nexus_ptr_t nex_ptr)
{
      num_net_consts += 1;
      net_consts = realloc(net_consts, num_net_consts * sizeof(net_const_t));
      net_consts[num_net_consts-1].sig = sig;
      net_consts[num_net_consts-1].nex_ptr = nex_ptr;
}

static unsigned emit_and_free_net_const_list(ivl_scope_t scope)
{
      unsigned idx;
      for (idx = 0; idx < num_net_consts; idx += 1) {
	    emit_signal_net_const_as_ca(scope, net_consts[idx].sig,
                                        net_consts[idx].nex_ptr);
      }
      free(net_consts);
      net_consts = 0;
      idx = num_net_consts != 0;
      num_net_consts = 0;
      return idx;
}

static void save_net_constants(const ivl_scope_t scope, ivl_signal_t sig)
{
      ivl_nexus_t nex = ivl_signal_nex(sig, 0);
      unsigned idx, count = ivl_nexus_ptrs(nex);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_net_const_t net_const = ivl_nexus_ptr_con(nex_ptr);
	    if (! net_const) continue;
	    if (scope != ivl_const_scope(net_const)) continue;
	    add_net_const_to_list(sig, nex_ptr);
      }
}

static void emit_net_def(ivl_scope_t scope, ivl_signal_t sig)
{
      int msb, lsb;
      get_sig_msb_lsb(sig, &msb, &lsb);
      if (ivl_signal_local(sig)) return;
      fprintf(vlog_out, "%*c", indent, ' ');
      if (ivl_signal_data_type(sig) == IVL_VT_REAL){
	    fprintf(vlog_out, "wire ");
	    emit_sig_id(sig);
	    fprintf(stderr, "%s:%u: vlog95 error: Real nets (%s) are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else if (ivl_signal_dimensions(sig) > 0) {
	    fprintf(vlog_out, "wire ");
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, "[%d:%d] ", msb, lsb);
	    emit_sig_id(sig);
	    fprintf(stderr, "%s:%u: vlog95 error: Array nets (%s) are "
	                    "not supported.\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), ivl_signal_basename(sig));
	    vlog_errors += 1;
      } else {
	    switch (ivl_signal_type(sig)) {
	      case IVL_SIT_TRI:
	      case IVL_SIT_UWIRE:
// HERE: Need to add support for supply nets. Probably supply strength
//       with a constant 0/1 driver for all the bits.
		  fprintf(vlog_out, "wire ");
		  break;
	      case IVL_SIT_TRI0:
		  fprintf(vlog_out, "tri0 ");
		  break;
	      case IVL_SIT_TRI1:
		  fprintf(vlog_out, "tri1 ");
		  break;
	      case IVL_SIT_TRIAND:
		  fprintf(vlog_out, "wand ");
		  break;
	      case IVL_SIT_TRIOR:
		  fprintf(vlog_out, "wor ");
		  break;
	      default:
		  fprintf(vlog_out, "<unknown> ");
		  fprintf(stderr, "%s:%u: vlog95 error: Unknown net type "
	                    "(%d).\n", ivl_signal_file(sig),
	                    ivl_signal_lineno(sig), (int)ivl_signal_type(sig));
		  vlog_errors += 1;
		  break;
	    }
	    if (ivl_signal_signed(sig)) {
		  if (allow_signed) {
			fprintf(vlog_out, "signed ");
		  } else {
			fprintf(stderr, "%s:%u: vlog95 error: Signed nets (%s) "
			                "are not supported.\n",
			                ivl_signal_file(sig),
			                ivl_signal_lineno(sig),
			                ivl_signal_basename(sig));
			vlog_errors += 1;
		  }
	    }
	    if (msb != 0 || lsb != 0) fprintf(vlog_out, "[%d:%d] ", msb, lsb);
	    emit_sig_id(sig);
	      /* A constant driving a net does not create an lpm or logic
	       * element in the design so save them from the definition. */
	    save_net_constants(scope, sig);
      }
}

static bool scope_is_unique(ivl_scope_t scope)
{
      unsigned int count = ivl_scope_params(scope);

      for (unsigned int idx = 0; idx < count; idx++) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	    if (!ivl_parameter_local(par)) {
		  return false;
	    }
      }

      return true;
}

static void emit_mangled_name(ivl_scope_t scope, unsigned root)
{
	/* If the module has non-local parameters and it's not a root module then it
	 * may not be unique so we create a mangled name version instead. The
	 * mangled name is of the form:
	 *   <module_name>[<full_instance_scope>]. */
      if (!root && !scope_is_unique(scope)) {
	    char *name;
	    size_t len = strlen(ivl_scope_name(scope)) +
	                 strlen(ivl_scope_tname(scope)) + 3;
	    name = (char *)malloc(len);
	    (void) strcpy(name, ivl_scope_tname(scope));
	    (void) strcat(name, "[");
	    (void) strcat(name, ivl_scope_name(scope));
	    (void) strcat(name, "]");
	    assert(name[len-1] == 0);
	      /* Emit the mangled name as an escaped identifier. */
	    fprintf(vlog_out, "\\%s ", name);
	    free(name);
      } else {
	    emit_id(ivl_scope_tname(scope));
      }
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

void emit_scope_variables(ivl_scope_t scope)
{
      unsigned idx, count;
      assert(! num_net_consts);
	/* Output the parameters for this scope. */
      count = ivl_scope_params(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	      // vlog95 does not support type parameters. Places where type
	      // parameters have been used it will be replaced with the actual
	      // type that the module was instantiated with. Similar to
	      // typedefs.
	    if (ivl_parameter_is_type(par))
		  continue;

	    ivl_expr_t pex = ivl_parameter_expr(par);
	    fprintf(vlog_out, "%*cparameter ", indent, ' ');
	    emit_id(ivl_parameter_basename(par));
	    fprintf(vlog_out, " = ");
	      /* Need to emit the parameters value not its name. */
	    emitting_param = par;
	    emit_expr(scope, pex, ivl_parameter_width(par), 1, 0, 0);
	    emitting_param = 0;
	    fprintf(vlog_out, ";");
	    if (emit_file_line) {
		  fprintf(vlog_out, " /* %s:%u */",
		                    ivl_parameter_file(par),
		                    ivl_parameter_lineno(par));
	    }
	    fprintf(vlog_out, "\n");
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the signals for this scope. */
      count = ivl_scope_sigs(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(scope, idx);
	    if (ivl_signal_type(sig) == IVL_SIT_REG) {
		    /* Do not output the implicit function return register. */
		  if (ivl_scope_type(scope) == IVL_SCT_FUNCTION &&
                      strcmp(ivl_signal_basename(sig),
		              ivl_scope_tname(scope)) == 0) continue;
		  emit_var_def(sig);
	    } else {
		  emit_net_def(scope, sig);
	    }
      }
      if (count) fprintf(vlog_out, "\n");

	/* Output the named events for this scope. */
      count = ivl_scope_events(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_event_t event = ivl_scope_event(scope, idx);
	      /* If this event has any type of edge sensitivity then it is
	       * not a named event. */
	    if (ivl_event_nany(event)) continue;
	    if (ivl_event_npos(event)) continue;
	    if (ivl_event_nneg(event)) continue;
	    fprintf(vlog_out, "%*cevent ", indent, ' ');
	    emit_id(ivl_event_basename(event));
	    fprintf(vlog_out, ";");
	    if (emit_file_line) {
		  fprintf(vlog_out, " /* %s:%u */",
		                    ivl_event_file(event),
		                    ivl_event_lineno(event));
	    }
	    fprintf(vlog_out, "\n");
      }
      if (count) fprintf(vlog_out, "\n");
      if (emit_and_free_net_const_list(scope)) fprintf(vlog_out, "\n");
}

static void emit_scope_file_line(ivl_scope_t scope)
{
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_scope_file(scope),
	                      ivl_scope_lineno(scope));
      }
}

static void emit_module_ports(ivl_scope_t scope)
{
      unsigned idx, count = ivl_scope_ports(scope);

      if (count == 0) return;

      fprintf(vlog_out, "(");
      emit_nexus_as_ca(scope, ivl_scope_mod_port(scope, 0), 0, 0);
      for (idx = 1; idx < count; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_nexus_as_ca(scope, ivl_scope_mod_port(scope, idx), 0, 0);
      }
      fprintf(vlog_out, ")");
}

static ivl_signal_t get_port_from_nexus(const ivl_scope_t scope, ivl_nexus_t nex,
	                                unsigned *word)
{
      assert(nex);
      unsigned idx, count = ivl_nexus_ptrs(nex);
      ivl_signal_t sig = 0;
      *word = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_ptr_t nex_ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t t_sig = ivl_nexus_ptr_sig(nex_ptr);
	    if (t_sig) {
		  if (ivl_signal_scope(t_sig) != scope) continue;
		  assert(! sig);
		  sig = t_sig;
		  *word = ivl_nexus_ptr_pin(nex_ptr);
	    }
      }
      return sig;
}

static void emit_sig_type(ivl_signal_t sig)
{
      ivl_signal_type_t type = ivl_signal_type(sig);
      if (ivl_signal_dimensions(sig) != 0) {
	    fprintf(stderr, "%s:%u: vlog95 error: Array ports (%s) are not "
	                    "supported.\n",
	                    ivl_signal_file(sig),
	                    ivl_signal_lineno(sig),
	                    ivl_signal_basename(sig));
	    vlog_errors += 1;
      }
	/* Check to see if we have a variable (reg) or a net. */
      if (type == IVL_SIT_REG) {
	      /* The variable data type will be declared later, so here
		 we just want to declare the range and whether or not it
		 is signed. */
	    if (ivl_signal_integer(sig)) {
		  /* nothing to do */
	    } else if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
		  /* nothing to do */
	    } else {
		  int msb, lsb;
		  get_sig_msb_lsb(sig, &msb, &lsb);
		  if (ivl_signal_signed(sig)) {
			if (allow_signed) {
			      fprintf(vlog_out, " signed");
			} else {
			      fprintf(stderr, "%s:%u: vlog95 error: Signed "
			                      "ports (%s) are not supported.\n",
			                      ivl_signal_file(sig),
			                      ivl_signal_lineno(sig),
			                      ivl_signal_basename(sig));
			      vlog_errors += 1;
			}
		  }
		  if (msb != 0 || lsb != 0) {
			fprintf(vlog_out, " [%d:%d]", msb, lsb);
		  }
	    }
      } else {
	    assert((type == IVL_SIT_TRI) ||
	           (type == IVL_SIT_TRI0) ||
	           (type == IVL_SIT_TRI1) ||
	           (type == IVL_SIT_UWIRE));
	    if (ivl_signal_data_type(sig) == IVL_VT_REAL) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real net ports (%s) "
		                  "are not supported.\n",
		                  ivl_signal_file(sig),
		                  ivl_signal_lineno(sig),
		                  ivl_signal_basename(sig));
		  vlog_errors += 1;
	    } else {
		  int msb, lsb;
		  get_sig_msb_lsb(sig, &msb, &lsb);
		  if (ivl_signal_signed(sig)) {
			if (allow_signed) {
			      fprintf(vlog_out, " signed");
			} else {
			      fprintf(stderr, "%s:%u: vlog95 error: Signed net "
			                      "ports (%s) are not supported.\n",
			                      ivl_signal_file(sig),
			                      ivl_signal_lineno(sig),
			                      ivl_signal_basename(sig));
			      vlog_errors += 1;
			}
		  }
		  if (msb != 0 || lsb != 0) {
			fprintf(vlog_out, " [%d:%d]", msb, lsb);
		  }
	    }
      }
}

static void emit_port(ivl_signal_t port)
{
      assert(port);
      fprintf(vlog_out, "%*c", indent, ' ');
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
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown port direction (%d) "
	                    "for signal %s.\n", ivl_signal_file(port),
	                    ivl_signal_lineno(port), (int)ivl_signal_port(port),
	                    ivl_signal_basename(port));
	    vlog_errors += 1;
	    break;
      }
      emit_sig_type(port);
      fprintf(vlog_out, " ");
	/* Split port (arg[7:4],arg[3:0]) are generated using local signals. */
      if (ivl_signal_local(port)) {
	    fprintf(vlog_out, "ivlog%s", ivl_signal_basename(port));
      } else {
	    emit_id(ivl_signal_basename(port));
      }
      fprintf(vlog_out, ";");
      emit_sig_file_line(port);
      fprintf(vlog_out, "\n");
}

static void emit_module_port_defs(ivl_scope_t scope)
{
      unsigned word, idx, count = ivl_scope_ports(scope);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_t nex = ivl_scope_mod_port(scope, idx);
	    ivl_signal_t port = get_port_from_nexus(scope, nex, &word);
// HERE: Do we need to use word?
	    if (port) emit_port(port);
	    else {
		  fprintf(vlog_out, "<missing>");
		  fprintf(stderr, "%s:%u: vlog95 error: Could not find signal "
	                    "definition for port (%u) of module %s.\n",
		            ivl_scope_file(scope), ivl_scope_lineno(scope),
	                    idx + 1, ivl_scope_basename(scope));
		  vlog_errors += 1;
	    }
      }
      if (count) fprintf(vlog_out, "\n");
}

static void emit_module_call_expr(ivl_scope_t scope, unsigned idx)
{
      unsigned word;
      ivl_nexus_t nex = ivl_scope_mod_port(scope, idx);
      ivl_signal_t port = get_port_from_nexus(scope, nex, &word);
	/* For an input port we need to emit the driving expression. */
      if (ivl_signal_port(port) == IVL_SIP_INPUT) {
	    emit_nexus_port_driver_as_ca(ivl_scope_parent(scope),
	                                 ivl_signal_nex(port, word));
	/* For an output we need to emit the signal the output is driving. */
      } else {
	    emit_nexus_as_ca(ivl_scope_parent(scope),
	                     ivl_signal_nex(port, word), 0, 0);
      }
}

static void emit_module_call_expressions(ivl_scope_t scope)
{
      unsigned idx, count = ivl_scope_ports(scope);
      if (count == 0) return;
      emit_module_call_expr(scope, 0);
      for (idx = 1; idx < count; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_module_call_expr(scope, idx);
      }
}

static void emit_task_func_port_defs(ivl_scope_t scope)
{
      unsigned idx, count = ivl_scope_ports(scope);
      unsigned start = ivl_scope_type(scope) == IVL_SCT_FUNCTION;
      for (idx = start; idx < count; idx += 1) {
	    ivl_signal_t port = ivl_scope_port(scope, idx);
	    emit_port(port);
      }
	/* If the start and count are both 1 then this is a SystemVerilog
	 * function that does not have an argument so add a dummy one. */
      if ((start == 1) && (count == 1)) {
	    fprintf(vlog_out, "%*cinput _vlog95_dummy;", indent, ' ');
	    if (emit_file_line) fprintf(vlog_out, " /* no file/line */");
	    fprintf(vlog_out, "\n");
      }
      if (count) fprintf(vlog_out, "\n");
}

/*
 * Recursively look for the named block in the given statement.
 */
static int has_named_block(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count;
      int rtn = 0;
      if (! stmt) return 0;
      switch (ivl_statement_type(stmt)) {
	  /* Block or fork items can contain a named block. */
	case IVL_ST_BLOCK:
	case IVL_ST_FORK:
	case IVL_ST_FORK_JOIN_ANY:
	case IVL_ST_FORK_JOIN_NONE:
	    if (ivl_stmt_block_scope(stmt) == scope) return 1;
	    count = ivl_stmt_block_count(stmt);
	    for (idx = 0; (idx < count) && ! rtn ; idx += 1) {
		  rtn |= has_named_block(scope,
		                         ivl_stmt_block_stmt(stmt, idx));
	    }
	    break;
	  /* Case items can contain a named block. */
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	case IVL_ST_CASEX:
	case IVL_ST_CASEZ:
	    count = ivl_stmt_case_count(stmt);
	    for (idx = 0; (idx < count) && ! rtn; idx += 1) {
		  rtn |= has_named_block(scope,
		                         ivl_stmt_case_stmt(stmt, idx));
	    }
	    break;
	  /* Either the true or false clause may have a named block. */
	case IVL_ST_CONDIT:
	    rtn = has_named_block(scope, ivl_stmt_cond_true(stmt));
	    if (! rtn) {
		  rtn = has_named_block(scope, ivl_stmt_cond_false(stmt));
	    }
	    break;
	  /* The looping statements may have a named block. */
	case IVL_ST_DO_WHILE:
	case IVL_ST_FOREVER:
	case IVL_ST_REPEAT:
	case IVL_ST_WHILE:
	  /* The delay and wait statements may have a named block. */
	case IVL_ST_DELAY:
	case IVL_ST_DELAYX:
	case IVL_ST_WAIT:
	    rtn = has_named_block(scope, ivl_stmt_sub_stmt(stmt));
	    break;
	default: /* The rest cannot have a named block. */ ;
      }
      return rtn;
}

/*
 * Look at all the processes to see if we can find one with the expected
 * scope. If we don't find one then we can assume the block only has
 * variable definitions and needs to be emitted here in the scope code.
 */
static int no_stmts_in_process(ivl_process_t proc, ivl_scope_t scope)
{
      return has_named_block(scope, ivl_process_stmt(proc));
}

/*
 * If a named block has no statements then we may need to emit it here if
 * there are variable definitions in the scope. We translate all this to
 * an initial and named begin since that is enough to hold the variables.
 */
static void emit_named_block_scope(ivl_scope_t scope)
{
      unsigned idx, count = ivl_scope_events(scope);
      unsigned named_ev = 0;

	/* If there are no parameters, signals or named events then skip
	 * this block. */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_event_t event = ivl_scope_event(scope, idx);
	      /* If this event has any type of edge sensitivity then it is
	       * not a named event. */
	    if (ivl_event_nany(event)) continue;
	    if (ivl_event_npos(event)) continue;
	    if (ivl_event_nneg(event)) continue;
	    named_ev = 1;
	    break;
      }
      if ((ivl_scope_params(scope) == 0) && (ivl_scope_sigs(scope) == 0) &&
	  (named_ev == 0)) return;
	/* Currently we only need to emit a named block for the variables
	 * if the parent scope is a module. This gets much more complicated
	 * if this is not true. */
      if (ivl_scope_type(ivl_scope_parent(scope)) != IVL_SCT_MODULE) return;
	/* Scan all the processes looking for one that matches this scope.
	 * If a match is found then this named block was already emitted by
	 * the process code. */
      if (ivl_design_process(design, (ivl_process_f)no_stmts_in_process,
                             scope)) return;
	/* A match was not found so emit the named block here to get the
	 * variable definitions. */
      fprintf(vlog_out, "\n%*cinitial begin: ", indent, ' ');
      emit_id(ivl_scope_tname(scope));
      emit_scope_file_line(scope);
      fprintf(vlog_out, "\n");
      indent += indent_incr;
      emit_scope_variables(scope);
      indent -= indent_incr;
      fprintf(vlog_out, "%*cend  /* ", indent, ' ');
      emit_id(ivl_scope_tname(scope));
      fprintf(vlog_out, " */\n");
}

/*
 * In SystemVerilog a task, function, or block can have a process to
 * initialize variables. SystemVerilog requires this to be before the
 * initial/always blocks are processed, but there's no way to express
 * this in Verilog-95.
 */
static int find_tfb_process(ivl_process_t proc, ivl_scope_t scope)
{
      if (scope == ivl_process_scope(proc)) {
	    ivl_scope_t mod_scope = scope;
	      /* A task or function or named block can only have initial
	       * processes that are used to set local variables. */
	    assert(ivl_process_type(proc) == IVL_PR_INITIAL);
	      /* Find the module scope for this task/function. */
	    while (ivl_scope_type(mod_scope) != IVL_SCT_MODULE &&
		   ivl_scope_type(mod_scope) != IVL_SCT_PACKAGE) {
		  mod_scope = ivl_scope_parent(mod_scope);
		  assert(mod_scope);
	    }
	      /* Emit the process in the module scope since that is where
	       * this all started. */
	    emit_process(mod_scope, proc);
      }
      return 0;
}

/*
 * Emit any initial blocks for the tasks/functions/named blocks in a module.
 */
static int emit_tfb_process(ivl_scope_t scope, ivl_scope_t parent)
{
      ivl_scope_type_t sc_type = ivl_scope_type(scope);
      (void)parent;  /* Parameter is not used. */
      if ((sc_type == IVL_SCT_FUNCTION) || (sc_type == IVL_SCT_TASK) ||
          (sc_type == IVL_SCT_BEGIN) || (sc_type == IVL_SCT_FORK)) {
	/* Output the initial/always blocks for this module. */
	    ivl_design_process(design, (ivl_process_f)find_tfb_process, scope);
      }
      return 0;
}

static void emit_path_delay(ivl_scope_t scope, ivl_delaypath_t dpath)
{
      unsigned idx, count = 6;
      uint64_t pdlys [12];
      pdlys[0] = ivl_path_delay(dpath, IVL_PE_01);
      pdlys[1] = ivl_path_delay(dpath, IVL_PE_10);
      pdlys[2] = ivl_path_delay(dpath, IVL_PE_0z);
      pdlys[3] = ivl_path_delay(dpath, IVL_PE_z1);
      pdlys[4] = ivl_path_delay(dpath, IVL_PE_1z);
      pdlys[5] = ivl_path_delay(dpath, IVL_PE_z0);
      pdlys[6] = ivl_path_delay(dpath, IVL_PE_0x);
      pdlys[7] = ivl_path_delay(dpath, IVL_PE_x1);
      pdlys[8] = ivl_path_delay(dpath, IVL_PE_1x);
      pdlys[9] = ivl_path_delay(dpath, IVL_PE_x0);
      pdlys[10] = ivl_path_delay(dpath, IVL_PE_xz);
      pdlys[11] = ivl_path_delay(dpath, IVL_PE_zx);
	/* If the first six pdlys match then this may be a 1 delay form. */
      if ((pdlys[0] == pdlys[1]) &&
          (pdlys[0] == pdlys[2]) &&
          (pdlys[0] == pdlys[3]) &&
          (pdlys[0] == pdlys[4]) &&
          (pdlys[0] == pdlys[5])) count = 1;
	/* Check to see if only a rise and fall value are given for the first
	 * six pdlys. */
      else if ((pdlys[0] == pdlys[2]) &&
               (pdlys[0] == pdlys[3]) &&
               (pdlys[1] == pdlys[4]) &&
               (pdlys[1] == pdlys[5])) count = 2;
	/* Check to see if a rise, fall and high-Z value are given for the
	 * first six pdlys. */
      else if ((pdlys[0] == pdlys[3]) &&
               (pdlys[1] == pdlys[5]) &&
               (pdlys[2] == pdlys[4])) count = 3;
	/* Now check to see if the 'bx related pdlys match the reduced
	 * delay form. If not then this is a twelve delay value. */
      if ((pdlys[6]  != ((pdlys[0] < pdlys[2]) ?  pdlys[0] : pdlys[2])) ||
          (pdlys[8]  != ((pdlys[1] < pdlys[4]) ?  pdlys[1] : pdlys[4])) ||
          (pdlys[11] != ((pdlys[3] < pdlys[5]) ?  pdlys[3] : pdlys[5])) ||
          (pdlys[7]  != ((pdlys[0] > pdlys[3]) ?  pdlys[0] : pdlys[3])) ||
          (pdlys[9]  != ((pdlys[1] > pdlys[5]) ?  pdlys[1] : pdlys[5])) ||
          (pdlys[10] != ((pdlys[2] > pdlys[4]) ?  pdlys[2] : pdlys[4]))) {
	    count = 12;
      }
      emit_scaled_delay(scope, pdlys[0]);
      for(idx = 1; idx < count; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_scaled_delay(scope, pdlys[idx]);
      }
}

static void emit_specify_paths(ivl_scope_t scope, ivl_signal_t sig)
{
      unsigned idx, count = ivl_signal_npath(sig);
      for(idx = 0; idx < count; idx += 1) {
	    ivl_delaypath_t dpath = ivl_signal_path(sig, idx);
	    ivl_nexus_t cond = ivl_path_condit(dpath);
	    ivl_nexus_t source = ivl_path_source(dpath);
	    unsigned has_edge = 0;
	    fprintf(vlog_out, "%*c", indent, ' ');
	    if (cond) {
		  fprintf(vlog_out, "if (");
		  emit_nexus_as_ca(scope, cond, 0, 0);
		  fprintf(vlog_out, ") ");
	    } else if (ivl_path_is_condit(dpath)) {
		  fprintf(vlog_out, "ifnone ");
	    }
	    fprintf(vlog_out, "(");
	    if (ivl_path_source_posedge(dpath)) {
		  fprintf(vlog_out, "posedge ");
		  has_edge = 1;
	    }
	    if (ivl_path_source_negedge(dpath)) {
		  fprintf(vlog_out, "negedge ");
		  has_edge = 1;
	    }
	    emit_nexus_as_ca(scope, source, 0, 0);
	    if (ivl_path_is_parallel(dpath)) {
		  fprintf(vlog_out, " =>");
	    } else {
		  fprintf(vlog_out, " *>");
	    }
	      /* The compiler does not keep the source expression for an edge
	       * sensitive path so add a constant to get the syntax right. */
	    if (has_edge) {
		  fprintf(vlog_out, "(%s : 1'bx /* Missing */)",
		                    ivl_signal_basename(sig));
	    } else {
		  fprintf(vlog_out, "%s", ivl_signal_basename(sig));
	    }
	    fprintf(vlog_out, ") = (");
	    emit_path_delay(scope, dpath);
	    fprintf(vlog_out, ");\n");
      }
}

/*
 * The path delay information from the specify block is attached to the
 * output ports.
 */
static void emit_specify(ivl_scope_t scope)
{
      unsigned word, idx, count = ivl_scope_ports(scope);
      unsigned need_specify = 0;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_nexus_t nex = ivl_scope_mod_port(scope, idx);
	    ivl_signal_t port = get_port_from_nexus(scope, nex, &word);
// HERE: Do we need to use word? See emit_module_port_def().
	    assert(port);
	    if (ivl_signal_npath(port)) {
		  if (! need_specify) {
			fprintf(vlog_out, "\n%*cspecify\n", indent, ' ');
			need_specify = 1;
			indent += indent_incr;
		  }
		  emit_specify_paths(scope, port);
	    }
      }
      if (need_specify) {
	    indent -= indent_incr;
	    fprintf(vlog_out, "%*cendspecify\n", indent, ' ');
      }
}

/*
 * Look for a disable in the statement (function body) for this scope.
 */
static unsigned has_func_disable(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count, rtn = 0;
	/* If there is a statement then look to see if it is or has a
	 * disable for this function scope. */
      if (! stmt) return 0;
      assert(ivl_scope_type(scope) == IVL_SCT_FUNCTION);
      switch (ivl_statement_type(stmt)) {
	  /* These are not allowed in a function. */
	case IVL_ST_ASSIGN_NB:
	case IVL_ST_DELAY:
	case IVL_ST_DELAYX:
	case IVL_ST_FORK:
	case IVL_ST_FORK_JOIN_ANY:
	case IVL_ST_FORK_JOIN_NONE:
	case IVL_ST_WAIT:
	    assert(0);
	    break;
	  /* These are allowed in a function and cannot have a disable. */
	case IVL_ST_NOOP:
	case IVL_ST_ALLOC:
	case IVL_ST_ASSIGN:
        case IVL_ST_BREAK:
	case IVL_ST_CASSIGN:
        case IVL_ST_CONTINUE:
	case IVL_ST_DEASSIGN:
	case IVL_ST_FORCE:
	case IVL_ST_FREE:
	case IVL_ST_RELEASE:
	case IVL_ST_STASK:
	case IVL_ST_UTASK: // this will be generated for a SV void function
	case IVL_ST_TRIGGER:
	case IVL_ST_NB_TRIGGER:
	    break;
	  /* Look for a disable in each block statement. */
	case IVL_ST_BLOCK:
	    count = ivl_stmt_block_count(stmt);
	    for (idx = 0; (idx < count) && ! rtn; idx += 1) {
		  rtn |= has_func_disable(scope,
		                          ivl_stmt_block_stmt(stmt, idx));
	    }
	    break;
	  /* Look for a disable in each case branch. */
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	case IVL_ST_CASEX:
	case IVL_ST_CASEZ:
	    count = ivl_stmt_case_count(stmt);
	    for (idx = 0; (idx < count) && ! rtn; idx += 1) {
		  rtn |= has_func_disable(scope,
		                          ivl_stmt_case_stmt(stmt, idx));
	    }
	    break;
	  /* Either the true or false clause may have a disable. */
	case IVL_ST_CONDIT:
	    rtn = has_func_disable(scope, ivl_stmt_cond_true(stmt));
	    if (! rtn) {
		  rtn = has_func_disable(scope, ivl_stmt_cond_false(stmt));
	    }
	    break;
	  /* These have a single sub-statement so look for a disable there. */
	case IVL_ST_DO_WHILE:
	case IVL_ST_FOREVER:
        case IVL_ST_FORLOOP:
	case IVL_ST_REPEAT:
	case IVL_ST_WHILE:
	    rtn = has_func_disable(scope, ivl_stmt_sub_stmt(stmt));
	    break;
	  /* The function has a disable if the disable scope matches the
	   * function scope. */
	case IVL_ST_DISABLE:
	    rtn = scope == ivl_stmt_call(stmt);
	    break;
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown statement type (%d) "
	                    "in function disable check.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt),
	                    (int)ivl_statement_type(stmt));

	    vlog_errors += 1;
	    break;
      }
      return rtn;
}

/*
 * This is the block name used when a SystemVerilog return is used in a
 * function and the body does not already have an enclosing named block.
 * This is needed since the actual function cannot be disabled.
 */
static char *get_func_return_name(ivl_scope_t scope)
{
      const char *name_func = ivl_scope_basename(scope);
      const char *name_head = "_ivl_";
      const char *name_tail = "_return";
      char *name_return;
      name_return = (char *)malloc(strlen(name_head) +
                                   strlen(name_func) +
                                   strlen(name_tail) + 1);
      name_return[0] = 0;
      (void) strcpy(name_return, name_head);
      (void) strcat(name_return, name_func);
      (void) strcat(name_return, name_tail);
      return name_return;
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

void free_emitted_scope_list(void)
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

int emit_scope(ivl_scope_t scope, const ivl_scope_t parent)
{
      char *package_name = 0;
      ivl_scope_type_t sc_type = ivl_scope_type(scope);
      unsigned is_auto = ivl_scope_is_auto(scope);
      unsigned idx;

	/* Output the scope definition. */
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(!is_auto);
	      /* This is an instantiation. */
	    if (parent) {
		  assert(indent != 0);
		    /* If the module has parameters then it may not be unique
		     * so we create a mangled name version instead. */
		  fprintf(vlog_out, "\n%*c", indent, ' ');
		  emit_mangled_name(scope, !parent && !emitting_scopes);
		  fprintf(vlog_out, " ");
		  emit_id(ivl_scope_basename(scope));
		  fprintf(vlog_out, "(");
		  emit_module_call_expressions(scope);
		  fprintf(vlog_out, ");");
		  emit_scope_file_line(scope);
		  fprintf(vlog_out, "\n");
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
	    fprintf(vlog_out, "/* This module was originally defined in "
	                      "file %s at line %u. */\n",
	                      ivl_scope_def_file(scope),
	                      ivl_scope_def_lineno(scope));
	    fprintf(vlog_out, "module ");
	    emit_mangled_name(scope, !parent && !emitting_scopes);
	    emit_module_ports(scope);
	    break;
	case IVL_SCT_FUNCTION:
	      /* Root scope functions have already been emitted. */
	    if (! parent) return 0;
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*cfunction", indent, ' ');
	    if (ivl_scope_ports(scope) < 1) {
		  fprintf(stderr, "%s:%u: vlog95 error: Function (%s) has "
		                  "no return value.\n",
		                  ivl_scope_file(scope),
		                  ivl_scope_lineno(scope),
		                  ivl_scope_tname(scope));
		  vlog_errors += 1;
	    }
	      /* The function return information is the zero port. */
	    emit_func_return(ivl_scope_port(scope, 0));
	    fprintf(vlog_out, " ");
	    emit_id(ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic functions "
	                    "(%s) are not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), ivl_scope_tname(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_TASK:
	      /* Root scope tasks have already been emitted. */
	    if (! parent) return 0;
	    assert(indent != 0);
	    fprintf(vlog_out, "\n%*ctask ", indent, ' ');
	    emit_id(ivl_scope_tname(scope));
	    if (is_auto) {
		  fprintf(stderr, "%s:%u: vlog95 error: Automatic tasks "
	                    "(%s) are not supported.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), ivl_scope_tname(scope));
		  vlog_errors += 1;
	    }
	    break;
	case IVL_SCT_BEGIN:
	case IVL_SCT_FORK:
	    assert(indent != 0);
	    emit_named_block_scope(scope);
	    return 0; /* A named begin/fork is handled in line. */
	case IVL_SCT_GENERATE:
	    fprintf(stderr, "%s:%u: vlog95 sorry: generate scopes are not "
	                    "currently translated \"%s\".\n",
	                    ivl_scope_file(scope),
	                    ivl_scope_lineno(scope),
	                    ivl_scope_tname(scope));
	    vlog_errors += 1;
	    return 0;
	case IVL_SCT_PACKAGE:
	    assert(indent == 0);
	    assert(! parent);
	      /* Set the time scale for this scope. */
	    fprintf(vlog_out, "\n`timescale %s/%s\n",
	                      get_time_const(ivl_scope_time_units(scope)),
	                      get_time_const(ivl_scope_time_precision(scope)));
	      /* Emit a package as a module with a special name. */
	    fprintf(vlog_out, "/* This package (module) was originally "
	                      "defined in file %s at line %u. */\n",
	                      ivl_scope_def_file(scope),
	                      ivl_scope_def_lineno(scope));
	    fprintf(vlog_out, "module ");
	    package_name = get_package_name(scope);
	    emit_id(package_name);
	    break;
	case IVL_SCT_CLASS:
	    fprintf(stderr, "%s:%u: vlog95 sorry: class scopes are not "
	                    "currently translated \"%s\".\n",
	                    ivl_scope_file(scope),
	                    ivl_scope_lineno(scope),
	                    ivl_scope_tname(scope));
	    vlog_errors += 1;
	    return 0;
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: Unsupported scope type "
	                    "(%d) named: %s.\n", ivl_scope_file(scope),
	                    ivl_scope_lineno(scope), sc_type,
	                    ivl_scope_tname(scope));
	    vlog_errors += 1;
	    return 0;
      }
      fprintf(vlog_out, ";");
      emit_scope_file_line(scope);
      fprintf(vlog_out, "\n");
      indent += indent_incr;

	/* Output the scope ports for this scope. */
      if (sc_type == IVL_SCT_MODULE) {
	    emit_module_port_defs(scope);
      } else {
	    emit_task_func_port_defs(scope);
      }

      emit_scope_variables(scope);

      if (sc_type == IVL_SCT_MODULE) {
	    unsigned count = ivl_scope_lpms(scope);
	      /* Output the LPM devices. */
	    for (idx = 0; idx < count; idx += 1) {
		  emit_lpm(scope, ivl_scope_lpm(scope, idx));
	    }

	      /* Output any logic devices. */
	    count = ivl_scope_logs(scope);
	    for (idx = 0; idx < count; idx += 1) {
		  emit_logic(scope, ivl_scope_log(scope, idx));
	    }

	      /* Output any switch (logic) devices. */
	    count = ivl_scope_switches(scope);
	    for (idx = 0; idx < count; idx += 1) {
		  emit_tran(scope, ivl_scope_switch(scope, idx));
	    }
      }

      if (sc_type == IVL_SCT_MODULE || sc_type == IVL_SCT_PACKAGE) {
	      /* Output any initial blocks for tasks or functions or named
	       * blocks defined in this module. Used to initialize local
	       * variables. */
	    ivl_scope_children(scope, (ivl_scope_f*) emit_tfb_process, scope);

	      /* Output the initial/always blocks for this module. */
	    ivl_design_process(design, (ivl_process_f)find_process, scope);
      }

	/* Output the function body. */
      if (sc_type == IVL_SCT_FUNCTION) {
	    ivl_statement_t body = ivl_scope_def(scope);
	    assert(func_rtn_name == 0);
	      /* If the function disables itself then that is really a
	       * SystemVerilog return statement in disguise. A toplevel
	       * named begin is needed to make this work in standard Verilog
	       * so add one if it is needed. */
	    if (ivl_statement_type(body) == IVL_ST_BLOCK) {
		  ivl_scope_t blk_scope = ivl_stmt_block_scope(body);
		  if (blk_scope) {
			func_rtn_name = ivl_scope_basename(blk_scope);
			emit_stmt(scope, body);
			func_rtn_name = 0;
		  } else if (has_func_disable(scope, body)) {
			char *name_return = get_func_return_name(scope);
			unsigned count = ivl_stmt_block_count(body);
			fprintf(vlog_out, "%*cbegin: %s\n", indent, ' ',
			                  name_return);
			indent += indent_incr;
			func_rtn_name = name_return;
			for (idx = 0; idx < count; idx += 1) {
			      emit_stmt(scope, ivl_stmt_block_stmt(body, idx));
			}
			func_rtn_name = 0;
			indent -= indent_incr;
			fprintf(vlog_out, "%*cend /* %s */\n", indent, ' ',
			                  name_return);
			free(name_return);
		  } else emit_stmt(scope, body);
	      /* A non-block statement may need a named block for a return. */
	    } else if (has_func_disable(scope, body)) {
		  char *name_return = get_func_return_name(scope);
		  fprintf(vlog_out, "%*cbegin: %s\n", indent, ' ',
		                     name_return);
		  indent += indent_incr;
		  func_rtn_name = name_return;
		  emit_stmt(scope, body);
		  func_rtn_name = 0;
		  indent -= indent_incr;
		  fprintf(vlog_out, "%*cend /* %s */\n", indent, ' ',
		                    name_return);
		  free(name_return);
	    } else emit_stmt(scope, body);
      }
	/* Output the task body. */
      if (sc_type == IVL_SCT_TASK) emit_stmt(scope, ivl_scope_def(scope));

	/* Print any sub-scopes. */
      ivl_scope_children(scope, (ivl_scope_f*) emit_scope, scope);

	/* And finally print a specify block when needed. */
      if (sc_type == IVL_SCT_MODULE) emit_specify(scope);

	/* Output the scope ending. */
      assert(indent >= indent_incr);
      indent -= indent_incr;
      switch (sc_type) {
	case IVL_SCT_MODULE:
	    assert(indent == 0);
	    fprintf(vlog_out, "endmodule  /* ");
	    emit_mangled_name(scope, !parent && !emitting_scopes);
	    fprintf(vlog_out, " */\n");
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
			if (!scope_is_unique(scope_to_emit)) continue;
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
	case IVL_SCT_PACKAGE:
	    fprintf(vlog_out, "endmodule  /* ");
	    emit_id(package_name);
	    free(package_name);
	    fprintf(vlog_out, " */\n");
	    break;
	default:
	    assert(0);
	    break;
      }
      return 0;
}
