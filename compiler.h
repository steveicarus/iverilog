#ifndef IVL_compiler_H
#define IVL_compiler_H
/*
 * Copyright (c) 1999-2021 Stephen Williams (steve@icarus.com)
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

# include  <list>
# include  <map>
# include  "netlist.h"
# include  "StringHeap.h"

/*
 * This defines constants and defaults for the compiler in general.
 */


/*
 * The integer_width is the width of integer variables. This is also
 * the minimum width of unsized integers when they are found in
 * self-determined contexts.
 */
extern unsigned integer_width;

/*
 * The width_cap is the width limit for unsized expressions.
 */
extern unsigned width_cap;

/*
 * This is the maximum number of recursive module loops allowed within
 * a generate block.
 */
extern unsigned recursive_mod_limit;

/* The TIME_WIDTH is the width of time variables. */
#ifndef TIME_WIDTH
# define TIME_WIDTH 64
#endif

/*
 * When doing dynamic linking, we need a uniform way to identify the
 * symbol. Some compilers put leading _, some trailing _. The
 * configure script figures out which is the local convention and
 * defines NEED_LU and NEED_TU as required.
 */
#ifdef NEED_LU
#define LU "_"
#else
#define LU ""
#endif

#ifdef NEED_TU
#define TU "_"
#else
#define TU ""
#endif


/*
 * These are flags to enable various sorts of warnings. By default all
 * the warnings are off, the -W<list> parameter arranges for each to be
 * enabled.
 */

/* Implicit definitions of wires. */
extern bool warn_implicit;

/* Warn if dimensions of port or var/net are implicitly taken from
   the input/output/inout declaration. */
extern bool warn_implicit_dimensions;

/* inherit timescales across files. */
extern bool warn_timescale;

/* Warn about legal but questionable module port bindings. */
extern bool warn_portbinding;

/* Warn about constant out of bound selects. */
extern bool warn_ob_select;

/* Warn about structures that may have infinite loops. */
extern bool warn_inf_loop;

/* Warn about always @* statements where a part or word select causes
   sensitivity to an entire vector or array. */
extern bool warn_sens_entire_vec;
extern bool warn_sens_entire_arr;

/* Warn about level-appropriate anachronisms. */
extern bool warn_anachronisms;

/* Warn about nets that are references but not driven. */
extern bool warn_floating_nets;

/* This is true if verbose output is requested. */
extern bool verbose_flag;

extern bool debug_scopes;
extern bool debug_eval_tree;
extern bool debug_elaborate;
extern bool debug_emit;
extern bool debug_synth2;
extern bool debug_optimizer;

/* Ignore errors about missing modules */
extern bool ignore_missing_modules;

/* Treat each source file as a separate compilation unit (as defined
   by SystemVerilog). */
extern bool separate_compilation;

/* Control evaluation of functions at compile time:
 *   0 = only for functions in constant expressions
 *   1 = only for automatic functions
 *   2 = for all functions
 * Level 2 should only be used if the user can guarantee that a
 * function's local variables are never accessed from outside the
 * function. */
extern unsigned opt_const_func;

/* Possibly temporary flag to control virtualization of pin arrays */
extern bool disable_virtual_pins;

/* The vlog95 code generator does not want the compiler to generate concat-Z
 * LPM objects so this flag is used to block them from being generated. */
extern bool disable_concatz_generation;

/* Limit to size of devirtualized arrays */
extern unsigned long array_size_limit;

/* Path to a directory useful for finding subcomponents. */
extern const char*basedir;

/* This is an ordered list of library suffixes to search. */
extern std::list<const char*>library_suff;
extern int build_library_index(const char*path, bool key_case_sensitive);

/* This is the generation of Verilog that the compiler is asked to
   support. Then there are also more detailed controls for more
   specific language features. Note that the compiler often assumes
   this is an ordered list. */
enum generation_t {
      GN_VER1995  = 1,
      GN_VER2001_NOCONFIG  = 2,
      GN_VER2001  = 3,
      GN_VER2005  = 4,
      GN_VER2005_SV  = 5,
      GN_VER2009  = 6,
      GN_VER2012  = 7,
      GN_DEFAULT  = 4
};

extern generation_t generation_flag;

/* If this flag is true, enable extended types support. */
extern bool gn_cadence_types_flag;

/* If this flag is true, enable miscellaneous extensions. */
extern bool gn_icarus_misc_flag;

/* If this flag is true, then elaborate specify blocks. If this flag
   is false, then skip elaboration of specify behavior. */
extern bool gn_specify_blocks_flag;

