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
# include  "scope.h"
# include  "library.h"
# include  "subprogram.h"

int SequentialStmt::elaborate(Entity*, ScopeBase*)
{
      return 0;
}

int LoopStatement::elaborate_substatements(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      return errors;
}

int CaseSeqStmt::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      const VType*ctype = cond_->probe_type(ent, scope);
      errors += cond_->elaborate_expr(ent, scope, ctype);

      for (list<CaseStmtAlternative*>::iterator cur = alt_.begin()
		 ; cur != alt_.end() ; ++cur) {
	    CaseStmtAlternative*curp = *cur;
	    errors += curp->elaborate_expr(ent, scope, ctype);
	    errors += curp->elaborate(ent, scope);
      }

      return errors;
}

/*
 * This method elaborates the case expression for the alternative. The
 * ltype is the probed type for the main case condition. The
 * expression needs to elaborate itself in that context.
 */
int CaseSeqStmt::CaseStmtAlternative::elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype)
{
      int errors = 0;
      if (exp_) {
            for (list<Expression*>::iterator it = exp_->begin(); it != exp_->end();
                    ++it) {
                errors += (*it)->elaborate_expr(ent, scope, ltype);
            }
      }
      return errors;
}

int CaseSeqStmt::CaseStmtAlternative::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    SequentialStmt*curp = *cur;
	    errors += curp->elaborate(ent, scope);
      }

      return errors;
}

int ForLoopStatement::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      errors += elaborate_substatements(ent, scope);
      return errors;
}

int IfSequential::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      errors += cond_->elaborate_expr(ent, scope, 0);

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      for (list<IfSequential::Elsif*>::iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      for (list<SequentialStmt*>::iterator cur = else_.begin()
		 ; cur != else_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      return errors;
}

int IfSequential::Elsif::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      errors += cond_->elaborate_expr(ent, scope, 0);

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      return errors;
}

int SignalSeqAssignment::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

	// Elaborate the l-value expression.
      errors += lval_->elaborate_lval(ent, scope, true);

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
	    errors += (*cur)->elaborate_expr(ent, scope, lval_type);
      }

      return errors;
}

int ProcedureCall::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      def_ = scope->find_subprogram(name_);

      if(!def_)
            def_ = library_find_subprogram(name_);

      assert(def_);

	// Elaborate arguments
      size_t idx = 0;
      if(param_list_) {
	    for(list<named_expr_t*>::iterator cur = param_list_->begin()
		 ; cur != param_list_->end() ; ++cur) {
                const VType*tmp = (*cur)->expr()->probe_type(ent, scope);
                const VType*param_type = def_ ? def_->peek_param_type(idx) : NULL;

                if(!tmp && param_type)
                    tmp = param_type;

                errors += (*cur)->expr()->elaborate_expr(ent, scope, tmp);
            }
      }

      return errors;
}

int VariableSeqAssignment::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;

	// Elaborate the l-value expression.
      errors += lval_->elaborate_lval(ent, scope, true);

	// The elaborate_lval should have resolved the type of the
	// l-value expression. We'll use that type to elaborate the
	// r-value.
      const VType*lval_type = lval_->peek_type();
      if (lval_type == 0) {
	    if (errors == 0) errors += 1;
	    return errors;
      }

	// Elaborate the r-value expression.
      errors += rval_->elaborate_expr(ent, scope, lval_type);

      return errors;
}

int WhileLoopStatement::elaborate(Entity*, ScopeBase*)
{
    //TODO:check whether there is any wait statement in the statements (there should be)
    return 0;
}

int BasicLoopStatement::elaborate(Entity*, ScopeBase*)
{
    return 0;
}

int AssertStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    return cond_->elaborate_expr(ent, scope, 0);
}

int WaitForStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    return delay_->elaborate_expr(ent, scope, 0);
}

int WaitStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    if(type_ == UNTIL) {
        struct fill_sens_list_t : public ExprVisitor {
            fill_sens_list_t(set<ExpName*>& sig_list)
            : sig_list_(sig_list) {};

            void operator() (Expression*s) {
                if(ExpName*name = dynamic_cast<ExpName*>(s))
                    sig_list_.insert(name);
            }

            private:
                set<ExpName*>& sig_list_;
        } fill_sens_list(sens_list_);

        // Fill the sensitivity list
        expr_->visit(fill_sens_list);
    }

    return expr_->elaborate_expr(ent, scope, 0);
}
