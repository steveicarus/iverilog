#ifndef __compile_H
#define __compile_H
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
#ident "$Id: compile.h,v 1.30 2001/07/06 04:46:44 steve Exp $"
#endif

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
 * The argc and argv are a list of char* that are the port paramters
 * of the functor. The compile should match those port parameters up
 * to existing functors to manage the linking.
 */
extern void compile_functor(char*label, char*type,
			    unsigned argc, struct symb_s*argv);


/*
 * This is called by the parser to make a resolver. This is a special
 * kind of functor; a strength aware functor.
 */
extern void compile_resolver(char*label, char*type,
			     unsigned argc, struct symb_s*argv);

/*
 * This is called by the parser to make the various arithmetic and
 * comparison functors.
 */
extern void compile_arith_mult(char*label, long width,
			       unsigned argc, struct symb_s*argv);
extern void compile_arith_sum(char*label, long width,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_sub(char*label, long width,
			      unsigned argc, struct symb_s*argv);
extern void compile_cmp_ge(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_gt(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_shiftl(char*label, long width,
			   unsigned argc, struct symb_s*argv);


extern void compile_vpi_symbol(const char*label, vpiHandle obj);
extern vpiHandle compile_vpi_lookup(const char*label);

/* 
 * The `compile_udp_def' function creates a UDP.  The `table' is a
 * NULL terminated array of char*, as assembled by `compile_udp_table'.  
 * `compile_udp_table' is called with `table'==NULL to create a new 
 * table, or with an existing table to append to.
 *
 * `compile_udp_functor' creates a mode-3 functor refering to the 
 * labeled UDP.  
 */

extern void compile_udp_def(int sequ, char*label, char *name,
			    unsigned nin, unsigned init, char **table);

extern void compile_udp_functor(char*label, char*type,
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
 * The compile_event function takes the parts of the event statement
 * and makes the various objects needed to simulate it. This includes
 * the functor that receives the signals and the event_t that holds
 * the threads.
 */
extern void compile_event(char*label, char*type,
			  unsigned argc, struct symb_s*argv);

extern void compile_named_event(char*label, char*name);

extern void compile_event_or(char*label, unsigned argc, struct symb_s*argv);

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

extern void compile_vpi_func_call(char*label, char*name,
				  unsigned vbit, unsigned vwid,
				  unsigned argc, vpiHandle*argv);

extern void compile_fork(char*label, struct symb_s targ_s,
			 struct symb_s scope_s);
extern void compile_codelabel(char*label);

/*
 * The parser uses these functions to compile .scope statements.
 * The implementations of these live in the vpi_scope.cc file.
 */
extern void compile_scope_decl(char*lab, char*nam, char*par);
extern void compile_scope_recall(char*sym);

/*
 * The parser uses this function to declare a thread. The start_sym is
 * the start instruction, and must already be defined.
 */
extern void compile_thread(char*start_sym);

/*
 * This function is called to create a var vector with the given name.
 */
extern void compile_variable(char*label, char*name,
			     int msb, int lsb, bool signed_flag);

extern void compile_net(char*label, char*name,
			int msb, int lsb, bool signed_flag,
			unsigned argc, struct symb_s*argv);

/*
 * $Log: compile.h,v $
 * Revision 1.30  2001/07/06 04:46:44  steve
 *  Add structural left shift (.shift/l)
 *
 * Revision 1.29  2001/06/30 23:03:17  steve
 *  support fast programming by only writing the bits
 *  that are listed in the input file.
 *
 * Revision 1.28  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.27  2001/06/15 04:07:58  steve
 *  Add .cmp statements for structural comparison.
 *
 * Revision 1.26  2001/06/15 03:28:31  steve
 *  Change the VPI call process so that loaded .vpi modules
 *  use a function table instead of implicit binding.
 *
 * Revision 1.25  2001/06/07 03:09:03  steve
 *  Implement .arith/sub subtraction.
 *
 * Revision 1.24  2001/06/05 03:05:41  steve
 *  Add structural addition.
 *
 * Revision 1.23  2001/05/20 00:46:12  steve
 *  Add support for system function calls.
 *
 * Revision 1.22  2001/05/09 04:23:18  steve
 *  Now that the interactive debugger exists,
 *  there is no use for the output dump.
 *
 * Revision 1.21  2001/05/09 02:53:25  steve
 *  Implement the .resolv syntax.
 *
 * Revision 1.20  2001/05/02 04:05:17  steve
 *  Remove the init parameter of functors, and instead use
 *  the special C<?> symbols to initialize inputs. This is
 *  clearer and more regular.
 *
 * Revision 1.19  2001/05/01 01:09:39  steve
 *  Add support for memory objects. (Stephan Boettcher)
 *
 * Revision 1.18  2001/04/25 04:35:05  steve
 *  Document the UDP implementation.
 *
 * Revision 1.17  2001/04/24 02:23:59  steve
 *  Support for UDP devices in VVP (Stephen Boettcher)
 *
 * Revision 1.16  2001/04/18 04:21:23  steve
 *  Put threads into scopes.
 *
 * Revision 1.15  2001/04/14 05:10:56  steve
 *  support the .event/or statement.
 *
 * Revision 1.14  2001/04/05 01:34:26  steve
 *  Add the .var/s and .net/s statements for VPI support.
 *
 * Revision 1.13  2001/04/01 06:40:45  steve
 *  Support empty statements for hanging labels.
 *
 * Revision 1.12  2001/03/29 03:46:36  steve
 *  Support named events as mode 2 functors.
 *
 * Revision 1.11  2001/03/26 04:00:39  steve
 *  Add the .event statement and the %wait instruction.
 *
 * Revision 1.10  2001/03/25 00:35:35  steve
 *  Add the .net statement.
 *
 * Revision 1.9  2001/03/23 02:40:22  steve
 *  Add the :module header statement.
 *
 * Revision 1.8  2001/03/22 22:38:14  steve
 *  Detect undefined system tasks at compile time.
 *
 * Revision 1.7  2001/03/21 05:13:03  steve
 *  Allow var objects as vpiHandle arguments to %vpi_call.
 *
 * Revision 1.6  2001/03/20 06:16:24  steve
 *  Add support for variable vectors.
 *
 * Revision 1.5  2001/03/18 04:35:18  steve
 *  Add support for string constants to VPI.
 *
 * Revision 1.4  2001/03/18 00:37:55  steve
 *  Add support for vpi scopes.
 *
 * Revision 1.3  2001/03/16 01:44:34  steve
 *  Add structures for VPI support, and all the %vpi_call
 *  instruction. Get linking of VPI modules to work.
 *
 * Revision 1.2  2001/03/11 22:42:11  steve
 *  Functor values and propagation.
 *
 * Revision 1.1  2001/03/11 00:29:38  steve
 *  Add the vvp engine to cvs.
 *
 */
#endif
