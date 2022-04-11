#ifndef IVL_ivl_target_H
#define IVL_ivl_target_H
/*
 * Copyright (c) 2000-2021 Stephen Williams (steve@icarus.com)
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

# include  <inttypes.h>
# include  <stddef.h>
# include  <stdbool.h>

/* Re the _CLASS define: clang++ wants this to be class to match the
 * definition, but clang (the C) compiler needs it to be a struct
 * since class is not defined in C. They are effectively both pointers
 * to an object so everything works out. */

#ifdef __cplusplus
#define _BEGIN_DECL extern "C" {
#define _END_DECL }
#define _CLASS class
#else
#define _BEGIN_DECL
#define _END_DECL
#define _CLASS struct
#endif

#ifndef __GNUC__
# define __attribute__(x)
#endif

#if defined(__cplusplus) && defined(_MSC_VER)
# define ENUM_UNSIGNED_INT : unsigned int
#else
# define ENUM_UNSIGNED_INT
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
 * ivl_array_t
 *    This object represents an array that can be a memory or a net
 *    array. (They are the same from the perspective of ivl_target.h.)
 *
 * ivl_branch_t
 *    this object represents an analog branch.
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
 *    Objects of this type represent expressions in processes.
 *    Structural expressions are instead treated as logic gates.
 *
 * ivl_island_t
 *    Certain types of objects may belong to islands. The island that
 *    they belong to is represented by the ivl_island_t cookie. To
 *    know if objects belong to the same island, it is sufficient to
 *    compare island cookies. If a==b, then island a is the same as
 *    island b.
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
 * ivl_switch_t
 *    Switches are the tran/tranif devices in the design.
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
typedef struct ivl_array_s    *ivl_array_t;
typedef struct ivl_branch_s   *ivl_branch_t;
typedef struct ivl_delaypath_s*ivl_delaypath_t;
typedef struct ivl_design_s   *ivl_design_t;
typedef _CLASS ivl_discipline_s*ivl_discipline_t;
typedef const _CLASS netenum_t*ivl_enumtype_t;
typedef struct ivl_event_s    *ivl_event_t;
typedef struct ivl_expr_s     *ivl_expr_t;
typedef struct ivl_island_s   *ivl_island_t;
typedef struct ivl_lpm_s      *ivl_lpm_t;
typedef struct ivl_lval_s     *ivl_lval_t;
typedef struct ivl_net_const_s*ivl_net_const_t;
typedef struct ivl_net_logic_s*ivl_net_logic_t;
typedef struct ivl_udp_s      *ivl_udp_t;
typedef _CLASS ivl_nature_s   *ivl_nature_t;
typedef struct ivl_net_probe_s*ivl_net_probe_t;
typedef struct ivl_nexus_s    *ivl_nexus_t;
typedef struct ivl_nexus_ptr_s*ivl_nexus_ptr_t;
typedef struct ivl_parameter_s*ivl_parameter_t;
typedef struct ivl_process_s  *ivl_process_t;
typedef struct ivl_scope_s    *ivl_scope_t;
typedef struct ivl_signal_s   *ivl_signal_t;
typedef struct ivl_port_info_s*ivl_port_info_t;
typedef struct ivl_switch_s   *ivl_switch_t;
typedef struct ivl_memory_s   *ivl_memory_t; //XXXX __attribute__((deprecated));
typedef struct ivl_statement_s*ivl_statement_t;
typedef const _CLASS ivl_type_s*ivl_type_t;

/*
 * These are types that are defined as enumerations. These have
 * explicit values so that the binary API is a bit more resilient to
 * changes and additions to the enumerations.
 */

typedef enum ivl_dis_domain_e {
      IVL_DIS_NONE       = 0,
      IVL_DIS_DISCRETE   = 1,
      IVL_DIS_CONTINUOUS = 2
} ivl_dis_domain_t;

typedef enum ivl_drive_e ENUM_UNSIGNED_INT {
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
      IVL_EX_ARRAY = 18,
      IVL_EX_BACCESS= 19,
      IVL_EX_BINARY = 2,
      IVL_EX_CONCAT = 3,
      IVL_EX_DELAY = 20,
      IVL_EX_ENUMTYPE = 21,
      IVL_EX_EVENT  = 17,
      IVL_EX_MEMORY = 4,
      IVL_EX_NEW    = 23,
      IVL_EX_NULL   = 22,
      IVL_EX_NUMBER = 5,
      IVL_EX_ARRAY_PATTERN  = 26,
      IVL_EX_PROPERTY = 24,
      IVL_EX_REALNUM  = 16,
      IVL_EX_SCOPE  = 6,
      IVL_EX_SELECT = 7,
      IVL_EX_SFUNC  = 8,
      IVL_EX_SHALLOWCOPY = 25,
      IVL_EX_SIGNAL = 9,
      IVL_EX_STRING = 10,
      IVL_EX_TERNARY = 11,
      IVL_EX_UFUNC = 12,
      IVL_EX_ULONG = 13,
      IVL_EX_UNARY = 14
} ivl_expr_type_t;

typedef enum ivl_select_type_e ENUM_UNSIGNED_INT {
      IVL_SEL_OTHER = 0,
      IVL_SEL_IDX_UP = 1,
      IVL_SEL_IDX_DOWN = 2
} ivl_select_type_t;

/* This is the type code for an ivl_net_logic_t object. */
typedef enum ivl_logic_e {
      IVL_LO_NONE   =  0,
      IVL_LO_AND    =  1,
      IVL_LO_BUF    =  2,
      IVL_LO_BUFIF0 =  3,
      IVL_LO_BUFIF1 =  4,
      IVL_LO_BUFT   = 24, /* transparent bufz. (NOT "tri-state") */
      IVL_LO_BUFZ   =  5,
      IVL_LO_CMOS   = 22,
      IVL_LO_EQUIV  = 25,
      IVL_LO_IMPL   = 26,
      IVL_LO_NAND   =  6,
      IVL_LO_NMOS   =  7,
      IVL_LO_NOR    =  8,
      IVL_LO_NOT    =  9,
      IVL_LO_NOTIF0 = 10,
      IVL_LO_NOTIF1 = 11,
      IVL_LO_OR     = 12,
      IVL_LO_PMOS   = 17,
      IVL_LO_PULLDOWN  = 13,
      IVL_LO_PULLUP = 14,
      IVL_LO_RCMOS  = 23,
      IVL_LO_RNMOS  = 15,
      IVL_LO_RPMOS  = 16,
      IVL_LO_XNOR   = 18,
      IVL_LO_XOR    = 19,
      IVL_LO_UDP    = 21
} ivl_logic_t;

/* This is the type of a ivl_switch_t object */
typedef enum ivl_switch_type_e {
      IVL_SW_TRAN     = 0,
      IVL_SW_TRANIF0  = 1,
      IVL_SW_TRANIF1  = 2,
      IVL_SW_RTRAN    = 3,
      IVL_SW_RTRANIF0 = 4,
      IVL_SW_RTRANIF1 = 5,
      IVL_SW_TRAN_VP  = 6
} ivl_switch_type_t;

/* This is the type of an LPM object. */
typedef enum ivl_lpm_type_e {
      IVL_LPM_ABS    = 32,
      IVL_LPM_ADD    =  0,
      IVL_LPM_ARRAY  = 30,
      IVL_LPM_CAST_INT  = 34,
      IVL_LPM_CAST_INT2 = 35,
      IVL_LPM_CAST_REAL = 33,
      IVL_LPM_CONCAT = 16,
      IVL_LPM_CONCATZ = 36, /* Transparent concat */
      IVL_LPM_CMP_EEQ= 18, /* Case EQ (===) */
      IVL_LPM_CMP_EQX= 37, /* Wildcard EQ (casex) */
      IVL_LPM_CMP_EQZ= 38, /* casez EQ */
      IVL_LPM_CMP_WEQ= 41,
      IVL_LPM_CMP_WNE= 42,
      IVL_LPM_CMP_EQ = 10,
      IVL_LPM_CMP_GE =  1,
      IVL_LPM_CMP_GT =  2,
      IVL_LPM_CMP_NE = 11,
      IVL_LPM_CMP_NEE= 19, /* Case NE (!==) */
      IVL_LPM_DIVIDE = 12,
      IVL_LPM_FF     =  3,
      IVL_LPM_LATCH  = 40,
      IVL_LPM_MOD    = 13,
      IVL_LPM_MULT   =  4,
      IVL_LPM_MUX    =  5,
      /* IVL_LPM_PART_BI= 28, / obsolete */
      IVL_LPM_PART_VP= 15, /* part select: vector to part */
      IVL_LPM_PART_PV= 17, /* part select: part written to vector */
      IVL_LPM_POW    = 31,
      IVL_LPM_RE_AND = 20,
      IVL_LPM_RE_NAND= 21,
      IVL_LPM_RE_NOR = 22,
      IVL_LPM_RE_OR  = 23,
      IVL_LPM_RE_XNOR= 24,
      IVL_LPM_RE_XOR = 25,
      IVL_LPM_REPEAT = 26,
      IVL_LPM_SFUNC  = 29,
      IVL_LPM_SHIFTL =  6,
      IVL_LPM_SHIFTR =  7,
      IVL_LPM_SIGN_EXT=27,
      IVL_LPM_SUB    =  8,
      IVL_LPM_SUBSTITUTE=39,
      /* IVL_LPM_RAM =  9, / obsolete */
      IVL_LPM_UFUNC  = 14
} ivl_lpm_type_t;

