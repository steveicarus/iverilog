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
			   std::list<SequentialStmt*>*fa)
{
      cond_ = cond;
      if (tr) if_.splice(if_.end(), *tr);
      if (fa) else_.splice(else_.end(), *fa);
}

IfSequential::~IfSequential()
{
      delete cond_;
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
