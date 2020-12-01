/*
 * Copyright (C) 2011-2020 Cary R. (cygcary@yahoo.com)
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

/* The expression code needs to know when a parameter definition is being
 * emitted so it can print the numeric value instead of the parameter name. */
ivl_parameter_t emitting_param = 0;

/*
 * Data type used to signify if a $signed or $unsigned should be emitted.
 */
typedef enum expr_sign_e {
      NO_SIGN = 0,
      NEED_SIGNED = 1,
      NEED_UNSIGNED = 2
} expr_sign_t;

static expr_sign_t expr_get_binary_sign_type(ivl_expr_t expr)
{
      ivl_expr_t oper1, oper2;
      int opr_sign = 0;
      int expr_sign = ivl_expr_signed(expr);
      expr_sign_t rtn = NO_SIGN;

      switch (ivl_expr_opcode(expr)) {
	case 'E':
	case 'e':
	case 'w':
	case 'N':
	case 'n':
	case 'W':
	case '<':
	case 'L':
	case '>':
	case 'G':
	      /* The comparison operators always act as if the argument is
	       * unsigned. */
	    break;
	case 'l':
	case 'r':
	case 'R':
	      /* For the shift operators only the left operand is used to
	       * determine the sign information. */
	    opr_sign = ivl_expr_signed(ivl_expr_oper1(expr));
	    break;
	default:
	      /* For the rest of the opcodes the operator is considered to
	       * be signed if either argument is real or if both arguments
	       * are signed. */
	    oper1 = ivl_expr_oper1(expr);
	    oper2 = ivl_expr_oper2(expr);
	    if ((ivl_expr_value(oper1) == IVL_VT_REAL)  ||
	        (ivl_expr_value(oper2) == IVL_VT_REAL)) {
		  opr_sign = 1;
	    } else if (ivl_expr_signed(oper1) && ivl_expr_signed(oper2)) {
		  opr_sign = 1;
	    }
	    break;
      }

	/* Check to see if a $signed() or $unsigned() is needed. */
      if (expr_sign && ! opr_sign) rtn = NEED_SIGNED;
      if (! expr_sign && opr_sign) rtn = NEED_UNSIGNED;

      return rtn;
}

static expr_sign_t expr_get_select_sign_type(ivl_expr_t expr,
                                             unsigned can_skip_unsigned)
{
      int opr_sign = 0;
      int expr_sign = ivl_expr_signed(expr);
      expr_sign_t rtn = NO_SIGN;

	/* If there is no select expression then the sign is determined by
	 * the expression that is being selected (padded). */
      if (! ivl_expr_oper2(expr)) {
	    ivl_expr_t oper1 = ivl_expr_oper1(expr);
	    opr_sign = ivl_expr_signed(oper1);
	      /* If the expression being padded is not a signal then don't
	       * skip a soft $unsigned() (from the binary operator). */
	    if (ivl_expr_type(oper1) != IVL_EX_SIGNAL) can_skip_unsigned -= 1;
      }

	/* Check to see if a $signed() or $unsigned() is needed. */
      if (expr_sign && ! opr_sign) rtn = NEED_SIGNED;
      if (! expr_sign && opr_sign && ! can_skip_unsigned) rtn = NEED_UNSIGNED;

      return rtn;
}

static expr_sign_t expr_get_signal_sign_type(ivl_expr_t expr,
                                             unsigned can_skip_unsigned)
{
      int opr_sign = ivl_signal_signed(ivl_expr_signal(expr));
      int expr_sign = ivl_expr_signed(expr);
      expr_sign_t rtn = NO_SIGN;

	/* Check to see if a $signed() or $unsigned() is needed. */
      if (expr_sign && ! opr_sign) rtn = NEED_SIGNED;
      if (! expr_sign && opr_sign && ! can_skip_unsigned) rtn = NEED_UNSIGNED;

      return rtn;
}

