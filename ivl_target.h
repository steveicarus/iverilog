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
#ident "$Id: ivl_target.h,v 1.3 2000/08/19 18:12:42 steve Exp $"
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

typedef struct ivl_net_bufz_s *ivl_net_bufz_t;
typedef struct ivl_net_const_s*ivl_net_const_t;
typedef struct ivl_net_event_s*ivl_net_event_t;
typedef struct ivl_net_logic_s*ivl_net_logic_t;
typedef struct ivl_net_probe_s*ivl_net_probe_t;
typedef struct ivl_process_s  *ivl_process_t;
typedef struct ivl_scope_s    *ivl_scope_t;


/* This function returns the string value of the named flag. The key
   is used to select the flag. If the key does not exist or the flag
   does not have a value, this function returns 0.

   Flags come from the "-fkey=value" options to the iverilog command
   line.

   The key "-o" is special and is the argument to the -o flag of the
   iverilog command. This is generally how the target learns the name
   of the output file. */
extern const char* ivl_get_flag(ivl_design_t, const char*key);


  /* TARGET MODULE ENTRY POINTS */

/* target_start_design  (required)

   The "target_start_design" function is called once before
   any other functions in order to start the processing of the
   netlist. The function returns a value <0 if there is an error. */
typedef int  (*start_design_f)(ivl_design_t des);


/* target_end_design  (required)

   The target_end_design function in the loaded module is called once
   to clean up (for example to close files) from handling of the
   netlist. */
typedef void (*end_design_f)(ivl_design_t des);


/* target_net_bufz

   The "target_net_bufz" function is called for all the BUFZ devices
   in the netlist. */
typedef int (*net_bufz_f)(const char*name, ivl_net_bufz_t net);


/* target_net_const

   The "target_net_const" function is called for structural constant
   values that appear in the design. */
typedef int (*net_const_f)(const char*name, ivl_net_const_t net);


/* target_net_event

   Verilog code such as @event and @(posedge foo) create event
   objects. These named objects can be triggered by structural probes
   or behavioral triggers. The target_net_event function is called
   once for each event in the netlist. The event function is
   guaranteed to be called before probe or trigger functions. */
typedef int (*net_event_f)(const char*name, ivl_net_event_t net);


/* target_net_logic

   This function is called for each logic gate in the design. The name
   parameter is the name of the gate in the design. If the gate is
   part of an array of gates, the name includes its index. */
typedef int (*net_logic_f)(const char*name, ivl_net_logic_t net);


/* target_net_probe

   This is the probe, or structural trigger, of an event. The
   net_event_f is guaranteed to be called for the associated event
   before this probe is called. */
typedef int (*net_probe_f)(const char*name, ivl_net_probe_t net);


/* target_process

   The "target_process" function is called for each always and initial
   block in the design. In principle, the target creates a thread for
   each process in the Verilog original.

   XXXX I have not yet decided if it it up to the target to scan the
   process statements, or if I will do that myself. Hmm... */
typedef int (*process_f)(ivl_process_t net);


/* target_scope (optional)

   If the "target_scope" function is implemented in the module, it is
   called to introduce a new scope in the design. If scopes are
   nested, this method is always called for the containing scope
   before the contained scope. Also, this is guaranteed to be called
   before functions for any objects contained in this scope. */
typedef void (*scope_f)(ivl_scope_t net);


_END_DECL

/*
 * $Log: ivl_target.h,v $
 * Revision 1.3  2000/08/19 18:12:42  steve
 *  Add target calls for scope, events and logic.
 *
 * Revision 1.2  2000/08/14 04:39:56  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */
#endif
