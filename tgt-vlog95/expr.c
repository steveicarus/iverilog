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

# include <inttypes.h>
# include "config.h"
# include "vlog95_priv.h"

// HERE: Do we need to use wid in these routines? We should probably use
//       it to verify that the expressions have the expected width.

/*
 * We can convert any 2^n ** <unsigned variable> expression to
 * 1 << (n * <unsigned variable>). If the variable is signed then we need
 * to add a check for less than zero and for that case return 0.
 */
static unsigned emit_power_as_shift(ivl_scope_t scope, ivl_expr_t expr,
                                    unsigned wid)
{
      int rtype;
      int64_t value, scale;
      unsigned is_signed_rval = 0;
      unsigned expr_wid;
      ivl_expr_t lval = ivl_expr_oper1(expr);
      ivl_expr_t rval = ivl_expr_oper2(expr);
	/* The L-value must be a number. */
      if (ivl_expr_type(lval) != IVL_EX_NUMBER) return 0;
	/* The L-value must of the form 2^n. */
      value = get_int64_from_number(lval, &rtype);
      if (rtype) return 0;
      expr_wid = ivl_expr_width(lval);
      if (value < 2) return 0;
      if (value % 2) return 0;
	/* Generate the appropriate conversion. */
      if (ivl_expr_signed(rval)) {
	    emit_expr(scope, rval, 0);
	    fprintf(vlog_out, " < 0 ? %u'h0 : (", expr_wid);
	    is_signed_rval = 1;
      }
      scale = value / 2;
      fprintf(vlog_out, "%u'h1 << ", expr_wid);
      if (scale != 1) {
	    if (is_signed_rval) {
		  fprintf(vlog_out, "(%"PRId64, scale);
	    } else {
		  fprintf(vlog_out, "(%u'h%"PRIx64,
		                    ivl_expr_width(rval), scale);
	    }
	    fprintf(vlog_out, " * ");
      }
      emit_expr(scope, rval, 0);
      if (scale != 1) fprintf(vlog_out, ")");
      if (is_signed_rval) fprintf(vlog_out, ")");
      return 1;
}

static void emit_expr_array(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      emit_scope_module_path(scope, ivl_signal_scope(sig));
      fprintf(vlog_out, "%s", ivl_signal_basename(sig));
}

static void emit_expr_binary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      char *oper = "<invalid>";
      switch(ivl_expr_opcode(expr)) {
	case '+': oper = "+"; break;
	case '-': oper = "-"; break;
	case '*': oper = "*"; break;
	case '/': oper = "/"; break;
	case '%': oper = "%"; break;
	case 'p': oper = "**"; break;
	case 'E': oper = "==="; break;
	case 'e': oper = "=="; break;
	case 'N': oper = "!=="; break;
	case 'n': oper = "!="; break;
	case '<': oper = "<"; break;
	case 'L': oper = "<="; break;
	case '>': oper = ">"; break;
	case 'G': oper = ">="; break;
	case '&': oper = "&"; break;
	case '|': oper = "|"; break;
	case '^': oper = "^"; break;
	case 'A': oper = "&"; break;
	case 'O': oper = "|"; break;
	case 'X': oper = "~^"; break;
	case 'a': oper = "&&"; break;
	case 'o': oper = "||"; break;
	case 'l': oper = "<<"; break;
	case 'r': oper = ">>"; break;
	case 'R': oper = ">>>"; break;
      }

      fprintf(vlog_out, "(");
      switch(ivl_expr_opcode(expr)) {
	case '%':
	    if (ivl_expr_value(expr) == IVL_VT_REAL) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real modulus operator "
		                  "is not supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	case '+':
	case '-':
	case '*':
	case '/':
	case 'E':
	case 'e':
	case 'N':
	case 'n':
	case '<':
	case 'L':
	case '>':
	case 'G':
	case '&':
	case '|':
	case '^':
	case 'X':
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, ivl_expr_oper2(expr), wid);
	    break;
	case 'a':
	case 'o':
	    emit_expr(scope, ivl_expr_oper1(expr), 0);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, ivl_expr_oper2(expr), 0);
	    break;
	case 'l':
	case 'r':
	case 'R':
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, ivl_expr_oper2(expr), 0);
	    break;
	case 'A':
	case 'O':
	    fprintf(vlog_out, "~(");
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, ivl_expr_oper2(expr), wid);
	    fprintf(vlog_out, ")");
	    break;
	case 'p':
	    if (! emit_power_as_shift(scope, expr, wid)) {
		  emit_expr(scope, ivl_expr_oper1(expr), wid);
		  fprintf(vlog_out, " ** ");
		  emit_expr(scope, ivl_expr_oper2(expr), 0);
		  fprintf(stderr, "%s:%u: vlog95 error: Power operator is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	    break;
	default:
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, "<unknown>");
	    emit_expr(scope, ivl_expr_oper2(expr), wid);
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown expression "
	                    "operator (%c).\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr),
	                    ivl_expr_opcode(expr));
	    vlog_errors += 1;
	    break;
      }
      fprintf(vlog_out, ")");
}

