/*
 * Copyright (c) 1998 Stephen Williams <steve@icarus.com>
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
#ident "$Id: target.cc,v 1.3 1998/11/09 18:55:34 steve Exp $"
#endif

# include  "target.h"
# include  <typeinfo>

target_t::~target_t()
{
}

void target_t::start_design(ostream&os, const Design*)
{
}

void target_t::signal(ostream&os, const NetNet*)
{
}

void target_t::logic(ostream&os, const NetLogic*)
{
}

void target_t::bufz(ostream&os, const NetBUFZ*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled continuous assign (BUFZ)." << endl;
}

void target_t::net_assign(ostream&os, const NetAssign*)
{
}

void target_t::net_const(ostream&os, const NetConst*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled CONSTANT node." << endl;
}

void target_t::net_pevent(ostream&os, const NetPEvent*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled EVENT node." << endl;
}

void target_t::start_process(ostream&os, const NetProcTop*)
{
}

void target_t::proc_assign(ostream&os, const NetAssign*)
{
}

void target_t::proc_block(ostream&os, const NetBlock*)
{
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

void target_t::proc_event(ostream&os, const NetPEvent*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_event." << endl;
}

void target_t::proc_task(ostream&os, const NetTask*)
{
}

void target_t::proc_while(ostream&os, const NetWhile*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled while:" << endl;
      net->dump(cerr, 6);
}

void target_t::end_process(ostream&os, const NetProcTop*)
{
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

void expr_scan_t::expr_ident(const NetEIdent*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_ident." << endl;
}

void expr_scan_t::expr_signal(const NetESignal*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_signal." << endl;
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