static expr_sign_t expr_get_unary_sign_type(ivl_expr_t expr)
{
      ivl_expr_t expr1;
      int opr_sign = 0;
      int expr_sign = ivl_expr_signed(expr);
      expr_sign_t rtn = NO_SIGN;

      switch (ivl_expr_opcode(expr)) {
	case '&':
	case '|':
	case '^':
	case 'A':
	case 'N':
	case 'X':
	      /* The reduction operators always act as if the argument is
	       * unsigned. */
	case '!':
	      /* The logical negation operator works on either type. */
	    break;
	case 'r':
	      /* For a cast to real the expression should be signed and no
	       * sign conversion is needed. */
	    opr_sign = expr_sign;
	    if (! expr_sign)  {
		  fprintf(stderr, "%s:%u: vlog95 error: Cast to real "
		                  "expression is not signed.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	    break;
	case '2':
	case 'v':
	      /* If the cast to a vector value is from a real then no sign
	       * conversion is needed. Otherwise use the actual argument. */
	    expr1 = ivl_expr_oper1(expr);
	    if (ivl_expr_value(expr1) == IVL_VT_REAL) {
		  opr_sign = expr_sign;
	    } else {
		  opr_sign = ivl_expr_signed(expr1);
	    }
	    break;
	default:
	      /* For the rest of the opcodes the argument sign type depends
	       * on the actual argument. */
	    opr_sign = ivl_expr_signed(ivl_expr_oper1(expr));
	    break;
      }

	/* Check to see if a $signed() or $unsigned() is needed. */
      if (expr_sign && ! opr_sign) rtn = NEED_SIGNED;
      if (! expr_sign && opr_sign) rtn = NEED_UNSIGNED;

      return rtn;
}

/*
 * This routine is used to determine if the expression is a binary operator
 * where the width could be different to create a self-determined context.
 */
static unsigned expr_is_binary_self_det(ivl_expr_t expr)
{
      unsigned rtn = 0;
      if (ivl_expr_type(expr) == IVL_EX_BINARY) {
	    switch (ivl_expr_opcode(expr)) {
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '&':
		case '|':
		case '^':
		case 'X':
		case 'A':
		case 'O':
		case 'R':
		case 'l':
		case 'r':
		  rtn = 1;
		  break;
		default:
		  break;
	    }
      }
      return rtn;
}

/*
 * Determine if a $signed() or $unsigned() system function is needed to get
 * the expression sign information correct. can_skip_unsigned may be set for
 * the binary/ternary operators if one of the operands will implicitly cast
 * the expression to unsigned. See calc_can_skip_unsigned() for the details.
 */
static expr_sign_t expr_get_sign_type(ivl_expr_t expr, unsigned wid,
                                      unsigned can_skip_unsigned,
                                      unsigned is_full_prec)
{
      unsigned expr_wid = ivl_expr_width(expr);
      expr_sign_t rtn = NO_SIGN;
      ivl_expr_type_t type = ivl_expr_type(expr);

      switch (type) {
	case IVL_EX_BINARY:
	    rtn = expr_get_binary_sign_type(expr);
	    break;
	case IVL_EX_CONCAT:
	      /* A concatenation is always unsigned so add a $signed() when
	       * needed. */
	    if (ivl_expr_signed(expr)) rtn = NEED_SIGNED;
	    break;
	case IVL_EX_SELECT:
	    rtn = expr_get_select_sign_type(expr, can_skip_unsigned);
	      /* If there is no select expression then this is padding so use
	       * the actual expressions width if it is a binary operator that
	       * could create a self-determined context. */
	    if (! ivl_expr_oper2(expr)) {
		  ivl_expr_t oper1 = ivl_expr_oper1(expr);
		  if (expr_is_binary_self_det(oper1)) {
			expr_wid = ivl_expr_width(oper1);
		  }
	    }
	    break;
	case IVL_EX_SIGNAL:
	    rtn = expr_get_signal_sign_type(expr, can_skip_unsigned);
	    break;
	case IVL_EX_UNARY:
	    rtn = expr_get_unary_sign_type(expr);
	    break;
	  /* These do not currently have sign casting information. A select
	   * is used for that purpose. */
	case IVL_EX_ARRAY:
	case IVL_EX_DELAY:
	case IVL_EX_ENUMTYPE:
	case IVL_EX_EVENT:
	case IVL_EX_NEW:
	case IVL_EX_NULL:
	case IVL_EX_NUMBER:
	case IVL_EX_PROPERTY:
	case IVL_EX_REALNUM:
	case IVL_EX_SCOPE:
	case IVL_EX_SFUNC:
	case IVL_EX_SHALLOWCOPY:
	case IVL_EX_STRING:
	case IVL_EX_TERNARY:
	case IVL_EX_UFUNC:
	default:
	    break;
      }

	/* Check for a self-determined context. A zero width expression
	 * is special and is not considered a self determined context. */
      if ((rtn == NO_SIGN) && (wid != expr_wid) && expr_wid &&
          ! (is_full_prec && ((expr_wid < wid) || (type == IVL_EX_SIGNAL)))) {
	    if (ivl_expr_signed(expr)) rtn = NEED_SIGNED;
	    else rtn = NEED_UNSIGNED;
      }

      return rtn;
}

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
      unsigned is_signed_expr = ivl_expr_signed(expr);
      unsigned expr_wid;
      ivl_expr_t lval = ivl_expr_oper1(expr);
      ivl_expr_t rval = ivl_expr_oper2(expr);
      (void)wid;  /* Parameter is not used. */
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
	    emit_expr(scope, rval, 0, 0, 0, 0);
	    fprintf(vlog_out, " < 0 ? ");
	    if (is_signed_expr) {
		  if (expr_wid == 32) {
			fprintf(vlog_out, "0");
		  } else if (allow_signed) {
			fprintf(vlog_out, "%u'sh0", expr_wid);
		  } else {
			fprintf(stderr, "%s:%u: vlog95 error: Sized signed "
			                "power operator l-value is not "
			                "supported.\n", ivl_expr_file(expr),
			                ivl_expr_lineno(expr));
			vlog_errors += 1;
			fprintf(vlog_out, "%u'h0", expr_wid);
		  }
	    } else {
		  fprintf(vlog_out, "%u'h0", expr_wid);
	    }
	    fprintf(vlog_out, " : (");
	    is_signed_rval = 1;
      }
      scale = value / 2;
      if (is_signed_expr) {
	    if (expr_wid == 32) {
		  fprintf(vlog_out, "1");
	    } else if (allow_signed) {
		  fprintf(vlog_out, "%u'sh1", expr_wid);
	    } else {
		    /* This is an error condition and has already been
		     * reported above. */
		  fprintf(vlog_out, "%u'h1", expr_wid);
	    }
      } else {
	    fprintf(vlog_out, "%u'h1", expr_wid);
      }
      fprintf(vlog_out, " << ");
      if (scale != 1) {
	    if (is_signed_rval) {
		  fprintf(vlog_out, "(%"PRId64, scale);
	    } else {
		  fprintf(vlog_out, "(%u'h%"PRIx64,
		                    ivl_expr_width(rval), scale);
	    }
	    fprintf(vlog_out, " * ");
      }
      emit_expr(scope, rval, 0, 0, 0, 0);
      if (scale != 1) fprintf(vlog_out, ")");
      if (is_signed_rval) fprintf(vlog_out, ")");
      return 1;
}

