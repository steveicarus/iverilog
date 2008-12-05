#ifndef __PExpr_H
#define __PExpr_H
/*
 * Copyright (c) 1998-2000 Stephen Williams <steve@icarus.com>
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
#ident "$Id: PExpr.h,v 1.66.2.2 2006/07/28 16:26:17 steve Exp $"
#endif

# include  <string>
# include  "netlist.h"
# include  "verinum.h"
# include  "LineInfo.h"

class Design;
class Module;
class NetNet;
class NetExpr;
class NetScope;

/*
 * The PExpr class hierarchy supports the description of
 * expressions. The parser can generate expression objects from the
 * source, possibly reducing things that it knows how to reduce.
 *
 * The elaborate_net method is used by structural elaboration to build
 * up a netlist interpretation of the expression.
 */

class PExpr : public LineInfo {

    public:
      PExpr();
      virtual ~PExpr();

      virtual void dump(ostream&) const;

	// Procedural elaboration of the expression. Set the
	// bare_memory_ok flag if the result is allowed to be a
	// NetEMemory without an index.
      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope,
				     bool sys_task_arg =false) const;

	// Elaborate expressions that are the r-value of parameter
	// assignments. This elaboration follows the restrictions of
	// constant expressions and supports later overriding and
	// evaluation of parameters.
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// This method elaborate the expression as gates, for use in a
	// continuous assign or other wholly structural context.
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0 =Link::STRONG,
				    Link::strength_t drive1 =Link::STRONG)
	    const;

	// This method elaborates the expression as NetNet objects. It
	// only allows regs suitable for procedural continuous assignments.
      virtual NetNet* elaborate_anet(Design*des, NetScope*scope) const;

	// This method elaborates the expression as gates, but
	// restricted for use as l-values of continuous assignments.
      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope,
				     bool implicit_net_ok =false) const;

	// Expressions that can be in the l-value of procedural
	// assignments can be elaborated with this method.
      virtual NetAssign_* elaborate_lval(Design*des, NetScope*scope) const;

	// This attempts to evaluate a constant expression, and return
	// a verinum as a result. If the expression cannot be
	// evaluated, return 0.
      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

	// This method returns true if that expression is the same as
	// this expression. This method is used for comparing
	// expressions that must be structurally "identical".
      virtual bool is_the_same(const PExpr*that) const;

	// Return true if this expression is a valid constant
	// expression. the Module pointer is needed to find parameter
	// identifiers and any other module specific interpretations
	// of expressions.
      virtual bool is_constant(Module*) const;

    private: // not implemented
      PExpr(const PExpr&);
      PExpr& operator= (const PExpr&);
};

ostream& operator << (ostream&, const PExpr&);

class PEConcat : public PExpr {

    public:
      PEConcat(const svector<PExpr*>&p, PExpr*r =0);
      ~PEConcat();

      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

      virtual void dump(ostream&) const;

	// Concatenated Regs can be on the left of procedural
	// continuous assignments.
      virtual NetNet* elaborate_anet(Design*des, NetScope*scope) const;

      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope,
				     bool implicit_net_ok =false) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetEConcat*elaborate_pexpr(Design*des, NetScope*) const;
      virtual NetAssign_* elaborate_lval(Design*des, NetScope*scope) const;
      virtual bool is_constant(Module*) const;

    private:
      svector<PExpr*>parms_;
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
      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

	/* A PEFNumber is a constant, so this returns true. */
      virtual bool is_constant(Module*) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

      virtual void dump(ostream&) const;

    private:
      verireal*value_;
};

class PEIdent : public PExpr {

    public:
      explicit PEIdent(const hname_t&s);
      ~PEIdent();

      virtual void dump(ostream&) const;

	// Regs can be on the left of procedural continuous assignments
      virtual NetNet* elaborate_anet(Design*des, NetScope*scope) const;

	// Identifiers are allowed (with restrictions) is assign l-values.
      virtual NetNet* elaborate_lnet(Design*des, NetScope*scope,
				     bool implicit_net_ok =false) const;

	// Identifiers are also allowed as procedural assignment l-values.
      virtual NetAssign_* elaborate_lval(Design*des, NetScope*scope) const;

	// Structural r-values are OK.
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// Elaborate the PEIdent as a port to a module. This method
	// only applies to Ident expressions.
      NetNet* elaborate_port(Design*des, NetScope*sc) const;

      virtual bool is_constant(Module*) const;
      verinum* eval_const(const Design*des, const NetScope*sc) const;

      const hname_t& path() const;

    private:
      hname_t path_;

    public:
	// Use these to support bit- and part-select operators.
      PExpr*msb_;
      PExpr*lsb_;

	// If this is a reference to a memory, this is the index
	// expression.
      PExpr*idx_;

