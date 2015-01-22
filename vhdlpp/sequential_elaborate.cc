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

int SequentialStmt::elaborate(Entity*, Architecture*)
{
      return 0;
}

int LoopStatement::elaborate_substatements(Entity*ent, Architecture*arc)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, arc);
      }

      return errors;
}

int CaseSeqStmt::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      const VType*ctype = cond_->probe_type(ent, arc);
      errors += cond_->elaborate_expr(ent, arc, ctype);

      for (list<CaseStmtAlternative*>::iterator cur = alt_.begin()
		 ; cur != alt_.end() ; ++cur) {
	    CaseStmtAlternative*curp = *cur;
	    errors += curp->elaborate_expr(ent, arc, ctype);
	    errors += curp->elaborate(ent, arc);
      }

      return errors;
}

/*
 * This method elaborates the case expression for the alternative. The
 * ltype is the probed type for the main case condition. The
 * expression needs to elaborate itself in that context.
 */
int CaseSeqStmt::CaseStmtAlternative::elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype)
{
      int errors = 0;
      if (exp_)
	    errors += exp_->elaborate_expr(ent, arc, ltype);
      return errors;
}

int CaseSeqStmt::CaseStmtAlternative::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    SequentialStmt*curp = *cur;
	    errors += curp->elaborate(ent, arc);
      }

      return errors;
}

int ForLoopStatement::elaborate(Entity*ent, Architecture*arc)
{
      int errors = 0;
      errors += elaborate_substatements(ent, arc);
      return errors;
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

int VariableSeqAssignment::elaborate(Entity*ent, Architecture*arc)
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

	// Elaborate the r-value expression.
      errors += rval_->elaborate_expr(ent, arc, lval_type);

	// Handle functions that return unbounded arrays
      if(ExpFunc*call = dynamic_cast<ExpFunc*>(rval_)) {
	    const VType*ret_type = call->func_ret_type();
            if(ret_type && ret_type->is_unbounded())
                rval_ = new ExpCast(rval_, get_global_typedef(lval_type));
      }

      return errors;
}

int WhileLoopStatement::elaborate(Entity*, Architecture*)
{
    //TODO:check whether there is any wait statement in the statements (there should be)
    return 0;
}

int BasicLoopStatement::elaborate(Entity*, Architecture*)
{
    return 0;
}
