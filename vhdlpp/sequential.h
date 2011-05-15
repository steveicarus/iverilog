#ifndef __sequential_H
#define __sequential_H
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

# include  "LineInfo.h"
# include  <list>

class Architecture;
class Entity;
class Expression;

class SequentialStmt  : public LineInfo {

    public:
      SequentialStmt();
      virtual ~SequentialStmt() =0;

    public:
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent) const;
};

class IfSequential  : public SequentialStmt {

    public:
      IfSequential(Expression*cond, std::list<SequentialStmt*>*tr,
		   std::list<SequentialStmt*>*fa);
      ~IfSequential();

    public:
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      Expression*cond_;
      std::list<SequentialStmt*> if_;
      std::list<SequentialStmt*> else_;
};

class SignalSeqAssignment  : public SequentialStmt {
    public:
      SignalSeqAssignment(Expression*sig, std::list<Expression*>*wav);
      ~SignalSeqAssignment();

    public:
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      Expression*lval_;
      std::list<Expression*> waveform_;
};

#endif
