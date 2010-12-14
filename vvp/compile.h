#ifndef __compile_H
#define __compile_H
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

# include  <stdio.h>
# include  "parse_misc.h"
# include  "vpi_user.h"

/*
 * The functions described here are the compile time support
 * functions. Various bits of the compile process are taken care of
 * here. What is called when is mostly controlled by the parser.
 *
 * Before compilation takes place, the compile_init function must be
 * called once to set stuff up.
 */

extern void compile_init(void);

extern void compile_cleanup(void);

extern bool verbose_flag;

/*
 *  Connect a list of symbols to a contiguous set of ipoints.
 *  Constants C<?> are handled by setting the ival of the ipoint.
 */
extern void inputs_connect(vvp_ipoint_t fdx, unsigned argc, struct symb_s*argv);

/*
 *  Add a functor to the symbol table
 */
extern void define_functor_symbol(const char*label, vvp_ipoint_t ipt);


/*
 * This is a count of errors encountered during compilation. If this
 * is non-zero, then simulation is not recommended.
 */
extern unsigned compile_errors;

extern const char* module_path;
extern void compile_load_vpi_module(char*name);

extern void compile_vpi_time_precision(long pre);

/*
 * This function is called by the parser to compile a functor
 * statement. The strings passed in are allocated by the lexor, but
 * this function will free them. (Or save them permanently.) This
 * includes the argv array and the strings it references.
 *
 * The argc and argv are a list of char* that are the port parameters
 * of the functor. The compile should match those port parameters up
 * to existing functors to manage the linking.
 */
extern void compile_functor(char*label, char*type,
			    vvp_delay_t delay, unsigned ostr0,
			    unsigned ostr1,
			    unsigned argc, struct symb_s*argv);


/*
 * This is called by the parser to make a resolver. This is a special
 * kind of functor; a strength aware functor.
 */
extern void compile_resolver(char*label, char*type,
			     unsigned argc, struct symb_s*argv);

/*
 * This is called by the parser to make force functors.
 */
extern void compile_force(char*label, struct symb_s signal,
			  unsigned argc, struct symb_s*argv);

/*
 * This is called by the parser to make the various arithmetic and
 * comparison functors.
 */
