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
#ident "$Id: design_dump.cc,v 1.68 2000/03/08 04:36:53 steve Exp $"
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

/* Dump a net. This can be a wire or register. */
void NetNet::dump_net(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << type() << ": " << name() << "[" <<
	    pin_count() << "]";
      if (local_flag_)
	    o << " (local)";
      if (scope_)
	    o << " scope=" << scope_->name();
      o << " #(" << rise_time() << "," << fall_time() << "," <<
	    decay_time() << ") init=";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << ivalue_[idx-1];
      o << endl;
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
	    o << ":";

	    unsigned cpin;
	    const NetObj*cur;
	    for (pin(idx).next_link(cur, cpin)
		       ; (cur != this) || (cpin != idx)
		       ; cur->pin(cpin).next_link(cur, cpin)) {

		  const NetNet*sig = dynamic_cast<const NetNet*>(cur);
		  if (sig) o << " " << sig->name() << "[" << cpin << "]";
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

void NetAssign::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Procedural assign (NetAssign): " << name();
      if (bmux())
	    o << "[" << *bmux() << "]";
      o << " = " << *rval() << endl;
      dump_node_pins(o, ind+4);
}

void NetAssignNB::dump_node(ostream&o, unsigned ind) const
{
      if (bmux())
	    o << setw(ind) << "" << "Procedural NB assign (NetAssignNB): "
	      << name() << "[" << *bmux() << "] <= " << *rval() << endl;
      else
	    o << setw(ind) << "" << "Procedural NB assign (NetAssignNB): "
	      << name() << " <= " << *rval() << endl;
      dump_node_pins(o, ind+4);
}

void NetBUFZ::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "NetBUFZ: " << name() << endl;
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
	<< endl;

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
      o << setw(ind) << "" << "Combinational UDP: ";
      o << " #(" << rise_time() << "," << fall_time() << "," << decay_time() <<
	    ") " << name() << endl;

      for (CM_::const_iterator ent = cm_.begin()
		 ; ent != cm_.end() ;  ent++) {
	    o << setw(ind+6) << "" << (*ent).first << " --> " <<
		  (*ent).second << endl;
      }

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUDP::dump_node(ostream&o, unsigned ind) const
{
      if (sequential_)
	    dump_sequ_(o, ind);
      else
	    dump_comb_(o, ind);
}

void NetNEvent::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "event: ";
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
	  case POSITIVE:
	    o << "positive ";
	    break;
      }

      o << name() << " --> " << fore_ptr()->name() << endl;

      dump_node_pins(o, ind+4);
}

void NetProcTop::dump(ostream&o, unsigned ind) const
{
      switch (type_) {
	  case NetProcTop::KINITIAL:
	    o << "initial  /* " << get_line() << " */" << endl;
	    break;
	  case NetProcTop::KALWAYS:
	    o << "always  /* " << get_line() << " */" << endl;
	    break;
      }

      statement_->dump(o, ind+2);
}

/* Dump an assignment statement */
void NetAssign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";

      if (bmux()) {
	    o << name() << "[" << *bmux() << "] = ";
	    if (rise_time())
		  o << "#" << rise_time() << " ";
	    o << *rval() << ";" << endl;

      } else {
	    o << name() << " = ";
	    if (rise_time())
		  o << "#" << rise_time() << " ";
	    o << *rval() << ";" << endl;
      }
}

void NetAssignNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";

      if (bmux()) {
	    o << name() << "[" << *bmux() << "] <= ";
	    if (rise_time())
		  o << "#" << rise_time() << " ";
	    o << *rval() << ";" << endl;

      } else {
	    o << name() << " <= ";
	    if (rise_time())
		  o << "#" << rise_time() << " ";
	    o << *rval() << ";" << endl;
      }
}

void NetAssignMem::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      o << "/* " << get_line() << " */" << endl;
      o << setw(ind) << "";
      o << memory()->name() << "[" << index()->name() << "] = ";
      rval()->dump(o);
      o << ";" << endl;
}

