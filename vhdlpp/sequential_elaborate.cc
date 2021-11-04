/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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
# include  "std_types.h"

using namespace std;

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

      errors += cond_->elaborate_expr(ent, scope, &type_BOOLEAN);

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

      errors += cond_->elaborate_expr(ent, scope, &type_BOOLEAN);

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur) {
	    errors += (*cur)->elaborate(ent, scope);
      }

      return errors;
}

int ReturnStmt::elaborate(Entity*ent, ScopeBase*scope)
{
      const VType*ltype = NULL;

      // Try to determine the expression type by
      // looking up the function return type.
      const SubprogramBody*subp = dynamic_cast<const SubprogramBody*>(scope);
      if(subp) {
          if(const SubprogramHeader*header = subp->header()) {
              ltype = header->peek_return_type();
          }
      }

      return val_->elaborate_expr(ent, scope, ltype);
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

      assert(!def_);   // do not elaborate twice

      // Create a list of argument types to find a matching subprogram
      list<const VType*> arg_types;
      if(param_list_) {
            for(list<named_expr_t*>::iterator it = param_list_->begin();
                    it != param_list_->end(); ++it) {
                named_expr_t* e = *it;
                arg_types.push_back(e->expr()->probe_type(ent, scope));
            }
      }

      def_ = scope->match_subprogram(name_, &arg_types);

      if(!def_)
            def_ = library_match_subprogram(name_, &arg_types);

      if(!def_) {
            cerr << get_fileline() << ": error: could not find procedure ";
            emit_subprogram_sig(cerr, name_, arg_types);
            cerr << endl;
            return 1;
      }

	// Elaborate arguments
      if(param_list_) {
	    size_t idx = 0;
	    for(list<named_expr_t*>::iterator cur = param_list_->begin()
		 ; cur != param_list_->end() ; ++cur) {
		errors += def_->elaborate_argument((*cur)->expr(), idx, ent, scope);
		++idx;
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

int WhileLoopStatement::elaborate(Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      errors += elaborate_substatements(ent, scope);
      errors += cond_->elaborate_expr(ent, scope, cond_->probe_type(ent, scope));
      return errors;
}

int BasicLoopStatement::elaborate(Entity*ent, ScopeBase*scope)
{
    return elaborate_substatements(ent, scope);
}

int ReportStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    return msg_->elaborate_expr(ent, scope, &primitive_STRING);
}

int AssertStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    int errors = 0;
    errors += ReportStmt::elaborate(ent, scope);
    errors += cond_->elaborate_expr(ent, scope, cond_->probe_type(ent, scope));
    return errors;
}

int WaitForStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    return delay_->elaborate_expr(ent, scope, &primitive_TIME);
}

int WaitStmt::elaborate(Entity*ent, ScopeBase*scope)
{
    if(type_ == UNTIL) {
        struct fill_sens_list_t : public ExprVisitor {
            explicit fill_sens_list_t(set<ExpName*>& sig_list)
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
    } else if(type_ == FINAL) {
        return 0;   // nothing to be elaborated
    }

    return expr_->elaborate_expr(ent, scope, 0);
}
