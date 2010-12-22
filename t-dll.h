#ifndef __t_dll_H
#define __t_dll_H
/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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

# include  "target.h"
# include  "ivl_target.h"
# include  "StringHeap.h"
# include  "netlist.h"

#if defined(__MINGW32__)
#include <windows.h>
typedef void *ivl_dll_t;
#elif defined(HAVE_DLFCN_H)
# include  <dlfcn.h>
typedef void* ivl_dll_t;
#elif defined(HAVE_DL_H)
# include  <dl.h>
typedef shl_t ivl_dll_t;
#else
# error No DLL stub support for this target.
#endif

struct ivl_design_s {

      int time_precision;

      ivl_scope_t *roots_;
      unsigned nroots_;

      ivl_process_t threads_;

      ivl_net_const_t*consts;
      unsigned nconsts;

      const Design*self;
};

/*
 * The DLL target type loads a named object file to handle the process
 * of scanning the netlist. When it is time to start the design, I
 * locate and link in the desired DLL, then start calling methods. The
 * DLL will call me back to get information out of the netlist in
 * particular.
 */
struct dll_target  : public target_t, public expr_scan_t {

      bool start_design(const Design*);
      int  end_design(const Design*);

      bool bufz(const NetBUFZ*);
      void event(const NetEvent*);
      void variable(const NetVariable*);
      void logic(const NetLogic*);
      void net_case_cmp(const NetCaseCmp*);
      void udp(const NetUDP*);
      void lpm_add_sub(const NetAddSub*);
      void lpm_clshift(const NetCLShift*);
      void lpm_compare(const NetCompare*);
      bool lpm_decode(const NetDecode*);
      bool lpm_demux(const NetDemux*);
      void lpm_divide(const NetDivide*);
      void lpm_ff(const NetFF*);
      void lpm_latch( const NetLatch * );
      void lpm_modulo(const NetModulo*);
      void lpm_mult(const NetMult*);
      void lpm_mux(const NetMux*);
      void lpm_ram_dq(const NetRamDq*);
      void net_assign(const NetAssign_*);
      bool net_cassign(const NetCAssign*);
      bool net_force(const NetForce*);
      bool net_function(const NetUserFunc*);
      bool net_const(const NetConst*);
      void net_probe(const NetEvProbe*);

      bool process(const NetProcTop*);
      void scope(const NetScope*);
      void signal(const NetNet*);
      void memory(const NetMemory*);

      ivl_dll_t dll_;

      ivl_design_s des_;

      target_design_f target_;


	/* These methods and members are used for forming the
	   statements of a thread. */
      struct ivl_statement_s*stmt_cur_;
      void proc_assign(const NetAssign*);
      void proc_assign_nb(const NetAssignNB*);
      bool proc_block(const NetBlock*);
      void proc_case(const NetCase*);
      bool proc_cassign(const NetCAssign*);
      bool proc_condit(const NetCondit*);
      bool proc_deassign(const NetDeassign*);
      bool proc_delay(const NetPDelay*);
      bool proc_disable(const NetDisable*);
      bool proc_force(const NetForce*);
      void proc_forever(const NetForever*);
      bool proc_release(const NetRelease*);
      void proc_repeat(const NetRepeat*);
      void proc_stask(const NetSTask*);
      bool proc_trigger(const NetEvTrig*);
      void proc_utask(const NetUTask*);
      bool proc_wait(const NetEvWait*);
      void proc_while(const NetWhile*);

      bool func_def(const NetScope*);
      void task_def(const NetScope*);

      struct ivl_expr_s*expr_;
      void expr_binary(const NetEBinary*);
      void expr_concat(const NetEConcat*);
      void expr_memory(const NetEMemory*);
      void expr_const(const NetEConst*);
      void expr_creal(const NetECReal*);
      void expr_param(const NetEConstParam*);
      void expr_event(const NetEEvent*);
      void expr_scope(const NetEScope*);
      void expr_select(const NetESelect*);
      void expr_sfunc(const NetESFunc*);
      void expr_subsignal(const NetEBitSel*);
      void expr_ternary(const NetETernary*);
      void expr_ufunc(const NetEUFunc*);
      void expr_unary(const NetEUnary*);
      void expr_signal(const NetESignal*);
      void expr_variable(const NetEVariable*);

      ivl_scope_t lookup_scope_(const NetScope*scope);

      ivl_attribute_s* fill_in_attributes(const Attrib*net);
      void logic_attributes(struct ivl_net_logic_s *obj, const NetNode*net);

    private:
      StringHeap strings_;

