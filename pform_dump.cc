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
#ident "$Id: pform_dump.cc,v 1.19 1999/06/10 04:03:53 steve Exp $"
#endif

/*
 * This file provides the pform_dump function, that dumps the module
 * passed as a parameter. The dump is as much as possible in Verilog
 * syntax, so that a human can tell that it really does describe the
 * module in question.
 */
# include  "pform.h"
# include  <iostream>
# include  <iomanip>
# include  <typeinfo>

ostream& operator << (ostream&out, const PExpr&obj)
{
      obj.dump(out);
      return out;
}

void PExpr::dump(ostream&out) const
{
      out << typeid(*this).name();
}

void PEConcat::dump(ostream&out) const
{
      if (repeat_)
	    out << "{" << *repeat_;

      if (parms_.count() == 0) {
	    out << "{}";
	    return;
      }

      out << "{" << *parms_[0];
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1)
	    out << ", " << *parms_[idx];

      out << "}";

      if (repeat_) out << "}";
}

void PEEvent::dump(ostream&out) const
{
      switch (type_) {
	  case NetNEvent::ANYEDGE:
	    break;
	  case NetNEvent::POSEDGE:
	    out << "posedge ";
	    break;
	  case NetNEvent::NEGEDGE:
	    out << "negedge ";
	    break;
	  case NetNEvent::POSITIVE:
	    out << "positive ";
	    break;
      }
      out << *expr_;
}

void PENumber::dump(ostream&out) const
{
      out << value();
}

void PEIdent::dump(ostream&out) const
{
      out << text_;
      if (msb_) {
	    out << "[" << *msb_;
	    if (lsb_) {
		  out << ":" << *lsb_;
	    }
	    out << "]";
      }
}

void PEString::dump(ostream&out) const
{
      out << "\"" << text_ << "\"";
}

void PEUnary::dump(ostream&out) const
{
      out << op_ << "(" << *expr_ << ")";
}

void PEBinary::dump(ostream&out) const
{
      out << "(" << *left_ << ")";
      switch (op_) {
	  case 'e':
	    out << "==";
	    break;
	  case 'E':
	    out << "===";
	    break;
	  case 'l':
	    out << "<<";
	    break;
	  case 'n':
	    out << "!=";
	    break;
	  case 'N':
	    out << "!==";
	    break;
	  case 'r':
	    out << ">>";
	    break;
	  default:
	    out << op_;
	    break;
      }
      out << "(" << *right_ << ")";
}


void PWire::dump(ostream&out) const
{
      out << "    " << type;

      switch (port_type) {
	  case NetNet::PIMPLICIT:
	    out << " (implicit input)";
	    break;
	  case NetNet::PINPUT:
	    out << " (input)";
	    break;
	  case NetNet::POUTPUT:
	    out << " (output)";
	    break;
	  case NetNet::PINOUT:
	    out << " (input output)";
	    break;
	  case NetNet::NOT_A_PORT:
	    break;
      }

      if (lsb || msb) {
	    assert(lsb && msb);
	    out << " [" << *msb << ":" << *lsb << "]";
      }

      out << " " << name;

	// If the wire has indices, dump them.
      if (lidx || ridx) {
	    out << "[";
	    if (lidx) out << *lidx;
	    if (ridx) out << ":" << *ridx;
	    out << "]";
      }

      out << ";" << endl;
      for (map<string,string>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end()
		 ; idx ++) {
	    out << "        " << (*idx).first << " = \"" <<
		  (*idx).second << "\"" << endl;
      }
}

void PGate::dump_pins(ostream&out) const
{
      if (pin_count()) {
	    out << *pin(0);
			      
	    for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {
		  out << ", ";
		  if (pin(idx)) out << *pin(idx);
	    }
      }
}

void PGate::dump(ostream&out) const
{
      out << "    " << typeid(*this).name() << " #"
	  << get_delay() << " " << get_name() << "(";
      dump_pins(out);
      out << ");" << endl;

}

void PGAssign::dump(ostream&out) const
{
      out << "    assign " << *pin(0) << " = " << *pin(1) << ";" << endl;
}

