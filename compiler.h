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
#ident "$Id: compiler.h,v 1.5 2000/10/31 17:49:02 steve Exp $"
#endif

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

/*
 * $Log: compiler.h,v $
 * Revision 1.5  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.4  2000/08/20 04:13:56  steve
 *  Add ivl_target support for logic gates, and
 *  make the interface more accessible.
 *
 * Revision 1.3  2000/03/17 21:50:25  steve
 *  Switch to control warnings.
 *
 * Revision 1.2  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.1  1999/06/06 20:42:48  steve
 *  Make compiler width a compile time constant.
 *
 */
#endif
