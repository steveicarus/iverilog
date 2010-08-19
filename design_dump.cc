/*
 * Copyright (c) 1998-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

/*
 * This file contains all the dump methods of the netlist classes.
 */
# include  <typeinfo>
# include  <iostream>
# include  <iomanip>
# include  "netlist.h"
# include  "compiler.h"
# include  "discipline.h"
# include  "ivl_assert.h"

static ostream& operator<< (ostream&o, NetBlock::Type t)
{
      switch (t) {
	  case NetBlock::SEQU:
	    o << "begin";
	    break;
	  case NetBlock::PARA:
	    o << "fork";
	    break;
      }
      return o;
}

ostream& operator << (ostream&o, Link::strength_t str)
{
      switch (str) {
	  case Link::HIGHZ:
	    o << "highz";
	    break;
	  case Link::WEAK:
	    o << "weak";
	    break;
	  case Link::PULL:
	    o << "pull";
	    break;
	  case Link::STRONG:
	    o << "strong";
	    break;
	  case Link::SUPPLY:
	    o << "supply";
	    break;
	  default:
	    assert(0);
      }
      return o;
}

ostream& operator << (ostream&o, ivl_variable_type_t val)
{
      switch (val) {
	  case IVL_VT_VOID:
	    o << "void";
	    break;
	  case IVL_VT_NO_TYPE:
	    o << "<no_type>";
	    break;
	  case IVL_VT_REAL:
	    o << "real";
	    break;
	  case IVL_VT_BOOL:
	    o << "bool";
	    break;
	  case IVL_VT_LOGIC:
	    o << "logic";
	    break;
      }
      return o;
}

ostream& operator << (ostream&o, ivl_switch_type_t val)
{
      switch (val) {
	  case IVL_SW_TRAN:
	    o << "tran";
	    break;
	  case IVL_SW_TRANIF0:
	    o << "tranif0";
	    break;
	  case IVL_SW_TRANIF1:
	    o << "tranif1";
	    break;
	  case IVL_SW_RTRAN:
	    o << "rtran";
	    break;
	  case IVL_SW_RTRANIF0:
	    o << "rtranif0";
	    break;
	  case IVL_SW_RTRANIF1:
	    o << "rtranif1";
	    break;
	  case IVL_SW_TRAN_VP:
	    o << "tran(VP)";
	    break;
      }
      return o;
}

static inline void dump_scope_path(ostream&o, const NetScope*scope)
{
      const NetScope*parent = scope->parent();
      if (parent) {
	    dump_scope_path(o, parent);
	    o << ".";
      }
      const hname_t name = scope->fullname();
      o << name.peek_name();
      if (name.has_number())
	    o << "[" << name.peek_number() << "]";
}

ostream& operator <<(ostream&o, struct __ScopePathManip marg)
{
      if (marg.scope != 0)
	    dump_scope_path(o, marg.scope);
      return o;
}

void NetBranch::dump(ostream&o, unsigned ind) const
{
      static const char*pin_names[2] = {
	    "terminal0",
	    "terminal1" };

      o << setw(ind) << "" << "branch island=" << get_island();
      o << " // " << get_fileline() << endl;
      dump_node_pins(o, ind+4, pin_names);
}

void NetDelaySrc::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "specify delay";
      if (posedge_) o << " posedge";
      if (negedge_) o << " negedge";
      if (is_condit()) {
	    if (has_condit()) o << " if";
	    else o << " ifnone";
      }
      o << " src "
	<< "(" << transition_delays_[IVL_PE_01]
	<< "," << transition_delays_[IVL_PE_10]
	<< "," << transition_delays_[IVL_PE_0z]
	<< "/" << transition_delays_[IVL_PE_z1]
	<< "," << transition_delays_[IVL_PE_1z]
	<< "," << transition_delays_[IVL_PE_z0]
	<< "/" << transition_delays_[IVL_PE_0x]
	<< "," << transition_delays_[IVL_PE_x1]
	<< "," << transition_delays_[IVL_PE_1x]
	<< "/" << transition_delays_[IVL_PE_x0]
	<< "," << transition_delays_[IVL_PE_xz]
	<< "," << transition_delays_[IVL_PE_zx]
	<< ") scope=" << scope_path(scope()) << endl;
      dump_node_pins(o, ind+4);
}

