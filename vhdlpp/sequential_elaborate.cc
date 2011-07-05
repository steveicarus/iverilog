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

int SequentialStmt::elaborate(Entity*, Architecture*)
{
      return 0;
}

int IfSequential::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += cond_->elaborate_expr(ent, arc, 0);

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, arc);
      }

      for (list<IfSequential::Elsif*>::iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, arc);
      }

      for (list<SequentialStmt*>::iterator cur = else_.begin()
		 ; cur != else_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, arc);
      }

      return errors;
}

int IfSequential::Elsif::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      errors += cond_->elaborate_expr(ent, arc, 0);

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, arc);
      }

      return errors;
}

int SignalSeqAssignment::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

	// Elaborate the l-value expression.
      errors += lval_->elaborate_lval(ent, arc, true);

	// The elaborate_lval should have resolved the type of the
	// l-value expression. We'll use that type to elaborate the
	// r-value.
      const VType*lval_type = lval_->peek_type();
      if (lval_type == 0) {
	    if (errors == 0) errors += 1;
	    return errors;
      }

	// Elaborate the r-value expressions.
      for (list<Expression*>::iterator cur = waveform_.begin()
		 ; cur != waveform_.end() ; ++cur) {

	    errors += (*cur)->elaborate_expr(ent, arc, lval_type);
      }

      return errors;
}

int ProcedureCall::elaborate(Entity*, Architecture*)
{
      return 0;
}
