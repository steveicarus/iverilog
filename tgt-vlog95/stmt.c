/*
 * Copyright (C) 2011-2022 Cary R. (cygcary@yahoo.com)
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

static unsigned get_indent(void)
{
      if (single_indent) {
	    single_indent = 0;
	    return single_indent;
      }
      return indent;
}

static void emit_stmt_file_line(ivl_statement_t stmt)
{
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_stmt_file(stmt),
	                      ivl_stmt_lineno(stmt));
      }
}

static void emit_stmt_block_body(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count = ivl_stmt_block_count(stmt);
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      indent += indent_incr;
      if (my_scope) emit_scope_variables(my_scope);
      else my_scope = scope;
      for (idx = 0; idx < count; idx += 1) {
	    emit_stmt(my_scope, ivl_stmt_block_stmt(stmt, idx));
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
			emit_expr(scope, count, 0, 0, 0, 0);
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
	    fprintf(vlog_out, "#(");
	    emit_scaled_delayx(scope, delay, 1);
	    fprintf(vlog_out, ") ");
      }
}

static void emit_stmt_lval_name(ivl_scope_t scope, ivl_lval_t lval,
                                ivl_signal_t sig)
{
      ivl_expr_t array_idx = ivl_lval_idx(lval);
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      if (array_idx) {
	    int msb, lsb;
	    ivl_type_t net_type = ivl_signal_net_type(sig);
	    fprintf(vlog_out, "[");
	    if (ivl_type_base(net_type) == IVL_VT_QUEUE) {
		  lsb = 0;
		  msb = 1;
	    } else {
		  assert(ivl_signal_dimensions(sig));
		    /* For an array the LSB/MSB order is not important.
		     * They are always accessed from base counting up. */
		  lsb = ivl_signal_array_base(sig);
		  msb = lsb + ivl_signal_array_count(sig) - 1;
	    }
	    emit_scaled_expr(scope, array_idx, msb, lsb);
	    fprintf(vlog_out, "]");
      }
}

static void emit_stmt_lval_packed(ivl_scope_t scope, ivl_lval_t lval,
                                  ivl_signal_t sig, ivl_expr_t sel_expr,
                                  unsigned wid)
{
      unsigned idx;
      assert(wid > 0);
      fprintf(vlog_out, "{");
      for (idx = wid - 1; idx > 0; idx -= 1) {
	    emit_stmt_lval_name(scope, lval, sig);
	    fprintf(vlog_out, "[");
	    emit_expr(scope, sel_expr, 0, 0, 0, 1);
	    fprintf(vlog_out, " + %u], ", idx);
      }
      emit_stmt_lval_name(scope, lval, sig);
      fprintf(vlog_out, "[");
      emit_expr(scope, sel_expr, 0, 0, 0, 1);
      fprintf(vlog_out, "]}");
}

static void emit_stmt_lval_ips(ivl_scope_t scope, ivl_lval_t lval,
                               ivl_signal_t sig, ivl_expr_t sel_expr,
                               ivl_select_type_t sel_type,
                               unsigned wid, int msb, int lsb)
{
      unsigned idx;
      assert(wid > 0);
      fprintf(vlog_out, "{");
      if (msb >= lsb) {
	    if (sel_type == IVL_SEL_IDX_DOWN) {
		  lsb += wid - 1;
		  msb += wid - 1;
		  emit_stmt_lval_name(scope, lval, sig);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_stmt_lval_name(scope, lval, sig);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert(sel_type == IVL_SEL_IDX_UP);
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_stmt_lval_name(scope, lval, sig);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u], ", idx);
		  }
		  emit_stmt_lval_name(scope, lval, sig);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      } else {
	    if (sel_type == IVL_SEL_IDX_UP) {
		  lsb -= wid - 1;
		  msb -= wid - 1;
		  emit_stmt_lval_name(scope, lval, sig);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_stmt_lval_name(scope, lval, sig);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert(sel_type == IVL_SEL_IDX_DOWN);
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_stmt_lval_name(scope, lval, sig);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u], ", idx);
		  }
		  emit_stmt_lval_name(scope, lval, sig);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      }
}

/*
 * Dynamic arrays are not supported in vlog95, but this assignment can be
 * translated correctly.
 */
static void emit_stmt_lval_darray(ivl_scope_t scope, ivl_lval_t lval,
                                  ivl_signal_t sig)
{
      ivl_expr_t idx = ivl_lval_idx(lval);
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      if (idx) {
	    fprintf(vlog_out, "[");
	    emit_expr(scope, idx, 0, 0, 0, 1);
	    fprintf(vlog_out, "]");
      }
}

/*
 * Class or class properties are not supported in vlog95, but this assignment
 * can be translated correctly.
 */
static ivl_type_t emit_stmt_lval_class(ivl_scope_t scope, ivl_lval_t lval)
{
      ivl_lval_t nest = ivl_lval_nest(lval);
      ivl_signal_t sig = ivl_lval_sig(lval);
      ivl_type_t type;
      int idx = ivl_lval_property_idx(lval);

      if (nest) {
	    type = emit_stmt_lval_class(scope, nest);
	    assert(type);
      } else {
	    assert(sig);
	    emit_scope_call_path(scope, ivl_signal_scope(sig));
	    emit_id(ivl_signal_basename(sig));
	    type = ivl_signal_net_type(sig);
      }

      if (idx >= 0) {
	    fprintf(vlog_out, ".%s", ivl_type_prop_name(type, idx));
	    return ivl_type_prop_type(type, idx);
      } else return 0;
}