/* Dump a net. This can be a wire or register. */
void NetNet::dump_net(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type() << ": " << name()
	<< "[" << s0_ << ":" << e0_ << " count=" << pin_count() << "]";
      if (local_flag_)
	    o << " (local)";
      o << " " << data_type_;
      if (signed_)
	    o << " signed";
      switch (port_type_) {
	  case NetNet::NOT_A_PORT:
	    break;
	  case NetNet::PIMPLICIT:
	    o << " implicit-port?";
	    break;
	  case NetNet::PINPUT:
	    o << " input";
	    break;
	  case NetNet::POUTPUT:
	    o << " output";
	    break;
	  case NetNet::PINOUT:
	    o << " inout";
	    break;
      }

      if (ivl_discipline_t dis = get_discipline())
	    o << " discipline=" << dis->name();

      o << " (eref=" << peek_eref() << ", lref=" << peek_lref() << ")";
      if (scope())
	    o << " scope=" << scope_path(scope());
      o << " #(" << rise_time() << "," << fall_time() << ","
	<<  decay_time() << ") vector_width=" << vector_width()
	<< " pin_count=" << pin_count();
      if (pins_are_virtual()) {
	    o << " pins_are_virtual" << endl;
	    return;
      }
      o << " init=";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << pin(idx-1).get_init();

      o << " (";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    if (const Nexus*tmp = pin(idx-1).nexus())
		  o << tmp->get_init();
	    else
		  o << ".";
      o << ")" << endl;

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    if (! pin(idx).is_linked())
		  continue;

	    const Nexus*nex = pin(idx).nexus();
	    o << setw(ind+4) << "" << "[" << idx << "]: " << nex
	      << " " << nex->name() << endl;
      }

      for (unsigned idx = 0 ;  idx < delay_paths_.size() ;  idx += 1) {
	    const NetDelaySrc*cur = delay_paths_[idx];
	    cur->dump(o, ind+4);
      }

      dump_obj_attr(o, ind+4);
}

/* Dump a NetNode and its pins. Dump what I know about the netnode on
   the first line, then list all the pins, with the name of the
   connected signal. */
void NetNode::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "node: ";
      o << typeid(*this).name() << " #(" << rise_time()
	<< "," << fall_time() << "," << decay_time() << ") " << name()
	<< endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

/* This is the generic dumping of all the signals connected to each
   pin of the object. The "this" object is not printed, only the
   signals connected to this. */
void NetPins::dump_node_pins(ostream&o, unsigned ind, const char**pin_names) const
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    o << setw(ind) << "" << idx;
	    if (pin_names && pin_names[idx])
		  o << " " << pin_names[idx];
	    else
		  o << " pin" << idx;

	    switch (pin(idx).get_dir()) {
		case Link::PASSIVE:
		  o << " p";
		  break;
		case Link::INPUT:
		  o << " I";
		  break;
		case Link::OUTPUT:
		  o << " O";
		  break;
	    }

	    o << " (" << pin(idx).drive0() << "0 "
	      << pin(idx).drive1() << "1): ";

	    if (pin(idx).is_linked()) {
		  const Nexus*nex = pin(idx).nexus();
		  o << nex << " " << nex->name();
	    }
	    o << endl;

      }
}

void NetObj::dump_obj_attr(ostream&o, unsigned ind) const
{
      unsigned idx;
      for (idx = 0 ;  idx < attr_cnt() ;  idx += 1) {
	    o << setw(ind) << "" << attr_key(idx) << " = \"" <<
		  attr_value(idx) << "\"" << endl;
      }
}

void NetAbs::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Absolute value (NetAbs): " << name()
	<< " width=" << width() << " pin_count=" << pin_count()
	<< endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetAddSub::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Adder (NetAddSub): " << name()
	<< " width=" << width() << " pin_count=" << pin_count()
	<< endl;
      static const char* pin_names[] = {
	    "Cout  ",
	    "DataA ",
	    "DataB ",
	    "Result"
      };

      dump_node_pins(o, ind+4, pin_names);
      dump_obj_attr(o, ind+4);
}

