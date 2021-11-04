#ifndef IVL_sequential_H
#define IVL_sequential_H
/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
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
# include  <set>

class ScopeBase;
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
      virtual int elaborate(Entity*ent, ScopeBase*scope);
      virtual int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      virtual void dump(std::ostream&out, int indent) const;
      virtual void write_to_stream(std::ostream&fd);

      // Recursively visits a tree of sequential statements.
      virtual void visit(SeqStmtVisitor& func) { func(this); }
};

/*
 * The LoopStatement is an abstract base class for the various loop
 * statements.
 */
class LoopStatement : public SequentialStmt {
    public:
      LoopStatement(perm_string block_name, std::list<SequentialStmt*>*);
      virtual ~LoopStatement();

      inline perm_string loop_name() const { return name_; }

      void dump(std::ostream&out, int indent)  const;
      void visit(SeqStmtVisitor& func);

    protected:
      int elaborate_substatements(Entity*ent, ScopeBase*scope);
      int emit_substatements(std::ostream&out, Entity*ent, ScopeBase*scope);
      void write_to_stream_substatements(std::ostream&fd);

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

	    int elaborate(Entity*entity, ScopeBase*scope);
	    int condition_emit(std::ostream&out, Entity*entity, ScopeBase*scope);
	    int statement_emit(std::ostream&out, Entity*entity, ScopeBase*scope);

	    void condition_write_to_stream(std::ostream&fd);
	    void statement_write_to_stream(std::ostream&fd);

	    void dump(std::ostream&out, int indent) const;
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
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;
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
      explicit ReturnStmt(Expression*val);
      ~ReturnStmt();

    public:
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;

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
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;

    private:
      Expression*lval_;
      std::list<Expression*> waveform_;
};

class CaseSeqStmt : public SequentialStmt {
    public:
      class CaseStmtAlternative : public LineInfo {
        public:
            CaseStmtAlternative(std::list<Expression*>*exp, std::list<SequentialStmt*>*stmts);
            ~CaseStmtAlternative();
            void dump(std::ostream& out, int indent) const;
	    int elaborate_expr(Entity*ent, ScopeBase*scope, const VType*ltype);
	    int elaborate(Entity*ent, ScopeBase*scope);
	    int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
            void write_to_stream(std::ostream&fd);
	    void visit(SeqStmtVisitor& func);

        private:
            std::list<Expression*>*exp_;
	    std::list<SequentialStmt*> stmts_;
        private: // not implemented
            CaseStmtAlternative(const CaseStmtAlternative&);
            CaseStmtAlternative& operator =(const CaseStmtAlternative&);
      };

    public:
      CaseSeqStmt(Expression*cond, std::list<CaseStmtAlternative*>*sp);
      ~CaseSeqStmt();

    public:
      void dump(std::ostream&out, int indent) const;
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void visit(SeqStmtVisitor& func);

    private:
      Expression* cond_;
      std::list<CaseStmtAlternative*> alt_;
};

class ProcedureCall : public SequentialStmt {
    public:
      explicit ProcedureCall(perm_string name);
      ProcedureCall(perm_string name, std::list<named_expr_t*>* param_list);
      ProcedureCall(perm_string name, std::list<Expression*>* param_list);
      ~ProcedureCall();

      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void dump(std::ostream&out, int indent) const;

    private:
      perm_string name_;
      std::list<named_expr_t*>* param_list_;
      SubprogramHeader*def_;
};

class VariableSeqAssignment  : public SequentialStmt {
    public:
      VariableSeqAssignment(Expression*sig, Expression*rval);
      ~VariableSeqAssignment();

    public:
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;

    private:
      Expression*lval_;
      Expression*rval_;
};

class WhileLoopStatement : public LoopStatement {
    public:
      WhileLoopStatement(perm_string loop_name,
			 Expression*, std::list<SequentialStmt*>*);
      ~WhileLoopStatement();

      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*ent, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;

    private:
      Expression* cond_;
};

class ForLoopStatement : public LoopStatement {
    public:
      ForLoopStatement(perm_string loop_name,
		       perm_string index, ExpRange*, std::list<SequentialStmt*>*);
      ~ForLoopStatement();

      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*ent, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;

    private:
      // Emits for-loop which direction is determined at run-time.
      // It is used for 'range & 'reverse_range attributes.
      int emit_runtime_(std::ostream&out, Entity*ent, ScopeBase*scope);

      perm_string it_;
      ExpRange* range_;
};

class BasicLoopStatement : public LoopStatement {
    public:
      BasicLoopStatement(perm_string lname, std::list<SequentialStmt*>*);
      ~BasicLoopStatement();

      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*ent, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);
      void dump(std::ostream&out, int indent) const;
};

class ReportStmt : public SequentialStmt {
    public:
      typedef enum { UNSPECIFIED, NOTE, WARNING, ERROR, FAILURE } severity_t;

      ReportStmt(Expression*message, severity_t severity);
      virtual ~ReportStmt() {}

      void dump(std::ostream&out, int indent) const;
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);

      inline Expression*message() const { return msg_; }
      inline severity_t severity() const { return severity_; }

    protected:
      void dump_sev_msg(std::ostream&out, int indent) const;

      Expression*msg_;
      severity_t severity_;
};

class AssertStmt : public ReportStmt {
    public:
      AssertStmt(Expression*condition, Expression*message,
                 ReportStmt::severity_t severity = ReportStmt::ERROR);

      void dump(std::ostream&out, int indent) const;
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);

    private:
      Expression*cond_;

      // Message displayed when there is no report assigned.
      static const char*default_msg_;
};

class WaitForStmt : public SequentialStmt {
    public:
      explicit WaitForStmt(Expression*delay);

      void dump(std::ostream&out, int indent) const;
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);

    private:
      Expression*delay_;
};

class WaitStmt : public SequentialStmt {
    public:
      typedef enum { ON, UNTIL, FINAL } wait_type_t;
      WaitStmt(wait_type_t typ, Expression*expression);

      void dump(std::ostream&out, int indent) const;
      int elaborate(Entity*ent, ScopeBase*scope);
      int emit(std::ostream&out, Entity*entity, ScopeBase*scope);
      void write_to_stream(std::ostream&fd);

      inline wait_type_t type() const { return type_; }

    private:
      wait_type_t type_;
      Expression*expr_;
      // Sensitivity list for 'wait until' statement
      std::set<ExpName*> sens_list_;
};

#endif /* IVL_sequential_H */
