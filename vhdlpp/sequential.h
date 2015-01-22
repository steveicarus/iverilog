#ifndef IVL_sequential_H
#define IVL_sequential_H
/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include  "LineInfo.h"
# include "parse_types.h"
# include  <list>
# include  <functional>

class Architecture;
class Entity;
class Expression;
class SequentialStmt;

struct SeqStmtVisitor {
    virtual ~SeqStmtVisitor() {};
    virtual void operator() (SequentialStmt*s) = 0;
};

class SequentialStmt  : public LineInfo {

    public:
      SequentialStmt();
      virtual ~SequentialStmt() =0;

    public:
      virtual int elaborate(Entity*ent, Architecture*arc);
      virtual int emit(ostream&out, Entity*entity, Architecture*arc);
      virtual void dump(ostream&out, int indent) const;

      // Recursively visits a tree of sequential statements.
      virtual void visit(SeqStmtVisitor& func) { func(this); }
};

/*
 * The LoopStatement is an abstract base class for the various loop
 * statements.
 */
class LoopStatement : public SequentialStmt {
    public:
      LoopStatement(perm_string block_name, list<SequentialStmt*>*);
      virtual ~LoopStatement();

      inline perm_string loop_name() const { return name_; }

      void dump(ostream&out, int indent)  const;
      void visit(SeqStmtVisitor& func);

    protected:
      int elaborate_substatements(Entity*ent, Architecture*arc);
      int emit_substatements(std::ostream&out, Entity*ent, Architecture*arc);

    private:
      perm_string name_;
      std::list<SequentialStmt*> stmts_;
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
	    void visit(SeqStmtVisitor& func);

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
      void visit(SeqStmtVisitor& func);

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

class ReturnStmt  : public SequentialStmt {
    public:
      ReturnStmt(Expression*val);
      ~ReturnStmt();

    public:
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

      const Expression*peek_expr() const { return val_; };
      void cast_to(const VType*type);

    private:
      Expression*val_;
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

class CaseSeqStmt : public SequentialStmt {
    public:
      class CaseStmtAlternative : public LineInfo {
        public:
            CaseStmtAlternative(Expression* exp, std::list<SequentialStmt*>* stmts);
            ~CaseStmtAlternative();
            void dump(std::ostream& out, int indent) const;
	    int elaborate_expr(Entity*ent, Architecture*arc, const VType*ltype);
	    int elaborate(Entity*ent, Architecture*arc);
	    int emit(ostream&out, Entity*entity, Architecture*arc);
	    void visit(SeqStmtVisitor& func);

        private:
            Expression* exp_;
	    std::list<SequentialStmt*> stmts_;
        private: // not implemented
            CaseStmtAlternative(const CaseStmtAlternative&);
            CaseStmtAlternative& operator =(const CaseStmtAlternative&);
      };

    public:
      CaseSeqStmt(Expression*cond, std::list<CaseStmtAlternative*>*sp);
      ~CaseSeqStmt();

    public:
      void dump(ostream&out, int indent) const;
      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void visit(SeqStmtVisitor& func);

    private:
      Expression* cond_;
      std::list<CaseStmtAlternative*> alt_;
};

class ProcedureCall : public SequentialStmt {
    public:
      ProcedureCall(perm_string name);
      ProcedureCall(perm_string name, std::list<named_expr_t*>* param_list);
      ~ProcedureCall();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      perm_string name_;
      std::list<named_expr_t*>* param_list_;
};

class VariableSeqAssignment  : public SequentialStmt {
    public:
      VariableSeqAssignment(Expression*sig, Expression*rval);
      ~VariableSeqAssignment();

    public:
      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      Expression*lval_;
      Expression*rval_;
};

class WhileLoopStatement : public LoopStatement {
    public:
      WhileLoopStatement(perm_string loop_name,
			 ExpLogical*, list<SequentialStmt*>*);
      ~WhileLoopStatement();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      ExpLogical* cond_;
};

class ForLoopStatement : public LoopStatement {
    public:
      ForLoopStatement(perm_string loop_name,
		       perm_string index, prange_t*, list<SequentialStmt*>*);
      ~ForLoopStatement();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*ent, Architecture*arc);
      void dump(ostream&out, int indent) const;

    private:
      // Emits for-loop which direction is determined at run-time.
      // It is used for 'range & 'reverse_range attributes.
      int emit_runtime_(ostream&out, Entity*ent, Architecture*arc);

      perm_string it_;
      prange_t* range_;
};

class BasicLoopStatement : public LoopStatement {
    public:
      BasicLoopStatement(perm_string lname, list<SequentialStmt*>*);
      ~BasicLoopStatement();

      int elaborate(Entity*ent, Architecture*arc);
      int emit(ostream&out, Entity*entity, Architecture*arc);
      void dump(ostream&out, int indent) const;
};

#endif /* IVL_sequential_H */
