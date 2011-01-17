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

# include "config.h"
# include "vlog95_priv.h"

// HERE: Do we need to use wid?

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
	case '+':
	case '-':
	case '*':
	case '/':
	case '%':
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
// HERE: We can convert 2 ** <r-val> to 1 << <r-val>
	    emit_expr(scope, ivl_expr_oper1(expr), wid);
	    fprintf(vlog_out, " ** ");
	    emit_expr(scope, ivl_expr_oper2(expr), 0);
	    fprintf(stderr, "%s:%u: vlog95 error: Power operator is not "
	                    "supported.\n",
	                    ivl_expr_file(expr), ivl_expr_lineno(expr));
	    vlog_errors += 1;
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
      for (idx = 0; idx < count; idx += 1) {
	    emit_expr(scope, ivl_expr_parm(expr, idx), 0);
      }
      fprintf(vlog_out, "}");
      if (repeat != 1) fprintf(vlog_out, "}");
}

static void emit_expr_delay(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      emit_scaled_delay(scope, ivl_expr_delay_val(expr));
}

static void emit_expr_number(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      const char *bits = ivl_expr_bits(expr);
      unsigned nbits = ivl_expr_width(expr);
	/* A signed value can only be 32 bits long since it can only be
	 * represented as an integer. We can trim any matching MSB bits
	 * to make it fit. We do not support undefined bits. */
      if (ivl_expr_signed(expr)) {
	    int rtype;
	    int32_t value = get_int32_from_number(expr, &rtype);
	    if (rtype > 0) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Signed number is "
		                  "greater than 32 bits (%u) and cannot be "
		                  "safely represented.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr),
		                  rtype);
	    }
	    if (rtype < 0) {
		  fprintf(vlog_out, "<invalid>");
		  fprintf(stderr, "%s:%u: vlog95 error: Signed number has "
		                  "an undefined bit and cannot be "
		                  "represented.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
		  return;
	    }
	    fprintf(vlog_out, "%"PRId32, value);
	/* An unsigned number is represented in hex if all the bits are
	 * defined and it is more than a single bit otherwise it is
	 * represented in binary form to preserve all the information. */
      } else {
	    int idx;
	    unsigned has_undef = 0;
	    for (idx = (int)nbits -1; idx >= 0; idx -= 1) {
		  if ((bits[idx] != '0') && (bits[idx] != '1')) {
			has_undef = 1;
			break;
		  }
	    }
	    if (has_undef || (nbits < 2)) {
		  fprintf(vlog_out, "%u'b", nbits);
		  for (idx = (int)nbits-1; idx >= 0; idx -= 1) {
			fprintf(vlog_out, "%c", bits[idx]);
		  }
	    } else {
		  int start = 4*(nbits/4);
		  unsigned result = 0;
		  fprintf(vlog_out, "%u'h", nbits);
		    /* The first digit may not be a full hex digit. */
		  if (start < nbits) {
			for (idx = start; idx < nbits; idx += 1) {
			      if (bits[idx] == '1') result |= 1U << (idx%4);
			}
			fprintf(vlog_out, "%1x", result);
		  }
		    /* Now print the full hex digits. */
		  for (idx = start-1; idx >= 0; idx -= 4) {
			result = 0;
			if (bits[idx] == '1') result |= 0x8;
			if (bits[idx-1] == '1') result |= 0x4;
			if (bits[idx-2] == '1') result |= 0x2;
			if (bits[idx-3] == '1') result |= 0x1;
			fprintf(vlog_out, "%1x", result);
		  }
	    }
      }
}

static void emit_expr_scope(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      fprintf(vlog_out, "%s", ivl_scope_name(ivl_expr_scope(expr)));
}

static void emit_expr_select(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t sel_expr = ivl_expr_oper2(expr);
      ivl_expr_t sub_expr = ivl_expr_oper1(expr);
      if (sel_expr) {
	    ivl_signal_t sig = ivl_expr_signal(sub_expr);
	    int msb = 1;
	    int lsb = 0;
	    unsigned width = ivl_expr_width(expr);
	    assert(width > 0);
	    if (ivl_expr_type(sub_expr) == IVL_EX_SIGNAL) {
		  msb = ivl_signal_msb(sig);
		  lsb = ivl_signal_lsb(sig);
	    }
	    emit_expr(scope, sub_expr, wid);
	    if (width == 1) {
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
	    } else {
		  emit_scaled_range(scope, sel_expr, width, msb, lsb);
	    }
      } else {
// HERE: Should this sign extend if the expression is signed?
	    emit_expr(scope, sub_expr, wid);
      }
}

/*
 * This routine is used to emit both system and user functions.
 */
static void emit_expr_func(ivl_scope_t scope, ivl_expr_t expr, const char* name)
{
      unsigned idx, count = ivl_expr_parms(expr);
      fprintf(vlog_out, "%s", name);
      if (count != 0) {
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
		  emit_expr(scope, ivl_expr_parm(expr, idx), 0);
// HERE: Do we need to support a NULL argument for the system functions?
//       See what was done system tasks.
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
// HERE: I think we need to also consider the scope of the func relative
//       to the calling scope to get the correct name.
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
	case IVL_EX_BINARY:
	    emit_expr_binary(scope, expr, wid);
	    break;
	case IVL_EX_CONCAT:
	    emit_expr_concat(scope, expr, wid);
	    break;
	case IVL_EX_DELAY:
	    emit_expr_delay(scope, expr, wid);
	    break;
	case IVL_EX_NUMBER:
	    emit_expr_number(scope, expr, wid);
	    break;
	case IVL_EX_REALNUM:
	    fprintf(vlog_out, "%#g", ivl_expr_dvalue(expr));
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
