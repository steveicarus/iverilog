/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
 * Copyright CERN 2015
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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
# include  "package.h"
# include  "compiler.h"
# include  "subprogram.h"
# include  "std_types.h"
# include  <iostream>
# include  <cstdio>
# include  <typeinfo>
# include  <limits>
# include  <ivl_assert.h>

using namespace std;

int SequentialStmt::emit(ostream&out, Entity*, ScopeBase*)
{
      out << " // " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this sequential statement! "
	  << "type=" << typeid(*this).name() << endl;
      return 1;
}

void SequentialStmt::write_to_stream(std::ostream&fd)
{
      fd << " // " << get_fileline() << ": internal error: "
	  << "I don't know how to write_to_stream this sequential statement! "
	  << "type=" << typeid(*this).name() << endl;
}

int IfSequential::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      out << "if (";
      errors += cond_->emit(out, ent, scope);
      out << ") begin" << endl;

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    errors += (*cur)->emit(out, ent, scope);

      for (list<IfSequential::Elsif*>::iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur) {
	    out << "end else if (";
	    errors += (*cur)->condition_emit(out, ent, scope);
	    out << ") begin" << endl;
	    errors += (*cur)->statement_emit(out, ent, scope);
      }

      if (! else_.empty()) {
	    out << "end else begin" << endl;

	    for (list<SequentialStmt*>::iterator cur = else_.begin()
		       ; cur != else_.end() ; ++cur)
		  errors += (*cur)->emit(out, ent, scope);

      }

      out << "end" << endl;
      return errors;
}

void IfSequential::write_to_stream(std::ostream&fd)
{
      fd << "if ";
      cond_->write_to_stream(fd);
      fd << " then" << endl;

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    (*cur)->write_to_stream(fd);

      for (list<IfSequential::Elsif*>::iterator cur = elsif_.begin()
		 ; cur != elsif_.end() ; ++cur) {
	    fd << "elsif ";
	    (*cur)->condition_write_to_stream(fd);
	    fd << " " << endl;
	    (*cur)->statement_write_to_stream(fd);
      }

      if (! else_.empty()) {
	    fd << " else " << endl;

	    for (list<SequentialStmt*>::iterator cur = else_.begin()
		       ; cur != else_.end() ; ++cur)
		  (*cur)->write_to_stream(fd);
      }

      fd << "end if;" << endl;
}

int IfSequential::Elsif::condition_emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      return cond_->emit(out, ent, scope);
}

int IfSequential::Elsif::statement_emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    errors += (*cur)->emit(out, ent, scope);

      return errors;
}

void IfSequential::Elsif::condition_write_to_stream(ostream&fd)
{
      cond_->write_to_stream(fd);
}

void IfSequential::Elsif::statement_write_to_stream(ostream&fd)
{
      for (list<SequentialStmt*>::iterator cur = if_.begin()
		 ; cur != if_.end() ; ++cur)
	    (*cur)->write_to_stream(fd);
}

int ReturnStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      out << "return ";
      errors += val_->emit(out, ent, scope);
      out << ";" << endl;
      return errors;
}

void ReturnStmt::write_to_stream(ostream&fd)
{
      fd << "return ";
      val_->write_to_stream(fd);
      fd << ";" << endl;
}

int SignalSeqAssignment::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      errors += lval_->emit(out, ent, scope);

      if (waveform_.size() != 1) {
	    out << "/* Confusing waveform? */;" << endl;
	    errors += 1;

      } else {
	    Expression*tmp = waveform_.front();
	    out << " <= ";
	    errors += tmp->emit(out, ent, scope);
	    out << ";" << endl;
      }

      return errors;
}

void SignalSeqAssignment::write_to_stream(ostream&fd)
{
      lval_->write_to_stream(fd);

      if (waveform_.size() != 1) {
	    fd << "-- Confusing waveform?" << endl;

      } else {
	    Expression*tmp = waveform_.front();
	    fd << " <= ";
	    tmp->write_to_stream(fd);
	    fd << ";" << endl;
      }
}

