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
#ident "$Id: pform_dump.cc,v 1.5 1998/11/23 00:20:23 steve Exp $"
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
	  case 'n':
	    out << "!=";
	    break;
	  case 'N':
	    out << "!==";
	    break;
	  default:
	    out << op_;
	    break;
      }
      out << "(" << *right_ << ")";
}


void PWire::dump(ostream&out) const
{
      switch (type) {
	  case NetNet::IMPLICIT:
	    out << "    implicit wire ";
	    break;
	  case NetNet::WIRE:
	    out << "    wire ";
	    break;
	  case NetNet::REG:
	    out << "    reg ";
	    break;
      }

      switch (port_type) {
	  case NetNet::PIMPLICIT:
	    out << "(implicit input) ";
	    break;
	  case NetNet::PINPUT:
	    out << "(input) ";
	    break;
	  case NetNet::POUTPUT:
	    out << "(output) ";
	    break;
	  case NetNet::PINOUT:
	    out << "(input output) ";
	    break;
	  case NetNet::NOT_A_PORT:
	    break;
      }

      if (lsb || msb) {
	    assert(lsb && msb);
	    out << "[" << *msb << ":" << *lsb << "] ";
      }

      out << name << ";" << endl;
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
	  case PGBuiltin::NAND:
	    out << "    nand #";
	    break;
	  default:
	    out << "    builtin gate #";
      }

      out << get_delay() << " " << get_name() << "(";
      dump_pins(out);
      out << ");" << endl;
}

void PGModule::dump(ostream&out) const
{
      out << "    " << type_ << " " << get_name() << "(";
      dump_pins(out);
      out << ");" << endl;
}

void Statement::dump(ostream&out, unsigned ind) const
{
	/* I give up. I don't know what type this statement is,
	   so just print the C++ typeid and let the user figure
	   it out. */
      out << setw(ind) << "";
      out << "/* " << typeid(*this) .name() << " */ ;" << endl;
}

void PAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << to_name_ << " = " << *expr_ << ";" << endl;
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

      if (nparms_) {
	    out << "(";
	    if (parms_[0])
		  out << *parms_[0];

	    for (unsigned idx = 1 ;  idx < nparms_ ;  idx += 1) {
		  out << ", ";
		  if (parms_[idx])
			out << *parms_[idx];
	    }
	    out << ")";
      }

      out << ";" << endl;
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
      out << setw(ind) << "" << "#" << *delay_ << endl;
      statement_->dump(out, ind+2);
}

void PEventStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "@(";
      switch (type_) {
	  case NetPEvent::ANYEDGE:
	    break;
	  case NetPEvent::POSEDGE:
	    out << "posedge ";
	    break;
	  case NetPEvent::NEGEDGE:
	    out << "negedge ";
	    break;
	  case NetPEvent::POSITIVE:
	    out << "positive ";
	    break;
      }
      out << *expr_ << ")" << endl;

      statement_->dump(out, ind+2);
}

void PForStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "for (" << name1_ << " = " << *expr1_
	  << "; " << *cond_ << "; " << name2_ << " = " << *expr2_ <<
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
	    out << setw(ind) << "" << "initial" << endl;
	    break;
	  case PProcess::PR_ALWAYS:
	    out << setw(ind) << "" << "always" << endl;
	    break;
      }

      statement_->dump(out, ind+2);
}

void pform_dump(ostream&out, Module*mod)
{
      out << "module " << mod->get_name() << ";" << endl;

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


/*
 * $Log: pform_dump.cc,v $
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