      static ivl_scope_t find_scope(ivl_design_s &des, const NetScope*cur);
      static ivl_signal_t find_signal(ivl_design_s &des, const NetNet*net);
      static ivl_memory_t find_memory(ivl_design_s &des, const NetMemory*net);
      static ivl_variable_t find_variable(ivl_design_s &des, const NetVariable*net);

      static ivl_parameter_t scope_find_param(ivl_scope_t scope,
					      const char*name);

      void add_root(ivl_design_s &des_, const NetScope *s);

      ivl_lpm_t lpm_decode_ff_(const NetDecode*);

      void sub_off_from_expr_(long);
      void mul_expr_by_const_(long);

      void make_scope_parameters(ivl_scope_t scope, const NetScope*net);

      static ivl_expr_t expr_from_value_(const verinum&that);
};

/*
 * These are various private declarations used by the t-dll target.
 */

struct ivl_event_s {
      perm_string name;
      ivl_scope_t scope;
      unsigned nany, nneg, npos;
      ivl_nexus_t*pins;
};

/*
 * The ivl_expr_t is an opaque reference to one of these
 * structures. This structure holds all the information we need about
 * an expression node, including its type, the expression width, and
 * type specific properties.
 */
struct ivl_expr_s {
      ivl_expr_type_t type_;
      ivl_variable_type_t value_;

      unsigned width_;
      unsigned signed_ : 1;

      union {
	    struct {
		  char op_;
		  ivl_expr_t lef_;
		  ivl_expr_t rig_;
	    } binary_;

	    struct {
		  ivl_signal_t sig;
		  ivl_expr_t bit;
	    } bitsel_;

	    struct {
		  unsigned   rept;
		  unsigned   parms;
		  ivl_expr_t*parm;
	    } concat_;

	    struct {
		  char*bits_;
		  ivl_parameter_t parameter;
	    } number_;

	    struct {
		  ivl_event_t event;
	    } event_;

	    struct {
		  ivl_scope_t scope;
	    } scope_;

	    struct {
		  ivl_signal_t sig;
		  unsigned lsi, msi;
	    } signal_;

	    struct {
		  const char *name_;
		  ivl_expr_t *parm;
		  unsigned   parms;
	    } sfunc_;

	    struct {
		  char*value_;
		  ivl_parameter_t parameter;
	    } string_;

	    struct {
		  ivl_expr_t cond;
		  ivl_expr_t true_e;
		  ivl_expr_t false_e;
	    } ternary_;

	    struct {
		  ivl_memory_t mem_;
		  ivl_expr_t idx_;
	    } memory_;

	    struct {
		  ivl_scope_t def;
		  ivl_expr_t  *parm;
		  unsigned    parms;
	    } ufunc_;

	    struct {
		  unsigned long value;
	    } ulong_;

	    struct {
		  double value;
		  ivl_parameter_t parameter;
	    } real_;

	    struct {
		  char op_;
		  ivl_expr_t sub_;
	    } unary_;

	    struct {
		  ivl_variable_t var;
	    } variable_;
      } u_;
};

/*
 * LPM devices are handled by this suite of types. The ivl_lpm_s
 * structure holds the core, including a type code, the object name
 * and scope. The other properties of the device are held in the type
 * specific member of the union.
 */

struct ivl_lpm_s {
      ivl_lpm_type_t type;
      ivl_scope_t scope;
      perm_string name;

      struct ivl_attribute_s*attr;
      unsigned nattr;

      union {
	    struct ivl_lpm_ff_s {
		  unsigned width;
		  unsigned swid; // ram only
		  unsigned scnt;
		  ivl_nexus_t clk;
		  ivl_nexus_t we;
		  ivl_nexus_t aclr;
		  ivl_nexus_t aset;
		  ivl_nexus_t sclr;
		  ivl_nexus_t sset;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } q;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } d;
		  union { // ram only
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } s;
		  union {
			ivl_memory_t mem; // ram only
			ivl_lpm_t decode; // FF only
		  } a;
		  ivl_expr_t aset_value;
		  ivl_expr_t sset_value;
	    } ff;

	    struct ivl_lpm_mux_s {
		  unsigned width;
		  unsigned size;
		  unsigned swid;
		  ivl_nexus_t*d;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } q;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } s;
	    } mux;

	    struct ivl_lpm_demux_s {
		  unsigned width;
		  unsigned awid;
		  unsigned size;
		  ivl_nexus_t*bit_in;
		  ivl_nexus_t *q,*d,*a;
	    } demux;

	    struct ivl_lpm_shift_s {
		  unsigned width;
		  unsigned select;
		  unsigned signed_flag :1;
		  ivl_nexus_t*q;
		  ivl_nexus_t*d;
		  ivl_nexus_t*s;
	    } shift;

	    struct ivl_lpm_arith_s {
		  unsigned width;
		  unsigned signed_flag :1;
		  ivl_nexus_t*q, *a, *b;
	    } arith;

	    struct ivl_lpm_ufunc_s {
		  ivl_scope_t def;
		  unsigned ports;
		  unsigned *port_wid;
		  ivl_nexus_t*pins;
	    } ufunc;
      } u_;
};

