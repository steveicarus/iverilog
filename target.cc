/*
 * Copyright (c) 1998-2003 Stephen Williams <steve@icarus.com>
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
#ident "$Id: target.cc,v 1.69 2004/05/31 23:34:39 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

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

void target_t::memory(const NetMemory*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled memory." << endl;
}

void target_t::variable(const NetVariable*that)
{
      cerr << that->get_line() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled variable <" << that->basename() << ">." << endl;
}

bool target_t::func_def(const NetScope*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled function definition." << endl;
      return false;
}

void target_t::task_def(const NetScope*)
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

void target_t::lpm_modulo(const NetModulo*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetModulo." << endl;
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

void target_t::net_case_cmp(const NetCaseCmp*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case compare node." << endl;
}

bool target_t::net_cassign(const NetCAssign*dev)
{
	cerr << "target (" << typeid(*this).name() <<  "): ";
	cerr << dev->get_line();
	cerr << ": Target does not support procedural continuous assignment." << endl;
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

bool target_t::net_function(const NetUserFunc*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetUserFunc node." << endl;
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

void target_t::proc_assign_nb(const NetAssignNB*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled non-blocking assignment." << endl;
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
	cerr << "target (" << typeid(*this).name() <<  "): ";
	cerr << dev->get_line();
	cerr << ": Target does not support procedural continuous assignment." << endl;
      return false;
}

bool target_t::proc_condit(const NetCondit*condit)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled conditional:" << endl;
      condit->dump(cerr, 6);
      return false;
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

int target_t::end_design(const Design*)
{
      return 0;
}

expr_scan_t::~expr_scan_t()
{
}

void expr_scan_t::expr_const(const NetEConst*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_const." << endl;
}

void expr_scan_t::expr_param(const NetEConstParam*that)
{
      expr_const(that);
}

void expr_scan_t::expr_creal(const NetECReal*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_creal." << endl;
}

void expr_scan_t::expr_rparam(const NetECRealParam*that)
{
      expr_creal(that);
}

void expr_scan_t::expr_concat(const NetEConcat*that)
{
      cerr << that->get_line() << ": expr_scan_t (" <<
	    typeid(*this).name() << "): unhandled expr_concat." << endl;
}

void expr_scan_t::expr_memory(const NetEMemory*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_memory." << endl;
}

void expr_scan_t::expr_event(const NetEEvent*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_event." << endl;
}

void expr_scan_t::expr_scope(const NetEScope*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_scope." << endl;
}

void expr_scan_t::expr_select(const NetESelect*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_select." << endl;
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

void expr_scan_t::expr_subsignal(const NetEBitSel*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled bit select expression." << endl;
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

void expr_scan_t::expr_variable(const NetEVariable*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_variable." << endl;
}

void expr_scan_t::expr_binary(const NetEBinary*ex)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_binary: " <<*ex  << endl;
}

/*
 * $Log: target.cc,v $
 * Revision 1.69  2004/05/31 23:34:39  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.68  2003/05/30 02:55:32  steve
 *  Support parameters in real expressions and
 *  as real expressions, and fix multiply and
 *  divide with real results.
 *
 * Revision 1.67  2003/04/22 04:48:30  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.66  2003/03/10 23:40:54  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.65  2003/01/30 16:23:08  steve
 *  Spelling fixes.
 *
 * Revision 1.64  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.63  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.62  2002/06/05 03:44:25  steve
 *  Add support for memory words in l-value of
 *  non-blocking assignments, and remove the special
 *  NetAssignMem_ and NetAssignMemNB classes.
 *
 * Revision 1.61  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.60  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.59  2002/01/28 00:52:41  steve
 *  Add support for bit select of parameters.
 *  This leads to a NetESelect node and the
 *  vvp code generator to support that.
 *
 * Revision 1.58  2002/01/19 19:02:08  steve
 *  Pass back target errors processing conditionals.
 *
 * Revision 1.57  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.56  2001/07/27 04:51:44  steve
 *  Handle part select expressions as variants of
 *  NetESignal/IVL_EX_SIGNAL objects, instead of
 *  creating new and useless temporary signals.
 *
 * Revision 1.55  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.54  2001/06/27 18:34:43  steve
 *  Report line of unsupported cassign.
 *
 * Revision 1.53  2001/04/22 23:09:46  steve
 *  More UDP consolidation from Stephan Boettcher.
 *
 * Revision 1.52  2001/04/06 02:28:02  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.51  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.50  2001/03/27 03:31:06  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.49  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
 * Revision 1.48  2000/09/17 21:26:16  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.47  2000/09/03 17:57:53  steve
 *  Slightly more helpful warning.
 *
 * Revision 1.46  2000/09/02 20:54:21  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.45  2000/08/27 15:51:51  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
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
 */

