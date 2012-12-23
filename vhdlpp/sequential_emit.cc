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
# include  "architec.h"
# include  "compiler.h"
# include  <iostream>
# include  <cstdio>
# include  <typeinfo>
# include  <ivl_assert.h>

int SequentialStmt::emit(ostream&out, Entity*, Architecture*)
{
      out << " // " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this sequential statement! "
	  << "type=" << typeid(*this).name() << endl;
      return 1;
}

int IfSequential::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      out << "if (";
      errors += cond_->emit(out, ent, arc);
      out << ") begin" << endl;

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    errors += (*cur)->emit(out, ent, arc);

      for (list<IfSequential::Elsif*>::iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur) {
	    out << "end else if (";
	    errors += (*cur)->condition_emit(out, ent, arc);
	    out << ") begin" << endl;
	    errors += (*cur)->statement_emit(out, ent, arc);
      }

      if (! else_.empty()) {
	    out << "end else begin" << endl;

	    for (list<SequentialStmt*>::iterator cur = else_.begin()
		       ; cur != else_.end() ; ++cur)
		  errors += (*cur)->emit(out, ent, arc);

      }

      out << "end" << endl;
      return errors;
}

int IfSequential::Elsif::condition_emit(ostream&out, Entity*ent, Architecture*arc)
{
      return cond_->emit(out, ent, arc);
}

int IfSequential::Elsif::statement_emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    errors += (*cur)->emit(out, ent, arc);

      return errors;
}


int SignalSeqAssignment::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += lval_->emit(out, ent, arc);

      if (waveform_.size() != 1) {
	    out << "/* Confusing waveform? */;" << endl;
	    errors += 1;

      } else {
	    Expression*tmp = waveform_.front();
	    out << " <= ";
	    errors += tmp->emit(out, ent, arc);
	    out << ";" << endl;
      }

      return errors;
}

int VariableSeqAssignment::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += lval_->emit(out, ent, arc);

      out << " = ";
      errors += rval_->emit(out, ent, arc);
      out << ";" << endl;

      return errors;
}

int ProcedureCall::emit(ostream&out, Entity*, Architecture*)
{
      out << " // " << get_fileline() << ": internal error: "
      << "I don't know how to emit this sequential statement! "
      << "type=" << typeid(*this).name() << endl;
      return 1;
}

int LoopStatement::emit_substatements(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    SequentialStmt*tmp = *cur;
	    errors += tmp->emit(out, ent, arc);
      }
      return errors;
}

int CaseSeqStmt::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      out << "case (";
      errors += cond_->emit(out, ent, arc);
      out << ")" << endl;

      for (list<CaseStmtAlternative*>::iterator cur = alt_.begin()
		 ; cur != alt_.end() ; ++cur) {
	    CaseStmtAlternative*curp = *cur;
	    errors += curp ->emit(out, ent, arc);
      }

      out << "endcase" << endl;

      return errors;
}

int CaseSeqStmt::CaseStmtAlternative::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (exp_) {
	    errors += exp_->emit(out, ent, arc);
	    out << ":" << endl;
      } else {
	    out << "default:" << endl;
      }

      SequentialStmt*curp;

      switch (stmts_.size()) {
	  case 0:
	    out << "/* no op */;" << endl;
	    break;
	  case 1:
	    curp = stmts_.front();
	    errors += curp->emit(out, ent, arc);
	    break;
	  default:
	    out << "begin" << endl;
	    for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		       ; cur != stmts_.end() ; ++cur) {
		  curp = *cur;
		  errors += curp->emit(out, ent, arc);
	    }
	    out << "end" << endl;
	    break;
      }

      return errors;
}

int ForLoopStatement::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;
      ivl_assert(*this, range_);

      int64_t start_val;
      bool start_rc = range_->msb()->evaluate(ent, arc, start_val);

      int64_t finish_val;
      bool finish_rc = range_->lsb()->evaluate(ent, arc, finish_val);

      ivl_assert(*this, start_rc);
      ivl_assert(*this, finish_rc);

      if (range_->is_downto() && start_val < finish_val) {
	    out << "begin /* Degenerate loop at " << get_fileline() << " */ end" << endl;
	    return errors;
      }

      if (!range_->is_downto() && start_val > finish_val) {
	    out << "begin /* Degenerate loop at " << get_fileline() << " */ end" << endl;
	    return errors;
      }

      perm_string scope_name = loop_name();
      if (scope_name.nil()) {
	    char buf[80];
	    snprintf(buf, sizeof buf, "__%p", this);
	    scope_name = lex_strings.make(buf);
      }

      out << "begin : " << scope_name << endl;
      out << "longint \\" << it_ << " ;" << endl;
      out << "for (\\" << it_ << " = " << start_val << " ; ";
      if (range_->is_downto())
	    out << "\\" << it_ << " >= " << finish_val;
      else
	    out << "\\" << it_ << " <= " << finish_val;
      out << "; \\" << it_ << " = \\" << it_;
      if (range_->is_downto())
	    out << " - 1";
      else
	    out << " + 1";

      out << ") begin" << endl;

      errors += emit_substatements(out, ent, arc);

      out << "end" << endl;
      out << "end /* " << scope_name << " */" << endl;

      return errors;
}

int WhileLoopStatement::emit(ostream&out, Entity*, Architecture*)
{
    out << " // " << get_fileline() << ": internal error: "
    << "I don't know how to emit this sequential statement! "
    << "type=" << typeid(*this).name() << endl;
    return 1;
}

int BasicLoopStatement::emit(ostream&out, Entity*, Architecture*)
{
    out << " // " << get_fileline() << ": internal error: "
    << "I don't know how to emit this sequential statement! "
    << "type=" << typeid(*this).name() << endl;
    return 1;
}
