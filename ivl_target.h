#ifndef __ivl_target_H
#define __ivl_target_H
/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: ivl_target.h,v 1.144 2005/03/03 04:34:42 steve Exp $"
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
 * ivl_net_logic_t
 *    This object represents various built in logic devices. In fact,
 *    this includes just about every directional device that has a
 *    single output, including logic gates and nmos, pmos and cmos
 *    devices. There is also the occasional Icarus Verilog creation.
 *    What is common about these devices is that they are
 *    bitwise. That is, when fed a vector, they produce a vector
 *    result where each bit of the output is made only from the same
 *    bits in the vector inputs.
 *
 * ivl_nexus_t
 *    Structural links within an elaborated design are connected
 *    together at each bit. The connection point is a nexus, so pins
 *    of devices refer to an ivl_nexus_t. Furthermore, from a nexus
 *    there are backward references to all the device pins that point
 *    to it.
 *
 * ivl_parameter_t
 *    Scopes have zero or more parameter objects that represent
 *    parameters that the source defined. The parameter has a value
 *    that is fully elaborated, with defparams and other parameter
 *    overrides taken care of.
 *
 * ivl_process_t
 *    A Verilog process is represented by one of these. A process may
 *    be an "initial" or an "always" process. These come from initial
 *    or always statements from the Verilog source.
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
 *
 * basenames are the name of the object without the containing
 * scope. These names are unique within a scope, but not necessarily
 * throughout the design.
 */
typedef struct ivl_design_s   *ivl_design_t;
typedef struct ivl_event_s    *ivl_event_t;
typedef struct ivl_expr_s     *ivl_expr_t;
typedef struct ivl_lpm_s      *ivl_lpm_t;
typedef struct ivl_lval_s     *ivl_lval_t;
typedef struct ivl_net_const_s*ivl_net_const_t;
typedef struct ivl_net_logic_s*ivl_net_logic_t;
typedef struct ivl_udp_s      *ivl_udp_t;
typedef struct ivl_net_probe_s*ivl_net_probe_t;
typedef struct ivl_nexus_s    *ivl_nexus_t;
typedef struct ivl_nexus_ptr_s*ivl_nexus_ptr_t;
typedef struct ivl_parameter_s*ivl_parameter_t;
typedef struct ivl_process_s  *ivl_process_t;
typedef struct ivl_scope_s    *ivl_scope_t;
typedef struct ivl_signal_s   *ivl_signal_t;
typedef struct ivl_memory_s   *ivl_memory_t;
typedef struct ivl_statement_s*ivl_statement_t;
typedef struct ivl_variable_s *ivl_variable_t;

/*
 * These are types that are defined as enumerations. These have
 * explicit values so that the binary API is a bit more resilient to
 * changes and additions to the enumerations.
 */

typedef enum ivl_drive_e {
      IVL_DR_HiZ    = 0,
      IVL_DR_SMALL  = 1,
      IVL_DR_MEDIUM = 2,
      IVL_DR_WEAK   = 3,
      IVL_DR_LARGE  = 4,
      IVL_DR_PULL   = 5,
      IVL_DR_STRONG = 6,
      IVL_DR_SUPPLY = 7
} ivl_drive_t;

/* This is the type of an ivl_expr_t object. The explicit numbers
   allow additions to the enumeration without causing values to shift
   and incompatibilities to be introduced. */
typedef enum ivl_expr_type_e {
      IVL_EX_NONE = 0,
	/* IVL_EX_BITSEL = 1, */
      IVL_EX_BINARY = 2,
      IVL_EX_CONCAT = 3,
      IVL_EX_EVENT  = 17,
      IVL_EX_MEMORY = 4,
      IVL_EX_NUMBER = 5,
      IVL_EX_SCOPE  = 6,
      IVL_EX_SELECT = 7,
      IVL_EX_SFUNC  = 8,
      IVL_EX_SIGNAL = 9,
      IVL_EX_STRING = 10,
      IVL_EX_TERNARY = 11,
      IVL_EX_UFUNC = 12,
      IVL_EX_ULONG = 13,
      IVL_EX_UNARY = 14,
      IVL_EX_VARIABLE = 15,
      IVL_EX_REALNUM  = 16
} ivl_expr_type_t;

/* This is the type code for an ivl_net_logic_t object. */
typedef enum ivl_logic_e {
      IVL_LO_NONE   =  0,
      IVL_LO_AND    =  1,
      IVL_LO_BUF    =  2,
      IVL_LO_BUFIF0 =  3,
      IVL_LO_BUFIF1 =  4,
      IVL_LO_BUFZ   =  5,
      IVL_LO_NAND   =  6,
      IVL_LO_NMOS   =  7,
      IVL_LO_NOR    =  8,
      IVL_LO_NOT    =  9,
      IVL_LO_NOTIF0 = 10,
      IVL_LO_NOTIF1 = 11,
      IVL_LO_OR     = 12,
      IVL_LO_PULLDOWN  = 13,
      IVL_LO_PULLUP = 14,
      IVL_LO_RNMOS  = 15,
      IVL_LO_RPMOS  = 16,
      IVL_LO_PMOS   = 17,
      IVL_LO_XNOR   = 18,
      IVL_LO_XOR    = 19,

      IVL_LO_UDP    = 21
} ivl_logic_t;

/* This is the type of an LPM object. */
typedef enum ivl_lpm_type_e {
      IVL_LPM_ADD    =  0,
      IVL_LPM_CONCAT = 16,
      IVL_LPM_CMP_EEQ= 18, /* Case EQ (===) */
      IVL_LPM_CMP_EQ = 10,
      IVL_LPM_CMP_GE =  1,
      IVL_LPM_CMP_GT =  2,
      IVL_LPM_CMP_NE = 11,
      IVL_LPM_CMP_NEE= 19, /* Case NE (!==) */
      IVL_LPM_DIVIDE = 12,
      IVL_LPM_FF     =  3,
      IVL_LPM_MOD    = 13,
      IVL_LPM_MULT   =  4,
      IVL_LPM_MUX    =  5,
      IVL_LPM_PART_VP= 15, /* part select: vector to part */
      IVL_LPM_PART_PV= 17, /* part select: part written to vector */
      IVL_LPM_RE_AND = 20,
      IVL_LPM_RE_NAND= 21,
      IVL_LPM_RE_NOR = 22,
      IVL_LPM_RE_OR  = 23,
      IVL_LPM_RE_XNOR= 24,
      IVL_LPM_RE_XOR = 25,
      IVL_LPM_REPEAT = 26,
      IVL_LPM_SHIFTL =  6,
      IVL_LPM_SHIFTR =  7,
      IVL_LPM_SUB    =  8,
      IVL_LPM_RAM    =  9,
      IVL_LPM_UFUNC  = 14
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
      IVL_SIT_NONE = 0,
      IVL_SIT_REG  = 1,
      IVL_SIT_TRI  = 4,
      IVL_SIT_TRI0 = 5,
      IVL_SIT_TRI1 = 6,
      IVL_SIT_TRIAND = 7,
      IVL_SIT_TRIOR  = 8
} ivl_signal_type_t;