static void emit_expr_concat(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      unsigned repeat = ivl_expr_repeat(expr);
      unsigned idx, count = ivl_expr_parms(expr);

      if (repeat != 1) fprintf(vlog_out, "{%u", repeat);
      fprintf(vlog_out, "{");
      count -= 1;
      for (idx = 0; idx < count; idx += 1) {
	    emit_expr(scope, ivl_expr_parm(expr, idx), 0);
	    fprintf(vlog_out, ", ");
      }
      emit_expr(scope, ivl_expr_parm(expr, count), 0);
      fprintf(vlog_out, "}");
      if (repeat != 1) fprintf(vlog_out, "}");
}

static void emit_expr_delay(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      emit_scaled_delay(scope, ivl_expr_delay_val(expr));
}

/*
 * An event in an expression context must be a named event.
 */
static void emit_expr_event(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_event_t event = ivl_expr_event(expr);
      ivl_scope_t ev_scope = ivl_event_scope(event);
      assert(! ivl_event_nany(event));
      assert(! ivl_event_npos(event));
      assert(! ivl_event_nneg(event));
      emit_scope_module_path(scope, ev_scope);
      fprintf(vlog_out, "%s", ivl_event_basename(event));
}

static void emit_expr_scope(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      fprintf(vlog_out, "%s", ivl_scope_name(ivl_expr_scope(expr)));
}

static void emit_expr_select(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t sel_expr = ivl_expr_oper2(expr);
      ivl_expr_t sig_expr = ivl_expr_oper1(expr);
      if (sel_expr) {
	    int msb = 1;
	    int lsb = 0;
	    unsigned width = ivl_expr_width(expr);
	    assert(width > 0);
	    if (ivl_expr_type(sig_expr) == IVL_EX_SIGNAL) {
		  ivl_signal_t sig = ivl_expr_signal(sig_expr);
		  msb = ivl_signal_msb(sig);
		  lsb = ivl_signal_lsb(sig);
	    }
// HERE: If this is a constant then it was likely a parameter reference.
//       We need to find the appropriate parameter and use it instead.
	    emit_expr(scope, sig_expr, wid);
	    if (width == 1) {
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
	    } else {
		  emit_scaled_range(scope, sel_expr, width, msb, lsb);
	    }
      } else {
// HERE: Should this sign extend if the expression is signed?
	    emit_expr(scope, sig_expr, wid);
      }
}

/*
 * This routine is used to emit both system and user functions.
 */
static void emit_expr_func(ivl_scope_t scope, ivl_expr_t expr, const char* name)
{
      unsigned count = ivl_expr_parms(expr);
      fprintf(vlog_out, "%s", name);
      if (count != 0) {
	    unsigned idx;
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
		  emit_expr(scope, ivl_expr_parm(expr, idx), 0);
		  fprintf(vlog_out, ", ");
	    }
	    emit_expr(scope, ivl_expr_parm(expr, count), 0);
	    fprintf(vlog_out, ")");
      }
}

static void emit_expr_sfunc(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      emit_expr_func(scope, expr, ivl_expr_name(expr));
}

