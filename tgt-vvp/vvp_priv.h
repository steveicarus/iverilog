#ifndef __vvp_priv_H
#define __vvp_priv_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvp_priv.h,v 1.18 2002/08/12 01:35:04 steve Exp $"
#endif

# include  "ivl_target.h"
# include  <stdio.h>

/*
 * The target_design entry opens the output file that receives the
 * compiled design, and sets the vvp_out to the descripter.
 */
extern FILE* vvp_out;

/*
 * Mangle all non-symbol characters in an identifier, quotes in names
 */
extern const char *vvp_mangle_id(const char *);
extern const char *vvp_mangle_name(const char *);

/*
 * This generates a string from a signal that uniquely identifies
 * that signal with letters that can be used in a label.
 */
extern const char* vvp_signal_label(ivl_signal_t sig);

/*
 * This generates a label string for a memory.
 */
extern const char* vvp_memory_label(ivl_memory_t mem);

/*
 * This function draws a process (initial or always) into the output
 * file. It normally returns 0, but returns !0 of there is some sort
 * of error.
 */
extern int draw_process(ivl_process_t net, void*x);

extern int draw_task_definition(ivl_scope_t scope);
extern int draw_func_definition(ivl_scope_t scope);

extern int draw_scope(ivl_scope_t scope, ivl_scope_t parent);

extern void draw_lpm_mux(ivl_lpm_t net);

/*
 * Given a nexus, draw a string that represents the functor output
 * that feeds the nexus. This function can be used to get the input to
 * a functor, event, or even a %load in cases where I have the
 * ivl_nexus_t object.
 */
extern void draw_nexus_input(ivl_nexus_t nex);

extern const char* draw_net_input(ivl_nexus_t nex);

extern void draw_input_from_net(ivl_nexus_t nex);

/*
 * The draw_eval_expr function writes out the code to evaluate a
 * behavioral expression.
 *
 * Expression results are placed into a vector allocated in the bit
 * space of the thread. The vector_info structure represents that
 * allocation. When the caller is done with the bits, it must release
 * the vector with clr_vector so that the code generator can reuse
 * those bits.
 */
struct vector_info {
      unsigned short base;
      unsigned short wid;
};

extern struct vector_info draw_eval_expr(ivl_expr_t exp);
extern struct vector_info draw_eval_expr_wid(ivl_expr_t exp, unsigned w);
extern void draw_memory_index_expr(ivl_memory_t mem, ivl_expr_t exp);

extern unsigned short allocate_vector(unsigned short wid);
extern void clr_vector(struct vector_info vec);

extern int number_is_unknown(ivl_expr_t ex);
extern int number_is_immediate(ivl_expr_t ex, unsigned lim_wid);
extern unsigned long get_number_immediate(ivl_expr_t ex);

/*
 * These are used to count labels as I generate code.
 */
extern unsigned local_count;
extern unsigned thread_count;

/*
 * $Log: vvp_priv.h,v $
 * Revision 1.18  2002/08/12 01:35:04  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.17  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.16  2002/08/03 22:30:48  steve
 *  Eliminate use of ivl_signal_name for signal labels.
 *
 * Revision 1.15  2002/07/08 04:04:07  steve
 *  Generate code for wide muxes.
 *
 * Revision 1.14  2002/06/02 18:57:17  steve
 *  Generate %cmpi/u where appropriate.
 *
 * Revision 1.13  2002/04/22 02:41:30  steve
 *  Reduce the while loop expression if needed.
 *
 * Revision 1.12  2001/11/01 04:26:57  steve
 *  Generate code for deassign and cassign.
 *
 * Revision 1.11  2001/06/18 03:10:34  steve
 *   1. Logic with more than 4 inputs
 *   2. Id and name mangling
 *   3. A memory leak in draw_net_in_scope()
 *   (Stephan Boettcher)
 *
 * Revision 1.10  2001/05/17 04:37:02  steve
 *  Behavioral ternary operators for vvp.
 *
 * Revision 1.9  2001/05/06 17:54:33  steve
 *  Behavioral code to read memories. (Stephan Boettcher)
 *
 * Revision 1.8  2001/04/06 02:28:03  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.7  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.6  2001/03/31 17:36:39  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.5  2001/03/27 06:27:41  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.4  2001/03/27 03:31:07  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.3  2001/03/22 05:06:21  steve
 *  Geneate code for conditional statements.
 *
 * Revision 1.2  2001/03/21 01:49:43  steve
 *  Scan the scopes of a design, and draw behavioral
 *  blocking  assignments of constants to vectors.
 *
 * Revision 1.1  2001/03/19 01:20:46  steve
 *  Add the tgt-vvp code generator target.
 *
 */
#endif
