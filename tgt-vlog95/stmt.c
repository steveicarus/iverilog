/*
 * Copyright (C) 2011 Cary R. (cygcary@yahoo.com)
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

# include <stdlib.h>
# include <string.h>
# include "config.h"
# include "vlog95_priv.h"

static unsigned single_indent = 0;

static unsigned get_indent()
{
      if (single_indent) {
	    single_indent = 0;
	    return single_indent;
      }
      return indent;
}

static void emit_stmt_block_body(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count = ivl_stmt_block_count(stmt);
      indent += indent_incr;
      for (idx = 0; idx < count; idx += 1) {
	    emit_stmt(scope, ivl_stmt_block_stmt(stmt, idx));
      }
      assert(indent >= indent_incr);
      indent -= indent_incr;
}

static void emit_stmt_inter_delay(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_expr_t delay = ivl_stmt_delay_expr(stmt);
      unsigned nevents = ivl_stmt_nevent(stmt);
      if (nevents) {
	    ivl_expr_t count = ivl_stmt_cond_expr(stmt);
	    if (count) {
		  if (ivl_expr_type(count) == IVL_EX_ULONG) {
			unsigned long repeat = ivl_expr_uvalue(count);
			if (repeat != 1) {
			      fprintf(vlog_out, "repeat(%lu) ", repeat);
			}
		  } else {
			fprintf(vlog_out, "repeat(");
			emit_expr(scope, count, 0);
			fprintf(vlog_out, ") ");
		  }
	    }
	    assert(delay == 0);
	    fprintf(vlog_out, "@(");
	    emit_event(scope, stmt);
	    fprintf(vlog_out, ") ");
      }
      if (delay) {
	    assert(nevents == 0);
	    fprintf(vlog_out, "#");
	    emit_expr(scope, delay, 0);
	    fprintf(vlog_out, " ");
      }
}

static void emit_stmt_lval_piece(ivl_scope_t scope, ivl_lval_t lval)
{
      ivl_expr_t expr;
      ivl_signal_t sig = ivl_lval_sig(lval);
      unsigned width = ivl_lval_width(lval);
      int msb, lsb;
      assert(width > 0);
      emit_scope_module_path(scope, ivl_signal_scope(sig));
      fprintf(vlog_out, "%s", ivl_signal_basename(sig));
	/* Check to see if we have an array word access. */
      expr = ivl_lval_idx(lval);
      if (expr) {
	    assert(ivl_signal_dimensions(sig));
	    fprintf(vlog_out, "[");
	      /* For an array the LSB/MSB order is not important. They are
	       * always accessed from base counting up. */
	    lsb = ivl_signal_array_base(sig);
	    msb = lsb + ivl_signal_array_count(sig) - 1;
	    emit_scaled_expr(scope, expr, msb, lsb);
	    fprintf(vlog_out, "]");
      }

	/* If there are no selects then just return. */
      if (width == ivl_signal_width(sig)) return;

	/* We have some kind of select. */
      lsb = ivl_signal_lsb(sig);
      msb = ivl_signal_msb(sig);
      if (width == 1) {
	    fprintf(vlog_out, "[");
	    emit_scaled_expr(scope, ivl_lval_part_off(lval), msb, lsb);
	    fprintf(vlog_out, "]");
      } else {
	    emit_scaled_range(scope, ivl_lval_part_off(lval), width, msb, lsb);
      }
}

static unsigned emit_stmt_lval(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned count = ivl_stmt_lvals(stmt);
      unsigned wid = 0;
      if (count > 1) {
	    unsigned idx;
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid += ivl_lval_width(lval);
	    fprintf(vlog_out, "{");
	    emit_stmt_lval_piece(scope, lval);
	    for (idx = 1; idx < count; idx += 1) {
		  fprintf(vlog_out, ", ");
		  lval = ivl_stmt_lval(stmt, idx);
		  wid += ivl_lval_width(lval);
		  emit_stmt_lval_piece(scope, lval);
	    }
	    fprintf(vlog_out, "}");
      } else {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid = ivl_lval_width(lval);
	    emit_stmt_lval_piece(scope, lval);
      }
      return wid;
}