void NetArrayDq::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetArrayDq: " << name()
	<< " array=" << mem_->name()
	<< endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetCastInt::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Cast to int. (NetCastInt): " <<
	    name() << " width=" << width() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetCastReal::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Cast to real (NetCastReal): " <<
	    name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetCLShift::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Combinatorial shift (NetCLShift): " <<
	    name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetCompare::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_COMPARE (NetCompare "
	<< (get_signed()? "signed" : "unsigned") << "): "
	<< name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetConcat::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetConcat: "
	<< name();
      if (rise_time())
	    o << " #(" << *rise_time()
	      << "," << *fall_time() << "," << *decay_time() << ")";
      else
	    o << " #(0,0,0)";
      o << " scope=" << scope_path(scope())
	<< " width=" << width_ << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetDivide::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NET_DIVIDE (NetDivide): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetMult::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_MULT (NetMult): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetPow::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_POW (NetPow): " << name()
	<< " scope=" << scope_path(scope())
	<< " delay=(";
      if (rise_time())
	    o << *rise_time() << "," << *fall_time() << ","
	      <<  *decay_time();
      else
	    o << "0,0,0";

      o << ")"  << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetMux::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Multiplexer (NetMux): " << name()
	<< " width=" << width_ << " swidth=" << swidth_ << " size=" << size_
	<< " scope=" << scope_path(scope()) << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetBUFZ::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetBUFZ: " << name()
	<< " scope=" << scope_path(scope())
	<< " delay=(" << rise_time() << "," << fall_time() << "," <<
	    decay_time() << ") width=" << width() << endl;
      dump_node_pins(o, ind+4);
}

void NetCaseCmp::dump_node(ostream&o, unsigned ind) const
{
      if (eeq_)
	    o << setw(ind) << "" << "case compare === : " << name() << endl;
      else
	    o << setw(ind) << "" << "case compare !== : " << name() << endl;

      dump_node_pins(o, ind+4);
}

void NetConst::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "constant " << width_ << "'b";
      for (unsigned idx = width_ ;  idx > 0 ;  idx -= 1)
	    o << value_[idx-1];
      o << ": " << name();
      if (rise_time())
	    o << " #(" << *rise_time()
	      << "," << *fall_time()
	      << "," << *decay_time() << ")";
      else
	    o << " #(.,.,.)";
      o << endl;
      dump_node_pins(o, ind+4);
}

void NetFF::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_FF: " << name()
	<< " scope=" << scope_path(scope())
	<< " aset_value=" << aset_value_ << endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetLiteral::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "constant real " << real_
	<< ": " << name();
      if (rise_time())
	    o << " #(" << *rise_time()
	      << "," << *fall_time()
	      << "," << *decay_time() << ")";
      else
	    o << " #(.,.,.)";
      o << endl;
      dump_node_pins(o, ind+4);
}

void NetLogic::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "logic: ";
      switch (type_) {
	  case AND:
	    o << "and";
	    break;
	  case BUF:
	    o << "buf";
	    break;
	  case BUFIF0:
	    o << "bufif0";
	    break;
	  case BUFIF1:
	    o << "bufif1";
	    break;
	  case CMOS:
	    o << "cmos";
	    break;
	  case NAND:
	    o << "nand";
	    break;
	  case NMOS:
	    o << "nmos";
	    break;
	  case NOR:
	    o << "nor";
	    break;
	  case NOT:
	    o << "not";
	    break;
	  case NOTIF0:
	    o << "notif0";
	    break;
	  case NOTIF1:
	    o << "notif1";
	    break;
	  case OR:
	    o << "or";
	    break;
	  case PULLDOWN:
	    o << "pulldown";
	    break;
	  case PULLUP:
	    o << "pullup";
	    break;
	  case RCMOS:
	    o << "rcmos";
	    break;
	  case RNMOS:
	    o << "rnmos";
	    break;
	  case RPMOS:
	    o << "rpmos";
	    break;
	  case PMOS:
	    o << "pmos";
	    break;
	  case XNOR:
	    o << "xnor";
	    break;
	  case XOR:
	    o << "xor";
	    break;
      }
      o << " #(" << rise_time()
	<< "," << fall_time() << "," << decay_time() << ") " << name()
	<< " scope=" << scope_path(scope())
	<< endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetModulo::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NET_MODULO (NetModulo): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetPartSelect::dump_node(ostream&o, unsigned ind) const
{
      const char*pt = "";
      switch (dir_) {
	  case VP:
	    pt = "VP";
	    break;
	  case PV:
	    pt = "PV";
	    break;
      }
      o << setw(ind) << "" << "NetPartSelect(" << pt << "): "
	<< name();
      if (rise_time())
	    o << " #(" << *rise_time()
	      << "," << *fall_time()
	      << "," << *decay_time() << ")";
      else
	    o << " #(.,.,.)";
      o << " off=" << off_ << " wid=" << wid_ <<endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetReplicate::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetReplicate: "
	<< name() << " wid=" << width_ << ", repeat_=" << repeat_
	<< ", input wid=" << width_/repeat_ << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetSignExtend::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetSignExtend: " << name();
      if (rise_time())
	    o << " #(" << *rise_time()
	      << "," << *fall_time()
	      << "," << *decay_time() << ")";
      else
	    o << " #(.,.,.)";
      o << " output width=" << width_ << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUReduce::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "reduction logic: ";
      switch (type_) {
	  case NONE:
	    o << "NONE";
	    break;
	  case AND:
	    o << "and";
	    break;
	  case OR:
	    o << "or";
	    break;
	  case XOR:
	    o << "xor";
	    break;
	  case NAND:
	    o << "nand";
	    break;
	  case NOR:
	    o << "nor";
	    break;
	  case XNOR:
	    o << "xnor";
	    break;
      }
      o << " #(" << rise_time()
	<< "," << fall_time() << "," << decay_time() << ") " << name()
	<< " scope=" << scope_path(scope())
	<< endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetSysFunc::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << def_->name << "(...) -->"
	<< data_type() << " width=" << vector_width() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUserFunc::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "USER FUNC: "
	<< scope_path(def_);
      if (rise_time())
	    o << " #(" <<*rise_time()
	      <<","<<*fall_time()
	      << "," <<*decay_time() << ")" << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetTaskDef::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "task " << scope_path(scope_) << ";" << endl;

      for (unsigned idx = 0 ;  idx < ports_.count() ;  idx += 1) {
	    o << setw(ind+4) << "";
	    assert(ports_[idx]);
	    switch (ports_[idx]->port_type()) {
		case NetNet::PINPUT:
		  o << "input ";
		  break;
		case NetNet::POUTPUT:
		  o << "output ";
		  break;
		case NetNet::PINOUT:
		  o << "input ";
		  break;
		default:
		  o << "NOT_A_PORT ";
		  break;
	    }
	    o << ports_[idx]->name() << ";" << endl;
      }

      proc_->dump(o, ind+4);

      o << setw(ind) << "" << "endtask" << endl;
}