static void emit_expr_array(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      (void)wid;  /* Parameter is not used. */
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
}

/*
 * Some of the operators can do an implicit cast to real so for them emit
 * the non-real argument in a self-determined context.
 */
static unsigned get_cast_width(ivl_expr_t expr, ivl_expr_t oper, unsigned wid)
{
      ivl_variable_type_t expr_type = ivl_expr_value(expr);
      if ((expr_type == IVL_VT_REAL) && (expr_type != ivl_expr_value(oper))) {
	    wid = 0;
      }
      return wid;
}

static unsigned calc_can_skip_unsigned(ivl_expr_t oper1, ivl_expr_t oper2)
{
      unsigned oper1_signed, oper2_signed;
	/* Check to see if the operands are signed or softly signed.
	 *   The expression is signed (hard).
	 *   It is a signed signal cast to unsigned (soft).
	 *   It is a padding cast from signed to unsigned (soft). */
      oper1_signed = ivl_expr_signed(oper1);
      oper1_signed |= (ivl_expr_type(oper1) == IVL_EX_SIGNAL) &&
                      (ivl_signal_signed(ivl_expr_signal(oper1)));
      oper1_signed |= (ivl_expr_type(oper1) == IVL_EX_SELECT) &&
                      (! ivl_expr_oper2(oper1)) &&
                      (ivl_expr_signed(ivl_expr_oper1(oper1)));
      oper2_signed = ivl_expr_signed(oper2);
      oper2_signed |= (ivl_expr_type(oper2) == IVL_EX_SIGNAL) &&
                      (ivl_signal_signed(ivl_expr_signal(oper2)));
      oper2_signed |= (ivl_expr_type(oper2) == IVL_EX_SELECT) &&
                      (! ivl_expr_oper2(oper2)) &&
                      (ivl_expr_signed(ivl_expr_oper1(oper2)));
	/* If either operand is a hard unsigned skip adding an explicit
	 * $unsigned() since it will be added implicitly. */
      return ! oper1_signed || ! oper2_signed;
}

