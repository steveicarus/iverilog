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
#if !defined(WINNT)
#ident "$Id: vvp_priv.h,v 1.7 2001/04/02 02:28:13 steve Exp $"
#endif

# include  "ivl_target.h"
# include  <stdio.h>

/*
 * The target_design entry opens the output file that receives the
 * compiled design, and sets the vvp_out to the descripter.
 */
extern FILE* vvp_out;

/*
 * This function draws a process (initial or always) into the output
 * file. It normally returns 0, but returns !0 of there is some sort
 * of error.
 */
extern int draw_process(ivl_process_t net, void*x);

extern int draw_task_definition(ivl_scope_t scope);

extern int draw_scope(ivl_scope_t scope, ivl_scope_t parent);

/*
 * Given a nexus, draw a string that represents the functor output
 * that feeds the nexus. This function can be used to get the input to
 * a functor, event, or even a %load in cases where I have the
 * ivl_nexus_t object.
 */
extern void draw_nexus_input(ivl_nexus_t nex);


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

extern void clr_vector(struct vector_info vec);

/*
 * $Log: vvp_priv.h,v $
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
