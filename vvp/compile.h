#ifndef __compile_H
#define __compile_H
/*
 * Copyright (c) 2001-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: compile.h,v 1.86 2007/03/01 06:19:39 steve Exp $"
#endif

# include  <stdio.h>
# include  "parse_misc.h"
# include  "vpi_user.h"
# include  "vvp_net.h"

using namespace std;

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
extern void inputs_connect(vvp_net_t*fdx, unsigned argc, struct symb_s*argv);
extern void input_connect(vvp_net_t*fdx, unsigned port, char*label);

/*
 * This function is an expansion of the inputs_connect function. It
 * uses the inputs_connect function, but it creates vvp_wide_fun_t
 * nodes to handle arbitrary width nodes, and connects those nodes to
 * the vvp_wide_fun_core object passed in.
 */
extern void wide_inputs_connect(vvp_wide_fun_core*core,
				unsigned argc, struct symb_s*argv);

extern vvp_net_t* vvp_net_lookup(const char*label);

/*
 *  Add a functor to the symbol table
 */
extern void define_functor_symbol(const char*label, vvp_net_t*ipt);


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
extern void compile_functor(char*label, char*type, unsigned width,
			    unsigned ostr0, unsigned ostr1,
			    unsigned argc, struct symb_s*argv);


/*
 * This is called by the parser to make a resolver. This is a special
 * kind of functor; a strength aware functor. It has up to 4 inputs
 * that are blended to make a resolved output. The type string selects
 * a resolution algorithm.
 */
extern void compile_resolver(char*label, char*type,
			     unsigned argc, struct symb_s*argv);

extern void compile_concat(char*label, unsigned w0, unsigned w1,
			   unsigned w2, unsigned w3,
			   unsigned argc, struct symb_s*argv);

/*
 * Compile delay nodes of various form.
 */
extern void compile_delay(char*label, vvp_delay_t*del, struct symb_s input);
extern void compile_delay(char*label, unsigned argc, struct symb_s*argv);

/*
 * This is called by the parser to create a part select node.
 * See the PART SELECT STATEMENT section in the README.txt
 */
extern void compile_part_select(char*label, char*src,
				unsigned base, unsigned wid);
extern void compile_part_select_pv(char*label, char*src,
				   unsigned base, unsigned wid,
				   unsigned vec_wid);