void PGBuiltin::dump(ostream&out) const
{
      switch (type()) {
	  case PGBuiltin::BUFIF0:
	    out << "    bufif0 #";
	    break;
	  case PGBuiltin::BUFIF1:
	    out << "    bufif1 #";
	    break;
	  case PGBuiltin::NAND:
	    out << "    nand #";
	    break;
	  default:
	    out << "    builtin gate #";
      }

      out << get_delay() << " " << get_name();

      if (msb_) {
	    out << " [" << *msb_ << ":" << *lsb_ << "]";
      }

      out << "(";
      dump_pins(out);
      out << ");" << endl;
}

void PGModule::dump(ostream&out) const
{
      out << "    " << type_ << " " << get_name() << "(";
      if (pins_) {
	    out << "." << pins_[0].name << "(" << *pins_[0].parm << ")";
	    for (unsigned idx = 1 ;  idx < npins_ ;  idx += 1) {
		  out << ", ." << pins_[idx].name << "(" <<
			*pins_[idx].parm << ")";
	    }
      } else {
	    dump_pins(out);
      }
      out << ");" << endl;
}

void Statement::dump(ostream&out, unsigned ind) const
{
	/* I give up. I don't know what type this statement is,
	   so just print the C++ typeid and let the user figure
	   it out. */
      out << setw(ind) << "";
      out << "/* " << get_line() << ": " << typeid(*this).name()
	  << " */ ;" << endl;
}

void PAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << *lval_ << " = " << *expr_ << ";";
      out << "  /* " << get_line() << " */" << endl;
}

void PAssignNB::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << *lval_ << " <= " << *rval_ << ";";
      out << "  /* " << get_line() << " */" << endl;
}

void PBlock::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "begin" << endl;

      for (unsigned idx = 0 ;  idx < size() ;  idx += 1) {
	    stat(idx)->dump(out, ind+2);
      }

      out << setw(ind) << "" << "end" << endl;
}

void PCallTask::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << name_;

      if (parms_.count() > 0) {
	    out << "(";
	    if (parms_[0])
		  out << *parms_[0];

	    for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
		  out << ", ";
		  if (parms_[idx])
			out << *parms_[idx];
	    }
	    out << ")";
      }

      out << ";" << endl;
}

void PCase::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "case (" << *expr_ << ") /* " <<
	    get_line() << " */" << endl;

      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {
	    if ((*items_)[idx]->expr)
		  out << setw(ind+2) << "" << *(*items_)[idx]->expr << ":";
	    else
		  out << setw(ind+2) << "" << "default:";

	    if ((*items_)[idx]->stat) {
		  out << endl;
		  (*items_)[idx]->stat->dump(out, ind+6);
	    } else {
		  out << " ;" << endl;
	    }
      }

      out << setw(ind) << "" << "endcase" << endl;
}

void PCondit::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "if (" << *expr_ << ")" << endl;
      if_->dump(out, ind+3);
      if (else_) {
	    out << setw(ind) << "" << "else" << endl;
	    else_->dump(out, ind+3);
      }
}

void PDelayStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "#" << *delay_ << " /* " <<
	    get_line() << " */";
      if (statement_) {
	    out << endl;
	    statement_->dump(out, ind+2);
      } else {
	    out << " /* noop */;" << endl;
      }
}

void PEventStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "@(" << *(expr_[0]);
      if (expr_.count() > 1)
	    for (unsigned idx = 1 ;  idx < expr_.count() ;  idx += 1)
		  out << " or " << *(expr_[idx]);

      out << ")";


      if (statement_) {
	    out << endl;
	    statement_->dump(out, ind+2);
      } else {
	    out << " ;" << endl;
      }
}

void PForStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "for (" << *name1_ << " = " << *expr1_
	  << "; " << *cond_ << "; " << *name2_ << " = " << *expr2_ <<
	    ")" << endl;
      statement_->dump(out, ind+3);
}

void PWhile::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "while (" << *cond_ << ")" << endl;
      statement_->dump(out, ind+3);
}

void PProcess::dump(ostream&out, unsigned ind) const
{
      switch (type_) {
	  case PProcess::PR_INITIAL:
	    out << setw(ind) << "" << "initial";
	    break;
	  case PProcess::PR_ALWAYS:
	    out << setw(ind) << "" << "always";
	    break;
      }

      out << " /* " << get_line() << " */" << endl;

      statement_->dump(out, ind+2);
}