void NetTran::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type_ << " " << name()
	<< " island " << get_island();
      if (type_ == IVL_SW_TRAN_VP) {
	    o << " width=" << vector_width()
	      << " part=" << part_width()
	      << " offset=" << part_offset();
      }
      o << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUDP::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "UDP (" << udp_name() << "): ";
      o << " #(" << rise_time() << "," << fall_time() << "," << decay_time() <<
	    ") " << name() << endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetProcTop::dump(ostream&o, unsigned ind) const
{
      switch (type_) {
	  case IVL_PR_INITIAL:
	    o << "initial  /* " << get_fileline() << " in "
	      << scope_path(scope_) << " */" << endl;
	    break;
	  case IVL_PR_ALWAYS:
	    o << "always  /* " << get_fileline() << " in "
	      << scope_path(scope_) << " */" << endl;
	    break;
      }

      for (unsigned idx = 0 ;  idx < attr_cnt() ;  idx += 1) {
	    o << setw(ind+2) << "" << "(* " << attr_key(idx) << " = "
	      << attr_value(idx) << " *)" << endl;
      }

      statement_->dump(o, ind+2);
}

void NetAnalogTop::dump(ostream&o, unsigned ind) const
{
      switch (type_) {
	  case IVL_PR_INITIAL:
	    o << "analog initial /* " << get_fileline() << " in "
	      << scope_path(scope_) << " */" << endl;
	    break;

	  case IVL_PR_ALWAYS:
	    o << "analog /* " << get_fileline() << " in "
	      << scope_path(scope_) << " */" << endl;
	    break;
      }

      statement_->dump(o, ind+2);
}

void NetAlloc::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "// allocate storage : " << scope_path(scope_) << endl;
}

void NetAssign_::dump_lval(ostream&o) const
{
      if (sig_) {
	    o << sig_->name();
	    if (word_) {
		  o << "[word=" << *word_ << "]";
	    }
	    if (base_) {
		  o << "[" << *base_ << " +: " << lwid_ << "]";
	    }
      } else {
	    o << "";
      }
}

void NetAssignBase::dump_lval(ostream&o) const
{
      o << "{";
      lval_->dump_lval(o);

      for (NetAssign_*cur = lval_->more ;  cur ;  cur = cur->more) {
	    o << ", ";
	    cur->dump_lval(o);
      }

      o << "}";
}

/* Dump an assignment statement */
void NetAssign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      dump_lval(o);

      o << " = ";

      if (const NetExpr*de = get_delay())
	    o << "#(" << *de << ") ";

      o << *rval() << ";" << endl;
}

void NetAssignNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      dump_lval(o);

      o << " <= ";

      if (const NetExpr*de = get_delay())
	    o << "#(" << *de << ") ";
      if (count_)
	    o << "repeat(" << *count_ << ") ";
      if (event_) {
	    o << *event_;
      }

      o << *rval() << ";" << endl;

}

void NetAssignBase::dump(ostream&o, unsigned ind) const
{
      if (const NetAssignNB *n1 = dynamic_cast<const NetAssignNB*>(this)) {
	    n1->dump(o,ind);
      } else if (const NetAssign *n2 = dynamic_cast<const NetAssign*>(this)) {
	    n2->dump(o,ind);
      }
}

/* Dump a block statement */
void NetBlock::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type_;
      if (subscope_)
	    o << " : " << scope_path(subscope_);
      o << endl;

      if (last_) {
	    const NetProc*cur = last_;
	    do {
		  cur = cur->next_;
		  cur->dump(o, ind+4);
	    } while (cur != last_);
      }

      o << setw(ind) << "" << "end" << endl;
}

void NetCase::dump(ostream&o, unsigned ind) const
{
      switch (type_) {
	  case EQ:
	    o << setw(ind) << "" << "case (" << *expr_ << ")" << endl;
	    break;
	  case EQX:
	    o << setw(ind) << "" << "casex (" << *expr_ << ")" << endl;
	    break;
	  case EQZ:
	    o << setw(ind) << "" << "casez (" << *expr_ << ")" << endl;
	    break;
      }

      for (unsigned idx = 0 ;  idx < nitems_ ;  idx += 1) {
	    o << setw(ind+2) << "";
	    if (items_[idx].guard)
		  o << *items_[idx].guard << ":";
	    else
		  o << "default:";

	    if (items_[idx].statement) {
		  o << endl;
		  items_[idx].statement->dump(o, ind+6);
	    } else {
		  o << " ;" << endl;
	    }
      }

      o << setw(ind) << "" << "endcase" << endl;
}

void NetCAssign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "cassign ";
      dump_lval(o);
      o << " = " << *rval() << "; /* " << get_fileline() << " */" << endl;
}

void NetCondit::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "if (";
      expr_->dump(o);
      o << ")" << endl;

      if (if_) if_->dump(o, ind+4);
      else o << setw(ind+4) << "" << "/* empty */ ;" << endl;

      if (else_) {
	    o << setw(ind) << "" << "else" << endl;
	    else_->dump(o, ind+4);
      }
}

void NetContribution::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      lval_->dump(o);
      o << " <+ ";
      rval_->dump(o);
      o << ";" << endl;
}

void NetDeassign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "deassign ";
      dump_lval(o);
      o << "; /* " << get_fileline() << " */" << endl;
}

void NetDisable::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "disable " << scope_path(target_) << "; "
	<< "/* " << get_fileline() << " */" << endl;
}

void NetEvProbe::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";

      switch (edge_) {
	  case ANYEDGE:
	    o << "anyedge ";
	    break;
	  case POSEDGE:
	    o << "posedge ";
	    break;
	  case NEGEDGE:
	    o << "negedge ";
	    break;
      }
      o << setw(ind) << "" << "-> " << scope_path(event_->scope()) << "." << event_->name() << "; " << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetEvTrig::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "-> " << event_->name() << "; "
	<< "// " << get_fileline() << endl;
}

void NetEvWait::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) <<"" << "@(";

      if (nevents() > 0)
	    o << scope_path(event(0)->scope()) << "." << event(0)->name();

      for (unsigned idx = 1 ;  idx < nevents() ;  idx += 1)
	    o << " or " << scope_path(event(idx)->scope()) << "." << event(idx)->name();

      o << ")  // " << get_fileline() << endl;

      if (statement_)
	    statement_->dump(o, ind+2);
      else
	    o << setw(ind+2) << "" << "/* noop */ ;" << endl;
}

ostream& operator << (ostream&out, const NetEvWait&obj)
{
      obj.dump_inline(out);
      return out;
}

void NetEvWait::dump_inline(ostream&o) const
{
      o << "@(";

      if (nevents() > 0)
	    o << scope_path(event(0)->scope()) << "." << event(0)->name();

      for (unsigned idx = 1 ;  idx < nevents() ;  idx += 1)
	    o << " or " << scope_path(event(idx)->scope())<<"."<<event(idx)->name();

      o << ") ";
}