int VariableSeqAssignment::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      errors += lval_->emit(out, ent, scope);

      out << " = ";
      errors += rval_->emit(out, ent, scope);
      out << ";" << endl;

      return errors;
}

void VariableSeqAssignment::write_to_stream(ostream&fd)
{
      lval_->write_to_stream(fd);
      fd << " := ";
      rval_->write_to_stream(fd);
      fd << ";" << endl;
}

int ProcedureCall::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      vector<Expression*>argv;

      if(!def_) {
          cerr << get_fileline() << ": error: unknown procedure: " << name_ << endl;
          return 1;
      }

      // Convert the parameter list to vector
      if(param_list_) {
          argv.reserve(param_list_->size());

          for(std::list<named_expr_t*>::iterator it = param_list_->begin();
                  it != param_list_->end(); ++it)
              argv.push_back((*it)->expr());
      }

      def_->emit_full_name(argv, out, ent, scope);
      out << " (";

      if(param_list_)
	    errors += def_->emit_args(argv, out, ent, scope);

      out << ");" << endl;
      return errors;
}

int LoopStatement::emit_substatements(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;
      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    SequentialStmt*tmp = *cur;
	    errors += tmp->emit(out, ent, scope);
      }
      return errors;
}

void LoopStatement::write_to_stream_substatements(ostream&fd)
{
      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		 ; cur != stmts_.end() ; ++cur) {
	    SequentialStmt*tmp = *cur;
	    tmp->write_to_stream(fd);
      }
}

int CaseSeqStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      out << "case (";
      errors += cond_->emit(out, ent, scope);
      out << ")" << endl;

      for (list<CaseStmtAlternative*>::iterator cur = alt_.begin()
		 ; cur != alt_.end() ; ++cur) {
	    CaseStmtAlternative*curp = *cur;
	    errors += curp ->emit(out, ent, scope);
      }

      out << "endcase" << endl;

      return errors;
}

void CaseSeqStmt::write_to_stream(ostream&fd)
{
      fd << "case ";
      cond_->write_to_stream(fd);
      fd << " is" << endl;

      for (list<CaseStmtAlternative*>::iterator cur = alt_.begin()
		 ; cur != alt_.end() ; ++cur) {
	    CaseStmtAlternative*curp = *cur;
	    curp ->write_to_stream(fd);
      }

      fd << "end case;" << endl;
}

int CaseSeqStmt::CaseStmtAlternative::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
      int errors = 0;

      if (exp_) {
	    bool first = true;
            for (list<Expression*>::iterator it = exp_->begin(); it != exp_->end(); ++it) {
		  if(first)
		    first = false;
		  else
		    out << ",";
		  errors += (*it)->emit(out, ent, scope);
            }
      } else {
		  out << "default";
      }
      out << ":" << endl;

      SequentialStmt*curp;

      switch (stmts_.size()) {
	  case 0:
	    out << "/* no op */;" << endl;
	    break;
	  case 1:
	    curp = stmts_.front();
	    errors += curp->emit(out, ent, scope);
	    break;
	  default:
	    out << "begin" << endl;
	    for (list<SequentialStmt*>::iterator cur = stmts_.begin()
		       ; cur != stmts_.end() ; ++cur) {
		  curp = *cur;
		  errors += curp->emit(out, ent, scope);
	    }
	    out << "end" << endl;
	    break;
      }

      return errors;
}

void CaseSeqStmt::CaseStmtAlternative::write_to_stream(ostream&fd)
{
      fd << "when ";
      if (exp_) {
          bool first = true;
	  for (list<Expression*>::iterator it = exp_->begin(); it != exp_->end(); ++it) {
              if(first)
                  first = false;
              else
                  fd << "|";

              (*it)->write_to_stream(fd);
	  }
      } else {
	  fd << "others" << endl;
      }
      fd << "=>" << endl;

      for (list<SequentialStmt*>::iterator cur = stmts_.begin()
                  ; cur != stmts_.end() ; ++cur) {
              (*cur)->write_to_stream(fd);
      }
}

