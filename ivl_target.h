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
#ident "$Id: ivl_target.h,v 1.44 2001/04/02 00:28:35 steve Exp $"
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
 * ivl_event_t
 *    This object represents an event node. An event node stands for
 *    named events written explicitly in the Verilog, and net events
 *    that are implicit when @ statements are used.
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
 * ivl_lpm_t
 *    This object is the base class for all the various LPM type
 *    device nodes. This object carries a few base properties
 *    (including a type) including a handle to the specific type.
 *
 *    All the ivl_lpm_*_t objects are derived from this type, and
 *    there are methods to get one from the other.
 *
 * ivl_lpm_ff_t
 *    This is a flip-flop.
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
typedef struct ivl_event_s    *ivl_event_t;
typedef struct ivl_expr_s     *ivl_expr_t;
typedef struct ivl_lpm_s      *ivl_lpm_t;
typedef struct ivl_lpm_ff_s   *ivl_lpm_ff_t;
typedef struct ivl_lval_s     *ivl_lval_t;
typedef struct ivl_net_const_s*ivl_net_const_t;
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
      IVL_EX_CONCAT,
      IVL_EX_NUMBER,
      IVL_EX_SCOPE,
      IVL_EX_SFUNC,
      IVL_EX_SIGNAL,
      IVL_EX_STRING,
      IVL_EX_SUBSIG,
      IVL_EX_UNARY
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
      IVL_LO_NMOS,
      IVL_LO_NOR,
      IVL_LO_NOT,
      IVL_LO_NOTIF0,
      IVL_LO_NOTIF1,
      IVL_LO_OR,
      IVL_LO_RNMOS,
      IVL_LO_RPMOS,
      IVL_LO_PMOS,
      IVL_LO_XNOR,
      IVL_LO_XOR
} ivl_logic_t;

/* This is the type of an LPM object. */
typedef enum ivl_lpm_type_e {
      IVL_LPM_FF
} ivl_lpm_type_t;

/* Processes are initial or always blocks with a statement. This is
   the type of the ivl_process_t object. */
typedef enum ivl_process_type_e {
      IVL_PR_INITIAL = 0,
      IVL_PR_ALWAYS  = 1
} ivl_process_type_t;

/* These are the sorts of reasons a scope may come to be. These types
   are properties of ivl_scope_t objects. */
typedef enum ivl_scope_type_e {
      IVL_SCT_MODULE  = 0,
      IVL_SCT_FUNCTION= 1,
      IVL_SCT_TASK    = 2,
      IVL_SCT_BEGIN   = 3,
      IVL_SCT_FORK    = 4
} ivl_scope_type_t;

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
      IVL_ST_CASE,
      IVL_ST_CASEX,
      IVL_ST_CASEZ,
      IVL_ST_CONDIT,
      IVL_ST_DELAY,
      IVL_ST_DELAYX,
      IVL_ST_FORK,
      IVL_ST_STASK,
      IVL_ST_TRIGGER,
      IVL_ST_WAIT,
      IVL_ST_WHILE
} ivl_statement_type_t;

/* This is the type of the function to apply to a process. */
typedef int (*ivl_process_f)(ivl_process_t net, void*cd);

/* This is the type of a function to apply to a scope. The ivl_scope_t
   parameter is the scope, and the cd parameter is client data that
   the user passes to the scanner. */
typedef int (ivl_scope_f)(ivl_scope_t net, void*cd);


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
extern int         ivl_design_process(ivl_design_t des,
				      ivl_process_f fun, void*cd);
extern ivl_scope_t ivl_design_root(ivl_design_t des);

/*
 * These methods apply to ivl_net_const_t objects.
 */
extern const char* ivl_const_bits(ivl_net_const_t net);
extern ivl_nexus_t ivl_const_pin(ivl_net_const_t net, unsigned idx);
extern unsigned    ivl_const_pins(ivl_net_const_t net);
extern int         ivl_const_signed(ivl_net_const_t net);

/* EVENTS
 *
 * Events are a unification of named events and implicit events
 * generated by the @ statements.
 *
 * ivl_event_name
 * ivl_event_basename
 *
 * ivl_event_edge
 *    Return the edge type for the event. If this is a named event
 *    that has no network input, then the edge is IVL_EDGE_NONE.
 */
extern const char*     ivl_event_name(ivl_event_t net);
extern const char*     ivl_event_basename(ivl_event_t net);

extern unsigned    ivl_event_nany(ivl_event_t net);
extern ivl_nexus_t ivl_event_any(ivl_event_t net, unsigned idx);