/* This is the type code for ivl_statement_t objects. */
typedef enum ivl_statement_type_e {
      IVL_ST_NONE    = 0,
      IVL_ST_NOOP    = 1,
      IVL_ST_ASSIGN    = 2,
      IVL_ST_ASSIGN_NB = 3,
      IVL_ST_BLOCK   = 4,
      IVL_ST_CASE    = 5,
      IVL_ST_CASER   = 24, /* Case statement with real expressions. */
      IVL_ST_CASEX   = 6,
      IVL_ST_CASEZ   = 7,
      IVL_ST_CASSIGN = 8,
      IVL_ST_CONDIT  = 9,
      IVL_ST_DEASSIGN = 10,
      IVL_ST_DELAY   = 11,
      IVL_ST_DELAYX  = 12,
      IVL_ST_DISABLE = 13,
      IVL_ST_FORCE   = 14,
      IVL_ST_FOREVER = 15,
      IVL_ST_FORK    = 16,
      IVL_ST_RELEASE = 17,
      IVL_ST_REPEAT  = 18,
      IVL_ST_STASK   = 19,
      IVL_ST_TRIGGER = 20,
      IVL_ST_UTASK   = 21,
      IVL_ST_WAIT    = 22,
      IVL_ST_WHILE   = 23
} ivl_statement_type_t;

/* This is the type of a variable, and also used as the type for an
   expression. */
typedef enum ivl_variable_type_e {
      IVL_VT_VOID = 0,  /* Not used */
      IVL_VT_REAL,
      IVL_VT_VECTOR
} ivl_variable_type_t;

/* This is the type of the function to apply to a process. */
typedef int (*ivl_process_f)(ivl_process_t net, void*cd);

/* This is the type of a function to apply to a scope. The ivl_scope_t
   parameter is the scope, and the cd parameter is client data that
   the user passes to the scanner. */
typedef int (ivl_scope_f)(ivl_scope_t net, void*cd);

/* Attributes, which can be attached to various object types, have
   this form. */
typedef enum ivl_attribute_type_e {
      IVL_ATT_VOID = 0,
      IVL_ATT_STR,
      IVL_ATT_NUM
} ivl_attribute_type_t;

struct ivl_attribute_s {
      const char*key;
      ivl_attribute_type_t type;
      union val_ {
	    const char*str;
	    long num;
      } val;
};
typedef const struct ivl_attribute_s*ivl_attribute_t;


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
 *    calls the user supplied function on each of the processes until
 *    one of the functors returns non-0 or all the processes are
 *    scanned. This function will return 0, or the non-zero value that
 *    was returned from the last scanned process.
 *
 * ivl_design_root
 *    A design has a root named scope that is an instance of the top
 *    level module in the design. This is a hook for naming the
 *    design, or for starting the scope scan.
 *
 * ivl_design_time_precision
 *    A design as a time precision. This is the size in seconds (a
 *    signed power of 10) of a simulation tick.
 */

extern const char* ivl_design_flag(ivl_design_t des, const char*key);
extern int         ivl_design_process(ivl_design_t des,
				      ivl_process_f fun, void*cd);
extern ivl_scope_t ivl_design_root(ivl_design_t des);
extern void        ivl_design_roots(ivl_design_t des,
				    ivl_scope_t **scopes,
				    unsigned int *nscopes);
extern int         ivl_design_time_precision(ivl_design_t des);

extern unsigned        ivl_design_consts(ivl_design_t des);
extern ivl_net_const_t ivl_design_const(ivl_design_t, unsigned idx);

/* VECTOR CONSTANTS
 * Vector constants are nodes with no input and a single vector
 * output. The output is an array of 4-value bits, using a single char
 * value for each bit. The bits of the vector are in canonical (lsb
 * first) order for the width of the constant.
 *
 * ivl_const_bits
 *    This returns a pointer to an array of conststant characters,
 *    each byte a '0', '1', 'x' or 'z'. The array is *not* nul
 *    terminated.
 *
 * ivl_const_nex
 *    Return the ivl_nexus_t of the output for the constant.
 *
 * ivl_const_signed
 *    Return true (!0) if the constant is a signed value, 0 otherwise.
 *
 * ivl_const_width
 *    Return the width, in logical bits, of the constant.
 *
 * ivl_const_pin
 * ivl_const_pins
 *    DEPRECATED
 */
extern const char* ivl_const_bits(ivl_net_const_t net);
extern ivl_nexus_t ivl_const_nex(ivl_net_const_t net);
extern int         ivl_const_signed(ivl_net_const_t net);
extern unsigned    ivl_const_width(ivl_net_const_t net);

/* extern ivl_nexus_t ivl_const_pin(ivl_net_const_t net, unsigned idx); */
/* extern unsigned    ivl_const_pins(ivl_net_const_t net); */

/* EVENTS
 *
 * Events are a unification of named events and implicit events
 * generated by the @ statements.
 *
 * FUNCTION SUMMARY
 *
 * ivl_event_name (Obsolete)
 * ivl_event_basename
 *    Return the name of the event. The basename is the name within
 *    the scope, as declared by the user or generated by elaboration.
 *
 * ivl_event_scope
 *    All events exist within a scope.
 *
 * SEMANTICS NOTES
 *
 * Named events (i.e. event objects declared by the Verilog
 * declaration "event foo") are recognized by the fact that they have
 * no edge sources. The name of the event as given in the Verilog
 * source is available from the ivl_event_basename function.
 *
 * Named events are referenced in trigger statements.
 *
 * Edge events are created implicitly by the @(...) Verilog syntax to
 * watch for the correct type of edge for the functor being
 * watched. The nodes to watch are collected into groups based on the
 * type of edge to be watched for on that node. For example, nodes to
 * be watched for positive edges are accessed via the ivl_event_npos
 * and ivl_event_pos functions.
 */
extern const char* ivl_event_name(ivl_event_t net);
extern const char* ivl_event_basename(ivl_event_t net);
extern ivl_scope_t ivl_event_scope(ivl_event_t net);

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
 * ivl_expr_value
 *    Get the data type of the expression node. This uses the variable
 *    type enum to express the type of the expression node.
 *
 * ivl_expr_width
 *    This method returns the bit width of the expression at this
 *    node. It can be applied to any expression node, and returns the
 *    *output* width of the expression node.
 *
 * ivl_expr_parameter
 *    This function returns the ivl_parameter_t object that represents
 *    this object, or 0 (nil) if it is not a parameter value. This
 *    function allows the code generator to detect the case where the
 *    expression is a parameter. This will normally only return a
 *    non-nil value for constants.
 *
 * ivl_expr_opcode
 *    IVL_EX_BINARY and IVL_EX_UNARY expression nodes include an
 *    opcode from this table:
 *              &   -- AND
 *              A   -- NAND (~&)
 *              X   -- XNOR (~^)
 *
 * SEMANTIC NOTES
 *
 * - IVL_EX_SELECT
 * This expression takes two operands, oper1 is the expression to
 * select from, and oper2 is the selection base. The ivl_expr_width
 * value is the width of the bit/part select. The ivl_expr_oper1 value
 * is the base of a vector. The compiler has already figured out any
 * conversion from signal units to vector units, so the result of
 * ivl_expr_oper1 should range from 0 to ivl_expr_width().
 */

extern ivl_expr_type_t ivl_expr_type(ivl_expr_t net);
extern ivl_variable_type_t ivl_expr_value(ivl_expr_t net);

  /* IVL_EX_NUMBER */
extern const char* ivl_expr_bits(ivl_expr_t net);
  /* IVL_EX_UFUNC */
extern ivl_scope_t ivl_expr_def(ivl_expr_t net);
  /* IVL_EX_REALNUM */
extern double ivl_expr_dvalue(ivl_expr_t net);
  /* IVL_EX_SIGNAL, IVL_EX_SFUNC, IVL_EX_VARIABLE */