extern void compile_part_select_var(char*label, char*src,
				     char*var, unsigned wid);

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
extern void compile_cmp_eeq(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_nee(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_eq(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_ne(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_ge(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);
extern void compile_cmp_gt(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);

extern void compile_arith_div_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_arith_sub_r(char*label, unsigned argc, struct symb_s*argv);

extern void compile_dff(char*label,
			struct symb_s arg_d,
			struct symb_s arg_c,
			struct symb_s arg_e,
			struct symb_s arg_a);

class vvp_fun_modpath;
extern vvp_fun_modpath* compile_modpath(char*label, struct symb_s src);
extern void compile_modpath_src(vvp_fun_modpath*dst,
				struct symb_s input,
				struct numbv_s d);
extern void compile_modpath_src(vvp_fun_modpath*dst,
				struct symb_s input,
				struct numbv_s d,
				struct symb_s condit_input);

extern void compile_reduce_and(char*label, struct symb_s arg);
extern void compile_reduce_or(char*label, struct symb_s arg);
extern void compile_reduce_xor(char*label, struct symb_s arg);
extern void compile_reduce_nand(char*label, struct symb_s arg);
extern void compile_reduce_nor(char*label, struct symb_s arg);
extern void compile_reduce_xnor(char*label, struct symb_s arg);

extern void compile_extend_signed(char*label, long width, struct symb_s arg);

extern void compile_sfunc(char*label, char*name, char*format_string,
			  unsigned argc, struct symb_s*argv);
extern void compile_repeat(char*label, long width, long repeat,
			   struct symb_s arg);

extern void compile_shiftl(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_shiftr(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);

extern void compile_timescale(long units);

extern void compile_vpi_symbol(const char*label, vpiHandle obj);
extern void compile_vpi_lookup(vpiHandle *objref, char*label);

extern void compile_param_string(char*label, char*name, char*value);
extern void compile_param_logic(char*label, char*name, char*value,
				bool signed_flag);

/*
 * This function schedules a lookup of an indexed label. The ref
 * points to the vvp_net_t that receives the result. The result may
 * be assigned later, if the symbol is defined later in the source
 * file, so the memory that ref points to must persist.
 *
 * The text for the label will be deleted by this function, or the
 * cleanup that completes the binding.
 */
extern void functor_ref_lookup(vvp_net_t**ref, char*lab);

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
				vvp_delay_t*delay,
				unsigned argc, struct symb_s*argv);

extern char **compile_udp_table(char **table, char *row);

/*
 * Memory Instances, Ports, and Initialization
 */

extern void compile_array(char*label, char*name, int last, int first);

extern void compile_array_port(char*label, char*name, char*addr);

extern void compile_memory(char *label, char *name, int lsb, int msb,
			   unsigned idxs, long *idx);

extern void compile_memory_port(char *label, char *memid,
				unsigned argc, struct symb_s *argv);

extern void compile_memory_init(char *memid, unsigned idx, long val);

/*
 * Compile the .ufunc statement.
 */
extern void compile_ufunc(char*label, char*code, unsigned wid,
			  unsigned argc, struct symb_s*argv,
			  unsigned portc, struct symb_s*portv,
			  struct symb_s retv);

/*
 * The compile_event function takes the parts of the event statement
 * and makes the various objects needed to simulate it. This includes
 * the functor that receives the signals and the event_t that holds
 * the threads.
 */
extern void compile_event(char*label, char*type,
			  unsigned argc, struct symb_s*argv,
			  bool debug_flag);
extern void compile_named_event(char*label, char*type);


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
extern void compile_var_real(char*label, char*name,
			     int msb, int lsb);
extern void compile_variablew(char*label, char*array_symbol,
			     int msb, int lsb, char signed_flag);
extern void compile_varw_real(char*label, char*array_symbol,
			     int msb, int lsb);

extern void compile_net(char*label, char*name,
			int msb, int lsb, bool signed_flag,
			bool net8_flag,
			unsigned argc, struct symb_s*argv);
extern void compile_net_real(char*label, char*name,
			     int msb, int lsb,
			     unsigned argc, struct symb_s*argv);

extern void compile_netw(char*label, char*array_symbol,
			int msb, int lsb, bool signed_flag,
			bool net8_flag,
			unsigned argc, struct symb_s*argv);
extern void compile_netw_real(char*label, char*array_symbol,
			     int msb, int lsb,
			     unsigned argc, struct symb_s*argv);

extern void compile_alias(char*label, char*name,
			  int msb, int lsb, bool signed_flag,
			  unsigned argc, struct symb_s*argv);
extern void compile_alias_real(char*label, char*name,
			       int msb, int lsb,
			       unsigned argc, struct symb_s*argv);

/*
 * $Log: compile.h,v $
 * Revision 1.86  2007/03/01 06:19:39  steve
 *  Add support for conditional specify delay paths.
 *
 * Revision 1.85  2007/01/16 05:44:16  steve
 *  Major rework of array handling. Memories are replaced with the
 *  more general concept of arrays. The NetMemory and NetEMemory
 *  classes are removed from the ivl core program, and the IVL_LPM_RAM
 *  lpm type is removed from the ivl_target API.
 *
 * Revision 1.84  2006/11/22 06:10:05  steve
 *  Fix spurious event from net8 that is forced.
 *
 * Revision 1.83  2006/09/23 04:57:19  steve
 *  Basic support for specify timing.
 *
 * Revision 1.82  2006/07/30 02:51:36  steve
 *  Fix/implement signed right shift.
 *
 * Revision 1.81  2006/06/18 04:15:50  steve
 *  Add support for system functions in continuous assignments.
 *
 * Revision 1.80  2006/03/18 22:51:10  steve
 *  Syntax for carrying sign with parameter.
 *
 * Revision 1.79  2006/03/08 05:29:42  steve
 *  Add support for logic parameters.
 *
 * Revision 1.78  2006/01/02 05:32:07  steve
 *  Require explicit delay node from source.
 *
 * Revision 1.77  2005/11/25 17:55:26  steve
 *  Put vec8 and vec4 nets into seperate net classes.
 *
 * Revision 1.76  2005/10/12 17:23:15  steve
 *  Add alias nodes.
 *
 * Revision 1.75  2005/07/14 23:34:19  steve
 *  gcc4 compile errors.
 *
 * Revision 1.74  2005/07/06 04:29:25  steve
 *  Implement real valued signals and arith nodes.
 *
 * Revision 1.73  2005/06/17 03:46:52  steve
 *  Make functors know their own width.
 *
 * Revision 1.72  2005/05/24 01:43:27  steve
 *  Add a sign-extension node.
 *
 * Revision 1.71  2005/05/08 23:40:14  steve
 *  Add support for variable part select.
 *
 * Revision 1.70  2005/04/28 04:59:53  steve
 *  Remove dead functor code.
 *
 * Revision 1.69  2005/04/24 20:07:26  steve
 *  Add DFF nodes.
 *
 * Revision 1.68  2005/04/03 05:45:51  steve
 *  Rework the vvp_delay_t class.
 *
 * Revision 1.67  2005/04/01 06:02:45  steve
 *  Reimplement combinational UDPs.
 *
 * Revision 1.66  2005/03/18 02:56:04  steve
 *  Add support for LPM_UFUNC user defined functions.
 *
 * Revision 1.65  2005/03/09 05:52:04  steve
 *  Handle case inequality in netlists.
 *
 * Revision 1.64  2005/03/09 04:52:40  steve
 *  reimplement memory ports.
 *
 * Revision 1.63  2005/03/03 04:33:10  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 * Revision 1.62  2005/02/07 22:42:42  steve
 *  Add .repeat functor and BIFIF functors.
 *
 * Revision 1.61  2005/02/03 04:55:13  steve
 *  Add support for reduction logic gates.
 *
 * Revision 1.60  2005/01/22 01:06:20  steve
 *  Implement the .cmp/eeq LPM node.
 *
 * Revision 1.59  2005/01/09 20:11:15  steve
 *  Add the .part/pv node and related functionality.
 *
 * Revision 1.58  2004/12/29 23:45:13  steve
 *  Add the part concatenation node (.concat).
 *
 *  Add a vvp_event_anyedge class to handle the special
 *  case of .event statements of edge type. This also
 *  frees the posedge/negedge types to handle all 4 inputs.
 *
 *  Implement table functor recv_vec4 method to receive
 *  and process vectors.
 *
 * Revision 1.57  2004/12/11 02:31:29  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.56  2004/10/04 01:10:59  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.55  2004/06/30 02:15:57  steve
 *  Add signed LPM divide.
 */
#endif