static void emit_expr_signal(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      emit_scope_module_path(scope, ivl_signal_scope(sig));
      fprintf(vlog_out, "%s", ivl_signal_basename(sig));
      if (ivl_signal_dimensions(sig)) {
	    int lsb = ivl_signal_array_base(sig);
	    int msb = lsb + ivl_signal_array_count(sig);
	    fprintf(vlog_out, "[");
	    emit_scaled_expr(scope, ivl_expr_oper1(expr), msb, lsb);
	    fprintf(vlog_out, "]");
      }
}

static void emit_expr_ternary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      fprintf(vlog_out, "(");
      emit_expr(scope, ivl_expr_oper1(expr), 0);
      fprintf(vlog_out, " ? ");
      emit_expr(scope, ivl_expr_oper2(expr), wid);
      fprintf(vlog_out, " : ");
      emit_expr(scope, ivl_expr_oper3(expr), wid);
      fprintf(vlog_out, ")");
}

static void emit_expr_ufunc(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_scope_t ufunc_def = ivl_expr_def(expr);
      emit_scope_module_path(scope, ufunc_def);
      emit_expr_func(scope, expr, ivl_scope_tname(ufunc_def));
}

static void emit_expr_unary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      char *oper = "invalid";
      switch (ivl_expr_opcode(expr)) {
	case '-': oper = "-"; break;
	case '~': oper = "~"; break;
	case '&': oper = "&"; break;
	case '|': oper = "|"; break;
	case '^': oper = "^"; break;
	case 'A': oper = "~&"; break;
	case 'N': oper = "~|"; break;
	case 'X': oper = "~^"; break;
	case '!': oper = "!"; break;
      }
      switch (ivl_expr_opcode(expr)) {
	case '-':
	case '~':
	case '&':
	case '|':
	case '^':
	case 'A':
	case 'N':
	case 'X':
	case '!':
	    fprintf(vlog_out, "%s", oper);
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    break;
	case '2':
	case 'i':
	case 'r':
	    /* A cast is a noop. */
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown unary "
	                    "operator (%c).\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr),
	                    ivl_expr_opcode(expr));
	    break;
      }
}

void emit_expr(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      switch(ivl_expr_type(expr)) {
	case IVL_EX_ARRAY:
	    emit_expr_array(scope, expr, wid);
	    break;
	case IVL_EX_BINARY:
	    emit_expr_binary(scope, expr, wid);
	    break;
	case IVL_EX_CONCAT:
	    emit_expr_concat(scope, expr, wid);
	    break;
	case IVL_EX_DELAY:
	    emit_expr_delay(scope, expr, wid);
	    break;
	case IVL_EX_EVENT:
	    emit_expr_event(scope, expr, wid);
	    break;
	case IVL_EX_NUMBER:
	    emit_number(ivl_expr_bits(expr), ivl_expr_width(expr),
	                ivl_expr_signed(expr), ivl_expr_file(expr),
	                ivl_expr_lineno(expr));
	    break;
	case IVL_EX_REALNUM:
	    emit_real_number(ivl_expr_dvalue(expr));
	    break;
	case IVL_EX_SCOPE:
	    emit_expr_scope(scope, expr, wid);
	    break;
	case IVL_EX_SELECT:
	    emit_expr_select(scope, expr, wid);
	    break;
	case IVL_EX_SFUNC:
	    emit_expr_sfunc(scope, expr, wid);
	    break;
	case IVL_EX_SIGNAL:
	    emit_expr_signal(scope, expr, wid);
	    break;
	case IVL_EX_STRING:
	    fprintf(vlog_out, "\"%s\"", ivl_expr_string(expr));
	    break;
	case IVL_EX_TERNARY:
	    emit_expr_ternary(scope, expr, wid);
	    break;
	case IVL_EX_UFUNC:
	    emit_expr_ufunc(scope, expr, wid);
	    break;
	case IVL_EX_UNARY:
	    emit_expr_unary(scope, expr, wid);
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown expression "
	                    "type (%d).\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr),
	                    (int)ivl_expr_type(expr));
	    vlog_errors += 1;
	    break;
      }
}