int WhileLoopStatement::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    out << "while(";
    errors += cond_->emit(out, ent, scope);
    out << ") begin" << endl;
    errors += emit_substatements(out, ent, scope);
    out << "end" << endl;

    return errors;
}

void WhileLoopStatement::write_to_stream(ostream&out)
{
    out << "while(";
    cond_->write_to_stream(out);
    out << ") loop" << endl;
    write_to_stream_substatements(out);
    out << "end loop;" << endl;
}

int ForLoopStatement::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;
    ivl_assert(*this, range_);

    int64_t start_val;
    bool start_rc = range_->left()->evaluate(ent, scope, start_val);

    int64_t finish_val;
    bool finish_rc = range_->right()->evaluate(ent, scope, finish_val);

    perm_string scope_name = loop_name();
    if (scope_name.nil()) {
        char buf[80];
        snprintf(buf, sizeof buf, "__%p", this);
        scope_name = lex_strings.make(buf);
    }

    out << "begin : " << scope_name << endl;
    out << "longint \\" << it_ << " ;" << endl;

    if(!start_rc || !finish_rc) {
        // Could not evaluate one of the loop boundaries, it has to be
        // determined during the run-time
        errors += emit_runtime_(out, ent, scope);
    } else {
        ExpRange::range_dir_t dir = range_->direction();

        if(dir == ExpRange::AUTO)
            dir = start_val < finish_val ? ExpRange::TO : ExpRange::DOWNTO;

        if ((dir == ExpRange::DOWNTO && start_val < finish_val) ||
                (dir == ExpRange::TO && start_val > finish_val)) {
            out << "begin /* Degenerate loop at " << get_fileline()
                << ": " << start_val;
            out << (dir == ExpRange::DOWNTO ? " downto " : " to ");
            out << finish_val << " */ end" << endl
                << "end" << endl;
            return errors;
        }

        out << "for (\\" << it_ << " = " << start_val << " ; ";

        if (dir == ExpRange::DOWNTO)
            out << "\\" << it_ << " >= " << finish_val;
        else
            out << "\\" << it_ << " <= " << finish_val;

        out << "; \\" << it_ << " = \\" << it_;

        if (dir == ExpRange::DOWNTO)
            out << " - 1)";
        else
            out << " + 1)";
    }

    out << " begin" << endl;

    errors += emit_substatements(out, ent, scope);

    out << "end" << endl;
    out << "end /* " << scope_name << " */" << endl;

    return errors;
}

void ForLoopStatement::write_to_stream(ostream&fd)
{
    fd << "for " << it_ << " in ";
    range_->write_to_stream(fd);
    fd << " loop" << endl;
    write_to_stream_substatements(fd);
    fd << "end loop;" << endl;
}

int ForLoopStatement::emit_runtime_(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    out << "for (\\" << it_ << " = ";
    errors += range_->left()->emit(out, ent, scope);

    // Twisted way of determining the loop direction at runtime
    out << " ;\n(";
    errors += range_->left()->emit(out, ent, scope);
    out << " < ";
    errors += range_->right()->emit(out, ent, scope);
    out << " ? \\" << it_ << " <= ";
    errors += range_->right()->emit(out, ent, scope);
    out << " : \\" << it_ << " >= ";
    errors += range_->right()->emit(out, ent, scope);
    out << ");\n\\" << it_ << " = \\" << it_ << " + (";
    errors += range_->left()->emit(out, ent, scope);
    out << " < ";
    errors += range_->right()->emit(out, ent, scope);
    out << " ? 1 : -1))";

    return errors;
}

int BasicLoopStatement::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    out << "forever begin" << endl;
    errors += emit_substatements(out, ent, scope);
    out << "end" << endl;

    return errors;
}

void BasicLoopStatement::write_to_stream(std::ostream&fd)
{
    fd << "loop" << endl;
    write_to_stream_substatements(fd);
    fd << "end loop;" << endl;
}

int ReportStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    out << "$display(\"** ";

    switch(severity_)
    {
        case NOTE:          out << "Note"; break;
        case WARNING:       out << "Warning"; break;
        case ERROR:         out << "Error"; break;
        case FAILURE:       out << "Failure"; break;
        case UNSPECIFIED:   ivl_assert(*this, false); break;
    }

    out << ": \",";

    struct emitter : public ExprVisitor {
        emitter(ostream&outp, Entity*enti, ScopeBase*scop)
            : out_(outp), ent_(enti), scope_(scop),
              level_lock_(numeric_limits<int>::max()) {}

        void operator() (Expression*s) {
            if(!dynamic_cast<ExpConcat*>(s)) {
                if(level() > level_lock_)
                    return;

                if(dynamic_cast<ExpAttribute*>(s)) {
                    level_lock_ = level();
                } else {
                    level_lock_ = numeric_limits<int>::max();
                }

                const VType*type = s->probe_type(ent_, scope_);

                if(dynamic_cast<ExpName*>(s) && type
                        && type->type_match(&primitive_STRING)) {
                    out_ << "$sformatf(\"%s\", (";
                    s->emit(out_, ent_, scope_);
                    out_ << "))";
                } else {
                    s->emit(out_, ent_, scope_);
                }

                out_ << ", ";
            }
        }

        private:
            ostream&out_;
            Entity*ent_;
            ScopeBase*scope_;
            int level_lock_;
    } emit_visitor(out, ent, scope);

    msg_->visit(emit_visitor);

    out << "\" (" << get_fileline() << ")\");";

    if(severity_ == FAILURE)
        out << "$finish(0);";

    out << std::endl;

    return 0;
}

void ReportStmt::write_to_stream(std::ostream&fd)
{
    fd << "report ";
    msg_->write_to_stream(fd);

    fd << "severity ";
    switch(severity_)
    {
        case NOTE:          fd << "NOTE"; break;
        case WARNING:       fd << "WARNING"; break;
        case ERROR:         fd << "ERROR"; break;
        case FAILURE:       fd << "FAILURE"; break;
        case UNSPECIFIED:   break;
    }
    fd << ";" << std::endl;
}

int AssertStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    out << "if(!(";
    errors += cond_->emit(out, ent, scope);
    out << ")) begin" << std::endl;
    errors += ReportStmt::emit(out, ent, scope);
    out << "end" << std::endl;

    return errors;
}

void AssertStmt::write_to_stream(std::ostream&fd)
{
    fd << "assert ";
    cond_->write_to_stream(fd);
    fd << std::endl;
    ReportStmt::write_to_stream(fd);
}

int WaitForStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    out << "#(";
    errors += delay_->emit(out, ent, scope);
    out << ");" << std::endl;

    return errors;
}

void WaitForStmt::write_to_stream(std::ostream&fd)
{
    fd << "wait for ";
    delay_->write_to_stream(fd);
}

int WaitStmt::emit(ostream&out, Entity*ent, ScopeBase*scope)
{
    int errors = 0;

    switch(type_) {
        case ON:
            out << "@(";
            break;

        case UNTIL:
            if(!sens_list_.empty()) {
                out << "@(";

                for(std::set<ExpName*>::iterator it = sens_list_.begin();
                        it != sens_list_.end(); ++it) {
                    if(it != sens_list_.begin())
                        out << ",";

                    (*it)->emit(out, ent, scope);
                }

                out << "); ";
            }

            out << "wait(";
            break;

        case FINAL:
            out << "/* final wait */" << endl;
            return 0;   // no expression to be emitted
    }

    errors += expr_->emit(out, ent, scope);
    out << ");" << endl;

    return errors;
}

void WaitStmt::write_to_stream(std::ostream&fd)
{
    switch(type_) {
        case ON:
            fd << "wait on ";
            break;

        case UNTIL:
            fd << "wait until ";
            break;

        case FINAL:
            fd << "wait";
            return;     // no expression to be emitted
    }

    expr_->write_to_stream(fd);
}
