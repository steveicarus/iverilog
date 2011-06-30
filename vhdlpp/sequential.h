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
      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent) const;
};

class IfSequential  : public SequentialStmt {

    public:
      class Elsif : public LineInfo {
	  public:
	    Elsif(Expression*cond, std::list<SequentialStmt*>*tr);
	    ~Elsif();

	    int elaborate(Entity*entity, Architecture*arc);
	    int condition_emit(ostream&out, Entity*entity, Architecture*arc);
	    int statement_emit(ostream&out, Entity*entity, Architecture*arc);

	    void dump(ostream&out, int indent) const;

	  private:
	    Expression*cond_;
	    std::list<SequentialStmt*>if_;
	  private: // not implemented
	    Elsif(const Elsif&);
	    Elsif& operator =(const Elsif&);
      };

    public:
      IfSequential(Expression*cond, std::list<SequentialStmt*>*tr,
		   std::list<IfSequential::Elsif*>*elsif,
		   std::list<SequentialStmt*>*fa);
      ~IfSequential();

    public:
      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

      const Expression*peek_condition() const { return cond_; }

      size_t false_size() const { return else_.size(); }

	// These method extract (and remove) the sub-statements from
	// the true or false clause.
      void extract_true(std::list<SequentialStmt*>&that);
      void extract_false(std::list<SequentialStmt*>&that);

    private:
      Expression*cond_;
      std::list<SequentialStmt*> if_;
      std::list<IfSequential::Elsif*> elsif_;
      std::list<SequentialStmt*> else_;
};

class SignalSeqAssignment  : public SequentialStmt {
    public:
      SignalSeqAssignment(Expression*sig, std::list<Expression*>*wav);
      ~SignalSeqAssignment();

    public:
      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      Expression*lval_;
      std::list<Expression*> waveform_;
};

#endif
