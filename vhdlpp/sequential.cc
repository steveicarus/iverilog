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
# include  <cassert>

using namespace std;

template<typename T>
inline static void visit_stmt_list(std::list<T*>& stmts, SeqStmtVisitor& func)
{
    for(typename std::list<T*>::iterator it = stmts.begin(); it != stmts.end(); ++it) {
        (*it)->visit(func);
    }
}

SequentialStmt::SequentialStmt()
{
}

SequentialStmt::~SequentialStmt()
{
}

IfSequential::IfSequential(Expression*cond, std::list<SequentialStmt*>*tr,
			   std::list<IfSequential::Elsif*>*el,
			   std::list<SequentialStmt*>*fa)
{
      cond_ = cond;
      if (tr) if_.splice(if_.end(), *tr);
      if (el) elsif_.splice(elsif_.end(), *el);
      if (fa) else_.splice(else_.end(), *fa);
}

IfSequential::~IfSequential()
{
      delete cond_;
      while (!if_.empty()) {
	    SequentialStmt*cur = if_.front();
	    if_.pop_front();
	    delete cur;
      }
      while (!elsif_.empty()) {
	    IfSequential::Elsif*cur = elsif_.front();
	    elsif_.pop_front();
	    delete cur;
      }
      while (!else_.empty()) {
	    SequentialStmt*cur = else_.front();
	    else_.pop_front();
	    delete cur;
      }

}

void IfSequential::extract_true(std::list<SequentialStmt*>&that)
{
      while (! if_.empty()) {
	    that.push_back(if_.front());
	    if_.pop_front();
      }
}

void IfSequential::extract_false(std::list<SequentialStmt*>&that)
{
      while (! else_.empty()) {
	    that.push_back(else_.front());
	    else_.pop_front();
      }
}

void IfSequential::visit(SeqStmtVisitor& func)
{
    visit_stmt_list(if_, func);
    visit_stmt_list(elsif_, func);
    visit_stmt_list(else_, func);
    func(this);
}

IfSequential::Elsif::Elsif(Expression*cond, std::list<SequentialStmt*>*tr)
: cond_(cond)
{
      if (tr) if_.splice(if_.end(), *tr);
}

IfSequential::Elsif::~Elsif()
{
      delete cond_;
      while (!if_.empty()) {
	    SequentialStmt*cur = if_.front();
	    if_.pop_front();
	    delete cur;
      }
}

void IfSequential::Elsif::visit(SeqStmtVisitor& func)
{
    visit_stmt_list(if_, func);
}

SignalSeqAssignment::SignalSeqAssignment(Expression*sig, std::list<Expression*>*wav)
{
      lval_ = sig;
      if (wav) waveform_.splice(waveform_.end(), *wav);
}

SignalSeqAssignment::~SignalSeqAssignment()
{
      delete lval_;
}

CaseSeqStmt::CaseSeqStmt(Expression*cond, list<CaseSeqStmt::CaseStmtAlternative*>* ap)
: cond_(cond)
{
      if (ap) alt_.splice(alt_.end(), *ap);
}

CaseSeqStmt::~CaseSeqStmt()
{
      delete cond_;
      while(!alt_.empty()) {
	    CaseSeqStmt::CaseStmtAlternative* cur = alt_.front();
	    alt_.pop_front();
	    delete cur;
      }
}

void CaseSeqStmt::visit(SeqStmtVisitor& func)
{
    visit_stmt_list(alt_, func);
    func(this);
}

CaseSeqStmt::CaseStmtAlternative::CaseStmtAlternative(std::list<Expression*>*exp,
        list<SequentialStmt*>*stmts)
: exp_(exp)
{
      if (stmts) stmts_.splice(stmts_.end(), *stmts);
}

CaseSeqStmt::CaseStmtAlternative::~CaseStmtAlternative()
{
      delete exp_;
      while(!stmts_.empty()) {
	    SequentialStmt* cur = stmts_.front();
	    stmts_.pop_front();
	    delete cur;
      }
}

