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

/* Dump a net. This can be a wire or register. */
void NetNet::dump_net(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type() << ": " << name() << "[" <<
	    pin_count() << "]";
      if (local_flag_)
	    o << " (local)";
      if (signed_)
	    o << " signed";
      if (mref_)
	    o << " mref_=" << mref_->name();
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
      o << " (eref=" << peek_eref() << ", lref=" << peek_lref() << ")";
      if (scope())
	    o << " scope=" << scope()->name();
      o << " #(" << rise_time() << "," << fall_time() << "," <<
	    decay_time() << ") init=";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << pin(idx-1).get_init();

      o << " (";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << pin(idx-1).nexus()->get_init();
      o << ")" << endl;

      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    if (! pin(idx).is_linked())
		  continue;

	    const Nexus*nex = pin(idx).nexus();
	    o << setw(ind+4) << "" << "[" << idx << "]: " << nex
	      << " " << nex->name() << endl;
      }
      dump_obj_attr(o, ind+4);
}

void NetSubnet::dump_net(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "** " << name() << " is a NetSubnet **"
	<< endl;
      NetNet::dump_net(o, ind);
}

void NetMemory::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << ""  << name_ << "[" << width_ << "] " <<
	    "[" << idxh_ << ":" << idxl_ << "]" << endl;
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
void NetObj::dump_node_pins(ostream&o, unsigned ind) const
{
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1) {
	    o << setw(ind) << "" << idx << " " << pin(idx).get_name()
	      << "<" << pin(idx).get_inst() << ">";

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
		  const char*nex_name = nex->name();
		  o << nex << " " << (nex_name? nex_name : "????");
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

void NetAddSub::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Adder (NetAddSub): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetCAssign::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Procedural continuous assign (NetCAssign): "
	<< name() << endl;
      dump_node_pins(o, ind+4);
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
      o << setw(ind) << "" << "LPM_COMPARE (NetCompare): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetDecode::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_DECODE (NetDecode): " << name()
	<< " ff=" << ff_->name() << ", word width=" << width() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetDemux::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_DEMUX (NetDemux): " << name()
	<< " bus width=" << width() << ", address width=" << awidth()
	<< ", word count=" << size() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetDivide::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NET_DIVIDE (NetDivide): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetForce::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "force " << lval_->name() << endl;
      dump_node_pins(o, ind+4);
}

void NetMult::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_MULT (NetMult): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetMux::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Multiplexer (NetMux): " << name()
	<< " scope=" << scope()->name() << endl;
      o << setw(ind+6) << "" << "source=" << get_line() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetBUFZ::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetBUFZ: " << name()
	<< " scope=" << (scope()? scope()->name() : "")
	<< " delay=(" << rise_time() << "," << fall_time() << "," <<
	    decay_time() << ")" << endl;
      dump_node_pins(o, ind+4);
}

void NetCaseCmp::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "case compare === : " << name() << endl;
      dump_node_pins(o, ind+4);
}

void NetConst::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "constant ";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << value_[idx-1];
      o << ": " << name() << endl;
      dump_node_pins(o, ind+4);
}

void NetFF::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_FF: " << name()
	<< " scope=" << (scope()? scope()->name() : "")
	<< " aset_value=" << aset_value_
	<< " sset_value=" << sset_value_;

      if (demux_) o << " demux=" << demux_->name();

      o << endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
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
	<< " scope=" << (scope()? scope()->name() : "")
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

void NetRamDq::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "LPM_RAM_DQ (";

      if (mem_) {
	    if (NetNet*tmp = mem_->reg_from_explode())
		  o << "exploded mem=" << tmp->name();
	    else
		  o << "mem=" << mem_->name();
      }

      o << "): " << name() << endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUserFunc::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << def_->name() << "(";
      o << ")" << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetTaskDef::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "task " << name_ << ";" << endl;

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
	  case NetProcTop::KINITIAL:
	    o << "initial  /* " << get_line() << " in "
	      << scope_->name() << " */" << endl;
	    break;
	  case NetProcTop::KALWAYS:
	    o << "always  /* " << get_line() << " in "
	      << scope_->name() << " */" << endl;
	    break;
      }

      for (unsigned idx = 0 ;  idx < attr_cnt() ;  idx += 1) {
	    o << setw(ind+2) << "" << "(* " << attr_key(idx) << " = "
	      << attr_value(idx) << " *)" << endl;
      }

      statement_->dump(o, ind+2);
}

