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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: PExpr.h,v 1.42 2000/09/07 22:38:13 steve Exp $"
#endif

# include  <string>
# include  "netlist.h"
# include  "verinum.h"
# include  "verireal.h"
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

	// Procedural elaboration of the expression.
      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope) const;

	// Elaborate expressions that are the r-value of parameter
	// assignments. This elaboration follows the restrictions of
	// constant expressions and supports later overriding and
	// evaluation of parameters.
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// This method elaborate the expression as gates, for use in a
	// continuous assign or other wholly structural context.
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0 =Link::STRONG,
				    Link::strength_t drive1 =Link::STRONG)
	    const;

	// This method elaborates the expression as gates, but
	// restricted for use as l-values of continuous assignments.
      virtual NetNet* elaborate_lnet(Design*des, const string&path) const;

	// This attempts to evaluate a constant expression, and return
	// a verinum as a result. If the expression cannot be
	// evaluated, return 0.
      virtual verinum* eval_const(const Design*des, const string&path) const;

	// This method returns true if that expression is the same as
	// this expression. This method is used for comparing
	// expressions that must be structurally "identical".
      virtual bool is_the_same(const PExpr*that) const;

	// Return true if this expression is a valid constant
	// expression. the Module pointer is needed to find parameter
	// identifiers and any other module specific interpretations
	// of expresions.
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

      virtual void dump(ostream&) const;
      virtual NetNet* elaborate_lnet(Design*des, const string&path) const;
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*) const;
      virtual NetEConcat*elaborate_pexpr(Design*des, NetScope*) const;
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

class PEIdent : public PExpr {

    public:
      explicit PEIdent(const string&s);
      ~PEIdent();

      virtual void dump(ostream&) const;

	// Identifiers are allowed (with restrictions) is assign l-values.
      virtual NetNet* elaborate_lnet(Design*des, const string&path) const;

	// Structural r-values are OK.
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;

      virtual NetExpr*elaborate_expr(Design*des, NetScope*) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;

	// Elaborate the PEIdent as a port to a module. This method
	// only applies to Ident expressions.
      NetNet* elaborate_port(Design*des, NetScope*sc) const;

      virtual bool is_constant(Module*) const;
      verinum* eval_const(const Design*des, const string&path) const;

      string name() const;

    private:
      string text_;

    public:
	// Use these to support bit- and part-select operators.
      PExpr*msb_;
      PExpr*lsb_;

	// If this is a reference to a memory, this is the index
	// expression.
      PExpr*idx_;

      NetNet* elaborate_net_ram_(Design*des, const string&path,
				 NetMemory*mem, unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
};

class PENumber : public PExpr {

    public:
      explicit PENumber(verinum*vp);
      ~PENumber();

      const verinum& value() const;

      virtual void dump(ostream&) const;
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned lwidth,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetEConst*elaborate_expr(Design*des, NetScope*) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const string&path) const;

      virtual bool is_the_same(const PExpr*that) const;
      virtual bool is_constant(Module*) const;

    private:
      verinum*const value_;
};

class PEString : public PExpr {

    public:
      explicit PEString(const string&s);
      ~PEString();

      string value() const;
      virtual void dump(ostream&) const;
      virtual NetEConst*elaborate_expr(Design*des, NetScope*) const;
      virtual NetEConst*elaborate_pexpr(Design*des, NetScope*sc) const;

      virtual bool is_constant(Module*) const;

    private:
      const string text_;
};

class PEUnary : public PExpr {

    public:
      explicit PEUnary(char op, PExpr*ex);
      ~PEUnary();

      virtual void dump(ostream&out) const;
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetEUnary*elaborate_expr(Design*des, NetScope*) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const string&path) const;

      virtual bool is_constant(Module*) const;

    private:
      char op_;
      PExpr*expr_;
};

class PEBinary : public PExpr {

    public:
      explicit PEBinary(char op, PExpr*l, PExpr*r)
      : op_(op), left_(l), right_(r) { }

      virtual bool is_constant(Module*) const;

      virtual void dump(ostream&out) const;
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetEBinary*elaborate_expr(Design*des, NetScope*) const;
      virtual NetExpr*elaborate_pexpr(Design*des, NetScope*sc) const;
      virtual verinum* eval_const(const Design*des, const string&path) const;

    private:
      char op_;
      PExpr*left_;
      PExpr*right_;

      NetEBinary*elaborate_expr_base_(Design*, NetExpr*lp, NetExpr*rp) const;

      NetNet* elaborate_net_add_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_bit_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_cmp_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_div_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_log_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_mul_(Design*des, const string&path,
				 unsigned lwidth,
				 unsigned long rise,
				 unsigned long fall,
				 unsigned long decay) const;
      NetNet* elaborate_net_shift_(Design*des, const string&path,
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
      virtual NetNet* elaborate_net(Design*des, const string&path,
				    unsigned width,
				    unsigned long rise,
				    unsigned long fall,
				    unsigned long decay,
				    Link::strength_t drive0,
				    Link::strength_t drive1) const;
      virtual NetETernary*elaborate_expr(Design*des, NetScope*) const;
      virtual verinum* eval_const(const Design*des, const string&path) const;

    private:
      PExpr*expr_;
      PExpr*tru_;
      PExpr*fal_;
};

/*
 * This class represents a parsed call to a function, including calls
 * to system functions.
 */
class PECallFunction : public PExpr {
    public:
      explicit PECallFunction(const char*n, const svector<PExpr *> &parms);
      explicit PECallFunction(const char*n);
      ~PECallFunction();

