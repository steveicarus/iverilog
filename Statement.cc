/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: Statement.cc,v 1.16 1999/09/29 18:36:02 steve Exp $"
#endif

# include  "Statement.h"
# include  "PExpr.h"

Statement::~Statement()
{
}

PAssign_::PAssign_(PExpr*lval, PExpr*ex)
: event_(0), lval_(lval), rval_(ex)
{
}

PAssign_::PAssign_(PExpr*lval, PExpr*de, PExpr*ex)
: event_(0), lval_(lval), rval_(ex)
{
      if (de) delay_.set_delay(de);
}

PAssign_::PAssign_(PExpr*lval, PEventStatement*ev, PExpr*ex)
: event_(ev), lval_(lval), rval_(ex)
{
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

PBlock::PBlock(const string&n, BL_TYPE t, const svector<Statement*>&st)
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

PCallTask::PCallTask(const string&n, const svector<PExpr*>&p)
: name_(n), parms_(p)
{
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

PCondit::~PCondit()
{
      delete expr_;
      delete if_;
      delete else_;
}

PForever::PForever(Statement*s)
: statement_(s)
{
}

PForever::~PForever()
{
      delete statement_;
}

PProcess::~PProcess()
{
      delete statement_;
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

PWhile::~PWhile()
{
      delete cond_;
      delete statement_;
}

/*
 * $Log: Statement.cc,v $
 * Revision 1.16  1999/09/29 18:36:02  steve
 *  Full case support
 *
 * Revision 1.15  1999/09/22 02:00:48  steve
 *  assignment with blocking event delay.
 *
 * Revision 1.14  1999/09/04 19:11:46  steve
 *  Add support for delayed non-blocking assignments.
 *
 * Revision 1.13  1999/09/02 01:59:27  steve
 *  Parse non-blocking assignment delays.
 *
 * Revision 1.12  1999/07/12 00:59:36  steve
 *  procedural blocking assignment delays.
 *
 * Revision 1.11  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.10  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.9  1999/06/15 05:38:39  steve
 *  Support case expression lists.
 *
 * Revision 1.8  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
 * Revision 1.7  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.6  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.5  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
 * Revision 1.4  1999/01/25 05:45:56  steve
 *  Add the LineInfo class to carry the source file
 *  location of things. PGate, Statement and PProcess.
 *
 *  elaborate handles module parameter mismatches,
 *  missing or incorrect lvalues for procedural
 *  assignment, and errors are propogated to the
 *  top of the elaboration call tree.
 *
 *  Attach line numbers to processes, gates and
 *  assignment statements.
 *
 * Revision 1.3  1998/11/11 03:13:04  steve
 *  Handle while loops.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:55  steve
 *  Introduce verilog to CVS.
 *
 */

