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
#ident "$Id: ivl_target.h,v 1.16 2000/10/06 23:46:50 steve Exp $"
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
 *
 * The interface is divided into two parts: the entry points within
 * the core that are called by the module, and the entry points in
 * the module that are called by the core. It is the latter that
 * causes the module to be invoked in the first place, but most of the
 * interesting information about the design is accessed through the
 * various access functions that the modules calls into the core.
 */


/* 
 * In order to grab onto data in the design, the core passes cookies
 * to the various functions of the module. These cookies can in turn
 * be passed to access functions in the core to get more detailed
 * information.
 *
 * The following typedefs list the various cookies that may be passed
 * around.
 *
 * ivl_design_t
 *    This object represents the entire elaborated design. Various
 *    global properties and methods are available from this.
 *
 * ivl_expr_t
 *    This object represents a node of an expression. If the
 *    expression has sub-expressions, they can be accessed from
 *    various method described below. The ivl_expr_type method in
 *    particular gets the type of the node in the form of an
 *    ivl_expr_type_t enumeration value.
 *
 *    Objects of this type represent expressions in
 *    processes. Structural expressions are instead treated as logic
 *    gates.
 *
 * ivl_process_t
 *    A Verilog process is represented by one of these. A process may
 *    be an "initial" or an "always" process. These come from initial
 *    or always statements from the verilog source.
 *
 * ivl_scope_t
 *    Elaborated scopes within a design are represented by this
 *    type. Objects of this type also act as containers for scoped
 *    objects such as signals.
 *
 * ivl_statement_t
 *    Statements within processes are represented by one of these. The
 *    ivl_process_t object holds one of these, but a statement may in
 *    turn contain other statements.
 *
 * -- A Note About Bit Sets --
 * Some objects hold a value as an array of bits. In these cases there
 * is some method that retrieves the width of the value and another
 * that returns a "char*". The latter is a pointer to the least
 * significant bit value. Bit values are represented by the characters
 * '0', '1', 'x' and 'z'. Strengths are stored elsewhere.
 *
 * -- A Note About Names --
 * The names of objects are complete, hierarchical names. That is,
 * they include the instance name of the module that contains them.
 */
typedef struct ivl_design_s   *ivl_design_t;
typedef struct ivl_expr_s     *ivl_expr_t;
typedef struct ivl_net_bufz_s *ivl_net_bufz_t;
typedef struct ivl_net_const_s*ivl_net_const_t;
typedef struct ivl_net_event_s*ivl_net_event_t;
typedef struct ivl_net_logic_s*ivl_net_logic_t;
typedef struct ivl_net_probe_s*ivl_net_probe_t;
typedef struct ivl_nexus_s    *ivl_nexus_t;
typedef struct ivl_process_s  *ivl_process_t;
typedef struct ivl_scope_s    *ivl_scope_t;
typedef struct ivl_signal_s   *ivl_signal_t;
typedef struct ivl_statement_s*ivl_statement_t;

/*
 * These are types that are defined as enumerations. These have
 * explicit values so that the binary API is a bit more resilient to
 * changes and additions to the enumerations.
 */

/* This is the type of an ivl_expr_t object. */
typedef enum ivl_expr_type_e {
      IVL_EX_NONE = 0,
      IVL_EX_BINARY,
      IVL_EX_NUMBER,
      IVL_EX_SFUNC,
      IVL_EX_SIGNAL,
      IVL_EX_STRING,
      IVL_EX_SUBSIG,
} ivl_expr_type_t;

/* This is the type code for an ivl_net_logic_t object. */
typedef enum ivl_logic_e {
      IVL_LO_NONE = 0,
      IVL_LO_AND,
      IVL_LO_BUF,
      IVL_LO_BUFIF0,
      IVL_LO_BUFIF1,
      IVL_LO_NAND,
      IVL_LO_NOR,
      IVL_LO_NOT,
      IVL_LO_NOTIF0,
      IVL_LO_NOTIF1,
      IVL_LO_OR,
      IVL_LO_XNOR,
      IVL_LO_XOR
} ivl_logic_t;

/* Processes are initial or always blocks with a statement. This is
   the type of the ivl_process_t object. */
typedef enum ivl_process_type_e {
      IVL_PR_INITIAL = 0,
      IVL_PR_ALWAYS  = 1
} ivl_process_type_t;

/* Signals (ivl_signal_t) that are ports into the scope that contains
   them have a port type. Otherwise, they are port IVL_SIP_NONE. */
