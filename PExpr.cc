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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: PExpr.cc,v 1.15 2000/04/01 19:31:57 steve Exp $"
#endif

# include  "PExpr.h"
# include  "Module.h"
# include  <typeinfo>

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

NetNet* PExpr::elaborate_net(Design*des, const string&path, unsigned,
			     unsigned long,
			     unsigned long,
			     unsigned long) const
{
      cerr << get_line() << ": error: Unable to elaborate `"
	   << *this << "' as gates." << endl;
      return 0;
}

NetNet* PExpr::elaborate_lnet(Design*des, const string&path) const
{
      cerr << get_line() << ": error: expression not valid in assign l-value: "
	   << *this << endl;
      return 0;
}

bool PEBinary::is_constant(Module*mod) const
{
      return left_->is_constant(mod) && right_->is_constant(mod);
}

PECallFunction::PECallFunction(const string &n, const svector<PExpr *> &parms) 
: name_(n), parms_(parms)
{
}

PECallFunction::~PECallFunction()
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

PEEvent::PEEvent(NetNEvent::Type t, PExpr*e)
: type_(t), expr_(e)
{
}

PEEvent::PEEvent(const string&n)
: name_(n)
{
}

PEEvent::~PEEvent()
{
}

NetNEvent::Type PEEvent::type() const
{
      return type_;
}

PExpr* PEEvent::expr() const
{
      return expr_;
}

string PEEvent::name() const
{
      return name_;
}

/*
 * An identifier can be in a constant expresion if (and only if) it is
 * a parameter.
 */
bool PEIdent::is_constant(Module*mod) const
{
      map<string,PExpr*>::const_iterator cur;

      cur = mod->parameters.find(text_);
      if (cur != mod->parameters.end()) return true;

      cur = mod->localparams.find(text_);
      if (cur != mod->localparams.end()) return true;

      return false;
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

bool PETernary::is_constant(Module*) const
{
      return false;
}

/*
 * $Log: PExpr.cc,v $
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

