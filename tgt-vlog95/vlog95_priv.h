#ifndef IVL_vlog95_priv_H
#define IVL_vlog95_priv_H
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

# include "config.h"
# include "ivl_target.h"
# include <inttypes.h>
# include <stdio.h>
# include <assert.h>

/*
 * The design we are emitting.
 */
extern ivl_design_t design;

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
 * The expression code needs to know when a parameter definition is being
 * emitted so it can emit the numeric value instead of the name.
 */
extern ivl_parameter_t emitting_param;

/*
 * The statement code needs to know what name to use for a translated
 * function return statement (disable).
 */
extern const char *func_rtn_name;

/*
 * Keep the current indent level.
 */
extern unsigned indent;
extern unsigned indent_incr;

/*
 * Set to non-zero when the user wants to emit all the file and line
 * number information.
 */
extern unsigned emit_file_line;

/*
 * Some tools that are mostly 1364-1995 compliant also support signed so
 * add support for that as an extension.
 */
extern unsigned allow_signed;

/*
 * Emit various Verilog types.
 */
extern void emit_event(ivl_scope_t scope, ivl_statement_t stmt);
extern void emit_expr(ivl_scope_t scope, ivl_expr_t expr, unsigned width,
                      unsigned is_lval_width, unsigned can_skip_unsigned,
                      unsigned is_full_prec);
extern void emit_logic(ivl_scope_t scope, ivl_net_logic_t nlogic);
extern void emit_lpm(ivl_scope_t scope, ivl_lpm_t lpm);
extern void emit_process(ivl_scope_t scope, ivl_process_t proc);
extern int emit_scope(ivl_scope_t scope, ivl_scope_t parent);
extern void emit_stmt(ivl_scope_t scope, ivl_statement_t stmt);
extern void emit_tran(ivl_scope_t scope, ivl_switch_t tran);

extern void emit_scaled_delay(ivl_scope_t scope, uint64_t delay);
extern void emit_scaled_delayx(ivl_scope_t scope, ivl_expr_t expr,
                               unsigned is_stmt);
extern void emit_scaled_expr(ivl_scope_t scope, ivl_expr_t expr,
                             int msb, int lsb);
extern void emit_scaled_range(ivl_scope_t scope, ivl_expr_t expr,
                              unsigned width, int msb, int lsb);
extern void emit_scope_path(ivl_scope_t scope, ivl_scope_t call_scope);
extern void emit_scope_variables(ivl_scope_t scope);
extern void emit_scope_call_path(ivl_scope_t scope, ivl_scope_t call_scope);
extern void emit_scope_module_path(ivl_scope_t scope, ivl_scope_t call_scope);
extern void emit_name_of_nexus(ivl_scope_t scope, ivl_nexus_t nex,
                               unsigned allow_UD);
extern void emit_nexus_as_ca(ivl_scope_t scope, ivl_nexus_t nex,
                             unsigned allow_UD, unsigned sign_extend);
extern void emit_nexus_port_driver_as_ca(ivl_scope_t scope, ivl_nexus_t nex);
extern void emit_const_nexus(ivl_scope_t scope, ivl_net_const_t const_net);
extern void emit_signal_net_const_as_ca(ivl_scope_t scope, ivl_signal_t sig,
                                        ivl_nexus_ptr_t nex_ptr);

extern void add_udp_to_list(ivl_udp_t udp);
extern void emit_udp_list(void);
extern void emit_sig_file_line(ivl_signal_t sig);

extern void emit_id(const char *id);
extern void emit_real_number(double value);
extern void emit_number(const char *bits, unsigned nbits, unsigned is_signed,
                        const char *file, unsigned lineno);
extern void emit_string(const char *string);

extern void emit_icarus_generated_udps(void);
extern void emit_icarus_generated_top_module(void);

/*
 * Find the enclosing module scope.
 */
extern ivl_scope_t get_module_scope(ivl_scope_t scope);

/*
 * Get an int32_t/uint64_t from a number is possible. The return type is
 * 0 for a valid value, negative for a number with undefined bits and
 * positive it the value is too large. The positive value is the minimum
 * number of bits required to represent the value.
 */
extern int32_t get_int32_from_number(ivl_expr_t expr, int *return_type);
extern int64_t get_int64_from_number(ivl_expr_t expr, int *return_type);
extern uint64_t get_uint64_from_number(ivl_expr_t expr, int *return_type);

/*
 * A package is translated to a module with a special name. This routine
 * does that translation. To avoid a memory leak the calling routine must
 * use free() to cleanup the string returned.
 */
extern char * get_package_name(ivl_scope_t scope);

/*
 * Get the appropriate MSB and LSB for a signal.
 */
extern void get_sig_msb_lsb(ivl_signal_t sig, int *msb, int *lsb);

/*
 * Convert a timescale value to a string.
 */
extern const char*get_time_const(int time_value);

/*
 * Cleanup functions.
 */
extern void free_emitted_scope_list(void);

/*
 * Debug routine to dump the various pieces of nexus information.
*/
extern void dump_nexus_information(ivl_scope_t scope, ivl_nexus_t nex);

static inline unsigned is_void_function(ivl_scope_t scope)
{
      return ivl_scope_type(scope) == IVL_SCT_FUNCTION
          && ivl_scope_port(scope, 0) == 0;
}

#endif /* IVL_vlog95_priv_H */