extern unsigned    ivl_event_nneg(ivl_event_t net);
extern ivl_nexus_t ivl_event_neg(ivl_event_t net, unsigned idx);

extern unsigned    ivl_event_npos(ivl_event_t net);
extern ivl_nexus_t ivl_event_pos(ivl_event_t net, unsigned idx);


/* EXPRESSIONS
 *
 * These methods operate on expression objects from the
 * design. Expressions mainly exist in behavioral code. The
 * ivl_expr_type() function returns the type of the expression node,
 * and the remaining functions access value bits of the expression.
 *
 * ivl_expr_signed
 *    This method returns true (!= 0) if the expression node
 *    represents a signed expression. It is possible for sub-
 *    expressions to be unsigned even if a node is signed, but the
 *    IVL core figures all this out for you. At any rate, this method
 *    can be applied to any expression node.
 *
 * ivl_expr_type
 *    Get the type of the expression node. Every expression node has a
 *    type, which can affect how some of the other expression methods
 *    operate on the node
 *
 * ivl_expr_width
 *    This method returns the bit width of the expression at this
 *    node. It can be applied to any expression node.
 */

extern ivl_expr_type_t ivl_expr_type(ivl_expr_t net);

  /* IVL_EX_NUMBER */
extern const char* ivl_expr_bits(ivl_expr_t net);
  /* IVL_EX_SIGNAL, IVL_EX_SFUNC */