void CaseSeqStmt::CaseStmtAlternative::visit(SeqStmtVisitor& func)
{
    visit_stmt_list(stmts_, func);
}

ProcedureCall::ProcedureCall(perm_string name)
: name_(name), param_list_(NULL), def_(NULL)
{
}

ProcedureCall::ProcedureCall(perm_string name, std::list<named_expr_t*>* param_list)
: name_(name), param_list_(param_list), def_(NULL)
{
}

ProcedureCall::ProcedureCall(perm_string name, std::list<Expression*>* param_list)
: name_(name), def_(NULL)
{
    param_list_ = new std::list<named_expr_t*>;
    for(std::list<Expression*>::const_iterator it = param_list->begin();
            it != param_list->end(); ++it)
    {
        param_list_->push_back(new named_expr_t(empty_perm_string, *it));
    }
}

ProcedureCall::~ProcedureCall()
{
    if(!param_list_)
        return;

    while(!param_list_->empty()) {
        named_expr_t* cur = param_list_->front();
        param_list_->pop_front();
        delete cur;
    }

    delete param_list_;
}

ReturnStmt::ReturnStmt(Expression*val)
: val_(val)
{
}

ReturnStmt::~ReturnStmt()
{
      delete val_;
}

void ReturnStmt::cast_to(const VType*type)
{
    assert(val_);
    val_ = new ExpCast(val_, type);
}

LoopStatement::LoopStatement(perm_string name, list<SequentialStmt*>* stmts)
: name_(name)
{
    if (stmts) stmts_.splice(stmts_.end(), *stmts);
}

LoopStatement::~LoopStatement()
{
    while(!stmts_.empty()) {
        SequentialStmt* cur = stmts_.front();
        stmts_.pop_front();
        delete cur;
    }
}

void LoopStatement::visit(SeqStmtVisitor& func)
{
    visit_stmt_list(stmts_, func);
    func(this);
}

ForLoopStatement::ForLoopStatement(perm_string scope_name, perm_string it, ExpRange* range, list<SequentialStmt*>* stmts)
: LoopStatement(scope_name, stmts), it_(it), range_(range)
{
}

ForLoopStatement::~ForLoopStatement()
{
    delete range_;
}

VariableSeqAssignment::VariableSeqAssignment(Expression*lval, Expression*rval)
: lval_(lval), rval_(rval)
{
}

VariableSeqAssignment::~VariableSeqAssignment()
{
      delete lval_;
      delete rval_;
}

WhileLoopStatement::WhileLoopStatement(perm_string lname, Expression* cond, list<SequentialStmt*>* stmts)
: LoopStatement(lname, stmts), cond_(cond)
{
}

WhileLoopStatement::~WhileLoopStatement()
{
    delete cond_;
}

BasicLoopStatement::BasicLoopStatement(perm_string lname, list<SequentialStmt*>* stmts)
: LoopStatement(lname, stmts)
{
}

BasicLoopStatement::~BasicLoopStatement()
{
}

ReportStmt::ReportStmt(Expression*msg, severity_t sev)
: msg_(msg), severity_(sev)
{
    if(sev == ReportStmt::UNSPECIFIED)
        severity_ = ReportStmt::NOTE;
}

AssertStmt::AssertStmt(Expression*condition, Expression*msg, ReportStmt::severity_t sev)
: ReportStmt(msg, sev), cond_(condition)
{
    if(msg == NULL)
        msg_ = new ExpString(default_msg_);

    if(sev == ReportStmt::UNSPECIFIED)
        severity_ = ReportStmt::ERROR;
}

const char*AssertStmt::default_msg_ = "Assertion violation.";

WaitForStmt::WaitForStmt(Expression*delay)
: delay_(delay)
{
}

WaitStmt::WaitStmt(wait_type_t typ, Expression*expr)
: type_(typ), expr_(expr)
{
}
