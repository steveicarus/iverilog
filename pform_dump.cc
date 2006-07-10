/*
 * Copyright (c) 1998-2004 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: pform_dump.cc,v 1.88.2.1 2006/07/10 00:21:54 steve Exp $"
#endif

# include "config.h"

/*
 * This file provides the pform_dump function, that dumps the module
 * passed as a parameter. The dump is as much as possible in Verilog
 * syntax, so that a human can tell that it really does describe the
 * module in question.
 */
# include  "pform.h"
# include  "PData.h"
# include  "PEvent.h"
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

ostream& operator<< (ostream&o, PGate::strength_t str)
{
      switch (str) {
	  case PGate::HIGHZ:
	    o << "highz";
	    break;
	  case PGate::WEAK:
	    o << "weak";
	    break;
	  case PGate::PULL:
	    o << "pull";
	    break;
	  case PGate::STRONG:
	    o << "strong";
	    break;
	  case PGate::SUPPLY:
	    o << "supply";
	    break;
	  default:
	    assert(0);
      }
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

      out << "{";
      if (parms_[0]) out << *parms_[0];
      for (unsigned idx = 1 ;  idx < parms_.count() ;  idx += 1) {
	    out << ", ";
	    if (parms_[idx]) out << *parms_[idx];
      }

      out << "}";

      if (repeat_) out << "}";
}

void PECallFunction::dump(ostream &out) const
{
      out << path_ << "(";

      if (parms_.count() > 0) {
	    if (parms_[0]) parms_[0]->dump(out);
	    for (unsigned idx = 1; idx < parms_.count(); ++idx) {
		  out << ", ";
		  if (parms_[idx]) parms_[idx]->dump(out);
	    }
      }
      out << ")";
}

void PEEvent::dump(ostream&out) const
{
      switch (type_) {
	  case PEEvent::ANYEDGE:
	    break;
	  case PEEvent::POSEDGE:
	    out << "posedge ";
	    break;
	  case PEEvent::NEGEDGE:
	    out << "negedge ";
	    break;
	  case PEEvent::POSITIVE:
	    out << "positive ";
	    break;
      }
      out << *expr_;

}

void PEFNumber::dump(ostream &out) const
{
      out << value();
}

void PENumber::dump(ostream&out) const
{
      out << value();
}

void PEIdent::dump(ostream&out) const
{
      out << path_;
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
	  case 'a':
	    out << "&&";
	    break;
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

      if (signed_) {
	    out << " signed";
      }

      assert(msb_.count() == lsb_.count());
      for (unsigned idx = 0 ;  idx < msb_.count() ;  idx += 1) {

	    if (msb_[idx] == 0) {
		  assert(lsb_[idx] == 0);
		  out << " <scalar>";

	    } else {
		  if (lsb_[idx])
			out << " [" << *msb_[idx] << ":" << *lsb_[idx] << "]";
		  else
			out << " [" << *msb_[idx] << "]";
	    }
      }

      out << " " << hname_;

	// If the wire has indices, dump them.
      if (lidx_ || ridx_) {
	    out << "[";
	    if (lidx_) out << *lidx_;
	    if (ridx_) out << ":" << *ridx_;
	    out << "]";
      }

      out << ";" << endl;
      for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end()
		 ; idx ++) {
	    out << "        " << (*idx).first;
	    if ((*idx).second)
		  out << " = " << *(*idx).second;
	    out << endl;
      }
}

