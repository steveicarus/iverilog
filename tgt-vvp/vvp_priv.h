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
 * Keep a count of errors that would render the output unusable.
 */
extern int vvp_errors;

struct vector_info {
      unsigned base;
      unsigned wid;
};

/*
 * Convenient constants...
 */
  /* Width limit for typical immediate arguments. */
# define IMM_WID 32

  /* The number of words available in a thread. */
# define WORD_COUNT 16

/*
 * Mangle all non-symbol characters in an identifier, quotes in names
 */
extern const char *vvp_mangle_id(const char *);
extern const char *vvp_mangle_name(const char *);

extern char* draw_Cr_to_string(double value);

/*
 * This generates a string from a signal that uniquely identifies
 * that signal with letters that can be used in a label.
 *
 * NOTE: vvp_signal_label should be removed. All it does is a %p of
 * the pointer, and return a pointer to a static. The code that wants
 * to reference a signal needs to use the format V_%p, so the presence
 * of this function is just plain inconsistent.
 */
extern const char* vvp_signal_label(ivl_signal_t sig);

extern unsigned width_of_nexus(ivl_nexus_t nex);
extern ivl_variable_type_t data_type_of_nexus(ivl_nexus_t nex);

extern int can_elide_bufz(ivl_net_logic_t net, ivl_nexus_ptr_t nptr);

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

extern struct vector_info draw_ufunc_expr(ivl_expr_t expr, unsigned wid);
extern int draw_ufunc_real(ivl_expr_t expr);

extern void pad_expr_in_place(ivl_expr_t expr, struct vector_info res,
                              unsigned swid);

/*
 * modpath.c symbols.
 *
 * draw_modpath arranges for a .modpath record to be written out.
 *
 * cleanup_modpath() cleans up any pending .modpath records that may
 * have been scheduled by draw_modpath() but not yet written.
 *
 * Note: draw_modpath drive_label must be malloc'ed by the
 * caller. This function will free the string sometime in the future.
 */
extern void draw_modpath(ivl_signal_t path_sig, char*drive_label);
extern void cleanup_modpath(void);

/*
 * This function draws the execution of a vpi_call statement, along
 * with the tricky handling of arguments. If this is called with a
 * statement handle, it will generate a %vpi_call
 * instruction. Otherwise, it will generate a %vpi_func instruction.
 */
extern void draw_vpi_task_call(ivl_statement_t net);

extern struct vector_info draw_vpi_func_call(ivl_expr_t expr,
					     unsigned wid);
extern int draw_vpi_rfunc_call(ivl_expr_t expr);

/*
 * Switches (tran)
 */
extern void draw_switch_in_scope(ivl_switch_t sw);

/* Draw_net_input and friends uses this. */
struct vvp_nexus_data {
	/* draw_net_input uses this */
      const char*net_input;
      unsigned drivers_count;
      int flags;
	/* draw_net_in_scope uses these to identify the controlling word. */
      ivl_signal_t net;
      unsigned net_word;
};
#define VVP_NEXUS_DATA_STR 0x0001


/*
 * Given a nexus, draw a string that represents the functor output
 * that feeds the nexus. This function can be used to get the input to
 * a functor, event, or even a %load in cases where I have the
 * ivl_nexus_t object. The draw_net_input function will get the string
 * cached in the nexus, if there is one, or will generate a string and
 * cache it.
 */
extern const char* draw_net_input(ivl_nexus_t nex);
void EOC_cleanup_drivers();

/*
 * See draw_net_input.c for details on draw_net_input_x. (It would be
 * nice if this can be made private.)
 */
  /* Omit LPMPART_BI device pin-data(0) drivers. */
# define OMIT_PART_BI_DATA 0x0001
struct vvp_nexus_data;
extern char* draw_net_input_x(ivl_nexus_t nex,
			      ivl_nexus_ptr_t omit_ptr, int omit_flags,
			      struct vvp_nexus_data*nex_data);

/*
 * This function is different from draw_net_input in that it will
 * return a reference to a net as its first choice. This reference
 * will follow the net value, even if the net is assigned or
 * forced. The draw_net_input above will return a reference to the
 * *input* to the net and so will not follow direct assignments to
 * the net. This function will not return references to local signals,
 * and will in those cases resort to the net input, or a non-local
 * signal if one exists for the nexus.
 */