static void emit_stmt_lval_piece(ivl_scope_t scope, ivl_lval_t lval)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      ivl_expr_t sel_expr;
      ivl_select_type_t sel_type;
      unsigned width = ivl_lval_width(lval);
      int msb, lsb;
      assert(width > 0);

	/* A class supports a nested L-value so it may not have a signal
	 * at this level. */
      if (! sig) {
	    (void) emit_stmt_lval_class(scope, lval);
	    return;
      }

      switch (ivl_signal_data_type(sig)) {
	case IVL_VT_DARRAY:
	    emit_stmt_lval_darray(scope, lval, sig);
	    return;
	case IVL_VT_CLASS:
	    (void) emit_stmt_lval_class(scope, lval);
	    return;
	default:
	    break;
      }

	/* If there are no selects then just print the name. */
      sel_expr = ivl_lval_part_off(lval);
      if (! sel_expr && ((width == ivl_signal_width(sig)) ||
                         (ivl_signal_data_type(sig) == IVL_VT_QUEUE))) {
	    emit_stmt_lval_name(scope, lval, sig);
	    return;
      }

	/* We have some kind of select. */
      get_sig_msb_lsb(sig, &msb, &lsb);
      sel_type = ivl_lval_sel_type(lval);
      assert(sel_expr);
	/* A bit select. */
      if (width == 1) {
	    emit_stmt_lval_name(scope, lval, sig);
	    fprintf(vlog_out, "[");
	    emit_scaled_expr(scope, sel_expr, msb, lsb);
	    fprintf(vlog_out, "]");
      } else if (ivl_expr_type(sel_expr) == IVL_EX_NUMBER) {
	      /* A constant part select. */
	    emit_stmt_lval_name(scope, lval, sig);
	    emit_scaled_range(scope, sel_expr, width, msb, lsb);
      } else if (sel_type == IVL_SEL_OTHER) {
	    assert(lsb == 0);
	    assert(msb >= 0);
	    emit_stmt_lval_packed(scope, lval, sig, sel_expr, width);
      } else {
	      /* An indexed part select. */
	    emit_stmt_lval_ips(scope, lval, sig, sel_expr, sel_type,
	                       width, msb, lsb);
      }
}

static unsigned emit_stmt_lval(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned count = ivl_stmt_lvals(stmt);
      unsigned wid = 0;
      if (count > 1) {
	    unsigned idx;
	    ivl_lval_t lval;
	    fprintf(vlog_out, "{");
	    for (idx = count - 1; idx > 0; idx -= 1) {
		  lval = ivl_stmt_lval(stmt, idx);
		  wid += ivl_lval_width(lval);
		  emit_stmt_lval_piece(scope, lval);
		  fprintf(vlog_out, ", ");
	    }
	    lval = ivl_stmt_lval(stmt, 0);
	    wid += ivl_lval_width(lval);
	    emit_stmt_lval_piece(scope, lval);
	    fprintf(vlog_out, "}");
      } else {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid = ivl_lval_width(lval);
	    emit_stmt_lval_piece(scope, lval);
      }
      return wid;
}

/*
 * Icarus translated <var> = <delay or event> <value> into
 *   begin
 *    <tmp> = <value>;
 *    <delay or event> <var> = <tmp>;
 *   end
 * This routine looks for this pattern and turns it back into the
 * appropriate blocking assignment.
 */
static unsigned is_delayed_or_event_assign(ivl_scope_t scope,
                                           ivl_statement_t stmt)
{
      unsigned wid;
      ivl_statement_t assign, delay, delayed_assign;
      ivl_statement_type_t delay_type;
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
      delay_type = ivl_statement_type(delay);
      if ((delay_type != IVL_ST_DELAYX) &&
          (delay_type != IVL_ST_WAIT)) return 0;
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
	/* The two signals must be the same. */
      if (lsig != rsig) return 0;
	/* And finally the three statements must have the same line number
	 * as the block. */
      if ((ivl_stmt_lineno(stmt) != ivl_stmt_lineno(assign)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(delay)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(delayed_assign))) {
	    return 0;
      }

	/* The pattern matched so generate the appropriate code. */
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      wid = emit_stmt_lval(scope, delayed_assign);
      fprintf(vlog_out, " = ");
      if (delay_type == IVL_ST_DELAYX) {
	    fprintf(vlog_out, "#(");
	    emit_scaled_delayx(scope, ivl_stmt_delay_expr(delay), 1);
      } else {
	    fprintf(vlog_out, "@(");
	    emit_event(scope, delay);
      }
      fprintf(vlog_out, ") ");
      emit_expr(scope, ivl_stmt_rval(assign), wid, 1, 0, 0);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");

      return 1;
}

/*
 * A common routine to emit the basic assignment construct. It can also
 * translate an assignment with an opcode when allowed.
 */
