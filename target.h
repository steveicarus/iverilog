#ifndef __target_H
#define __target_H
/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: target.h,v 1.27 1999/11/28 23:42:03 steve Exp $"
#endif

# include  "netlist.h"
class ostream;

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
      string name;
      struct target_t* meth;
};

/*
 * The emit process uses a target_t driver to send the completed
 * design to a file. It is up to the driver object to follow along in
 * the iteration through the design, generating output as it can.
 */

struct target_t {
      virtual ~target_t();

	/* Start the design. */
      virtual void start_design(ostream&os, const Design*);

	/* This is called once for each scope in the design, before
	   anything else is called. */
      virtual void scope(ostream&os, const NetScope*);

	/* Output a signal (called for each signal) */
      virtual void signal(ostream&os, const NetNet*);

	/* Output a memory (called for each memory object) */
      virtual void memory(ostream&os, const NetMemory*);

	/* Output a defined task. */
      virtual void task_def(ostream&, const NetTaskDef*);
      virtual void func_def(ostream&, const NetFuncDef*);

	/* LPM style components are handled here. */
      virtual void lpm_add_sub(ostream&os, const NetAddSub*);
      virtual void lpm_clshift(ostream&os, const NetCLShift*);
      virtual void lpm_compare(ostream&os, const NetCompare*);
      virtual void lpm_ff(ostream&os, const NetFF*);
      virtual void lpm_mux(ostream&os, const NetMux*);
      virtual void lpm_ram_dq(ostream&os, const NetRamDq*);

	/* Output a gate (called for each gate) */
      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os,  const NetUDP*);
      virtual void net_assign(ostream&os, const NetAssign*);
      virtual void net_assign_nb(ostream&os, const NetAssignNB*);
      virtual void net_case_cmp(ostream&os, const NetCaseCmp*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual void net_event(ostream&os, const NetNEvent*);

	/* Output a process (called for each process). It is up to the
	   target to recurse if desired. */
      virtual bool process(ostream&os, const NetProcTop*);

	/* Various kinds of process nodes are dispatched through these. */
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_assign_mem(ostream&os, const NetAssignMem*);
      virtual void proc_assign_nb(ostream&os, const NetAssignNB*);
      virtual void proc_assign_mem_nb(ostream&os, const NetAssignMemNB*);
      virtual bool proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os,  const NetCase*);
      virtual void proc_condit(ostream&os, const NetCondit*);
      virtual void proc_forever(ostream&os, const NetForever*);
      virtual void proc_repeat(ostream&os, const NetRepeat*);
      virtual void proc_stask(ostream&os, const NetSTask*);
      virtual void proc_utask(ostream&os, const NetUTask*);
      virtual void proc_while(ostream&os, const NetWhile*);

      virtual void proc_event(ostream&os, const NetPEvent*);
      virtual void proc_delay(ostream&os, const NetPDelay*);

	/* Done with the design. */
      virtual void end_design(ostream&os, const Design*);
};

/* This class is used by the NetExpr class to help with the scanning
   of expressions. */
struct expr_scan_t {
      virtual ~expr_scan_t();
      virtual void expr_const(const NetEConst*);
      virtual void expr_concat(const NetEConcat*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_scope(const NetEScope*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetESubSignal*);
      virtual void expr_ternary(const NetETernary*);
      virtual void expr_ufunc(const NetEUFunc*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
};


/* The emit functions take a design and emit it to the output stream
   using the specified target. If the target is given by name, it is
   located in the target_table and used. */
extern bool emit(ostream&o, const Design*des, const char*type);

/* This function takes a fully qualified verilog name (which may have,
   for example, dots in it) and produces a mangled version that can be
   used by most any language. */
extern string mangle(const string&str);

/* This is the table of supported output targets. It is a null
   terminated array of pointers to targets. */
extern const struct target *target_table[];

/*
 * $Log: target.h,v $
 * Revision 1.27  1999/11/28 23:42:03  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.26  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 * Revision 1.25  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.24  1999/11/14 23:43:46  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.23  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.22  1999/11/04 03:53:26  steve
 *  Patch to synthesize unary ~ and the ternary operator.
 *  Thanks to Larry Doolittle <LRDoolittle@lbl.gov>.
 *
 *  Add the LPM_MUX device, and integrate it with the
 *  ternary synthesis from Larry. Replace the lpm_mux
 *  generator in t-xnf.cc to use XNF EQU devices to
 *  put muxs into function units.
 *
 *  Rewrite elaborate_net for the PETernary class to
 *  also use the LPM_MUX device.
 *
 * Revision 1.21  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.20  1999/10/10 01:59:55  steve
 *  Structural case equals device.
 *
 * Revision 1.19  1999/09/22 16:57:24  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.18  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.17  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.16  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.15  1999/07/17 19:51:00  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.14  1999/07/17 03:39:11  steve
 *  simplified process scan for targets.
 *
 * Revision 1.13  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.12  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.11  1999/06/09 03:00:06  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.10  1999/06/06 20:45:39  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.9  1999/05/12 04:03:20  steve
 *  emit NetAssignMem objects in vvm target.
 *
 * Revision 1.8  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.7  1999/04/25 00:44:10  steve
 *  Core handles subsignal expressions.
 *
 * Revision 1.6  1999/04/19 01:59:37  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.5  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.4  1998/12/01 00:42:15  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.3  1998/11/09 18:55:35  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:05:06  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:29:06  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
