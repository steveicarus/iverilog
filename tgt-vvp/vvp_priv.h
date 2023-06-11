#ifndef IVL_vvp_priv_H
#define IVL_vvp_priv_H
/*
 * Copyright (c) 2001-2020 Stephen Williams (steve@icarus.com)
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

# include  "vvp_config.h"
# include  "ivl_target.h"
# include  <stdio.h>

extern int debug_draw;

/*
 * The target_design entry opens the output file that receives the
 * compiled design, and sets the vvp_out to the descriptor.
 */
extern FILE* vvp_out;

/*
 * Keep a count of errors that would render the output unusable.
 */
extern int vvp_errors;

extern unsigned transient_id;

/*
 * Set to non-zero when the user wants to display file and line number
 * information for procedural statements.
 */
extern unsigned show_file_line;

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
extern void draw_lpm_substitute(ivl_lpm_t net);

extern void draw_ufunc_vec4(ivl_expr_t expr);
extern void draw_ufunc_real(ivl_expr_t expr);
extern void draw_ufunc_string(ivl_expr_t expr);
extern void draw_ufunc_object(ivl_expr_t expr);

extern char* process_octal_codes(const char*txt, unsigned wid);

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
extern void draw_modpath(ivl_signal_t path_sig, char*drive_label, unsigned drive_index);
extern void cleanup_modpath(void);

/*
 * This function draws the execution of a vpi_call statement, along
 * with the tricky handling of arguments. If this is called with a
 * statement handle, it will generate a %vpi_call
 * instruction. Otherwise, it will generate a %vpi_func instruction.
 */
extern void draw_vpi_task_call(ivl_statement_t net);

extern void draw_vpi_func_call(ivl_expr_t expr);
extern void draw_vpi_rfunc_call(ivl_expr_t expr);
extern void draw_vpi_sfunc_call(ivl_expr_t expr);

extern void draw_class_in_scope(ivl_type_t classtype);

/*
 * Enumeration draw routine.
 */
void draw_enumeration_in_scope(ivl_enumtype_t enumtype);

/*
 * Switches (tran)
 */
extern void draw_switch_in_scope(ivl_switch_t sw);

/* Draw_net_input and friends uses this. */
struct vvp_nexus_data {
	/* draw_net_input uses this */
      const char*net_input;
	/* draw_isnald_net_input uses these */
      const char*island_input;
      ivl_island_t island;
	/* */
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
void EOC_cleanup_drivers(void);

/*
 * This is different from draw_net_input in that it is intended to be
 * used within the network of an island. This finds and prepares the
 * link for a nexus within the scope of the given island, instead of
 * the net as a whole.
 */
extern const char* draw_island_net_input(ivl_island_t island, ivl_nexus_t nex);

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
 * This evaluates an expression and leaves the result in the numbered
 * integer index register. It also will set bit-4 to 1 if the value is
 * not fully defined (i.e. contains x or z).
 */
extern void draw_eval_expr_into_integer(ivl_expr_t expr, unsigned ix);

/*
 * This evaluates an expression as a condition flag and leaves the
 * result in a flag that is returned. This result may be used as an
 * operand for conditional jump instructions.
 */
extern int draw_eval_condition(ivl_expr_t expr);

/*
 * Return true if the signal is the return value of a function.
 */
extern int signal_is_return_value(ivl_signal_t sig);

extern int number_is_unknown(ivl_expr_t ex);
extern int number_is_immediate(ivl_expr_t ex, unsigned lim_wid, int negative_is_ok);
extern long get_number_immediate(ivl_expr_t ex);
extern uint64_t get_number_immediate64(ivl_expr_t ex);

/*
 * draw_eval_vec4 evaluates vec4 expressions. The result of the
 * evaluation is the vec4 result in the top of the vec4 expression stack.
 */
extern void draw_eval_vec4(ivl_expr_t ex);
extern void resize_vec4_wid(ivl_expr_t expr, unsigned wid);

/*
 * draw_eval_real evaluates real value expressions. The result of the
 * evaluation is the real result in the top of the real expression stack.
 */
extern void draw_eval_real(ivl_expr_t ex);

/*
 * The draw_eval_string function evaluates the expression as a string,
 * and pushes the string onto the string stack.
 */
extern void draw_eval_string(ivl_expr_t ex);

/*
 * The draw_eval_string function evaluates the expression as an object,
 * and pushes the object onto the object stack.
 */
extern int draw_eval_object(ivl_expr_t ex);

extern int show_stmt_assign(ivl_statement_t net);
extern void show_stmt_file_line(ivl_statement_t net, const char*desc);

/*
 */
extern int test_immediate_vec4_ok(ivl_expr_t expr);
extern void draw_immediate_vec4(ivl_expr_t expr, const char*opcode);

/*
 * Draw a delay statement.
 */
extern void draw_delay(void*ptr, unsigned wid, const char*input,
		       ivl_expr_t rise_exp, ivl_expr_t fall_exp,
		       ivl_expr_t decay_exp);

/*
 * These functions manage word register allocation.
 */
extern int allocate_word(void);
extern void clr_word(int idx);

/*
 * These functions manage flag bit allocation.
 */
extern int allocate_flag(void);
extern void clr_flag(int idx);

/*
 * These are used to count labels as I generate code.
 */
extern unsigned local_count;
extern unsigned thread_count;

extern void darray_new(ivl_type_t element_type, unsigned size_reg);

/*
 * These are various statement code generators.
 */
extern int show_statement(ivl_statement_t net, ivl_scope_t sscope);

extern int show_stmt_break(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_continue(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_forever(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_forloop(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_repeat(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_while(ivl_statement_t net, ivl_scope_t sscope);
extern int show_stmt_do_while(ivl_statement_t net, ivl_scope_t sscope);

#endif /* IVL_vvp_priv_H */
