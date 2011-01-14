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
 *
 *
 * This is the vlog95 target module. It generates a 1364-1995 compliant
 * netlist from the input netlist. The generated netlist is expected to
 * be simulation equivalent to the original.
 */

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
// HERE: No support for event based delays/
      if (nevents) {
	    assert(delay == 0);
	    assert(0);
      }
      if (delay) {
	    assert(nevents == 0);
	    fprintf(vlog_out, "#");
	    emit_expr(scope, delay, 0);
	    fprintf(vlog_out, " ");
      }
}

static void emit_stmt_lval_piece(ivl_lval_t lval)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      fprintf(vlog_out, "%s", ivl_signal_basename(sig));
// HERE: Need support for bit, part or array word.
// ivl_lval_width and ivl_lval_part_off is used for part select.
// If the lval width is less than the signal width this is a zero based PS.
// ivl_lval_idx is used for an array select. Handle non-zero based and
// different endian accesses.
}

static unsigned emit_stmt_lval(ivl_statement_t stmt)
{
      unsigned idx;
      unsigned count = ivl_stmt_lvals(stmt);
      unsigned wid = 0;
      if (count > 1) {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid += ivl_lval_width(lval);
	    fprintf(vlog_out, "{");
	    emit_stmt_lval_piece(lval);
	    for (idx = 1; idx < count; idx += 1) {
		  fprintf(vlog_out, ", ");
		  lval = ivl_stmt_lval(stmt, idx);
		  wid += ivl_lval_width(lval);
		  emit_stmt_lval_piece(lval);
	    }
	    fprintf(vlog_out, "}");
      } else {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid = ivl_lval_width(lval);
	    emit_stmt_lval_piece(lval);
      }
      return wid;
}

static void emit_stmt_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*c", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(stmt);
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
      wid = emit_stmt_lval(stmt);
      fprintf(vlog_out, " <= ");
      emit_stmt_inter_delay(scope, stmt);
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

void emit_stmt_block(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*cbegin\n", get_indent(), ' ');
      emit_stmt_block_body(scope, stmt);
      fprintf(vlog_out, "%*cend\n", get_indent(), ' ');
}

void emit_stmt_block_named(ivl_scope_t scope, ivl_statement_t stmt)
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
      wid = emit_stmt_lval(stmt);
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
      (void) emit_stmt_lval(stmt);
      fprintf(vlog_out, ";\n");
}

static void emit_stmt_delay(ivl_scope_t scope, ivl_statement_t stmt)
{
      fprintf(vlog_out, "%*c#", get_indent(), ' ');
      emit_scaled_delay(scope, ivl_stmt_delay_val(stmt));
      single_indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
}

static void emit_stmt_force(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned wid;
      fprintf(vlog_out, "%*cforce ", get_indent(), ' ');
// HERE: Do we need to calculate the width? The compiler should have already
//       done this for us.
      wid = emit_stmt_lval(stmt);
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
      (void) emit_stmt_lval(stmt);
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
      unsigned idx, count = ivl_stmt_parm_count(stmt);
      fprintf(vlog_out, "%*c%s", get_indent(), ' ', ivl_stmt_name(stmt));
      if (count != 0) {
	    fprintf(vlog_out, "(");
	    count -= 1;
	    for (idx = 0; idx < count; idx += 1) {
		  emit_expr(scope, ivl_stmt_parm(stmt, idx), 0);
		  fprintf(vlog_out, ", ");
	    }
	    emit_expr(scope, ivl_stmt_parm(stmt, count), 0);
	    fprintf(vlog_out, ")");
      }
      fprintf(vlog_out, ";\n");
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
	case IVL_ST_RELEASE:
	    emit_stmt_release(scope, stmt);
	    break;
	case IVL_ST_REPEAT:
	    emit_stmt_repeat(scope, stmt);
	    break;
	case IVL_ST_STASK:
	    emit_stmt_stask(scope, stmt);
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