void NetForce::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "force ";
      dump_lval(o);
      o << " = " << *rval() << "; /* " << get_fileline() << " */" << endl;
}

void NetForever::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "forever" << endl;
      statement_->dump(o, ind+2);
}

void NetFree::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "// free storage : " << scope_path(scope_) << endl;
}

void NetFuncDef::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "function definition for " << scope_path(scope_) << endl;
      if (result_sig_) {
	    o << setw(ind+2) << "" << "Return signal: ";
	    if (result_sig_->get_signed()) o << "+";
	    o << result_sig_->name() << "[" << result_sig_->msb() << ":"
	      << result_sig_->lsb() << "]" << endl;
      }
      o << setw(ind+2) << "" << "Arguments: ";
      if (port_count() == 0) o << "<none>";
      o << endl;
      for (unsigned idx = 0; idx < port_count(); idx += 1) {
	    o << setw(ind+4) << "" << "Arg[" << idx+1 << "] = ";
	    switch (port(idx)->port_type()) {
		default:
		  o << "implicit-port? ";
		  break;
		case NetNet::PINPUT:
		  o << "input ";
		  break;
		case NetNet::POUTPUT:
		  o << "output ";
		  break;
		case NetNet::PINOUT:
		  o << "inout ";
		  break;
	    }
	    if (port(idx)->get_signed()) o << "+";
	    o << port(idx)->name() << "[" << port(idx)->msb() << ":"
	      << port(idx)->lsb() << "]" << endl;
      }
      if (statement_)
	    statement_->dump(o, ind+2);
      else
	    o << setw(ind+2) << "" << "// NO STATEMENT" << endl;
}

void NetPDelay::dump(ostream&o, unsigned ind) const
{
      if (expr_) {
	    o << setw(ind) << "" << "#" << *expr_;

      } else {
	    o << setw(ind) << "" << "#" << delay_;
      }

      if (statement_) {
	    o << endl;
	    statement_->dump(o, ind+2);
      } else {
	    o << " /* noop */;" << endl;
      }
}

void NetRelease::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "release ";
      dump_lval(o);
      o << "; /* " << get_fileline() << " */" << endl;
}

void NetRepeat::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "repeat (" << *expr_ << ")" << endl;
      statement_->dump(o, ind+2);
}