void NetAssign_::dump_lval(ostream&o) const
{
      if (sig_) {
	    o << "sig=" << sig_->name();
	    if (bmux_) {
		  o << "[" << *bmux_ << "]";

	    } else {
		  o << "[" << (loff_+lwid_-1) << ":" << loff_ << "]";
	    }
      } else if (mem_) {
	    // Is there an obvious way to flag memories in the dump
	    // as different from the _real_ bit mux case?
	    // o << "**memory**";
	    o << "mem=" << mem_->name() << "[";
	    if (bmux_) o << *bmux_;
	      else     o << "**oops**";
	    o << "]";
      } else if (var_) {
	    o << "<real " << var_->basename() << ">";
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
#if 0
      if (l_val(0)->rise_time())
	    o << "#" << l_val(0)->rise_time() << " ";
#endif
      o << *rval() << ";" << endl;
}

void NetAssignNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      dump_lval(o);

      o << " <= ";
#if 0
      if (l_val(0)->rise_time())
	    o << "#" << l_val(0)->rise_time() << " ";
#endif
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
	    o << " : " << subscope_->name();
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

      dump_proc_attr(o, ind+2);

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
      o << setw(ind) << "" << "cassign " << lval_->name() << " = "
	<< name() << ";" << endl;
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

void NetDeassign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "deassign " << lval_->name() << "; "
	<< "/* " << get_line() << " */" << endl;
}

void NetDisable::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "disable " << target_->name() << "; "
	<< "/* " << get_line() << " */" << endl;
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
      o << setw(ind) << "" << "-> " << event_->full_name() << "; " << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetEvTrig::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "-> " << event_->name() << "; "
	<< "// " << get_line() << endl;
}

void NetEvWait::dump(ostream&o, unsigned ind) const
{
      assert(nevents() > 0);
      o << setw(ind) <<"" << "@(" << event(0)->full_name();

      for (unsigned idx = 1 ;  idx < nevents() ;  idx += 1)
	    o << " or " << event(idx)->full_name();

      o << ")  // " << get_line() << endl;

      if (statement_)
	    statement_->dump(o, ind+2);
      else
	    o << setw(ind+2) << "" << "/* noop */ ;" << endl;
}

void NetForce::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "force " << lval_->name() << " = "
	<< name() << ";" << endl;
}

void NetForever::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "forever" << endl;
      statement_->dump(o, ind+2);
}

void NetFuncDef::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "function " << scope_->name() << endl;
      if (result_sig_)
	    o << setw(ind+2) << "" << "Return signal: "
	      << result_sig_->name() << endl;
      if (result_var_)
	    o << setw(ind+2) << "" << "Return variable: "
	      << result_var_->basename() << endl;

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
      if (lval_)
	    o << setw(ind) << "" << "release " << lval_->name() << "; "
	      << "/* " << get_line() << " */" << endl;
      else
	    o << setw(ind) << "" << "release (null); "
	      << "/* " << get_line() << " */" << endl;
}

void NetRepeat::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "repeat (" << *expr_ << ")" << endl;
      statement_->dump(o, ind+2);
}

