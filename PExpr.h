#ifndef __PExpr_H
#define __PExpr_H
/*
 * Copyright (c) 1998-2008 Stephen Williams <steve@icarus.com>
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

# include  <string>
# include  <vector>
# include  <valarray>
# include  "netlist.h"
# include  "verinum.h"
# include  "LineInfo.h"
# include  "pform_types.h"

class Design;
class Module;
class NetNet;
class NetExpr;
class NetScope;

/*
 * The PExpr class hierarchy supports the description of
 * expressions. The parser can generate expression objects from the
 * source, possibly reducing things that it knows how to reduce.
 */

class PExpr : public LineInfo {

    public:
      PExpr();
      virtual ~PExpr();

      virtual void dump(ostream&) const;

        // This method tests whether the expression contains any
        // references to automatically allocated variables.
      virtual bool has_aa_term(Design*des, NetScope*scope) const;

	// This method tests the width that the expression wants to
	// be. It is used by elaboration of assignments to figure out
	// the width of the expression.
	//
	// The "min" is the width of the local context, so is the
	// minimum width that this function should return. Initially
	// this is the same as the lval width.
	//
	// The "lval" is the width of the destination where this
	// result is going to go. This can be used to constrain the
	// amount that an expression can reasonably expand. For
	// example, there is no point expanding an addition to beyond
	// the lval. This extra bit of information allows the
	// expression to optimize itself a bit. If the lval==0, then
	// the subexpression should not make l-value related
	// optimizations.
	//
	// The expr_type is an output argument that gives the
	// calculated type for the expression.
	//
	// The unsized_flag is set to true if the expression is
	// unsized and therefore expandable. This happens if a
	// sub-expression is an unsized literal. Some expressions make
	// special use of that.
      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

	// After the test_width method is complete, these methods
	// return valid results.
      ivl_variable_type_t expr_type() const { return expr_type_; }
      unsigned expr_width() const           { return expr_width_; }

	// During the elaborate_sig phase, we may need to scan
	// expressions to find implicit net declarations.
      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

	// Procedural elaboration of the expression. The expr_width is
	// the width of the context of the expression (i.e. the
	// l-value width of an assignment),
	//
	// ... or -1 if the expression is self-determined. or
	// ... or -2 if the expression is losslessly
	// self-determined. This can happen in situations where the
	// result is going to a pseudo-infinitely wide context.
	//
	// The sys_task_arg flag is true if expressions are allowed to
	// be incomplete.
      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope,
				     int expr_width, bool sys_task_arg) const;

	// Elaborate expressions that are the r-value of parameter
	// assignments. This elaboration follows the restrictions of
	// constant expressions and supports later overriding and
	// evaluation of parameters.
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// This method elaborates the expression as gates, but
	// restricted for use as l-values of continuous assignments.
      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope) const;

	// This is similar to elaborate_lnet, except that the
	// expression is evaluated to be bi-directional. This is
	// useful for arguments to inout ports of module instances and
	// ports of tran primitives.
      virtual NetNet* elaborate_bi_net(Design*des, NetScope*scope) const;

	// Expressions that can be in the l-value of procedural
	// assignments can be elaborated with this method. If the
	// is_force flag is true, then the set of valid l-value types
	// is slightly modified to accommodate the Verilog force
	// statement
      virtual NetAssign_* elaborate_lval(Design*des,
					 NetScope*scope,
					 bool is_force) const;

	// This attempts to evaluate a constant expression, and return
	// a verinum as a result. If the expression cannot be
	// evaluated, return 0.
      virtual verinum* eval_const(Design*des, NetScope*sc) const;

	// This method returns true if that expression is the same as
	// this expression. This method is used for comparing
	// expressions that must be structurally "identical".
      virtual bool is_the_same(const PExpr*that) const;

    protected:
	// The derived class test_width methods should fill these in.
      ivl_variable_type_t expr_type_;
      unsigned expr_width_;

    private: // not implemented
      PExpr(const PExpr&);
      PExpr& operator= (const PExpr&);
};

ostream& operator << (ostream&, const PExpr&);

class PEConcat : public PExpr {

    public:
      PEConcat(const svector<PExpr*>&p, PExpr*r =0);
      ~PEConcat();