void NetScope::dump(ostream&o) const
{
	/* This is a constructed hierarchical name. */
      o << scope_path(this) << " ";

      print_type(o);
      if (is_auto()) o << " (automatic)";
      if (is_cell()) o << " (cell)";
      o << endl;

      for (unsigned idx = 0 ;  idx < attr_cnt() ;  idx += 1)
	    o << "    (* " << attr_key(idx) << " = "
	      << attr_value(idx) << " *)" << endl;

      o << "    timescale = 10e" << time_unit() << " / 10e"
	<< time_precision() << endl;

	/* Dump the parameters for this scope. */
      {
	    map<perm_string,param_expr_t>::const_iterator pp;
	    for (pp = parameters.begin()
		       ; pp != parameters.end() ;  pp ++) {
		  o << "    parameter ";

		  o << pp->second.type << " ";

		  if ((*pp).second.signed_flag)
			o << "signed ";

		  if ((*pp).second.msb)
			o << "[" << *(*pp).second.msb
			  << ":" << *(*pp).second.lsb << "] ";

		  o << (*pp).first << " = ";
		  if (pp->second.expr)
			o << *(*pp).second.expr;
		  else
			o << "<nil>";

		  for (range_t*ran = (*pp).second.range ; ran ; ran = ran->next) {
			if (ran->exclude_flag)
			      o << " exclude ";
			else
			      o << " from ";

			if (ran->low_open_flag)
			      o << "(";
			else
			      o << "[";
			if (ran->low_expr)
			      o << *ran->low_expr;
			else if (ran->low_open_flag==false)
			      o << "-inf";
			else
			      o << "<?>";
			if (ran->high_expr)
			      o << ":" << *ran->high_expr;
			else if (ran->high_open_flag==false)
			      o << ":inf";
			else
			      o << ":<?>";
			if (ran->high_open_flag)
			      o << ")";
			else
			      o << "]";
		  }

		  o << ";" << endl;
	    }

	    for (pp = localparams.begin()
		       ; pp != localparams.end() ;  pp ++) {
		  o << "    localparam " << (*pp).first << " = "  <<
			*(*pp).second.expr << ";" << endl;
	    }
      }

	/* Dump the saved defparam assignments here. */
      {
	    list<pair<pform_name_t,NetExpr*> >::const_iterator pp;
	    for (pp = defparams.begin()
		       ; pp != defparams.end() ;  pp ++ ) {
		  o << "    defparam " << (*pp).first << " = " <<
			*(*pp).second << ";" << endl;
	    }
      }

      {
	    list<pair<list<hname_t>,NetExpr*> >::const_iterator pp;
	    for (pp = defparams_later.begin()
		       ; pp != defparams_later.end() ;  pp ++ ) {
		  o << "    defparam(later) " << pp->first << " = " <<
			*(pp->second) << ";" << endl;
	    }
      }

	/* Dump the events in this scope. */
      for (NetEvent*cur = events_ ;  cur ;  cur = cur->snext_) {
	    o << "    event " << cur->name() << "; nprobe="
	      << cur->nprobe() << " scope=" << scope_path(cur->scope())
	      << " // " << cur->get_fileline() << endl;
      }

	// Dump the signals,
      for (signals_map_iter_t cur = signals_map_.begin()
		 ; cur != signals_map_.end() ; cur ++) {
	    cur->second->dump_net(o, 4);
      }

	// Dump specparams
      typedef map<perm_string,spec_val_t>::const_iterator specparam_it_t;
      for (specparam_it_t cur = specparams.begin()
		 ; cur != specparams.end() ;  cur ++ ) {
	    o << "    specparam " << (*cur).first
	      << " = ";
	    spec_val_t value = (*cur).second;
	    switch (value.type) {
		case IVL_VT_REAL:
		  o << "R:" << value.real_val;
		  break;
		case IVL_VT_BOOL:
		  o << "I:" << value.integer;
		  break;
		default:
		  o << "<bad type>";
		  break;
	    }
	    o << endl;
      }

      switch (type_) {
	  case FUNC:
	    if (func_def())
		  func_def()->dump(o, 4);
	    else
		  o << "    MISSING FUNCTION DEFINITION" << endl;
	    break;
	  case TASK:
	    if (task_def())
		  task_def()->dump(o, 4);
	    else
		  o << "    MISSING TASK DEFINITION" << endl;
	    break;
	  default:
	    break;
      }

	/* Dump any sub-scopes. */
      for (NetScope*cur = sub_ ;  cur ;  cur = cur->sib_)
	    cur->dump(o);
}

void NetSTask::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << name_;

      if (parms_.count() > 0) {
	    o << "(";
	    if (parms_[0])
		  parms_[0]->dump(o);

	    for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
		  o << ", ";
		  if (parms_[idx])
			parms_[idx]->dump(o);
	    }

	    o << ")";
      }
      o << ";" << endl;
}

void NetUTask::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << scope_path(task_) << ";" << endl;
}

void NetWhile::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "while (" << *cond_ << ")" << endl;
      proc_->dump(o, ind+3);
}

/* Dump a statement type that someone didn't write a dump for. */
void NetProc::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "// " << typeid(*this).name() << endl;
}

/* Dump an expression that no one wrote a dump method for. */
void NetExpr::dump(ostream&o) const
{
      o << "(?" << typeid(*this).name() << "?)";
}

void NetEAccess::dump(ostream&o) const
{
      o << nature_->name() << "." << nature_->access() << "(";
      assert(branch_);
      if (branch_->pin(0).is_linked())
	    o << branch_->pin(0).nexus()->name();
      o << ", ";
      if (branch_->pin(1).is_linked())
	    o << branch_->pin(1).nexus()->name();
      o << ")";
}

void NetEBinary::dump(ostream&o) const
{
      if (op_ == 'm' || op_ == 'M') {
	    if (op_ == 'm')
		  o << "min";
	    else
		  o << "max";

	    o << "(";
	    left_->dump(o);
	    o << ", ";
	    right_->dump(o);
	    o << ")";
	    return;
      }

      o << "(";
      left_->dump(o);
      o << ")";
      switch (op_) {
	  default:
	    o << op_;
	    break;
	  case 'a':
	    o << "&&";
	    break;
	  case 'E':
	    o << "===";
	    break;
	  case 'e':
	    o << "==";
	    break;
	  case 'G':
	    o << ">=";
	    break;
	  case 'l':
	    o << "<<";
	    break;
	  case 'L':
	    o << "<=";
	    break;
	  case 'n':
	    o << "!=";
	    break;
	  case 'N':
	    o << "!==";
	    break;
	  case 'o':
	    o << "||";
	    break;
	  case 'O':
	    o << "~|";
	    break;
	  case 'p':
	    o << "**";
	    break;
	  case 'r':
	    o << ">>";
	    break;
	  case 'R':
	    o << ">>>";
	    break;
	  case 'X':
	    o << "~^";
	    break;
      }
      o << "(";
      right_->dump(o);
      o << ")";
}

