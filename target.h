#ifndef __target_H
#define __target_H
/*
 * Copyright (c) 1998-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: target.h,v 1.65 2004/05/31 23:34:39 steve Exp $"
#endif

# include  "netlist.h"

/*
 * This header file describes the types and constants used to describe
 * the possible target output types of the compiler. The backends
 * provide one of these in order to tell the previous steps what the
 * backend is able to do.
 */

/*
 * The backend driver is hooked into the compiler, and given a name,
 * by creating an instance of the target structure. The structure has
 * the name that the compiler will use to locate the driver, and a
 * pointer to a target_t object that is the actual driver.
 */
struct target {
      const char* name;
      struct target_t* meth;
};

/*
 * The emit process uses a target_t driver to send the completed
 * design to a file. It is up to the driver object to follow along in
 * the iteration through the design, generating output as it can.
 */

struct target_t {
      virtual ~target_t();

	/* Start the design. This sets the main output file stream
	   that the target should use. */
      virtual bool start_design(const Design*) =0;

	/* This is called once for each scope in the design, before
	   anything else is called. */
      virtual void scope(const NetScope*);

	/* Output an event object. Called for each named event in the scope. */
      virtual void event(const NetEvent*);

	/* Output a signal (called for each signal) */
      virtual void signal(const NetNet*) =0;

	/* Output a memory (called for each memory object) */
      virtual void memory(const NetMemory*);

	/* Output an event object. Called for each named event in the scope. */
      virtual void variable(const NetVariable*);

	/* Output a defined task. */
      virtual void task_def(const NetScope*);
      virtual bool func_def(const NetScope*);

	/* LPM style components are handled here. */
      virtual void lpm_add_sub(const NetAddSub*);
      virtual void lpm_clshift(const NetCLShift*);
      virtual void lpm_compare(const NetCompare*);
      virtual void lpm_divide(const NetDivide*);
      virtual void lpm_modulo(const NetModulo*);
      virtual void lpm_ff(const NetFF*);
      virtual void lpm_mult(const NetMult*);
      virtual void lpm_mux(const NetMux*);
      virtual void lpm_ram_dq(const NetRamDq*);

	/* Output a gate (called for each gate) */
      virtual void logic(const NetLogic*);
      virtual bool bufz(const NetBUFZ*);
      virtual void udp(const NetUDP*);
      virtual void net_case_cmp(const NetCaseCmp*);
      virtual bool net_cassign(const NetCAssign*);
      virtual bool net_const(const NetConst*);
      virtual bool net_force(const NetForce*);
      virtual bool net_function(const NetUserFunc*);
      virtual void net_probe(const NetEvProbe*);

	/* Output a process (called for each process). It is up to the
	   target to recurse if desired. */
      virtual bool process(const NetProcTop*);

	/* Various kinds of process nodes are dispatched through these. */
      virtual void proc_assign(const NetAssign*);
      virtual void proc_assign_nb(const NetAssignNB*);
      virtual bool proc_block(const NetBlock*);
      virtual void proc_case(const NetCase*);
      virtual bool proc_cassign(const NetCAssign*);
      virtual bool proc_condit(const NetCondit*);
      virtual bool proc_deassign(const NetDeassign*);
      virtual bool proc_delay(const NetPDelay*);
      virtual bool proc_disable(const NetDisable*);
      virtual bool proc_force(const NetForce*);
      virtual void proc_forever(const NetForever*);
      virtual bool proc_release(const NetRelease*);
      virtual void proc_repeat(const NetRepeat*);
      virtual bool proc_trigger(const NetEvTrig*);
      virtual void proc_stask(const NetSTask*);
      virtual void proc_utask(const NetUTask*);
      virtual bool proc_wait(const NetEvWait*);
      virtual void proc_while(const NetWhile*);

	/* Done with the design. The target returns !0 if there is
	   some error in the code generation. */
      virtual int end_design(const Design*);
};

/* This class is used by the NetExpr class to help with the scanning
   of expressions. */
struct expr_scan_t {
      virtual ~expr_scan_t();
      virtual void expr_const(const NetEConst*);
      virtual void expr_param(const NetEConstParam*);
      virtual void expr_rparam(const NetECRealParam*);
      virtual void expr_creal(const NetECReal*);
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_event(const NetEEvent*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_select(const NetESelect*);
      virtual void expr_sfunc(const NetESFunc*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetEBitSel*);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_ufunc(const NetEUFunc*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_variable(const NetEVariable*);
      virtual void expr_binary(const NetEBinary*);
};


/* The emit functions take a design and emit it to the output stream
   using the specified target. If the target is given by name, it is
   located in the target_table and used. */
extern bool emit(const Design*des, const char*type);

/* This function takes a fully qualified verilog name (which may have,
   for example, dots in it) and produces a mangled version that can be
   used by most any language. */
extern string mangle(const string&str);

/* This function takes a string and produces an escaped version that can be
   used inside a string constant for a C++ compiler. */
extern string stresc(const string&str);

/* This is the table of supported output targets. It is a null
   terminated array of pointers to targets. */
extern const struct target *target_table[];

/*
 * $Log: target.h,v $
 * Revision 1.65  2004/05/31 23:34:39  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.64  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.63  2003/04/22 04:48:30  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.62  2003/03/10 23:40:54  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.61  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.60  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.59  2002/06/05 03:44:25  steve
 *  Add support for memory words in l-value of
 *  non-blocking assignments, and remove the special
 *  NetAssignMem_ and NetAssignMemNB classes.
 *
 * Revision 1.58  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.57  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.56  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.55  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.54  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.53  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.52  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 */
#endif