      virtual verinum* eval_const(Design*des, NetScope*sc) const;
      virtual void dump(ostream&) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual bool elaborate_sig(Design*des, NetScope*scope) const;
      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope) const;
      virtual NetNet* elaborate_bi_net(Design*des, NetScope*scope) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     int expr_width, bool sys_task_arg) const;
      virtual NetEConcat*elaborate_pexpr(Design*des, NetScope*) const;
      virtual NetAssign_* elaborate_lval(Design*des,
					 NetScope*scope,
					 bool is_force) const;
    private:
      NetNet* elaborate_lnet_common_(Design*des, NetScope*scope,
				     bool bidirectional_flag) const;
    private:
      svector<PExpr*>parms_;
      std::valarray<unsigned>tested_widths_;

      PExpr*repeat_;

};

/*
 * Event expressions are expressions that can be combined with the
 * event "or" operator. These include "posedge foo" and similar, and
 * also include named events. "edge" events are associated with an
 * expression, whereas named events simply have a name, which
 * represents an event variable.
 */
class PEEvent : public PExpr {

    public:
      enum edge_t {ANYEDGE, POSEDGE, NEGEDGE, POSITIVE};

	// Use this constructor to create events based on edges or levels.
      PEEvent(edge_t t, PExpr*e);

      ~PEEvent();

      edge_t type() const;
      PExpr* expr() const;

      virtual void dump(ostream&) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

    private:
      edge_t type_;
      PExpr *expr_;
};

/*
 * This holds a floating point constant in the source.
 */
class PEFNumber : public PExpr {

    public:
      explicit PEFNumber(verireal*vp);
      ~PEFNumber();

      const verireal& value() const;

	/* The eval_const method as applied to a floating point number
	   gets the *integer* value of the number. This accounts for
	   any rounding that is needed to get the value. */
      virtual verinum* eval_const(Design*des, NetScope*sc) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);
      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     int expr_width, bool sys_task_arg) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

      virtual void dump(ostream&) const;

    private:
      verireal*value_;
};

class PEIdent : public PExpr {

    public:
      explicit PEIdent(perm_string);
      explicit PEIdent(const pform_name_t&);
      ~PEIdent();

	// Add another name to the string of hierarchy that is the
	// current identifier.
      void append_name(perm_string);

      virtual void dump(ostream&) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

	// Identifiers are allowed (with restrictions) is assign l-values.
      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope) const;

      virtual NetNet* elaborate_bi_net(Design*des, NetScope*scope) const;

	// Identifiers are also allowed as procedural assignment l-values.
      virtual NetAssign_* elaborate_lval(Design*des,
					 NetScope*scope,
					 bool is_force) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     int expr_width, bool sys_task_arg) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// Elaborate the PEIdent as a port to a module. This method
	// only applies to Ident expressions.
      NetNet* elaborate_port(Design*des, NetScope*sc) const;

      verinum* eval_const(Design*des, NetScope*sc) const;

      const pform_name_t& path() const { return path_; }

    private:
      pform_name_t path_;

    private:
	// Common functions to calculate parts of part/bit selects.
      bool calculate_parts_(Design*, NetScope*, long&msb, long&lsb) const;
      NetExpr* calculate_up_do_base_(Design*, NetScope*) const;
      bool calculate_param_range_(Design*, NetScope*,
				  const NetExpr*msb_ex, long&msb,
				  const NetExpr*lsb_ex, long&lsb) const;

      bool calculate_up_do_width_(Design*, NetScope*, unsigned long&wid) const;

    private:
      NetAssign_*elaborate_lval_net_word_(Design*, NetScope*, NetNet*) const;
      bool elaborate_lval_net_bit_(Design*, NetScope*, NetAssign_*) const;
      bool elaborate_lval_net_part_(Design*, NetScope*, NetAssign_*) const;
      bool elaborate_lval_net_idx_(Design*, NetScope*, NetAssign_*,
                                   index_component_t::ctype_t) const;

    private:
      NetExpr*elaborate_expr_param_(Design*des,
				    NetScope*scope,
				    const NetExpr*par,
				    NetScope*found,
				    const NetExpr*par_msb,
				    const NetExpr*par_lsb,
				    int expr_wid) const;
      NetExpr*elaborate_expr_param_part_(Design*des,
					 NetScope*scope,
					 const NetExpr*par,
					 NetScope*found,
					 const NetExpr*par_msb,
					 const NetExpr*par_lsb) const;
      NetExpr*elaborate_expr_param_idx_up_(Design*des,
					   NetScope*scope,
					   const NetExpr*par,
					   NetScope*found,
					   const NetExpr*par_msb,
					   const NetExpr*par_lsb) const;
      NetExpr*elaborate_expr_net(Design*des,
				 NetScope*scope,
				 NetNet*net,
				 NetScope*found,
				 bool sys_task_arg) const;
      NetExpr*elaborate_expr_net_word_(Design*des,
				       NetScope*scope,
				       NetNet*net,
				       NetScope*found,
				       bool sys_task_arg) const;
      NetExpr*elaborate_expr_net_part_(Design*des,
				   NetScope*scope,
				   NetESignal*net,
				   NetScope*found) const;
      NetExpr*elaborate_expr_net_idx_up_(Design*des,
				   NetScope*scope,
				   NetESignal*net,
				   NetScope*found) const;
      NetExpr*elaborate_expr_net_idx_do_(Design*des,
				   NetScope*scope,
				   NetESignal*net,
				   NetScope*found) const;
      NetExpr*elaborate_expr_net_bit_(Design*des,
				   NetScope*scope,
				   NetESignal*net,
				   NetScope*found) const;

    private:
      NetNet* elaborate_lnet_common_(Design*des, NetScope*scope,
				     bool bidirectional_flag) const;

      NetNet*make_implicit_net_(Design*des, NetScope*scope) const;

      bool eval_part_select_(Design*des, NetScope*scope, NetNet*sig,
			     long&midx, long&lidx) const;
};