void PGate::dump_pins(ostream&out) const
{
      if (pin_count()) {
	    if (pin(0)) out << *pin(0);

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
      out << "    assign (" << strength0() << "0 " << strength1() << "1) ";
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
	  case PGBuiltin::NOTIF0:
	    out << "    bufif0 ";
	    break;
	  case PGBuiltin::NOTIF1:
	    out << "    bufif1 ";
	    break;
	  case PGBuiltin::NAND:
	    out << "    nand ";
	    break;
	  case PGBuiltin::NMOS:
	    out << "    nmos ";
	    break;
	  case PGBuiltin::RNMOS:
	    out << "    rnmos ";
	    break;
	  case PGBuiltin::RPMOS:
	    out << "    rpmos ";
	    break;
	  case PGBuiltin::PMOS:
	    out << "    pmos ";
	    break;
	  default:
	    out << "    builtin gate ";
      }

      out << "(" << strength0() << "0 " << strength1() << "1) ";
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

	// If parameters are overridden by order, dump them.
      if (overrides_) {
	    assert(parms_ == 0);
            out << "#(";
	    out << *((*overrides_)[0]);
	    for (unsigned idx = 1 ;  idx < overrides_->count() ;  idx += 1) {
	          out << "," << *((*overrides_)[idx]);
	    }
	    out << ") ";
      }

	// If parameters are overridden by name, dump them.
      if (parms_) {
	    assert(overrides_ == 0);
	    out << "#(";
	    out << "." << parms_[0].name << "(" << *parms_[0].parm << ")";
	    for (unsigned idx = 1 ;  idx < nparms_ ;  idx += 1) {
		  out << ", ." << parms_[idx].name << "(" <<
			*parms_[idx].parm << ")";
	    }
	    out << ") ";
      }

      out << get_name();

	// If the module is arrayed, print the index expressions.
      if (msb_ || lsb_) {
	    out << "[";
	    if (msb_) out << *msb_;
	    out << ":";
	    if (lsb_) out << *lsb_;
	    out << "]";
      }

      out << "(";
      if (pins_) {
	    out << "." << pins_[0].name << "(";
	    if (pins_[0].parm) out << *pins_[0].parm;
	    out << ")";
	    for (unsigned idx = 1 ;  idx < npins_ ;  idx += 1) {
		  out << ", ." << pins_[idx].name << "(";
		  if (pins_[idx].parm)
			out << *pins_[idx].parm;
		  out << ")";
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
      dump_attributes(out, ind+2);
}

void Statement::dump_attributes(ostream&out, unsigned ind) const
{
      for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end()
		 ; idx ++) {
	    out << setw(ind) << "" << "(* " << (*idx).first;
	    if ((*idx).second)
		  out << " = " << *(*idx).second;
	    out << " *)" << endl;
      }
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
      if (name_ != 0)
	    out << " : " << name_;
      out << endl;

      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1) {
	    if (list_[idx])
		  list_[idx]->dump(out, ind+2);
	    else
		  out << setw(ind+2) << "" << "/* NOOP */ ;" << endl;
      }

      out << setw(ind) << "" << "end" << endl;
}

void PCallTask::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << path_;

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
      out << setw(ind) << "";
      switch (type_) {
	  case NetCase::EQ:
	    out << "case";
	    break;
	  case NetCase::EQX:
	    out << "casex";
	    break;
	  case NetCase::EQZ:
	    out << "casez";
	    break;
      }
      out << " (" << *expr_ << ") /* " << get_line() << " */" << endl;

      dump_attributes(out, ind+2);

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

void PCAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "assign " << *lval_ << " = " << *expr_
	  << "; /* " << get_line() << " */" << endl;
}

void PDeassign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "deassign " << *lval_ << "; /* "
	  << get_line() << " */" << endl;
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

void PDisable::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "disable " << scope_ << "; /* "
	  << get_line() << " */" << endl;
}

void PEventStatement::dump(ostream&out, unsigned ind) const
{
      if (expr_.count() == 0) {
	    out << setw(ind) << "" << "@* ";

      } else {
	    out << setw(ind) << "" << "@(" << *(expr_[0]);
	    if (expr_.count() > 1)
		  for (unsigned idx = 1 ;  idx < expr_.count() ;  idx += 1)
			out << " or " << *(expr_[idx]);

	    out << ")";
      }

      if (statement_) {
	    out << endl;
	    statement_->dump(out, ind+2);
      } else {
	    out << " ;" << endl;
      }
}

