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
#ident "$Id: t-dll.h,v 1.40 2001/04/26 05:12:02 steve Exp $"
#endif

# include  "target.h"
# include  "ivl_target.h"

#if defined(HAVE_DLFCN_H)
# include  <dlfcn.h>
typedef void* ivl_dll_t;
#elif defined(HAVE_DL_H)
# include  <dl.h>
typedef shl_t ivl_dll_t;
#endif

struct ivl_design_s {

      ivl_scope_t root_;

      ivl_process_t threads_;

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
      void udp(const NetUDP*);
      void lpm_ff(const NetFF*);
      void lpm_mux(const NetMux*);
      void net_assign(const NetAssign_*);
      bool net_const(const NetConst*);
      void net_probe(const NetEvProbe*);

      bool process(const NetProcTop*);
      void scope(const NetScope*);
      void signal(const NetNet*);

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
      void proc_condit(const NetCondit*);
      bool proc_delay(const NetPDelay*);
      bool proc_disable(const NetDisable*);
      void proc_forever(const NetForever*);
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
      void expr_const(const NetEConst*);
      void expr_scope(const NetEScope*);
      void expr_sfunc(const NetESFunc*);
      void expr_ufunc(const NetEUFunc*);
      void expr_unary(const NetEUnary*);
      void expr_signal(const NetESignal*);

      ivl_scope_t lookup_scope_(const NetScope*scope);
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
		  char*name_;
	    } sfunc_;

	    struct {
		  char*value_;
	    } string_;

	    struct {
		  char*name_;
		  ivl_expr_t msb_;
		  ivl_expr_t lsb_;
	    } subsig_;

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
		  ivl_nexus_t clk;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } q;
		  union {
			ivl_nexus_t*pins;
			ivl_nexus_t pin;
		  } d;
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

      } u_;
};

/*
 * This object contains references to ivl_nexus_t objects that in turn
 * are reg nets. This is used by the assignment to represent the
 * l-value expressions.
 */
struct ivl_lval_s {
      unsigned width_  :24;
      ivl_expr_t mux;
      union {
	    ivl_nexus_t*pins_;
	    ivl_nexus_t pin_;
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
 */
struct ivl_nexus_ptr_s {
      unsigned pin_  :24;
      unsigned type_ : 8;
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
      ivl_scope_t child_, sibling_;

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

	/* Scopes that are tasks/functions have a definition. */
      ivl_statement_t def;

      unsigned ports;
      char **port;
};

/*
 * A signal is a think like a wire, a reg, or whatever. It has a type,
 * and if it is a port is also has a directory. Signals are collected
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
		  int expr_; /* XXXX */
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
 * Revision 1.40  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.39  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.38  2001/04/15 02:58:11  steve
 *  vvp support for <= with internal delay.
 *
 * Revision 1.37  2001/04/07 19:26:32  steve
 *  Add the disable statemnent.
 *
 * Revision 1.36  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.35  2001/04/05 03:20:58  steve
 *  Generate vvp code for the repeat statement.
 *
 * Revision 1.34  2001/04/04 04:50:35  steve
 *  Support forever loops in the tgt-vvp target.
 *
 * Revision 1.33  2001/04/03 04:50:37  steve
 *  Support non-blocking assignments.
 *
 * Revision 1.32  2001/04/02 02:28:12  steve
 *  Generate code for task calls.
 *
 * Revision 1.31  2001/04/02 00:28:35  steve
 *  Support the scope expression node.
 *
 * Revision 1.30  2001/04/01 06:52:28  steve
 *  support the NetWhile statement.
 *
 * Revision 1.29  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.28  2001/03/31 17:36:39  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.27  2001/03/30 05:49:52  steve
 *  Generate code for fork/join statements.
 *
 * Revision 1.26  2001/03/29 02:52:39  steve
 *  Add unary ~ operator to tgt-vvp.
 *
 * Revision 1.25  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.24  2001/03/27 06:27:40  steve
 *  Generate code for simple @ statements.
 *
 * Revision 1.23  2001/03/27 03:31:06  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.22  2001/03/20 01:44:14  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.21  2001/01/15 00:47:02  steve
 *  Pass scope type information to the target module.
 *
 * Revision 1.20  2000/12/15 05:45:25  steve
 *  Autoconfigure the dlopen functions.
 *
 * Revision 1.19  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.18  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.17  2000/10/28 22:32:34  steve
 *  API for concatenation expressions.
 *
 * Revision 1.16  2000/10/28 17:55:03  steve
 *  stub for the concat operator.
 *
 * Revision 1.15  2000/10/21 16:49:45  steve
 *  Reduce the target entry points to the target_design.
 *
 * Revision 1.14  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.13  2000/10/15 04:46:23  steve
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
 * Revision 1.12  2000/10/13 03:39:27  steve
 *  Include constants in nexus targets.
 *
 * Revision 1.11  2000/10/08 04:01:55  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
 * Revision 1.10  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.9  2000/10/06 23:46:51  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.8  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 *
 * Revision 1.7  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.6  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.5  2000/09/24 02:21:53  steve
 *  Add support for signal expressions.
 *
 * Revision 1.4  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.3  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.2  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.1  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
 */
#endif
