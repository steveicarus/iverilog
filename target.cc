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
#ident "$Id: target.cc,v 1.44 2000/08/14 04:39:57 steve Exp $"
#endif

# include  "target.h"
# include  <typeinfo>

target_t::~target_t()
{
}

void target_t::scope(const NetScope*)
{
}

void target_t::event(const NetEvent*ev)
{
      cerr << ev->get_line() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled event <" << ev->full_name() << ">." << endl;
}

void target_t::signal(const NetNet*)
{
}

void target_t::memory(const NetMemory*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled memory." << endl;
}

void target_t::func_def(const NetFuncDef*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled function definition." << endl;
}

void target_t::task_def(const NetTaskDef*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled task definition." << endl;
}

void target_t::logic(const NetLogic*)
{
}

bool target_t::bufz(const NetBUFZ*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled continuous assign (BUFZ)." << endl;
      return false;
}

void target_t::udp(const NetUDP*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled UDP." << endl;
}

void target_t::udp_comb(const NetUDP_COMB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled combinational primitive." << endl;
}

void target_t::lpm_add_sub(const NetAddSub*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetAddSub." << endl;
}

void target_t::lpm_clshift(const NetCLShift*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCLShift." << endl;
}

void target_t::lpm_compare(const NetCompare*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCompare." << endl;
}

void target_t::lpm_divide(const NetDivide*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetDivide." << endl;
}

void target_t::lpm_ff(const NetFF*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetFF." << endl;
}

void target_t::lpm_mult(const NetMult*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetMult." << endl;
}

void target_t::lpm_mux(const NetMux*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetMux." << endl;
}

void target_t::lpm_ram_dq(const NetRamDq*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetRamDq." << endl;
}

void target_t::net_assign(const NetAssign*)
{
}

void target_t::net_assign_nb(const NetAssignNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking assignment node." << endl;
}

void target_t::net_case_cmp(const NetCaseCmp*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case compare node." << endl;
}

bool target_t::net_cassign(const NetCAssign*dev)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetCAssign node." << endl;
      return false;
}

bool target_t::net_const(const NetConst*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled CONSTANT node." << endl;
      return false;
}

bool target_t::net_force(const NetForce*dev)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetForce node." << endl;
      return false;
}

void target_t::net_probe(const NetEvProbe*net)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled probe trigger node" << endl;
      net->dump_node(cerr, 4);
}

bool target_t::process(const NetProcTop*top)
{
      return top->statement()->emit_proc(this);
}

void target_t::proc_assign(const NetAssign*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled procedural assignment." << endl;
}

void target_t::proc_assign_mem(const NetAssignMem*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled memory assignment." << endl;
}

void target_t::proc_assign_nb(const NetAssignNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking assignment." << endl;
}

void target_t::proc_assign_mem_nb(const NetAssignMemNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking memory assignment." << endl;
}

bool target_t::proc_block(const NetBlock*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_block." << endl;
      return false;
}

void target_t::proc_case(const NetCase*cur)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case:" << endl;
      cur->dump(cerr, 6);
}

bool target_t::proc_cassign(const NetCAssign*dev)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_cassign." << endl;
      return false;
}

void target_t::proc_condit(const NetCondit*condit)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled conditional:" << endl;
      condit->dump(cerr, 6);
}

bool target_t::proc_deassign(const NetDeassign*dev)
{
      cerr << dev->get_line() << ": internal error: "
	   << "target (" << typeid(*this).name() <<  "): "
	   << "Unhandled proc_deassign." << endl;
      return false;
}

bool target_t::proc_delay(const NetPDelay*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_delay." << endl;
      return false;
}

bool target_t::proc_disable(const NetDisable*obj)
{
      cerr << obj->get_line() << ": internal error: "
	   << "target (" << typeid(*this).name() << "): "
	   << "does not support disable statements." << endl;
      return false;
}

bool target_t::proc_force(const NetForce*dev)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_force." << endl;
      return false;
}

void target_t::proc_forever(const NetForever*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_forever." << endl;
}

bool target_t::proc_release(const NetRelease*dev)
{
      cerr << dev->get_line() << ": internal error: "
	   << "target (" << typeid(*this).name() <<  "): "
	   << "Unhandled proc_release." << endl;
      return false;
}

void target_t::proc_repeat(const NetRepeat*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_repeat." << endl;
}

bool target_t::proc_trigger(const NetEvTrig*tr)
{
      cerr << tr->get_line() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled event trigger." << endl;
      return false;
}

void target_t::proc_stask(const NetSTask*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_stask." << endl;
}

void target_t::proc_utask(const NetUTask*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_utask." << endl;
}

bool target_t::proc_wait(const NetEvWait*tr)
{
      cerr << tr->get_line() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled event wait." << endl;
      return false;
}

void target_t::proc_while(const NetWhile*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled while:" << endl;
      net->dump(cerr, 6);
}

void target_t::end_design(const Design*)
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

void expr_scan_t::expr_sfunc(const NetESFunc*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_sfunc." << endl;
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
 * Revision 1.44  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.43  2000/08/09 03:43:45  steve
 *  Move all file manipulation out of target class.
 *
 * Revision 1.42  2000/08/08 01:50:42  steve
 *  target methods need not take a file stream.
 *
 * Revision 1.41  2000/07/29 16:21:08  steve
 *  Report code generation errors through proc_delay.
 *
 * Revision 1.40  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.39  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.38  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.37  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.36  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.35  2000/04/12 04:23:58  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 *
 * Revision 1.34  2000/04/10 05:26:06  steve
 *  All events now use the NetEvent class.
 *
 * Revision 1.33  2000/04/04 03:20:15  steve
 *  Simulate named event trigger and waits.
 *
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