      NetNet* elaborate_net_ram_(Design*des, NetScope*scope,
				 NetMemory*mem, unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;

      NetNet* elaborate_net_bitmux_(Design*des, NetScope*scope,
				    NetNet*sig,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;

    private:
      NetAssign_* elaborate_mem_lval_(Design*des, NetScope*scope,
				      NetMemory*mem) const;

};

class PENumber : public PExpr {

    public:
      explicit PENumber(verinum*vp);
      ~PENumber();

      const verinum& value() const;

      virtual void dump(ostream&) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetEConst*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual NetAssign_* elaborate_lval(Design*des, NetScope*scope) const;

      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

      virtual bool is_the_same(const PExpr*that) const;
      virtual bool is_constant(Module*) const;

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
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetEConst*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetEConst*elaborate_pexpr(Design*des, NetScope*sc) const;
      verinum* eval_const(const Design*, const NetScope*) const;

      virtual bool is_constant(Module*) const;

    private:
      char*text_;
};

class PEUnary : public PExpr {

    public:
      explicit PEUnary(char op, PExpr*ex);
      ~PEUnary();

      virtual void dump(ostream&out) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
				     bool sys_task_arg =false) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

      virtual bool is_constant(Module*) const;

    private:
      char op_;
      PExpr*expr_;
};

class PEBinary : public PExpr {

    public:
      explicit PEBinary(char op, PExpr*l, PExpr*r);
      ~PEBinary();

      virtual bool is_constant(Module*) const;

      virtual void dump(ostream&out) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*,
					bool sys_task_arg =false) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

    private:
      char op_;
      PExpr*left_;
      PExpr*right_;

      NetExpr*elaborate_expr_base_(Design*, NetExpr*lp, NetExpr*rp) const;

      NetNet* elaborate_net_add_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_bit_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_cmp_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_div_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_mod_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_log_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_mul_(Design*des, NetScope*scope,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_shift_(Design*des, NetScope*scope,
				   unsigned lwidth,
				   unsigned long rise,
				   unsigned long fall,
				   unsigned long decay) const;
};

/*
 * This class supports the ternary (?:) operator. The operator takes
 * three expressions, the test, the true result and the false result.
 */
class PETernary : public PExpr {

    public:
      explicit PETernary(PExpr*e, PExpr*t, PExpr*f);
      ~PETernary();

      virtual bool is_constant(Module*) const;

      virtual void dump(ostream&out) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetETernary*elaborate_expr(Design*des, NetScope*,
					 bool sys_task_arg =false) const;
      virtual NetETernary*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const NetScope*sc) const;

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
      explicit PECallFunction(const hname_t&n, const svector<PExpr *> &parms);
      explicit PECallFunction(const hname_t&n);
      ~PECallFunction();

      virtual void dump(ostream &) const;
      virtual NetNet* elaborate_net(Design*des, NetScope*scope,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope,
				     bool sys_task_arg =false) const;

    private:
      hname_t path_;
      svector<PExpr *> parms_;

      bool check_call_matches_definition_(Design*des, NetScope*dscope) const;

      NetExpr* elaborate_sfunc_(Design*des, NetScope*scope) const;
};

/*
 * $Log: PExpr.h,v $
 * Revision 1.66.2.2  2006/07/28 16:26:17  steve
 *  Remove excess PEString:: prefix for stubborn compilers.
 *
 * Revision 1.66.2.1  2005/12/07 03:28:44  steve
 *  Support constant concatenation of constants.
 *
 * Revision 1.66  2004/10/04 01:10:51  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.65  2003/02/08 19:49:21  steve
 *  Calculate delay statement delays using elaborated
 *  expressions instead of pre-elaborated expression
 *  trees.
 *
 *  Remove the eval_pexpr methods from PExpr.
 *
 * Revision 1.64  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.63  2002/11/09 19:20:48  steve
 *  Port expressions for output ports are lnets, not nets.
 *
 * Revision 1.62  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.61  2002/06/04 05:38:43  steve
 *  Add support for memory words in l-value of
 *  blocking assignments, and remove the special
 *  NetAssignMem class.
 *
 * Revision 1.60  2002/05/23 03:08:51  steve
 *  Add language support for Verilog-2001 attribute
 *  syntax. Hook this support into existing $attribute
 *  handling, and add number and void value types.
 *
 *  Add to the ivl_target API new functions for access
 *  of complex attributes attached to gates.
 *
 * Revision 1.59  2002/04/23 03:53:59  steve
 *  Add support for non-constant bit select.
 *
 * Revision 1.58  2002/04/14 03:55:25  steve
 *  Precalculate unary - if possible.
 *
 * Revision 1.57  2002/04/13 02:33:17  steve
 *  Detect missing indices to memories (PR#421)
 *
 * Revision 1.56  2002/03/09 04:02:26  steve
 *  Constant expressions are not l-values for task ports.
 *
 * Revision 1.55  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.54  2001/12/30 21:32:03  steve
 *  Support elaborate_net for PEString objects.
 *
 * Revision 1.53  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.52  2001/11/08 05:15:50  steve
 *  Remove string paths from PExpr elaboration.
 *
 * Revision 1.51  2001/11/07 04:26:46  steve
 *  elaborate_lnet uses scope instead of string path.
 *
 * Revision 1.50  2001/11/07 04:01:59  steve
 *  eval_const uses scope instead of a string path.
 */
#endif