class PENumber : public PExpr {

    public:
      explicit PENumber(verinum*vp);
      ~PENumber();

      const verinum& value() const;

      virtual void dump(ostream&) const;
      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual NetEConst*elaborate_expr(Design*des, NetScope*,
				       int expr_width, bool) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual NetAssign_* elaborate_lval(Design*des,
					 NetScope*scope,
					 bool is_force) const;

      virtual verinum* eval_const(Design*des, NetScope*sc) const;

      virtual bool is_the_same(const PExpr*that) const;

    private:
      verinum*const value_;
};

/*
 * This represents a string constant in an expression.
 *
 * The s parameter to the PEString constructor is a C string that this
 * class instance will take for its own. The caller should not delete
 * the string, the destructor will do it.
 */
class PEString : public PExpr {

    public:
      explicit PEString(char*s);
      ~PEString();

      string value() const;
      virtual void dump(ostream&) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual NetEConst*elaborate_expr(Design*des, NetScope*,
				       int expr_width, bool) const;
      virtual NetEConst*elaborate_pexpr(Design*des, NetScope*sc) const;
      verinum* eval_const(Design*, NetScope*) const;

    private:
      char*text_;
};

class PEUnary : public PExpr {

    public:
      explicit PEUnary(char op, PExpr*ex);
      ~PEUnary();

      virtual void dump(ostream&out) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     int expr_width, bool sys_task_arg) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(Design*des, NetScope*sc) const;

    private:
      NetExpr* elaborate_expr_bits_(NetExpr*operand, int expr_wid) const;

    private:
      char op_;
      PExpr*expr_;
};

class PEBinary : public PExpr {

    public:
      explicit PEBinary(char op, PExpr*l, PExpr*r);
      ~PEBinary();

      virtual void dump(ostream&out) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
					int expr_width, bool sys_task_arg) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(Design*des, NetScope*sc) const;

    protected:
      char op_;
      PExpr*left_;
      PExpr*right_;

      NetExpr*elaborate_expr_base_(Design*, NetExpr*lp, NetExpr*rp,
				   int use_wid, bool is_pexpr =false) const;
      NetExpr*elaborate_eval_expr_base_(Design*, NetExpr*lp, NetExpr*rp,
					int use_wid) const;

      NetExpr*elaborate_expr_base_bits_(Design*, NetExpr*lp, NetExpr*rp, int use_wid) const;
      NetExpr*elaborate_expr_base_div_(Design*, NetExpr*lp, NetExpr*rp,
				       int use_wid, bool is_pexpr) const;
      NetExpr*elaborate_expr_base_lshift_(Design*, NetExpr*lp, NetExpr*rp, int use_wid) const;
      NetExpr*elaborate_expr_base_rshift_(Design*, NetExpr*lp, NetExpr*rp, int use_wid) const;
      NetExpr*elaborate_expr_base_mult_(Design*, NetExpr*lp, NetExpr*rp,
					int use_wid, bool is_pexpr) const;
      NetExpr*elaborate_expr_base_add_(Design*, NetExpr*lp, NetExpr*rp,
				       int use_wid, bool is_pexpr) const;

};

