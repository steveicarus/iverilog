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
#ident "$Id: Statement.h,v 1.15 1999/07/12 00:59:36 steve Exp $"
#endif

# include  <string>
# include  "svector.h"
# include  "PExpr.h"
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
class PAssign_  : public Statement {
    public:
      explicit PAssign_(PExpr*lval, PExpr*de, PExpr*ex);
      virtual ~PAssign_() =0;

      const PExpr* lval() const  { return lval_; }
      const PExpr* delay() const { return delay_; }
      const PExpr* rval() const  { return rval_; }

    protected:
      NetNet*elaborate_lval(Design*, const string&path,
			    unsigned&lsb, unsigned&msb,
			    NetExpr*&mux) const;

    private:
      PExpr* lval_;
      PExpr* delay_;
      PExpr* rval_;
};

class PAssign  : public PAssign_ {

    public:
      explicit PAssign(PExpr*lval, PExpr*ex);
      explicit PAssign(PExpr*lval, PExpr*de, PExpr*ex);
      ~PAssign();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      NetProc*assign_to_memory_(class NetMemory*, PExpr*,
				Design*des, const string&path) const;
};

class PAssignNB  : public PAssign_ {

    public:
      explicit PAssignNB(PExpr*lval, PExpr*ex);
      ~PAssignNB();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
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

      explicit PBlock(const string&n, BL_TYPE t, const svector<Statement*>&st);
      explicit PBlock(BL_TYPE t, const svector<Statement*>&st);
      explicit PBlock(BL_TYPE t);
      ~PBlock();

      BL_TYPE bl_type() const { return bl_type_; }

	//unsigned size() const { return list_.count(); }
	//const Statement*stat(unsigned idx) const { return list_[idx]; }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      string name_;
      const BL_TYPE bl_type_;
      svector<Statement*>list_;
};

class PCallTask  : public Statement {

    public:
      explicit PCallTask(const string&n, const svector<PExpr*>&parms);

      string name() const { return name_; }

      unsigned nparms() const { return parms_.count(); }

      PExpr*&parm(unsigned idx)
	    { assert(idx < parms_.count());
	      return parms_[idx];
	    }

      PExpr* parm(unsigned idx) const
	    { assert(idx < parms_.count());
	      return parms_[idx];
	    }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      NetProc* elaborate_sys(Design*des, const string&path) const;
      NetProc* elaborate_usr(Design*des, const string&path) const;

      const string name_;
      svector<PExpr*> parms_;
};

class PCase  : public Statement {

    public:
      struct Item {
	    svector<PExpr*>expr;
	    Statement*stat;
      };

      PCase(PExpr*ex, svector<Item*>*);
      ~PCase();

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;

      svector<Item*>*items_;

    private: // not implemented
      PCase(const PCase&);
      PCase& operator= (const PCase&);
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

      PEventStatement(const svector<PEEvent*>&ee)
      : expr_(ee), statement_(0) { }

      PEventStatement(PEEvent*ee)
      : expr_(1), statement_(0) { expr_[0] = ee; }

      void set_statement(Statement*st) { statement_ = st; }

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, const string&path) const;

    private:
      svector<PEEvent*>expr_;
      Statement*statement_;
};

class PForever : public Statement {
    public:
      explicit PForever(Statement*s);
      ~PForever();

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      Statement*statement_;
};

class PForStatement  : public Statement {

    public:
      PForStatement(PExpr*n1, PExpr*e1, PExpr*cond,
		    PExpr*n2, PExpr*e2, Statement*st)
      : name1_(n1), expr1_(e1), cond_(cond), name2_(n2), expr2_(e2),
	statement_(st)
      { }

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr* name1_;
      PExpr* expr1_;

      PExpr*cond_;

      PExpr* name2_;
      PExpr* expr2_;

      Statement*statement_;
};

class PNoop  : public Statement {

    public:
      PNoop() { }
};

class PRepeat : public Statement {
    public:
      explicit PRepeat(PExpr*expr, Statement*s);
      ~PRepeat();

      virtual NetProc* elaborate(Design*des, const string&path) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*statement_;
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
 * Revision 1.15  1999/07/12 00:59:36  steve
 *  procedural blocking assignment delays.
 *
 * Revision 1.14  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.13  1999/06/24 04:24:18  steve
 *  Handle expression widths for EEE and NEE operators,
 *  add named blocks and scope handling,
 *  add registers declared in named blocks.
 *
 * Revision 1.12  1999/06/19 21:06:16  steve
 *  Elaborate and supprort to vvm the forever
 *  and repeat statements.
 *
 * Revision 1.11  1999/06/15 05:38:39  steve
 *  Support case expression lists.
 *
 * Revision 1.10  1999/06/13 23:51:16  steve
 *  l-value part select for procedural assignments.
 *
 * Revision 1.9  1999/06/06 20:45:38  steve
 *  Add parse and elaboration of non-blocking assignments,
 *  Replace list<PCase::Item*> with an svector version,
 *  Add integer support.
 *
 * Revision 1.8  1999/05/10 00:16:58  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.7  1999/04/29 02:16:26  steve
 *  Parse OR of event expressions.
 *
 * Revision 1.6  1999/02/03 04:20:11  steve
 *  Parse and elaborate the Verilog CASE statement.
 *
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