/* If this flag is true, then add input/output buffers to modules so that
   VVP can insert intermodpaths inbetween. If this flag
   is false, then no input/output buffers are inserted if not needed. */
extern bool gn_interconnect_flag;

/* If this flag is true, then elaborate supported assertion statements. If
   this flag is false, then stub out supported assertion statements. */
extern bool gn_supported_assertions_flag;
/* If this flag is true, then error on unsupported assertion statements. If
   this flag is false, then stub out unsupported assertion statements. */
extern bool gn_unsupported_assertions_flag;

/* If this flag is true, then support/elaborate Verilog-AMS. */
extern bool gn_verilog_ams_flag;

/* If this flag is false a warning is printed when the port declaration
   is scalar and the net/register definition is vectored. */
extern bool gn_io_range_error_flag;

/* If this flag is true, then force re-evaluation of user functions
   in a continuous assignment when any part of the expression is
   re-evaluated. */
extern bool gn_strict_ca_eval_flag;

/* If this flag is true, then force strict conformance to the IEEE
   standard expression width rules. */
extern bool gn_strict_expr_width_flag;

/* If this flag is true, then don't add a for-loop control variable
   to an implicit event_expression list if it is only used inside the
   loop. */
extern bool gn_shared_loop_index_flag;

static inline bool gn_system_verilog(void)
{
      if (generation_flag >= GN_VER2005_SV)
	    return true;
      return false;
}

/* If variables can be converted to uwires by a continuous assignment
   (assuming no procedural assign), then return true. This will be true
   for SystemVerilog */
static inline bool gn_var_can_be_uwire(void)
{
      return gn_system_verilog();
}

static inline bool gn_modules_nest(void)
{
      return gn_system_verilog();
}

/* The bits of these GN_KEYWORDS_* constants define non-intersecting
   sets of keywords. The compiler enables groups of keywords by setting
   lexor_keyword_mask with the OR of the bits for the keywords to be
   enabled. */
enum { GN_KEYWORDS_1364_1995        = 0x0001,
       GN_KEYWORDS_1364_2001        = 0x0002,
       GN_KEYWORDS_1364_2001_CONFIG = 0x0004,
       GN_KEYWORDS_1364_2005        = 0x0008,
       GN_KEYWORDS_VAMS_2_3         = 0x0010,
       GN_KEYWORDS_1800_2005        = 0x0020,
       GN_KEYWORDS_1800_2009        = 0x0040,
       GN_KEYWORDS_1800_2012        = 0x0080,
       GN_KEYWORDS_ICARUS           = 0x8000
};
extern int lexor_keyword_mask;

  /* This is the string to use to invoke the preprocessor. */
extern char*ivlpp_string;

extern std::map<perm_string,unsigned> missing_modules;

  /* Files that are library files are in this map. The lexor compares
     file names as it processes `line directives, and if the file name
     matches an entry in this table, it will turn on the
     library_active_flag so that modules know that they are in a
     library. */
extern std::map<perm_string,bool> library_file_map;

/*
 * the lex_strings are perm_strings made up of tokens from the source
 * file. Identifiers are so likely to be used many times that it makes
 * much sense to use a StringHeapLex to hold them.
 */
extern StringHeapLex lex_strings;

/*
 * The ivl_target.h API in a variety of places keeps strings of
 * bits. Manage these as perm_string in a StringHeap.
 */
extern StringHeapLex bits_strings;

/*
 * The filename_strings are perm_strings for file names. They are put
 * into their own StringHeapLex because these paths are used a *lot*
 * and this makes them more sure to hash together.
 */
extern StringHeapLex filename_strings;


/*
 * system task/function listings.
 */
/*
 * This table describes all the return values of various system
 * functions. This table is used to elaborate expressions that are
 * system function calls.
 */
struct sfunc_return_type {
      const char*   name;
      ivl_variable_type_t type;
      unsigned      wid;
      bool          signed_flag;
      bool          override_flag;
};

extern void add_sys_func(const struct sfunc_return_type&ret_type);
extern const struct sfunc_return_type* lookup_sys_func(const char*name);
extern int load_sys_func_table(const char*path);
extern void cleanup_sys_func_table();
/*
 * This temporarily loads a VPI module, to determine the return values
 * of system functions provided by that module, and adds the return values
 * to the system function table.
 */
extern bool load_vpi_module(const char*path);

/*
 * In system Verilog it is allowed with a warning to call a function
 * as a task. You can even cast the return value away and have no
 * warning message.
 */
extern ivl_sfunc_as_task_t def_sfunc_as_task;

extern void pre_process_failed(const char*text);

#endif /* IVL_compiler_H */
