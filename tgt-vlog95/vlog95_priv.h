#ifndef __vlog95_priv_H
#define __vlog95_priv_H
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

# include "config.h"
# include "ivl_target.h"
# include <inttypes.h>
# include <stdio.h>
# include <assert.h>

/*
 * This is the file that the converted design is written to.
 */
extern FILE*vlog_out;

/*
 * Keep a count of the fatal errors that happen during code generation.
 */
extern int vlog_errors;

/*
 * Keep the simulation time precision so that we can scale delays.
 */
extern int sim_precision;

/*
 * Keep the current indent level.
 */
extern unsigned indent;
extern unsigned indent_incr;

/*
 * Emit the Verilog code for the given scope.
 */
extern int emit_scope(ivl_scope_t scope, ivl_scope_t parent);

/*
 * Emit a Verilog statement.
 */
extern void emit_stmt(ivl_scope_t scope, ivl_statement_t stmt);

/*
 * Emit a Verilog expression.
 */
extern void emit_expr(ivl_scope_t scope, ivl_expr_t expr, unsigned width);

/*
 * Emit a delay scaled to the current timescale (units and precision).
 */
extern void emit_scaled_delay(ivl_scope_t scope, uint64_t delay);

/*
 * Cleanup functions.
 */
extern void free_emitted_scope_list();

#endif /* __vlog95_priv_H */