/*
 * This object represents l-values to assignments. The l-value can be
 * a register bit or part select, or a memory word select with a part
 * select.
 */

enum ivl_lval_type_t {
      IVL_LVAL_REG = 0,
      IVL_LVAL_MUX = 1,
      IVL_LVAL_MEM = 2,
      IVL_LVAL_NET = 3, /* Only force can have NET l-values */
      IVL_LVAL_VAR = 4
};

struct ivl_lval_s {
      unsigned width_;
      unsigned loff_;
      unsigned type_   : 8;
      ivl_expr_t idx;
      union {
	    ivl_signal_t sig;
	    ivl_memory_t mem;
	    ivl_variable_t var;
      } n;
};

/*
 * This object represents a vector constant, possibly signed, in a
 * structural context.
 */
struct ivl_net_const_s {
      unsigned width_;
      unsigned signed_ : 1;

      union {
	    char bit_[sizeof(char*)];
	    char *bits_;
      } b;

      union {
	    ivl_nexus_t pin_;
	    ivl_nexus_t*pins_;
      } n;
};

/*
 * Logic gates (just about everything that has a single output) are
 * represented structurally by instances of this object.
 */
struct ivl_net_logic_s {
      ivl_logic_t type_;
      ivl_udp_t udp;

      perm_string name_;
      ivl_scope_t scope_;

      unsigned npins_;
      ivl_nexus_t*pins_;

      struct ivl_attribute_s*attr;
      unsigned nattr;

      unsigned delay[3];
};


/*
 * UDP definition.
 */
struct ivl_udp_s {
      perm_string name;
      unsigned nin;
      unsigned sequ;
      char init;
      unsigned nrows;
      typedef const char*ccharp_t;
      ccharp_t*table; // zero terminated array of pointers
};

/*
 * The ivl_nexus_t is a single-bit link of some number of pins of
 * devices. the __nexus_ptr structure is a helper that actually does
 * the pointing.
 *
 * The type_ member specifies which of the object pointers in the
 * union are valid.
 *
 * The drive01 members gives the strength of the drive that the device
 * is applying to the nexus, with 0 HiZ and 3 supply. If the pin is an
 * input to the device, then the drives are both HiZ.
 */
struct ivl_nexus_ptr_s {
      unsigned pin_;
      unsigned type_ : 8;
      unsigned drive0 : 3;
      unsigned drive1 : 3;
      union {
	    ivl_signal_t    sig; /* type 0 */
	    ivl_net_logic_t log; /* type 1 */
	    ivl_net_const_t con; /* type 2 */
	    ivl_lpm_t       lpm; /* type 3 */
      } l;
};
# define __NEXUS_PTR_SIG 0
# define __NEXUS_PTR_LOG 1
# define __NEXUS_PTR_CON 2
# define __NEXUS_PTR_LPM 3

struct ivl_nexus_s {
      unsigned nptr_;
      struct ivl_nexus_ptr_s*ptrs_;
      const char*name_;
      void*private_data;
};


/*
 * Memory.
 */
struct ivl_memory_s {
      perm_string basename_;
      ivl_scope_t scope_;
      unsigned width_;
      unsigned signed_ : 1;
      unsigned size_;
      int root_;
};

/*
 * This is the implementation of a parameter. Each scope has a list of
 * these.
 */
struct ivl_parameter_s {
      perm_string basename;
      ivl_scope_t scope;
      ivl_expr_t  value;
};
/*
 * All we know about a process it its type (initial or always) and the
 * single statement that is it. A process also has a scope, although
 * that generally only matters for VPI calls.
 */
struct ivl_process_s {
      ivl_process_type_t type_;
      ivl_scope_t scope_;
      ivl_statement_t stmt_;

      struct ivl_attribute_s*attr;
      unsigned nattr;

      ivl_process_t next_;
};

/*
 * Scopes are kept in a tree. Each scope points to its first child,
 * and also to any siblings. Thus a parent can scan all its children
 * by following its child pointer then following sibling pointers from
 * there.
 */
struct ivl_scope_s {
      ivl_scope_t child_, sibling_, parent;