/*
 * Icarus translated <var> = <delay> <value> into
 *   begin
 *    <tmp> = <value>;
 *    <delay> <var> = <tmp>;
 *   end
 * This routine looks for this pattern and turns it back into the
 * appropriate blocking assignment.
 */
static unsigned find_delayed_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      ivl_statement_t assign, delay, delayed_assign;
      ivl_lval_t lval;
      ivl_expr_t rval;
      ivl_signal_t lsig, rsig;

	/* We must have two block elements. */
      if (ivl_stmt_block_count(stmt) != 2) return 0;
	/* The first must be an assign. */
      assign = ivl_stmt_block_stmt(stmt, 0);
      if (ivl_statement_type(assign) != IVL_ST_ASSIGN) return 0;
	/* The second must be a delayx. */
      delay = ivl_stmt_block_stmt(stmt, 1);
      if (ivl_statement_type(delay) != IVL_ST_DELAYX) return 0;
	/* The statement for the delayx must be an assign. */
      delayed_assign = ivl_stmt_sub_stmt(delay);
      if (ivl_statement_type(delayed_assign) != IVL_ST_ASSIGN) return 0;
	/* The L-value must be a single signal. */
      if (ivl_stmt_lvals(assign) != 1) return 0;
      lval = ivl_stmt_lval(assign, 0);
	/* It must not have an array select. */
      if (ivl_lval_idx(lval)) return 0;
	/* It must not have a non-zero base. */
      if (ivl_lval_part_off(lval)) return 0;
      lsig = ivl_lval_sig(lval);
	/* It must not be part of the signal. */
      if (ivl_lval_width(lval) != ivl_signal_width(lsig)) return 0;
	/* The R-value must be a single signal. */
      rval = ivl_stmt_rval(delayed_assign);
      if (ivl_expr_type(rval) != IVL_EX_SIGNAL) return 0;
	/* It must not be an array word. */
      if (ivl_expr_oper1(rval)) return 0;
      rsig = ivl_expr_signal(rval);
	/* And finally the two signals must be the same. */
      if (lsig != rsig) return 0;

	/* The pattern matched so generate the appropriate code. */
      fprintf(vlog_out, "%*c", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, delayed_assign);
      fprintf(vlog_out, " = #");
      emit_scaled_delayx(scope, ivl_stmt_delay_expr(delay));
      fprintf(vlog_out, " ");
      emit_expr(scope, ivl_stmt_rval(assign), wid);
      fprintf(vlog_out, ";\n");

      return 1;
}

/*
 * Check to see if the statement L-value is a port in the given scope.
 * If it is return the zero based port number.
 */
static unsigned utask_in_port_idx(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, ports = ivl_scope_ports(scope);
      ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
      ivl_signal_t lsig = ivl_lval_sig(lval);
      const char *sig_name;
	/* The L-value must be a single signal. */
      if (ivl_stmt_lvals(stmt) != 1) return ports;
	/* It must not have an array select. */
      if (ivl_lval_idx(lval)) return ports;
	/* It must not have a non-zero base. */
      if (ivl_lval_part_off(lval)) return ports;
	/* It must not be part of the signal. */
      if (ivl_lval_width(lval) != ivl_signal_width(lsig)) return ports;
	/* It must have the same scope as the task. */
      if (scope != ivl_signal_scope(lsig)) return ports;
	/* It must be an input or inout port of the task. */
      sig_name = ivl_signal_basename(lsig);
      for (idx = 0; idx < ports; idx += 1) {
	    ivl_signal_t port = ivl_scope_port(scope, idx);
	    ivl_signal_port_t port_type = ivl_signal_port(port);
	    if ((port_type != IVL_SIP_INPUT) &&
	        (port_type != IVL_SIP_INOUT)) continue;
	    if (strcmp(sig_name, ivl_signal_basename(port)) == 0) break;
      }
      return idx;
}

