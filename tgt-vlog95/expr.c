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
# include <string.h>
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
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
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
	case 'R':
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: >>> operator is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	case 'l':
	case 'r':
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
      emit_scope_call_path(scope, ev_scope);
      emit_id(ivl_event_basename(event));
}

static void emit_expr_scope_piece(ivl_scope_t scope)
{
      ivl_scope_t parent = ivl_scope_parent(scope);
	/* If this scope has a parent then emit it first. */
      if (parent) {
	    emit_expr_scope_piece(parent);
	    fprintf(vlog_out, ".");
      }
      emit_id(ivl_scope_basename(scope));
}

static void emit_expr_scope(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      emit_expr_scope_piece(ivl_expr_scope(expr));
}

static unsigned emit_param_name_in_scope(ivl_scope_t scope, ivl_expr_t expr)
{
      unsigned idx, count, lineno;
      const char *file;
      count = ivl_scope_params(scope);
      file = ivl_expr_file(expr);
      lineno = ivl_expr_lineno(expr);
      for (idx = 0; idx < count; idx += 1) {
	    ivl_parameter_t par = ivl_scope_param(scope, idx);
	    if (lineno != ivl_parameter_lineno(par)) continue;
	    if (strcmp(file, ivl_parameter_file(par)) == 0) {
		    /* Check that the appropriate expression bits match the
		     * original parameter bits. */
		  ivl_expr_t pex = ivl_parameter_expr(par);
		  unsigned wid = ivl_expr_width(expr);
		  unsigned param_wid = ivl_expr_width(pex);
		  const char *my_bits = ivl_expr_bits(expr);
		  const char *param_bits = ivl_expr_bits(pex);
		  unsigned bit;
		  if (param_wid < wid) wid = param_wid;
		  for (bit = 0; bit < wid; bit += 1) {
			if (my_bits[bit] != param_bits[bit]) {
			      fprintf(stderr, "%s:%u: vlog95 error: Constant "
			                      "expression bits do not match "
			                      "parameter bits.\n",
			                      ivl_expr_file(expr),
			                      ivl_expr_lineno(expr));
			      break;
			}
		  }
// HERE: Does this work with an out of scope parameter reference?
//       What about real parameters?
		  emit_id(ivl_parameter_basename(par));
		  return 1;
	    }
      }
      return 0;
}

static void emit_select_name(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
	/* A select of a number is really a parameter select. */
      if (ivl_expr_type(expr) == IVL_EX_NUMBER) {
	      /* Look in the current scope. */
	    if (emit_param_name_in_scope(scope, expr)) return;
	      /* Now look in the enclosing module scope if this is not a
	       * module scope. */
	    if (ivl_scope_type(scope) != IVL_SCT_MODULE) {
		  if (emit_param_name_in_scope(get_module_scope(scope),
		                               expr)) return;
	    }
// HERE: For now we only look in the current and module scope for the
//       parameter that is being selected. We need to also look in other
//       scopes, but that involves a big search.
	    fprintf(vlog_out, "<missing>");
	    fprintf(stderr, "%s:%u: vlog95 error: Unable to find parameter "
	                    "for select expression \n",
	                    ivl_expr_file(expr), ivl_expr_lineno(expr));
	    vlog_errors += 1;
      } else {
	    emit_expr(scope, expr, wid);
      }
}

/*
 * Emit an indexed part select as a concatenation of bit selects.
 */
static void emit_expr_ips(ivl_scope_t scope, ivl_expr_t sig_expr,
                          ivl_expr_t sel_expr, ivl_select_type_t sel_type,
                          unsigned wid, int msb, int lsb)
{
      unsigned idx;
      assert(wid > 0);
      fprintf(vlog_out, "{");
      if (msb >= lsb) {
	    if (sel_type == IVL_SEL_IDX_DOWN) {
		  lsb  += wid - 1;
		  msb  += wid - 1;
		  emit_select_name(scope, sig_expr, wid);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_select_name(scope, sig_expr, wid);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert(sel_type == IVL_SEL_IDX_UP);
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_select_name(scope, sig_expr, wid);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u], ", idx);
		  }
		  emit_select_name(scope, sig_expr, wid);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      } else {
	    if (sel_type == IVL_SEL_IDX_UP) {
		  lsb  -= wid - 1;
		  msb  -= wid - 1;
		  emit_select_name(scope, sig_expr, wid);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_select_name(scope, sig_expr, wid);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert(sel_type == IVL_SEL_IDX_DOWN);
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_select_name(scope, sig_expr, wid);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u], ", idx);
		  }
		  emit_select_name(scope, sig_expr, wid);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      }
}

static void emit_expr_select(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t sel_expr = ivl_expr_oper2(expr);
      ivl_expr_t sig_expr = ivl_expr_oper1(expr);
      ivl_select_type_t sel_type = ivl_expr_sel_type(expr);
      if (sel_expr) {
	    unsigned width = ivl_expr_width(expr);
	    ivl_expr_type_t type = ivl_expr_type(sig_expr);
	    assert(width > 0);
	      /* The compiler uses selects for some shifts. */
	    if (type != IVL_EX_NUMBER && type != IVL_EX_SIGNAL) {
		  fprintf(vlog_out, "(" );
		  emit_select_name(scope, sig_expr, wid);
		  fprintf(vlog_out, " >> " );
		  emit_scaled_expr(scope, sel_expr, 1, 0);
		  fprintf(vlog_out, ")" );
	    } else {
		    /* A constant/parameter must be zero based in 1364-1995
		     * so keep the compiler generated normalization. This
		     * does not always work for selects before the parameter
		     * since 1364-1995 does not support signed math. */
		  int msb = 1;
		  int lsb = 0;
		  if (type == IVL_EX_SIGNAL) {
			ivl_signal_t sig = ivl_expr_signal(sig_expr);
			msb = ivl_signal_msb(sig);
			lsb = ivl_signal_lsb(sig);
		  }
		    /* A bit select. */
		  if (width == 1) {
			emit_select_name(scope, sig_expr, wid);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, "]");
		  } else {
			if (ivl_expr_type(sel_expr) == IVL_EX_NUMBER) {
				/* A constant part select. */
			      emit_select_name(scope, sig_expr, wid);
			      emit_scaled_range(scope, sel_expr, width,
			                        msb, lsb);
			} else {
				/* An indexed part select. */
			      assert(sel_type != IVL_SEL_OTHER);
			      emit_expr_ips(scope, sig_expr, sel_expr,
			                    sel_type, width, msb, lsb);
			}
		  }
	    }
      } else {
// HERE: Should this sign extend if the expression is signed?
	    emit_expr(scope, sig_expr, wid);
      }
}

/*
 * This routine is used to emit both system and user functions.
 */
static void emit_expr_func(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      unsigned count = ivl_expr_parms(expr);
      if (count) {
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

static void emit_expr_signal(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
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
	    fprintf(vlog_out, "(%s", oper);
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, ")");
	    break;
	case '2':
	case 'i':
	case 'r':
	    /* A cast is a noop. */
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
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
	    fprintf(vlog_out, "%s", ivl_expr_name(expr));
	    emit_expr_func(scope, expr, wid);
	    break;
	case IVL_EX_SIGNAL:
	    emit_expr_signal(scope, expr, wid);
	    break;
	case IVL_EX_STRING:
	    emit_string(ivl_expr_string(expr));
	    break;
	case IVL_EX_TERNARY:
	    emit_expr_ternary(scope, expr, wid);
	    break;
	case IVL_EX_UFUNC:
	    emit_scope_path(scope, ivl_expr_def(expr));
	    emit_expr_func(scope, expr, wid);
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