static void emit_expr_binary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid,
                             unsigned is_full_prec)
{
      const char *oper = "<invalid>";
      ivl_expr_t oper1 = ivl_expr_oper1(expr);
      ivl_expr_t oper2 = ivl_expr_oper2(expr);
      unsigned can_skip_unsigned = calc_can_skip_unsigned(oper1, oper2);

      switch (ivl_expr_opcode(expr)) {
	case '+': oper = "+"; break;
	case '-': oper = "-"; break;
	case '*': oper = "*"; break;
	case '/': oper = "/"; break;
	case '%': oper = "%"; break;
	case 'p': oper = "**"; break;
	case 'E': oper = "==="; break;
	case 'e': oper = "=="; break;
	case 'w': oper = "==?"; break;
	case 'N': oper = "!=="; break;
	case 'n': oper = "!="; break;
	case 'W': oper = "!=?"; break;
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
	case 'm': oper = "min"; break;
	case 'M': oper = "max"; break;
      }

      fprintf(vlog_out, "(");
      switch (ivl_expr_opcode(expr)) {
	case '%':
	    if (ivl_expr_value(expr) == IVL_VT_REAL) {
		  fprintf(stderr, "%s:%u: vlog95 error: Real modulus operator "
		                  "is not supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	    //fallthrough
	case '+':
	case '-':
	case '*':
	case '/':
	    emit_expr(scope, oper1, get_cast_width(expr, oper1, wid), 0,
	              can_skip_unsigned, is_full_prec);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, get_cast_width(expr, oper2, wid), 0,
	              can_skip_unsigned, is_full_prec);
	    break;
	case '&':
	case '|':
	case '^':
	case 'X':
	    emit_expr(scope, oper1, wid, 0, can_skip_unsigned, is_full_prec);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, wid, 0, can_skip_unsigned, is_full_prec);
	    break;
	case 'w':
	case 'W':
	    fprintf(stderr, "%s:%u: vlog95 error: The wild equality operators "
	                    "cannot be converted.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    // fallthrough
	case 'E':
	case 'e':
	case 'N':
	case 'n':
	case '<':
	case 'L':
	case '>':
	case 'G':
	    emit_expr(scope, oper1, ivl_expr_width(oper1), 0,
	              can_skip_unsigned, 0);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, ivl_expr_width(oper2), 0,
	              can_skip_unsigned, 0);
	    break;
	case 'a':
	case 'o':
	    emit_expr(scope, oper1, ivl_expr_width(oper1), 0, 1, 0);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, ivl_expr_width(oper2), 0, 1, 0);
	    break;
	case 'q': // The arguments have already been reduced
	    fprintf(vlog_out, "{~");
	    emit_expr(scope, oper1, ivl_expr_width(oper1), 0, 1, 0);
	    fprintf(vlog_out, " | ");
	    emit_expr(scope, oper2, ivl_expr_width(oper2), 0, 1, 0);
	    fprintf(vlog_out, "}");
	    break;
	case 'Q': // The arguments have already been reduced
	    fprintf(vlog_out, "{");
	    emit_expr(scope, oper1, ivl_expr_width(oper1), 0, 1, 0);
	    fprintf(vlog_out, " ~^ ");
	    emit_expr(scope, oper2, ivl_expr_width(oper2), 0, 1, 0);
	    fprintf(vlog_out, "}");
	    break;
	case 'R':
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: >>> operator is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	    // fallthrough
	case 'l':
	case 'r':
	    emit_expr(scope, oper1, wid, 0, 0, 0);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, ivl_expr_width(oper2), 0, 2, 0);
	    break;
	case 'A':
	case 'O':
	    fprintf(vlog_out, "~(");
	    emit_expr(scope, oper1, wid, 0, can_skip_unsigned, is_full_prec);
	    fprintf(vlog_out, " %s ", oper);
	    emit_expr(scope, oper2, wid, 0, can_skip_unsigned, is_full_prec);
	    fprintf(vlog_out, ")");
	    break;
	case 'p':
	    if (! emit_power_as_shift(scope, expr, wid)) {
		  emit_expr(scope, oper1, wid, 0, 0, is_full_prec);
		  fprintf(vlog_out, " ** ");
		  emit_expr(scope, oper2, ivl_expr_width(oper2), 0, 0, 0);
		  fprintf(stderr, "%s:%u: vlog95 error: Power operator is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	    break;
	  /* Convert the Verilog-A min() or max() functions. */
	case 'm':
	case 'M':
	    if (ivl_expr_value(expr) == IVL_VT_REAL) {
		    /* For a real expression use the $min()/$max() function. */
		  if (ivl_expr_opcode(expr) == 'm') fprintf(vlog_out, "$min(");
		  else fprintf(vlog_out, "$max(");
		  emit_expr(scope, oper1, wid, 0, 0, 0);
		  fprintf(vlog_out, ",");
		  emit_expr(scope, oper2, wid, 0, 0, 0);
		  fprintf(vlog_out, ")");
	    } else {
		    /* This only works when the argument has no side effect. */
		  fprintf(vlog_out, "((");
		  emit_expr(scope, oper1, wid, 0, 0, 0);
		  fprintf(vlog_out, ") %s (", oper);
		  emit_expr(scope, oper2, wid, 0, 0, 0);
		  fprintf(vlog_out, ") ? (");
		  emit_expr(scope, oper1, wid, 0, 0, 0);
		  fprintf(vlog_out, ") : (");
		  emit_expr(scope, oper2, wid, 0, 0, 0);
		  fprintf(vlog_out, "))");
	    }
	    break;
	default:
	    emit_expr(scope, oper1, wid, 0, can_skip_unsigned, 0);
	    fprintf(vlog_out, "<unknown>");
	    emit_expr(scope, oper2, wid, 0, can_skip_unsigned, 0);
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

      (void)wid;  /* Parameter is not used. */
      if (repeat != 1) fprintf(vlog_out, "{%u", repeat);
      fprintf(vlog_out, "{");
      count -= 1;
      for (idx = 0; idx < count; idx += 1) {
	    emit_expr(scope, ivl_expr_parm(expr, idx), 0, 0, 0, 0);
	    fprintf(vlog_out, ", ");
      }
      emit_expr(scope, ivl_expr_parm(expr, count), 0, 0, 0, 0);
      fprintf(vlog_out, "}");
      if (repeat != 1) fprintf(vlog_out, "}");
}

static void emit_expr_delay(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      (void)wid;  /* Parameter is not used. */
      emit_scaled_delay(scope, ivl_expr_delay_val(expr));
}

/*
 * An event in an expression context must be a named event.
 */
static void emit_expr_event(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_event_t event = ivl_expr_event(expr);
      ivl_scope_t ev_scope = ivl_event_scope(event);
      (void)wid;  /* Parameter is not used. */
      assert(! ivl_event_nany(event));
      assert(! ivl_event_npos(event));
      assert(! ivl_event_nneg(event));
      emit_scope_call_path(scope, ev_scope);
      emit_id(ivl_event_basename(event));
}

/*
 * A number can also be a parameter reference. If it is a parameter
 * reference then emit the appropriate parameter name instead of the
 * numeric value unless this is the actual parameter definition.
 */
static void emit_expr_number(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_parameter_t param = ivl_expr_parameter(expr);
      (void)wid;  /* Parameter is not used. */
      if (param && (param != emitting_param)) {
	    emit_scope_call_path(scope, ivl_parameter_scope(param));
	    emit_id(ivl_parameter_basename(param));
      } else {
	    emit_number(ivl_expr_bits(expr), ivl_expr_width(expr),
	                ivl_expr_signed(expr), ivl_expr_file(expr),
	                ivl_expr_lineno(expr));
      }
}

static void emit_expr_real_number(ivl_scope_t scope, ivl_expr_t expr,
                                  unsigned wid)
{
      ivl_parameter_t param = ivl_expr_parameter(expr);
      (void)wid;  /* Parameter is not used. */
      if (param && (param != emitting_param)) {
	    emit_scope_call_path(scope, ivl_parameter_scope(param));
	    emit_id(ivl_parameter_basename(param));
      } else {
	    emit_real_number(ivl_expr_dvalue(expr));
      }
}

/*
 * Class properties are not supported in vlog95, but they can be translated.
 */
static void emit_class_property(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      (void)wid;  /* Parameter is not used. */
      emit_scope_call_path(scope, ivl_signal_scope(sig));
      emit_id(ivl_signal_basename(sig));
      fprintf(vlog_out, ".%s", ivl_expr_name(expr));
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
      (void)scope;  /* Parameter is not used. */
      (void)wid;  /* Parameter is not used. */
      emit_expr_scope_piece(ivl_expr_scope(expr));
}

static void emit_select_name(ivl_scope_t scope, ivl_expr_t expr)
{
	/* A select of a number is really a parameter select. */
      if (ivl_expr_type(expr) == IVL_EX_NUMBER) {
	    ivl_parameter_t param = ivl_expr_parameter(expr);
	    if (param) {
		  emit_scope_call_path(scope, ivl_parameter_scope(param));
		  emit_id(ivl_parameter_basename(param));
	    } else {
		  fprintf(vlog_out, "<missing>");
		  fprintf(stderr, "%s:%u: vlog95 error: Unable to find "
		                  "parameter for select expression \n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
      } else {
	    emit_expr(scope, expr, 0, 0, 0, 0);
      }
}

/*
 * Emit a packed array access as a concatenation of bit selects.
 */
static void emit_expr_packed(ivl_scope_t scope, ivl_expr_t sig_expr,
                             ivl_expr_t sel_expr, unsigned wid)
{
      unsigned idx;
      assert(wid > 0);
      fprintf(vlog_out, "{");
      for (idx = wid - 1; idx > 0; idx -= 1) {
	    emit_select_name(scope, sig_expr);
	    fprintf(vlog_out, "[");
	    emit_expr(scope, sel_expr, 0, 0, 0, 1);
	    fprintf(vlog_out, " + %u], ", idx);
      }
      emit_select_name(scope, sig_expr);
      fprintf(vlog_out, "[");
      emit_expr(scope, sel_expr, 0, 0, 0, 1);
      fprintf(vlog_out, "]}");
}

/*
 * Emit an indexed part select as a concatenation of bit selects. Since a
 * parameter in 1364-1995 is always zero based the select expression needs
 * to keep any scaling that was done by the compiler.
 */
static void emit_expr_ips(ivl_scope_t scope, ivl_expr_t sig_expr,
                          ivl_expr_t sel_expr, ivl_select_type_t sel_type,
                          unsigned wid, int msb, int lsb, unsigned is_param)
{
      unsigned idx;
      assert(wid > 0);
	/* If it was not already given, note that a down index part select will
	 * require the -pallowsigned=1 flag to get the index values correct if
	 * the select expression is not signed. */
      if ((! allow_signed ) && (sel_type == IVL_SEL_IDX_DOWN) &&
          (! ivl_expr_signed(sel_expr))) {
	    fprintf(stderr, "%s:%u: vlog95 note: Translating a down indexed "
	                    "part select requires the -pallowsigned=1 flag.\n",
	                    ivl_expr_file(sel_expr), ivl_expr_lineno(sel_expr));
      }
      fprintf(vlog_out, "{");
      if (msb >= lsb) {
	    if ((sel_type == IVL_SEL_IDX_DOWN) && ! is_param) {
		  lsb  += wid - 1;
		  msb  += wid - 1;
		  emit_select_name(scope, sig_expr);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_select_name(scope, sig_expr);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert((sel_type == IVL_SEL_IDX_UP) ||
		         (is_param && (sel_type == IVL_SEL_IDX_DOWN)));
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_select_name(scope, sig_expr);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u], ", idx);
		  }
		  emit_select_name(scope, sig_expr);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      } else {
	    if ((sel_type == IVL_SEL_IDX_UP) && ! is_param) {
		  lsb  -= wid - 1;
		  msb  -= wid - 1;
		  emit_select_name(scope, sig_expr);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]");
		  for (idx = 1; idx < wid; idx += 1) {
			fprintf(vlog_out, ", ");
			emit_select_name(scope, sig_expr);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " + %u]", idx);
		  }
		  fprintf(vlog_out, "}");
	    } else {
		  assert((sel_type == IVL_SEL_IDX_DOWN) ||
		         (is_param && (sel_type == IVL_SEL_IDX_UP)));
		  for (idx = wid - 1; idx > 0; idx -= 1) {
			emit_select_name(scope, sig_expr);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, " - %u], ", idx);
		  }
		  emit_select_name(scope, sig_expr);
		  fprintf(vlog_out, "[");
		  emit_scaled_expr(scope, sel_expr, msb, lsb);
		  fprintf(vlog_out, "]}");
	    }
      }
}