/*
 * Check to see if the statement R-value is a port in the given scope.
 * If it is return the zero based port number.
 */
static unsigned utask_out_port_idx(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, ports = ivl_scope_ports(scope);
      ivl_expr_t rval = ivl_stmt_rval(stmt);
      ivl_signal_t rsig = 0;
      ivl_expr_type_t expr_type = ivl_expr_type(rval);
      const char *sig_name;
	/* We can have a simple signal. */
      if (expr_type == IVL_EX_SIGNAL) {
	    rsig = ivl_expr_signal(rval);
	/* Or a simple select of a simple signal. */
      } else if (expr_type == IVL_EX_SELECT) {
	    ivl_expr_t expr = ivl_expr_oper1(rval);
	      /* We must have a zero select base. */
	    if (ivl_expr_oper2(rval)) return ports;
	      /* We must be selecting a signal. */
	    if (ivl_expr_type(expr) != IVL_EX_SIGNAL) return ports;
	    rsig = ivl_expr_signal(expr);
      } else return ports;
	/* The R-value must have the same scope as the task. */
      if (scope != ivl_signal_scope(rsig)) return ports;
	/* It must not be an array element. */
      if (ivl_signal_dimensions(rsig)) return ports;
	/* It must be an output or inout port of the task. */
      sig_name = ivl_signal_basename(rsig);
      for (idx = 0; idx < ports; idx += 1) {
	    ivl_signal_t port = ivl_scope_port(scope, idx);
	    ivl_signal_port_t port_type = ivl_signal_port(port);
	    if ((port_type != IVL_SIP_OUTPUT) &&
	        (port_type != IVL_SIP_INOUT)) continue;
	    if (strcmp(sig_name, ivl_signal_basename(port)) == 0) break;
      }
      return idx;
}

/*
 * Structure to hold the port information as we extract it from the block.
 */
typedef struct port_expr_s {
      ivl_signal_port_t type;
      union {
	    ivl_statement_t lval;
	    ivl_expr_t rval;
      };
}  *port_expr_t;

/*
 * An input prints the R-value and an output or inout print the L-value.
 */
static void emit_port(ivl_scope_t scope, struct port_expr_s port_expr)
{
      if (port_expr.type == IVL_SIP_INPUT) {
	    emit_expr(scope, port_expr.rval, 0);
      } else {
	      /* This is a self-determined context so we don't care about
	       * the width of the L-value. */
	    (void) emit_stmt_lval(scope, port_expr.lval);
      }
}


/*
 * Icarus encodes a task call with arguments as:
 *   begin
 *     <input 1> = <arg>
 *     ...
 *     <input n> = <arg>
 *     <task_call>
 *     <arg> = <output 1>
 *     ...
 *     <arg> = <output n>
 *   end
 * This routine looks for that pattern and translates it into the
 * appropriate task call. It returns true (1) if it successfully
 * translated the block to a task call, otherwise it returns false
 * (0) to indicate the block needs to be emitted.
 */
