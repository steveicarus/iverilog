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
#ident "$Id: ivl_target.h,v 1.20 2000/10/15 04:46:23 steve Exp $"
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
 * ivl_net_logic_t
 *    This object represents various built in logic devices. In fact,
 *    this includes just about every directional device that has a
 *    single output, including logic gates and nmos, pmos and cmon
 *    devices. There is also the occasional Icarus Verilog creation.
 *
 * ivl_nexus_t
 *    Structural links within an elaborated design are connected
 *    together at each bit. The connection point is a nexus, so pins
 *    of devices refer to an ivl_nexus_t. Furthermore, from a nexus
 *    there are backward references to all the device pins that point
 *    to it.
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
typedef struct ivl_net_const_s*ivl_net_const_t;
typedef struct ivl_net_event_s*ivl_net_event_t;
typedef struct ivl_net_logic_s*ivl_net_logic_t;
typedef struct ivl_net_probe_s*ivl_net_probe_t;
typedef struct ivl_nexus_s    *ivl_nexus_t;
typedef struct ivl_nexus_ptr_s*ivl_nexus_ptr_t;
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
      IVL_LO_BUFZ,
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

/* This is the type of the function to apply to a process. */
typedef int (*ivl_process_f)(ivl_process_t net);

/* This is the type of a function to apply to a scope. */
typedef int (ivl_scope_f)(ivl_scope_t net);


/* DESIGN
 * When handed a design (ivl_design_t) there are a few things that you
 * can do with it. The Verilog program has one design that carries the
 * entire program. Use the design methods to iterate over the elements
 * of the design.
 *
 * ivl_design_flag
 *    This function returns the string value of a named flag. Flags
 *    come from the "-fkey=value" options to the iverilog command and
 *    are stored in a map for this function. Given the key, this
 *    function returns the value.
 *
 *    The special key "-o" is the argument to the -o flag of the
 *    command line (or the default if the -o flag is not used) and is
 *    generally how the target learns the name of the output file.
 *
 * ivl_design_process
 *    This function scans the processes (threads) in the design. It
 *    calls the user suplied function on each of the processes until
 *    one of the functors returns non-0 or all the processes are
 *    scanned. This function will return 0, or the non-zero value that
 *    was returned from the last scanned process.
 *
 * ivl_design_root
 *    A design has a root named scope that is an instance of the top
 *    level module in the design. This is a hook for naming the
 *    design, or for starting the scope scan.
 */

extern const char* ivl_design_flag(ivl_design_t des, const char*key);
extern int         ivl_design_process(ivl_design_t des, ivl_process_f fun);
extern ivl_scope_t ivl_design_root(ivl_design_t des);

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
 *
 * ivl_logic_type
 *    This method returns the type of logic gate that the cookie
 *    represents.
 *
 * ivl_logic_name
 *    This method returns the complete name of the logic gate. Every
 *    gate has a complete name (that includes the scope) even if the
 *    Verilog source doesn't include one. The compiler will choose one
 *    if necessary.
 *
 * ivl_logic_basename
 *    This is the name of the gate without the scope part.
 *
 * ivl_logic_pins
 * ivl_logic_pin
 */

extern const char* ivl_logic_name(ivl_net_logic_t net);
extern const char* ivl_logic_basename(ivl_net_logic_t net);
extern ivl_logic_t ivl_logic_type(ivl_net_logic_t net);
extern ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin);
extern unsigned    ivl_logic_pins(ivl_net_logic_t net);

/* NEXUS
 * connections of signals and nodes is handled by single-bit
 * nexus. These functions manage the ivl_nexus_t object. They also
 * manage the ivl_nexus_ptr_t objects that are closely related to the
 * nexus. 
 *
 * ivl_nexus_name
 *    Each nexus is given a name, typically derived from the signals
 *    connected to it, but completely made up if need be. The name of
 *    every nexus is unique.
 *
 * ivl_nexus_ptrs
 *    This function returns the number of pointers that are held by
 *    the nexus. It should always return at least 1. The pointer
 *    proper is accessed by index.
 *
 * ivl_nexus_ptr
 *    Return a nexus pointer given the nexus and an index.
 */

extern const char*     ivl_nexus_name(ivl_nexus_t net);
extern unsigned        ivl_nexus_ptrs(ivl_nexus_t net);
extern ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx);


/* SCOPE
 * Scopes of various sort have these properties. Use these methods to
 * access them. Scopes come to exist in the elaborated design
 * generally when a module is instantiated, though they also come from
 * named blocks, tasks and functions.
 *
 * (NOTE: Module scopes are *instances* of modules, and not the module
 * definition. A definition may apply to many instances.)
 *
 * ivl_scope_children
 *    A scope may in turn contain other scopes. This method iterates
 *    through all the child scopes of a given scope. If the function
 *    returns any value other then 0, the iteration stops and the
 *    method returns that value. Otherwise, iteration continues until
 *    the children run out.
 *
 *    If the scope has no children, this method will return 0 and
 *    otherwise do nothing.
 *
 * ivl_scope_log
 * ivl_scope_logs
 *    Scopes have 0 or more logic devices in them. A logic device is
 *    represented by ivl_logic_t.
 *
 * ivl_scope_name
 *    Every scope has a hierarchical name. This name is also a prefix
 *    of all the names of objects contained within the scope.
 *
 * ivl_scope_sig
 * ivl_scope_sigs
 *    Scopes have 0 or more signals in them. These signals are
 *    anything that can become and ivl_signal_t, include synthetic
 *    signals generated by the compiler.
 */

extern int          ivl_scope_children(ivl_scope_t net, ivl_scope_f func);
extern unsigned     ivl_scope_logs(ivl_scope_t net);
extern ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx);
extern const char*  ivl_scope_name(ivl_scope_t net);
extern unsigned     ivl_scope_sigs(ivl_scope_t net);
extern ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx);


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
extern const char* ivl_signal_name(ivl_signal_t net);
extern const char* ivl_signal_basename(ivl_signal_t net);


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


/* target_net_const (optional)

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


/* target_net_probe

   This is the probe, or structural trigger, of an event. The
   net_event_f is guaranteed to be called for the associated event
   before this probe is called. */
typedef int (*net_probe_f)(const char*name, ivl_net_probe_t net);


_END_DECL

/*
 * $Log: ivl_target.h,v $
 * Revision 1.20  2000/10/15 04:46:23  steve
 *  Scopes and processes are accessible randomly from
 *  the design, and signals and logic are accessible
 *  from scopes. Remove the target calls that are no
 *  longer needed.
 *
 *  Add the ivl_nexus_ptr_t and the means to get at
 *  them from nexus objects.
 *
 *  Give names to methods that manipulate the ivl_design_t
 *  type more consistent names.
 *
 * Revision 1.19  2000/10/13 03:39:27  steve
 *  Include constants in nexus targets.
 *
 * Revision 1.18  2000/10/08 04:01:54  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
 * Revision 1.17  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
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
 */
#endif