static void check_select_signed(ivl_expr_t sig_expr, ivl_expr_t sel_expr,
                                int msb, int lsb)
{
      expr_sign_t sign_type = expr_get_sign_type(sel_expr,
                                                 ivl_expr_width(sel_expr),
                                                 0, 1);
      char msg[64];
      snprintf(msg, sizeof(msg), "%s:%u",
                                 ivl_expr_file(sel_expr),
                                 ivl_expr_lineno(sel_expr));
      msg[sizeof(msg)-1] = 0;

// HERE:  These first two can be fixed if the compiler is enhanced to pass
//	  the original sign information for the base expression.

	/* If the element has a MSB that is greater than or equal to the LSB
	 * and the LSB is greater than zero the compiler created a signed
	 * expression to normalize the access. This normalization will be
	 * removed, but we cannot currently determine if the base expression
	 * started out signed or not so any extra $signed() operators will
	 * need to be removed manually. */
      if ((msb >= lsb) && (lsb > 0) && (sign_type == NEED_SIGNED)) {
	    fprintf(stderr, "%s: vlog95 sorry: The translation of a select "
	                    "with MSB >= LSB > 0 is not smart enough to remove "
	                    "the extra $signed() operators.\n", msg);
	    fprintf(stderr, "%*s               Any extra $signed() operators "
	                    "will need to be removed manually.\n",
                            (int)strlen(msg), " ");
	    vlog_errors += 1;

	/* If the element is not a parameter and the LSB > MSB then the cast
	 * to signed ($signed()) from the normalization process may need to
	 * be removed. If the select expression is a constant number then
	 * this is not needed. */
      } else if ((lsb > msb) && (ivl_expr_type(sig_expr) != IVL_EX_NUMBER) &&
                 (ivl_expr_type(sel_expr) != IVL_EX_NUMBER) &&
                 (sign_type == NEED_SIGNED)) {
	    fprintf(stderr, "%s: vlog95 sorry: The translation of a select "
	                    "with LSB > MSB is not smart enough to remove "
	                    "the extra $signed() operators.\n", msg);
	    fprintf(stderr, "%*s               Any extra $signed() operators "
	                    "will need to be removed manually.\n",
                            (int)strlen(msg), " ");
	    vlog_errors += 1;

	/* Parameters are translated with normalization so for some of them
	 * the -pallowsigned=1 flag is required to get the selection
	 * expression 100% correct. */
      } else if ((! allow_signed) &&
                 (ivl_expr_type(sig_expr) == IVL_EX_NUMBER)) {
	    int pmsb, plsb;
	    ivl_parameter_t param = ivl_expr_parameter(sig_expr);
	    assert(param);
	    pmsb = ivl_parameter_msb(param);
	    plsb = ivl_parameter_lsb(param);
	    if ((pmsb >= plsb) && (plsb > 0)) {
		  fprintf(stderr, "%s: vlog95 note: Translating a MSB >= "
		                  "LSB > 0 parameter select requires the "
		                  "-pallowsigned=1 flag.\n", msg);
	    } else if (plsb > pmsb) {
		  fprintf(stderr, "%s: vlog95 note: Translating a LSB > "
		                  "MSB parameter select requires the "
		                  "-pallowsigned=1 flag.\n", msg);
	    }
      }
}