void NetAssignMemNB::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";
      o << "/* " << get_line() << " */" << endl;
      o << setw(ind) << "";
      o << memory()->name() << "[" << index()->name() << "] <= ";
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
      o << setw(ind) << "" << "#" << delay_;
      if (statement_) {
	    o << endl;
	    statement_->dump(o, ind+2);
      } else {
	    o << " /* noop */;" << endl;
      }
}

void NetPEvent::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "@(";
      svector<const NetNEvent*>*list = back_list();
      (*list)[0]->dump_proc(o);
      for (unsigned idx = 1 ;  idx < list->count() ;  idx += 1) {
	    o << " or ";
	    (*list)[idx]->dump_proc(o);
      }
      delete list;
      o << ") /* " << name_ << " */";

      if (statement_) {
	    o << endl;
	    statement_->dump(o, ind+2);
      } else {
	    o << " /* noop */;" << endl;
      }
}

void NetNEvent::dump_proc(ostream&o) const
{
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
	  case POSITIVE:
	    o << "positive ";
	    break;
      }
      o << name();
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

	/* Dump the parameters for this scope. */
      {
	    map<string,NetExpr*>::const_iterator pp;
	    for (pp = parameters_.begin()
		       ; pp != parameters_.end() ;  pp ++) {
		  o << "    parameter " << (*pp).first << " = "  <<
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

void NetEIdent::dump(ostream&o) const
{
      o << name_;
}

void NetEScope::dump(ostream&o) const
{
      o << "<scope=" << scope_->name() << ">";
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
      o << "SCOPES:" << endl;
      root_scope_->dump(o);

      o << "ELABORATED SIGNALS:" << endl;

	// Dump the signals,
      if (signals_) {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  cur->dump_net(o, 0);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }

      o << "ELABORATED MEMORIES:" << endl;
      {
	    map<string,NetMemory*>::const_iterator pp;
	    for (pp = memories_.begin()
		       ; pp != memories_.end() ; pp ++) {
		  (*pp).second->dump(o, 0);
	    }
      }

      o << "ELABORATED FUNCTION DEFINITIONS:" << endl;
      {
	    map<string,NetFuncDef*>::const_iterator pp;
	    for (pp = funcs_.begin()
		       ; pp != funcs_.end() ; pp ++) {
		  (*pp).second->dump(o, 0);
	    }
      }

      o << "ELABORATED TASK DEFINITIONS:" << endl;
      {
	    map<string,NetTaskDef*>::const_iterator pp;
	    for (pp = tasks_.begin()
		       ; pp != tasks_.end() ; pp ++) {
		  (*pp).second->dump(o, 0);
	    }
      }

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
 * Revision 1.68  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.67  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.66  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.65  2000/01/10 01:35:23  steve
 *  Elaborate parameters afer binding of overrides.
 *
 * Revision 1.64  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 *
 * Revision 1.63  1999/12/12 06:03:14  steve
 *  Allow memories without indices in expressions.
 *
 * Revision 1.62  1999/12/05 02:24:08  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 *
 * Revision 1.61  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.60  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.59  1999/11/24 04:01:58  steve
 *  Detect and list scope names.
 *
 * Revision 1.58  1999/11/21 00:13:08  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.57  1999/11/14 23:43:45  steve
 *  Support combinatorial comparators.
 *
 * Revision 1.56  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.55  1999/11/04 03:53:26  steve
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
 * Revision 1.54  1999/11/04 01:12:41  steve
 *  Elaborate combinational UDP devices.
 *
 * Revision 1.53  1999/11/01 02:07:40  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 * Revision 1.52  1999/10/31 20:08:24  steve
 *  Include subtraction in LPM_ADD_SUB device.
 *
 * Revision 1.51  1999/10/31 04:11:27  steve
 *  Add to netlist links pin name and instance number,
 *  and arrange in vvm for pin connections by name
 *  and instance number.
 *
 * Revision 1.50  1999/10/10 01:59:54  steve
 *  Structural case equals device.
 *
 * Revision 1.49  1999/10/08 02:00:35  steve
 *  Fix dump of sase statements.
 *
 * Revision 1.48  1999/10/07 05:25:33  steve
 *  Add non-const bit select in l-value of assignment.
 *
 * Revision 1.47  1999/10/06 05:06:16  steve
 *  Move the rvalue into NetAssign_ common code.
 *
 * Revision 1.46  1999/10/05 06:19:46  steve
 *  Add support for reduction NOR.
 *
 * Revision 1.45  1999/09/30 02:43:01  steve
 *  Elaborate ~^ and ~| operators.
 *
 * Revision 1.44  1999/09/29 18:36:03  steve
 *  Full case support
 *
 * Revision 1.43  1999/09/21 00:13:40  steve
 *  Support parameters that reference other paramters.
 *
 * Revision 1.42  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 * Revision 1.41  1999/09/19 01:06:36  steve
 *  dump the repeat count, if applicable.
 *
 * Revision 1.40  1999/09/15 01:55:06  steve
 *  Elaborate non-blocking assignment to memories.
 *
 * Revision 1.39  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.38  1999/09/03 04:28:38  steve
 *  elaborate the binary plus operator.
 *
 * Revision 1.37  1999/09/01 20:46:19  steve
 *  Handle recursive functions and arbitrary function
 *  references to other functions, properly pass
 *  function parameters and save function results.
 *
 * Revision 1.36  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.35  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.34  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.33  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.32  1999/07/17 19:50:59  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.31  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.30  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.29  1999/06/15 05:38:15  steve
 *  Handle total lack of signals or nodes.
 *
 * Revision 1.28  1999/06/09 03:00:05  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.27  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.26  1999/05/31 15:46:20  steve
 *  Compilation warning.
 *
 * Revision 1.25  1999/05/30 01:11:46  steve
 *  Exressions are trees that can duplicate, and not DAGS.
 *
 * Revision 1.24  1999/05/17 04:53:47  steve
 *  translate the letter synonyms for operators.
 *
 * Revision 1.23  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.22  1999/05/06 02:29:32  steve
 *  Excesss endl.
 *
 * Revision 1.21  1999/05/05 03:04:46  steve
 *  Fix handling of null delay statements.
 *
 * Revision 1.20  1999/05/01 20:43:55  steve
 *  Handle wide events, such as @(a) where a has
 *  many bits in it.
 *
 *  Add to vvm the binary ^ and unary & operators.
 *
 *  Dump events a bit more completely.
 *
 * Revision 1.19  1999/05/01 02:57:52  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.18  1999/04/25 00:44:10  steve
 *  Core handles subsignal expressions.
 *
 * Revision 1.17  1999/04/19 01:59:36  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.16  1999/03/15 02:43:32  steve
 *  Support more operators, especially logical.
 *
 * Revision 1.15  1999/03/01 03:27:53  steve
 *  Prevent the duplicate allocation of ESignal objects.
 *
 * Revision 1.14  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.13  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.12  1999/02/08 02:49:56  steve
 *  Turn the NetESignal into a NetNode so
 *  that it can connect to the netlist.
 *  Implement the case statement.
 *  Convince t-vvm to output code for
 *  the case statement.
 *
 * Revision 1.11  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
 * Revision 1.10  1999/02/01 00:26:48  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.9  1998/12/20 02:05:41  steve
 *  Function to calculate wire initial value.
 *
 * Revision 1.8  1998/12/14 02:01:34  steve
 *  Fully elaborate Sequential UDP behavior.
 *
 * Revision 1.7  1998/12/07 04:53:17  steve
 *  Generate OBUF or IBUF attributes (and the gates
 *  to garry them) where a wire is a pad. This involved
 *  figuring out enough of the netlist to know when such
 *  was needed, and to generate new gates and signales
 *  to handle what's missing.
 *
 * Revision 1.6  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful will wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.5  1998/12/01 00:42:13  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.4  1998/11/23 00:20:22  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.3  1998/11/09 18:55:34  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:56  steve
 *  Introduce verilog to CVS.
 *
 */

