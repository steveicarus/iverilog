/*
 * Copyright (c) 1998-1999 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: Statement.cc,v 1.29 2004/02/18 17:11:54 steve Exp $"
#endif

# include "config.h"

# include  "Statement.h"
# include  "PExpr.h"

Statement::~Statement()
{
}

PAssign_::PAssign_(PExpr*lval, PExpr*ex)
: event_(0), lval_(lval), rval_(ex)
{
      delay_ = 0;
}

PAssign_::PAssign_(PExpr*lval, PExpr*de, PExpr*ex)
: event_(0), lval_(lval), rval_(ex)
{
      delay_ = de;
}

PAssign_::PAssign_(PExpr*lval, PEventStatement*ev, PExpr*ex)
: event_(ev), lval_(lval), rval_(ex)
{
      delay_ = 0;
}

PAssign_::~PAssign_()
{
      delete lval_;
      delete rval_;
}

PAssign::PAssign(PExpr*lval, PExpr*ex)
: PAssign_(lval, ex)
{
}

PAssign::PAssign(PExpr*lval, PExpr*d, PExpr*ex)
: PAssign_(lval, d, ex)
{
}

PAssign::PAssign(PExpr*lval, PEventStatement*d, PExpr*ex)
: PAssign_(lval, d, ex)
{
}

PAssign::~PAssign()
{
}

PAssignNB::PAssignNB(PExpr*lval, PExpr*ex)
: PAssign_(lval, ex)
{
}

PAssignNB::PAssignNB(PExpr*lval, PExpr*d, PExpr*ex)
: PAssign_(lval, d, ex)
{
}

PAssignNB::~PAssignNB()
{
}

PBlock::PBlock(perm_string n, BL_TYPE t, const svector<Statement*>&st)
: name_(n), bl_type_(t), list_(st)
{
}

PBlock::PBlock(BL_TYPE t, const svector<Statement*>&st)
: bl_type_(t), list_(st)
{
}

PBlock::PBlock(BL_TYPE t)
: bl_type_(t)
{
}

PBlock::~PBlock()
{
      for (unsigned idx = 0 ;  idx < list_.count() ;  idx += 1)
	    delete list_[idx];
}

PCallTask::PCallTask(const hname_t&n, const svector<PExpr*>&p)
: path_(n), parms_(p)
{
}

PCallTask::~PCallTask()
{
}

const hname_t& PCallTask::path() const
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

PDisable::PDisable(const hname_t&sc)
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

PTrigger::PTrigger(const hname_t&e)
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

/*
 * $Log: Statement.cc,v $
 * Revision 1.29  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.28  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.27  2002/04/21 22:31:02  steve
 *  Redo handling of assignment internal delays.
 *  Leave it possible for them to be calculated
 *  at run time.
 *
 * Revision 1.26  2002/04/21 04:59:07  steve
 *  Add support for conbinational events by finding
 *  the inputs to expressions and some statements.
 *  Get case and assignment statements working.
 *
 * Revision 1.25  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.24  2001/11/22 06:20:59  steve
 *  Use NetScope instead of string for scope path.
 *
 * Revision 1.23  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 */

