/*
 * Copyright (c) 1998-1999 Stephen Williams <steve@icarus.com>
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
#ident "$Id: PExpr.cc,v 1.34 2004/02/20 06:22:56 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "PExpr.h"
# include  "Module.h"
# include  <typeinfo>

PExpr::PExpr()
{
}

PExpr::~PExpr()
{
}

bool PExpr::is_the_same(const PExpr*that) const
{
      return typeid(this) == typeid(that);
}

bool PExpr::is_constant(Module*) const
{
      return false;
}

NetNet* PExpr::elaborate_lnet(Design*des, NetScope*, bool) const
{
      cerr << get_line() << ": error: expression not valid in assign l-value: "
	   << *this << endl;
      return 0;
}

PEBinary::PEBinary(char op, PExpr*l, PExpr*r)
: op_(op), left_(l), right_(r)
{
}

PEBinary::~PEBinary()
{
}

bool PEBinary::is_constant(Module*mod) const
{
      return left_->is_constant(mod) && right_->is_constant(mod);
}

PECallFunction::PECallFunction(const hname_t&n, const svector<PExpr *> &parms) 
: path_(n), parms_(parms)
{
}

PECallFunction::PECallFunction(const hname_t&n) 
: path_(n)
{
}

PECallFunction::~PECallFunction()
{
}

PEConcat::PEConcat(const svector<PExpr*>&p, PExpr*r)
: parms_(p), repeat_(r)
{
}

bool PEConcat::is_constant(Module *mod) const
{
      bool constant = repeat_? repeat_->is_constant(mod) : true;
      for (unsigned i = 0; constant && i < parms_.count(); ++i) {
	    constant = constant && parms_[i]->is_constant(mod);
      }
      return constant;
}

PEConcat::~PEConcat()
{
      delete repeat_;
}

PEEvent::PEEvent(PEEvent::edge_t t, PExpr*e)
: type_(t), expr_(e)
{
}

PEEvent::~PEEvent()
{
}

PEEvent::edge_t PEEvent::type() const
{
      return type_;
}

PExpr* PEEvent::expr() const
{
      return expr_;
}

PEFNumber::PEFNumber(verireal*v)
: value_(v)
{
}

PEFNumber::~PEFNumber()
{
      delete value_;
}

const verireal& PEFNumber::value() const
{
      return *value_;
}

bool PEFNumber::is_constant(Module*) const
{
      return true;
}

PEIdent::PEIdent(const hname_t&s)
: path_(s), msb_(0), lsb_(0), idx_(0)
{
}

PEIdent::~PEIdent()
{
}

const hname_t& PEIdent::path() const
{
      return path_;
}

/*
 * An identifier can be in a constant expression if (and only if) it is
 * a parameter.
 */
bool PEIdent::is_constant(Module*mod) const
{
      if (mod == 0) return false;

	/* This is a work-around for map not matching < even when
	   there is a perm_string operator that can do the comprare.

	   The real fix is to make the path_ carry perm_strings. */
      perm_string tmp = perm_string::literal(path_.peek_name(0));

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->parameters.find(tmp);
	if (cur != mod->parameters.end()) return true;
      }

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->localparams.find(tmp);
	if (cur != mod->localparams.end()) return true;
      }

      return false;
}

PENumber::PENumber(verinum*vp)
: value_(vp)
{
      assert(vp);
}

PENumber::~PENumber()
{
      delete value_;
}

const verinum& PENumber::value() const
{
      return *value_;
}

bool PENumber::is_the_same(const PExpr*that) const
{
      const PENumber*obj = dynamic_cast<const PENumber*>(that);
      if (obj == 0)
	    return false;

      return *value_ == *obj->value_;
}

bool PENumber::is_constant(Module*) const
{
      return true;
}

PEString::PEString(char*s)
: text_(s)
{
}

PEString::~PEString()
{
      delete[]text_;
}

string PEString::value() const
{
      return text_;
}

bool PEString::is_constant(Module*) const
{
      return true;
}

PETernary::PETernary(PExpr*e, PExpr*t, PExpr*f)
: expr_(e), tru_(t), fal_(f)
{
}

