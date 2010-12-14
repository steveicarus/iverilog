#ifndef __vvp_priv_H
#define __vvp_priv_H
/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include  "vvp_config.h"
# include  "ivl_target.h"
# include  <stdio.h>

/*
 * The target_design entry opens the output file that receives the
 * compiled design, and sets the vvp_out to the descriptor.
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

extern const char* vvp_word_label(ivl_variable_t var);

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
 * This function draws the execution of a vpi_call statement, along
 * with the tricky handling of arguments. If this is called with a
 * statement handle, it will generate a %vpi_call
 * instruction. Otherwise, it will generate a %vpi_func instruction.
 */
extern void draw_vpi_task_call(ivl_statement_t net);

extern struct vector_info draw_vpi_func_call(ivl_expr_t exp,
					     unsigned wid);
extern int draw_vpi_rfunc_call(ivl_expr_t exp);

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
 *
 * The stuff_ok_flag is normally empty. Bits in the bitmask are set
 * true in cases where certain special situations are allows. This
 * might allow deeper expressions to make assumptions about the
 * caller.
 *
 *   STUFF_OK_XZ -- This bit is set if the code processing the result
 *        doesn't distinguish between x and z values.
 *
 *   STUFF_OK_47 -- This bit is set if the node is allowed to leave a
 *        result in any of the 4-7 vthread bits.
 */
struct vector_info {
      unsigned base;
      unsigned wid;
};

extern struct vector_info draw_eval_expr(ivl_expr_t exp, int stuff_ok_flag);
extern struct vector_info draw_eval_expr_wid(ivl_expr_t exp, unsigned w,
					     int stuff_ok_flag);
#define STUFF_OK_XZ 0x0001
#define STUFF_OK_47 0x0002

/*
 * This function draws code to evaluate the index expression exp for
 * the memory mem. The result is loaded into index register i3, and
 * the flag bit 4 is set to 0 if the numerical value is defined, or 1
 * if not.
 */
extern void draw_memory_index_expr(ivl_memory_t mem, ivl_expr_t exp);

/*
 * These functions manage vector allocation in the thread register
 * space. They presume that we work on one thread at a time, to
 * completion.
 *
 *  allocate_vector
 *    Return the base of an allocated vector in the thread. The bits
 *    are marked allocated in the process.
 *
 *  clr_bector
 *    Clear a vector previously allocated.
 *
 * The thread vector allocator also keeps a lookaside of expression
 * results that are stored in register bit. This lookaside can be used
 * by the code generator to notice that certain expression bits are
 * already calculated, and can be reused.
 *
 *  clear_expression_lookaside
 *    Clear the lookaside tables for the current thread.
 *
 *  save_expression_lookaside
 *    Mark the given expression as available in the given register
 *    bits. This remains until the lookaside is cleared.
 *
 *  allocate_vector_exp
 *    This function attempts to locate the expression in the
 *    lookaside. If it finds it, return a reallocated base for the
 *    expression. Otherwise, return 0.
 */
extern unsigned allocate_vector(unsigned wid);
extern void clr_vector(struct vector_info vec);

extern void clear_expression_lookaside(void);
extern void save_expression_lookaside(unsigned addr,
				      ivl_expr_t exp,
				      unsigned wid);

extern unsigned allocate_vector_exp(ivl_expr_t exp, unsigned wid);

extern int number_is_unknown(ivl_expr_t ex);
extern int number_is_immediate(ivl_expr_t ex, unsigned lim_wid);
extern unsigned long get_number_immediate(ivl_expr_t ex);

/*
 * draw_eval_real evaluates real value expressions. The return code
 * from the function is the index of the word register that contains
 * the result.
 */
extern int draw_eval_real(ivl_expr_t ex);

/*
 * These functions manage word register allocation.
 */
extern int allocate_word(void);
extern void clr_word(int idx);

/*
 * These are used to count labels as I generate code.
 */
extern unsigned local_count;
extern unsigned thread_count;

#endif