void PForce::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "force " << *lval_ << " = " << *expr_
	  << "; /* " << get_line() << " */" << endl;
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
      out << setw(ind) << "" << "function ";
      switch (return_type_.type) {
	  case PTF_NONE:
	    out << "?none? ";
	    break;
	  case PTF_REG:
	    out << "reg ";
	    break;
	  case PTF_INTEGER:
	    out << "integer ";
	    break;
	  case PTF_REAL:
	    out << "real ";
	    break;
	  case PTF_REALTIME:
	    out << "realtime ";
	    break;
	  case PTF_TIME:
	    out << "time ";
	    break;
      }

      if (return_type_.range) {
	    out << "[";
	    out << "] ";
      }

      out << name_ << ";" << endl;

      if (ports_)
	    for (unsigned idx = 0 ;  idx < ports_->count() ;  idx += 1) {
		  out << setw(ind) << "";
		  out << "input ";
		  out << (*ports_)[idx]->path() << ";" << endl;
	    }

      if (statement_)
	    statement_->dump(out, ind);
      else
	    out << setw(ind) << "" << "/* NOOP */" << endl;
}

void PRelease::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "release " << *lval_ << "; /* "
	  << get_line() << " */" << endl;
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
		      default:
			assert(0);
			break;
		  }
		  out << (*ports_)[idx]->path() << ";" << endl;
	    }

      if (statement_)
	    statement_->dump(out, ind);
      else
	    out << setw(ind) << "" << "/* NOOP */" << endl;
}

void PTrigger::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "-> " << event_ << ";" << endl;
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

      for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end() ; idx++ ) {

	    out << setw(ind+2) << "" << "(* " << (*idx).first;
	    if ((*idx).second) {
		  out << " = " << *(*idx).second;
	    }
	    out << " *)" << endl;
      }

      statement_->dump(out, ind+2);
}

void Module::dump(ostream&out) const
{
      if (attributes.begin() != attributes.end()) {
	    out << "(* ";
	    for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end() ; idx++ ) {
		    if (idx != attributes.begin()) {
			out << " , ";
		    }
		    out << (*idx).first;
		    if ((*idx).second) {
			out << " = " << *(*idx).second;
		    }
	    }
	    out << " *)  ";
      }

      out << "module " << name_ << ";" << endl;

      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {
	    port_t*cur = ports[idx];

	    if (cur == 0) {
		  out << "    unconnected" << endl;
		  continue;
	    }

	    out << "    ." << cur->name << "(" << *cur->expr[0];
	    for (unsigned wdx = 1 ;  wdx < cur->expr.count() ;  wdx += 1) {
		  out << ", " << *cur->expr[wdx];
	    }

	    out << ")" << endl;
      }

      typedef map<perm_string,param_expr_t>::const_iterator parm_iter_t;
      typedef map<hname_t,PExpr*>::const_iterator parm_hiter_t;
      for (parm_iter_t cur = parameters.begin()
		 ; cur != parameters.end() ; cur ++) {
	    out << "    parameter ";
	    if ((*cur).second.signed_flag)
		  out << "signed ";
	    if ((*cur).second.msb)
		  out << "[" << *(*cur).second.msb << ":"
		      << *(*cur).second.lsb << "] ";
	    out << (*cur).first << " = ";
	    if ((*cur).second.expr)
		  out << *(*cur).second.expr << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }

      for (parm_iter_t cur = localparams.begin()
		 ; cur != localparams.end() ; cur ++) {
	    out << "    localparam ";
	    if ((*cur).second.msb)
		  out << "[" << *(*cur).second.msb << ":"
		      << *(*cur).second.lsb << "] ";
	    out << (*cur).first << " = ";
	    if ((*cur).second.expr)
		  out << *(*cur).second.expr << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }

      typedef map<perm_string,PExpr*>::const_iterator specparm_iter_t;
      for (specparm_iter_t cur = specparams.begin()
		 ; cur != specparams.end() ; cur ++) {
	    out << "    specparam " << (*cur).first << " = "
		<< *(*cur).second << ";" << endl;
      }

      for (parm_hiter_t cur = defparms.begin()
		 ; cur != defparms.end() ;  cur ++) {
	    out << "    defparam " << (*cur).first << " = ";
	    if ((*cur).second)
		  out << *(*cur).second << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }

      for (map<perm_string,PEvent*>::const_iterator cur = events.begin()
		 ; cur != events.end() ;  cur ++ ) {
	    PEvent*ev = (*cur).second;
	    out << "    event " << ev->name() << "; // "
		<< ev->get_line() << endl;
      }

      for (map<hname_t,PData*>::const_iterator cur = datum.begin()
		 ; cur != datum.end() ;  cur ++ ) {
	    PData*tmp = (*cur).second;
	    out << "    real " << tmp->name() << "; // "
		<< tmp->get_line() << endl;
      }

	// Iterate through and display all the wires.
      for (map<hname_t,PWire*>::const_iterator wire = wires_.begin()
		 ; wire != wires_.end()
		 ; wire ++ ) {

	    (*wire).second->dump(out);
      }

	// Dump the task definitions.
      typedef map<perm_string,PTask*>::const_iterator task_iter_t;
      for (task_iter_t cur = tasks_.begin()
		 ; cur != tasks_.end() ; cur ++) {
	    out << "    task " << (*cur).first << ";" << endl;
	    (*cur).second->dump(out, 6);
	    out << "    endtask;" << endl;
      }

	// Dump the function definitions.
      typedef map<perm_string,PFunction*>::const_iterator func_iter_t;
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

	// Dump the attributes for the primitive as attribute
	// statements.
      for (map<string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end()
		 ; idx ++) {
	    out << "    attribute " << (*idx).first;
	    if ((*idx).second)
		  out << " = " << *(*idx).second;
	    out << endl;
      }

      out << "endprimitive" << endl;
}