extern const char*draw_input_from_net(ivl_nexus_t nex);

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
 *
 *   STUFF_OK_RO -- This bit is set if the node is allowed to nest its
 *        allocation from vector. It is only true if the client is not
 *        planning to use this vector as an output. This matters only
 *        if the expression might be found in the lookaside table, and
 *        therefore might be multiply allocated if allowed.
 */

extern struct vector_info draw_eval_expr(ivl_expr_t expr, int stuff_ok_flag);
extern struct vector_info draw_eval_expr_wid(ivl_expr_t expr, unsigned w,
					     int stuff_ok_flag);
#define STUFF_OK_XZ 0x0001
#define STUFF_OK_47 0x0002
#define STUFF_OK_RO 0x0004

/*
 * This evaluates an expression and leaves the result in the numbered
 * integer index register. It also will set bit-4 to 1 if the value is
 * not fully defined (i.e. contains x or z).
 */
extern void draw_eval_expr_into_integer(ivl_expr_t expr, unsigned ix);

/*
 * These functions manage vector allocation in the thread register
 * space. They presume that we work on one thread at a time, to
 * completion.
 *
 *  allocate_vector
 *    Return the base of an allocated vector in the thread. The bits
 *    are marked allocated in the process.
 *
 *  clr_vector
 *    Clear a vector previously allocated.
 *
 * The thread vector allocator also keeps a lookaside of expression
 * results that are stored in register bit. This lookaside can be used
 * by the code generator to notice that certain expression bits are
 * already calculated, and can be reused.
 *
 *  clear_expression_lookaside
 *    Clear the lookaside tables for the current thread. This must be
 *    called before starting a new thread, and around basic blocks
 *    that are entered from unknown places.
 *
 *  save_expression_lookaside
 *    Mark the given expression as available in the given register
 *    bits. This remains until the lookaside is cleared. This does not
 *    clear the allocation, it is still necessary to call clr_vector.
 *
 *  save_signal_lookaside
 *    Mark the given signal as available in the given register bits.
 *    This is different from a given expression, in that the signal
 *    lookaside is in addition to the expression lookaside. The signal
 *    lookaside is specifically to save on unnecessary loads of a
 *    signal recently written.
 *
 *  allocate_vector_exp
 *    This function attempts to locate the expression in the
 *    lookaside. If it finds it, return a reallocated base for the
 *    expression. Otherwise, return 0.
 *
 * The allocate_vector and allocate_vector_exp calls must have
 * matching call to clr_vector. Although the allocate_vector will
 * never reallocate a vector already allocated, the allocate_vector_exp
 * might, so it is possible for allocations to nest in that
 * manner. The exclusive_flag to allocate_vector_exp will prevent
 * nested allocations. This is needed where the expression result is
 * expected to be overwritten.
 */
extern unsigned allocate_vector(unsigned wid);
extern void clr_vector(struct vector_info vec);

extern void clear_expression_lookaside(void);
extern void save_expression_lookaside(unsigned addr,
				      ivl_expr_t expr,
				      unsigned wid);
extern void save_signal_lookaside(unsigned addr,
				  ivl_signal_t sig, unsigned use_word,
				  unsigned wid);

extern unsigned allocate_vector_exp(ivl_expr_t expr, unsigned wid,
				    int exclusive_flag);

extern int number_is_unknown(ivl_expr_t ex);
extern int number_is_immediate(ivl_expr_t ex, unsigned lim_wid, int negative_is_ok);
extern long get_number_immediate(ivl_expr_t ex);
extern uint64_t get_number_immediate64(ivl_expr_t ex);

/*
 * draw_eval_real evaluates real value expressions. The return code
 * from the function is the index of the word register that contains
 * the result.
 */
extern int draw_eval_real(ivl_expr_t ex);

/*
 * draw_eval_bool64 evaluates a bool expression. The return code from
 * the function is the index of the word register that contains the
 * result. The word is allocated with allocate_word(), so the caller
 * must arrange for it to be released with clr_word(). The width must
 * be such that it fits in a 64bit word.
 */
extern int draw_eval_bool64(ivl_expr_t ex);

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