/* The path edge type is the edge type used to select a specific
   delay. */
typedef enum ivl_path_edge_e {
      IVL_PE_01 = 0, IVL_PE_10, IVL_PE_0z,
      IVL_PE_z1,     IVL_PE_1z, IVL_PE_z0,
      IVL_PE_0x,     IVL_PE_x1, IVL_PE_1x,
      IVL_PE_x0,     IVL_PE_xz, IVL_PE_zx,
      IVL_PE_COUNT
} ivl_path_edge_t;

/* Processes are initial, always, or final blocks with a statement. This is
   the type of the ivl_process_t object. */
typedef enum ivl_process_type_e ENUM_UNSIGNED_INT {
      IVL_PR_INITIAL      = 0,
      IVL_PR_ALWAYS       = 1,
      IVL_PR_ALWAYS_COMB  = 3,
      IVL_PR_ALWAYS_FF    = 4,
      IVL_PR_ALWAYS_LATCH = 5,
      IVL_PR_FINAL        = 2
} ivl_process_type_t;

/* These are the sorts of reasons a scope may come to be. These types
   are properties of ivl_scope_t objects. */
typedef enum ivl_scope_type_e {
      IVL_SCT_MODULE  = 0,
      IVL_SCT_FUNCTION= 1,
      IVL_SCT_TASK    = 2,
      IVL_SCT_BEGIN   = 3,
      IVL_SCT_FORK    = 4,
      IVL_SCT_GENERATE= 5,
      IVL_SCT_PACKAGE = 6,
      IVL_SCT_CLASS   = 7
} ivl_scope_type_t;

/* Signals (ivl_signal_t) that are ports into the scope that contains
   them have a port type. Otherwise, they are port IVL_SIP_NONE. */
typedef enum OUT {
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
      IVL_SIT_TRIOR  = 8,
      IVL_SIT_UWIRE  = 9
} ivl_signal_type_t;

/* This is the type code for ivl_statement_t objects. */
typedef enum ivl_statement_type_e {
      IVL_ST_NONE    = 0,
      IVL_ST_NOOP    = 1,
      IVL_ST_ALLOC   = 25,
      IVL_ST_ASSIGN    = 2,
      IVL_ST_ASSIGN_NB = 3,
      IVL_ST_BLOCK   = 4,
      IVL_ST_CASE    = 5,
      IVL_ST_CASER   = 24, /* Case statement with real expressions. */
      IVL_ST_CASEX   = 6,
      IVL_ST_CASEZ   = 7,
      IVL_ST_CASSIGN = 8,
      IVL_ST_CONDIT  = 9,
      IVL_ST_CONTRIB = 27,
      IVL_ST_DEASSIGN = 10,
      IVL_ST_DELAY   = 11,
      IVL_ST_DELAYX  = 12,
      IVL_ST_DISABLE = 13,
      IVL_ST_DO_WHILE = 30,
      IVL_ST_FORCE   = 14,
      IVL_ST_FOREVER = 15,
      IVL_ST_FORK    = 16,
      IVL_ST_FORK_JOIN_ANY  = 28,
      IVL_ST_FORK_JOIN_NONE = 29,
      IVL_ST_FREE    = 26,
      IVL_ST_NB_TRIGGER = 31,
      IVL_ST_RELEASE = 17,
      IVL_ST_REPEAT  = 18,
      IVL_ST_STASK   = 19,
      IVL_ST_TRIGGER = 20,
      IVL_ST_UTASK   = 21,
      IVL_ST_WAIT    = 22,
      IVL_ST_WHILE   = 23
} ivl_statement_type_t;

/* Case statements can be tagged as unique/unique0/priority. */
typedef enum ivl_case_quality_t {
      IVL_CASE_QUALITY_BASIC    = 0,  /* no quality flags */
      IVL_CASE_QUALITY_UNIQUE   = 1,
      IVL_CASE_QUALITY_UNIQUE0  = 2,
      IVL_CASE_QUALITY_PRIORITY = 3
} ivl_case_quality_t;

/* SystemVerilog allows a system function to be called as a task. */
typedef enum ivl_sfunc_as_task_e {
      IVL_SFUNC_AS_TASK_ERROR   = 0,
      IVL_SFUNC_AS_TASK_WARNING = 1,
      IVL_SFUNC_AS_TASK_IGNORE  = 2
} ivl_sfunc_as_task_t;

/* This is the type of a variable, and also used as the type for an
   expression. */
