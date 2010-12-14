#ifndef __compiler_H
#define __compiler_H
/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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

# include  <list>
# include  <map>
# include  "netlist.h"
# include  "StringHeap.h"

/*
 * This defines constants and defaults for the compiler in general.
 */


/* The INTEGER_WIDTH is the default width of integer variables, and
   the presumed width of unsigned literal numbers. */
#ifndef INTEGER_WIDTH
# define INTEGER_WIDTH 32
#endif

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
extern bool error_implicit;

/* inherit timescales across files. */
extern bool warn_timescale;

/* Warn about legal but questionable module port bindings. */
extern bool warn_portbinding;

/* Warn about unused or unassigned variables. */
extern bool warn_unused;

/* This is true if verbose output is requested. */
extern bool verbose_flag;

extern bool debug_scopes;
extern bool debug_eval_tree;
extern bool debug_elaborate;
extern bool debug_synth;
extern bool debug_cprop;

/* Path to a directory useful for finding subcomponents. */
extern const char*basedir;

/* This is an ordered list of library suffixes to search. */
extern list<const char*>library_suff;
extern int build_library_index(const char*path, bool key_case_sensitive);

/* This is the generation of Verilog that the compiler is asked to
   support. */
enum generation_t {
      GN_VER1995  = 1,
      GN_VER2001  = 2,
      GN_SYSVER30 = 3,
      GN_DEFAULT  = 3
};

extern generation_t generation_flag;

  /* This is the string to use to invoke the preprocessor. */
extern char*ivlpp_string;

extern map<perm_string,unsigned> missing_modules;

/*
 * the lex_strings are perm_strings made up of tokens from the source
 * file. Identifiers are so likely to be used many times that it makes
 * much sense to use a StringHeapLex to hold them.
 */
extern StringHeapLex lex_strings;


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
      NetExpr::TYPE type;
      unsigned      wid;
      int           signed_flag;
};

extern const struct sfunc_return_type* lookup_sys_func(const char*name);
extern int load_sys_func_table(const char*path);

#endif
