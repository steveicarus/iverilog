#ifndef __compile_H
#define __compile_H
/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  <cstdio>
# include  <fstream>
# include  <vector>
# include  "parse_misc.h"
# include  "vpi_user.h"
# include  "vvp_net.h"

using namespace std;

/*
 * The file names are kept in this vector. Entry 0 is "N/A" and 1 is
 * for interactive commands.
 */
extern vector<const char*> file_names;

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
 * If this file opened, then write debug information to this
 * file. This is used for debugging the VVP runtime itself.
 */
extern ofstream debug_file;

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
extern vpiHandle vvp_lookup_handle(const char*label);

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
 * Arrange for the system task/function call to have its compiletf
 * function called.
 */
extern void compile_compiletf(struct __vpiSysTaskCall*);

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
extern void compile_arith_pow(char*label, long width, bool signed_flag,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_abs(char*label,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_cast_int(char*label, long width,
			           unsigned argc, struct symb_s*argv);
extern void compile_arith_cast_real(char*label, bool signed_flag,
			            unsigned argc, struct symb_s*argv);
extern void compile_arith_div(char*label, long width, bool signed_flag,
			      unsigned argc, struct symb_s*argv);
extern void compile_arith_mod(char*label, long width, bool signed_flag,
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

extern void compile_arith_mult_r(char*label, unsigned argc,
                                 struct symb_s*argv);
extern void compile_arith_pow_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_arith_div_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_arith_mod_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_arith_sum_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_arith_sub_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_cmp_eq_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_cmp_ne_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_cmp_ge_r(char*label, unsigned argc, struct symb_s*argv);
extern void compile_cmp_gt_r(char*label, unsigned argc, struct symb_s*argv);

extern void compile_dff(char*label,
			struct symb_s arg_d,
			struct symb_s arg_c,
			struct symb_s arg_e,
			struct symb_s arg_a);

struct __vpiModPath;
extern __vpiModPath* compile_modpath(char*label,
				     struct symb_s drv,
				     struct symb_s dest);
extern void compile_modpath_src(__vpiModPath*dst,
				char edge,
				struct symb_s input,
				struct numbv_s d,
				int condit_input, /* match with '0' */
				struct symb_s path_term_input,
				bool ifnone);
extern void compile_modpath_src(__vpiModPath*dst,
				char edge,
				struct symb_s input,
				struct numbv_s d,
				struct symb_s condit_input,
				struct symb_s path_term_input);

extern void compile_reduce_and(char*label, struct symb_s arg);
extern void compile_reduce_or(char*label, struct symb_s arg);
extern void compile_reduce_xor(char*label, struct symb_s arg);
extern void compile_reduce_nand(char*label, struct symb_s arg);
extern void compile_reduce_nor(char*label, struct symb_s arg);
extern void compile_reduce_xnor(char*label, struct symb_s arg);

extern void compile_extend_signed(char*label, long width, struct symb_s arg);

extern void compile_sfunc(char*label, char*name, char*format_string,
			  long file_idx, long lineno,
			  unsigned argc, struct symb_s*argv,
                          char*trigger_label);

extern void compile_repeat(char*label, long width, long repeat,
			   struct symb_s arg);

extern void compile_shiftl(char*label, long width,
			   unsigned argc, struct symb_s*argv);
extern void compile_shiftr(char*label, long width, bool signed_flag,
			   unsigned argc, struct symb_s*argv);

extern void compile_timescale(long units, long precision);

extern void compile_vpi_symbol(const char*label, vpiHandle obj);
extern void compile_vpi_lookup(vpiHandle *objref, char*label);

extern void compile_param_string(char*label, char*name, char*value,
                                 long file_idx, long lineno);
extern void compile_param_logic(char*label, char*name, char*value,
				bool signed_flag,
                                long file_idx, long lineno);
extern void compile_param_real(char*label, char*name, char*value,
                               long file_idx, long lineno);

/*
 * The resolv_list_s is the base class for a symbol resolve
 * action. Some function creates an instance of a resolv_list_s object
 * that contains the data pertinent to that resolution request, and
 * executes it with the resolv_submit function. If the operation can
 * complete, then the resolv_submit deletes the object. Otherwise, it
 * pushes it onto the resolv_list for later processing.
 *
 * Derived classes implement the resolve function to perform the
 * actual binding or resolution that the instance requires. If the
 * function succeeds, the resolve method returns true and the object
 * can be deleted any time.
 *
 * The mes parameter of the resolve method tells the resolver that
 * this call is its last chance. If it cannot complete the operation,
 * it must print an error message and return false.
 */
class resolv_list_s {

    public:
      explicit resolv_list_s(char*lab) : label_(lab) { }
      virtual ~resolv_list_s();
      virtual bool resolve(bool mes = false) = 0;

    protected:
      const char*label() const { return label_; }

    private:
      friend void resolv_submit(class resolv_list_s*cur);
      friend void compile_cleanup(void);

      char*label_;
      class resolv_list_s*next;
};

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

extern void compile_var_array(char*label, char*name,
			      int last, int first,
			      int msb, int lsb, char signed_flag);
extern void compile_real_array(char*label, char*name,
			       int last, int first,
			       int msb, int lsb);
extern void compile_net_array(char*label, char*name,
			      int last, int first);
extern void compile_array_alias(char*label, char*name, char*src);

  /* Index is a net. */
extern void compile_array_port(char*label, char*name, char*addr);
  /* Index is a constant address */
extern void compile_array_port(char*label, char*name, long addr);

extern void compile_array_cleanup(void);

/*
 * Compile the .ufunc statement.
 */
extern void compile_ufunc(char*label, char*code, unsigned wid,
			  unsigned argc, struct symb_s*argv,
			  unsigned portc, struct symb_s*portv,
			  struct symb_s retv, char*scope_label,
                          char*trigger_label);

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
			     long file_idx, long lineno,
			     unsigned argc, vpiHandle*argv);

/* Compile a function call. The vbit and vwid encode the return
   type. If the vwid >0, the return type is a vector. If the vwid is
   <0, the return type is -vpiRealConst or some other constant subtype
   code that represents the function type. */
extern void compile_vpi_func_call(char*label, char*name,
				  unsigned vbit, int vwid,
				  long file_idx, long lineno,
				  unsigned argc, vpiHandle*argv);
extern void print_vpi_call_errors();

extern void compile_fork(char*label, struct symb_s targ_s,
			 struct symb_s scope_s);
extern void compile_codelabel(char*label);

/*
 * The parser uses these functions to compile .scope statements.
 * The implementations of these live in the vpi_scope.cc file.
 */
extern void compile_scope_decl(char*typ, char*lab, char*nam, char*tnam,
                               char*par, long file_idx, long lineno,
                               long def_file_idx, long def_lineno);
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
			     int msb, int lsb, char signed_flag,
			     bool local_flag);
extern void compile_var_real(char*label, char*name,
			     int msb, int lsb);

extern void compile_net(char*label, char*name,
			int msb, int lsb, bool signed_flag,
			bool net8_flag, bool local_flag,
			unsigned argc, struct symb_s*argv);
extern void compile_net_real(char*label, char*name,
			     int msb, int lsb, bool local_flag,
			     unsigned argc, struct symb_s*argv);

extern void compile_netw(char*label, char*array_symbol,
			 unsigned long array_addr,
			 int msb, int lsb, bool signed_flag,
			 bool net8_flag,
			 unsigned argc, struct symb_s*argv);
extern void compile_netw_real(char*label, char*array_symbol,
			      unsigned long array_addr,
			      int msb, int lsb,
			      unsigned argc, struct symb_s*argv);

extern void compile_alias(char*label, char*name,
			  int msb, int lsb, bool signed_flag,
			  unsigned argc, struct symb_s*argv);
extern void compile_alias_real(char*label, char*name,
			       int msb, int lsb,
			       unsigned argc, struct symb_s*argv);
extern void compile_aliasw(char*label, char*array_symbol,
			   unsigned long array_addr, int msb, int lsb,
			   unsigned argc, struct symb_s*argv);

extern void compile_island(char*label, char*type);
extern void compile_island_port(char*label, char*island, char*src);
extern void compile_island_import(char*label, char*island, char*src);
extern void compile_island_export(char*label, char*island);
extern void compile_island_cleanup(void);

extern void compile_island_tran(char*label);
extern void compile_island_tranif(int sense, char*island,
				  char*ba, char*bb, char*src);
extern void compile_island_tranvp(char*island, char*ba, char*bb,
				  unsigned width, unsigned part, unsigned off);

extern void delete_udp_symbols(void);

#endif
