#ifndef IVL_Statement_H
#define IVL_Statement_H
/*
 * Copyright (c) 1998-2024 Stephen Williams (steve@icarus.com)
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

# include  <string>
# include  <vector>
# include  <list>
# include  "ivl_target.h"
# include  "StringHeap.h"
# include  "PDelays.h"
# include  "PExpr.h"
# include  "PScope.h"
# include  "HName.h"
# include  "LineInfo.h"
class PExpr;
class PChainConstructor;
class PPackage;
class Statement;
class PEventStatement;
class Design;
class NetAssign_;
class NetCAssign;
class NetDeassign;
class NetForce;
class NetScope;

/*
 * The PProcess is the root of a behavioral process. Each process gets
 * one of these, which contains its type (initial, always, or final)
 * and a pointer to the single statement that is the process. A module
 * may have several concurrent processes.
 */
class PProcess : public LineInfo {

    public:
      PProcess(ivl_process_type_t t, Statement*st)
      : type_(t), statement_(st) { }

      virtual ~PProcess();

      bool elaborate(Design*des, NetScope*scope) const;

      ivl_process_type_t type() const { return type_; }
      Statement*statement() { return statement_; }

      std::map<perm_string,PExpr*> attributes;

      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      ivl_process_type_t type_;
      Statement*statement_;
};

/*
 * The PProcess is a process, the Statement is the actual action. In
 * fact, the Statement class is abstract and represents all the
 * possible kinds of statements that exist in Verilog.
 */
class Statement : virtual public LineInfo {

    public:
      Statement() { }
      virtual ~Statement() =0;

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;

      std::map<perm_string,PExpr*> attributes;
};

/*
 * Assignment statements of the various forms are handled by this
 * type. The rvalue is an expression. The lvalue needs to be figured
 * out by the parser as much as possible.
 */
class PAssign_  : public Statement {
    public:
      explicit PAssign_(PExpr*lval, PExpr*ex, bool is_constant,
			bool is_init = false);
      explicit PAssign_(PExpr*lval, PExpr*de, PExpr*ex);
      explicit PAssign_(PExpr*lval, PExpr*cnt, PEventStatement*de, PExpr*ex);
      virtual ~PAssign_() =0;

      const PExpr* lval() const  { return lval_; }
      PExpr* rval() const  { return rval_; }

    protected:
      NetAssign_* elaborate_lval(Design*, NetScope*scope) const;
      NetExpr* elaborate_rval_(Design*, NetScope*, ivl_type_t lv_net_type,
			       ivl_variable_type_t lv_type,
			       unsigned lv_width,
			       bool force_unsigned =false) const;
      NetExpr* elaborate_rval_(Design*, NetScope*, ivl_type_t ntype) const;

      NetExpr* elaborate_rval_obj_(Design*, NetScope*,
				   ivl_variable_type_t type) const;

      PExpr* delay_;
      PEventStatement*event_;
      PExpr* count_;

    private:
      PExpr* lval_;
      PExpr* rval_;
      bool is_constant_;
        // Whether the assignment is a variable initializer expression
      bool is_init_ = false;
};

class PAssign  : public PAssign_ {

    public:
	// lval  - assignment l-value
	// ex    - assignment r-value
	// op    - compressed assignment operator (i.e. '+', '-', ...)
	// de    - delayed assignment delay expression
      explicit PAssign(PExpr*lval, PExpr*ex);
      explicit PAssign(PExpr*lval, char op, PExpr*ex);
      explicit PAssign(PExpr*lval, PExpr*de, PExpr*ex);
      explicit PAssign(PExpr*lval, PExpr*cnt, PEventStatement*de, PExpr*ex);
      explicit PAssign(PExpr*lval, PExpr*ex, bool is_constant, bool is_init);
      ~PAssign();

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
      NetProc* elaborate_compressed_(Design*des, NetScope*scope) const;
      char op_;
};

class PAssignNB  : public PAssign_ {

    public:
      explicit PAssignNB(PExpr*lval, PExpr*ex);
      explicit PAssignNB(PExpr*lval, PExpr*de, PExpr*ex);
      explicit PAssignNB(PExpr*lval, PExpr*cnt, PEventStatement*de, PExpr*ex);
      ~PAssignNB();

      virtual void dump(std::ostream&out, unsigned ind) const;
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
class PBlock  : public PScope, public Statement, public PNamedItem {

    public:
      enum BL_TYPE { BL_SEQ, BL_PAR, BL_JOIN_NONE, BL_JOIN_ANY };

	// If the block has a name, it is a scope and also has a parent.
      explicit PBlock(perm_string n, LexicalScope*parent, BL_TYPE t);
	// If it doesn't have a name, it's not a scope
      explicit PBlock(BL_TYPE t);
      ~PBlock();