extern const char* ivl_expr_name(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_UNARY */
extern char        ivl_expr_opcode(ivl_expr_t net);
  /* IVL_EX_BINARY  IVL_EX_UNARY, IVL_EX_MEMORY IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper1(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper2(ivl_expr_t net);
  /* IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper3(ivl_expr_t net);
  /* and expression */
extern ivl_parameter_t ivl_expr_parameter(ivl_expr_t net);
  /* IVL_EX_CONCAT IVL_EX_UFUNC */
extern ivl_expr_t  ivl_expr_parm(ivl_expr_t net, unsigned idx);
  /* IVL_EX_CONCAT IVL_EX_SFUNC IVL_EX_UFUNC */
extern unsigned    ivl_expr_parms(ivl_expr_t net);
  /* IVL_EX_CONCAT */
extern unsigned    ivl_expr_repeat(ivl_expr_t net);
  /* IVL_EX_EVENT */
extern ivl_event_t ivl_expr_event(ivl_expr_t net);
  /* IVL_EX_SCOPE */
extern ivl_scope_t ivl_expr_scope(ivl_expr_t net);
  /* IVL_EX_SIGNAL */
extern ivl_signal_t ivl_expr_signal(ivl_expr_t net);
  /* any expression */
extern int         ivl_expr_signed(ivl_expr_t net);
  /* IVL_EX_STRING */
extern const char* ivl_expr_string(ivl_expr_t net);
  /* IVL_EX_ULONG */
extern unsigned long ivl_expr_uvalue(ivl_expr_t net);
  /* IVL_EX_VARIABLE */
extern ivl_variable_t ivl_expr_variable(ivl_expr_t net);
  /* any expression */
extern unsigned    ivl_expr_width(ivl_expr_t net);

/*
 * Memory.
 * Memories are declared in Verilog source as 2 dimensional arrays of
 * reg bits. This means that they are simple arrays of vectors. The
 * vectors in the array are called "words".
 *
 * ivl_memory_name (DEPRECATED)
 *
 * ivl_memory_basename
 *    This returns the base name of the memory object. The base name
 *    does not include the name of the scopes that contains the object.
 *
 * ivl_memory_root
 *    The root of the memory is the value to add to a calculated
 *    address to get to a canonical (0-based) address. This value is
 *    used when external code wishes to access a word. All the
 *    compiled references to the word within the compiled design are
 *    converted to cannonical form by the compiler.
 *
 * ivl_memory_size
 * ivl_memory_width
 *    These functions return the dimensions of the memory. The size is
 *    the number of words in the memory, and the width is the number
 *    of bits in each word.
 *
 * ivl_memory_scope
 *    This returns the scope that contains the memory.
 */

extern const char*ivl_memory_basename(ivl_memory_t net);
extern int ivl_memory_root(ivl_memory_t net);
extern ivl_scope_t ivl_memory_scope(ivl_memory_t net);
extern unsigned ivl_memory_size(ivl_memory_t net);
extern unsigned ivl_memory_width(ivl_memory_t net);

extern ivl_memory_t ivl_expr_memory(ivl_expr_t net);

/* LOGIC
 * These types and functions support manipulation of logic gates. The
 * ivl_logic_t enumeration identifies the various kinds of gates that
 * the ivl_net_logic_t can represent. The various functions then
 * provide access to the bits of information for a given logic device.
 *
 * The ivl_net_logic_t nodes are bit-slice devices. That means that
 * the device may have width (and therefore processes vectors) but
 * each bit slice of the width is independent.
 *
 * ivl_logic_type
 *    This method returns the type of logic gate that the node
 *    represents. The logic type implies the meaning of the various pins.
 *
 * ivl_logic_name (obsolete)
 *    This method returns the complete name of the logic gate. Every
 *    gate has a complete name (that includes the scope) even if the
 *    Verilog source doesn't include one. The compiler will choose one
 *    if necessary.
 *
 * ivl_logic_basename
 *    This is the name of the gate without the scope part.
 *
 * ivl_logic_scope
 *    This is the scope that directly contains the logic device.
 *
 * ivl_logic_pins
 * ivl_logic_pin
 *    Return the nexus for the pin. If two pins are connected
 *    together, then these values are the same. Use the nexus
 *    functions to find other pins that are connected to this nexus.
 *
 * ivl_logic_width
 *    This returns the width of the logic array. This does not affect
 *    the number of pins, but implies the width of the vector at each
 *    pin.
 *
 * ivl_logic_attr (obsolete)
 *    Return the value of a specific attribute, given the key name as
 *    a string. If the key is not defined, then return 0 (null).
 *
 * ivl_logic_attr_cnt
 * ivl_logic_attr_val
 *    These support iterating over logic attributes. The _cnt method
 *    returns the number of attributes attached to the gate, and the
 *    ivl_logic_attr_val returns the value of the attribute.
 *
 * SEMANTIC NOTES
 * The ivl_logic_width applies to all the pins of a logic device. If a
 * logic device has width, that means that it is actually an array of
 * logic devices tha each process a bit slice of the
 * inputs/output. That implies that the widths of all the inputs and
 * the output must be identical.
 *
 * The ivl_logic_width  and ivl_logic_pins are *not* related. A logic
 * device has a number of pins that is the number of inputs to a logic
 * array of identical gates, and the ivl_logic_width, is the width of
 * the vector into each input pin and out of the output pin.
 *
 * - IVL_LO_PULLUP/IVL_LO_PULLDOWN
 * These devices are grouped as logic devices with zero inputs because
 * the outputs have the same characteristics as other logic
 * devices. They are special only in that they have zero inputs, and
 * their drivers typically have strength other then strong.
 */

extern const char* ivl_logic_name(ivl_net_logic_t net);
extern const char* ivl_logic_basename(ivl_net_logic_t net);
extern ivl_scope_t ivl_logic_scope(ivl_net_logic_t net);
extern ivl_logic_t ivl_logic_type(ivl_net_logic_t net);
extern ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin);
extern unsigned    ivl_logic_pins(ivl_net_logic_t net);
extern ivl_udp_t   ivl_logic_udp(ivl_net_logic_t net);
extern unsigned    ivl_logic_delay(ivl_net_logic_t net, unsigned transition);
extern unsigned    ivl_logic_width(ivl_net_logic_t net);

  /* DEPRECATED */
extern const char* ivl_logic_attr(ivl_net_logic_t net, const char*key);

extern unsigned        ivl_logic_attr_cnt(ivl_net_logic_t net);
extern ivl_attribute_t ivl_logic_attr_val(ivl_net_logic_t net, unsigned idx);

/* UDP
 *
 */

extern unsigned    ivl_udp_sequ(ivl_udp_t net);
extern unsigned    ivl_udp_nin(ivl_udp_t net);
extern unsigned    ivl_udp_init(ivl_udp_t net);
extern const char* ivl_udp_row(ivl_udp_t net, unsigned idx);
extern unsigned    ivl_udp_rows(ivl_udp_t net);
extern const char* ivl_udp_name(ivl_udp_t net);


