/*
 * Copyright (c) 1998-2000 Stephen Williams <steve@icarus.com>
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
#ident "$Id: target.cc,v 1.32 2000/04/01 21:40:23 steve Exp $"
#endif

# include  "target.h"
# include  <typeinfo>

target_t::~target_t()
{
}

void target_t::start_design(ostream&os, const Design*)
{
}

void target_t::scope(ostream&, const NetScope*)
{
}

void target_t::signal(ostream&os, const NetNet*)
{
}

void target_t::memory(ostream&os, const NetMemory*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled memory." << endl;
}

void target_t::func_def(ostream&os, const NetFuncDef*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled function definition." << endl;
}

void target_t::task_def(ostream&os, const NetTaskDef*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled task definition." << endl;
}

void target_t::logic(ostream&os, const NetLogic*)
{
}

void target_t::bufz(ostream&os, const NetBUFZ*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled continuous assign (BUFZ)." << endl;
}

void target_t::udp(ostream&os, const NetUDP*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled UDP." << endl;
}

void target_t::udp_comb(ostream&os, const NetUDP_COMB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled combinational primitive." << endl;
}

void target_t::lpm_add_sub(ostream&, const NetAddSub*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetAddSub." << endl;
}

void target_t::lpm_clshift(ostream&, const NetCLShift*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCLShift." << endl;
}

void target_t::lpm_compare(ostream&, const NetCompare*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCompare." << endl;
}

void target_t::lpm_divide(ostream&, const NetDivide*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetDivide." << endl;
}

void target_t::lpm_ff(ostream&, const NetFF*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetFF." << endl;
}

void target_t::lpm_mult(ostream&, const NetMult*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetMult." << endl;
}

void target_t::lpm_mux(ostream&, const NetMux*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetMux." << endl;
}

void target_t::lpm_ram_dq(ostream&, const NetRamDq*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetRamDq." << endl;
}

void target_t::net_assign(ostream&os, const NetAssign*)
{
}

void target_t::net_assign_nb(ostream&os, const NetAssignNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking assignment node." << endl;
}

void target_t::net_case_cmp(ostream&os, const NetCaseCmp*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case compare node." << endl;
}

void target_t::net_const(ostream&os, const NetConst*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled CONSTANT node." << endl;
}

void target_t::net_event(ostream&os, const NetNEvent*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled EVENT net node." << endl;
      net->dump_node(cerr, 4);
}

bool target_t::process(ostream&os, const NetProcTop*top)
{
      return top->statement()->emit_proc(os, this);
}

void target_t::proc_assign(ostream&os, const NetAssign*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled procedural assignment." << endl;
}

void target_t::proc_assign_mem(ostream&os, const NetAssignMem*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled memory assignment." << endl;
}

void target_t::proc_assign_nb(ostream&os, const NetAssignNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking assignment." << endl;
}

void target_t::proc_assign_mem_nb(ostream&os, const NetAssignMemNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking memory assignment." << endl;
}

bool target_t::proc_block(ostream&os, const NetBlock*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_block." << endl;
      return false;
}

void target_t::proc_case(ostream&os, const NetCase*cur)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case:" << endl;
      cur->dump(cerr, 6);
}

void target_t::proc_condit(ostream&os, const NetCondit*condit)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled conditional:" << endl;
      condit->dump(cerr, 6);
}

void target_t::proc_delay(ostream&os, const NetPDelay*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_delay." << endl;
}

void target_t::proc_event(ostream&os, const NetPEvent*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_event." << endl;
      net->dump(cerr, 4);
}

void target_t::proc_forever(ostream&os, const NetForever*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_forever." << endl;
}

void target_t::proc_repeat(ostream&os, const NetRepeat*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_repeat." << endl;
}

void target_t::proc_stask(ostream&os, const NetSTask*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_stask." << endl;
}

void target_t::proc_utask(ostream&os, const NetUTask*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_utask." << endl;
}

void target_t::proc_while(ostream&os, const NetWhile*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled while:" << endl;
      net->dump(cerr, 6);
}

void target_t::end_design(ostream&os, const Design*)
{
}

expr_scan_t::~expr_scan_t()
{
}

void expr_scan_t::expr_const(const NetEConst*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_const." << endl;
}

void expr_scan_t::expr_concat(const NetEConcat*that)
{
      cerr << that->get_line() << ": expr_scan_t (" <<
	    typeid(*this).name() << "): unhandled expr_concat." << endl;
}

void expr_scan_t::expr_ident(const NetEIdent*that)
{
      cerr << that->get_line() << ": expr_scan_t (" <<
	    typeid(*this).name() << "): unhandled expr_ident." << endl;
}

void expr_scan_t::expr_memory(const NetEMemory*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_memory." << endl;
}

void expr_scan_t::expr_scope(const NetEScope*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_scope." << endl;
}

void expr_scan_t::expr_signal(const NetESignal*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_signal." << endl;
}

void expr_scan_t::expr_subsignal(const NetESubSignal*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_subsignal." << endl;
}

void expr_scan_t::expr_ternary(const NetETernary*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_ternary." << endl;
}

void expr_scan_t::expr_ufunc(const NetEUFunc*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled function call." << endl;
}

void expr_scan_t::expr_unary(const NetEUnary*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_unary." << endl;
}

void expr_scan_t::expr_binary(const NetEBinary*ex)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_binary: " <<*ex  << endl;
}

/*
 * $Log: target.cc,v $
 * Revision 1.32  2000/04/01 21:40:23  steve
 *  Add support for integer division.
 *
 * Revision 1.31  2000/03/29 04:37:11  steve
 *  New and improved combinational primitives.
 *
 * Revision 1.30  2000/02/23 02:56:56  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.29  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.28  1999/11/28 23:42:03  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.27  1999/11/27 19:07:58  steve
 *  Support the creation of scopes.
 *
 * Revision 1.26  1999/11/21 00:13:09  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.25  1999/11/14 23:43:45  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.24  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.23  1999/11/04 03:53:26  steve
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
 * Revision 1.22  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.21  1999/10/10 01:59:55  steve
 *  Structural case equals device.
 *
 * Revision 1.20  1999/09/22 16:57:24  steve
 *  Catch parallel blocks in vvm emit.
 *
 * Revision 1.19  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.18  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.17  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.16  1999/08/18 04:00:02  steve
 *  Fixup spelling and some error messages. <LRDoolittle@lbl.gov>
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
 * Revision 1.9  1999/05/12 04:03:19  steve
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
 * Revision 1.3  1998/11/09 18:55:34  steve
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