static void emit_expr_select(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_expr_t sel_expr = ivl_expr_oper2(expr);
      ivl_expr_t sig_expr = ivl_expr_oper1(expr);
      ivl_select_type_t sel_type = ivl_expr_sel_type(expr);
      (void)wid;  /* Parameter is not used. */
	/* If this is a dynamic array or queue select, translate the
	 * select differently. */
      if ((ivl_expr_type(sig_expr) == IVL_EX_SIGNAL)  &&
          ((ivl_signal_data_type(ivl_expr_signal(sig_expr)) == IVL_VT_DARRAY) ||
           (ivl_signal_data_type(ivl_expr_signal(sig_expr)) == IVL_VT_QUEUE))) {
	    assert(sel_expr);
	    emit_select_name(scope, sig_expr);
	    fprintf(vlog_out, "[");
	    emit_expr(scope, sel_expr, 0, 0, 0, 1);
	    fprintf(vlog_out, "]");
	    return;
      }
      if (sel_expr) {
	    unsigned width = ivl_expr_width(expr);
	    ivl_expr_type_t type = ivl_expr_type(sig_expr);
	    assert(width > 0);
	      /* The compiler uses selects for some shifts. */
	    if (type != IVL_EX_NUMBER && type != IVL_EX_SIGNAL) {
		  fprintf(vlog_out, "(" );
		  emit_select_name(scope, sig_expr);
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
			get_sig_msb_lsb(ivl_expr_signal(sig_expr), &msb, &lsb);
		  }
		    /* A bit select. */
		  if (width == 1) {
			check_select_signed(sig_expr, sel_expr, msb, lsb);
			emit_select_name(scope, sig_expr);
			fprintf(vlog_out, "[");
			emit_scaled_expr(scope, sel_expr, msb, lsb);
			fprintf(vlog_out, "]");
		  } else if (ivl_expr_type(sel_expr) == IVL_EX_NUMBER) {
			  /* A constant part select. */
			emit_select_name(scope, sig_expr);
			emit_scaled_range(scope, sel_expr, width, msb, lsb);
		  } else if (sel_type == IVL_SEL_OTHER) {
			  /* A packed array access. */
			assert(lsb == 0);
			assert(msb >= 0);
			emit_expr_packed(scope, sig_expr, sel_expr, width);
		  } else {
			  /* An indexed part select. */
			check_select_signed(sig_expr, sel_expr, msb, lsb);
			emit_expr_ips(scope, sig_expr, sel_expr, sel_type,
			              width, msb, lsb, type == IVL_EX_NUMBER);
		  }
	    }
      } else {
// HERE: Should this sign extend if the expression is signed?
	    unsigned width = ivl_expr_width(expr);
	    unsigned sig_wid = ivl_expr_width(sig_expr);
	    emit_expr(scope, sig_expr, sig_wid, 0, 0, 0);
	      /* Select part of a signal when needed. */
	    if ((ivl_expr_type(sig_expr) == IVL_EX_SIGNAL) &&
	        (width < sig_wid)) {
		  int msb, lsb;
		  int64_t value;
		  get_sig_msb_lsb(ivl_expr_signal(sig_expr), &msb, &lsb);
		  value = lsb;
		  if (msb >= lsb) value += width - 1;
		  else value -= width - 1;
		  fprintf(vlog_out, "[%"PRId64":%u]", value, lsb);
	    }
      }
}