void pform_dump(ostream&out, Module*mod)
{
      out << "module " << mod->get_name() << ";" << endl;

      typedef map<string,PExpr*>::const_iterator parm_iter_t;
      for (parm_iter_t cur = mod->parameters.begin()
		 ; cur != mod->parameters.end() ; cur ++) {
	    out << "    parameter " << (*cur).first << " = " <<
		  *(*cur).second << ";" << endl;
      }

	// Iterate through and display all the wires.
      const list<PWire*>&wires = mod->get_wires();
      for (list<PWire*>::const_iterator wire = wires.begin()
		 ; wire != wires.end()
		 ; wire ++ ) {

	    (*wire)->dump(out);
      }


	// Iterate through and display all the gates
      const list<PGate*>&gates = mod->get_gates();
      for (list<PGate*>::const_iterator gate = gates.begin()
		 ; gate != gates.end()
		 ; gate ++ ) {

	    (*gate)->dump(out);
      }


      const list<PProcess*>&behaves = mod->get_behaviors();
      for (list<PProcess*>::const_iterator behav = behaves.begin()
		 ; behav != behaves.end()
		 ; behav ++ ) {

	    (*behav)->dump(out, 4);
      }

      out << "endmodule" << endl;
}

void PUdp::dump(ostream&out) const
{
      out << "primitive " << name_ << "(" << ports[0];
      for (unsigned idx = 1 ;  idx < ports.size() ;  idx += 1)
	    out << ", " << ports[idx];
      out << ");" << endl;

      if (sequential)
	    out << "    reg " << ports[0] << ";" << endl;

      out << "    table" << endl;
      for (unsigned idx = 0 ;  idx < tinput.size() ;  idx += 1) {
	    out << "     ";
	    for (unsigned chr = 0 ;  chr < tinput[idx].length() ;  chr += 1)
		  out << " " << tinput[idx][chr];

	    if (sequential)
		  out << " : " << tcurrent[idx];

	    out << " : " << toutput[idx] << " ;" << endl;
      }
      out << "    endtable" << endl;

      if (sequential)
	    out << "    initial " << ports[0] << " = 1'b" << initial
		<< ";" << endl;

      out << "endprimitive" << endl;

	// Dump the attributes for the primitive as attribute
	// statements.
      for (map<string,string>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end()
		 ; idx ++) {
	    out << "$attribute(" << name_ << ", \"" << (*idx).first <<
		  "\", \"" << (*idx).second << "\")" << endl;
      }
}


/*
 * $Log: pform_dump.cc,v $
 * Revision 1.19  1999/06/10 04:03:53  steve
 *  Add support for the Ternary operator,
 *  Add support for repeat concatenation,
 *  Correct some seg faults cause by elaboration
 *  errors,
 *  Parse the casex anc casez statements.
 *
 * Revision 1.18  1999/06/06 20:45:39  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.17  1999/05/29 02:36:17  steve
 *  module parameter bind by name.
 *
 * Revision 1.16  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.15  1999/05/05 03:04:46  steve
 *  Fix handling of null delay statements.
 *
 * Revision 1.14  1999/05/01 02:57:53  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.13  1999/04/29 02:16:26  steve
 *  Parse OR of event expressions.
 *
 * Revision 1.12  1999/04/19 01:59:37  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.11  1999/02/21 17:01:57  steve
 *  Add support for module parameters.
 *
 * Revision 1.10  1999/02/15 02:06:15  steve
 *  Elaborate gate ranges.
 *
 * Revision 1.9  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
 * Revision 1.8  1999/02/01 00:26:49  steve
 *  Carry some line info to the netlist,
 *  Dump line numbers for processes.
 *  Elaborate prints errors about port vector
 *  width mismatch
 *  Emit better handles null statements.
 *
 * Revision 1.7  1998/12/01 00:42:14  steve
 *  Elaborate UDP devices,
 *  Support UDP type attributes, and
 *  pass those attributes to nodes that
 *  are instantiated by elaboration,
 *  Put modules into a map instead of
 *  a simple list.
 *
 * Revision 1.6  1998/11/25 02:35:54  steve
 *  Parse UDP primitives all the way to pform.
 *
 * Revision 1.5  1998/11/23 00:20:23  steve
 *  NetAssign handles lvalues as pin links
 *  instead of a signal pointer,
 *  Wire attributes added,
 *  Ability to parse UDP descriptions added,
 *  XNF generates EXT records for signals with
 *  the PAD attribute.
 *
 * Revision 1.4  1998/11/11 03:13:04  steve
 *  Handle while loops.
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
 * Revision 1.1  1998/11/03 23:29:04  steve
 *  Introduce verilog to CVS.
 *
 */

