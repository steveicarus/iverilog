#ifndef __t_dll_H
#define __t_dll_H
/*
 * Copyright (c) 2000-2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: t-dll.h,v 1.98 2003/02/06 16:43:20 steve Exp $"
#endif

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
      void lpm_divide(const NetDivide*);
      void lpm_ff(const NetFF*);
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
      string dll_path_;

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

      void func_def(const NetScope*);
      void task_def(const NetScope*);

      struct ivl_expr_s*expr_;
      void expr_binary(const NetEBinary*);
      void expr_concat(const NetEConcat*);
      void expr_memory(const NetEMemory*);
      void expr_const(const NetEConst*);
      void expr_creal(const NetECReal*);
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
      void add_root(ivl_design_s &des_, const NetScope *s);

      void sub_off_from_expr_(long);
      void mul_expr_by_const_(long);

      static ivl_expr_t expr_from_value_(const verinum&that);
};

/*
 * These are various private declarations used by the t-dll target.
 */

struct ivl_event_s {
      const char*name;
      ivl_scope_t scope;
      unsigned short nany, nneg, npos;
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

      unsigned width_  :24;
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
		  unsigned   rept  :16;
		  unsigned   parms :16;
		  ivl_expr_t*parm;
	    } concat_;

	    struct {
		  char*bits_;
	    } number_;

	    struct {
		  ivl_scope_t scope;
	    } scope_;

	    struct {
		  ivl_signal_t sig;
		  unsigned lsi, msi;
	    } signal_;

	    struct {
		  char*name_;
		  ivl_expr_t    *parm;
		  unsigned short parms;
	    } sfunc_;

	    struct {
		  char*value_;
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
		  ivl_expr_t    *parm;
		  unsigned short parms;
	    } ufunc_;

	    struct {
		  unsigned long value;
	    } ulong_;

	    struct {
		  double value;
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
      const char* name;

      union {
	    struct ivl_lpm_ff_s {
		  unsigned short width;
		  unsigned short swid; // ram only
		  ivl_nexus_t clk;
		  ivl_nexus_t we;
		  ivl_nexus_t aclr;
		  ivl_nexus_t aset;
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
		  ivl_memory_t mem; // ram only
		  ivl_expr_t aset_value;
	    } ff;

	    struct ivl_lpm_mux_s {
		  unsigned short width;
		  unsigned short size;
		  unsigned short swid;
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

	    struct ivl_lpm_shift_s {
		  unsigned short width;
		  unsigned short select;
		  ivl_nexus_t*q;
		  ivl_nexus_t*d;
		  ivl_nexus_t*s;
	    } shift;

	    struct ivl_lpm_arith_s {
		  unsigned short width;
		  ivl_nexus_t*q, *a, *b;
	    } arith;

	    struct ivl_lpm_ufunc_s {
		  ivl_scope_t def;
		  unsigned short ports;
		  unsigned short*port_wid;
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
      unsigned width_  :24;
      unsigned loff_   :24;
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
      unsigned width_  :24;
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

      const char* name_;
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
      const char* name;
      unsigned nin;
      unsigned short sequ;
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
      unsigned pin_  :24;
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
      const char*basename_;
      ivl_scope_t scope_;
      unsigned width_  :24;
      unsigned signed_ : 1;
      unsigned size_;
      int root_;
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

      const char* name_;
      const char* tname_;
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

	/* Scopes that are tasks/functions have a definition. */
      ivl_statement_t def;

      unsigned ports;
      ivl_signal_t*port;

      signed int time_units :8;
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

      unsigned width_  :24;
      unsigned signed_ : 1;
      unsigned isint_  : 1;
      unsigned local_  : 1;

	/* These encode the run-time index for the least significant
	   bit, and the distance to the second bit. */
      signed lsb_index :24;
      signed lsb_dist  : 8;

      const char*name_;
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
		  char* name_;
		  unsigned nparm_;
		  ivl_expr_t*parms_;
	    } stask_;

	    struct { /* IVL_ST_TRIGGER */
		  ivl_event_t event_;
	    } trig_;

	    struct { /* IVL_ST_UTASK */
		  ivl_scope_t def;
	    } utask_;

	    struct { /* IVL_ST_WAIT */
		  ivl_event_t event_;
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
      const char* name;
      ivl_scope_t scope;
};

/*
 * $Log: t-dll.h,v $
 * Revision 1.98  2003/02/06 16:43:20  steve
 *  Satisfy declaration requirements of some picky compilers.
 *
 * Revision 1.97  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.96  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 *
 * Revision 1.95  2002/10/23 01:47:17  steve
 *  Fix synth2 handling of aset/aclr signals where
 *  flip-flops are split by begin-end blocks.
 *
 * Revision 1.94  2002/09/26 03:18:04  steve
 *  Generate vvp code for asynch set/reset of NetFF.
 *
 * Revision 1.93  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.92  2002/08/07 02:36:10  steve
 *  Get local StringHeap.h
 *
 * Revision 1.91  2002/08/07 00:54:39  steve
 *  Add force to nets.
 *
 * Revision 1.90  2002/08/05 04:18:45  steve
 *  Store only the base name of memories.
 *
 * Revision 1.89  2002/08/04 19:13:16  steve
 *  dll uses StringHeap for named items.
 *
 * Revision 1.88  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.87  2002/07/05 21:26:17  steve
 *  Avoid emitting to vvp local net symbols.
 *
 * Revision 1.86  2002/06/21 04:59:35  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.85  2002/06/16 20:39:12  steve
 *  Normalize run-time index expressions for bit selects
 *
 * Revision 1.84  2002/06/16 19:19:16  steve
 *  Generate runtime code to normalize indices.
 *
 * Revision 1.83  2002/06/05 03:44:25  steve
 *  Add support for memory words in l-value of
 *  non-blocking assignments, and remove the special
 *  NetAssignMem_ and NetAssignMemNB classes.
 *
 * Revision 1.82  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.81  2002/05/29 22:05:55  steve
 *  Offset lvalue index expressions.
 *
 * Revision 1.80  2002/05/27 00:08:45  steve
 *  Support carrying the scope of named begin-end
 *  blocks down to the code generator, and have
 *  the vvp code generator use that to support disable.
 *
 * Revision 1.79  2002/05/26 01:39:03  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 */
#endif
