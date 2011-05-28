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
      while (if_.size() > 0) {
	    SequentialStmt*cur = if_.front();
	    if_.pop_front();
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

SignalSeqAssignment::SignalSeqAssignment(Expression*sig, std::list<Expression*>*wav)
{
      lval_ = sig;
      if (wav) waveform_.splice(waveform_.end(), *wav);
}

SignalSeqAssignment::~SignalSeqAssignment()
{
      delete lval_;
}
