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
#ident "$Id: pform_dump.cc,v 1.38 1999/09/08 02:24:39 steve Exp $"
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

ostream& operator << (ostream&o, const PDelays&d)
{
      d.dump_delays(o);
      return o;
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

void PECallFunction::dump(ostream &out) const
{
      out << name_ << "(";
      parms_[0]->dump(out);
      for (unsigned idx = 1; idx < parms_.count(); ++idx) {
	    out << ", ";
	    parms_[idx]->dump(out);
      }
      out << ")";
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

void PETernary::dump(ostream&out) const
{
      out << "(" << *expr_ << ")?(" << *tru_ << "):(" << *fal_ << ")";
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
      out << "    " << type_;

      switch (port_type_) {
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

      for (unsigned idx = 0 ;  idx < msb_.count() ;  idx += 1) {
	    assert(lsb_[idx] && msb_[idx]);
	    out << " [" << *msb_[idx] << ":" << *lsb_[idx] << "]";
      }

      out << " " << name_;

	// If the wire has indices, dump them.
      if (lidx_ || ridx_) {
	    out << "[";
	    if (lidx_) out << *lidx_;
	    if (ridx_) out << ":" << *ridx_;
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

void PDelays::dump_delays(ostream&out) const
{
      if (delay_[0] && delay_[1] && delay_[2])
	    out << "#(" << *delay_[0] << "," << *delay_[1] << "," <<
		  *delay_[2] << ")";
      else if (delay_[0])
	    out << "#" << *delay_[0];
      else
	    out << "#0";

}

void PGate::dump_delays(ostream&out) const
{
      delay_.dump_delays(out);
}

void PGate::dump(ostream&out) const
{
      out << "    " << typeid(*this).name() << " ";
      delay_.dump_delays(out);
      out << " " << get_name() << "(";
      dump_pins(out);
      out << ");" << endl;

}

void PGAssign::dump(ostream&out) const
{
      out << "    assign ";
      dump_delays(out);
      out << " " << *pin(0) << " = " << *pin(1) << ";" << endl;
}

void PGBuiltin::dump(ostream&out) const
{
      switch (type()) {
	  case PGBuiltin::BUFIF0:
	    out << "    bufif0 ";
	    break;
	  case PGBuiltin::BUFIF1:
	    out << "    bufif1 ";
	    break;
	  case PGBuiltin::NAND:
	    out << "    nand ";
	    break;
	  default:
	    out << "    builtin gate ";
      }

      dump_delays(out);
      out << " " << get_name();

      if (msb_) {
	    out << " [" << *msb_ << ":" << *lsb_ << "]";
      }

      out << "(";
      dump_pins(out);
      out << ");" << endl;
}

void PGModule::dump(ostream&out) const
{
      out << "    " << type_ << " ";
      if (overrides_) {
            out << "#(";
	    out << *((*overrides_)[0]);
	    for (unsigned idx = 1 ;  idx < overrides_->count() ;  idx += 1) {
	          out << "," << *((*overrides_)[idx]);
	    }
	    out << ") ";
      }
      out << get_name() << "(";
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
      out << *lval() << " = " << delay_ << " " << *rval() << ";";
      out << "  /* " << get_line() << " */" << endl;
}

void PAssignNB::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << *lval() << " <= " << delay_ << " " << *rval() << ";";
      out << "  /* " << get_line() << " */" << endl;
}

void PBlock::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "begin";
      if (name_.length())
	    out << " : " << name_;
      out << endl;

      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1) {
	    list_[idx]->dump(out, ind+2);
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

      out << "; /* " << get_line() << " */" << endl;
}

void PCase::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "case (" << *expr_ << ") /* " <<
	    get_line() << " */" << endl;

      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.count() == 0) {
		  out << setw(ind+2) << "" << "default:";

	    } else {
		  out << setw(ind+2) << "" << *cur->expr[0];
			
		  for(unsigned e = 1 ; e < cur->expr.count() ; e += 1)
			out << ", " << *cur->expr[e];

		  out << ":";
	    }

	    if (cur->stat) {
		  out << endl;
		  cur->stat->dump(out, ind+6);
	    } else {
		  out << " ;" << endl;
	    }
      }

      out << setw(ind) << "" << "endcase" << endl;
}

void PCondit::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "if (" << *expr_ << ")" << endl;
      if (if_)
	    if_->dump(out, ind+3);
      else
	    out << setw(ind) << ";" << endl;
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

void PForever::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "forever /* " << get_line() << " */" << endl;
      statement_->dump(out, ind+3);
}

void PForStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "for (" << *name1_ << " = " << *expr1_
	  << "; " << *cond_ << "; " << *name2_ << " = " << *expr2_ <<
	    ")" << endl;
      statement_->dump(out, ind+3);
}

void PFunction::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "output " << out_->name() << ";" << endl;
      for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {
	    out << setw(ind) << "";
	    out << "input ";
	    out << (*ports_)[idx]->name() << ";" << endl;
      }

      statement_->dump(out, ind);
}

void PRepeat::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "repeat (" << *expr_ << ")" << endl;
      statement_->dump(out, ind+3);
}

