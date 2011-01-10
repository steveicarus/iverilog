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

# include <inttypes.h>
# include <stdlib.h>
# include "config.h"
# include "vlog95_priv.h"

static unsigned indent_save = 0;

static void emit_stmt_lval(ivl_lval_t lval)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      fprintf(vlog_out, "%s", ivl_signal_basename(sig));
// HERE: Need support for bit, part or array word.
// ivl_lval_width and ivl_lval_part_off is used for part select.
// If the lval width is less than the signal width this is a zero based PS.
// ivl_lval_idx is used for an array select.
}

static void emit_stmt_assign(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx;
      unsigned count = ivl_stmt_lvals(stmt);
      unsigned wid = 0;
      fprintf(vlog_out, "%*c", indent, ' ');
      if (count > 1) {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid += ivl_lval_width(lval);
	    fprintf(vlog_out, "{");
	    emit_stmt_lval(lval);
	    for (idx = 1; idx < count; idx += 1) {
		  fprintf(vlog_out, ", ");
		  lval = ivl_stmt_lval(stmt, idx);
		  wid += ivl_lval_width(lval);
		  emit_stmt_lval(lval);
	    }
	    fprintf(vlog_out, "}");
      } else {
	    ivl_lval_t lval = ivl_stmt_lval(stmt, 0);
	    wid = ivl_lval_width(lval);
	    emit_stmt_lval(lval);
      }
      fprintf(vlog_out, " = ");
      emit_expr(scope, ivl_stmt_rval(stmt), wid);
      fprintf(vlog_out, ";\n");
}

void emit_stmt_block(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count = ivl_stmt_block_count(stmt);
      fprintf(vlog_out, "%*cbegin\n", indent, ' ');
      indent += indent_incr;
      for (idx = 0; idx < count; idx += 1) {
	    emit_stmt(scope, ivl_stmt_block_stmt(stmt, idx));
      }
      assert(indent >= indent_incr);
      indent -= indent_incr;
      fprintf(vlog_out, "%*cend\n", indent, ' ');
}

void emit_stmt_block_named(ivl_scope_t scope, ivl_statement_t stmt)
{
      ivl_scope_t my_scope = ivl_stmt_block_scope(stmt);
      unsigned idx, count = ivl_stmt_block_count(stmt);
      fprintf(vlog_out, "%*cbegin: %s\n", indent, ' ',
                        ivl_scope_basename(my_scope));
      indent += indent_incr;
      for (idx = 0; idx < count; idx += 1) {
	    emit_stmt(scope, ivl_stmt_block_stmt(stmt, idx));
      }
      assert(indent >= indent_incr);
      indent -= indent_incr;
      fprintf(vlog_out, "%*cend /* %s */\n", indent, ' ',
                        ivl_scope_basename(my_scope));
}

static void emit_stmt_delay(ivl_scope_t scope, ivl_statement_t stmt)
{
      uint64_t delay = ivl_stmt_delay_val(stmt);
      int scale = ivl_scope_time_units(scope) - sim_precision;
      int pre = ivl_scope_time_units(scope) - ivl_scope_time_precision(scope);
      char *frac;
      uint8_t real_dly = 0;
      assert(scale >= 0);
      assert(pre >= 0);
      assert(scale >= pre);
      frac = (char *)malloc(pre+1);
      frac[pre] = 0;
      for (/* none */; scale > 0; scale -= 1) {
	    if (scale > pre) {
		  assert((delay % 10) == 0);
	    } else {
		  frac[scale-1] = (delay % 10) + '0';
		  if (frac[scale-1] != '0') {
			real_dly = 1;
		  } else if (!real_dly) {
			frac[scale-1] = 0;
		  }
	    }
	    delay /= 10;
      }
      fprintf(vlog_out, "%*c#%"PRIu64, indent, ' ', delay);
      if (real_dly) {
	    fprintf(vlog_out, ".%s", frac);
      }
      free(frac);
      indent_save = indent;
      indent = 1;
      emit_stmt(scope, ivl_stmt_sub_stmt(stmt));
      indent = indent_save;
}

static void emit_stmt_stask(ivl_scope_t scope, ivl_statement_t stmt)
{
      unsigned idx, count = ivl_stmt_parm_count(stmt);
      fprintf(vlog_out, "%*c%s", indent, ' ', ivl_stmt_name(stmt));
      if (count != 0) {
	    fprintf(vlog_out, "(");
	    for (idx = 0; idx < count; idx += 1) {
		  emit_expr(scope, ivl_stmt_parm(stmt, idx), 0);
	    }
	    fprintf(vlog_out, ")");
      }
      fprintf(vlog_out, ";\n");
}

void emit_stmt(ivl_scope_t scope, ivl_statement_t stmt)
{
      switch(ivl_statement_type(stmt)) {
	case IVL_ST_NOOP:
	    fprintf(vlog_out, ";\n");
	    break;
	case IVL_ST_ASSIGN:
	    emit_stmt_assign(scope, stmt);
	    break;
	case IVL_ST_BLOCK:
	    if (ivl_stmt_block_scope(stmt)) {
		  emit_stmt_block_named(scope, stmt);
	    } else {
		  emit_stmt_block(scope, stmt);
	    }
	    break;
	case IVL_ST_DELAY:
	    emit_stmt_delay(scope, stmt);
	    break;
	case IVL_ST_STASK:
	    emit_stmt_stask(scope, stmt);
	    break;
	default:
	    fprintf(vlog_out, "%*c<unknown>;\n", indent, ' ');
	    fprintf(stderr, "%s:%u: vlog95 error: Unknown statement "
	                    "type (%d).\n",
	                    ivl_stmt_file(stmt),
	                    ivl_stmt_lineno(stmt),
	                    (int)ivl_statement_type(stmt));
	    vlog_errors += 1;
	    break;
      }
}
