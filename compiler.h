#ifndef __compiler_H
#define __compiler_H
/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: compiler.h,v 1.12 2002/05/28 20:40:37 steve Exp $"
#endif

# include  <list>

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
 * the warnings are of, the -W<list> parameter arranges for each to be
 * enabled. 
 */

/* Implicit definitions of wires. */
extern bool warn_implicit;
extern bool error_implicit;

/* inherit timescales accross files. */
extern bool warn_timescale;

/* This is true if verbose output is requested. */
extern bool verbose_flag;

/* This is an ordered list of library suffixxes to search. */
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

/*
 * $Log: compiler.h,v $
 * Revision 1.12  2002/05/28 20:40:37  steve
 *  ivl indexes the search path for libraries, and
 *  supports case insensitive module-to-file lookup.
 *
 * Revision 1.11  2002/05/28 00:50:39  steve
 *  Add the ivl -C flag for bulk configuration
 *  from the driver, and use that to run library
 *  modules through the preprocessor.
 *
 * Revision 1.10  2002/05/24 01:13:00  steve
 *  Support language generation flag -g.
 *
 * Revision 1.9  2002/04/22 00:53:39  steve
 *  Do not allow implicit wires in sensitivity lists.
 *
 * Revision 1.8  2002/04/15 00:04:22  steve
 *  Timescale warnings.
 *
 * Revision 1.7  2001/11/16 05:07:19  steve
 *  Add support for +libext+ in command files.
 */
#endif