void NetEConcat::dump(ostream&o) const
{
      if (repeat_calculated_) {
	    if (repeat_value_ != 1)
		  o << repeat_value_;
      } else if (repeat_) {
	    o << "<" << *repeat_ << ">";
      }

      if (parms_[0])
	    o << "{" << *parms_[0];
      else
	    o << "{";

      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    if (parms_[idx])
		  o << ", " << *parms_[idx];
	    else
		  o << ", ";
      }
      o << "}";
}

void NetEConst::dump(ostream&o) const
{
      if (value_.is_string())
	    o << "\"" << value_.as_string() << "\"";
      else
	    o << value_;
}

void NetEConstParam::dump(ostream&o) const
{
      o << "<" << name_ << "=";
      NetEConst::dump(o);
      o << ", wid=" << expr_width() << ">";
}

void NetECReal::dump(ostream&o) const
{
      o << value_;
}

void NetECRealParam::dump(ostream&o) const
{
      o << "<" << name_ << "=";
      NetECReal::dump(o);
      o << ">";
}

void NetEEvent::dump(ostream&o) const
{
      o << "<event=" << event_->name() << ">";
}

void NetEScope::dump(ostream&o) const
{
      o << "<scope=" << scope_path(scope_) << ">";
}

void NetESelect::dump(ostream&o) const
{
      o << "<select";
      if (has_sign())
	    o << "+=";
      else
	    o << "=";

      expr_->dump(o);
      o << "[";

      if (base_)
	    base_->dump(o);
      else
	    o << "(0)";

      o << "+:" << expr_width() << "]>";
}

void NetESFunc::dump(ostream&o) const
{
      o << name_ << "(";
      if (nparms() > 0)
	    o << *parm(0);
      for (unsigned idx = 1 ;  idx < nparms() ;  idx += 1)
	    o << ", " << *parm(idx);
      o << ")";
}

void NetESignal::dump(ostream&o) const
{
      if (has_sign())
	    o << "+";
      o << name();
      if (word_) o << "[word=" << *word_ << "]";
      o << "[" << msi()<<":"<<lsi() << "]";
}

void NetEParam::dump(ostream&o) const
{
      if (scope_ != 0)
	    o << "<" << scope_path(scope_) << "." << (*reference_).first << ">";
      else
	    o << "<" << (*reference_).first << ">";
}

void NetETernary::dump(ostream&o) const
{
      o << "(" << *cond_ << ") ? (" << *true_val_ << ") : (" <<
	    *false_val_ << ")";
}

void NetEUFunc::dump(ostream&o) const
{
      o << func_->basename() << "(";
      if (parms_.count() > 0) {
	    parms_[0]->dump(o);
	    for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
		  o << ", ";
		  parms_[idx]->dump(o);
	    }
      }
      o << ")";
}

void NetEUnary::dump(ostream&o) const
{
      switch (op_) {
	  case 'm':
	    o << "abs";
	    break;
	  case 'N':
	    o << "~|";
	    break;
	  default:
	    o << op_;
	    break;
      }
      o << "(";
      expr_->dump(o);
      o << ")";
}

void Design::dump(ostream&o) const
{
      o << "DESIGN TIME PRECISION: 10e" << get_precision() << endl;
      o << "SCOPES:" << endl;
      for (list<NetScope*>::const_iterator scope = root_scopes_.begin();
	   scope != root_scopes_.end(); scope++)
	    (*scope)->dump(o);

      o << "ELABORATED NODES:" << endl;

	// dump the nodes,
      if (nodes_) {
	    NetNode*cur = nodes_->node_next_;
	    do {
		  assert(cur);
		  cur->dump_node(o, 0);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }

      o << "ELABORATED BRANCHES:" << endl;

      if (branches_) {
	    for (NetBranch*cur = branches_ ; cur ; cur = cur->next_)
		  cur->dump(o, 0);
      }

      o << "ELABORATED PROCESSES:" << endl;

	// Dump the processes.
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    idx->dump(o, 0);

      for (const NetAnalogTop*idx = aprocs_ ; idx ; idx = idx->next_)
	    idx->dump(o, 0);
}