extern void compile_arith_div(char*label, long width, bool signed_flag,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_mod(char*label, long width,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_mult(char*label, long width,
			       unsigned argc, struct symb_s*argv);
extern void compile_arith_sum(char*label, long width,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_sub(char*label, long width,
			      unsigned argc, struct symb_s*argv);
extern void compile_cmp_eq(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_ne(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_ge(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_gt(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);
extern void compile_decode_adr(char*label,
			       unsigned argc, struct symb_s*argv);
extern void compile_decode_en(char*label, char*decoder, int slice,
			      struct symb_s enable,
			      struct symb_s mass_enable);
extern void compile_demux(char*label, char*decoder, int slice,
			  struct symb_s not_selected,
			  struct symb_s selected);
extern void compile_shiftl(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_shiftr(char*label, long width,
			   unsigned argc, struct symb_s*argv);

extern void compile_timescale(long units);

extern void compile_vpi_symbol(const char*label, vpiHandle obj);
extern void compile_vpi_lookup(vpiHandle *objref, char*label);

extern void compile_param_string(char*label, char*name,
				 char*str, char*value);

/*
 * This function schedules a lookup of an indexed label. The ref
 * points to the vvp_ipoint_t that receives the result. The result may
 * be assigned later, if the symbol is defined later in the source
 * file, so the memory that ref points to must persist.
 */
extern void functor_ref_lookup(vvp_ipoint_t *ref, char*lab, unsigned idx);

/*
 * This function schedules a lookup of the labeled instruction. The
 * code points to a code structure that points to the instruction
 * field that receives the result, and the label is the name to
 * lookup. The lookup will free the label text when it is done.
 */
extern void code_label_lookup(struct vvp_code_s *code, char *label);

/*
 * The `compile_udp_def' function creates a UDP.  The `table' is a
 * NULL terminated array of char*, as assembled by `compile_udp_table'.
 * `compile_udp_table' is called with `table'==NULL to create a new
 * table, or with an existing table to append to.
 *
 * `compile_udp_functor' creates a mode-3 functor referring to the
 * labeled UDP.
 */

extern void compile_udp_def(int sequ, char*label, char *name,
			    unsigned nin, unsigned init, char **table);

extern void compile_udp_functor(char*label, char*type,
				vvp_delay_t delay,
				unsigned argc, struct symb_s*argv);

extern char **compile_udp_table(char **table, char *row);

/*
 * Memory Instances, Ports, and Initialization
 */

extern void compile_memory(char *label, char *name, int lsb, int msb,
			   unsigned idxs, long *idx);

extern void compile_memory_port(char *label, char *memid,
				unsigned lsb, unsigned msb,
				unsigned naddr,
				unsigned argc, struct symb_s *argv);

extern void compile_memory_init(char *memid, unsigned idx, unsigned char val);

/*
 * Compile the .ufunc statement.
 */
extern void compile_ufunc(char*label, char*code, unsigned wid,
			  unsigned argc, struct symb_s*argv,
			  unsigned portc, struct symb_s*portv,
			  unsigned retc, struct symb_s*retv);

/*
 * The compile_event function takes the parts of the event statement
 * and makes the various objects needed to simulate it. This includes
 * the functor that receives the signals and the event_t that holds
 * the threads.
 */
extern void compile_event(char*label, char*type,
			  unsigned argc, struct symb_s*argv);
extern void compile_named_event(char*label, char*type);

/*
 * Word declarations include a label, a type symbol, and a vpi name.
 */
extern void compile_word(char*label, char*type, char*name);

/*
 * A code statement is a label, an opcode and up to 3 operands. There
 * are a few lexical types that the parser recognizes of the operands,
 * given by the ltype_e enumeration. The compile_code function takes
 * the label, mnemonic and parsed operands and writes a properly
 * formed instruction into the code space. The label is set into the
 * symbol table with the address of the instruction.
 */

#define OPERAND_MAX 3
enum ltype_e { L_NUMB, L_SYMB };

struct comp_operands_s {
      unsigned argc;
      struct {
	    enum ltype_e ltype;
	    union {
		  unsigned long numb;
		  struct symb_s symb;
	    };
      } argv[OPERAND_MAX];
};

typedef struct comp_operands_s*comp_operands_t;

extern void compile_code(char*label, char*mnem, comp_operands_t opa);
extern void compile_disable(char*label, struct symb_s symb);

extern void compile_vpi_call(char*label, char*name,
			     unsigned argc, vpiHandle*argv);

/* Compile a function call. The vbit and vwid encode the return
   type. If the vwid >0, the return type is a vector. If the vwid is
   <0, the return type is -vpiRealConst or some other constant subtype
   code that represents the function type. */
extern void compile_vpi_func_call(char*label, char*name,
				  unsigned vbit, int vwid,
				  unsigned argc, vpiHandle*argv);

extern void compile_fork(char*label, struct symb_s targ_s,
			 struct symb_s scope_s);
extern void compile_codelabel(char*label);

/*
 * The parser uses these functions to compile .scope statements.
 * The implementations of these live in the vpi_scope.cc file.
 */
extern void compile_scope_decl(char*typ, char*lab, char*nam,char*tnam,char*par);
extern void compile_scope_recall(char*sym);

/*
 * The parser uses this function to declare a thread. The start_sym is
 * the start instruction, and must already be defined.
 */
extern void compile_thread(char*start_sym, char*flag);

/*
 * This function is called to create a var vector with the given name.
 */
extern void compile_variable(char*label, char*name,
			     int msb, int lsb, char signed_flag);

extern void compile_net(char*label, char*name,
			int msb, int lsb, bool signed_flag,
			unsigned argc, struct symb_s*argv);

#endif