typedef enum ivl_variable_type_e ENUM_UNSIGNED_INT {
      IVL_VT_VOID    = 0,  /* Not used */
      IVL_VT_NO_TYPE = 1,  /* Place holder for missing/unknown type. */
      IVL_VT_REAL    = 2,
      IVL_VT_BOOL    = 3,
      IVL_VT_LOGIC   = 4,
      IVL_VT_STRING  = 5,
      IVL_VT_DARRAY  = 6,  /* Array (esp. dynamic array) */
      IVL_VT_CLASS   = 7,  /* SystemVerilog class instances */
      IVL_VT_QUEUE   = 8,  /* SystemVerilog queue instances */
      IVL_VT_VECTOR = IVL_VT_LOGIC /* For compatibility */
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

/* BRANCH
 * Branches are analog constructs, a pair of terminals that is used in
 * branch access functions. Terminal-1 is the reference node (The
 * "ground") for the purposes of the access function that accesses it.
 *
 * SEMANTIC NOTES
 * All the branches in an island are connected by terminals or by
 * expressions. The island is the connection of branches that must be
 * solved together.
 */
  /* extern ivl_scope_t ivl_branch_scope(ivl_branch_t obj); */
extern ivl_nexus_t ivl_branch_terminal(ivl_branch_t obj, int idx);
extern ivl_island_t ivl_branch_island(ivl_branch_t obj);

/* DELAYPATH
 * Delaypath objects represent delay paths called out by a specify
 * block in the Verilog source file. The destination signal references
 * the path object, which in turn points to the source for the path.
 *
 * ivl_path_scope
 *    This returns the scope of the delay path. This scope corresponds
 *    to the scope of the specify-block that led to this path.
 *
 * ivl_path_source
 *    This returns the nexus that is the source end of the delay
 *    path. Transitions on the source are the start of the delay time
 *    for this path.
 *
 * ivl_path_condit
 *    This returns the nexus that tracks the condition for the
 *    delay. If the delay path is unconditional, this returns nil.
 * ivl_path_is_condit
 *    Is this a conditional structure? Needed for ifnone.
 *
 * ivl_path_is_parallel
 *    This returns true if the path is a parallel connection and
 *    false if the path is a full connection.
 *
 * ivl_path_source_posedge
 * ivl_path_source_negedge
 *    These functions return true if the source is edge sensitive.
 */
extern ivl_scope_t ivl_path_scope(ivl_delaypath_t obj);
extern ivl_nexus_t ivl_path_source(ivl_delaypath_t obj);
extern uint64_t ivl_path_delay(ivl_delaypath_t obj, ivl_path_edge_t pt);
extern ivl_nexus_t ivl_path_condit(ivl_delaypath_t obj);
extern int ivl_path_is_condit(ivl_delaypath_t obj);

extern int ivl_path_is_parallel(ivl_delaypath_t obj);

extern int ivl_path_source_posedge(ivl_delaypath_t obj);
extern int ivl_path_source_negedge(ivl_delaypath_t obj);

/* DESIGN
 * When handed a design (ivl_design_t) there are a few things that you
 * can do with it. The Verilog program has one design that carries the
 * entire program. Use the design methods to iterate over the elements
 * of the design.
 *
 * ivl_design_delay_sel
 *    Returns the tool delay selection: "MINIMUM", "TYPICAL" or "MAXIMUM"?
 *
 * ivl_design_flag
 *    This function returns the string value of a named flag. Flags
 *    come from the "-pkey=value" options to the iverilog command and
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
 * ivl_design_root (ANACHRONISM)
 *    A design has a root named scope that is an instance of the top
 *    level module in the design. This is a hook for naming the
 *    design, or for starting the scope scan.
 *
 * ivl_design_roots
 *    A design has some number of root scopes. These are the starting
 *    points for structural elaboration. This function returns to the
 *    caller a pointer to an ivl_scope_t array, and the size of the
 *    array.
 *
 * ivl_design_time_precision
 *    A design as a time precision. This is the size in seconds (a
 *    signed power of 10) of a simulation tick.
 */

extern const char* ivl_design_delay_sel(ivl_design_t des);
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

extern unsigned         ivl_design_disciplines(ivl_design_t des);
extern ivl_discipline_t ivl_design_discipline(ivl_design_t des, unsigned idx);

/* LITERAL CONSTANTS
 * Literal constants are nodes with no input and a single constant
 * output. The form of the output depends on the type of the node.
 * The output is an array of 4-value bits, using a single char
 * value for each bit. The bits of the vector are in canonical (lsb
 * first) order for the width of the constant.
 *
 * ivl_const_type
 *    The is the type of the node.
 *
 * ivl_const_bits
 *    This returns a pointer to an array of constant characters,
 *    each byte a '0', '1', 'x' or 'z'. The array is *not* nul
 *    terminated. This value is only value if ivl_const_type is
 *    IVL_VT_LOGIC or IVL_VT_BOOL. It returns nil otherwise.
 *
 * ivl_const_nex
 *    Return the ivl_nexus_t of the output for the constant.
 *
 * ivl_const_scope
 *    Return the scope this constant was defined in.

 * ivl_const_signed
 *    Return true (!0) if the constant is a signed value, 0 otherwise.
 *
 * ivl_const_width
 *    Return the width, in logical bits, of the constant.
 *
 * ivl_const_delay
 *    T0 delay for a transition (0, 1 and Z).
 *
 * SEMANTIC NOTES
 *
 * The const_type of the literal constant must match the
 * ivl_signal_data_type if the signals that share the nexus of this
 * node. The compiler makes sure it is so, converting constant values
 * as needed.
 *
 * - IVL_VT_LOGIC
 *
 * - IVL_VT_REAL
 * Real valued constants have a width of 1. The value emitted to the
 * output is ivl_const_real.
 */
extern ivl_variable_type_t ivl_const_type(ivl_net_const_t net);
extern const char* ivl_const_bits(ivl_net_const_t net);
extern ivl_expr_t  ivl_const_delay(ivl_net_const_t net, unsigned transition);
extern ivl_nexus_t ivl_const_nex(ivl_net_const_t net);
extern ivl_scope_t ivl_const_scope(ivl_net_const_t net);
extern int         ivl_const_signed(ivl_net_const_t net);
extern unsigned    ivl_const_width(ivl_net_const_t net);
extern double      ivl_const_real(ivl_net_const_t net);

extern const char* ivl_const_file(ivl_net_const_t net);
extern unsigned ivl_const_lineno(ivl_net_const_t net);

/* extern ivl_nexus_t ivl_const_pin(ivl_net_const_t net, unsigned idx); */
/* extern unsigned    ivl_const_pins(ivl_net_const_t net); */

/* DISCIPLINES
 *
 * Disciplines are Verilog-AMS construct. A discipline is a collection
 * of attributes that can be attached to a signal.
 *
 * FUNCTION SUMMARY
 *
 * ivl_discipline_name
 *    This is the name of the discipline in the Verilog-AMS source.
 *
 * ivl_discipline_domain
 *    This is the domain: continuous or discrete.
 *
 * SEMANTIC NOTES
 *
 * The discipline domain will not be IVL_DIS_NONE. The "none" domain
 * is a place-holder internally for incomplete parsing, and is also
 * available for code generators to use.
 */
extern const char*ivl_discipline_name(ivl_discipline_t net);
extern ivl_dis_domain_t ivl_discipline_domain(ivl_discipline_t net);
extern ivl_nature_t ivl_discipline_potential(ivl_discipline_t net);
extern ivl_nature_t ivl_discipline_flow(ivl_discipline_t net);

extern const char* ivl_nature_name(ivl_nature_t net);

/* ENUMERATIONS
 *
 * Enumerations are a collections of symbolic names and vector
 * values. The enumeration has a base type, and a list of names and
 * values.
 *
 * FUNCTION SUMMARY
 *
 * ivl_enum_names
 *    This is the number of enumeration names in the enum type.
 *
 * ivl_enum_name
 *    Get the string name for an item in the enumeration
 *
 * ivl_enum_bits
 *    Get the bits (lsb first) of the enumeration value. The width
 *    of the enumeration should match the length of this string. Every
 *    name also has bits that make up the value.
 *
 * ivl_enum_signed
 *    Is the base type for the enum signed?
 *
 * ivl_enum_type
 *    Get the data-type for the base type that the enum uses. This
 *    will be either IVL_VT_BOOL or IVL_VT_LOGIC
 *
 * ivl_enum_width
 *    Return the bit width of the base type for this enum type.
 *
 * SEMANTIC NOTES
 */
extern unsigned ivl_enum_names(ivl_enumtype_t net);
extern const char*ivl_enum_name(ivl_enumtype_t net, unsigned idx);
extern const char*ivl_enum_bits(ivl_enumtype_t net, unsigned idx);
extern int ivl_enum_signed(ivl_enumtype_t net);
extern ivl_variable_type_t ivl_enum_type(ivl_enumtype_t net);
extern unsigned ivl_enum_width(ivl_enumtype_t net);

extern const char*ivl_enum_file(ivl_enumtype_t net);
extern unsigned ivl_enum_lineno(ivl_enumtype_t net);

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
 * Named events have file and line number information.
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

extern unsigned    ivl_event_nedg(ivl_event_t net);
extern ivl_nexus_t ivl_event_edg(ivl_event_t net, unsigned idx);

extern unsigned    ivl_event_nneg(ivl_event_t net);
extern ivl_nexus_t ivl_event_neg(ivl_event_t net, unsigned idx);

extern unsigned    ivl_event_npos(ivl_event_t net);
extern ivl_nexus_t ivl_event_pos(ivl_event_t net, unsigned idx);

extern const char*ivl_event_file(ivl_event_t net);
extern unsigned ivl_event_lineno(ivl_event_t net);


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
 * ivl_expr_sized
 *    This method returns false (0) if the expression node does not
 *    have a defined size. This is unusual, but may happen for
 *    constant expressions.
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
 * ivl_expr_net_type
 *    This is used in some cases to carry more advanced type
 *    descriptions. Over the long run, all type information will be
 *    moved into the ivl_type_t type description method.
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
 *              *   -- Multiply
 *
 * SEMANTIC NOTES
 *
 * - IVL_EX_ARRAY
 * This expression type is a special case of the IVL_EX_SIGNAL where
 * the target is an array (ivl_signal_t with an array_count) but there
 * is no index expression. This is used only in the special situation
 * where the array is passed to a system task/function. The function
 * ivl_expr_signal returns the ivl_signal_t of the array object, and
 * from that all the properties of the array can be determined.
 *
 * - IVL_EX_BINARY
 *
 * - IVL_EX_PROPERTY
 * This expression represents the property select from a class
 * type, for example "foo.property" where "foo" is a class handle and
 * "property" is the name of one of the properties of the class. The
 * ivl_expr_signal function returns the ivl_signal_t for "foo" and the
 * data_type for the signal will be IVL_VT_CLASS.
 *
 * The ivl_signal_net_type(sig) for the "foo" signal will be a class
 * type and from there you can get access to the type information.
 *
 * Elaboration reduces the properties of a class to a vector numbered
 * from 0 to the number of properties. The ivl_expr_property_idx()
 * function gets the index of the selected property into the property
 * table. That number can be passed to ivl_type_prop_*() functions to
 * get details about the property.
 *
 * If the property is an array, then the ivl_expr_oper1() function
 * returns the canonical expression for accessing the element of the
 * property.
 *
 * - IVL_EX_NEW
 * This expression takes one or two operands. The first operand,
 * returned by ivl_expr_oper1() is the number of elements to create
 * for the dynamic array. The second operand, if present, is returned
 * by the ivl_expr_oper2() function. If this returns a non-nil
 * expression, it is the initial value to be written to the elements
 * of the array. If the expression is an IVL_EX_ARRAY_PATTERN, then
 * this is the very special case of a list of values to be written to
 * array elements.
 *
 * - IVL_EX_SELECT
 * This expression takes two operands, oper1 is the expression to
 * select from, and oper2 is the selection base. The ivl_expr_width
 * value is the width of the bit/part select. The ivl_expr_oper1 value
 * is the base of a vector. The compiler has already figured out any
 * conversion from signal units to vector units, so the result of
 * ivl_expr_oper1 should range from 0 to ivl_expr_width().
 *
 * This expression is also used to implement string substrings. If the
 * sub-expression (oper1) is IVL_VT_STRING, then the base expression
 * (oper2) is a character address, with 0 the first address of the
 * string, 1 the second, and so on. This is OPPOSITE how a part select
 * of a string cast to a vector works, to be aware. The size of the
 * expression is an even multiple of 8, and is 8 times the number of
 * characters to pick.
 *
 * - IVL_EX_SIGNAL
 * This expression references a signal vector. The ivl_expr_signal
 * function gets a handle for the signal that is referenced. The
 * signal may be an array (see the ivl_signal_array_count function)
 * that is addressed by the expression returned by the ivl_expr_oper1
 * function. This expression returns a *canonical* address. The core
 * compiler already corrected the expression to account for index
 * bases.
 *
 * The ivl_expr_width function returns the vector width of the signal
 * word. The ivl_expr_value returns the data type of the word.
 *
 * Bit and part selects are not done here. The IVL_EX_SELECT
 * expression does bit/part selects on the word read from the signal.
 *
 * - IVL_EX_STRING
 * This expression refers to a string constant. The ivl_expr_string
 * function returns a pointer to the first byte of the string. The
 * compiler has translated it to a "vvp escaped string" which has
 * quoting and escapes eliminated. The string may contain octal
 * escapes (\<oct>) so that the string text returned by
 * ivl_expr_string will only contain graphical characters. It is up to
 * the target to change the escaped \NNN to the proper byte value when
 * using this string. No other escape sequences will appear in the
 * string. Quote (") and slash (\) characters will be delivered in
 * \NNN form.
 */

extern ivl_expr_type_t ivl_expr_type(ivl_expr_t net);
extern ivl_type_t ivl_expr_net_type(ivl_expr_t net);
extern ivl_variable_type_t ivl_expr_value(ivl_expr_t net);
extern const char*ivl_expr_file(ivl_expr_t net);
extern unsigned ivl_expr_lineno(ivl_expr_t net);

  /* IVL_EX_NUMBER */
extern const char* ivl_expr_bits(ivl_expr_t net);
  /* IVL_EX_BACCESS */
extern ivl_branch_t ivl_expr_branch(ivl_expr_t net);
  /* IVL_EX_UFUNC */
extern ivl_scope_t ivl_expr_def(ivl_expr_t net);
  /* IVL_EX_DELAY */
extern uint64_t ivl_expr_delay_val(ivl_expr_t net);
  /* IVL_EX_REALNUM */
extern double ivl_expr_dvalue(ivl_expr_t net);
  /* IVL_EX_ENUMTYPE */
extern ivl_enumtype_t ivl_expr_enumtype(ivl_expr_t net);
  /* IVL_EX_PROPERTY IVL_EX_SIGNAL IVL_EX_SFUNC IVL_EX_VARIABLE */
extern const char* ivl_expr_name(ivl_expr_t net);
  /* IVL_EX_BACCESS */
extern ivl_nature_t ivl_expr_nature(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_UNARY */
extern char        ivl_expr_opcode(ivl_expr_t net);
  /* IVL_EX_BINARY  IVL_EX_UNARY, IVL_EX_MEMORY IVL_EX_NEW IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper1(ivl_expr_t net);
  /* IVL_EX_BINARY IVL_EX_NEW IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper2(ivl_expr_t net);
  /* IVL_EX_TERNARY */
extern ivl_expr_t  ivl_expr_oper3(ivl_expr_t net);
  /* and expression */
extern ivl_parameter_t ivl_expr_parameter(ivl_expr_t net);
  /* IVL_EX_ARRAY_PATTERN IVL_EX_CONCAT IVL_EX_UFUNC */
extern ivl_expr_t  ivl_expr_parm(ivl_expr_t net, unsigned idx);
  /* IVL_EX_ARRAY_PATTERN IVL_EX_CONCAT IVL_EX_SFUNC IVL_EX_UFUNC */
extern unsigned    ivl_expr_parms(ivl_expr_t net);
  /* IVL_EX_CONCAT */
extern unsigned    ivl_expr_repeat(ivl_expr_t net);
  /* IVL_EX_SELECT */
extern ivl_select_type_t ivl_expr_sel_type(ivl_expr_t net);
  /* IVL_EX_EVENT */
extern ivl_event_t ivl_expr_event(ivl_expr_t net);
  /* IVL_EX_PROPERTY */
extern int ivl_expr_property_idx(ivl_expr_t net);
  /* IVL_EX_SCOPE */
extern ivl_scope_t ivl_expr_scope(ivl_expr_t net);
  /* IVL_EX_PROPERTY IVL_EX_SIGNAL */
extern ivl_signal_t ivl_expr_signal(ivl_expr_t net);
  /* any expression */
extern int         ivl_expr_signed(ivl_expr_t net);
  /* any expression */
extern int         ivl_expr_sized(ivl_expr_t net);
  /* IVL_EX_STRING */
extern const char* ivl_expr_string(ivl_expr_t net);
  /* IVL_EX_ULONG */
extern unsigned long ivl_expr_uvalue(ivl_expr_t net);
  /* any expression */
extern unsigned    ivl_expr_width(ivl_expr_t net);

extern const char* ivl_file_table_item(unsigned  idx);
extern unsigned ivl_file_table_index(const char *);
extern unsigned ivl_file_table_size(void);


/* ISLAND
 *
 * ivl_island_flag_set
 * ivl_island_flag_test
 *    Allow the user to test or set a boolean flag associated with the
 *    island.
 */
extern int ivl_island_flag_set(ivl_island_t net, unsigned flag, int value);
extern int ivl_island_flag_test(ivl_island_t net, unsigned flag);

extern const char* ivl_logic_file(ivl_net_logic_t net);
extern unsigned ivl_logic_lineno(ivl_net_logic_t net);

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
 * ivl_logic_delay
 *    Logic devices have a delay for each transition (0, 1 and Z).
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
 * logic devices that each process a bit slice of the
 * inputs/output. That implies that the widths of all the inputs and
 * the output must be identical.
 *
 * The ivl_logic_width  and ivl_logic_pins are *not* related. A logic
 * device has a number of pins that is the number of inputs to a logic
 * array of identical gates, and the ivl_logic_width, is the width of
 * the vector into each input pin and out of the output pin.
 *
 * The output pin is pin-0. The ivl_logic_driveX functions return the
 * drive strengths for the output pin-0, and match the drive values
 * stored in the ivl_nexus_ptr_t object for the pin.
 *
 * Logic devices have a logic propagation delay. The delay can be any
 * expression, although the most common expression is an IVL_EX_NUMBER
 * for a number value. The expression already includes scaling for the
 * containing module, so the delay value is always taken to be in
 * simulation clock ticks.
 *
 * If the delay is present, then ivl_logic_delay returns a non-nil
 * object. If any of the three delays is present, then all three are
 * present, even if they are all the same. The compiler will translate
 * shorthands into a complete set of delay expressions.
 *
 * The ivl_logic_delay expression will always be an IVL_EX_NUMBER, an
 * IVL_EX_ULONG, or an IVL_EX_SIGNAL. These expressions can easily be
 * used in structural contexts. The compiler will take care of
 * elaborating more complex expressions to nets.
 *
 * - IVL_LO_PULLUP/IVL_LO_PULLDOWN
 * These devices are grouped as logic devices with zero inputs because
 * the outputs have the same characteristics as other logic
 * devices. They are special only in that they have zero inputs, and
 * their drivers typically have strength other than strong.
 *
 * - IVL_LO_UDP
 * User defined primitives (UDPs) are like any other logic devices, in
 * that they are bit-slice devices. If they have a width, then they
 * are repeated to accommodate that width, and that implies that the
 * output and all the inputs must have the same width.
 *
 * The IVL_LO_UDP represents instantiations of UDP devices. The
 * ivl_udp_t describes the implementation.
 */

extern const char* ivl_logic_name(ivl_net_logic_t net);
extern const char* ivl_logic_basename(ivl_net_logic_t net);
extern ivl_scope_t ivl_logic_scope(ivl_net_logic_t net);
extern ivl_logic_t ivl_logic_type(ivl_net_logic_t net);
extern ivl_nexus_t ivl_logic_pin(ivl_net_logic_t net, unsigned pin);
extern unsigned    ivl_logic_pins(ivl_net_logic_t net);
extern ivl_udp_t   ivl_logic_udp(ivl_net_logic_t net);
extern ivl_expr_t  ivl_logic_delay(ivl_net_logic_t net, unsigned transition);
extern ivl_drive_t ivl_logic_drive0(ivl_net_logic_t net);
extern ivl_drive_t ivl_logic_drive1(ivl_net_logic_t net);
extern unsigned    ivl_logic_width(ivl_net_logic_t net);
extern unsigned    ivl_logic_is_cassign(ivl_net_logic_t net);

  /* DEPRECATED */
extern const char* ivl_logic_attr(ivl_net_logic_t net, const char*key);

extern unsigned        ivl_logic_attr_cnt(ivl_net_logic_t net);
extern ivl_attribute_t ivl_logic_attr_val(ivl_net_logic_t net, unsigned idx);

/* UDP
 * These methods allow access to the ivl_udp_t definition of a UDP.
 * The UDP definition is accessed through the ivl_logic_udp method of
 * an ivl_net_logic_t object.
 *
 * ivl_udp_name
 *    This returns the name of the definition of the primitive.
 *
 * ivl_udp_nin
 *    This is the number of inputs for the UDP definition.
 *
 * ivl_udp_rows
 * ivl_udp_row
 *    These methods give access to the rows that define the table of
 *    the primitive.
 *
 * SEMANTIC NOTES
 *
 * - Combinational primitives
 * These devices have no edge dependencies, and have no table entry
 * for the current input value. These have ivl_udp_sequ return 0
 * (false) and the length of each row is the number of inputs plus 1.
 * The first N characters correspond to the N inputs of the
 * device. The next character, the last character, is the output for
 * that row.
 *
 * - Sequential primitives
 * These devices allow edge transitions, and the rows are 1+N+1
 * characters long. The first character is the current output, the
 * next N characters the current input and the last character is the
 * new output.
 *
 * The ivl_udp_init value is only valid if the device is
 * sequential. It is the initial value for the output of the storage
 * element.
 */

extern int         ivl_udp_sequ(ivl_udp_t net);
extern unsigned    ivl_udp_nin(ivl_udp_t net);
extern char        ivl_udp_init(ivl_udp_t net);
extern const char* ivl_udp_row(ivl_udp_t net, unsigned idx);
extern unsigned    ivl_udp_rows(ivl_udp_t net);
extern const char* ivl_udp_name(ivl_udp_t net);
extern const char* ivl_udp_file(ivl_udp_t net);
extern unsigned    ivl_udp_lineno(ivl_udp_t net);
extern const char* ivl_udp_port(ivl_udp_t net, unsigned idx);

extern const char* ivl_lpm_file(ivl_lpm_t net);
extern unsigned    ivl_lpm_lineno(ivl_lpm_t net);

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
 * ivl_lpm_delay
 *    LPM devices have a delay for each transition (0, 1 and Z).
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
 * ivl_lpm_trigger
 *    SFUNC and UFUNC devices may have a trigger that forces the
 *    function output to be re-evaluated.
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
 * the concatenation. The ivl_lpm_size function returns the number of
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
 * more than the width of the output, although the possibility of
 * overflow exists at run time.
 *
 * The inputs are always treated as unsigned. If the expression is
 * supposed to be signed, elaboration will generate the necessary sign
 * extension, so the target need not (must not) consider signedness.
 *
 * - Power (IVL_LPM_POW)
 * The power takes two inputs and generates an output. Unlike other
 * arithmetic nodes, the width only refers to the output. The inputs
 * have independent widths, to reflect the arithmetic truth that the
 * width of a general power is the XXXX of the widths of the
 * inputs.
 *
 * Power may be signed. This indicates the type of the exponent. The
 * base will always be treated as unsigned. The compiler must ensure
 * the width of the base is equal to the width of the output to
 * obtain the correct result when the base is really a signed value.
 *
 * - Part Select (IVL_LPM_PART_VP and IVL_LPM_PART_PV)
 * There are two part select devices, one that extracts a part from a
 * vector, and another that writes a part of a vector. The _VP is
 * Vector-to-Part, and _PV is Part-to-Vector. The _VP form is meant to
 * model part/bin selects in r-value expressions, where the _PV from
 * is meant to model part selects in l-value nets.
 *
 * In both cases, ivl_lpm_data(0) is the input pin, and ivl_lpm_q is the
 * output. In the case of the _VP device, the vector is input and the
 * part is the output. In the case of the _PV device, the part is the
 * input and the vector is the output.
 *
 * If the base of the part select is non-constant, then
 * ivl_lpm_data(1) is non-nil and is the select, or base, address of
 * the part. If this pin is nil, then the constant base is used
 * instead.
 *
 * Also in both cases, the width of the device is the width of the
 * part. In the _VP case, this is obvious as the output nexus has the
 * part width. In the _PV case, this is a little less obvious, but
 * still correct. The output being written to the wider vector is
 * indeed the width of the part, even though it is written to a wider
 * gate. The target will need to handle this case specially.
 *
 * - Bi-directional Part Select (IVL_LPM_PART_BI)
 * This is not exactly a part select but a bi-directional partial link
 * of two nexa with different widths. This is used to implement tran
 * devices and inout ports in certain cases. The device width is the
 * width of the part. The ivl_lpm_q is the part end, and the
 * ivl_lpm_data(0) is the non-part end.
 *
 * - Comparisons (IVL_LPM_CMP_GT/GE/EQ/NE/EEQ/NEE/EQX/EQZ)
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
 * The EQX and EQZ nodes are wildcard compares, where xz bits (EQX) or
 * z bits (EQZ) in the data(1) operand are treated as wildcards. no
 * bits in the data(0) operand are wild. This matches the
 * SystemVerilog convention for the ==? operator.
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
 * ivl_lpm_q output. The type of the device is divined from the
 * inputs and the Q. All the types must be exactly the same.
 *
 * - D-FlipFlop (IVL_LPM_FF)
 * This device is an edge sensitive register. The ivl_lpm_q output and
 * single ivl_lpm_data input are the same width, ivl_lpm_width. This
 * device carries a vector like other LPM devices.
 *
 * - Latch (IVL_LPM_LATCH)
 * This device is an asynchronous latch. The ivl_lpm_q output and
 * single ivl_lpm_data input are the same width, ivl_lpm_width. This
 * device carries a vector like other LPM devices.
 *
 * - Memory port (IVL_LPM_RAM) (deprecated in favor of IVL_LPM_ARRAY)
 * These are structural ports into a memory device. They represent
 * address/data ports of a memory device that the context can hook to
 * for read or write. Read devices have an ivl_lpm_q output port that
 * is the data being read.
 *
 * The ivl_lpm_memory function returns the ivl_memory_t for the memory
 * that the port access. The ivl_lpm_width for the port then must
 * match the ivl_memory_width of the memory device.
 *
 * Read or write, the ivl_lpm_select nexus is the address. The
 * ivl_lpm_selects function returns the vector width of the
 * address. The range of the address is always from 0 to the memory
 * size-1 -- the canonical form. It is up to the compiler to generate
 * offsets to correct for a range declaration.
 *
 * Read ports use the ivl_lpm_q as the data output, and write ports
 * use the ivl_lpm_data(0) as the input. In either case the width of
 * the vector matches the width of the memory itself.
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
 * - Sign Extend (IVL_LPM_SIGN_EXT)
 * This node takes a single input and generates a single output. The
 * input must be signed, and the output will be a vector sign extended
 * to the desired width. The ivl_lpm_width() value is the output
 * width, the input will be whatever it wants to be.
 *
 * - Shifts (IVL_LPM_SHIFTL/SHIFTR)
 * This node takes two inputs, a vector and a shift distance. The
 * ivl_lpm_data(0) nexus is the vector input, and the ivl_lpm_data(1)
 * the shift distance. The vector input is the same width as the
 * output, but the distance has its own width.
 *
 * The ivl_lpm_signed() flag means for IVL_LPM_SHIFTR that the right
 * shift is *signed*. For SHIFTL, then signed-ness is meaningless.
 *
 * - System function call (IVL_LPM_SFUNC)
 * This device represents a netlist call to a system function. The
 * inputs to the device are passed to a system function, and the
 * result is sent via the output. The ivl_lpm_q function returns the
 * output nexus.
 *
 * The ivl_lpm_size function returns the number of arguments, and the
 * ivl_lpm_data(net,N) returns the nexa for the argument.
 *
 * The ivl_lpm_string(net) function returns the name of the system
 * function (i.e. "$display") that was found in the source code. The
 * compiler does little checking of that name.
 *
 * The ivl_lpm_trigger function retrieves the trigger event that
 * indicates when the system function needs to be re-evaluated. If
 * there is no trigger event, the system function only needs to be
 * re-evaluated when a change is detected on its input ports.
 *
 * - User Function Call (IVL_LPM_UFUNC)
 * This device is special as it represents a call to a user defined
 * function (behavioral code) within a netlist. The inputs to the
 * function are connected to the net, as is the output.
 *
 * The function definition is associated with a scope, and the
 * ivl_lpm_define function returns the scope that is that definition.
 * See the ivl_scope_* functions for how to get at the actual
 * definition.
 *
 * As with many LPM nodes, the ivl_lpm_q function returns the nexus
 * for the signal function return value. The width of this nexus must
 * exactly match the width of the device from ivl_lpm_width.
 *
 * The ivl_lpm_data function retrieves the nexa for all the input
 * ports. The ivl_lpm_size function returns the number of inputs for
 * the device, and the ivl_lpm_data() function index argument selects
 * the port to retrieve. Each port is sized independently.
 *
 * The ivl_lpm_trigger function retrieves the trigger event that
 * indicates when the user function needs to be re-evaluated. If
 * there is no trigger event, the user function only needs to be
 * re-evaluated when a change is detected on its input ports.
 */

extern const char*    ivl_lpm_name(ivl_lpm_t net); /* (Obsolete) */
extern const char*    ivl_lpm_basename(ivl_lpm_t net);
extern ivl_expr_t     ivl_lpm_delay(ivl_lpm_t net, unsigned transition);
extern ivl_scope_t    ivl_lpm_scope(ivl_lpm_t net);
extern int            ivl_lpm_signed(ivl_lpm_t net);
extern ivl_lpm_type_t ivl_lpm_type(ivl_lpm_t net);
extern unsigned       ivl_lpm_width(ivl_lpm_t net);
extern ivl_event_t    ivl_lpm_trigger(ivl_lpm_t net);

  /* IVL_LPM_FF */
extern ivl_nexus_t ivl_lpm_async_clr(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_async_set(ivl_lpm_t net);
extern ivl_expr_t  ivl_lpm_aset_value(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_sync_clr(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_sync_set(ivl_lpm_t net);
extern ivl_expr_t  ivl_lpm_sset_value(ivl_lpm_t net);
  /* IVL_LPM_ARRAY */
extern ivl_signal_t ivl_lpm_array(ivl_lpm_t net);
  /* IVL_LPM_PART IVL_LPM_SUBSTITUTE */
extern unsigned ivl_lpm_base(ivl_lpm_t net);
  /* IVL_LPM_FF */
extern unsigned    ivl_lpm_negedge(ivl_lpm_t net);
extern ivl_nexus_t ivl_lpm_clk(ivl_lpm_t net);
  /* IVL_LPM_UFUNC */
extern ivl_scope_t  ivl_lpm_define(ivl_lpm_t net);
  /* IVL_LPM_FF IVL_LPM_LATCH*/
extern ivl_nexus_t ivl_lpm_enable(ivl_lpm_t net);
  /* IVL_LPM_ADD IVL_LPM_CONCAT IVL_LPM_FF IVL_LPM_PART IVL_LPM_MULT
     IVL_LPM_MUX IVL_LPM_POW IVL_LPM_SHIFTL IVL_LPM_SHIFTR IVL_LPM_SUB
     IVL_LPM_UFUNC IVL_LPM_SUBSTITUTE IVL_LPM_LATCH */
extern ivl_nexus_t ivl_lpm_data(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_ADD IVL_LPM_MULT IVL_LPM_POW IVL_LPM_SUB IVL_LPM_CMP_EQ
     IVL_LPM_CMP_EEQ IVL_LPM_CMP_EQX IVL_LPM_CMP_EQZ IVL_LPM_CMP_NEE */
extern ivl_nexus_t ivl_lpm_datab(ivl_lpm_t net, unsigned idx);
  /* IVL_LPM_ADD IVL_LPM_FF IVL_LPM_MULT IVL_LPM_PART IVL_LPM_POW
     IVL_LPM_SUB IVL_LPM_UFUNC IVL_LPM_CMP_EEQ IVL_LPM_CMP_EQX
     IVL_LPM_CMP_EQZ IVL_LPM_CMP_NEE IVL_LPM_SUBSTITUTE IVL_LPM_LATCH */
extern ivl_nexus_t ivl_lpm_q(ivl_lpm_t net);
extern ivl_drive_t ivl_lpm_drive0(ivl_lpm_t net);
extern ivl_drive_t ivl_lpm_drive1(ivl_lpm_t net);
  /* IVL_LPM_MUX */
extern unsigned ivl_lpm_selects(ivl_lpm_t net);
  /* IVL_LPM_MUX */
extern ivl_nexus_t ivl_lpm_select(ivl_lpm_t net);
  /* IVL_LPM_CONCAT IVL_LPM_MUX IVL_LPM_REPEAT IVL_LPM_UFUNC */
extern unsigned ivl_lpm_size(ivl_lpm_t net);
  /* IVL_LPM_SFUNC */
extern const char*ivl_lpm_string(ivl_lpm_t net);

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
 * ivl_lval_mux (* obsolete *)
 *
 * ivl_lval_nest
 *    If the l-value is an object more complex than a variable, then
 *    this returns the nested l-value (and ivl_lval_sig==0).
 *
 * ivl_lval_sig
 *    If the l-value is a variable, this method returns the signal
 *    object that is the target of the assign.
 *
 * ivl_lval_part_off
 *    The part select of the signal is based here. This is the
 *    canonical index of bit-0 of the part select. The return value is
 *    an ivl_expr_t. If the return value is nil, then take the offset
 *    as zero. Otherwise, evaluate the expression to get the offset.
 *
 * ivl_lval_idx
 *    If the l-value is a memory, this method returns an
 *    ivl_expr_t that represents the index expression.  Otherwise, it
 *    returns 0.
 *
 * ivl_lval_property_idx
 *    If the l-value is a class object, this is the name of a property
 *    to select from the object. If this property is not present (<0)
 *    then the l-value represents the class object itself.
 *
 * SEMANTIC NOTES
 * The ivl_lval_width is not necessarily the same as the width of the
 * signal or memory word it represents. It is the width of the vector
 * it receives and assigns. This may be less than the width of the
 * signal (or even 1) if only a part of the l-value signal is to be
 * assigned.
 *
 * The ivl_lval_part_off is the canonical base of a part or
 * bit select.
 *
 * - Array words
 * If the l-value is an array, then ivl_lval_idx function will return
 * an expression that calculates the address of the array word. If
 * the referenced signal has more than one word, this expression must
 * be present. If the signal has exactly one word (it is not an array)
 * then the ivl_lval_idx expression must *not* be present.
 *
 * For array words, the ivl_lval_width is the width of the word.
 *
 * - Arrayed properties
 * If the l-value is a class property, then the ivl_lval_idx function
 * will return an expression if the property is in fact arrayed. The
 * expression is the canonical index for elements in the property.
 */

extern unsigned    ivl_lval_width(ivl_lval_t net);
extern ivl_expr_t  ivl_lval_mux(ivl_lval_t net) __attribute__((deprecated)); /* XXXX Obsolete? */
extern ivl_expr_t  ivl_lval_idx(ivl_lval_t net);
extern ivl_expr_t  ivl_lval_part_off(ivl_lval_t net);
extern ivl_select_type_t ivl_lval_sel_type(ivl_lval_t net);
extern int ivl_lval_property_idx(ivl_lval_t net);
extern ivl_signal_t ivl_lval_sig(ivl_lval_t net);
extern ivl_lval_t  ivl_lval_nest(ivl_lval_t net);


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
 *    does not really matter, as long as the output is not
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

extern const char*     ivl_nexus_name(ivl_nexus_t net) __attribute__((deprecated));
extern unsigned        ivl_nexus_ptrs(ivl_nexus_t net);
extern ivl_nexus_ptr_t ivl_nexus_ptr(ivl_nexus_t net, unsigned idx);

extern void  ivl_nexus_set_private(ivl_nexus_t net, void*data);
extern void* ivl_nexus_get_private(ivl_nexus_t net);


extern ivl_drive_t  ivl_nexus_ptr_drive0(ivl_nexus_ptr_t net);
extern ivl_drive_t  ivl_nexus_ptr_drive1(ivl_nexus_ptr_t net);
extern unsigned     ivl_nexus_ptr_pin(ivl_nexus_ptr_t net);
extern ivl_branch_t ivl_nexus_ptr_branch(ivl_nexus_ptr_t net);
extern ivl_net_const_t ivl_nexus_ptr_con(ivl_nexus_ptr_t net);
extern ivl_net_logic_t ivl_nexus_ptr_log(ivl_nexus_ptr_t net);
extern ivl_lpm_t    ivl_nexus_ptr_lpm(ivl_nexus_ptr_t net);
extern ivl_switch_t ivl_nexus_ptr_switch(ivl_nexus_ptr_t net);
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
 *
 * ivl_parameter_msb
 * ivl_parameter_lsb
 *    Returns the MSB and LSB for the parameter. For a parameter without
 *    a range the value is zero based and the width of the expression is
 *    used to determine the MSB.
 *
 * ivl_parameter_width
 *    return |MSB - LSB| + 1
 *
 * ivl_parameter_signed
 *    Returns if the parameter was declared to be signed.
 *
 * ivl_parameter_local
 *    Return whether parameter was local (localparam, implicit genvar etc)
 *    or not.
 *
 * ivl_parameter_file
 * ivl_parameter_lineno
 *    Returns the file and line where this parameter is defined
 */
extern const char* ivl_parameter_basename(ivl_parameter_t net);
extern ivl_scope_t ivl_parameter_scope(ivl_parameter_t net);
extern ivl_expr_t  ivl_parameter_expr(ivl_parameter_t net);
extern int         ivl_parameter_msb(ivl_parameter_t net);
extern int         ivl_parameter_lsb(ivl_parameter_t net);
extern unsigned    ivl_parameter_width(ivl_parameter_t net);
extern int         ivl_parameter_signed(ivl_parameter_t net);
extern int         ivl_parameter_local(ivl_parameter_t net);
extern const char* ivl_parameter_file(ivl_parameter_t net);
extern unsigned    ivl_parameter_lineno(ivl_parameter_t net);


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
 *    returns any value other than 0, the iteration stops and the
 *    method returns that value. Otherwise, iteration continues until
 *    the children run out.
 *
 *    If the scope has no children, this method will return 0 and
 *    otherwise do nothing.
 *
 * ivl_scope_childs
 * ivl_scope_child
 *    This is an alternative way of getting at the childs scopes of a
 *    given scope.
 *
 * ivl_scope_def
 *    Task definition scopes carry a task definition, in the form of
 *    a statement. This method accesses that definition. The
 *    ivl_scope_def function must return a statement for scopes that
 *    are type FUNCTION or TASK, and must return nil otherwise.
 *
 * ivl_scope_def_file
 * ivl_scope_def_lineno
 *    Returns the file and line where this scope is defined.
 *
 * ivl_scope_enumerate
 * ivl_scope_enumerates
 *    Scopes have 0 or more enumeration types in them.
 *
 * ivl_scope_event
 * ivl_scope_events
 *    Scopes have 0 or more event objects in them.
 *
 * ivl_scope_file
 * ivl_scope_lineno
 *    Returns the instantiation file and line for this scope.
 *
 * ivl_scope_func_type
 * ivl_scope_func_signed
 * ivl_scope_func_width
 *
 *    If the scope is a function, these function can be used to get
 *    the type of the return value.
 *
 * ivl_scope_is_auto
 *    Is the task or function declared to be automatic?
 *
 * ivl_scope_is_cell
 *    Is the module defined to be a cell?
 *
 * ivl_scope_var
 * ivl_scope_vars
 *    REMOVED
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
 * ivl_scope_time_precision
 *    Scopes have their own intrinsic time precision, typically from
 *    the timescale compiler directive. This method returns the
 *    precision as a signed power of 10 value.
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
extern const char* ivl_scope_def_file(ivl_scope_t net);
extern unsigned ivl_scope_def_lineno(ivl_scope_t net);

extern size_t      ivl_scope_childs(ivl_scope_t net);
extern ivl_scope_t ivl_scope_child(ivl_scope_t net, size_t idx);
extern unsigned   ivl_scope_classes(ivl_scope_t net);
extern ivl_type_t ivl_scope_class(ivl_scope_t net, unsigned idx);
extern unsigned       ivl_scope_enumerates(ivl_scope_t net);
extern ivl_enumtype_t ivl_scope_enumerate(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_events(ivl_scope_t net);
extern ivl_event_t  ivl_scope_event(ivl_scope_t net, unsigned idx);
extern const char* ivl_scope_file(ivl_scope_t net);
extern unsigned ivl_scope_is_auto(ivl_scope_t net);
extern unsigned ivl_scope_is_cell(ivl_scope_t net);
extern unsigned ivl_scope_lineno(ivl_scope_t net);
extern unsigned     ivl_scope_logs(ivl_scope_t net);
extern ivl_net_logic_t ivl_scope_log(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_lpms(ivl_scope_t net);
extern ivl_lpm_t    ivl_scope_lpm(ivl_scope_t, unsigned idx);
extern const char*  ivl_scope_name(ivl_scope_t net);
extern const char*  ivl_scope_basename(ivl_scope_t net);
extern unsigned     ivl_scope_params(ivl_scope_t net);
extern ivl_parameter_t ivl_scope_param(ivl_scope_t net, unsigned idx);
extern ivl_scope_t  ivl_scope_parent(ivl_scope_t net);

extern unsigned ivl_scope_mod_module_ports(ivl_scope_t net);
extern const char *ivl_scope_mod_module_port_name(ivl_scope_t net, unsigned idx );
extern ivl_signal_port_t ivl_scope_mod_module_port_type(ivl_scope_t net, unsigned idx );
extern unsigned ivl_scope_mod_module_port_width(ivl_scope_t net, unsigned idx );

extern unsigned     ivl_scope_ports(ivl_scope_t net);
extern ivl_signal_t ivl_scope_port(ivl_scope_t net, unsigned idx);
extern ivl_nexus_t  ivl_scope_mod_port(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_sigs(ivl_scope_t net);
extern ivl_signal_t ivl_scope_sig(ivl_scope_t net, unsigned idx);
extern unsigned     ivl_scope_switches(ivl_scope_t net);
extern ivl_switch_t ivl_scope_switch(ivl_scope_t net, unsigned idx);
extern ivl_scope_type_t ivl_scope_type(ivl_scope_t net);
extern const char*  ivl_scope_tname(ivl_scope_t net);
extern int          ivl_scope_time_precision(ivl_scope_t net);
extern int          ivl_scope_time_units(ivl_scope_t net);

extern ivl_variable_type_t ivl_scope_func_type(ivl_scope_t net);
extern int ivl_scope_func_signed(ivl_scope_t net);
extern unsigned ivl_scope_func_width(ivl_scope_t net);

/* SIGNALS
 * Signals are named things in the Verilog source, like wires and
 * regs, and also named things that are created as temporaries during
 * certain elaboration or optimization steps. A signal may also be a
 * port of a module or task.
 *
 * Signals have a name (obviously) and types. A signal may also be
 * signed or unsigned.
 *
 * ivl_signal_nex
 *    This is the nexus of the signal. This is used for managing
 *    connections to the rest of the net. There is exactly one pin for
 *    each word of a signal. Each word may in turn be a vector. The
 *    word address is the zero-based index for the word. It is up to
 *    the context to translate different bases to the canonical address.
 *
 * ivl_signal_array_base
 * ivl_signal_array_count
 * ivl_signal_array_addr_swapped
 *    The signal may be arrayed. If so, the array_count is >1. Each
 *    word of the array has its own nexus. The array_base is the
 *    address in the Verilog source for the canonical zero word. This
 *    may be negative, positive or zero. The array addresses may be
 *    reversed/swapped.
 *
 *    Note that arraying of the signal into words is distinct from the
 *    vectors. The width of a signal is the width of a WORD.
 *
 * ivl_signal_dimensions
 *    The signal may be an array (of vectors) in which case this
 *    function returns >0, the number of dimensions of the array.
 *
 * ivl_signal_discipline
 *    If the signal has been declared with a domain (Verilog-AMS) then
 *    this function will return a non-nil ivl_discipline_t.
 *
 * ivl_signal_msb (deprecated)
 * ivl_signal_lsb (deprecated)
 * ivl_signal_packed_dimensions
 * ivl_signal_packed_msb
 * ivl_signal_packed_lsb
 * ivl_signal_width
 *    These functions return the msb and lsb packed indices. The
 *    packed dimensions are declared differently from array
 *    dimensions, like so:
 *       reg [4:1][7:0] sig...
 *    which has two packed dimensions. The [4:1] dimension is the
 *    first, and so forth. If the signal is a scalar, it has 0
 *    dimension.
 *
 *    The ivl_signal_msb/ivl_signal_lsb functions are deprecated
 *    versions that only work with variables that have less than two
 *    dimensions. They will return msb==lsb==0 for scalars.
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
 * ivl_signal_forced_net
 *    Return whether the signal is a net that is the subject of a force
 *    statement.
 *
 * ivl_signal_type
 *    Return the type of the signal, i.e., reg, wire, tri0, etc.
 *
 * ivl_signal_data_type
 *    Return the data type of the signal, i.e. logic, real, bool,
 *    etc. All the signals connected to a nexus should have the same
 *    data type
 *
 * ivl_signal_npath
 * ivl_signal_path
 *    This function returns the delay path object for the signal. The
 *    delay path has this signal as the output, the source is attached
 *    to the delay path itself.
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
 *
 * ivl_signal_file
 * ivl_signal_lineno
 *    Returns the file and line where this signal is defined.
 */

extern ivl_scope_t ivl_signal_scope(ivl_signal_t net);
extern ivl_nexus_t ivl_signal_nex(ivl_signal_t net, unsigned word);
extern int         ivl_signal_array_base(ivl_signal_t net);
extern unsigned    ivl_signal_array_count(ivl_signal_t net);
extern unsigned    ivl_signal_array_addr_swapped(ivl_signal_t net);
extern unsigned    ivl_signal_dimensions(ivl_signal_t net);
extern ivl_discipline_t ivl_signal_discipline(ivl_signal_t net);
extern unsigned    ivl_signal_packed_dimensions(ivl_signal_t net);
extern int         ivl_signal_packed_msb(ivl_signal_t net, unsigned dim);
extern int         ivl_signal_packed_lsb(ivl_signal_t net, unsigned dim);
extern int         ivl_signal_msb(ivl_signal_t net) __attribute__((deprecated));
extern int         ivl_signal_lsb(ivl_signal_t net) __attribute__((deprecated));
extern unsigned    ivl_signal_width(ivl_signal_t net);
extern ivl_signal_port_t ivl_signal_port(ivl_signal_t net);
extern int         ivl_signal_module_port_index(ivl_signal_t net);
extern int         ivl_signal_signed(ivl_signal_t net);
extern int         ivl_signal_integer(ivl_signal_t net);
extern int         ivl_signal_local(ivl_signal_t net);
extern unsigned    ivl_signal_forced_net(ivl_signal_t net);
extern unsigned    ivl_signal_npath(ivl_signal_t net);
extern ivl_delaypath_t ivl_signal_path(ivl_signal_t net, unsigned idx);
extern ivl_signal_type_t ivl_signal_type(ivl_signal_t net);
extern ivl_variable_type_t ivl_signal_data_type(ivl_signal_t net);
extern ivl_type_t  ivl_signal_net_type(ivl_signal_t net);
extern const char* ivl_signal_name(ivl_signal_t net);
extern const char* ivl_signal_basename(ivl_signal_t net);
extern const char* ivl_signal_attr(ivl_signal_t net, const char*key);

extern const char* ivl_signal_file(ivl_signal_t net);
extern unsigned ivl_signal_lineno(ivl_signal_t net);

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
 * ivl_process_type
 * ivl_process_analog
 *    The ivl_process_type function returns the type of the process,
 *    an "initial" or "always" statement. The ivl_process_analog
 *    returns true if the process is analog.
 *
 * ivl_process_scope
 *    A process is placed in a scope. The statement within the process
 *    operates within the scope of the process unless there are calls
 *    outside the scope.
 *
 * The ivl_process_stmt function gets the statement that forms the
 * process. See the statement related functions for how to manipulate
 * statements.
 *
 * Processes can have attributes attached to them. the attr_cnt and
 * attr_val methods return those attributes.
 */
extern ivl_process_type_t ivl_process_type(ivl_process_t net);
extern int ivl_process_analog(ivl_process_t net);

extern ivl_scope_t ivl_process_scope(ivl_process_t net);

extern ivl_statement_t ivl_process_stmt(ivl_process_t net);

extern unsigned        ivl_process_attr_cnt(ivl_process_t net);
extern ivl_attribute_t ivl_process_attr_val(ivl_process_t net, unsigned idx);

extern const char* ivl_process_file(ivl_process_t net);
extern unsigned ivl_process_lineno(ivl_process_t net);

/*
 * These functions manage statements of various type. This includes
 * all the different kinds of statements (as enumerated in
 * ivl_statement_type_t) that might occur in behavioral code.
 *
 * The ivl_statement_type() function returns the type code for the
 * statement. This is the major type, and implies which of the later
 * functions are applicable to the statement.
 *
 * the ivl_statement_file() and _lineno() functions return the source
 * file and line number of the statement in the Verilog source. This
 * information is useful for diagnostic information.
 */
extern ivl_statement_type_t ivl_statement_type(ivl_statement_t net);

extern const char* ivl_stmt_file(ivl_statement_t net);
extern unsigned ivl_stmt_lineno(ivl_statement_t net);

/*
 * The following functions retrieve specific single values from the
 * statement. These values are the bits of data and parameters that
 * make up the statement. Many of these functions apply to more than
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
 * ivl_stmt_needs_t0_trigger
 * ivl_stmt_nevent
 *    Statements that have event arguments (TRIGGER and WAIT) make
 *    those event objects available through these methods.
 *
* ivl_stmt_lval
 * ivl_stmt_lvals
 *    Return the number of l-values for an assignment statement, or
 *    the specific l-value. If there is more than 1 l-value, then the
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
 * The blocking and non-blocking assignments may also have an internal
 * delay. These are of the form "lval = #<delay> rval;" and <delay> is
 * the internal delay expression. (It is internal because it is inside
 * the statement.) The ivl_stmt_delay_expr function returns the
 * expression for the delay, or nil if there is no delay expression.
 *
 * The blocking assignment (IVL_ST_ASSIGN) may have an associated
 * opcode, that can be extracted from ivl_stmt_opcode(). This opcode
 * is the compressed operator used it statements like this:
 *      foo += <expr>
 * The ivl_stmt_opcode() returns null (0) if this is not a compressed
 * assignment statement.
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
 * - IVL_ST_CONTRIB
 * This is an analog contribution statement. The ivl_stmt_lexp
 * function returns the l-value expression which is guaranteed to be a
 * branch access expression. The ivl_stmt_rval returns the r-value
 * expression for the assignment.
 *
 * - IVL_ST_DELAY, IVL_ST_DELAYX
 * These statement types are delay statements. They are a way to
 * attach a delay to a statement. The ivl_stmt_sub_stmt() function
 * gets the statement to be executed after the delay. If this is
 * IVL_ST_DELAY, then the ivl_stmt_delay_val function gets the
 * constant delay. If this is IVL_ST_DELAYX, then the
 * ivl_stmt_delay_expr gets the expression of the delay. In this case,
 * the expression is not necessarily constant.
 *
 * Whether constant or calculated, the resulting delay is in units of
 * simulation ticks. The compiler has already taken care of converting
 * the delay to the time scale/precision of the scope.
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

  /* IVL_ST_BLOCK, IVL_ST_FORK, IVL_ST_FORK_JOIN_ANY, IVL_ST_FORK_JOIN_NONE */
extern unsigned ivl_stmt_block_count(ivl_statement_t net);
  /* IVL_ST_BLOCK, IVL_ST_FORK, IVL_ST_FORK_JOIN_ANY, IVL_ST_FORK_JOIN_NONE */
extern ivl_scope_t ivl_stmt_block_scope(ivl_statement_t net);
  /* IVL_ST_BLOCK, IVL_ST_FORK, IVL_ST_FORK_JOIN_ANY, IVL_ST_FORK_JOIN_NONE */
extern ivl_statement_t ivl_stmt_block_stmt(ivl_statement_t net, unsigned i);
  /* IVL_ST_UTASK IVL_ST_DISABLE */
extern ivl_scope_t ivl_stmt_call(ivl_statement_t net);
  /* IVL_ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern unsigned ivl_stmt_case_count(ivl_statement_t net);
  /* IVL_ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern ivl_expr_t ivl_stmt_case_expr(ivl_statement_t net, unsigned i);
  /* IVL+ST_CASE,IVL_ST_CASER,IVL_ST_CASEX,IVL_ST_CASEZ */
extern ivl_case_quality_t ivl_stmt_case_quality(ivl_statement_t net);
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
extern uint64_t ivl_stmt_delay_val(ivl_statement_t net);
  /* IVL_ST_WAIT IVL_ST_TRIGGER */
extern unsigned    ivl_stmt_needs_t0_trigger(ivl_statement_t net);
extern unsigned    ivl_stmt_nevent(ivl_statement_t net);
extern ivl_event_t ivl_stmt_events(ivl_statement_t net, unsigned idx);
  /* IVL_ST_DISABLE */
extern bool ivl_stmt_flow_control(ivl_statement_t net);
  /* IVL_ST_CONTRIB */
extern ivl_expr_t ivl_stmt_lexp(ivl_statement_t net);
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
  /* IVL_ST_ASSIGN */
extern char ivl_stmt_opcode(ivl_statement_t net);
  /* IVL_ST_STASK */
extern ivl_expr_t ivl_stmt_parm(ivl_statement_t net, unsigned idx);
  /* IVL_ST_STASK */
extern unsigned ivl_stmt_parm_count(ivl_statement_t net);
  /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB IVL_ST_CASSIGN IVL_ST_CONTRIB
     IVL_ST_FORCE */
extern ivl_expr_t ivl_stmt_rval(ivl_statement_t net);
  /* IVL_ST_STASK */
extern ivl_sfunc_as_task_t ivl_stmt_sfunc_as_task(ivl_statement_t net);
  /* IVL_ST_DELAY, IVL_ST_DELAYX, IVL_ST_FOREVER, IVL_ST_REPEAT
     IVL_ST_WAIT, IVL_ST_WHILE */
extern ivl_statement_t ivl_stmt_sub_stmt(ivl_statement_t net);

/* SWITCHES
 *
 * The switches represent the tran devices in the design.
 *
 * FUNCTION SUMMARY
 *
 * ivl_switch_type
 *    Return the enumerated value that is the type of the switch.
 *
 * ivl_switch_basename
 *    This is the name given to the device in the source code.
 *
 * ivl_switch_a
 * ivl_switch_b
 *    The a and b ports are the two ports of the switch.
 *
 * ivl_switch_enable
 *    If the device has an enable (tranifX) then this is the enable
 *    port.
 *
 * SEMANTIC NOTES
 * The a/b ports can be any type, but the types must exactly
 * match, including vector widths. The enable must be a scalar.
 *
 * The IVL_SW_TRAN_VP is an exception to the above. In this case,
 * the B side may be a different size, and the a side will have a
 * a fixed width. The unused bits are padded to Z on the A side.
 */
extern ivl_switch_type_t ivl_switch_type(ivl_switch_t net);
extern ivl_scope_t ivl_switch_scope(ivl_switch_t net);
extern const char*ivl_switch_basename(ivl_switch_t net);
extern ivl_nexus_t ivl_switch_a(ivl_switch_t net);
extern ivl_nexus_t ivl_switch_b(ivl_switch_t net);
extern ivl_nexus_t ivl_switch_enable(ivl_switch_t net);
extern ivl_island_t ivl_switch_island(ivl_switch_t net);

  /* These are only support for IVL_SW_TRAN_VP switches. */
extern unsigned ivl_switch_width(ivl_switch_t net);
extern unsigned ivl_switch_part(ivl_switch_t net);
extern unsigned ivl_switch_offset(ivl_switch_t net);
extern ivl_expr_t ivl_switch_delay(ivl_switch_t net, unsigned transition);

/* Not implemented yet
extern unsigned        ivl_switch_attr_cnt(ivl_switch_t net);
extern ivl_attribute_t ivl_switch_attr_val(ivl_switch_t net, unsigned idx);
*** */
extern const char* ivl_switch_file(ivl_switch_t net);
extern unsigned ivl_switch_lineno(ivl_switch_t net);

/* TYPES
 *
 * ivl_type_base
 *    This returns the base type for the type. See the
 *    ivl_variable_type_t definition for the various base types.
 *
 * ivl_type_element
 *    Return the type of the element of an array. This is only valid
 *    for array types.
 *
 * ivl_type_signed
 *    Return TRUE if the type represents a signed packed vector or
 *    signed atomic type, and FALSE otherwise.
 *
 * SEMANTIC NOTES
 *
 * Class types have names and properties.
 */
extern ivl_variable_type_t ivl_type_base(ivl_type_t net);
extern ivl_type_t ivl_type_element(ivl_type_t net);
extern unsigned ivl_type_packed_dimensions(ivl_type_t net);
extern int ivl_type_packed_lsb(ivl_type_t net, unsigned dim);
extern int ivl_type_packed_msb(ivl_type_t net, unsigned dim);
extern int ivl_type_signed(ivl_type_t net);
extern const char* ivl_type_name(ivl_type_t net);
extern int         ivl_type_properties(ivl_type_t net);
extern const char* ivl_type_prop_name(ivl_type_t net, int idx);
extern ivl_type_t  ivl_type_prop_type(ivl_type_t net, int idx);


#if defined(__MINGW32__) || defined (__CYGWIN__)
#  define DLLEXPORT __declspec(dllexport)
#else
#  define DLLEXPORT
#endif

extern DLLEXPORT int target_design(ivl_design_t des);
extern DLLEXPORT const char* target_query(const char*key);

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
typedef const char* (*target_query_f) (const char*key);


_END_DECL

#undef ENUM_UNSIGNED_INT

#endif /* IVL_ivl_target_H */