typedef enum ivl_signal_port_e {
      IVL_SIP_NONE  = 0,
      IVL_SIP_INPUT = 1,
      IVL_SIP_OUTPUT= 2,
      IVL_SIP_INOUT = 3
} ivl_signal_port_t;

/* This is the type code for an ivl_signal_t object. Implicit types
   are resolved by the core compiler, and integers are converted into
   signed registers. */
typedef enum ivl_signal_type_e {
      IVL_SIT_NONE  = 0,
      IVL_SIT_REG,
      IVL_SIT_SUPPLY0,
      IVL_SIT_SUPPLY1,
      IVL_SIT_TRI,
      IVL_SIT_TRI0,
      IVL_SIT_TRI1,
      IVL_SIT_TRIAND,
      IVL_SIT_TRIOR,
      IVL_SIT_WAND,
      IVL_SIT_WIRE,
      IVL_SIT_WOR
} ivl_signal_type_t;

/* This is the type code for ivl_statement_t objects. */
typedef enum ivl_statement_type_e {
      IVL_ST_NONE   = 0,
      IVL_ST_NOOP   = 1,
      IVL_ST_ASSIGN,
      IVL_ST_BLOCK,
      IVL_ST_CONDIT,
      IVL_ST_DELAY,
      IVL_ST_DELAYX,
      IVL_ST_STASK,
      IVL_ST_TRIGGER,
      IVL_ST_WAIT,
      IVL_ST_WHILE
} ivl_statement_type_t;


/* This function returns the string value of the named flag. The key
   is used to select the flag. If the key does not exist or the flag
   does not have a value, this function returns 0.

   Flags come from the "-fkey=value" options to the iverilog command
   line.

   The key "-o" is special and is the argument to the -o flag of the
   iverilog command. This is generally how the target learns the name
   of the output file. */
extern const char* ivl_get_flag(ivl_design_t, const char*key);

/* Get the name of the root module. This can be used as the design name. */
extern const char* ivl_get_root_name(ivl_design_t net);

/*
 * These methods apply to ivl_net_const_t objects.
 */
extern const char* ivl_const_bits(ivl_net_const_t net);
extern ivl_nexus_t ivl_const_pin(ivl_net_const_t net, unsigned idx);
extern unsigned    ivl_const_pins(ivl_net_const_t net);
extern int         ivl_const_signed(ivl_net_const_t net);

/* EXPRESSION
 * These methods operate on expression objects from the
 * design. Expressions mainly exist in behavioral code. The
 * ivl_expr_type() function returns the type of the expression node,
 * and the remaining functions access value bits of the expression.
 */
extern ivl_expr_type_t ivl_expr_type(ivl_expr_t net);

  /* IVL_EX_NUMBER */
extern const char* ivl_expr_bits(ivl_expr_t net);
  /* IVL_EX_SIGNAL, IVL_EX_SFUNC */
extern const char* ivl_expr_name(ivl_expr_t net);
  /* IVL_EX_BINARY */
extern char        ivl_expr_opcode(ivl_expr_t net);
  /* IVL_EX_BINARY */
extern ivl_expr_t  ivl_expr_oper1(ivl_expr_t net);
  /* IVL_EX_BINARY */
extern ivl_expr_t  ivl_expr_oper2(ivl_expr_t net);
  /* */
extern ivl_expr_t  ivl_expr_oper3(ivl_expr_t net);
  /* any expression */
extern int         ivl_expr_signed(ivl_expr_t net);
  /* IVL_EX_STRING */
extern const char* ivl_expr_string(ivl_expr_t net);
  /* any expression */
extern unsigned    ivl_expr_width(ivl_expr_t net);

/* LOGIC
 * These types and functions support manipulation of logic gates. The
 * ivl_logic_t enumeration identifies the various kinds of gates that
 * the ivl_net_logic_t can represent. The various functions then
 * provide access to the bits of information for a given logic device.
 */

extern ivl_logic_t ivl_logic_type(ivl_net_logic_t net);
extern ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin);
extern unsigned    ivl_logic_pins(ivl_net_logic_t net);

/* NEXUS
 * connections of signals and nodes is handled by single-bit
 * nexus. These functions manage the ivl_nexus_t object.
 */

extern const char* ivl_nexus_name(ivl_nexus_t net);