PETernary::~PETernary()
{
}

bool PETernary::is_constant(Module*m) const
{
      return expr_->is_constant(m)
	    && tru_->is_constant(m)
	    && fal_->is_constant(m);
}

PEUnary::PEUnary(char op, PExpr*ex)
: op_(op), expr_(ex)
{
}

PEUnary::~PEUnary()
{
}

bool PEUnary::is_constant(Module*m) const
{
      return expr_->is_constant(m);
}

/*
 * $Log: PExpr.cc,v $
 * Revision 1.34  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.33  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.32  2002/11/09 19:20:48  steve
 *  Port expressions for output ports are lnets, not nets.
 *
 * Revision 1.31  2002/08/19 02:39:16  steve
 *  Support parameters with defined ranges.
 *
 * Revision 1.30  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.29  2001/12/30 21:32:03  steve
 *  Support elaborate_net for PEString objects.
 *
 * Revision 1.28  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.27  2001/11/08 05:15:50  steve
 *  Remove string paths from PExpr elaboration.
 *
 * Revision 1.26  2001/11/07 04:26:46  steve
 *  elaborate_lnet uses scope instead of string path.
 *
 * Revision 1.25  2001/11/06 06:11:55  steve
 *  Support more real arithmetic in delay constants.
 *
 * Revision 1.24  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.23  2001/01/14 23:04:55  steve
 *  Generalize the evaluation of floating point delays, and
 *  get it working with delay assignment statements.
 *
 *  Allow parameters to be referenced by hierarchical name.
 *
 * Revision 1.22  2001/01/12 04:31:27  steve
 *  Handle error idents in constants not in any scope (PR#97)
 *
 * Revision 1.21  2000/12/16 19:03:30  steve
 *  Evaluate <= and ?: in parameter expressions (PR#81)
 *
 * Revision 1.20  2000/12/10 22:01:35  steve
 *  Support decimal constants in behavioral delays.
 *
 * Revision 1.19  2000/06/30 15:50:20  steve
 *  Allow unary operators in constant expressions.
 *
 * Revision 1.18  2000/05/07 04:37:55  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.17  2000/05/04 03:37:58  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 * Revision 1.16  2000/04/12 04:23:57  steve
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
 * Revision 1.15  2000/04/01 19:31:57  steve
 *  Named events as far as the pform.
 *
 * Revision 1.14  2000/03/12 18:22:11  steve
 *  Binary and unary operators in parameter expressions.
 *
 * Revision 1.13  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.12  1999/12/31 17:38:37  steve
 *  Standardize some of the error messages.
 *
 * Revision 1.11  1999/10/31 04:11:27  steve
 *  Add to netlist links pin name and instance number,
 *  and arrange in vvm for pin connections by name
 *  and instance number.
 *
 * Revision 1.10  1999/09/25 02:57:29  steve
 *  Parse system function calls.
 *
 * Revision 1.9  1999/09/16 04:18:15  steve
 *  elaborate concatenation repeats.
 *
 * Revision 1.8  1999/09/15 04:17:52  steve
 *  separate assign lval elaboration for error checking.
 *
 * Revision 1.7  1999/07/22 02:05:20  steve
 *  is_constant method for PEConcat.
 *
 * Revision 1.6  1999/07/17 19:50:59  steve
 *  netlist support for ternary operator.
 *
 * Revision 1.5  1999/06/16 03:13:29  steve
 *  More syntax parse with sorry stubs.
 *
 * Revision 1.4  1999/06/10 04:03:52  steve
 *  Add support for the Ternary operator,
 *  Add support for repeat concatenation,
 *  Correct some seg faults cause by elaboration
 *  errors,
 *  Parse the casex anc casez statements.
 *
 * Revision 1.3  1999/05/16 05:08:42  steve
 *  Redo constant expression detection to happen
 *  after parsing.
 *
 *  Parse more operators and expressions.
 *
 * Revision 1.2  1998/11/11 00:01:51  steve
 *  Check net ranges in declarations.
 *
 * Revision 1.1  1998/11/03 23:28:53  steve
 *  Introduce verilog to CVS.
 *
 */

