/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: design_dump.cc,v 1.102 2000/11/04 06:36:24 steve Exp $"
#endif

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
      o << " (eref=" << get_eref() << ")";
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
		  o << nex << " " << nex->name();
	    }
	    o << endl;

      }
}

void NetObj::dump_obj_attr(ostream&o, unsigned ind) const
{
      for (map<string,string>::const_iterator idx = attributes_.begin()
		 ; idx != attributes_.end()
		 ; idx ++) {
	    o << setw(ind) << "" << (*idx).first << " = \"" <<
		  (*idx).second << "\"" << endl;
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
      o << setw(ind) << "" << "Multiplexer (NetMux): " << name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetAssign_::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Procedural assign (NetAssign_): " << name();
      if (bmux())
	    o << "[" << *bmux() << "]";
      o << endl;
      dump_node_pins(o, ind+4);
}

void NetBUFZ::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetBUFZ: " << name()
	<< " scope=" << (scope()? scope()->name() : "") << endl;
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
      o << setw(ind) << "" << "LPM_FF: " << name() << endl;
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
	  case NOR:
	    o << "nor";
	    break;
	  case NOT:
	    o << "not";
	    break;
	  case OR:
	    o << "or";
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
      o << setw(ind) << "" << "LPM_RAM_DQ (" << mem_->name() << "): "
	<< name() << endl;
      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetTaskDef::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "task " << name_ << ";" << endl;

      for (unsigned idx = 0 ;  idx < ports_.count() ;  idx += 1) {
	    o << setw(ind+4) << "";
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

void NetUDP::dump_sequ_(ostream&o, unsigned ind) const
{
      string tmp = "";
      for (unsigned idx = 0 ;  idx < ind ;  idx += 1)
	    tmp += " ";

      o << tmp << "Sequential UDP" << " #(" << rise_time() <<
	    "," << fall_time() << "," << decay_time() << ") " << name() <<
	    endl;

      for (FSM_::const_iterator ent = fsm_.begin()
		 ; ent != fsm_.end() ;  ent++) {
	    o << setw(ind+6) << "" << (*ent).first << " -->";

	    state_t_*st = (*ent).second;
	    assert((*ent).first[0] == st->out);
	    for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {
		  string tmp = (*ent).first;
		  if (st->pins[idx].zer) {
			tmp[0] = st->pins[idx].zer->out;
			tmp[idx] = '0';
			o << " " << tmp;
		  }

		  if (st->pins[idx].one) {
			tmp[0] = st->pins[idx].one->out;
			tmp[idx] = '1';
			o << " " << tmp;
		  }

		  if (st->pins[idx].xxx) {
			tmp[0] = st->pins[idx].xxx->out;
			tmp[idx] = 'x';
			o << " " << tmp;
		  }
	    }

	    o << endl;
      }

      o << setw(ind+6) << ""  << "initial value == " << init_ << endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUDP::dump_comb_(ostream&o, unsigned ind) const
{
      assert(0);
}

void NetUDP::dump_node(ostream&o, unsigned ind) const
{
      if (is_sequential())
	    dump_sequ_(o, ind);
      else
	    dump_comb_(o, ind);
}

void NetUDP_COMB::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Combinational primitive: ";
      o << " #(" << rise_time() << "," << fall_time() << "," << decay_time() <<
	    ") " << name() << endl;

      for (map<string,char>::const_iterator ent = cm_.begin()
		 ; ent != cm_.end() ;  ent++) {
	    o << setw(ind+6) << "" << (*ent).first << " --> " <<
		  (*ent).second << endl;
      }

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

      statement_->dump(o, ind+2);
}

void NetAssignBase::dump_lval(ostream&o) const
{
      o << "" << "{" << lval_->name();
      if (lval_->bmux())
	    o << "[" << *lval_->bmux() << "]";

      for (NetAssign_*cur = lval_->more ;  cur ;  cur = cur->more) {
	    o << ", " << cur->name();
	    if (cur->bmux())
		  o << "[" << *cur->bmux() << "]";
      }

      o << "}";
}

/* Dump an assignment statement */
void NetAssign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      dump_lval(o);

      o << " = ";
      if (l_val(0)->rise_time())
	    o << "#" << l_val(0)->rise_time() << " ";
      o << *rval() << ";" << endl;
}

void NetAssignNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      dump_lval(o);

      o << " <= ";
      if (l_val(0)->rise_time())
	    o << "#" << l_val(0)->rise_time() << " ";
      o << *rval() << ";" << endl;

}

void NetAssignMem::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      o << "/* " << get_line() << " */" << endl;
      o << setw(ind) << "";
      o << memory()->name() << "[" << *index() << "] = ";
      rval()->dump(o);
      o << ";" << endl;
}

void NetAssignMemNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      o << "/* " << get_line() << " */" << endl;
      o << setw(ind) << "";
      o << memory()->name() << "[" << *index() << "] <= ";
      rval()->dump(o);
      o << ";" << endl;
}

/* Dump a block statement */
void NetBlock::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type_ << endl;

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
      o << setw(ind) << "" << "release " << lval_->name() << "; "
	<< "/* " << get_line() << " */" << endl;
}

void NetRepeat::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "repeat (" << *expr_ << ")" << endl;
      statement_->dump(o, ind+2);
}

