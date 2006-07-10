#ifndef __Statement_H
#define __Statement_H
/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: Statement.h,v 1.40.2.1 2006/07/10 00:21:49 steve Exp $"
#endif

# include  <string>
# include  "svector.h"
# include  "StringHeap.h"
# include  "PDelays.h"
# include  "PExpr.h"
# include  "HName.h"
# include  "LineInfo.h"
class PExpr;
class Statement;
class PEventStatement;
class Design;
class NetAssign_;
class NetCAssign;
class NetDeassign;
class NetScope;

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

      map<perm_string,PExpr*> attributes;

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
class Statement : public LineInfo, public Attrib {

    public:
      Statement() { }
      virtual ~Statement() =0;

      map<perm_string,PExpr*> attributes;

      void dump_attributes(ostream&out, unsigned ind) const;
      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
};

/*
 * Assignment statements of the various forms are handled by this
 * type. The rvalue is an expression. The lvalue needs to be figured
 * out by the parser as much as possible.
 */
class PAssign_  : public Statement {
    public:
      explicit PAssign_(PExpr*lval, PExpr*ex);
      explicit PAssign_(PExpr*lval, PExpr*de, PExpr*ex);
      explicit PAssign_(PExpr*lval, PEventStatement*de, PExpr*ex);
      virtual ~PAssign_() =0;

      const PExpr* lval() const  { return lval_; }
      const PExpr* rval() const  { return rval_; }

    protected:
      NetAssign_* elaborate_lval(Design*, NetScope*scope) const;

      PExpr* delay_;
      PEventStatement*event_;

    private:
      PExpr* lval_;
      PExpr* rval_;
};

class PAssign  : public PAssign_ {

    public:
      explicit PAssign(PExpr*lval, PExpr*ex);
      explicit PAssign(PExpr*lval, PExpr*de, PExpr*ex);
      explicit PAssign(PExpr*lval, PEventStatement*de, PExpr*ex);
      ~PAssign();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
};

class PAssignNB  : public PAssign_ {

    public:
      explicit PAssignNB(PExpr*lval, PExpr*ex);
      explicit PAssignNB(PExpr*lval, PExpr*de, PExpr*ex);
      ~PAssignNB();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
      NetProc*assign_to_memory_(class NetMemory*, PExpr*,
				Design*des, NetScope*scope) const;
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

      explicit PBlock(perm_string n, BL_TYPE t, const svector<Statement*>&st);
      explicit PBlock(BL_TYPE t, const svector<Statement*>&st);
      explicit PBlock(BL_TYPE t);
      ~PBlock();

      BL_TYPE bl_type() const { return bl_type_; }


      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;

    private:
      perm_string name_;
      const BL_TYPE bl_type_;
      svector<Statement*>list_;
};

class PCallTask  : public Statement {

    public:
      explicit PCallTask(const hname_t&n, const svector<PExpr*>&parms);
      ~PCallTask();

      const hname_t& path() const;

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
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
      NetProc* elaborate_sys(Design*des, NetScope*scope) const;
      NetProc* elaborate_usr(Design*des, NetScope*scope) const;

      hname_t path_;
      svector<PExpr*> parms_;
};

class PCase  : public Statement {

    public:
      struct Item {
	    svector<PExpr*>expr;
	    Statement*stat;
      };

      PCase(NetCase::TYPE, PExpr*ex, svector<Item*>*);
      ~PCase();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      NetCase::TYPE type_;
      PExpr*expr_;

      svector<Item*>*items_;

    private: // not implemented
      PCase(const PCase&);
      PCase& operator= (const PCase&);
};

class PCAssign  : public Statement {

    public:
      explicit PCAssign(PExpr*l, PExpr*r);
      ~PCAssign();

      virtual NetCAssign* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
      PExpr*expr_;
};

class PCondit  : public Statement {

    public:
      PCondit(PExpr*ex, Statement*i, Statement*e);
      ~PCondit();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*if_;
      Statement*else_;

    private: // not implemented
      PCondit(const PCondit&);
      PCondit& operator= (const PCondit&);
};

class PDeassign  : public Statement {

    public:
      explicit PDeassign(PExpr*l);
      ~PDeassign();

      virtual NetDeassign* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
};

class PDelayStatement  : public Statement {

    public:
      PDelayStatement(PExpr*d, Statement*st);
      ~PDelayStatement();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;