extern const char* ivl_expr_name(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_UNARY */
extern char        ivl_expr_opcode(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_UNARY */
extern ivl_expr_t  ivl_expr_oper1(ivl_expr_t net);
  /* IVL_EX_BINARY */
extern ivl_expr_t  ivl_expr_oper2(ivl_expr_t net);
  /* */
extern ivl_expr_t  ivl_expr_oper3(ivl_expr_t net);
  /* IVL_EX_CONCAT */
extern ivl_expr_t  ivl_expr_parm(ivl_expr_t net, unsigned idx);
  /* IVL_EX_CONCAT */
extern unsigned    ivl_expr_parms(ivl_expr_t net);
  /* IVL_EX_CONCAT */
extern unsigned    ivl_expr_repeat(ivl_expr_t net);
  /* IVL_EX_SCOPE */
extern ivl_scope_t ivl_expr_scope(ivl_expr_t net);
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

/* LPM
 * These functions support access to the properties of LPM devices.
 *
 * ivl_lpm_name
 *    Return the name of the device.
 *
 * ivl_lpm_type
 *    Return the ivl_lpm_type_t of the secific LPM device.
 *
 * ivl_lpm_width
 *    Return the width of the LPM device. What this means depends on
 *    the LPM type, but it generally has to do with the width of the
 *    output data path.
 */
extern const char*    ivl_lpm_name(ivl_lpm_t net);
extern ivl_lpm_type_t ivl_lpm_type(ivl_lpm_t net);
extern unsigned       ivl_lpm_width(ivl_lpm_t net);


/*
 * These are cast functions for the ivl_lpm_t. They cast the object to
 * the requested type, checking for errors along the way.
 */
extern ivl_lpm_ff_t ivl_lpm_ff(ivl_lpm_t net);
extern ivl_nexus_t  ivl_lpm_ff_clk(ivl_lpm_ff_t net);
extern ivl_nexus_t  ivl_lpm_ff_data(ivl_lpm_ff_t net, unsigned idx);
extern ivl_nexus_t  ivl_lpm_ff_q(ivl_lpm_ff_t net, unsigned idx);


/* LVAL
 * The l-values of assignments are concatenation of ivl_lval_t
 * objects. Each l-value has a bunch of connections (in the form of
 * ivl_nexus_t objects) and possibly a mux expression. The compiler
 * takes care of part selects and nested concatenations. The
 * ivl_stmt_lval function pulls ivl_lval_t objects out of the
 * statement.
 *
 * ivl_lval_mux
 *    If the l-value includes a bit select expression, this method
 *    returns an ivl_expr_t that represents that
 *    expression. Otherwise, it returns 0.
 *
 * ivl_lval_pin
 *    Return an ivl_nexus_t for the connection of the ivl_lval_t.
 *
 * ivl_lval_pins
 *    Return the number of pins for this object.
 */

extern ivl_expr_t  ivl_lval_mux(ivl_lval_t net);
extern unsigned    ivl_lval_pins(ivl_lval_t net);
extern ivl_nexus_t ivl_lval_pin(ivl_lval_t net, unsigned idx);


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
 *
 * Once an ivl_nexus_ptr_t is selected by the ivl_nexus_ptr method,
 * the properties of the pointer can be accessed by the following
 * methods:
 *
 * ivl_nexus_ptr_pin
 *    This returns the pin number of the device where this nexus
 *    points. It is the bit within the signal or logic device that is
 *    connected to the nexus.
 *
 *    If the target is an LPM device, then this value is zero, and it
 *    is up to the application to find the pin that referse to this
 *    nexus. The problem is that LPM devices do not have a pinout per
 *    se, the pins all have specific names.
 *
 * ivl_nexus_ptr_log
 *    If the target object is an ivl_net_logic_t, this method returns
 *    the object. Otherwise, this method returns 0.
 *
 * ivl_nexus_ptr_lpm
 *    If the target object is an ivl_lpm_t, this method returns the
 *    object. Otherwise, this method returns 0.
 *
 * ivl_nexus_ptr_sig
 *    If the target object is an ivl_signal_t, this method returns the
 *    object. If the target is not a signal, this method returns 0.
 */

extern const char*     ivl_nexus_name(ivl_nexus_t net);
extern unsigned        ivl_nexus_ptrs(ivl_nexus_t net);
extern ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx);

extern unsigned     ivl_nexus_ptr_pin(ivl_nexus_ptr_t net);
extern ivl_net_logic_t ivl_nexus_ptr_log(ivl_nexus_ptr_t net);
extern ivl_lpm_t    ivl_nexus_ptr_lpm(ivl_nexus_ptr_t net);
extern ivl_signal_t ivl_nexus_ptr_sig(ivl_nexus_ptr_t net);


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
 * ivl_scope_event
 * ivl_scope_events
 *    Scopes have 0 or more event objects in them.
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
 *
 * ivl_scope_type
 * ivl_scope_tname
 *    Scopes have a type and a type name. For example, if a scope is
 *    an instance of module foo, its type is IVL_SCT_MODULE and its
 *    type name is "foo". This is different from the instance name
 *    returned by ivl_scope_name above.
 */

extern int          ivl_scope_children(ivl_scope_t net,
				       ivl_scope_f func, void*cd);
extern unsigned     ivl_scope_events(ivl_scope_t net);
extern ivl_event_t  ivl_scope_event(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_logs(ivl_scope_t net);
extern ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_lpms(ivl_scope_t net);
extern ivl_lpm_t    ivl_scope_lpm(ivl_scope_t, unsigned idx);
extern const char*  ivl_scope_name(ivl_scope_t net);
extern unsigned     ivl_scope_sigs(ivl_scope_t net);
extern ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx);
extern ivl_scope_type_t ivl_scope_type(ivl_scope_t net);
extern const char*  ivl_scope_tname(ivl_scope_t net);


/* SIGNALS
 * Signals are named things in the Verilog source, like wires and
 * regs, and also named things that are preated as temporaries during
 * certain elaboration or optimization steps. A signal may also be a
 * port of a module or task.
 *
 * Signals have a name (obviously) and types. A signal may also be
 * signed or unsigned.
 *
 * ivl_signal_pins
 * ivl_signal_pin
 *    The ivl_signal_pin function returns the nexus connected to the
 *    signal. If the signal is a vectory, the idx can be a non-zero
 *    value, and the result is the nexus for the specified bit.
 *
 * ivl_signal_port
 *    If the signal is a port to a module, this function returns the
 *    port direction. If the signal is not a port, it returns
 *    IVL_SIP_NONE.
 *
 * ivl_signal_type
 *
 * ivl_signal_name
 *    This function returns the fully scoped hierarchical name for the
 *    signal. The name refers to the entire vector that is the signal.
 *
 * ivl_signal_basename
 *    This function returns the name of the signal, without the scope
 *    information. This is the tail of the signal name.
 *
 * ivl_signal_attr
 *    Icarus Verilog supports attaching attributes to signals, with
 *    the attribute value (a string) associated with a key. This
 *    function returns the attribute value for the given key. If the
 *    key does not exist, the function returns 0.
 */

extern ivl_nexus_t ivl_signal_pin(ivl_signal_t net, unsigned idx);
extern unsigned    ivl_signal_pins(ivl_signal_t net);
extern ivl_signal_port_t ivl_signal_port(ivl_signal_t net);
extern ivl_signal_type_t ivl_signal_type(ivl_signal_t net);
extern const char* ivl_signal_name(ivl_signal_t net);
extern const char* ivl_signal_basename(ivl_signal_t net);
extern const char* ivl_signal_attr(ivl_signal_t net, const char*key);


/*
 * These functions get information about a process. A process is
 * an initial or always block within the original Verilog source, that
 * is translated into a type and a single statement. (The statement
 * may be a compound statement.)
 *
 * The ivl_process_type function gets the type of the process,
 * an "inital" or "always" statement.
 *
 * A process is placed in a scope. The statement within the process
 * operates within the scope of the process unless there are calls
 * outside the scope.
 *
 * The ivl_process_stmt function gets the statement that forms the
 * process. See the statement related functions for how to manipulate
 * statements.
 */
extern ivl_process_type_t ivl_process_type(ivl_process_t net);

extern ivl_scope_t ivl_process_scope(ivl_process_t net);

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

  /* IVL_ST_BLOCK, IVL_ST_FORK */
extern unsigned ivl_stmt_block_count(ivl_statement_t net);
  /* IVL_ST_BLOCK, IVL_ST_FORK */
extern ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_CASE */
extern unsigned ivl_stmt_case_count(ivl_statement_t net);
  /* IVL_ST_CASE */
extern ivl_expr_t ivl_stmt_case_expr(ivl_statement_t net, unsigned i);
  /* IVL_ST_CASE */
extern ivl_statement_t ivl_stmt_case_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_CONDIT, IVL_ST_CASE IVL_ST_WHILE */
extern ivl_expr_t      ivl_stmt_cond_expr(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_false(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_true(ivl_statement_t net);
  /* IVL_ST_DELAY */
extern unsigned long ivl_stmt_delay_val(ivl_statement_t net);
  /* IVL_ST_WAIT */
extern ivl_event_t   ivl_stmt_event(ivl_statement_t net);
  /* IVL_ST_ASSIGN */
extern ivl_lval_t ivl_stmt_lval(ivl_statement_t net, unsigned idx);
  /* IVL_ST_ASSIGN */
extern unsigned ivl_stmt_lvals(ivl_statement_t net);
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



/* target_design

   The "target_design" function is called once after the whole design
   is processed and available to the target. The target doesn't return
   from this function until it is finished with the design.

   This function is implemented in the loaded target, and not in the
   ivl core. This function is how the target module is invoked. */

typedef int  (*target_design_f)(ivl_design_t des);


_END_DECL

/*
 * $Log: ivl_target.h,v $
 * Revision 1.44  2001/04/02 00:28:35  steve
 *  Support the scope expression node.
 *
 * Revision 1.43  2001/04/01 06:52:27  steve
 *  support the NetWhile statement.
 *
 * Revision 1.42  2001/04/01 04:38:17  steve
 *  dead cruft.
 *
 * Revision 1.41  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.40  2001/03/31 17:36:38  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.39  2001/03/30 05:49:52  steve
 *  Generate code for fork/join statements.
 *
 * Revision 1.38  2001/03/29 02:52:39  steve
 *  Add unary ~ operator to tgt-vvp.
 *
 * Revision 1.37  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.36  2001/03/27 06:27:40  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.35  2001/03/20 01:44:13  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.34  2001/01/15 22:05:14  steve
 *  Declare ivl_scope_type functions.
 *
 * Revision 1.33  2001/01/15 00:47:01  steve
 *  Pass scope type information to the target module.
 *
 * Revision 1.32  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.31  2001/01/06 06:31:58  steve
 *  declaration initialization for time variables.
 *
 * Revision 1.30  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.29  2000/11/12 17:47:29  steve
 *  flip-flop pins for ivl_target API.
 *
 * Revision 1.28  2000/11/11 01:52:09  steve
 *  change set for support of nmos, pmos, rnmos, rpmos, notif0, and notif1
 *  change set to correct behavior of bufif0 and bufif1
 *  (Tim Leight)
 *
 *  Also includes fix for PR#27
 *
 * Revision 1.27  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.26  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.25  2000/10/28 22:32:34  steve
 *  API for concatenation expressions.
 *
 * Revision 1.24  2000/10/28 17:55:03  steve
 *  stub for the concat operator.
 *
 * Revision 1.23  2000/10/25 05:41:24  steve
 *  Get target signal from nexus_ptr.
 *
 * Revision 1.22  2000/10/21 16:49:45  steve
 *  Reduce the target entry points to the target_design.
 *
 * Revision 1.21  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
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
