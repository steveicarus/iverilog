#ifndef __t_dll_H
#define __t_dll_H
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
#ident "$Id: t-dll.h,v 1.74 2002/01/19 19:02:08 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"

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
      void proc_assign_mem(const NetAssignMem*);
      void proc_assign_nb(const NetAssignNB*);
      void proc_assign_mem_nb(const NetAssignMemNB*net);
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
      void expr_scope(const NetEScope*);
      void expr_sfunc(const NetESFunc*);
      void expr_subsignal(const NetEBitSel*);
      void expr_ternary(const NetETernary*);
      void expr_ufunc(const NetEUFunc*);
      void expr_unary(const NetEUnary*);
      void expr_signal(const NetESignal*);

      ivl_scope_t lookup_scope_(const NetScope*scope);
      ivl_memory_t lookup_memory_(const NetMemory*mem);

    private:
      static ivl_scope_t find_scope(ivl_design_s &des, const NetScope*cur);
      static ivl_signal_t find_signal(ivl_design_s &des, const NetNet*net);
      void add_root(ivl_design_s &des_, const NetScope *s);
};

/*
 * These are various private declarations used by the t-dll target.
 */

struct ivl_event_s {
      char*name;
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
		  char op_;
		  ivl_expr_t sub_;
	    } unary_;

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
      char* name;

      union {
	    struct ivl_lpm_ff_s {
		  unsigned short width;
		  unsigned short swid; // ram only
		  ivl_nexus_t clk;
		  ivl_nexus_t we;
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
};

struct ivl_lval_s {
      unsigned width_  :24;
      unsigned loff_   :24;
      unsigned type_   : 8;
      ivl_expr_t idx;
      union {
	    ivl_signal_t sig;
	    ivl_memory_t mem;
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

      char* name_;
      ivl_scope_t scope_;

      unsigned npins_;
      ivl_nexus_t*pins_;

      char**akey_;
      char**aval_;
      unsigned nattr_;

      unsigned delay[3];
};


/*
 * UDP definition.
 */
struct ivl_udp_s {
      char* name;
      unsigned nin;
      unsigned short sequ;
      char init;
      unsigned nrows;
      char **table; // zero terminated array of pointers
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
      char*name_;
      void*private_data;
};


/* 
 * Memory.
 */
struct ivl_memory_s {
      char*name_;
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

      char* name_;
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

	/* Scopes that are tasks/functions have a definition. */
      ivl_statement_t def;

      unsigned ports;
      ivl_signal_t*port;
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

      char*name_;
      ivl_scope_t scope_;

      union {
	    ivl_nexus_t pin_;
	    ivl_nexus_t*pins_;
      } n;

      char**akey_;
      char**aval_;
      unsigned nattr_;
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
 * $Log: t-dll.h,v $
 * Revision 1.74  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.73  2002/01/03 04:19:01  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.72  2001/12/06 03:11:01  steve
 *  Add ivl_logic_delay function to ivl_target.
 *
 * Revision 1.71  2001/11/14 03:28:49  steve
 *  DLL target support for force and release.
 *
 * Revision 1.70  2001/11/04 05:03:21  steve
 *  MacOSX 10.1 updates.
 *
 * Revision 1.69  2001/11/01 04:25:31  steve
 *  ivl_target support for cassign.
 *
 * Revision 1.68  2001/10/31 05:24:52  steve
 *  ivl_target support for assign/deassign.
 *
 * Revision 1.67  2001/10/30 02:52:07  steve
 *  Stubs for assign/deassign for t-dll.
 *
 * Revision 1.66  2001/10/19 21:53:24  steve
 *  Support multiple root modules (Philip Blundell)
 *
 * Revision 1.65  2001/10/16 02:19:27  steve
 *  Support IVL_LPM_DIVIDE for structural divide.
 *
 * Revision 1.64  2001/09/16 22:19:42  steve
 *  Support attributes to logic gates.
 *
 * Revision 1.63  2001/09/01 01:57:31  steve
 *  Make constants available through the design root
 *
 * Revision 1.62  2001/08/31 22:58:40  steve
 *  Support DFF CE inputs.
 *
 * Revision 1.61  2001/08/28 04:07:41  steve
 *  Add some ivl_target convenience functions.
 *
 * Revision 1.60  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.59  2001/08/10 00:40:45  steve
 *  tgt-vvp generates code that skips nets as inputs.
 *
 * Revision 1.58  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.57  2001/07/27 02:41:56  steve
 *  Fix binding of dangling function ports. do not elide them.
 *
 * Revision 1.56  2001/07/22 00:17:50  steve
 *  Support the NetESubSignal expressions in vvp.tgt.
 *
 * Revision 1.55  2001/07/19 04:55:06  steve
 *  Support calculated delays in vvp.tgt.
 *
 * Revision 1.54  2001/07/07 20:20:10  steve
 *  Pass parameters to system functions.
 *
 * Revision 1.53  2001/07/04 22:59:25  steve
 *  handle left shifter in dll output.
 *
 * Revision 1.52  2001/06/30 23:03:16  steve
 *  support fast programming by only writing the bits
 *  that are listed in the input file.
 *
 * Revision 1.51  2001/06/30 21:07:26  steve
 *  Support non-const right shift (unsigned).
 *
 * Revision 1.50  2001/06/19 03:01:10  steve
 *  Add structural EEQ gates (Stephan Boettcher)
 *
 * Revision 1.49  2001/06/16 23:45:05  steve
 *  Add support for structural multiply in t-dll.
 *  Add code generators and vvp support for both
 *  structural and behavioral multiply.
 *
 * Revision 1.48  2001/06/16 02:41:42  steve
 *  Generate code to support memory access in continuous
 *  assignment statements. (Stephan Boettcher)
 *
 * Revision 1.47  2001/06/15 04:14:19  steve
 *  Generate vvp code for GT and GE comparisons.
 *
 * Revision 1.46  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.45  2001/05/20 15:09:39  steve
 *  Mingw32 support (Venkat Iyer)
 */
#endif