static void emit_assign_and_opt_opcode(ivl_scope_t scope, ivl_statement_t stmt,
                                       unsigned allow_opcode)
{
      unsigned wid;
      char opcode;
      const char *opcode_str;

      assert (ivl_statement_type(stmt) == IVL_ST_ASSIGN);
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
	/* Get the opcode and the string version of the opcode. */
      opcode = ivl_stmt_opcode(stmt);
      switch (opcode) {
	case  0:  opcode_str = ""; break;
	case '+': opcode_str = "+"; break;
	case '-': opcode_str = "-"; break;
	case '*': opcode_str = "*"; break;
	case '/': opcode_str = "/"; break;
	case '%': opcode_str = "%"; break;
	case '&': opcode_str = "&"; break;
	case '|': opcode_str = "|"; break;
	case '^': opcode_str = "^"; break;
	case 'l': opcode_str = "<<"; break;
	case 'r': opcode_str = ">>"; break;
	case 'R': opcode_str = ">>>"; break;
	default:
	    fprintf(stderr, "%s:%u: vlog95 error: unknown assignment operator "
	                    "(%c).\n",
	                    ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
	                    opcode);
	    vlog_errors += 1;
	    opcode_str = "<unknown>";
	    break;
      }
      if (opcode && ! allow_opcode) {
	    fprintf(stderr, "%s:%u: vlog95 error: assignment operator %s= is "
	                    "not allowed in this context.\n",
	                    ivl_stmt_file(stmt), ivl_stmt_lineno(stmt),
	                    opcode_str);
	    vlog_errors += 1;
      }
      fprintf(vlog_out, " = ");
      if (opcode) {
	    unsigned twid = emit_stmt_lval(scope, stmt);
	    assert(twid == wid);
	    fprintf(vlog_out, " %s ", opcode_str);
	      /* The >>>= assignment operator is only allowed when the allow
	       * signed flag is true. */
	    if ((! allow_signed) && (opcode == 'R')) {
		  fprintf(stderr, "%s:%u: vlog95 error: >>>= operator is not "
		                  "supported.\n",
		                  ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
		  vlog_errors += 1;
	    }
      }
      emit_expr(scope, ivl_stmt_rval(stmt), wid, 1, 0, 0);
}

/*
 * Icarus translated <var> = repeat(<count>) <event> <value> into
 *   begin
 *    <tmp> = <value>;
 *    repeat(<count>) <event>;
 *    <var> = <tmp>;
 *   end
 * This routine looks for this pattern and turns it back into the
 * appropriate blocking assignment.
 */
static unsigned is_repeat_event_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      ivl_statement_t assign, event, event_assign, repeat;
      ivl_lval_t lval;
      ivl_expr_t rval;
      ivl_signal_t lsig, rsig;

	/* We must have three block elements. */
      if (ivl_stmt_block_count(stmt) != 3) return 0;
	/* The first must be an assign. */
      assign = ivl_stmt_block_stmt(stmt, 0);
      if (ivl_statement_type(assign) != IVL_ST_ASSIGN) return 0;
	/* The second must be a repeat with an event or an event. */
      repeat = ivl_stmt_block_stmt(stmt, 1);
      if (ivl_statement_type(repeat) != IVL_ST_REPEAT) return 0;
	/* The repeat must have an event statement. */
      event = ivl_stmt_sub_stmt(repeat);
      if (ivl_statement_type(event) != IVL_ST_WAIT) return 0;
	/* The third must be an assign. */
      event_assign = ivl_stmt_block_stmt(stmt, 2);
      if (ivl_statement_type(event_assign) != IVL_ST_ASSIGN) return 0;
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
      rval = ivl_stmt_rval(event_assign);
      if (ivl_expr_type(rval) != IVL_EX_SIGNAL) return 0;
	/* It must not be an array word. */
      if (ivl_expr_oper1(rval)) return 0;
      rsig = ivl_expr_signal(rval);
	/* The two signals must be the same. */
      if (lsig != rsig) return 0;
	/* And finally the four statements must have the same line number
	 * as the block. */
      if ((ivl_stmt_lineno(stmt) != ivl_stmt_lineno(assign)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(repeat)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(event)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(event_assign))) {
	    return 0;
      }

	/* The pattern matched so generate the appropriate code. */
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      wid = emit_stmt_lval(scope, event_assign);
      fprintf(vlog_out, " =");
      if (repeat) {
	    fprintf(vlog_out, " repeat (");
	    emit_expr(scope, ivl_stmt_cond_expr(repeat), 0, 0, 0, 0);
	    fprintf(vlog_out, ")");
      }
      fprintf(vlog_out, " @(");
      emit_event(scope, event);
      fprintf(vlog_out, ") ");
      emit_expr(scope, ivl_stmt_rval(assign), wid, 1, 0, 0);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");

      return 1;
}

/*
 * Icarus translated wait(<expr) <stmt> into
 *   begin
 *    while (<expr> !== 1'b1) @(<expr sensitivities>);
 *    <stmt>
 *   end
 * This routine looks for this pattern and turns it back into a
 * wait statement.
 */
static unsigned is_wait(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_statement_t while_wait, wait, wait_stmt;
      ivl_expr_t while_expr, expr;
      const char *bits;
	/* We must have two block elements. */
      if (ivl_stmt_block_count(stmt) != 2) return 0;
	/* The first must be a while. */
      while_wait = ivl_stmt_block_stmt(stmt, 0);
      if (ivl_statement_type(while_wait) != IVL_ST_WHILE) return 0;
	/* That has a wait with a NOOP statement. */
      wait = ivl_stmt_sub_stmt(while_wait);
      if (ivl_statement_type(wait) != IVL_ST_WAIT) return 0;
      wait_stmt = ivl_stmt_sub_stmt(wait);
      if (ivl_statement_type(wait_stmt) != IVL_ST_NOOP) return 0;
	/* Check that the while condition has the correct form. */
      while_expr = ivl_stmt_cond_expr(while_wait);
      if (ivl_expr_type(while_expr) != IVL_EX_BINARY) return 0;
      if (ivl_expr_opcode(while_expr) != 'N') return 0;
	/* Has a second operator that is a constant 1'b1. */
      expr = ivl_expr_oper2(while_expr);
      if (ivl_expr_type(expr) != IVL_EX_NUMBER) return 0;
      if (ivl_expr_width(expr) != 1) return 0;
      bits = ivl_expr_bits(expr);
      if (*bits != '1') return 0;
// HERE: There is no easy way to verify that the @ sensitivity list
//       matches the first expression so we don't check for that yet.
	/* And finally the two statements that represent the wait must
	 * have the same line number as the block. */
      if ((ivl_stmt_lineno(stmt) != ivl_stmt_lineno(while_wait)) ||
          (ivl_stmt_lineno(stmt) != ivl_stmt_lineno(wait))) {
	    return 0;
      }

	/* The pattern matched so generate the appropriate code. */
      fprintf(vlog_out, "%*cwait(", get_indent(), ' ');
      emit_expr(scope, ivl_expr_oper1(while_expr), 0, 0, 0, 0);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_block_stmt(stmt, 1));
      return 1;
}

