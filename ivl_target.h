#ifndef __ivl_target_H
#define __ivl_target_H
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: ivl_target.h,v 1.1 2000/08/12 16:34:37 steve Exp $"
#endif

#ifdef __cplusplus
#define _BEGIN_DECL extern "C" {
#define _END_DECL }
#else
#define _BEGIN_DECL
#define _END_DECL
#endif


_BEGIN_DECL

/*
 * This header file describes the API for the loadable target
 * module. The main program can load these modules and access the
 * functions within the loaded module to implement the backend
 * behavior.
 */


/* This is the opaque type of an entire design. This type is used when
   requesting an operation that affects the entire netlist. */
typedef struct ivl_design_s *ivl_design_t;


/* This function returns the string value of the named flag. The key
   is used to select the flag. If the key does not exist or the flag
   does not have a value, this function returns 0. */
extern const char* ivl_get_flag(ivl_design_t, const char*key);


  /* TARGET MODULE ENTRY POINTS */

/* target_start_design  (required)

   The "target_start_design" function is called once before
   any other functions in order to start the processing of the
   netlist. The function returns a value <0 if there is an error. */
typedef int  (*start_design_f)(ivl_design_t);


/* target_end_design  (required)

   The target_end_design function in the loaded module is called once
   to clean up (for example to close files) from handling of the
   netlist. */
typedef void (*end_design_f)(ivl_design_t);


_END_DECL

/*
 * $Log: ivl_target.h,v $
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */
#endif