void PTask::dump(ostream&out, unsigned ind) const
{
      if (ports_)
	    for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {
		  out << setw(ind) << "";
		  switch ((*ports_)[idx]->get_port_type()) {
		      case NetNet::PINPUT:
			out << "input ";
			break;
		      case NetNet::POUTPUT:
			out << "output ";
			break;
		      case NetNet::PINOUT:
			out << "inout ";
			break;
		  }
		  out << (*ports_)[idx]->name() << ";" << endl;
	    }

      statement_->dump(out, ind);
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

void Module::dump(ostream&out) const
{
      out << "module " << name_ << ";" << endl;

      for (unsigned idx = 0 ;  idx < ports_.count() ;  idx += 1) {
	    port_t*cur = ports_[idx];
	    switch (cur->wires[0]->get_port_type()) {
		case NetNet::PINPUT:
		  out << "    input ." << cur->name << "(";
		  break;
		case NetNet::POUTPUT:
		  out << "    output ." << cur->name << "(";
		  break;
		case NetNet::PINOUT:
		  out << "    inout ." << cur->name << "(";
		  break;
		default:
		  out << "    XXXX ." << cur->name << "(";
		  break;
	    }

	    out << cur->wires[0]->name();
	    for (unsigned wdx = 1 ;  wdx < cur->wires.count() ;  wdx += 1) {
		  out << ", " << cur->wires[wdx]->name();
	    }

	    out << ")" << endl;
      }

      typedef map<string,PExpr*>::const_iterator parm_iter_t;
      for (parm_iter_t cur = parameters.begin()
		 ; cur != parameters.end() ; cur ++) {
	    out << "    parameter " << (*cur).first << " = " <<
		  *(*cur).second << ";" << endl;
      }

	// Iterate through and display all the wires.
      for (list<PWire*>::const_iterator wire = wires_.begin()
		 ; wire != wires_.end()
		 ; wire ++ ) {

	    (*wire)->dump(out);
      }

	// Dump the task definitions.
      typedef map<string,PTask*>::const_iterator task_iter_t;
      for (task_iter_t cur = tasks_.begin()
		 ; cur != tasks_.end() ; cur ++) {
	    out << "    task " << (*cur).first << ";" << endl;
	    (*cur).second->dump(out, 6);
	    out << "    endtask;" << endl;
      }

	// Dump the function definitions.
      typedef map<string,PFunction*>::const_iterator func_iter_t;
      for (func_iter_t cur = funcs_.begin()
		 ; cur != funcs_.end() ; cur ++) {
	    out << "    function " << (*cur).first << ";" << endl;
	    (*cur).second->dump(out, 6);
	    out << "    endfunction;" << endl;
      }


	// Iterate through and display all the gates
      for (list<PGate*>::const_iterator gate = gates_.begin()
		 ; gate != gates_.end()
		 ; gate ++ ) {

	    (*gate)->dump(out);
      }


      for (list<PProcess*>::const_iterator behav = behaviors_.begin()
		 ; behav != behaviors_.end()
		 ; behav ++ ) {

	    (*behav)->dump(out, 4);
      }

      out << "endmodule" << endl;
}

void pform_dump(ostream&out, Module*mod)
{
      mod->dump(out);
}

void PUdp::dump(ostream&out) const
{
      out << "primitive " << name_ << "(" << ports[0];
      for (unsigned idx = 1 ;  idx < ports.count() ;  idx += 1)
	    out << ", " << ports[idx];
      out << ");" << endl;

      if (sequential)
	    out << "    reg " << ports[0] << ";" << endl;

      out << "    table" << endl;
      for (unsigned idx = 0 ;  idx < tinput.count() ;  idx += 1) {
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
 * Revision 1.38  1999/09/08 02:24:39  steve
 *  Empty conditionals (pmonta@imedia.com)
 *
 * Revision 1.37  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.36  1999/08/25 22:22:41  steve
 *  elaborate some aspects of functions.
 *
 * Revision 1.35  1999/08/23 16:48:39  steve
 *  Parameter overrides support from Peter Monta
 *  AND and XOR support wide expressions.
 *
 * Revision 1.34  1999/08/03 04:49:13  steve
 *  Proper port type names.
 *
 * Revision 1.33  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.32  1999/08/01 16:34:50  steve
 *  Parse and elaborate rise/fall/decay times
 *  for gates, and handle the rules for partial
 *  lists of times.
 *
 * Revision 1.31  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.30  1999/07/30 00:43:17  steve
 *  Handle dumping tasks with no ports.
 *
 * Revision 1.29  1999/07/24 02:11:20  steve
 *  Elaborate task input ports.
 *
 * Revision 1.28  1999/07/17 19:51:00  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.27  1999/07/12 00:59:36  steve
 *  procedural blocking assignment delays.
 *
 * Revision 1.26  1999/07/03 02:12:52  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.25  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.24  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.23  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 * Revision 1.22  1999/06/15 05:38:39  steve
 *  Support case expression lists.
 *
 * Revision 1.21  1999/06/15 03:44:53  steve
 *  Get rid of the STL vector template.
 *
 * Revision 1.20  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
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
 */

