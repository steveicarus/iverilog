/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: design_dump.cc,v 1.9 1998/12/20 02:05:41 steve Exp $"
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
      o << " #(" << delay1() << "," << delay2() << "," << delay3() <<
	    ") init=";
      for (unsigned idx = pin_count() ;  idx > 0 ;  idx -= 1)
	    o << ivalue_[idx-1];
      o << endl;
      dump_obj_attr(o, ind+4);
}


/* Dump a NetNode and its pins. Dump what I know about the netnode on
   the first line, then list all the pins, with the name of the
   connected signal. */
void NetNode::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "node: ";
      o << typeid(*this).name() << " #(" << delay1()
	<< "," << delay2() << "," << delay3() << ") " << name()
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
	    o << setw(ind) << "" << idx;
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

void NetAssign::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "Procedural assign: " << *rval_ << endl;
      dump_node_pins(o, ind+4);
}

void NetBUFZ::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "BUFZ: " << name() << endl;
      dump_node_pins(o, ind+4);
}

void NetConst::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "constant " << value_ << ": " << name() << endl;
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
      o << " #(" << delay1()
	<< "," << delay2() << "," << delay3() << ") " << name()
	<< endl;

      dump_node_pins(o, ind+4);
      dump_obj_attr(o, ind+4);
}

void NetUDP::dump_sequ_(ostream&o, unsigned ind) const
{
      string tmp = "";
      for (unsigned idx = 0 ;  idx < ind ;  idx += 1)
	    tmp += " ";

      o << tmp << "Sequential UDP" << " #(" << delay1() <<
	    "," << delay2() << "," << delay3() << ") " << name() <<
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
      o << " #(" << delay1() << "," << delay2() << "," << delay3() <<
	    ") " << name() << endl;

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

void NetPEvent::dump_node(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "event: ";
      switch (edge_) {
	  case ANYEDGE:
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

      o << name() << endl;

      dump_node_pins(o, ind+4);
}

void NetProcTop::dump(ostream&o, unsigned ind) const
{
      switch (type_) {
	  case NetProcTop::KINITIAL:
	    o << "initial" << endl;
	    break;
	  case NetProcTop::KALWAYS:
	    o << "always" << endl;
	    break;
      }

      statement_->dump(o, ind+2);
}

/* Dump an assignment statement */
void NetAssign::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "";

      NetNet*sig;
      unsigned msb, lsb;
      find_lval_range(sig, msb, lsb);
      o << sig->name() << "[" << msb;
      if (pin_count() > 1)
	    o << ":" << lsb;
      o << "] = ";
      
      rval_->dump(o);
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

void NetCondit::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "if (";
      expr_->dump(o);
      o << ")" << endl;
      if_->dump(o, ind+4);
      if (else_) {
	    o << setw(ind) << "" << "else" << endl;
	    else_->dump(o, ind+4);
      }
}

void NetPDelay::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << "#" << delay_ << endl;
      statement_->dump(o, ind+2);
}

void NetPEvent::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" ;
      switch (edge_) {
	  case NEGEDGE:
	    o << "@" << "(negedge " << name() << ")";
	    break;
	  case POSEDGE:
	    o << "@" << "(posedge " << name() << ")";
	    break;
	  case ANYEDGE:
	    o << "@" << name();
	    break;
	  case POSITIVE:
	    o << "wait (" << name() << ")";
	    break;
      }
      o << endl;

      statement_->dump(o, ind+2);
}



void NetTask::dump(ostream&o, unsigned ind) const
{
      o << setw(ind) << "" << name_;

      if (nparms_ > 0) {
	    o << "(";
	    if (parms_[0])
		  parms_[0]->dump(o);

	    for (unsigned idx = 1 ;  idx < nparms_ ;  idx += 1) {
		  o << ", ";
		  if (parms_[idx])
			parms_[idx]->dump(o);
	    }

	    o << ")";
      }
      o << ";" << endl;
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
	  case 'e':
	    o << "==";
	    break;
	  case 'n':
	    o << "!=";
	    break;
      }
      o << "(";
      right_->dump(o);
      o << ")";
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

void NetESignal::dump(ostream&o) const
{
      o << sig_->name();
}

void NetEUnary::dump(ostream&o) const
{
      o << op_ << "(";
      expr_->dump(o);
      o << ")";
}

void Design::dump(ostream&o) const
{

      o << "ELABORATED SIGNALS:" << endl;

	// Dump the signals,
      {
	    NetNet*cur = signals_->sig_next_;
	    do {
		  cur->dump_net(o, 0);
		  cur = cur->sig_next_;
	    } while (cur != signals_->sig_next_);
      }

      o << "ELABORATED NODES:" << endl;

	// dump the nodes,
      {
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