      BL_TYPE bl_type() const { return bl_type_; }

      bool var_init_needs_explicit_lifetime() const;

	// This is only used if this block is the statement list for a
	// constructor. We look for a PChainConstructor as the first
	// statement, and if it is there, extract it.
      PChainConstructor*extract_chain_constructor();

	// If the bl_type() is BL_PAR, it is possible to replace it
	// with JOIN_NONE or JOIN_ANY. This is to help the parser.
      void set_join_type(BL_TYPE);

      void set_statement(const std::vector<Statement*>&st);

	// Copy the statement from that block to the front of this
	// block.
      void push_statement_front(Statement*that);

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;

      SymbolType symbol_type() const;

    private:
      BL_TYPE bl_type_;
      std::vector<Statement*>list_;
};

class PBreak : public Statement {
    public:
      void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
};

class PCallTask  : public Statement {

    public:
      explicit PCallTask(PPackage *pkg, const pform_name_t &n, const std::list<named_pexpr_t> &parms);
      explicit PCallTask(const pform_name_t &n, const std::list<named_pexpr_t> &parms);
      explicit PCallTask(perm_string n, const std::list<named_pexpr_t> &parms);
      ~PCallTask();

      const pform_name_t& path() const;

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

      bool elaborate_elab(Design*des, NetScope*scope) const;

      void void_cast() { void_cast_ = true; }

    private:
      NetProc* elaborate_sys(Design*des, NetScope*scope) const;
      NetProc* elaborate_usr(Design*des, NetScope*scope) const;

      NetProc*elaborate_method_(Design*des, NetScope*scope,
                                bool add_this_flag = false) const;
      NetProc*elaborate_function_(Design*des, NetScope*scope) const;
      NetProc*elaborate_void_function_(Design*des, NetScope*scope,
				       NetFuncDef*def) const;
      NetProc *elaborate_non_void_function_(Design *des, NetScope *scope) const;

      NetProc*elaborate_build_call_(Design*des, NetScope*scope,
				    NetScope*task, NetExpr*use_this) const;
      NetProc*elaborate_sys_task_method_(Design*des, NetScope*scope,
					 NetNet*net,
					 perm_string method_name,
					 const char *sys_task_name,
				         const std::vector<perm_string> &parm_names = {}) const;
      NetProc*elaborate_queue_method_(Design*des, NetScope*scope,
				      NetNet*net,
				      perm_string method_name,
				      const char *sys_task_name,
				      const std::vector<perm_string> &parm_names) const;
      NetProc*elaborate_method_func_(NetScope*scope,
				     NetNet*net,
				     ivl_type_t type,
				     perm_string method_name,
				     const char*sys_task_name) const;
      bool test_task_calls_ok_(Design*des, NetScope*scope) const;

      PPackage*package_;
      pform_name_t path_;
      std::vector<named_pexpr_t> parms_;
      bool void_cast_ = false;
};

class PCase  : public Statement {

    public:
      struct Item {
	    std::list<PExpr*>expr;
	    Statement*stat;
      };

      PCase(ivl_case_quality_t, NetCase::TYPE, PExpr*ex, std::vector<Item*>*);
      ~PCase();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      ivl_case_quality_t quality_;
      NetCase::TYPE type_;
      PExpr*expr_;

      std::vector<Item*>*items_;

    private: // not implemented
      PCase(const PCase&);
      PCase& operator= (const PCase&);
};

class PCAssign  : public Statement {

    public:
      explicit PCAssign(PExpr*l, PExpr*r);
      ~PCAssign();

      virtual NetCAssign* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
      PExpr*expr_;
};

/*
 * This represents the syntax "super.new(...)". This is not really an
 * executable statement, but the elaborator will handle these
 * specially and will remove them from the statement stream. If any
 */
class PChainConstructor : public Statement {
    public:
      explicit PChainConstructor(const std::list<named_pexpr_t> &parms);
      explicit PChainConstructor(const std::vector<named_pexpr_t> &parms);
      ~PChainConstructor();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

      inline const std::vector<named_pexpr_t>& chain_args(void) const
      { return parms_; }

    private:
      std::vector<named_pexpr_t> parms_;
};

class PCondit  : public Statement {

    public:
      PCondit(PExpr*ex, Statement*i, Statement*e);
      ~PCondit();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*if_;
      Statement*else_;

    private: // not implemented
      PCondit(const PCondit&);
      PCondit& operator= (const PCondit&);
};

class PContinue : public Statement {

    public:
      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
};

class PDeassign  : public Statement {