void NetScope::dump(ostream&o) const
{
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
	    o << " module";
	    break;
	  case TASK:
	    o << " task";
	    break;
      }
      o << endl;

      o << "    timescale = 10e" << time_unit() << " / 10e"
	<< time_precision() << endl;

	/* Dump the parameters for this scope. */
      {
	    map<string,NetExpr*>::const_iterator pp;
	    for (pp = parameters_.begin()
		       ; pp != parameters_.end() ;  pp ++) {
		  o << "    parameter " << (*pp).first << " = "  <<
			*(*pp).second << ";" << endl;
	    }

	    for (pp = localparams_.begin()
		       ; pp != localparams_.end() ;  pp ++) {
		  o << "    localparam " << (*pp).first << " = "  <<
			*(*pp).second << ";" << endl;
	    }
      }

	/* Dump the saved defparam assignments here. */
      {
	    map<string,NetExpr*>::const_iterator pp;
	    for (pp = defparams.begin()
		       ; pp != defparams.end() ;  pp ++ ) {
		  o << "    defparam " << (*pp).first << " = " <<
			*(*pp).second << ";" << endl;
	    }
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
	    func_def()->dump(o, 4);
	    break;
	  case TASK:
	    task_def()->dump(o, 4);
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

/* Dump an expression that noone wrote a dump method for. */
void NetExpr::dump(ostream&o) const
{
      o << "(?)";
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
      if (repeat_ != 1)
	    o << repeat_;

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

void NetEScope::dump(ostream&o) const
{
      o << "<scope=" << scope_->name() << ">";
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
      o << name();
}

void NetESubSignal::dump(ostream&o) const
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
      o << "<" << scope_->name() << "." << name_ << ">";
}

void NetETernary::dump(ostream&o) const
{
      o << "(" << *cond_ << ") ? (" << *true_val_ << ") : (" <<
	    *false_val_ << ")";
}

void NetEUFunc::dump(ostream&o) const
{
      o << name() << "(";
      assert(parms_.count() > 0);
      parms_[0]->dump(o);
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    o << ", ";
	    parms_[idx]->dump(o);
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

void Design::dump(ostream&o) const
{
      o << "DESIGN TIME PRECISION: 10e" << get_precision() << endl;
      o << "SCOPES:" << endl;
      root_scope_->dump(o);

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

/*
 * $Log: design_dump.cc,v $
 * Revision 1.102  2000/11/04 06:36:24  steve
 *  Apply sequential UDP rework from Stephan Boettcher  (PR#39)
 *
 * Revision 1.101  2000/10/28 00:51:41  steve
 *  Add scope to threads in vvm, pass that scope
 *  to vpi sysTaskFunc objects, and add vpi calls
 *  to access that information.
 *
 *  $display displays scope in %m (PR#1)
 *
 * Revision 1.100  2000/10/07 19:45:42  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.99  2000/10/06 23:46:50  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.98  2000/09/26 01:35:42  steve
 *  Remove the obsolete NetEIdent class.
 *
 * Revision 1.97  2000/09/17 21:26:15  steve
 *  Add support for modulus (Eric Aardoom)
 *
 * Revision 1.96  2000/09/10 02:18:16  steve
 *  elaborate complex l-values
 *
 * Revision 1.95  2000/09/02 20:54:20  steve
 *  Rearrange NetAssign to make NetAssign_ separate.
 *
 * Revision 1.94  2000/07/30 18:25:43  steve
 *  Rearrange task and function elaboration so that the
 *  NetTaskDef and NetFuncDef functions are created during
 *  signal enaboration, and carry these objects in the
 *  NetScope class instead of the extra, useless map in
 *  the Design class.
 *
 * Revision 1.93  2000/07/29 03:55:38  steve
 *  fix problem coalescing events w/ probes.
 *
 * Revision 1.92  2000/07/27 05:13:44  steve
 *  Support elaboration of disable statements.
 *
 * Revision 1.91  2000/07/22 22:09:03  steve
 *  Parse and elaborate timescale to scopes.
 *
 * Revision 1.90  2000/07/14 06:12:57  steve
 *  Move inital value handling from NetNet to Nexus
 *  objects. This allows better propogation of inital
 *  values.
 *
 *  Clean up constant propagation  a bit to account
 *  for regs that are not really values.
 *
 * Revision 1.89  2000/07/07 04:53:53  steve
 *  Add support for non-constant delays in delay statements,
 *  Support evaluating ! in constant expressions, and
 *  move some code from netlist.cc to net_proc.cc.
 *
 * Revision 1.88  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.87  2000/06/24 22:55:19  steve
 *  Get rid of useless next_link method.
 *
 * Revision 1.86  2000/06/13 03:24:48  steve
 *  Index in memory assign should be a NetExpr.
 *
 * Revision 1.85  2000/05/11 23:37:27  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.84  2000/05/07 18:20:07  steve
 *  Import MCD support from Stephen Tell, and add
 *  system function parameter support to the IVL core.
 *
 * Revision 1.83  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.82  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.81  2000/05/02 03:13:30  steve
 *  Move memories to the NetScope object.
 *
 * Revision 1.80  2000/05/02 00:58:11  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.79  2000/04/28 21:00:29  steve
 *  Over agressive signal elimination in constant probadation.
 *
 * Revision 1.78  2000/04/23 03:45:24  steve
 *  Add support for the procedural release statement.
 *
 * Revision 1.77  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.76  2000/04/12 20:02:52  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 */

