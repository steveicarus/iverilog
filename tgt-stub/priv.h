/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  <ivl_target.h>
# include  <stdio.h>

/*
 * This is the output file where the generated result should be
 * written.
 */
extern FILE*out;

/*
 * Keep a running count of errors that the stub detects. This will be
 * the error count returned to the ivl_target environment.
 */
extern int stub_errors;

/*
 * This function finds the vector width of a signal. It relies on the
 * assumption that all the signal inputs to the nexus have the same
 * width. The ivl_target API should assert that condition.
 */
extern unsigned width_of_nexus(ivl_nexus_t nex);

extern ivl_variable_type_t type_of_nexus(ivl_nexus_t nex);

extern ivl_discipline_t discipline_of_nexus(ivl_nexus_t nex);

/*
 * Show the details of the expression.
 */
extern void show_expression(ivl_expr_t net, unsigned ind);

/*
 * Show the statement.
 */
extern void show_statement(ivl_statement_t net, unsigned ind);

extern void show_switch(ivl_switch_t net);

/*
*/
extern const char*data_type_string(ivl_variable_type_t vtype);