/*
 * This routine is used to emit both system and user functions.
 */
static void emit_expr_func(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      unsigned count = ivl_expr_parms(expr);
      (void)wid;  /* Parameter is not used. */
      if (count) {
	    unsigned idx;
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
// HERE: For a user function should the argument width be used here.
		  emit_expr(scope, ivl_expr_parm(expr, idx), 0, 1, 0, 0);
		  fprintf(vlog_out, ", ");
	    }
// HERE: For a user function should the argument width be used here.
	    emit_expr(scope, ivl_expr_parm(expr, count), 0, 1, 0, 0);
	    fprintf(vlog_out, ")");
	/* User functions without arguments are not supported so a dummy
	 * argument is added both here and in the definition. */
      } else if (ivl_expr_type(expr) == IVL_EX_UFUNC) {
	    fprintf(vlog_out, "(1'bx)");
      }
}

static void emit_expr_signal(ivl_scope_t scope, ivl_expr_t expr, unsigned wid)
{
      ivl_signal_t sig = ivl_expr_signal(expr);
      (void)wid;  /* Parameter is not used. */
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

static void emit_expr_ternary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid,
                              unsigned is_full_prec)
{
      ivl_expr_t oper2 = ivl_expr_oper2(expr);
      ivl_expr_t oper3 = ivl_expr_oper3(expr);
      unsigned can_skip_unsigned = calc_can_skip_unsigned(oper2, oper3);
      fprintf(vlog_out, "(");
      emit_expr(scope, ivl_expr_oper1(expr), 0, 0, 0, 0);
      fprintf(vlog_out, " ? ");
// HERE: Do these two emits need to use get_cast_width() like the binary
//       arithmetic operators?
      emit_expr(scope, oper2, wid, 0, can_skip_unsigned, is_full_prec);
      fprintf(vlog_out, " : ");
      emit_expr(scope, oper3, wid, 0, can_skip_unsigned, is_full_prec);
      fprintf(vlog_out, ")");
}