/*
 * Check to see if the statement L-value is a port in the given scope.
 * If it is return the zero based port number.
 */
static unsigned utask_in_port_idx(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, ports = ivl_scope_ports(scope);
      unsigned first_arg = is_void_function(scope) ? 1 : 0;
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
      for (idx = first_arg; idx < ports; idx += 1) {
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
      unsigned first_arg = is_void_function(scope) ? 1 : 0;
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
	/* Or a cast of a simple signal. */
      } else if (expr_type == IVL_EX_UNARY) {
	    ivl_expr_t expr = ivl_expr_oper1(rval);
	    char opcode = ivl_expr_opcode(rval);
	      /* This must be a cast opcode. */
	    if ((opcode != '2') && (opcode != 'v') &&
	        (opcode != 'r')) return ports;
	      /* We must be casting a signal. */
	    if (ivl_expr_type(expr) != IVL_EX_SIGNAL) return ports;
	    rsig = ivl_expr_signal(expr);
      } else return ports;
	/* The R-value must have the same scope as the task. */
      if (scope != ivl_signal_scope(rsig)) return ports;
	/* It must not be an array element. */
      if (ivl_signal_dimensions(rsig)) return ports;
	/* It must be an output or inout port of the task. */
      sig_name = ivl_signal_basename(rsig);
      for (idx = first_arg; idx < ports; idx += 1) {
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
      } expr;
}  *port_expr_t;

/*
 * An input prints the R-value and an output or inout print the L-value.
 */
static void emit_port(ivl_scope_t scope, struct port_expr_s port_expr)
{
      if (port_expr.type == IVL_SIP_INPUT) {
// HERE: For a user should the argument width be used here.
	    emit_expr(scope, port_expr.expr.rval, 0, 1, 0, 0);
      } else {
	      /* This is a self-determined context so we don't care about
	       * the width of the L-value. */
	    (void) emit_stmt_lval(scope, port_expr.expr.lval);
      }
}

/*
 * Icarus encodes a user task call with arguments as:
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
 *
 * When calling automatic tasks there is an initial ALLOC statement
 * and a final FREE statement.
 */
