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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "sequential.h"
# include  "expression.h"
# include  <iostream>
# include  <typeinfo>

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

      if (else_.size() > 0) {
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
	    tmp->emit(out, ent, arc);
	    out << ";" << endl;
      }

      return errors;
}

int ProcedureCall::emit(ostream&out, Entity*, Architecture*)
{
      out << " // " << get_fileline() << ": internal error: "
      << "I don't know how to emit this sequential statement! "
      << "type=" << typeid(*this).name() << endl;
      return 1;
}
