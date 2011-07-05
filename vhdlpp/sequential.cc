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
      while (if_.size() > 0) {
	    SequentialStmt*cur = if_.front();
	    if_.pop_front();
	    delete cur;
      }
      while (elsif_.size() > 0) {
	    IfSequential::Elsif*cur = elsif_.front();
	    elsif_.pop_front();
	    delete cur;
      }
      while (else_.size() > 0) {
	    SequentialStmt*cur = else_.front();
	    else_.pop_front();
	    delete cur;
      }

}

void IfSequential::extract_true(std::list<SequentialStmt*>&that)
{
      while (if_.size() > 0) {
	    that.push_back(if_.front());
	    if_.pop_front();
      }
}

void IfSequential::extract_false(std::list<SequentialStmt*>&that)
{
      while (else_.size() > 0) {
	    that.push_back(else_.front());
	    else_.pop_front();
      }
}

IfSequential::Elsif::Elsif(Expression*cond, std::list<SequentialStmt*>*tr)
: cond_(cond)
{
      if (tr) if_.splice(if_.end(), *tr);
}

IfSequential::Elsif::~Elsif()
{
      delete cond_;
      while (if_.size() > 0) {
	    SequentialStmt*cur = if_.front();
	    if_.pop_front();
	    delete cur;
      }
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

CaseSeqStmt::CaseSeqStmt(Expression*cond,	list<CaseSeqStmt::CaseStmtAlternative*>* ap)
    : cond_(cond)
{

	if (ap) alt_.splice(alt_.end(), *ap);
}

CaseSeqStmt::~CaseSeqStmt()
{
	delete cond_;
	while(alt_.size() > 0) {
		CaseSeqStmt::CaseStmtAlternative* cur = alt_.front();
		alt_.pop_front();
		delete cur;
	}
}

CaseSeqStmt::CaseStmtAlternative::~CaseStmtAlternative() {
    delete exp_;
    while(stmts_.size() > 0) {
        SequentialStmt* cur = stmts_.front();
        stmts_.pop_front();
        delete cur;
    }
}

CaseSeqStmt::CaseStmtAlternative::CaseStmtAlternative(Expression* exp, list<SequentialStmt*>* stmts)
    : exp_(exp)
{
      if (stmts) stmts_.splice(stmts_.end(), *stmts);
}

ProcedureCall::ProcedureCall(perm_string name)
: name_(name), param_list_(0)
{
}

ProcedureCall::ProcedureCall(perm_string name, std::list<named_expr_t*>* param_list)
: name_(name), param_list_(param_list)
{
}

ProcedureCall::~ProcedureCall()
{
    while(param_list_->size() > 0) {
        named_expr_t* cur = param_list_->front();
        param_list_->pop_front();
        delete cur;
    }
}

LoopStatement::LoopStatement(list<SequentialStmt*>* stmts)
{
    if (stmts) stmts_.splice(stmts_.end(), *stmts);
}

LoopStatement::~LoopStatement()
{
    while(stmts_.size() > 0) {
        SequentialStmt* cur = stmts_.front();
        stmts_.pop_front();
        delete cur;
    }
}

ForLoopStatement::ForLoopStatement(perm_string it, range_t* range, list<SequentialStmt*>* stmts)
: LoopStatement(stmts), it_(it), range_(range)
{ 
}

ForLoopStatement::~ForLoopStatement()
{
    delete range_;
}

WhileLoopStatement::WhileLoopStatement(ExpLogical* cond, list<SequentialStmt*>* stmts)
: LoopStatement(stmts), cond_(cond)
{
}

WhileLoopStatement::~WhileLoopStatement()
{
    delete cond_;
}

BasicLoopStatement::BasicLoopStatement(list<SequentialStmt*>* stmts)
: LoopStatement(stmts)
{
}

BasicLoopStatement::~BasicLoopStatement()
{
}