/* LPM
 * These functions support access to the properties of LPM
 * devices. LPM devices are a variety of devices that handle more
 * complex structural semantics. They are based on EIA LPM standard
 * devices, but vary to suite the technical situation.
 *
 * These are the functions that apply to all LPM devices:
 *
 * ivl_lpm_name (Obsolete)
 * ivl_lpm_basename
 *    Return the name of the device. The name is the name of the
 *    device with the scope part, and the basename is without the scope.
 *
 * ivl_lpm_scope
 *    LPM devices exist within a scope. Return the scope that contains
 *    this device.
 *
 * ivl_lpm_type
 *    Return the ivl_lpm_type_t of the specific LPM device.
 *
 * ivl_lpm_width
 *    Return the width of the LPM device. What this means depends on
 *    the LPM type, but it generally has to do with the width of the
 *    output data path.
 *
 *
 * These functions apply to a subset of the LPM devices, or may have
 * varying meaning depending on the device:
 *
 * ivl_lpm_base
 *    The IVL_LPM_PART objects use this value as the base (first bit)
 *    of the part select. The ivl_lpm_width is the size of the part.
 *
 * ivl_lpm_data
 *    Return the input data nexus for device types that have input
 *    vectors. The "idx" parameter selects which data input is selected.
 *
 * ivl_lpm_datab (ANACHRONISM)
 *    This is the same as ivl_lpm_data(net,1), in other words the
 *    second data input. Use the ivl_lpm_data method instead.
 *
 * ivl_lpm_q
 *    Return the output data nexus for device types that have a single
 *    output vector. This is most devices, it turns out.
 *
 * ivl_lpm_selects
 *    This is the size of the select input for a LPM_MUX device, or the
 *    address bus width of an LPM_RAM.
 *
 * ivl_lpm_signed
 *    Arithmetic LPM devices may be signed or unsigned if there is a
 *    distinction. For some devices this gives the signedness of the
 *    output, but not all devices.
 *
 * ivl_lpm_size
 *    In addition to a width, some devices have a size. The size is
 *    often the number of inputs per out, i.e., the number of inputs
 *    per bit for a MUX.
 *
 * SEMANTIC NOTES
 *
 * - Concatenation (IVL_LPM_CONCAT)
 * These devices take vectors in and combine them to form a single
 * output the width specified by ivl_lpm_width.
 *
 * The ivl_lpm_q nexus is the output from the concatenation.
 *
 * The ivl_lpm_data function returns the connections for the inputs to
 * the concatentation. The ivl_lpm_size function returns the number of
 * inputs help by the device.
 *
 * - Divide (IVL_LPM_DIVIDE)
 * The divide operators take two inputs and generate an output. The
 * ivl_lpm_width returns the width of the result. The width of the
 * inputs are their own.
 *
 * - Multiply (IVL_LPM_MULT)
 * The multiply takes two inputs and generates an output. Unlike other
 * arithmetic nodes, the width only refers to the output. The inputs
 * have independent widths, to reflect the arithmetic truth that the
 * width of a general multiply is the sum of the widths of the
 * inputs. In fact, the compiler doesn't assure that the widths of the
 * inputs add up to the width of the output, but the possibility
 * exists. It is *not* an error for the sum of the input widths to be
 * more then the width of the output, although the possibility of
 * overflow exists at run time.
 *
 * Multiply may be signed. If so, the output should be sign extended
 * to fill in its result.
 *
 * - Part Select (IVL_LPM_PART_VP and IVL_LPM_PART_PV)
 * There are two part select devices, one that extracts a part from a
 * vector, and another that writes a part of a vector. The _VP is
 * Vector-to-Part, and _PV is Part-to-Vector. The _VP form is meant to
 * model part/bin selects in r-value expressions, where the _PV from
 * is meant to model part selects in l-value nets.
 *
 * In both cases, ivl_lpm_data is the input pin, and ivl_lpm_q is the
 * output. In the case of the _VP device, the vector is input and the
 * part is the output. In the case of the _PV device, the part is the
 * input and the vector is the output.
 *
 * Also in both cases, the width of the device is the width of the
 * part. In the _VP case, this is obvious as the output nexus has the
 * part width. In the _PV case, this is a little less obvious, but
 * still correct. The output being written to the wider vector is
 * indeed the width of the part, even though it is written to a wider
 * gate. The target will need to handle this case specially.
 *
 * - Comparisons (IVL_LPM_CMP_GT/GE/EQ/NE/EEQ/NEE)
 * These devices have two inputs, available by the ivl_lpm_data()
 * function, and one output available by the ivl_lpm_q function. The
 * output width is always 1, but the ivl_lpm_width() returns the width
 * of the inputs. Both inputs must have the same width.
 *
 * The CMP_GE and CMP_GT nodes may also be signed or unsigned, with
 * the obvious implications. The widths are matched by the compiler
 * (so the target need not worry about sign extension) but when doing
 * magnitude compare, the signedness does matter. In any case, the
 * result of the compare is always unsigned.
 *
 * - Mux Device (IVL_LPM_MUX)
 * The MUX device has a q output, a select input, and a number of data
 * inputs. The ivl_lpm_q output and the ivl_lpm_data inputs all have
 * the width from the ivl_lpm_width() method. The Select input, from
 * ivl_lpm_select, has the width ivl_lpm_selects().
 *
 * The ivl_lpm_data() method returns the inputs of the MUX device. The
 * ivl_lpm_size() method returns the number of data inputs there
 * are. All the data inputs have the same width, the width of the
 * ivl_lpm_q output.
 *
 * - Reduction operators (IVL_LPM_RE_*)
 * These devices have one input, a vector, and generate a single bit
 * result. The width from the ivl_lpm_width is the width of the input
 * vector.
 *
 * - Repeat Node (IVL_LPM_REPEAT)
 * This node takes as input a single vector, and outputs a single
 * vector. The ivl_lpm_width if this node is the width of the *output*
 * vector. The ivl_lpm_size() returns the number of times the input is
 * repeated to get the desired width. The ivl core assures that the
 * input vector is exactly ivl_lpm_width() / ivl_lpm_size() bits.
 *
 * - Shifts (IVL_LPM_SHIFTL/SHIFTR)
 * This node takes two inputs, a vector and a shift distance. The
 * ivl_lpm_data(0) nexus is the vector input, and the ivl_lpm_data(1)
 * the shift distance. The vector input is the same width as the
 * output, but the distance has its own width.
 */

extern const char*    ivl_lpm_name(ivl_lpm_t net); /* (Obsolete) */
extern const char*    ivl_lpm_basename(ivl_lpm_t net);
extern ivl_scope_t    ivl_lpm_scope(ivl_lpm_t net);
extern int            ivl_lpm_signed(ivl_lpm_t net);
extern ivl_lpm_type_t ivl_lpm_type(ivl_lpm_t net);
extern unsigned       ivl_lpm_width(ivl_lpm_t net);

  /* IVL_LPM_FF */