      perm_string name_;
      perm_string tname_;
      ivl_scope_type_t type_;

      unsigned nsigs_;
      ivl_signal_t*sigs_;

      unsigned nlog_;
      ivl_net_logic_t*log_;

      unsigned nevent_;
      ivl_event_t* event_;

      unsigned nlpm_;
      ivl_lpm_t* lpm_;

      unsigned nmem_;
      ivl_memory_t* mem_;

      unsigned nvar_;
      ivl_variable_t* var_;

      unsigned nparam_;
      ivl_parameter_t param_;

	/* Scopes that are tasks/functions have a definition. */
      ivl_statement_t def;

      unsigned ports;
      ivl_signal_t*port;

      signed int time_units :8;

      struct ivl_attribute_s*attr;
      unsigned nattr;
};

/*
 * A signal is a thing like a wire, a reg, or whatever. It has a type,
 * and if it is a port is also has a direction. Signals are collected
 * into scopes (which also point back to me) and have pins that
 * connect to the rest of the netlist.
 */
struct ivl_signal_s {
      ivl_signal_type_t type_;
      ivl_signal_port_t port_;

      unsigned width_;
      unsigned signed_ : 1;
      unsigned isint_  : 1;
      unsigned local_  : 1;

	/* These encode the run-time index for the least significant
	   bit, and the distance to the second bit. */
      signed lsb_index;
      signed lsb_dist;

      perm_string name_;
      ivl_scope_t scope_;

      union {
	    ivl_nexus_t pin_;
	    ivl_nexus_t*pins_;
      } n;

      struct ivl_attribute_s*attr;
      unsigned nattr;
};

/*
 * The ivl_statement_t represents any statement. The type of statement
 * is defined by the ivl_statement_type_t enumeration. Given the type,
 * certain information about the statement may be available.
 */
struct ivl_statement_s {
      enum ivl_statement_type_e type_;
      union {
	    struct { /* IVL_ST_ASSIGN IVL_ST_ASSIGN_NB */
		  unsigned lvals_;
		  struct ivl_lval_s*lval_;
		  ivl_expr_t rval_;
		  ivl_expr_t delay;
	    } assign_;

	    struct { /* IVL_ST_BLOCK, IVL_ST_FORK */
		  struct ivl_statement_s*stmt_;
		  unsigned nstmt_;
		  ivl_scope_t scope;
	    } block_;

	    struct { /* IVL_ST_CASE, IVL_ST_CASEX, IVL_ST_CASEZ */
		  ivl_expr_t cond;
		  unsigned ncase;
		  ivl_expr_t*case_ex;
		  struct ivl_statement_s*case_st;
	    } case_;

	    struct { /* IVL_ST_CASSIGN, IVL_ST_DEASSIGN */
		  unsigned lvals;
		  struct ivl_lval_s*lval;
		  unsigned npins;
		  ivl_nexus_t*pins;
	    } cassign_;

	    struct { /* IVL_ST_CONDIT */
		    /* This is the condition expression */
		  ivl_expr_t cond_;
		    /* This is two statements, the true and false. */
		  struct ivl_statement_s*stmt_;
	    } condit_;

	    struct { /* IVL_ST_DELAY */
		  unsigned long delay_;
		  ivl_statement_t stmt_;
	    } delay_;

	    struct { /* IVL_ST_DELAYX */
		  ivl_expr_t expr; /* XXXX */
		  ivl_statement_t stmt_;
	    } delayx_;

	    struct { /* IVL_ST_DISABLE */
		  ivl_scope_t scope;
	    } disable_;

	    struct { /* IVL_ST_FOREVER */
		  ivl_statement_t stmt_;
	    } forever_;

	    struct { /* IVL_ST_STASK */
		  const char*name_;
		  unsigned   nparm_;
		  ivl_expr_t*parms_;
	    } stask_;

	    struct { /* IVL_ST_UTASK */
		  ivl_scope_t def;
	    } utask_;

	    struct { /* IVL_ST_TRIGGER IVL_ST_WAIT */
		  unsigned nevent;
		  union {
			ivl_event_t event;
			ivl_event_t*events;
		  };
		  ivl_statement_t stmt_;
	    } wait_;

	    struct { /* IVL_ST_WHILE IVL_ST_REPEAT */
		  ivl_expr_t cond_;
		  ivl_statement_t stmt_;
	    } while_;
      } u_;
};

/*
 * This holds the details about a variable object.
 */
struct ivl_variable_s {
      ivl_variable_type_t type;
      perm_string name;
      ivl_scope_t scope;
};

#endif