static void emit_expr_unary(ivl_scope_t scope, ivl_expr_t expr, unsigned wid,
                            unsigned is_full_prec)
{
      const char *oper = "invalid";
      ivl_expr_t oper1 = ivl_expr_oper1(expr);
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
	    fprintf(vlog_out, "(%s", oper);
	    emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
	    fprintf(vlog_out, ")");
	    break;
	case '&':
	case '|':
	case '^':
	case 'A':
	case 'N':
	case 'X':
	case '!':
	    fprintf(vlog_out, "(%s", oper);
	    emit_expr(scope, oper1,  0, 0, 0, 0);
	    fprintf(vlog_out, ")");
	    break;
	case '2':
	case 'v':
	case 'r':
	    /* A cast is a noop. */
	    emit_expr(scope, oper1,  0, 0, 0, 0);
	    break;
	case 'I':
	    fprintf(vlog_out, "(++");
	    emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 sorry: Pre-increment "
	                    "operator is not currently translated.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case 'i':
	    fprintf(vlog_out, "(");
	    emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
	    fprintf(vlog_out, "++)");
	    fprintf(stderr, "%s:%u: vlog95 sorry: Post-increment "
	                    "operator is not currently translated.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case 'D':
	    fprintf(vlog_out, "(--");
	    emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
	    fprintf(vlog_out, ")");
	    fprintf(stderr, "%s:%u: vlog95 sorry: Pre-decrement "
	                    "operator is not currently translated.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case 'd':
	    fprintf(vlog_out, "(");
	    emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
	    fprintf(vlog_out, "--)");
	    fprintf(stderr, "%s:%u: vlog95 sorry: Post-decrement "
	                    "operator is not currently translated.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	  /* Convert the Verilog-A abs() function. */
	case 'm':
	    if (ivl_expr_value(expr) == IVL_VT_REAL) {
		    /* For a real expression use the $abs() function. */
		  fprintf(vlog_out, "$abs(");
		  emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
		  fprintf(vlog_out, ")");
	    } else {
		    /* This only works when the argument has no side effect. */
		  fprintf(vlog_out, "((");
		  emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
		  fprintf(vlog_out, ") > 0 ? (");
		  emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
		  fprintf(vlog_out, ") : -(");
		  emit_expr(scope, oper1,  wid, 0, 0, is_full_prec);
		  fprintf(vlog_out, "))");
	    }
	    break;
	default:
	    fprintf(vlog_out, "<unknown>");
	    emit_expr(scope, oper1,  wid, 0, 0, 0);
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown unary "
	                    "operator (%c).\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr),
	                    ivl_expr_opcode(expr));
	    vlog_errors += 1;
	    break;
      }
}

void emit_expr(ivl_scope_t scope, ivl_expr_t expr, unsigned wid,
               unsigned is_lval_width, unsigned can_skip_unsigned,
               unsigned is_full_prec)
{
      expr_sign_t sign_type;
	/* If the width is from an L-value (assignment) then the actual
	 * expression width can be larger. */
      if (is_lval_width) {
	    unsigned expr_wid = ivl_expr_width(expr);
	    if (wid < expr_wid) wid = expr_wid;
      }
	/* In a self-determined context the expression set the width. */
      if (! wid) wid = ivl_expr_width(expr);
      sign_type = expr_get_sign_type(expr, wid, can_skip_unsigned,
                                     is_full_prec);

	/* Check to see if a $signed() or $unsigned() needs to be emitted
	 * before the expression. */
      if (sign_type == NEED_SIGNED) {
	    fprintf(vlog_out, "$signed(");
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: $signed() is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	      /* A $signed() creates a self-determined context. */
	    wid = 0;
	      /* It also clears the full precision flag. */
	    is_full_prec = 0;
      }
      if (sign_type == NEED_UNSIGNED) {
	    fprintf(vlog_out, "$unsigned(");
	    if (! allow_signed) {
		  fprintf(stderr, "%s:%u: vlog95 error: $unsigned() is not "
		                  "supported.\n",
		                  ivl_expr_file(expr), ivl_expr_lineno(expr));
		  vlog_errors += 1;
	    }
	      /* A $unsigned() creates a self-determined context. */
	    wid = 0;
	      /* It also clears the full precision flag. */
	    is_full_prec = 0;
      }
	/* Emit the expression. */
      switch (ivl_expr_type(expr)) {
	case IVL_EX_ARRAY:
	    emit_expr_array(scope, expr, wid);
	    break;
	case IVL_EX_ARRAY_PATTERN:
	    fprintf(vlog_out, "<array pattern>");
	    fprintf(stderr, "%s:%u: vlog95 error: Array pattern expressions "
	                    "are not supported.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case IVL_EX_BINARY:
	    emit_expr_binary(scope, expr, wid, is_full_prec);
	    break;
	case IVL_EX_CONCAT:
	    emit_expr_concat(scope, expr, wid);
	    break;
	case IVL_EX_DELAY:
	    emit_expr_delay(scope, expr, wid);
	    break;
	case IVL_EX_ENUMTYPE:
	    fprintf(vlog_out, "<enum>");
	    fprintf(stderr, "%s:%u: vlog95 error: Enum expressions "
	                    "are not supported.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case IVL_EX_EVENT:
	    emit_expr_event(scope, expr, wid);
	    break;
	case IVL_EX_NEW:
	    fprintf(vlog_out, "<new>");
	    fprintf(stderr, "%s:%u: vlog95 error: New operator "
	                    "is not supported.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case IVL_EX_NULL:
	    fprintf(vlog_out, "<null>");
	    fprintf(stderr, "%s:%u: vlog95 error: Null operator "
	                    "is not supported.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case IVL_EX_NUMBER:
	    emit_expr_number(scope, expr, wid);
	    break;
	case IVL_EX_PROPERTY:
	    emit_class_property(scope, expr, wid);
	    break;
	case IVL_EX_REALNUM:
	    emit_expr_real_number(scope, expr, wid);
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
	case IVL_EX_SHALLOWCOPY:
	    fprintf(vlog_out, "<new> ");
	    emit_expr(scope, ivl_expr_oper2(expr), wid, 0, 0, 0);
	    fprintf(stderr, "%s:%u: vlog95 error: New operator "
	                    "is not supported.\n",
	                    ivl_expr_file(expr),
	                    ivl_expr_lineno(expr));
	    vlog_errors += 1;
	    break;
	case IVL_EX_SIGNAL:
	    emit_expr_signal(scope, expr, wid);
	    break;
	case IVL_EX_STRING:
	    emit_string(ivl_expr_string(expr));
	    break;
	case IVL_EX_TERNARY:
	    emit_expr_ternary(scope, expr, wid, is_full_prec);
	    break;
	case IVL_EX_UFUNC:
	    emit_scope_path(scope, ivl_expr_def(expr));
	    emit_expr_func(scope, expr, wid);
	    break;
	case IVL_EX_UNARY:
	    emit_expr_unary(scope, expr, wid, is_full_prec);
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
	/* Close the $signed() or $unsigned() if need. */
      if (sign_type != NO_SIGN) fprintf(vlog_out, ")");
}