void NetScope::dump(ostream&o) const
{
	/* This is a constructed hierarchical name. */
      o << name();

      switch (type_) {
	  case BEGIN_END:
	    o << " sequential block";
	    break;
	  case FORK_JOIN:
	    o << " parallel block";
	    break;
	  case FUNC:
	    o << " function";
	    break;
	  case MODULE:
	    o << " module <" << (module_name_? module_name_.str() : "") << ">";
	    break;
	  case TASK:
	    o << " task";
	    break;
      }
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

		  if ((*pp).second.signed_flag)
			o << "signed ";

		  if ((*pp).second.msb)
			o << "[" << *(*pp).second.msb
			  << ":" << *(*pp).second.lsb << "] ";

		  o << (*pp).first << " = "  <<
			*(*pp).second.expr << ";" << endl;
	    }

	    for (pp = localparams.begin()
		       ; pp != localparams.end() ;  pp ++) {
		  o << "    localparam " << (*pp).first << " = "  <<
			*(*pp).second.expr << ";" << endl;
	    }
      }

	/* Dump the saved defparam assignments here. */
      {
	    map<hname_t,NetExpr*>::const_iterator pp;
	    for (pp = defparams.begin()
		       ; pp != defparams.end() ;  pp ++ ) {
		  o << "    defparam " << (*pp).first << " = " <<
			*(*pp).second << ";" << endl;
	    }
      }

      for (NetVariable*cur = vars_ ;  cur ;  cur = cur->snext_) {
	    o << "    real " << cur->basename() << " // "
	      << cur->get_line() << endl;
      }

	/* Dump the events in this scope. */
      for (NetEvent*cur = events_ ;  cur ;  cur = cur->snext_) {
	    o << "    event " << cur->name() << "; nprobe="
	      << cur->nprobe() << " // " << cur->get_line() << endl;
      }

	// Dump the signals,
      if (signals_) {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  cur->dump_net(o, 4);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }

	// Dump the memories,
      if (memories_) {
	    NetMemory*cur = memories_->snext_;
	    do {
		  cur->dump(o, 4);
		  cur = cur->snext_;
	    } while (cur != memories_->snext_);
      }

      switch (type_) {
	  case FUNC:
	    if (func_def())
		  func_def()->dump(o, 4);
	    else
		  o << "    MISSING FUNCTION DEFINITION" << endl;
	    break;
	  case TASK:
	    task_def()->dump(o, 4);
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
      o << setw(ind) << "" << task_->name() << ";" << endl;
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

void NetProc::dump_proc_attr(ostream&o, unsigned ind) const
{
      unsigned idx;
      for (idx = 0 ;  idx < attr_cnt() ;  idx += 1) {
	    o << setw(ind) << "" << "(* " << attr_key(idx) << " = "
	      << attr_value(idx) << " *)" << endl;
      }
}

/* Dump an expression that no one wrote a dump method for. */
void NetExpr::dump(ostream&o) const
{
      o << "(?" << typeid(*this).name() << "?)";
}

void NetEBinary::dump(ostream&o) const
{
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
      o << ">";
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
      o << "<scope=" << scope_->name() << ">";
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
      o << name() << "[" << msi_<<":"<<lsi_ << "]";
}

void NetEBitSel::dump(ostream&o) const
{
      sig_->dump(o);
      o << "[";
      idx_->dump(o);
      o << "]";
}

void NetEMemory::dump(ostream&o) const
{
      o << mem_->name() << "[";
      if (idx_) idx_->dump(o);
      o << "]";
}

void NetEParam::dump(ostream&o) const
{
      if (scope_ != 0)
	    o << "<" << scope_->name() << "." << name_ << ">";
      else
	    o << "<" << name_ << ">";
}

void NetETernary::dump(ostream&o) const
{
      o << "(" << *cond_ << ") ? (" << *true_val_ << ") : (" <<
	    *false_val_ << ")";
}

void NetEUFunc::dump(ostream&o) const
{
      o << name() << "(";
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

void NetEVariable::dump(ostream&o) const
{
      o << var_->basename();
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
		  cur->dump_node(o, 0);
		  cur = cur->node_next_;
	    } while (cur != nodes_->node_next_);
      }

      o << "ELABORATED PROCESSES:" << endl;

	// Dump the processes.
      for (const NetProcTop*idx = procs_ ;  idx ;  idx = idx->next_)
	    idx->dump(o, 0);

}
