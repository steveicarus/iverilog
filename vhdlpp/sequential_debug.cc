/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "sequential.h"
# include  "expression.h"
# include  <fstream>
# include  <iomanip>
# include  <typeinfo>

using namespace std;

void SequentialStmt::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "SequentialStmt[" << typeid(*this).name() << "]"
	  << " at file=" << get_fileline() << endl;
}

void IfSequential::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "IfSequential at file=" << get_fileline() << endl;
      out << setw(indent+3) << "" << "Condition:" << endl;
      cond_->dump(out, indent+4);

      out << setw(indent+3) << "" << "TRUE clause (" << if_.size() << "):" << endl;
      for (list<SequentialStmt*>::const_iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    (*cur)->dump(out, indent+4);

      for (list<IfSequential::Elsif*>::const_iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur)
	    (*cur)->dump(out, indent);

      out << setw(indent+3) << "" << "FALSE clause (" << else_.size() << "):" << endl;
      for (list<SequentialStmt*>::const_iterator cur = else_.begin()
		 ; cur != else_.end() ; ++cur)
	    (*cur)->dump(out, indent+4);

}

void IfSequential::Elsif::dump(ostream&out, int indent) const
{
      out << setw(indent+3) << "" << "Elsif Condition at " << get_fileline() << ":" << endl;
      cond_->dump(out, indent+4);

      out << setw(indent+3) << "" << "ELSIF TRUE clause (" << if_.size() << "):" << endl;
      for (list<SequentialStmt*>::const_iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    (*cur)->dump(out, indent+4);

}

void ReturnStmt::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "ReturnStmt at file=" << get_fileline() << endl;
      if (val_)
	    val_->dump(out, indent+4);
      else
	    out << setw(indent+4) << "" << "()" << endl;
}

void SignalSeqAssignment::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "SignalSeqAssignment at file=" << get_fileline() << endl;

      out << setw(indent+3) << "" << "l-value:" << endl;
      lval_->dump(out, indent+4);

      out << setw(indent+3) << "" << "r-values (" << waveform_.size() << "):" << endl;
      for (list<Expression*>::const_iterator cur = waveform_.begin()
		 ; cur != waveform_.end() ; ++cur)
	    (*cur)->dump(out, indent+4);
}

void CaseSeqStmt::dump(ostream& out, int indent) const
{
	out << setw(indent) << "" << "CaseSeqStmt at file=" << get_fileline() << endl;

	out << setw(indent+3) << "" << "Case: " << endl;
	cond_->dump(out, indent+4);

	for (list<CaseSeqStmt::CaseStmtAlternative*>::const_iterator cur = alt_.begin()
		; cur != alt_.end() ; ++cur)
		(*cur)->dump(out, indent+4);
}

void CaseSeqStmt::CaseStmtAlternative::dump(ostream& out, int indent) const
{
      out << setw(indent) << "" << "CaseStmtAlternative at file=" << get_fileline() << endl;

      out << setw(indent) << "" << "when ";
      if (exp_)
	    for (list<Expression*>::iterator it = exp_->begin(); it != exp_->end(); ++it) {
	        (*it)->dump(out, 0);
	    }
      else
	    out << "others" << endl;

      for (list<SequentialStmt*>::const_iterator cur = stmts_.begin()
		 ; cur != stmts_.end(); ++cur)
	    (*cur)->dump(out, indent+1);
}

void ProcedureCall::dump(ostream& out, int indent) const
{
    out << setw(indent) << "" << "ProcedureCall at file=" << get_fileline() << endl;
    out << setw(indent+2) << "" << name_ << "(";
    for(list<named_expr_t*>::const_iterator it = param_list_->begin();
        it != param_list_->end(); ++it)
        (*it)->dump(out, indent);
    out << ")" << endl;
}

void LoopStatement::dump(ostream&out, int indent) const
{
    for(list<SequentialStmt*>::const_iterator it = stmts_.begin();
        it != stmts_.end(); ++it)
        (*it)->dump(out, indent);
}

void ForLoopStatement::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "ForLoopStatement at file=" << get_fileline() << endl;
    out << setw(indent) << "" << it_ << " in ";
    range_->dump(out, indent);
    LoopStatement::dump(out, indent+2);
}

void VariableSeqAssignment::dump(ostream&out, int indent) const
{
      out << setw(indent) << "" << "VariableSeqAssignment at file=" << get_fileline() << endl;

      out << setw(indent+3) << "" << "l-value:" << endl;
      lval_->dump(out, indent+4);

      out << setw(indent+3) << "" << "r-value:" << endl;
      rval_->dump(out, indent+4);
}

void WhileLoopStatement::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "WhileLoopStatement at file=" << get_fileline() << endl;
    out << setw(indent) << "" << "condition: ";
    cond_->dump(out, indent);
    LoopStatement::dump(out, indent+2);
}

void BasicLoopStatement::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "BasicLoopStatement at file=" << get_fileline() << endl;
    LoopStatement::dump(out, indent+2);
}

void ReportStmt::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "ReportStmt at file=" << get_fileline() << endl;
    dump_sev_msg(out, indent+3);
}

void ReportStmt::dump_sev_msg(ostream&out, int indent) const
{
    out << setw(indent) << "" << "severity: " << severity_ << endl;

    if(msg_) {
        out << setw(indent) << "" << "message: ";
        msg_->dump(out, indent);
    }
}

void AssertStmt::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "AssertStmt at file=" << get_fileline() << endl;
    out << setw(indent+3) << "" << "condition: ";
    dump_sev_msg(out, indent+3);
}

void WaitForStmt::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "WaitForStmt at file=" << get_fileline() << endl;
    out << setw(indent+3) << "" << "delay: ";
    delay_->dump(out, indent+3);
}

void WaitStmt::dump(ostream&out, int indent) const
{
    out << setw(indent) << "" << "WaitStmt at file=" << get_fileline() << endl;
    out << setw(indent+3) << "type = ";

    switch(type_) {
        case ON: out << "ON" << endl; break;
        case UNTIL: out << "UNTIL" << endl; break;
        case FINAL: out << "FINAL" << endl; break;
    }

    if(type_ != FINAL) {
        out << setw(indent+3) << "" << "expression: ";
        expr_->dump(out, indent+3);
    }
}