/*
 * $Log: pform_dump.cc,v $
 * Revision 1.88.2.1  2006/07/10 00:21:54  steve
 *  Add support for full_case attribute.
 *
 * Revision 1.88  2004/10/04 01:10:55  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.87  2004/05/31 23:34:39  steve
 *  Rewire/generalize parsing an elaboration of
 *  function return values to allow for better
 *  speed and more type support.
 *
 * Revision 1.86  2004/05/25 19:21:07  steve
 *  More identifier lists use perm_strings.
 *
 * Revision 1.85  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.84  2004/02/20 06:22:58  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.83  2004/02/18 17:11:57  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.82  2003/07/05 20:42:08  steve
 *  Fix some enumeration warnings.
 *
 * Revision 1.81  2003/06/20 00:53:19  steve
 *  Module attributes from the parser
 *  through to elaborated form.
 *
 * Revision 1.80  2003/06/13 19:10:46  steve
 *  Properly manage real variables in subscopes.
 *
 * Revision 1.79  2003/02/27 06:45:11  steve
 *  specparams as far as pform.
 *
 * Revision 1.78  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.77  2002/10/19 22:59:49  steve
 *  Redo the parameter vector support to allow
 *  parameter names in range expressions.
 *
 * Revision 1.76  2002/08/19 02:39:17  steve
 *  Support parameters with defined ranges.
 *
 * Revision 1.75  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.74  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.73  2002/05/24 04:36:23  steve
 *  Verilog 2001 attriubtes on nets/wires.
 *
 * Revision 1.72  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.71  2002/05/19 23:37:28  steve
 *  Parse port_declaration_lists from the 2001 Standard.
 *
 * Revision 1.70  2002/04/21 04:59:08  steve
 *  Add support for conbinational events by finding
 *  the inputs to expressions and some statements.
 *  Get case and assignment statements working.
 *
 * Revision 1.69  2002/01/26 05:28:28  steve
 *  Detect scalar/vector declarion mismatch.
 *
 * Revision 1.68  2001/12/03 04:47:15  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 */

