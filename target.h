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
#ident "$Id: target.h,v 1.9 1999/05/12 04:03:20 steve Exp $"
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

	/* Output a signal (called for each signal) */
      virtual void signal(ostream&os, const NetNet*);

	/* Output a memory (called for each memory object) */
      virtual void memory(ostream&os, const NetMemory*);

	/* Output a gate (called for each gate) */
      virtual void logic(ostream&os, const NetLogic*);
      virtual void bufz(ostream&os, const NetBUFZ*);
      virtual void udp(ostream&os,  const NetUDP*);
      virtual void net_assign(ostream&os, const NetAssign*);
      virtual void net_const(ostream&os, const NetConst*);
      virtual void net_esignal(ostream&os, const NetESignal*);
      virtual void net_event(ostream&os, const NetNEvent*);

	/* Output a process (called for each process) */
      virtual void start_process(ostream&os, const NetProcTop*);

	/* Various kinds of process nodes are dispatched through these. */
      virtual void proc_assign(ostream&os, const NetAssign*);
      virtual void proc_assign_mem(ostream&os, const NetAssignMem*);
      virtual void proc_block(ostream&os, const NetBlock*);
      virtual void proc_case(ostream&os,  const NetCase*);
      virtual void proc_condit(ostream&os, const NetCondit*);
      virtual void proc_task(ostream&os, const NetTask*);
      virtual void proc_while(ostream&os, const NetWhile*);

      virtual void proc_event(ostream&os, const NetPEvent*);
      virtual void proc_delay(ostream&os, const NetPDelay*);

	/* (called for each process) */
      virtual void end_process(ostream&os, const NetProcTop*);

	/* Done with the design. */
      virtual void end_design(ostream&os, const Design*);
};

/* This class is used by the NetExpr class to help with the scanning
   of expressions. */
struct expr_scan_t {
      virtual ~expr_scan_t();
      virtual void expr_const(const NetEConst*);
      virtual void expr_ident(const NetEIdent*);
      virtual void expr_memory(const NetEMemory*);
      virtual void expr_signal(const NetESignal*);
      virtual void expr_subsignal(const NetESubSignal*);
      virtual void expr_unary(const NetEUnary*);
      virtual void expr_binary(const NetEBinary*);
};


/* The emit functions take a design and emit it to the output stream
   using the specified target. If the target is given by name, it is
   located in the target_table and used. */
extern void emit(ostream&o, const Design*des, const char*type);

/* This function takes a fully qualified verilog name (which may have,
   for example, dots in it) and produces a mangled version that can be
   used by most any language. */
extern string mangle(const string&str);

/* This is the table of supported output targets. It is a null
   terminated array of pointers to targets. */
extern const struct target *target_table[];

/*
 * $Log: target.h,v $
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