static unsigned is_utask_call_with_args(ivl_scope_t scope,
                                        ivl_statement_t stmt)
{
      unsigned idx, ports, task_idx = 0;
      unsigned count = ivl_stmt_block_count(stmt);
      ivl_scope_t task_scope = 0;
      port_expr_t port_exprs;
	/* Check to see if the block is of the basic form first.  */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_statement_t tmp = ivl_stmt_block_stmt(stmt, idx);
	    if (ivl_statement_type(tmp) == IVL_ST_ASSIGN) continue;
	    if (ivl_statement_type(tmp) == IVL_ST_UTASK && !task_scope) {
		  task_idx = idx;
		  task_scope = ivl_stmt_call(tmp);
		  assert(ivl_scope_type(task_scope) == IVL_SCT_TASK);
		  continue;
	    }
	    return 0;
      }
	/* If there is no task call or it takes no argument then return. */
      if (!task_scope) return 0;
      ports = ivl_scope_ports(task_scope);
      if (ports == 0) return 0;

	/* Allocate space to save the port information and initialize it. */
      port_exprs = (port_expr_t) malloc(sizeof(struct port_expr_s)*ports);
      for (idx = 0; idx < ports; idx += 1) {
	    port_exprs[idx].type = IVL_SIP_NONE;
	    port_exprs[idx].rval = 0;
      }
	/* Now do a detailed check that the arguments are correct. */
      for (idx = 0; idx < task_idx; idx += 1) {
	    ivl_statement_t assign = ivl_stmt_block_stmt(stmt, idx);
	    unsigned port = utask_in_port_idx(task_scope, assign);
	    if (port == ports) {
		  free(port_exprs);
		  return 0;
	    }
	    port_exprs[port].type = IVL_SIP_INPUT;
	    port_exprs[port].rval = ivl_stmt_rval(assign);
      }
      for (idx = task_idx + 1; idx < count; idx += 1) {
	    ivl_statement_t assign = ivl_stmt_block_stmt(stmt, idx);
	    unsigned port = utask_out_port_idx(task_scope, assign);
	    if (port == ports) {
		  free(port_exprs);
		  return 0;
	    }
	    if (port_exprs[port].type == IVL_SIP_INPUT) {
		  port_exprs[port].type = IVL_SIP_INOUT;
// HERE: We probably should verify that the current R-value matches the
//       new L-value.
	    } else {
		  port_exprs[port].type = IVL_SIP_OUTPUT;
	    }
	    port_exprs[port].lval = assign;
      }

	/* Verify that all the ports were defined. */
      for (idx = 0; idx < ports; idx += 1) {
	    if (port_exprs[idx].type == IVL_SIP_NONE) {
		  free(port_exprs);
		  return 0;
	    }
      }

	/* Now that we have the arguments figured out, print the task call. */
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      emit_scope_path(scope, task_scope);
      fprintf(vlog_out, "(");
      emit_port(scope, port_exprs[0]);
      for (idx = 1; idx < ports; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_port(scope, port_exprs[idx]);
      }
      free(port_exprs);
      fprintf(vlog_out, ");\n");
      return 1;
}

