/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "Statement.h"
# include  "PExpr.h"
# include  "ivl_assert.h"

using namespace std;

Statement::~Statement()
{
}

PAssign_::PAssign_(PExpr*lval__, PExpr*ex, bool is_constant, bool is_init)
: event_(0), count_(0), lval_(lval__), rval_(ex), is_constant_(is_constant),
  is_init_(is_init)
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
: PAssign_(lval__, ex, false), op_(0)
{
}

PAssign::PAssign(PExpr*lval__, char op, PExpr*ex)
: PAssign_(lval__, ex, false), op_(op)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*d, PExpr*ex)
: PAssign_(lval__, d, ex), op_(0)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*cnt, PEventStatement*d, PExpr*ex)
: PAssign_(lval__, cnt, d, ex), op_(0)
{
}

PAssign::PAssign(PExpr*lval__, PExpr*ex, bool is_constant, bool is_init)
: PAssign_(lval__, ex, is_constant, is_init), op_(0)
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

PBlock::PBlock(perm_string n, LexicalScope*parent, BL_TYPE t)
: PScope(n, parent), bl_type_(t)
{
}

PBlock::PBlock(BL_TYPE t)
: PScope(perm_string()), bl_type_(t)
{
}

PBlock::~PBlock()
{
      for (unsigned idx = 0 ;  idx < list_.size() ;  idx += 1)
	    delete list_[idx];
}

bool PBlock::var_init_needs_explicit_lifetime() const
{
      return default_lifetime == STATIC;
}

PChainConstructor* PBlock::extract_chain_constructor()
{
      if (list_.empty())
	    return 0;

      if (PChainConstructor*res = dynamic_cast<PChainConstructor*> (list_[0])) {
	    for (size_t idx = 0 ; idx < list_.size()-1 ; idx += 1)
		  list_[idx] = list_[idx+1];
	    list_.resize(list_.size()-1);
	    return res;
      }

      return 0;
}

void PBlock::set_join_type(PBlock::BL_TYPE type)
{
      ivl_assert(*this, bl_type_ == BL_PAR);
      ivl_assert(*this, type==BL_PAR || type==BL_JOIN_NONE || type==BL_JOIN_ANY);
      bl_type_ = type;
}

void PBlock::set_statement(const vector<Statement*>&st)
{
      list_ = st;
}

void PBlock::push_statement_front(Statement*that)
{
      ivl_assert(*this, bl_type_==BL_SEQ);

      list_.resize(list_.size()+1);
      for (size_t idx = list_.size()-1 ; idx > 0 ; idx -= 1)
	    list_[idx] = list_[idx-1];

      list_[0] = that;
}

PNamedItem::SymbolType PBlock::symbol_type() const
{
      return BLOCK;
}

PCallTask::PCallTask(const pform_name_t &n, const list<named_pexpr_t> &p)
: package_(0), path_(n), parms_(p.begin(), p.end())
{
}

PCallTask::PCallTask(PPackage *pkg, const pform_name_t &n, const list<named_pexpr_t> &p)
: package_(pkg), path_(n), parms_(p.begin(), p.end())
{
}

PCallTask::PCallTask(perm_string n, const list<named_pexpr_t> &p)
: package_(0), parms_(p.begin(), p.end())
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

PCase::PCase(ivl_case_quality_t q, NetCase::TYPE t, PExpr*ex, std::vector<PCase::Item*>*l)
: quality_(q), type_(t), expr_(ex), items_(l)
{
}

PCase::~PCase()
{
      delete expr_;
      for (unsigned idx = 0 ;  idx < items_->size() ;  idx += 1)
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

PChainConstructor::PChainConstructor(const list<named_pexpr_t> &parms)
: parms_(parms.begin(), parms.end())
{
}

PChainConstructor::PChainConstructor(const vector<named_pexpr_t> &parms)
: parms_(parms)
{
}

PChainConstructor::~PChainConstructor()
{
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

PDoWhile::PDoWhile(PExpr*ex, Statement*st)
: cond_(ex), statement_(st)
{
}

PDoWhile::~PDoWhile()
{
      delete cond_;
      delete statement_;
}

PEventStatement::PEventStatement(const std::vector<PEEvent*>&ee)
: expr_(ee), statement_(0), always_sens_(false)
{
      ivl_assert(*this, expr_.size() > 0);
}


PEventStatement::PEventStatement(PEEvent*ee)
: expr_(1), statement_(0), always_sens_(false)
{
      expr_[0] = ee;
}

PEventStatement::PEventStatement(bool always_sens)
: statement_(0), always_sens_(always_sens)
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
      for (unsigned idx = 0 ; idx < expr_.size() ; idx += 1) {
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

PForeach::PForeach(perm_string av, const list<perm_string>&ix, Statement*s)
: array_var_(av), index_vars_(ix.begin(), ix.end()), statement_(s)
{
}

PForeach::~PForeach()
{
      delete statement_;
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
			     Statement*step, Statement*st)
: name1_(n1), expr1_(e1), cond_(cond), step_(step), statement_(st)
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

PReturn::PReturn(PExpr*e)
: expr_(e)
{
}

PReturn::~PReturn()
{
      delete expr_;
}

PTrigger::PTrigger(PPackage*pkg, const pform_name_t&ev, unsigned lexical_pos)
: event_(pkg, ev), lexical_pos_(lexical_pos)
{
}

PTrigger::~PTrigger()
{
}

PNBTrigger::PNBTrigger(const pform_name_t&ev, unsigned lexical_pos, PExpr*dly)
: event_(ev), lexical_pos_(lexical_pos), dly_(dly)
{
}

PNBTrigger::~PNBTrigger()
{
}

PWhile::PWhile(PExpr*ex, Statement*st)
: cond_(ex), statement_(st)
{
}

PWhile::~PWhile()
{
      delete cond_;
      delete statement_;
}
