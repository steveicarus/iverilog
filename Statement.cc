/*
 * Copyright (c) 1998-2008 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "Statement.h"
# include  "PExpr.h"

Statement::~Statement()
{
}

PAssign_::PAssign_(PExpr*lval__, PExpr*ex, bool is_constant)
: event_(0), count_(0), lval_(lval__), rval_(ex), is_constant_(is_constant)
{
      delay_ = 0;
}

PAssign_::PAssign_(PExpr*lval__, PExpr*de, PExpr*ex)
: event_(0), count_(0), lval_(lval__), rval_(ex), is_constant_(false)
{
      delay_ = de;
}

PAssign_::PAssign_(PExpr*lval__, PExpr*cnt, PEventStatement*ev, PExpr*ex)
: event_(ev), count_(cnt), lval_(lval__), rval_(ex), is_constant_(false)
{
      delay_ = 0;
}

PAssign_::~PAssign_()
{
      delete lval_;
      delete rval_;
}

PAssign::PAssign(PExpr*lval__, PExpr*ex)
: PAssign_(lval__, ex, false)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*d, PExpr*ex)
: PAssign_(lval__, d, ex)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*cnt, PEventStatement*d, PExpr*ex)
: PAssign_(lval__, cnt, d, ex)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*ex, bool is_constant)
: PAssign_(lval__, ex, is_constant)
{
}

PAssign::~PAssign()
{
}

PAssignNB::PAssignNB(PExpr*lval__, PExpr*ex)
: PAssign_(lval__, ex, false)
{
}

PAssignNB::PAssignNB(PExpr*lval__, PExpr*d, PExpr*ex)
: PAssign_(lval__, d, ex)
{
}

PAssignNB::PAssignNB(PExpr*lval__, PExpr*cnt, PEventStatement*d, PExpr*ex)
: PAssign_(lval__, cnt, d, ex)
{
}

PAssignNB::~PAssignNB()
{
}

PBlock::PBlock(perm_string n, PScope*parent, BL_TYPE t)
: PScope(n, parent), bl_type_(t)
{
}

PBlock::PBlock(BL_TYPE t)
: PScope(perm_string()), bl_type_(t)
{
}

PBlock::~PBlock()
{
      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1)
	    delete list_[idx];
}

void PBlock::set_statement(const svector<Statement*>&st)
{
      list_ = st;
}

PCallTask::PCallTask(const pform_name_t&n, const svector<PExpr*>&p)
: path_(n), parms_(p)
{
}

PCallTask::PCallTask(perm_string n, const svector<PExpr*>&p)
: parms_(p)
{
      path_.push_back(name_component_t(n));
}

PCallTask::~PCallTask()
{
}

const pform_name_t& PCallTask::path() const
{
      return path_;
}

PCase::PCase(NetCase::TYPE t, PExpr*ex, svector<PCase::Item*>*l)
: type_(t), expr_(ex), items_(l)
{
}

PCase::~PCase()
{
      delete expr_;
      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1)
	    if ((*items_)[idx]->stat) delete (*items_)[idx]->stat;

      delete[]items_;
}

PCAssign::PCAssign(PExpr*l, PExpr*r)
: lval_(l), expr_(r)
{
}

PCAssign::~PCAssign()
{
      delete lval_;
      delete expr_;
}

PCondit::PCondit(PExpr*ex, Statement*i, Statement*e)
: expr_(ex), if_(i), else_(e)
{
}

PCondit::~PCondit()
{
      delete expr_;
      delete if_;
      delete else_;
}

PDeassign::PDeassign(PExpr*l)
: lval_(l)
{
}

PDeassign::~PDeassign()
{
      delete lval_;
}


PDelayStatement::PDelayStatement(PExpr*d, Statement*st)
: delay_(d), statement_(st)
{
}

PDelayStatement::~PDelayStatement()
{
}

PDisable::PDisable(const pform_name_t&sc)
: scope_(sc)
{
}

PDisable::~PDisable()
{
}

PEventStatement::PEventStatement(const svector<PEEvent*>&ee)
: expr_(ee), statement_(0)
{
      assert(expr_.count() > 0);
}


PEventStatement::PEventStatement(PEEvent*ee)
: expr_(1), statement_(0)
{
      expr_[0] = ee;
}

PEventStatement::PEventStatement(void)
: statement_(0)
{
}

PEventStatement::~PEventStatement()
{
	// delete the events and the statement?
}

void PEventStatement::set_statement(Statement*st)
{
      statement_ = st;
}

bool PEventStatement::has_aa_term(Design*des, NetScope*scope)
{
      bool flag = false;
      for (unsigned idx = 0 ; idx < expr_.count() ; idx += 1) {
	    flag = expr_[idx]->has_aa_term(des, scope) || flag;
      }
      return flag;
}

PForce::PForce(PExpr*l, PExpr*r)
: lval_(l), expr_(r)
{
}

PForce::~PForce()
{
      delete lval_;
      delete expr_;
}

PForever::PForever(Statement*s)
: statement_(s)
{
}

PForever::~PForever()
{
      delete statement_;
}

PForStatement::PForStatement(PExpr*n1, PExpr*e1, PExpr*cond,
			     PExpr*n2, PExpr*e2, Statement*st)
: name1_(n1), expr1_(e1), cond_(cond), name2_(n2), expr2_(e2),
  statement_(st)
{
}

PForStatement::~PForStatement()
{
}

PProcess::~PProcess()
{
      delete statement_;
}

PRelease::PRelease(PExpr*l)
: lval_(l)
{
}

PRelease::~PRelease()
{
      delete lval_;
}

PRepeat::PRepeat(PExpr*e, Statement*s)
: expr_(e), statement_(s)
{
}

PRepeat::~PRepeat()
{
      delete expr_;
      delete statement_;
}

PTrigger::PTrigger(const pform_name_t&e)
: event_(e)
{
}

PTrigger::~PTrigger()
{
}

PWhile::PWhile(PExpr*e1, Statement*st)
: cond_(e1), statement_(st)
{
}

PWhile::~PWhile()
{
      delete cond_;
      delete statement_;
}