static void emit_stmt_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*c", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, " = ");
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_assign_nb(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*c", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, " <= ");
      emit_stmt_inter_delay(scope, stmt);
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_block(ivl_scope_t scope, ivl_statement_t stmt)
{
      if (is_utask_call_with_args(scope, stmt)) return;
      fprintf(vlog_out, "%*cbegin\n", get_indent(), ' ');
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cend\n", get_indent(), ' ');
}

static void emit_stmt_block_named(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      fprintf(vlog_out, "%*cbegin: %s\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cend  /* %s */\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
}

static void emit_stmt_case(ivl_scope_t scope, ivl_statement_t stmt)
{
      char *name;
      unsigned idx, default_case, count = ivl_stmt_case_count(stmt);
      switch(ivl_statement_type(stmt)) {
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	    name = "case";
	    break;
	case IVL_ST_CASEX:
	    name = "casex";
	    break;
	case IVL_ST_CASEZ:
	    name = "casez";
	    break;
	default:
	    assert(0);
      }
      fprintf(vlog_out, "%*c%s (", get_indent(), ' ', name);
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0);
      fprintf(vlog_out, ")\n");
      indent += indent_incr;
      default_case = count;
      for (idx = 0; idx < count; idx += 1) {
	    ivl_expr_t expr = ivl_stmt_case_expr(stmt, idx);
	      /* This is the default case so emit it last. */
	    if (expr == 0) {
		  assert(default_case == count);
		  default_case = idx;
		  continue;
	    }
	    fprintf(vlog_out, "%*c", get_indent(), ' ');
	    emit_expr(scope, expr, 0);
	    fprintf(vlog_out, ":");
	    single_indent = 1;
	    emit_stmt(scope, ivl_stmt_case_stmt(stmt, idx));
      }
      if (default_case < count) {
	    fprintf(vlog_out, "%*cdefault:", get_indent(), ' ');
	    single_indent = 1;
	    emit_stmt(scope, ivl_stmt_case_stmt(stmt, default_case));
      }
      assert(indent >= indent_incr);
      indent -= indent_incr;
      fprintf(vlog_out, "%*cendcase\n", get_indent(), ' ');
}

static void emit_stmt_cassign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*cassign ", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, " = ");
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_condit(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cif (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0);
      fprintf(vlog_out, ")");
      if (ivl_stmt_cond_true(stmt)) {
	    single_indent = 1;
	    emit_stmt(scope, ivl_stmt_cond_true(stmt));
      } else {
	    fprintf(vlog_out, ";\n");
      }
      if (ivl_stmt_cond_false(stmt)) {
	    fprintf(vlog_out, "%*celse", get_indent(), ' ');
	    single_indent = 1;
	    emit_stmt(scope, ivl_stmt_cond_false(stmt));
      }
}

static void emit_stmt_deassign(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cdeassign ", get_indent(), ' ');
      (void) emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_delay(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c#", get_indent(), ' ');
      emit_scaled_delay(scope, ivl_stmt_delay_val(stmt));
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_delayx(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c#", get_indent(), ' ');
      emit_scaled_delayx(scope, ivl_stmt_delay_expr(stmt));
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_disable(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t disable_scope = ivl_stmt_call(stmt);
      fprintf(vlog_out, "%*cdisable ", get_indent(), ' ');
      emit_scope_path(scope, disable_scope);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_force(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*cforce ", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, " = ");
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_forever(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cforever", get_indent(), ' ');
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_fork(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cfork\n", get_indent(), ' ');
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cjoin\n", get_indent(), ' ');
}

static void emit_stmt_fork_named(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      fprintf(vlog_out, "%*cfork: %s\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cjoin  /* %s */\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
}

static void emit_stmt_release(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*crelease ", get_indent(), ' ');
      (void) emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_repeat(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*crepeat (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0);
      fprintf(vlog_out, ")");
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_stask(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned count = ivl_stmt_parm_count(stmt);
      fprintf(vlog_out, "%*c%s", get_indent(), ' ', ivl_stmt_name(stmt));
      if (count != 0) {
	    unsigned idx;
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
		  ivl_expr_t expr = ivl_stmt_parm(stmt, idx);
		  if (expr) emit_expr(scope, expr, 0);
		  fprintf(vlog_out, ", ");
	    }
	    emit_expr(scope, ivl_stmt_parm(stmt, count), 0);
	    fprintf(vlog_out, ")");
      }
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_trigger(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c-> ", get_indent(), ' ');
      assert(ivl_stmt_nevent(stmt) == 1);
      emit_event(scope, stmt);
      fprintf(vlog_out, ";\n");
}

/*
 * A user defined task call with arguments is generated as a block with
 * input assignments, a simple call and then output assignments. This is
 * handled by the is_utask_call_with_args() routine above.
 */
static void emit_stmt_utask(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t task_scope = ivl_stmt_call(stmt);
      assert(ivl_scope_type(task_scope) == IVL_SCT_TASK);
      assert(ivl_scope_ports(task_scope) == 0);
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      emit_scope_path(scope, task_scope);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_wait(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c@(", get_indent(), ' ');
      emit_event(scope, stmt);
      fprintf(vlog_out, ")");
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_while(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cwhile (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0);
      fprintf(vlog_out, ")");
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

void emit_stmt(ivl_scope_t scope, ivl_statement_t stmt)
{
      switch(ivl_statement_type(stmt)) {
	case IVL_ST_NOOP:
	    single_indent = 0;
	    fprintf(vlog_out, ";\n");
	    break;
	case IVL_ST_ALLOC:
	      /* This statement is only used with an automatic task so we
	       * can safely skip it. The automatic task definition will
	       * generate an appropriate error message.*/
	    break;
	case IVL_ST_ASSIGN:
	    emit_stmt_assign(scope, stmt);
	    break;
	case IVL_ST_ASSIGN_NB:
	    emit_stmt_assign_nb(scope, stmt);
	    break;
	case IVL_ST_BLOCK:
	    if (ivl_stmt_block_scope(stmt)) {
		  emit_stmt_block_named(scope, stmt);
	    } else {
		  if (find_delayed_assign(scope, stmt)) break;
		  emit_stmt_block(scope, stmt);
	    }
	    break;
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	case IVL_ST_CASEX:
	case IVL_ST_CASEZ:
	    emit_stmt_case(scope, stmt);
	    break;
	case IVL_ST_CASSIGN:
	    emit_stmt_cassign(scope, stmt);
	    break;
	case IVL_ST_CONDIT:
	    emit_stmt_condit(scope, stmt);
	    break;
	case IVL_ST_DEASSIGN:
	    emit_stmt_deassign(scope, stmt);
	    break;
	case IVL_ST_DELAY:
	    emit_stmt_delay(scope, stmt);
	    break;
	case IVL_ST_DELAYX:
	    emit_stmt_delayx(scope, stmt);
	    break;
	case IVL_ST_DISABLE:
	    emit_stmt_disable(scope, stmt);
	    break;
	case IVL_ST_FORCE:
	    emit_stmt_force(scope, stmt);
	    break;
	case IVL_ST_FOREVER:
	    emit_stmt_forever(scope, stmt);
	    break;
	case IVL_ST_FORK:
	    if (ivl_stmt_block_scope(stmt)) {
		  emit_stmt_fork_named(scope, stmt);
	    } else {
		  emit_stmt_fork(scope, stmt);
	    }
	    break;
	case IVL_ST_FREE:
	      /* This statement is only used with an automatic task so we
	       * can safely skip it. The automatic task definition will
	       * generate an appropriate error message.*/
	    break;
	case IVL_ST_RELEASE:
	    emit_stmt_release(scope, stmt);
	    break;
	case IVL_ST_REPEAT:
	    emit_stmt_repeat(scope, stmt);
	    break;
	case IVL_ST_STASK:
	    emit_stmt_stask(scope, stmt);
	    break;
	case IVL_ST_TRIGGER:
	    emit_stmt_trigger(scope, stmt);
	    break;
	case IVL_ST_UTASK:
	    emit_stmt_utask(scope, stmt);
	    break;
	case IVL_ST_WAIT:
	    emit_stmt_wait(scope, stmt);
	    break;
	case IVL_ST_WHILE:
	    emit_stmt_while(scope, stmt);
	    break;
	default:
	    fprintf(vlog_out, "%*c<unknown>;\n", get_indent(), ' ');
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown statement "
	                    "type (%d).\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt),
	                    (int)ivl_statement_type(stmt));
	    vlog_errors += 1;
	    break;
      }
}

void emit_process(ivl_scope_t scope, ivl_process_t proc)
{
      fprintf(vlog_out, "\n%*c", get_indent(), ' ');
      switch (ivl_process_type(proc)) {
        case IVL_PR_INITIAL:
            fprintf(vlog_out, "initial");
            break;
        case IVL_PR_ALWAYS:
            fprintf(vlog_out, "always");
            break;
        default:
            fprintf(vlog_out, "<unknown>");
            fprintf(stderr, "%s:%u: vlog95 error: Unknown process type (%d).\n",
                            ivl_process_file(proc), ivl_process_lineno(proc),
                            (int)ivl_process_type(proc));
            vlog_errors+= 1;
            break;
      }
      single_indent = 1;
      emit_stmt(scope, ivl_process_stmt(proc));
}
