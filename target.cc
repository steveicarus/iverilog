/*
 * Copyright (c) 1998-2021 Stephen Williams <steve@icarus.com>
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

# include  <iostream>

# include  "target.h"
# include  <typeinfo>

using namespace std;

target_t::~target_t()
{
}

void target_t::scope(const NetScope*)
{
}

void target_t::convert_module_ports(const NetScope*)
{
}

bool target_t::branch(const NetBranch*obj)
{
      cerr << obj->get_fileline() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled branch." << endl;
      return false;
}

bool target_t::class_type(const NetScope*, netclass_t*obj)
{
      cerr << "<>:0" << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled class_type <" << obj << ">." << endl;
      return false;
}

void target_t::event(const NetEvent*ev)
{
      cerr << ev->get_fileline() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled event <" << ev->name() << ">." << endl;
}

bool target_t::enumeration(const NetScope*, netenum_t*obj)
{
      cerr << "<>:0" << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled enumeration <" << obj << ">." << endl;
      return false;
}

bool target_t::signal_paths(const NetNet*)
{
      return true;
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
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled logic gate" << endl;
}

bool target_t::tran(const NetTran*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	   << "TRAN devices not supported." << endl;
      return false;
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

bool target_t::ureduce(const NetUReduce*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled unary reduction logic gate." << endl;
      return false;
}

void target_t::lpm_abs(const NetAbs*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetAbs." << endl;
}

void target_t::lpm_add_sub(const NetAddSub*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetAddSub." << endl;
}

bool target_t::lpm_array_dq(const NetArrayDq*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetArrayDq." << endl;
      return false;
}

bool target_t::lpm_cast_int2(const NetCastInt2*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCastInt2." << endl;
      return false;
}

bool target_t::lpm_cast_int4(const NetCastInt4*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCastInt4." << endl;
      return false;
}

bool target_t::lpm_cast_real(const NetCastReal*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetCastReal." << endl;
      return false;
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

void target_t::lpm_latch(const NetLatch*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetLatch." << endl;
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

void target_t::lpm_pow(const NetPow*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetPow." << endl;
}

bool target_t::concat(const NetConcat*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetConcat." << endl;
      return false;
}

bool target_t::part_select(const NetPartSelect*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetPartSelect." << endl;
      return false;
}

bool target_t::replicate(const NetReplicate*)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled NetReplicate." << endl;
      return false;
}

void target_t::net_case_cmp(const NetCaseCmp*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled case compare node." << endl;
}

bool target_t::net_const(const NetConst*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled CONSTANT node." << endl;
      return false;
}

bool target_t::net_sysfunction(const NetSysFunc*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetSysFunc node." << endl;
      return false;
}

bool target_t::net_function(const NetUserFunc*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetUserFunc node." << endl;
      return false;
}

bool target_t::net_literal(const NetLiteral*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled LITERAL node." << endl;
      return false;
}

void target_t::net_probe(const NetEvProbe*net)
{
      cerr << "target (" << typeid(*this).name() << "): "
	    "Unhandled probe trigger node" << endl;
      net->dump_node(cerr, 4);
}

bool target_t::sign_extend(const NetSignExtend*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetSignExtend node." << endl;
      return false;
}

bool target_t::substitute(const NetSubstitute*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled NetSubstitute node." << endl;
      return false;
}

bool target_t::process(const NetProcTop*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled process(NetProcTop)." << endl;
      return false;
}

bool target_t::process(const NetAnalogTop*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled process(NetAnalogTop)." << endl;
      return false;
}

void target_t::proc_alloc(const NetAlloc*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_alloc." << endl;
}

bool target_t::proc_assign(const NetAssign*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled procedural assignment." << endl;
      return false;
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

bool target_t::proc_break(const NetBreak*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_break." << endl;
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
	cerr << dev->get_fileline();
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

bool target_t::proc_continue(const NetContinue*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_continue." << endl;
      return false;
}

bool target_t::proc_contribution(const NetContribution*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled contribution:" << endl;
      net->dump(cerr, 6);
      return false;
}

bool target_t::proc_deassign(const NetDeassign*dev)
{
      cerr << dev->get_fileline() << ": internal error: "
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
      cerr << obj->get_fileline() << ": internal error: "
	   << "target (" << typeid(*this).name() << "): "
	   << "does not support disable statements." << endl;
      return false;
}

void target_t::proc_do_while(const NetDoWhile*net)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled do/while:" << endl;
      net->dump(cerr, 6);
}

bool target_t::proc_force(const NetForce*)
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

bool target_t::proc_forloop(const NetForLoop*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_forloop." << endl;
      return false;
}

void target_t::proc_free(const NetFree*)
{
      cerr << "target (" << typeid(*this).name() <<  "): "
	    "Unhandled proc_free." << endl;
}

bool target_t::proc_release(const NetRelease*dev)
{
      cerr << dev->get_fileline() << ": internal error: "
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
      cerr << tr->get_fileline() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled event trigger." << endl;
      return false;
}

bool target_t::proc_nb_trigger(const NetEvNBTrig*tr)
{
      cerr << tr->get_fileline() << ": error: target (" << typeid(*this).name()
	   <<  "): Unhandled non-blocking event trigger." << endl;
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
      cerr << tr->get_fileline() << ": error: target (" << typeid(*this).name()
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

void expr_scan_t::expr_access_func(const NetEAccess*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_access_func." << endl;
}

void expr_scan_t::expr_array_pattern(const NetEArrayPattern*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_array_pattern." << endl;
}

void expr_scan_t::expr_const(const NetEConst*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_const." << endl;
}

void expr_scan_t::expr_last(const NetELast*exp)
{
      cerr << exp->get_fileline() << ": expr_scan_t(" << typeid(*this).name() << "): "
	   << "unhandled expr_last." << endl;
}

void expr_scan_t::expr_new(const NetENew*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_new." << endl;
}

void expr_scan_t::expr_null(const NetENull*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_null." << endl;
}

void expr_scan_t::expr_param(const NetEConstParam*that)
{
      expr_const(that);
}

void expr_scan_t::expr_property(const NetEProperty*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_property." << endl;
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
      cerr << that->get_fileline() << ": expr_scan_t (" <<
	    typeid(*this).name() << "): unhandled expr_concat." << endl;
}
void expr_scan_t::expr_event(const NetEEvent*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_event." << endl;
}

void expr_scan_t::expr_netenum(const NetENetenum*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_netenum." << endl;
}

void expr_scan_t::expr_scope(const NetEScope*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_scope." << endl;
}

void expr_scan_t::expr_scopy(const NetEShallowCopy*)
{
      cerr << "expr_scan_t (" << typeid(*this).name() << "): "
	    "unhandled expr_scopy." << endl;
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