      virtual void dump(ostream &) const;
      virtual NetExpr*elaborate_expr(Design*des, NetScope*scope) const;

    private:
      string name_;
      svector<PExpr *> parms_;

      NetExpr* elaborate_sfunc_(Design*des, NetScope*scope) const;
};

/*
 * $Log: PExpr.h,v $
 * Revision 1.42  2000/09/07 22:38:13  steve
 *  Support unary + and - in constants.
 *
 * Revision 1.41  2000/06/30 15:50:20  steve
 *  Allow unary operators in constant expressions.
 *
 * Revision 1.40  2000/06/13 05:22:16  steve
 *  Support concatenation in parameter expressions.
 *
 * Revision 1.39  2000/06/01 02:31:39  steve
 *  Parameters can be strings.
 *
 * Revision 1.38  2000/05/16 04:05:15  steve
 *  Module ports are really special PEIdent
 *  expressions, because a name can be used
 *  many places in the port list.
 *
 * Revision 1.37  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.36  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.35  2000/04/12 04:23:57  steve
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
 *
 * Revision 1.34  2000/04/01 21:40:22  steve
 *  Add support for integer division.
 *
 * Revision 1.33  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.32  2000/03/12 18:22:11  steve
 *  Binary and unary operators in parameter expressions.
 *
 * Revision 1.31  2000/03/12 04:35:22  steve
 *  Allow parameter identifiers in parameter expressions.
 *
 * Revision 1.30  2000/03/08 04:36:53  steve
 *  Redesign the implementation of scopes and parameters.
 *  I now generate the scopes and notice the parameters
 *  in a separate pass over the pform. Once the scopes
 *  are generated, I can process overrides and evalutate
 *  paremeters before elaboration begins.
 *
 * Revision 1.29  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.28  2000/02/16 03:58:27  steve
 *  Fix up width matching in structural bitwise operators.
 *
 * Revision 1.27  2000/01/13 03:35:35  steve
 *  Multiplication all the way to simulation.
 *
 * Revision 1.26  1999/12/16 03:46:39  steve
 *  Structural logical or.
 *
 * Revision 1.25  1999/11/21 00:13:08  steve
 *  Support memories in continuous assignments.
 *
 * Revision 1.24  1999/11/14 20:24:28  steve
 *  Add support for the LPM_CLSHIFT device.
 *
 * Revision 1.23  1999/11/05 21:45:19  steve
 *  Fix NetConst being set to zero width, and clean
 *  up elaborate_set_cmp_ for NetEBinary.
 *
 * Revision 1.22  1999/10/31 20:08:24  steve
 *  Include subtraction in LPM_ADD_SUB device.
 *
 * Revision 1.21  1999/10/31 04:11:27  steve
 *  Add to netlist links pin name and instance number,
 *  and arrange in vvm for pin connections by name
 *  and instance number.
 *
 * Revision 1.20  1999/09/25 02:57:29  steve
 *  Parse system function calls.
 *
 * Revision 1.19  1999/09/15 04:17:52  steve
 *  separate assign lval elaboration for error checking.
 *
 * Revision 1.18  1999/08/31 22:38:29  steve
 *  Elaborate and emit to vvm procedural functions.
 *
 * Revision 1.17  1999/08/01 21:18:55  steve
 *  elaborate rise/fall/decay for continuous assign.
 *
 * Revision 1.16  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.15  1999/07/22 02:05:20  steve
 *  is_constant method for PEConcat.
 *
 * Revision 1.14  1999/07/17 19:50:59  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.13  1999/06/16 03:13:29  steve
 *  More syntax parse with sorry stubs.
 *
 * Revision 1.12  1999/06/15 02:50:02  steve
 *  Add lexical support for real numbers.
 *
 * Revision 1.11  1999/06/10 04:03:52  steve
 *  Add support for the Ternary operator,
 *  Add support for repeat concatenation,
 *  Correct some seg faults cause by elaboration
 *  errors,
 *  Parse the casex anc casez statements.
 *
 * Revision 1.10  1999/06/09 03:00:05  steve
 *  Add support for procedural concatenation expression.
 *
 * Revision 1.9  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.8  1999/05/10 00:16:57  steve
 *  Parse and elaborate the concatenate operator
 *  in structural contexts, Replace vector<PExpr*>
 *  and list<PExpr*> with svector<PExpr*>, evaluate
 *  constant expressions with parameters, handle
 *  memories as lvalues.
 *
 *  Parse task declarations, integer types.
 *
 * Revision 1.7  1999/05/01 02:57:52  steve
 *  Handle much more complex event expressions.
 *
 * Revision 1.6  1999/04/29 02:16:26  steve
 *  Parse OR of event expressions.
 *
 * Revision 1.5  1999/04/19 01:59:36  steve
 *  Add memories to the parse and elaboration phases.
 *
 * Revision 1.4  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
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
 * Revision 1.1  1998/11/03 23:28:54  steve
 *  Introduce verilog to CVS.
 *
 */
#endif
