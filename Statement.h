#ifndef __Statement_H
#define __Statement_H
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
#ident "$Id: Statement.h,v 1.5 1999/01/25 05:45:56 steve Exp $"
#endif

# include  <string>
# include  <list>
# include  "netlist.h"
# include  "LineInfo.h"
class PExpr;
class Statement;

/*
 * The PProcess is the root of a behavioral process. Each process gets
 * one of these, which contains its type (initial or always) and a
 * pointer to the single statement that is the process. A module may
 * have several concurrent processes.
 */
class PProcess : public LineInfo {

    public:
      enum Type { PR_INITIAL, PR_ALWAYS };

      PProcess(Type t, Statement*st)
      : type_(t), statement_(st) { }

      virtual ~PProcess();

      Type type() const { return type_; }
      Statement*statement() { return statement_; }

      virtual void dump(ostream&out, unsigned ind) const;

    private:
      Type type_;
      Statement*statement_;
};

/*
 * The PProcess is a process, the Statement is the actual action. In
 * fact, the Statement class is abstract and represents all the
 * possible kinds of statements that exist in Verilog.
 */
class Statement : public LineInfo {

    public:
      Statement() { }
      virtual ~Statement() =0;

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;
};

/*
 * Assignment statements of the various forms are handled by this
 * type. The rvalue is an expression. The lvalue needs to be figured
 * out by the parser as much as possible.
 */
class PAssign  : public Statement {

    public:
      explicit PAssign(const string&name, PExpr*ex)
      : to_name_(name), expr_(ex) { }

      string lval() const { return to_name_; }
      const PExpr* get_expr() const { return expr_; }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      const string to_name_;
      PExpr*const expr_;
};

/*
 * A block statement is an ordered list of statements that make up the
 * block. The block can be sequential or parallel, which only affects
 * how the block is interpreted. The parser collects the list of
 * statements before constructing this object, so it knows a priori
 * what is contained.
 */
class PBlock  : public Statement {

    public:
      enum BL_TYPE { BL_SEQ, BL_PAR };

      explicit PBlock(BL_TYPE t, const list<Statement*>&st);
      ~PBlock();

      BL_TYPE bl_type() const { return bl_type_; }

      unsigned size() const { return nlist_; }
      const Statement*stat(unsigned idx) const { return list_[idx]; }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      const BL_TYPE bl_type_;
      unsigned nlist_;
      Statement**list_;
};

class PCallTask  : public Statement {

    public:
      explicit PCallTask(const string&n, const list<PExpr*>&parms);

      string name() const { return name_; }

      unsigned nparms() const { return nparms_; }

      PExpr*&parm(unsigned idx)
	    { assert(idx < nparms_);
	      return parms_[idx];
	    }

      PExpr* parm(unsigned idx) const
	    { assert(idx < nparms_);
	      return parms_[idx];
	    }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      const string name_;
      const unsigned nparms_;
      PExpr**const parms_;
};

class PCondit  : public Statement {

    public:
      PCondit(PExpr*ex, Statement*i, Statement*e)
      : expr_(ex), if_(i), else_(e) { }
      ~PCondit();

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*if_;
      Statement*else_;

    private: // not implemented
      PCondit(const PCondit&);
      PCondit& operator= (const PCondit&);
};

class PDelayStatement  : public Statement {

    public:
      PDelayStatement(PExpr*d, Statement*st)
      : delay_(d), statement_(st) { }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      PExpr*delay_;
      Statement*statement_;
};

class PEventStatement  : public Statement {

    public:

      PEventStatement(NetPEvent::Type t, PExpr*ee)
      : type_(t), expr_(ee), statement_(0) { }

      void set_statement(Statement*st) { statement_ = st; }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      NetPEvent::Type type_;
      PExpr*expr_;
      Statement*statement_;
};

class PForStatement  : public Statement {

    public:
      PForStatement(const string&n1, PExpr*e1, PExpr*cond,
		    const string&n2, PExpr*e2, Statement*st)
      : name1_(n1), expr1_(e1), cond_(cond), name2_(n2), expr2_(e2),
	statement_(st)
      { }

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      string name1_;
      PExpr* expr1_;

      PExpr*cond_;

      string name2_;
      PExpr* expr2_;

      Statement*statement_;
};

class PNoop  : public Statement {

    public:
      PNoop() { }
};

class PWhile  : public Statement {

    public:
      PWhile(PExpr*e1, Statement*st)
      : cond_(e1), statement_(st) { }
      ~PWhile();

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*cond_;
      Statement*statement_;
};

/*
 * $Log: Statement.h,v $
 * Revision 1.5  1999/01/25 05:45:56  steve
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
 * Revision 1.4  1998/11/11 03:13:04  steve
 *  Handle while loops.
 *
 * Revision 1.3  1998/11/09 18:55:33  steve
 *  Add procedural while loops,
 *  Parse procedural for loops,
 *  Add procedural wait statements,
 *  Add constant nodes,
 *  Add XNOR logic gate,
 *  Make vvm output look a bit prettier.
 *
 * Revision 1.2  1998/11/07 17:05:05  steve
 *  Handle procedural conditional, and some
 *  of the conditional expressions.
 *
 *  Elaborate signals and identifiers differently,
 *  allowing the netlist to hold signal information.
 *
 * Revision 1.1  1998/11/03 23:28:56  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