    public:
      explicit PDeassign(PExpr*l);
      ~PDeassign();

      virtual NetDeassign* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
};

class PDelayStatement  : public Statement {

    public:
      PDelayStatement(PExpr*d, Statement*st);
      ~PDelayStatement();

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;

    private:
      PExpr*delay_;
      Statement*statement_;
};


/*
 * This represents the parsing of a disable <scope> statement.
 */
class PDisable  : public Statement {

    public:
      explicit PDisable(const pform_name_t&sc);
      ~PDisable();

      virtual void dump(std::ostream&out, unsigned ind) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;

    private:
      pform_name_t scope_;
};

class PDoWhile  : public Statement {

    public:
      PDoWhile(PExpr*ex, Statement*st);
      ~PDoWhile();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*cond_;
      Statement*statement_;
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

      explicit PEventStatement(const std::vector<PEEvent*>&ee);
      explicit PEventStatement(PEEvent*ee);
	// Make an @* statement or make a special @* version with the items
	// from functions added and outputs removed for always_comb/latch.
      explicit PEventStatement(bool always_sens = false);

      ~PEventStatement();

      void set_statement(Statement*st);

      virtual void dump(std::ostream&out, unsigned ind) const;
	// Call this with a NULL statement only. It is used to print
	// the event expression for inter-assignment event controls.
      virtual void dump_inline(std::ostream&out) const;
      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;

      bool has_aa_term(Design*des, NetScope*scope);

	// This method is used to elaborate, but attach a previously
	// elaborated statement to the event.
      NetProc* elaborate_st(Design*des, NetScope*scope, NetProc*st) const;

      NetProc* elaborate_wait(Design*des, NetScope*scope, NetProc*st) const;
      NetProc* elaborate_wait_fork(Design*des, NetScope*scope) const;

    private:
      std::vector<PEEvent*>expr_;
      Statement*statement_;
      bool always_sens_;
};

std::ostream& operator << (std::ostream&o, const PEventStatement&obj);

class PForce  : public Statement {

    public:
      explicit PForce(PExpr*l, PExpr*r);
      ~PForce();

      virtual NetForce* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
      PExpr*expr_;
};

class PForeach : public Statement {
    public:
      explicit PForeach(perm_string var, const std::list<perm_string>&ix, Statement*stmt);
      ~PForeach();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      NetProc* elaborate_static_array_(Design*des, NetScope*scope,
				       const netranges_t&dims) const;

    private:
      perm_string array_var_;
      std::vector<perm_string> index_vars_;
      Statement*statement_;
};

class PForever : public Statement {
    public:
      explicit PForever(Statement*s);
      ~PForever();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      Statement*statement_;
};

class PForStatement  : public Statement {

    public:
      PForStatement(PExpr*n1, PExpr*e1, PExpr*cond,
		    Statement*step, Statement*body);
      ~PForStatement();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr* name1_;
      PExpr* expr1_;

      PExpr*cond_;

      Statement*step_;

      Statement*statement_;
};

class PNoop  : public Statement {

    public:
      PNoop() { }
      ~PNoop() { }
};

class PRepeat : public Statement {
    public:
      explicit PRepeat(PExpr*expr, Statement*s);
      ~PRepeat();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
      Statement*statement_;
};

class PRelease  : public Statement {

    public:
      explicit PRelease(PExpr*l);
      ~PRelease();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*lval_;
};

class PReturn  : public Statement {

    public:
      explicit PReturn(PExpr*e);
      ~PReturn();

      NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*expr_;
};

/*
 * The PTrigger statement sends a trigger to a named event. Take the
 * name here.
 */
class PTrigger  : public Statement {

    public:
      explicit PTrigger(PPackage*pkg, const pform_name_t&ev, unsigned lexical_pos);
      ~PTrigger();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      pform_scoped_name_t event_;
      unsigned lexical_pos_;
};

class PNBTrigger  : public Statement {
    public:
      explicit PNBTrigger(const pform_name_t&ev, unsigned lexical_pos, PExpr*dly);
      ~PNBTrigger();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      pform_name_t event_;
      unsigned lexical_pos_;
      PExpr*dly_;
};

class PWhile  : public Statement {

    public:
      PWhile(PExpr*ex, Statement*st);
      ~PWhile();

      virtual NetProc* elaborate(Design*des, NetScope*scope) const;
      virtual void elaborate_scope(Design*des, NetScope*scope) const;
      virtual void elaborate_sig(Design*des, NetScope*scope) const;
      virtual void dump(std::ostream&out, unsigned ind) const;

    private:
      PExpr*cond_;
      Statement*statement_;
};

#endif /* IVL_Statement_H */