    private:
      PExpr*delay_;
      Statement*statement_;
};


/*
 * This represents the parsing of a disable <scope> statement.
 */
class PDisable  : public Statement {

    public:
      explicit PDisable(const hname_t&sc);
      ~PDisable();

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
      hname_t scope_;
};

/*
 * The event statement represents the event delay in behavioral
 * code. It comes from such things as:
 *
 *      @name <statement>;
 *      @(expr) <statement>;
 *      @* <statement>;
 */
class PEventStatement  : public Statement {

    public:

      explicit PEventStatement(const svector<PEEvent*>&ee);
      explicit PEventStatement(PEEvent*ee);
	// Make an @* statement.
      explicit PEventStatement(void);

      ~PEventStatement();

      void set_statement(Statement*st);

      virtual void dump(ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;

	// This method is used to elaborate, but attach a previously
	// elaborated statement to the event.
      NetProc* elaborate_st(Design*des, NetScope*scope, NetProc*st) const;

      NetProc* elaborate_wait(Design*des, NetScope*scope, NetProc*st) const;

    private:
      svector<PEEvent*>expr_;
      Statement*statement_;
};

class PForce  : public Statement {

    public:
      explicit PForce(PExpr*l, PExpr*r);
      ~PForce();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
      PExpr*expr_;
};

class PForever : public Statement {
    public:
      explicit PForever(Statement*s);
      ~PForever();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      Statement*statement_;
};

class PForStatement  : public Statement {

    public:
      PForStatement(PExpr*n1, PExpr*e1, PExpr*cond,
		    PExpr*n2, PExpr*e2, Statement*st);
      ~PForStatement();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
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

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*statement_;
};

class PRelease  : public Statement {

    public:
      explicit PRelease(PExpr*l);
      ~PRelease();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
};

/*
 * The PTrigger statement sends a trigger to a named event. Take the
 * name here.
 */
class PTrigger  : public Statement {

    public:
      explicit PTrigger(const hname_t&ev);
      ~PTrigger();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      hname_t event_;
};

class PWhile  : public Statement {

    public:
      PWhile(PExpr*e1, Statement*st);
      ~PWhile();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void dump(ostream&out, unsigned ind) const;

    private:
      PExpr*cond_;
      Statement*statement_;
};

/*
 * $Log: Statement.h,v $
 * Revision 1.40.2.1  2006/07/10 00:21:49  steve
 *  Add support for full_case attribute.
 *
 * Revision 1.40  2004/02/20 18:53:33  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.39  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.38  2003/05/19 02:50:58  steve
 *  Implement the wait statement behaviorally instead of as nets.
 *
 * Revision 1.37  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.36  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.35  2002/06/04 05:38:44  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.34  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.33  2002/04/21 22:31:02  steve
 *  Redo handling of assignment internal delays.
 *  Leave it possible for them to be calculated
 *  at run time.
 *
 * Revision 1.32  2002/04/21 04:59:07  steve
 *  Add support for conbinational events by finding
 *  the inputs to expressions and some statements.
 *  Get case and assignment statements working.
 *
 * Revision 1.31  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.30  2001/11/22 06:20:59  steve
 *  Use NetScope instead of string for scope path.
 *
 * Revision 1.29  2000/09/09 15:21:26  steve
 *  move lval elaboration to PExpr virtual methods.
 *
 * Revision 1.28  2000/09/03 17:58:35  steve
 *  Change elaborate_lval to return NetAssign_ objects.
 *
 * Revision 1.27  2000/07/26 05:08:07  steve
 *  Parse disable statements to pform.
 *
 * Revision 1.26  2000/05/11 23:37:26  steve
 *  Add support for procedural continuous assignment.
 *
 * Revision 1.25  2000/04/22 04:20:19  steve
 *  Add support for force assignment.
 *
 * Revision 1.24  2000/04/12 04:23:57  steve
 *  Named events really should be expressed with PEIdent
 *  objects in the pform,
 *
 *  Handle named events within the mix of net events
 *  and edges. As a unified lot they get caught together.
 *  wait statements are broken into more complex statements
 *  that include a conditional.
 *
 *  Do not generate NetPEvent or NetNEvent objects in
 *  elaboration. NetEvent, NetEvWait and NetEvProbe
 *  take over those functions in the netlist.
 */
#endif