/* SIGNALS
 * Signals are named things in the Verilog source, like wires and
 * regs, and also named things that are preated as temporaries during
 * certain elaboration or optimization steps. A signal may also be a
 * port of a module or task.
 *
 * Signals have a name (obviously) and types. A signal may also be
 * signed or unsigned.
 */
extern unsigned  ivl_signal_pins(ivl_signal_t net);
extern ivl_signal_port_t ivl_signal_port(ivl_signal_t net);
extern ivl_signal_type_t ivl_signal_type(ivl_signal_t net);

/*
 * These functions get information about a process. A process is
 * an initial or always block within the original Verilog source, that
 * is translated into a type and a single statement. (The statement
 * may be a compound statement.)
 *
 * The ivl_process_type function gets the type of the process,
 * an "inital" or "always" statement.
 *
 * The ivl_process_stmt function gets the statement that forms the
 * process. See the statement related functions for how to manipulate
 * statements.
 */
extern ivl_process_type_t ivl_process_type(ivl_process_t net);

extern ivl_statement_t ivl_process_stmt(ivl_process_t net);

/*
 * These functions manage statements of various type. This includes
 * all the different kinds of statements (as enumerated in
 * ivl_statement_type_t) that might occur in behavioral code.
 *
 * The ivl_statement_type() function returns the type code for the
 * statement. This is the major type, and implies which of the later
 * functions are applicable to the statemnt.
 */
extern ivl_statement_type_t ivl_statement_type(ivl_statement_t net);

/*
 * The following functions retrieve specific single values from the
 * statement. These values are the bits of data and parameters that
 * make up the statement. Many of these functions apply to more then
 * one type of statement, so the comment in front of them tells which
 * statement types can be passed to the function.
 */

  /* IVL_ST_BLOCK */
extern unsigned ivl_stmt_block_count(ivl_statement_t net);
  /* IVL_ST_BLOCK */
extern ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_CONDIT */
extern ivl_expr_t      ivl_stmt_cond_expr(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_false(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_true(ivl_statement_t net);
  /* IVL_ST_DELAY */
extern unsigned long ivl_stmt_delay_val(ivl_statement_t net);
  /* IVL_ST_ASSIGN */
extern unsigned ivl_stmt_lwidth(ivl_statement_t net);
  /* IVL_ST_STASK */
extern const char* ivl_stmt_name(ivl_statement_t net);
  /* IVL_ST_STASK */
extern ivl_expr_t ivl_stmt_parm(ivl_statement_t net, unsigned idx);
  /* IVL_ST_STASK */
extern unsigned ivl_stmt_parm_count(ivl_statement_t net);
  /* IVL_ST_ASSIGN */
extern ivl_expr_t ivl_stmt_rval(ivl_statement_t net);
  /* IVL_ST_DELAY, IVL_ST_WAIT, IVL_ST_WHILE */
extern ivl_statement_t ivl_stmt_sub_stmt(ivl_statement_t net);


/* TARGET MODULE ENTRY POINTS
 *
 * These are not functions in the API but functions that the target
 * module supplies. They are presented as typedefs of functions (which
 * are used internally) but the target module makes them work by
 * exporting them.
 *
 * The module entry points generally take a cookie and possibly a name
 * as parameters. They use the cookie to get the required detailed
 * information, and they do their job. The functions return an integer
 * value which usually should be 0 for success, or less then 0 for any
 * errors. How the error is interpreted depends on the function
 * returning the error.
 */

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

/* target_net_signal

   Signals are things like "wire foo" or "reg bar;" that is, declared
   signals in the verilog source. These are not memories, which are
   handled elsewhere. */
typedef int (*net_signal_f)(const char*name, ivl_signal_t net);


/* target_process

   The "target_process" function is called for each always and initial
   block in the design. In principle, the target creates a thread for
   each process in the Verilog original.

   This function is called with the entire thread generated. The
   process and statement access methods can be used to randomly
   (read-only) access all the code of the thread. Also, the module may
   hold on to the process, the core will not delete it. */
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
 * Revision 1.16  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.15  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.14  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.13  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.12  2000/09/24 15:46:00  steve
 *  API access to signal type and port type.
 *
 * Revision 1.11  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
 * Revision 1.10  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.9  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.8  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.7  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 * Revision 1.6  2000/08/27 15:51:50  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.5  2000/08/26 00:54:03  steve
 *  Get at gate information for ivl_target interface.
 *
 * Revision 1.4  2000/08/20 04:13:57  steve
 *  Add ivl_target support for logic gates, and
 *  make the interface more accessible.
 *
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