static unsigned is_utask_call_with_args(ivl_scope_t scope,
                                        ivl_statement_t stmt)
{
      unsigned idx, ports, task_idx = 0;
      unsigned count = ivl_stmt_block_count(stmt);
      unsigned lineno = ivl_stmt_lineno(stmt);
      unsigned start, stop, is_auto = 0;
      ivl_scope_t task_scope = 0;
      unsigned is_void_func = 0;
      port_expr_t port_exprs;
	/* Check to see if the block is of the basic form first.  */
      for (idx = 0; idx < count; idx += 1) {
	    ivl_statement_t tmp = ivl_stmt_block_stmt(stmt, idx);
	      /* For an automatic task the ALLOC must be first. */
	    if (ivl_statement_type(tmp) == IVL_ST_ALLOC) {
		  if (idx == 0) {
			is_auto = 1;
			continue;
		  }
	    }
	    if (ivl_statement_type(tmp) == IVL_ST_ASSIGN) continue;
	    if (ivl_statement_type(tmp) == IVL_ST_UTASK && !task_scope) {
		  task_idx = idx;
		  task_scope = ivl_stmt_call(tmp);
		  is_void_func = is_void_function(task_scope);
		  assert(ivl_scope_type(task_scope) == IVL_SCT_TASK || is_void_func);
		  continue;
	    }
	      /* For an automatic task the FREE must be last. */
	    if (ivl_statement_type(tmp) == IVL_ST_FREE) {
		  if (idx == count-1) {
			if (is_auto) continue;
		  }
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
	    port_exprs[idx].expr.rval = 0;
      }

	/* Check that the input arguments are correct. */
      if (is_auto) start = 1;
      else start = 0;
      for (idx = start; idx < task_idx; idx += 1) {
	    ivl_statement_t assign = ivl_stmt_block_stmt(stmt, idx);
	    unsigned port = utask_in_port_idx(task_scope, assign);
	    if ((port == ports) || (lineno != ivl_stmt_lineno(assign))) {
		  free(port_exprs);
		  return 0;
	    }
	    port_exprs[port].type = IVL_SIP_INPUT;
	    port_exprs[port].expr.rval = ivl_stmt_rval(assign);
      }
	/* Check that the output arguments are correct. */
      if (is_auto) stop = count-1;
      else stop = count;
      for (idx = task_idx + 1; idx < stop; idx += 1) {
	    ivl_statement_t assign = ivl_stmt_block_stmt(stmt, idx);
	    unsigned port = utask_out_port_idx(task_scope, assign);
	    if ((port == ports) || (lineno != ivl_stmt_lineno(assign))) {
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
	    port_exprs[port].expr.lval = assign;
      }
	/* Check that the task call has the correct line number. */
      if (lineno != ivl_stmt_lineno(ivl_stmt_block_stmt(stmt, task_idx))) {
	    free(port_exprs);
	    return 0;
      }

	/* Verify that all the ports were defined. */
      start = is_void_func ? 1 : 0;
      for (idx = start; idx < ports; idx += 1) {
	    if (port_exprs[idx].type == IVL_SIP_NONE) {
		  free(port_exprs);
		  return 0;
	    }
      }

	/* Now that we have the arguments figured out, print the task call. */
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      if (is_void_func)
            fprintf(vlog_out, "if (");
      emit_scope_path(scope, task_scope);
      fprintf(vlog_out, "(");
      emit_port(scope, port_exprs[start]);
      for (idx = start + 1; idx < ports; idx += 1) {
	    fprintf(vlog_out, ", ");
	    emit_port(scope, port_exprs[idx]);
      }
      free(port_exprs);
      if (is_void_func)
            fprintf(vlog_out, ")");
      fprintf(vlog_out, ");");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      return 1;
}

static void emit_stmt_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c", get_indent(), ' ');
	/* Emit the basic assignment (an opcode is allowed).*/
      emit_assign_and_opt_opcode(scope, stmt, 1);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
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
      emit_expr(scope, ivl_stmt_rval(stmt), wid, 1, 0, 0);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_block(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cbegin", get_indent(), ' ');
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cend\n", get_indent(), ' ');
}

static void emit_stmt_block_named(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      fprintf(vlog_out, "%*cbegin: ", get_indent(), ' ');
      emit_id(ivl_scope_basename(my_scope));
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cend  /* %s */\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
}

static void emit_stmt_case(ivl_scope_t scope, ivl_statement_t stmt)
{
      const char *case_type = 0;
      unsigned idx, default_case, count = ivl_stmt_case_count(stmt);
      switch (ivl_statement_type(stmt)) {
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	    case_type = "case";
	    break;
	case IVL_ST_CASEX:
	    case_type = "casex";
	    break;
	case IVL_ST_CASEZ:
	    case_type = "casez";
	    break;
	default:
	    assert(0);
      }
      fprintf(vlog_out, "%*c%s (", get_indent(), ' ', case_type);
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
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
	    emit_expr(scope, expr, 0, 0, 0, 0);
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
      emit_expr(scope, ivl_stmt_rval(stmt), wid, 1, 0, 0);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_condit(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_statement_t true_stmt = ivl_stmt_cond_true(stmt);
      ivl_statement_t false_stmt = ivl_stmt_cond_false(stmt);
      unsigned nest = 0;
      fprintf(vlog_out, "%*cif (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      if (true_stmt) {
	      /* If we have a false statement and the true statement is a
	       * condition that does not have a false clause then we need
	       * to add a begin/end pair to keep the else clause attached
	       * to this condition. */
	    if (false_stmt &&
	        (ivl_statement_type(true_stmt) == IVL_ST_CONDIT) &&
	        (! ivl_stmt_cond_false(true_stmt))) nest = 1;
	    if (nest) {
		  fprintf(vlog_out, " begin\n");
		  indent += indent_incr;
	    } else single_indent = 1;
	    emit_stmt(scope, true_stmt);
      } else {
	    fprintf(vlog_out, ";\n");
      }
      if (false_stmt) {
	    if (nest) {
		  assert(indent >= indent_incr);
		  indent -= indent_incr;
	    }
	    fprintf(vlog_out, "%*c", get_indent(), ' ');
	    if (nest) fprintf(vlog_out, "end ");
	    fprintf(vlog_out, "else");
	    single_indent = 1;
	    emit_stmt(scope, false_stmt);
      }
}

static void emit_stmt_deassign(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cdeassign ", get_indent(), ' ');
      (void) emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_delay(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c#", get_indent(), ' ');
      emit_scaled_delay(scope, ivl_stmt_delay_val(stmt));
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_delayx(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c#(", get_indent(), ' ');
      emit_scaled_delayx(scope, ivl_stmt_delay_expr(stmt), 1);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static unsigned is_func_disable(ivl_scope_t scope, const ivl_scope_t disable_scope)
{
      assert(func_rtn_name);
	/* Find the enclosing function scope. */
      while (ivl_scope_type(scope) != IVL_SCT_FUNCTION) {
	    scope = ivl_scope_parent(scope);
	    assert(scope);
      }
	/* If the function scope and the scope to be disabled match then this
	 * is a function disable (SystemVerilog return). */
      return scope == disable_scope;
}

static void emit_stmt_disable(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t disable_scope = ivl_stmt_call(stmt);
      fprintf(vlog_out, "%*cdisable ", get_indent(), ' ');
	/* A normal disable statement. */
      if (disable_scope) {
	      /* If this disable is in a function and it is disabling the
	       * function then emit the appropriate function return name. */
	    if (func_rtn_name && is_func_disable(scope, disable_scope)) {
		  fprintf(vlog_out, "%s", func_rtn_name);
	    } else emit_scope_path(scope, disable_scope);
	/* A SystemVerilog disable fork statement cannot be converted. */
      } else {
	    fprintf(vlog_out, "fork");
	    fprintf(stderr, "%s:%u: vlog95 sorry: disable fork is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
      }
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

/*
 * Emit just the statements for this named block since an outer named block
 * was added to keep all the translated code inside a single named block.
 */
static void emit_stmt_do_while_body(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count = ivl_stmt_block_count(stmt);
      unsigned is_begin = 0;
      assert(ivl_stmt_block_scope(stmt));
      switch (ivl_statement_type(stmt)) {
	case IVL_ST_BLOCK:
	    fprintf(vlog_out, "%*cbegin", get_indent(), ' ');
	    is_begin = 1;
	    break;
	case IVL_ST_FORK_JOIN_ANY:
	    fprintf(stderr, "%s:%u: vlog95 sorry: fork/join_any is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
	    return;
	case IVL_ST_FORK_JOIN_NONE:
	    fprintf(stderr, "%s:%u: vlog95 sorry: fork/join_none is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
	    return;
	case IVL_ST_FORK:
	    fprintf(vlog_out, "%*cfork", get_indent(), ' ');
	    break;
	      /* Only named blocks should make it to this code. */
	default:
	    assert(0);
	    break;
      }
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      indent += indent_incr;
      for (idx = 0; idx < count; idx += 1) {
	    emit_stmt(scope, ivl_stmt_block_stmt(stmt, idx));
      }
      assert(indent >= indent_incr);
      indent -= indent_incr;
      if (is_begin) fprintf(vlog_out, "%*cend\n", get_indent(), ' ');
      else fprintf(vlog_out, "%*cjoin\n", get_indent(), ' ');
}

/*
 * Currently a do/while can be translated in two ways: no named blocks in
 * the do/while statement and only a named block as the top level statement.
 * Anything else cannot be translated since a hierarchical variable reference
 * or disable cannot work correctly when the statement is duplicated.
 */
typedef enum stmt_named_type_e {
      NO_NAMED = 0,
      TOP_NAMED = 1,
      OTHER_NAMED = 2
} stmt_named_type_t;

static stmt_named_type_t get_named_type(ivl_statement_t stmt, unsigned is_top);

/*
 * A block can start as either a NO_NAMED or a TOP_NAMED so pass this in.
 */
static stmt_named_type_t get_named_type_block(ivl_statement_t stmt,
                                              stmt_named_type_t def_type)
{
      stmt_named_type_t rtn = def_type;
      unsigned idx, count = ivl_stmt_block_count(stmt);
      for (idx = 0; idx < count; idx += 1) {
	    stmt_named_type_t lrtn;
	    lrtn = get_named_type(ivl_stmt_block_stmt(stmt, idx), 0);
	    if (lrtn > rtn) rtn = lrtn;
	    if (rtn == OTHER_NAMED) break;
      }
      return rtn;
}

static stmt_named_type_t get_named_type_case(ivl_statement_t stmt)
{
      stmt_named_type_t rtn = NO_NAMED;
      unsigned idx, count = ivl_stmt_case_count(stmt);
      for (idx = 0; idx < count; idx += 1) {
	    stmt_named_type_t lrtn;
	    lrtn = get_named_type(ivl_stmt_case_stmt(stmt, idx), 0);
	    if (lrtn > rtn) rtn = lrtn;
	    if (rtn == OTHER_NAMED) break;
      }
      return rtn;
}

static stmt_named_type_t get_named_type_condit(ivl_statement_t stmt)
{
      stmt_named_type_t true_rtn, false_rtn;
      true_rtn = get_named_type(ivl_stmt_cond_true(stmt), 0);
      false_rtn = get_named_type(ivl_stmt_cond_false(stmt), 0);
      if (false_rtn > true_rtn) return false_rtn;
      return true_rtn;
}

static stmt_named_type_t get_named_type(ivl_statement_t stmt, unsigned is_top)
{
      stmt_named_type_t rtn = NO_NAMED;

      if (! stmt) return NO_NAMED;

      switch (ivl_statement_type(stmt)) {
	case IVL_ST_BLOCK:
	case IVL_ST_FORK:
	case IVL_ST_FORK_JOIN_ANY:
	case IVL_ST_FORK_JOIN_NONE:
	      /* Check to see if this is a named top block or sub block. */
	    if (ivl_stmt_block_scope(stmt)) {
		  if (is_top) rtn = TOP_NAMED;
		  else return OTHER_NAMED;
	    }
            rtn = get_named_type_block(stmt, rtn);
	    break;
	case IVL_ST_CASE:
	case IVL_ST_CASER:
	case IVL_ST_CASEX:
	case IVL_ST_CASEZ:
            rtn = get_named_type_case(stmt);
	    break;
	case IVL_ST_CONDIT:
            rtn = get_named_type_condit(stmt);
	    break;
	  /* These statements only have a single sub-statement that is not
	   * a top level statement relative to the original do/while. */
	case IVL_ST_DELAY:
	case IVL_ST_DELAYX:
	case IVL_ST_DO_WHILE:
	case IVL_ST_FOREVER:
	case IVL_ST_REPEAT:
	case IVL_ST_WAIT:
	case IVL_ST_WHILE:
            rtn = get_named_type(ivl_stmt_sub_stmt(stmt), 0);
	    break;
	default:
	      /* The rest of the statement types do not have sub-statements. */
	    break;
      }
      return rtn;
}

/*
 *  Translate a do/while to the following:
 *
 *   statement
 *   while (expr) statement
 */
static void emit_stmt_do_while(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_statement_t sub_stmt = ivl_stmt_sub_stmt(stmt);
      stmt_named_type_t named_type = get_named_type(sub_stmt, 1);

	/* If just the original do/while statement is named then emit it as:
	 *
	 *   begin : name
	 *     <var defs in named>
	 *     statement
	 *     while (expr) statement
	 *   end
	 */
      if (named_type == TOP_NAMED) {
	    ivl_scope_t my_scope = ivl_stmt_block_scope(sub_stmt);
	    assert(my_scope);
	    fprintf(vlog_out, "%*cbegin: ", get_indent(), ' ');
	    emit_id(ivl_scope_basename(my_scope));
	    emit_stmt_file_line(stmt);
	    fprintf(vlog_out, "\n");
	    indent += indent_incr;
	    emit_scope_variables(my_scope);
	    emit_stmt_do_while_body(my_scope, sub_stmt);
	    fprintf(vlog_out, "%*cwhile (", get_indent(), ' ');
	    emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
	    fprintf(vlog_out, ")");
	    emit_stmt_file_line(stmt);
	    single_indent = 1;
	    emit_stmt_do_while_body(my_scope, sub_stmt);
	    assert(indent >= indent_incr);
	    indent -= indent_incr;
	    fprintf(vlog_out, "%*cend  /* %s */\n", get_indent(), ' ',
	                      ivl_scope_basename(my_scope));
      } else {
	    emit_stmt(scope, sub_stmt);
	    fprintf(vlog_out, "%*cwhile (", get_indent(), ' ');
	    emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
	    fprintf(vlog_out, ")");
	    emit_stmt_file_line(stmt);
	    single_indent = 1;
	    emit_stmt(scope, sub_stmt);
      }

	/*
	 * If the do/while has sub-statements that are named it cannot be
	 * translated since the original do/while statement needs to be
	 * duplicated and doing this will create two statements with the
	 * same name.
	 */
      if (named_type == OTHER_NAMED) {
	    fprintf(stderr, "%s:%u: vlog95 sorry: do/while with named "
	                    "sub-statements cannot be translated.\n",
	                    ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
      }
}

static void emit_stmt_force(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*cforce ", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, " = ");
      emit_expr(scope, ivl_stmt_rval(stmt), wid, 1, 0, 0);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_forever(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cforever", get_indent(), ' ');
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_fork(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cfork", get_indent(), ' ');
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cjoin\n", get_indent(), ' ');
}

static void emit_stmt_fork_named(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      fprintf(vlog_out, "%*cfork: ", get_indent(), ' ');
      emit_id(ivl_scope_basename(my_scope));
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cjoin  /* %s */\n", get_indent(), ' ',
                        ivl_scope_basename(my_scope));
}

static void emit_stmt_forloop(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_statement_t use_init = ivl_stmt_init_stmt(stmt);
      ivl_statement_t use_step = ivl_stmt_step_stmt(stmt);
      ivl_statement_t use_stmt = ivl_stmt_sub_stmt(stmt);

      fprintf(vlog_out, "%*cfor (", get_indent(), ' ');
      /* Assume that the init statement is an assignment. */
      if (use_init)
	    emit_assign_and_opt_opcode(scope, use_init, 0);
      fprintf(vlog_out, "; ");
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
      fprintf(vlog_out, "; ");
      /* Assume that the step statement is an assignment. */
      if (use_step)
	    emit_assign_and_opt_opcode(scope, use_step, 1);
      fprintf(vlog_out, ")");
      single_indent = 1;
      emit_stmt(scope, use_stmt);
}

static void emit_stmt_release(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*crelease ", get_indent(), ' ');
      (void) emit_stmt_lval(scope, stmt);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_repeat(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*crepeat (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_stask(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned count = ivl_stmt_parm_count(stmt);
      fprintf(vlog_out, "%*c%s", get_indent(), ' ', ivl_stmt_name(stmt));
      if (count != 0) {
	    unsigned idx;
	    ivl_expr_t expr;
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
		  expr = ivl_stmt_parm(stmt, idx);
		  if (expr) emit_expr(scope, expr, 0, 0, 0, 0);
		  fprintf(vlog_out, ", ");
	    }
	    expr = ivl_stmt_parm(stmt, count);
	    if (expr) emit_expr(scope, expr, 0, 0, 0, 0);
	    fprintf(vlog_out, ")");
      }
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_trigger(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c-> ", get_indent(), ' ');
      assert(ivl_stmt_nevent(stmt) == 1);
      emit_event(scope, stmt);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

static void emit_stmt_nb_trigger(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c->> ", get_indent(), ' ');
      ivl_expr_t delay = ivl_stmt_delay_expr(stmt);
      if (delay) {
	    fprintf(vlog_out, "#(");
	    emit_scaled_delayx(scope, delay, 1);
	    fprintf(vlog_out, ") ");
      }
      assert(ivl_stmt_nevent(stmt) == 1);
      emit_event(scope, stmt);
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      fprintf(stderr, "%s:%u: vlog95 sorry: wait fork is not currently "
                      "translated.\n",
                      ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      vlog_errors += 1;
}

/*
 * A user defined task call with arguments is generated as a block with
 * input assignments, a simple call and then output assignments. This is
 * handled by the is_utask_call_with_args() routine above.
 */
static void emit_stmt_utask(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t task_scope = ivl_stmt_call(stmt);
      assert((ivl_scope_type(task_scope) == IVL_SCT_TASK
		&& ivl_scope_ports(task_scope) == 0)
	  || (is_void_function(task_scope)
		&& ivl_scope_ports(task_scope) == 1));
      fprintf(vlog_out, "%*c", get_indent(), ' ');
      if (is_void_function(task_scope))
            fprintf(vlog_out, "if (");
      emit_scope_path(scope, task_scope);
      if (is_void_function(task_scope))
            fprintf(vlog_out, "(1'bx))");
      fprintf(vlog_out, ";");
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
}

/* Look to see if this is a SystemVerilog wait fork statement. */
static unsigned is_wait_fork(ivl_scope_t scope, ivl_statement_t stmt)
{
      (void)scope;  /* Parameter is not used. */
      if (ivl_stmt_nevent(stmt) != 1) return 0;
      if (ivl_stmt_events(stmt, 0) != 0) return 0;
      assert(ivl_statement_type(ivl_stmt_sub_stmt(stmt)) == IVL_ST_NOOP);

      fprintf(vlog_out, "%*cwait fork;", get_indent(), ' ');
      emit_stmt_file_line(stmt);
      fprintf(vlog_out, "\n");
      fprintf(stderr, "%s:%u: vlog95 sorry: wait fork is not currently "
                      "translated.\n",
                      ivl_stmt_file(stmt), ivl_stmt_lineno(stmt));
      vlog_errors += 1;
      return 1;
}

static void emit_stmt_wait(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c@(", get_indent(), ' ');
      emit_event(scope, stmt);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_while(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cwhile (", get_indent(), ' ');
      emit_expr(scope, ivl_stmt_cond_expr(stmt), 0, 0, 0, 0);
      fprintf(vlog_out, ")");
      emit_stmt_file_line(stmt);
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

void emit_stmt(ivl_scope_t scope, ivl_statement_t stmt)
{
      switch (ivl_statement_type(stmt)) {
	case IVL_ST_NOOP:
	      /* If this is a statement termination then just finish the
	       * statement, otherwise print an empty begin/end pair. */
	    if (single_indent) {
		  single_indent = 0;
		  fprintf(vlog_out, ";\n");
	    } else {
		  fprintf(vlog_out, "%*cbegin\n", get_indent(), ' ');
		  fprintf(vlog_out, "%*cend\n", get_indent(), ' ');
	    }
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
		  if (is_delayed_or_event_assign(scope, stmt)) break;
		  if (is_repeat_event_assign(scope, stmt)) break;
		  if (is_wait(scope, stmt)) break;
		  if (is_utask_call_with_args(scope, stmt)) break;
		  emit_stmt_block(scope, stmt);
	    }
	    break;
	case IVL_ST_BREAK:
	    fprintf(stderr, "%s:%u: vlog95 sorry: 'break' is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
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
	case IVL_ST_CONTINUE:
	    fprintf(stderr, "%s:%u: vlog95 sorry: 'continue' is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
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
	case IVL_ST_DO_WHILE:
	    emit_stmt_do_while(scope, stmt);
	    break;
	case IVL_ST_FORCE:
	    emit_stmt_force(scope, stmt);
	    break;
	case IVL_ST_FOREVER:
	    emit_stmt_forever(scope, stmt);
	    break;
        case IVL_ST_FORLOOP:
	    emit_stmt_forloop(scope, stmt);
	    break;
	case IVL_ST_FORK:
	    if (ivl_stmt_block_scope(stmt)) {
		  emit_stmt_fork_named(scope, stmt);
	    } else {
		  emit_stmt_fork(scope, stmt);
	    }
	    break;
	case IVL_ST_FORK_JOIN_ANY:
	    fprintf(stderr, "%s:%u: vlog95 sorry: fork/join_any is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
	    break;
	case IVL_ST_FORK_JOIN_NONE:
	    fprintf(stderr, "%s:%u: vlog95 sorry: fork/join_none is not "
	                    "currently translated.\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt));
	    vlog_errors += 1;
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
	case IVL_ST_NB_TRIGGER:
	    emit_stmt_nb_trigger(scope, stmt);
	    break;
	case IVL_ST_UTASK:
	    emit_stmt_utask(scope, stmt);
	    break;
	case IVL_ST_WAIT:
	    if (is_wait_fork(scope, stmt)) break;
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
      ivl_statement_t stmt = ivl_process_stmt(proc);
      if (ivl_statement_type(stmt) == IVL_ST_NOOP) return;
      fprintf(vlog_out, "\n%*c", get_indent(), ' ');
      switch (ivl_process_type(proc)) {
        case IVL_PR_INITIAL:
            fprintf(vlog_out, "initial");
            break;
        case IVL_PR_ALWAYS:
        case IVL_PR_ALWAYS_COMB:
        case IVL_PR_ALWAYS_FF:
        case IVL_PR_ALWAYS_LATCH:
            fprintf(vlog_out, "always");
            break;
        case IVL_PR_FINAL:
            fprintf(vlog_out, "final");
            fprintf(stderr, "%s:%u: vlog95 sorry: final blocks are not "
	                    "currently translated.\n",
                            ivl_process_file(proc), ivl_process_lineno(proc));
            vlog_errors+= 1;
            break;
        default:
            fprintf(vlog_out, "<unknown>");
            fprintf(stderr, "%s:%u: vlog95 error: Unknown process type (%d).\n",
                            ivl_process_file(proc), ivl_process_lineno(proc),
                            (int)ivl_process_type(proc));
            vlog_errors+= 1;
            break;
      }
      if (emit_file_line) {
	    fprintf(vlog_out, " /* %s:%u */",
	                      ivl_process_file(proc),
	                      ivl_process_lineno(proc));
      }
      if (ivl_statement_type(stmt) == IVL_ST_NOOP) {
            fprintf(vlog_out, " begin\n%*cend\n", get_indent(), ' ');
      } else {
	    single_indent = 1;
	    emit_stmt(scope, stmt);
      }
}