extern ivl_nexus_t ivl_lpm_async_clr(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_async_set(ivl_lpm_t net);
extern ivl_expr_t  ivl_lpm_aset_value(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_sync_clr(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_sync_set(ivl_lpm_t net);
extern ivl_expr_t  ivl_lpm_sset_value(ivl_lpm_t net);
  /* IVL_LPM_PART */
extern unsigned ivl_lpm_base(ivl_lpm_t net);
  /* IVL_LPM_FF IVL_LPM_RAM */
extern ivl_nexus_t ivl_lpm_clk(ivl_lpm_t net);
  /* IVL_LPM_UFUNC */
extern ivl_scope_t  ivl_lpm_define(ivl_lpm_t net);
  /* IVL_LPM_FF IVL_LPM_RAM */
extern ivl_nexus_t ivl_lpm_enable(ivl_lpm_t net);
  /* IVL_LPM_ADD IVL_LPM_CONCAT IVL_LPM_FF IVL_LPM_PART IVL_LPM_MULT
     IVL_LPM_MUX IVL_LPM_RAM IVL_LPM_SHIFTL IVL_LPM_SHIFTR IVL_LPM_SUB */
extern ivl_nexus_t ivl_lpm_data(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_ADD IVL_LPM_MULT IVL_LPM_SUB */
extern ivl_nexus_t ivl_lpm_datab(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_UFUNC */
extern ivl_nexus_t ivl_lpm_data2(ivl_lpm_t net, unsigned sdx, unsigned idx);
  /* IVL_LPM_UFUNC */
extern unsigned ivl_lpm_data2_width(ivl_lpm_t net, unsigned sdx);
  /* IVL_LPM_ADD IVL_LPM_FF IVL_LPM_MULT IVL_LPM_PART IVL_LPM_RAM
     IVL_LPM_SUB IVL_LPM_UFUNC */
extern ivl_nexus_t ivl_lpm_q(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_MUX IVL_LPM_RAM */
extern unsigned ivl_lpm_selects(ivl_lpm_t net);
  /* IVL_LPM_MUX IVL_LPM_RAM */
extern ivl_nexus_t ivl_lpm_select(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_CONCAT IVL_LPM_MUX IVL_LPM_REPEAT */
extern unsigned ivl_lpm_size(ivl_lpm_t net);
  /* IVL_LPM_RAM */
extern ivl_memory_t ivl_lpm_memory(ivl_lpm_t net);


/* LVAL
 * The l-values of assignments are concatenation of ivl_lval_t
 * objects. Each lvi_lval_t object is an assignment to a var or a
 * memory, through a bit select, part select or word select.
 *
 * Var lvals are things like assignments to a part select or a bit
 * select. Assignment to the whole variable is a special case of a
 * part select, as is a bit select with a constant expression.
 *
 * ivl_lval_width
 *    The width of a vector that this lval can receive. This accounts
 *    for the local part selecting I might to in the lval object, as
 *    well as the target object width.
 *
 * ivl_lval_mux
 *    If the l-value includes a bit select expression, this method
 *    returns an ivl_expr_t that represents that
 *    expression.  Otherwise, it returns 0.
 *
 * ivl_lval_mem
 *    If the l-value is a memory, this method returns an
 *    ivl_memory_t that represents that memory. Otherwise, it
 *    returns 0.
 *
 * ivl_lval_sig
 *    If the l-value is a variable, this method returns the signal
 *    object that is the target of the assign.
 *
 * ivl_lval_var
 *    If the l-value is a non-signal variable (i.e. a real) this
 *    method returns the ivl_variable_t object that represents it.
 *    If the lval is this sort of variable, then the part_off, idx and
 *    pin methods do not apply.
 *
 * ivl_lval_part_off
 *    The part select of the signal is based here. This is the
 *    canonical index of bit-0 of the part select.
 *
 * ivl_lval_idx
 *    If the l-value is a memory, this method returns an
 *    ivl_expr_t that represents the index expression.  Otherwise, it
 *    returns 0.
 *
 * SEMANTIC NOTES
 * The ivl_lval_width is not necessarily the same as the width of the
 * signal or memory word it represents. It is the width of the vector
 * it receives and assigns. This may be less then the width of the
 * signal (or even 1) if only a part of the l-value signal is to be
 * assigned.
 *
 * The ivl_lval_part_off is the cannonical base of a constant part or
 * bit select. If the bit select base is non-constant, then the
 * ivl_lval_mux will contain an expression. If there is a mux
 * expression, then the ivl_lval_part_off result can be ignored.
 */

extern unsigned    ivl_lval_width(ivl_lval_t net);
extern ivl_expr_t  ivl_lval_mux(ivl_lval_t net);
extern ivl_expr_t  ivl_lval_idx(ivl_lval_t net);
extern ivl_memory_t ivl_lval_mem(ivl_lval_t net);
extern ivl_variable_t ivl_lval_var(ivl_lval_t net);
extern unsigned    ivl_lval_part_off(ivl_lval_t net);
extern ivl_signal_t ivl_lval_sig(ivl_lval_t net);
#if 0
extern unsigned    ivl_lval_pins(ivl_lval_t net);
extern ivl_nexus_t ivl_lval_pin(ivl_lval_t net, unsigned idx);
#endif

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
 * ivl_nexus_set_private
 * ivl_nexus_get_private
 *    The target module often needs to associate data with a nexus for
 *    later use when the nexus is encountered associated with a
 *    device. These methods allow the code generator to store to or
 *    retrieve from a nexus a void* of private data. This pointer is
 *    guaranteed to be 0 before the target module is invoked.
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
 *    is up to the application to find the pin that refers to this
 *    nexus. The problem is that LPM devices do not have a pinout per
 *    se, the pins all have specific names.
 *
 * ivl_nexus_ptr_con
 *    If this is a pointer to a magic constant device, then this
 *    returns the net_const object.
 *
 * ivl_nexus_ptr_drive0
 * ivl_nexus_ptr_drive1
 *    These are the 0 and 1 strength values for the devices. For most
 *    devices, these values are fixed by the description in the
 *    original source, with the default as IVL_DR_STRONG. For pins
 *    that are input only, drive0 and drive1 are both IVL_DR_HiZ.
 *
 *    The strength of strength-aware devices (such as nmos devices)
 *    does not really matter, as long at the output is not
 *    IVL_DR_HiZ. Testing for HiZ drivers is how code generators
 *    detect inputs.
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
 *
 * SEMANTIC NOTES
 * All the device pins that connect to a nexus have the same
 * type. That means, for example, that vector pins have the same
 * width. The compiler will insure this is so.
 */

extern const char*     ivl_nexus_name(ivl_nexus_t net);
extern unsigned        ivl_nexus_ptrs(ivl_nexus_t net);
extern ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx);

extern void  ivl_nexus_set_private(ivl_nexus_t net, void*data);
extern void* ivl_nexus_get_private(ivl_nexus_t net);


extern ivl_drive_t  ivl_nexus_ptr_drive0(ivl_nexus_ptr_t net);
extern ivl_drive_t  ivl_nexus_ptr_drive1(ivl_nexus_ptr_t net);
extern unsigned     ivl_nexus_ptr_pin(ivl_nexus_ptr_t net);
extern ivl_net_const_t ivl_nexus_ptr_con(ivl_nexus_ptr_t net);
extern ivl_net_logic_t ivl_nexus_ptr_log(ivl_nexus_ptr_t net);
extern ivl_lpm_t    ivl_nexus_ptr_lpm(ivl_nexus_ptr_t net);
extern ivl_signal_t ivl_nexus_ptr_sig(ivl_nexus_ptr_t net);

/* PARAMETER
 * Parameters are named constants associated with a scope. The user
 * may set in the Verilog source the value of parameters, and that
 * leads to ivl_parameter_t objects contained in the ivl_scope_t
 * objects.
 *
 * Parameters are essentially named constants. These constant values
 * can be accessed by looking at the scope (using ivl_scope_param) or
 * they can be discovered when they are used, via the
 * ivl_expr_parameter function. The fact that a constant has a name
 * (i.e. is a parameter) does not otherwise impose on the value or
 * interpretation of the constant expression so far as ivl_target is
 * concerned. The target may need this information, or may choose to
 * completely ignore it.
 *
 * ivl_parameter_basename
 *    return the name of the parameter.
 *
 * ivl_parameter_scope
 *    Return the scope of the parameter. The parameter name is only
 *    unique within its scope.
 *
 * ivl_parameter_expr
 *    Return the value of the parameter. This should be a simple
 *    constant expression, an IVL_EX_STRING or IVL_EX_NUMBER.
 */
extern const char* ivl_parameter_basename(ivl_parameter_t net);
extern ivl_scope_t ivl_parameter_scope(ivl_parameter_t net);
extern ivl_expr_t  ivl_parameter_expr(ivl_parameter_t net);


/* SCOPE
 * Scopes of various sort have these properties. Use these methods to
 * access them. Scopes come to exist in the elaborated design
 * generally when a module is instantiated, though they also come from
 * named blocks, tasks and functions.
 *
 * - module instances (IVL_SCT_MODULE)
 *    A module instance scope may contain events, logic gates, lpm
 *    nodes, signals, and possibly children. The children are further
 *    instances, or function/task scopes. Module instances do *not*
 *    contain a definition.
 *
 * - function scopes (IVL_SCT_FUNCTION)
 *    These scopes represent functions. A function may not be a root,
 *    so it is contained within a module instance scope. A function is
 *    required to have a definition (in the form of a statement) and a
 *    signal (IVL_SIG_REG) that is its return value.
 *
 *    A single function scope is created each time the module with the
 *    definition is instantiated.
 *
 *
 * - task scopes (IVL_SCT_TASK)
 *    [...]
 *
 * ivl_scope_attr_cnt
 * ivl_scope_attr_val
 *    A scope may have attributes attached to it. These functions
 *    allow the target to access the attributes values.
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
 * ivl_scope_def
 *    Task definition scopes carry a task definition, in the form of
 *    a statement. This method accesses that definition.
 *
 * ivl_scope_event
 * ivl_scope_events
 *    Scopes have 0 or more event objects in them.
 *
 * ivl_scope_var
 * ivl_scope_vars
 *    Scopes have 0 or more variable objects in them.
 *
 * ivl_scope_log
 * ivl_scope_logs
 *    Scopes have 0 or more logic devices in them. A logic device is
 *    represented by ivl_logic_t.
 *
 * ivl_scope_lpm
 * ivl_scope_lpms
 *    Scopes have 0 or more LPM devices in them. These functions access
 *    those devices.
 *
 * ivl_scope_name
 * ivl_scope_basename
 *    Every scope has a hierarchical name. This name is also a prefix
 *    of all the names of objects contained within the scope. The
 *    ivl_scope_basename is the name of the scope without the included
 *    hierarchy.
 *
 * ivl_scope_param
 * ivl_scope_params
 *    A scope has zero or more named parameters. These parameters have
 *    a name and an expression value.
 *
 * ivl_scope_parent
 *    If this is a non-root scope, then the parent is the scope that
 *    contains this scope. Otherwise, the parent is nil.
 *
 * ivl_scope_port
 * ivl_scope_ports
 *    Scopes that are functions or tasks have ports defined by
 *    signals. These methods access the ports by name.
 *
 *    If this scope represents a function, then the ports list
 *    includes the return value, as port 0. The remaining ports are
 *    the input ports in order.
 *
 * ivl_scope_sig
 * ivl_scope_sigs
 *    Scopes have 0 or more signals in them. These signals are
 *    anything that can become and ivl_signal_t, include synthetic
 *    signals generated by the compiler.
 *
 * ivl_scope_time_units
 *    Scopes have their own intrinsic time units, typically from the
 *    timescale compiler directive. This method returns the units as a
 *    signed power of 10 value.
 *
 * ivl_scope_type
 * ivl_scope_tname
 *    Scopes have a type and a type name. For example, if a scope is
 *    an instance of module foo, its type is IVL_SCT_MODULE and its
 *    type name is "foo". This is different from the instance name
 *    returned by ivl_scope_name above.
 */

extern unsigned        ivl_scope_attr_cnt(ivl_scope_t net);
extern ivl_attribute_t ivl_scope_attr_val(ivl_scope_t net, unsigned idx);

extern int          ivl_scope_children(ivl_scope_t net,
				       ivl_scope_f func, void*cd);

extern ivl_statement_t ivl_scope_def(ivl_scope_t net);

extern unsigned     ivl_scope_events(ivl_scope_t net);
extern ivl_event_t  ivl_scope_event(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_logs(ivl_scope_t net);
extern ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_lpms(ivl_scope_t net);
extern ivl_lpm_t    ivl_scope_lpm(ivl_scope_t, unsigned idx);
extern unsigned     ivl_scope_mems(ivl_scope_t net);
extern ivl_memory_t ivl_scope_mem(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_vars(ivl_scope_t net);
extern ivl_variable_t ivl_scope_var(ivl_scope_t net, unsigned idx);
extern const char*  ivl_scope_name(ivl_scope_t net);
extern const char*  ivl_scope_basename(ivl_scope_t net);
extern unsigned     ivl_scope_params(ivl_scope_t net);
extern ivl_parameter_t ivl_scope_param(ivl_scope_t net, unsigned idx);
extern ivl_scope_t  ivl_scope_parent(ivl_scope_t net);
extern unsigned     ivl_scope_ports(ivl_scope_t net);
extern ivl_signal_t ivl_scope_port(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_sigs(ivl_scope_t net);
extern ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx);
extern ivl_scope_type_t ivl_scope_type(ivl_scope_t net);
extern const char*  ivl_scope_tname(ivl_scope_t net);
extern int          ivl_scope_time_units(ivl_scope_t net);


/* SIGNALS
 * Signals are named things in the Verilog source, like wires and
 * regs, and also named things that are created as temporaries during
 * certain elaboration or optimization steps. A signal may also be a
 * port of a module or task.
 *
 * Signals have a name (obviously) and types. A signal may also be
 * signed or unsigned.
 *
 * ivl_signal_pins (replace these with ivl_signal_nex)
 * ivl_signal_pin
 *    The ivl_signal_pin function returns the nexus connected to the
 *    signal. If the signal is a vector, the idx can be a non-zero
 *    value, and the result is the nexus for the specified bit.
 *
 * ivl_signal_nex
 *    This is the nexus of the signal. This is used for managing
 *    connections to the rest of the net. There is exactly one pin for
 *    a signal. If the signal represents a vector, then the entire
 *    vector is carried through the single output.
 *
 * ivl_signal_msb
 * ivl_signal_lsb
 * ivl_signal_width
 *    These functions return the left and right indices, respectively,
 *    of the signal. If the signal is a scalar, both return 0. However,
 *    it doesn't mean that the signal is a scalar if both return 0, one
 *    can have a vector with 0 as both indices.
 *
 * ivl_signal_port
 *    If the signal is a port to a module, this function returns the
 *    port direction. If the signal is not a port, it returns
 *    IVL_SIP_NONE.
 *
 * ivl_signal_signed
 *    A signal, which is a vector, may be signed. In Verilog 2000, any
 *    net or variable may be signed. This function returns true if the
 *    signal is signed.
 *
 * ivl_signal_local
 *    A signal that was generated by the compiler as a place holder is
 *    marked as local.
 *
 * ivl_signal_type
 *    Return the type of the signal, i.e., reg, wire, tri0, etc.
 *
 * ivl_signal_name (DEPRECATED)
 *    This function returns the fully scoped hierarchical name for the
 *    signal. The name refers to the entire vector that is the signal.
 *
 *    NOTE: This function is deprecated. The hierarchical name is too
 *    vague a construct when escaped names can have . characters in
 *    them. Do no use this function in new code, it will disappear.
 *
 * ivl_signal_basename
 *    This function returns the name of the signal, without the scope
 *    information. This is the tail of the signal name. Since Verilog
 *    has an escape syntax, this name can contain any ASCII
 *    characters, except NULL or white space. The leading \ and
 *    trailing ' ' of escaped names in Verilog source are not part of
 *    the name, so not included here.
 *
 * ivl_signal_attr
 *    Icarus Verilog supports attaching attributes to signals, with
 *    the attribute value (a string) associated with a key. This
 *    function returns the attribute value for the given key. If the
 *    key does not exist, the function returns 0.
 */

extern ivl_nexus_t ivl_signal_nex(ivl_signal_t net);
extern int         ivl_signal_msb(ivl_signal_t net);
extern int         ivl_signal_lsb(ivl_signal_t net);
extern unsigned    ivl_signal_width(ivl_signal_t net);
extern ivl_signal_port_t ivl_signal_port(ivl_signal_t net);
extern int         ivl_signal_signed(ivl_signal_t net);
extern int         ivl_signal_integer(ivl_signal_t net);
extern int         ivl_signal_local(ivl_signal_t net);
extern ivl_signal_type_t ivl_signal_type(ivl_signal_t net);
extern const char* ivl_signal_name(ivl_signal_t net);
extern const char* ivl_signal_basename(ivl_signal_t net);
extern const char* ivl_signal_attr(ivl_signal_t net, const char*key);

extern unsigned        ivl_signal_attr_cnt(ivl_signal_t net);
extern ivl_attribute_t ivl_signal_attr_val(ivl_signal_t net, unsigned idx);

/* ivl_nexus_t ivl_signal_pin(ivl_signal_t net, unsigned idx); */
/* unsigned    ivl_signal_pins(ivl_signal_t net); */

/*
 * These functions get information about a process. A process is
 * an initial or always block within the original Verilog source, that
 * is translated into a type and a single statement. (The statement
 * may be a compound statement.)
 *
 * The ivl_process_type function gets the type of the process,
 * an "initial" or "always" statement.
 *
 * A process is placed in a scope. The statement within the process
 * operates within the scope of the process unless there are calls
 * outside the scope.
 *
 * The ivl_process_stmt function gets the statement that forms the
 * process. See the statement related functions for how to manipulate
 * statements.
 *
 * Processes can have attributes attached to them. the attr_cnt and
 * attr_val methods return those attributes.
 */
extern ivl_process_type_t ivl_process_type(ivl_process_t net);

extern ivl_scope_t ivl_process_scope(ivl_process_t net);

extern ivl_statement_t ivl_process_stmt(ivl_process_t net);

extern unsigned        ivl_process_attr_cnt(ivl_process_t net);
extern ivl_attribute_t ivl_process_attr_val(ivl_process_t net, unsigned idx);

/*
 * These functions manage statements of various type. This includes
 * all the different kinds of statements (as enumerated in
 * ivl_statement_type_t) that might occur in behavioral code.
 *
 * The ivl_statement_type() function returns the type code for the
 * statement. This is the major type, and implies which of the later
 * functions are applicable to the statement.
 */
extern ivl_statement_type_t ivl_statement_type(ivl_statement_t net);

/*
 * The following functions retrieve specific single values from the
 * statement. These values are the bits of data and parameters that
 * make up the statement. Many of these functions apply to more then
 * one type of statement, so the comment in front of them tells which
 * statement types can be passed to the function.
 *
 * FUNCTION SUMMARY:
 *
 * ivl_stmt_block_scope
 *    If the block is named, then there is a scope associated with
 *    this. The code generator may need to know this in order to
 *    handle disable statements.
 *
 * ivl_stmt_events
 * ivl_stmt_nevent
 *    Statements that have event arguments (TRIGGER and WAIT) make
 *    those event objects available through these methods.
 *
 * ivl_stmt_lval
 * ivl_stmt_lvals
 *    Return the number of l-values for an assignment statement, or
 *    the specific l-value. If there is more then 1 l-value, then the
 *    l-values are presumed to be vector values concatenated together
 *    from msb (idx==0) to lsb.
 *
 * ivl_stmt_rval
 *    Return the rval expression of the assignment. This is the value
 *    that is to be calculated and assigned to the l-value in all the
 *    assignment statements.
 *
 * ivl_stmt_sub_stmt
 *    Some statements contain a single, subordinate statement. An
 *    example is the IVL_ST_WAIT, which contains the statement to be
 *    executed after the wait completes. This method retrieves that
 *    sub-statement.
 *
 * SEMANTIC NOTES:
 *
 * - Assignments: IVL_ST_ASSIGN, IVL_ST_ASSIGN_NB, IVL_CASSIGN, IVL_ST_FORCE
 *
 * The assignments support ivl_stmt_rval to get the r-value expression
 * that is to be assign to the l-value, and ivl_stmt_lval[s] to get
 * the l-value that receives the value. The compiler has already made
 * sure that the types (l-value and r-value) are compatible.
 *
 * If the l-value is a vector, then the compiler also makes sure the
 * expression width of the r-values matches. It handles padding or
 * operator sizing as needed to get the width exactly right.
 *
 * - IVL_ST_CASSIGN
 * This reflects a procedural continuous assignment to an l-value. The
 * l-value is the same as any other assignment (use ivl_stmt_lval).
 *
 * The value to be assigned is an ivl_expr_t retrieved by the
 * ivl_stmt_rval function. The run time is expected to calculate the
 * value of the expression at the assignment, then continuous assign
 * that constant value. If the expression is non-constant, the code
 * generator is supposed to know what to do about that, too.
 *
 * - IVL_ST_FORCE
 * This is very much like IVL_ST_CASSIGN, but adds that l-values can
 * include nets (tri, wire, etc). Memory words are restricted from
 * force l-values, and also non-constant bit or part selects. The
 * compiler will assure these constraints are met.
 *
 * - IVL_ST_TRIGGER
 * This represents the "-> name" statement that sends a trigger to a
 * named event. The ivl_stmt_nevent function should always return 1,
 * and the ivl_stmt_events(net,0) function returns the target event,
 * as an ivl_event_t. The only behavior of this statement is to send a
 * "trigger" to the target event.
 *
 * - IVL_ST_WAIT
 * This is the edge sensitive wait (for event) statement. The
 * statement contains an array of events that are to be tested, and a
 * single statement that is to be executed when any of the array of
 * events triggers.
 *
 * the ivl_stmt_events function accesses the array of events to wait
 * for, and the ivl_stmt_sub_stmt function gets the sub-statement,
 * which may be null, that is to be executed when an event
 * triggers. The statement waits even if the sub-statement is nul.
 */

  /* IVL_ST_BLOCK, IVL_ST_FORK */
extern unsigned ivl_stmt_block_count(ivl_statement_t net);
  /* IVL_ST_BLOCK, IVL_ST_FORK */
extern ivl_scope_t ivl_stmt_block_scope(ivl_statement_t net);
  /* IVL_ST_BLOCK, IVL_ST_FORK */
extern ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_UTASK IVL_ST_DISABLE */
extern ivl_scope_t ivl_stmt_call(ivl_statement_t net);
  /* IVL_ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern unsigned ivl_stmt_case_count(ivl_statement_t net);
  /* IVL_ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern ivl_expr_t ivl_stmt_case_expr(ivl_statement_t net, unsigned i);
  /* IVL_ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern ivl_statement_t ivl_stmt_case_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_CONDIT IVL_ST_CASE IVL_ST_REPEAT IVL_ST_WHILE */
extern ivl_expr_t      ivl_stmt_cond_expr(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_false(ivl_statement_t net);
  /* IVL_ST_CONDIT */
extern ivl_statement_t ivl_stmt_cond_true(ivl_statement_t net);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_DELAYX */
extern ivl_expr_t ivl_stmt_delay_expr(ivl_statement_t net);
  /* IVL_ST_DELAY */
extern unsigned long ivl_stmt_delay_val(ivl_statement_t net);
  /* IVL_ST_WAIT IVL_ST_TRIGGER */
extern unsigned    ivl_stmt_nevent(ivl_statement_t net);
extern ivl_event_t ivl_stmt_events(ivl_statement_t net, unsigned idx);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_CASSIGN IVL_ST_DEASSIGN
     IVL_ST_FORCE IVL_ST_RELEASE */
extern ivl_lval_t ivl_stmt_lval(ivl_statement_t net, unsigned idx);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_CASSIGN IVL_ST_DEASSIGN
     IVL_ST_FORCE IVL_ST_RELEASE */
extern unsigned ivl_stmt_lvals(ivl_statement_t net);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_CASSIGN */
extern unsigned ivl_stmt_lwidth(ivl_statement_t net);
  /* IVL_ST_STASK */
extern const char* ivl_stmt_name(ivl_statement_t net);
#if 0
extern ivl_nexus_t ivl_stmt_nexus(ivl_statement_t net, unsigned idx);
extern unsigned    ivl_stmt_nexus_count(ivl_statement_t net);
#endif
  /* IVL_ST_STASK */
extern ivl_expr_t ivl_stmt_parm(ivl_statement_t net, unsigned idx);
  /* IVL_ST_STASK */
extern unsigned ivl_stmt_parm_count(ivl_statement_t net);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_CASSIGN IVL_ST_FORCE */
extern ivl_expr_t ivl_stmt_rval(ivl_statement_t net);
  /* IVL_ST_DELAY, IVL_ST_DELAYX, IVL_ST_FOREVER, IVL_ST_REPEAT
     IVL_ST_WAIT, IVL_ST_WHILE */
extern ivl_statement_t ivl_stmt_sub_stmt(ivl_statement_t net);

/*
 * These functions manipulate variable objects.
 *
 * ivl_variable_name
 *   Return the base name of the variable.
 *
 * ivl_variable_type
 *   Return the type of the variable. The ivl_variable_type_t is an
 *   enumeration that is defined earlier.
 */
extern const char*         ivl_variable_name(ivl_variable_t net);
extern ivl_variable_type_t ivl_variable_type(ivl_variable_t net);


#if defined(__MINGW32__) || defined (__CYGWIN32__)
#  define DLLEXPORT __declspec(dllexport)
#else
#  define DLLEXPORT
#endif

extern DLLEXPORT int target_design(ivl_design_t des);


/* target_design

   The "target_design" function is called once after the whole design
   is processed and available to the target. The target doesn't return
   from this function until it is finished with the design.

   The return value of this function should normally be zero. If the
   code generator detects errors, however, then the code generator
   returns a positive number to indicate the approximate number of
   errors detected (before it gave up.) Return values <0 are reserved
   for system and infrastructure errors.

   This function is implemented in the loaded target, and not in the
   ivl core. This function is how the target module is invoked. */

typedef int  (*target_design_f)(ivl_design_t des);


_END_DECL

/*
 * $Log: ivl_target.h,v $
 * Revision 1.144  2005/03/03 04:34:42  steve
 *  Rearrange how memories are supported as vvp_vector4 arrays.
 *
 * Revision 1.143  2005/02/19 02:43:38  steve
 *  Support shifts and divide.
 *
 * Revision 1.142  2005/02/14 01:51:39  steve
 *  Handle bit selects in l-values to assignments.
 *
 * Revision 1.141  2005/02/13 01:15:07  steve
 *  Replace supply nets with wires connected to pullup/down supply devices.
 *
 * Revision 1.140  2005/02/12 06:25:40  steve
 *  Restructure NetMux devices to pass vectors.
 *  Generate NetMux devices from ternary expressions,
 *  Reduce NetMux devices to bufif when appropriate.
 *
 * Revision 1.139  2005/02/08 00:12:36  steve
 *  Add the NetRepeat node, and code generator support.
 *
 * Revision 1.138  2005/02/03 04:56:20  steve
 *  laborate reduction gates into LPM_RED_ nodes.
 *
 * Revision 1.137  2005/01/29 18:46:18  steve
 *  Netlist boolean expressions generate gate vectors.
 *
 * Revision 1.136  2005/01/29 16:47:20  steve
 *  Clarify width of nexus.
 *
 * Revision 1.135  2005/01/28 05:39:33  steve
 *  Simplified NetMult and IVL_LPM_MULT.
 *
 * Revision 1.134  2005/01/24 05:28:30  steve
 *  Remove the NetEBitSel and combine all bit/part select
 *  behavior into the NetESelect node and IVL_EX_SELECT
 *  ivl_target expression type.
 *
 * Revision 1.133  2005/01/22 17:36:59  steve
 *  stub dump signed flags of magnitude compare.
 *
 * Revision 1.132  2005/01/22 01:06:55  steve
 *  Change case compare from logic to an LPM node.
 *
 * Revision 1.131  2005/01/09 20:16:01  steve
 *  Use PartSelect/PV and VP to handle part selects through ports.
 *
 * Revision 1.130  2004/12/29 23:55:43  steve
 *  Unify elaboration of l-values for all proceedural assignments,
 *  including assing, cassign and force.
 *
 *  Generate NetConcat devices for gate outputs that feed into a
 *  vector results. Use this to hande gate arrays. Also let gate
 *  arrays handle vectors of gates when the outputs allow for it.
 *
 * Revision 1.129  2004/12/18 18:56:18  steve
 *  Add ivl_event_scope, and better document ivl_event_X methods.
 *
 * Revision 1.128  2004/12/15 17:10:40  steve
 *  Fixup force statement elaboration.
 *
 * Revision 1.127  2004/12/11 02:31:26  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.126  2004/10/04 01:10:53  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.125  2004/09/25 01:58:12  steve
 *  Some commentary on ivl_logic_pin.
 *
 * Revision 1.124  2003/12/03 02:46:24  steve
 *  Add support for wait on list of named events.
 *
 * Revision 1.123  2003/11/08 20:06:21  steve
 *  Spelling fixes in comments.
 *
 * Revision 1.122  2003/08/22 23:14:26  steve
 *  Preserve variable ranges all the way to the vpi.
 *
 * Revision 1.121  2003/08/15 02:23:52  steve
 *  Add synthesis support for synchronous reset.
 *
 * Revision 1.120  2003/07/30 01:13:28  steve
 *  Add support for triand and trior.
 *
 * Revision 1.119  2003/06/23 01:25:44  steve
 *  Module attributes make it al the way to ivl_target.
 *
 * Revision 1.118  2003/05/14 05:26:41  steve
 *  Support real expressions in case statements.
 *
 * Revision 1.117  2003/04/22 04:48:29  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.116  2003/04/11 05:18:08  steve
 *  Handle signed magnitude compare all the
 *  way through to the vvp code generator.
 *
 * Revision 1.115  2003/03/10 23:40:53  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.114  2003/03/06 01:24:37  steve
 *  Obsolete the ivl_event_name function.
 *
 * Revision 1.113  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.112  2003/02/26 01:29:24  steve
 *  LPM objects store only their base names.
 *
 * Revision 1.111  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.110  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.109  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 */
#endif