/*
 * Here are a few specialized classes for handling specific binary
 * operators.
 */
class PEBComp  : public PEBinary {

    public:
      explicit PEBComp(char op, PExpr*l, PExpr*r);
      ~PEBComp();

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&flag);

      NetExpr* elaborate_expr(Design*des, NetScope*scope,
			      int expr_width, bool sys_task_arg) const;
      NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
};

/*
 * This derived class is for handling logical expressions: && and ||.
*/
class PEBLogic  : public PEBinary {

    public:
      explicit PEBLogic(char op, PExpr*l, PExpr*r);
      ~PEBLogic();

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&flag);

      NetExpr* elaborate_expr(Design*des, NetScope*scope,
			      int expr_width, bool sys_task_arg) const;
      NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
};

/*
 * A couple of the binary operands have a special sub-expression rule
 * where the expression width is carried entirely by the left
 * expression, and the right operand is self-determined.
 */
class PEBLeftWidth  : public PEBinary {

    public:
      explicit PEBLeftWidth(char op, PExpr*l, PExpr*r);
      ~PEBLeftWidth() =0;

      virtual NetExpr*elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
					  int expr_wid) const =0;

    protected:
      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&flag);

      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope,
				     int expr_width, bool sys_task_arg) const;

      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*scope) const;

};

class PEBPower  : public PEBLeftWidth {

    public:
      explicit PEBPower(char op, PExpr*l, PExpr*r);
      ~PEBPower();

      NetExpr*elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
				  int expr_wid) const;
};

class PEBShift  : public PEBLeftWidth {

    public:
      explicit PEBShift(char op, PExpr*l, PExpr*r);
      ~PEBShift();

      NetExpr*elaborate_expr_leaf(Design*des, NetExpr*lp, NetExpr*rp,
				  int expr_wid) const;
};

/*
 * This class supports the ternary (?:) operator. The operator takes
 * three expressions, the test, the true result and the false result.
 */
class PETernary : public PExpr {

    public:
      explicit PETernary(PExpr*e, PExpr*t, PExpr*f);
      ~PETernary();

      virtual void dump(ostream&out) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

      virtual bool elaborate_sig(Design*des, NetScope*scope) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
					 int expr_width, bool sys_task_arg) const;
      virtual NetETernary*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(Design*des, NetScope*sc) const;

    private:
      PExpr*expr_;
      PExpr*tru_;
      PExpr*fal_;
};

/*
 * This class represents a parsed call to a function, including calls
 * to system functions. The parameters in the parms list are the
 * expressions that are passed as input to the ports of the function.
 */
class PECallFunction : public PExpr {
    public:
      explicit PECallFunction(const pform_name_t&n, const vector<PExpr *> &parms);
	// Call of system function (name is not hierarchical)
      explicit PECallFunction(perm_string n, const vector<PExpr *> &parms);
      explicit PECallFunction(perm_string n);

	// svector versions. Should be removed!
      explicit PECallFunction(const pform_name_t&n, const svector<PExpr *> &parms);
      explicit PECallFunction(perm_string n, const svector<PExpr *> &parms);

      ~PECallFunction();

      virtual void dump(ostream &) const;

      virtual bool has_aa_term(Design*des, NetScope*scope) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope,
				     int expr_wid, bool sys_task_arg) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

      virtual unsigned test_width(Design*des, NetScope*scope,
				  unsigned min, unsigned lval,
				  ivl_variable_type_t&expr_type,
				  bool&unsized_flag);

    private:
      pform_name_t path_;
      vector<PExpr *> parms_;

      bool check_call_matches_definition_(Design*des, NetScope*dscope) const;

      NetExpr* elaborate_sfunc_(Design*des, NetScope*scope, int expr_wid) const;
      NetExpr* elaborate_access_func_(Design*des, NetScope*scope, ivl_nature_t) const;
      unsigned test_width_sfunc_(Design*des, NetScope*scope,
				 unsigned min, unsigned lval,
				 ivl_variable_type_t&expr_type,
				 bool&unsized_flag);
};

#endif
