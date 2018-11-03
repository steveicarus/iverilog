#ifndef IVL_priv_H
#define IVL_priv_H
/*
 * Copyright (c) 2013-2014 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "ivl_target.h"
# include  <cstdio>

/*
 * Errors are counted here. When the blif processing is done, this
 * value is returned to ivl so that it can report error counts.
 */
extern int blif_errors;

extern int print_logic_gate(FILE*fd, ivl_net_logic_t net);

extern int print_lpm(FILE*fd, ivl_lpm_t net);
extern int print_lpm_add(FILE*fd, ivl_lpm_t net);
extern int print_lpm_ff(FILE*fd, ivl_lpm_t net);
extern int print_lpm_sub(FILE*fd, ivl_lpm_t net);
extern int print_lpm_cmp_eq(FILE*fd, ivl_lpm_t net);
extern int print_lpm_cmp_gt(FILE*fd, ivl_lpm_t net);
extern int print_lpm_cmp_ne(FILE*fd, ivl_lpm_t net);
extern int print_lpm_mux(FILE*fd, ivl_lpm_t net);
extern int print_lpm_part_vp(FILE*fd, ivl_lpm_t net);
extern int print_lpm_re_logic(FILE*fd, ivl_lpm_t net);
extern int print_lpm_shift(FILE*fd, ivl_lpm_t net, bool left);
extern int print_lpm_sign_ext(FILE*fd, ivl_lpm_t net);

/*
 * Emit all the constants for a model. This works by scanning the
 * design for all constants, testing that they are part of the model,
 * and writing them out as .names records.
 */
extern void emit_constants(FILE*fd, ivl_design_t des, ivl_scope_t model);

/*
 * Return true if the passed scope is under the model scope, at any
 * depth. The scope may be an immediate child, or a child several
 * levels removed.
 */
extern bool scope_is_in_model(ivl_scope_t model, ivl_scope_t scope);

#endif /* IVL_priv_H */
