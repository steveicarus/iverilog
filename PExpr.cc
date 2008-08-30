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

# include "config.h"

# include  <iostream>

# include  "compiler.h"
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

NetNet* PExpr::elaborate_lnet(Design*des, NetScope*) const
{
      cerr << get_fileline() << ": error: expression not valid in assign l-value: "
	   << *this << endl;
      return 0;
}

NetNet* PExpr::elaborate_bi_net(Design*des, NetScope*) const
{
      cerr << get_fileline() << ": error: "
	   << "expression not valid as argument to inout port: "
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

PEBComp::PEBComp(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
}

PEBComp::~PEBComp()
{
}

PEBShift::PEBShift(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
}

PEBShift::~PEBShift()
{
}

PECallFunction::PECallFunction(const pform_name_t&n, const vector<PExpr *> &parms)
: path_(n), parms_(parms)
{
}

static pform_name_t pn_from_ps(perm_string n)
{
      name_component_t tmp_name (n);
      pform_name_t tmp;
      tmp.push_back(tmp_name);
      return tmp;
}

PECallFunction::PECallFunction(perm_string n, const vector<PExpr*>&parms)
: path_(pn_from_ps(n)), parms_(parms)
{
}

PECallFunction::PECallFunction(perm_string n)
: path_(pn_from_ps(n))
{
}

// NOTE: Anachronism. Try to work all use of svector out.
PECallFunction::PECallFunction(const pform_name_t&n, const svector<PExpr *> &parms)
: path_(n), parms_(vector_from_svector(parms))
{
}

PECallFunction::PECallFunction(perm_string n, const svector<PExpr*>&parms)
: path_(pn_from_ps(n)), parms_(vector_from_svector(parms))
{
}

PECallFunction::~PECallFunction()
{
}

bool PECallFunction::is_constant(Module*mod) const
{
	/* Only $clog2 and the builtin mathematical functions can
	 * be a constant system function. */
      perm_string name = peek_tail_name(path_);
      if (name[0] == '$' && (generation_flag >= GN_VER2005 ||
                             gn_icarus_misc_flag || gn_verilog_ams_flag)) {
	    if (name == "$clog2" ||
	        name == "$ln" ||
	        name == "$log10" ||
	        name == "$exp" ||
	        name == "$sqrt" ||
	        name == "$floor" ||
	        name == "$ceil" ||
	        name == "$sin" ||
	        name == "$cos" ||
	        name == "$tan" ||
	        name == "$asin" ||
	        name == "$acos" ||
	        name == "$atan" ||
	        name == "$sinh" ||
	        name == "$cosh" ||
	        name == "$tanh" ||
	        name == "$asinh" ||
	        name == "$acosh" ||
	        name == "$atanh") {
		  if (parms_.size() != 1 || parms_[0] == 0) {
			cerr << get_fileline() << ": error: " << name
			     << " takes a single argument." << endl;
			return false;
		  }
		  /* If the argument is constant the function is constant. */
		  return parms_[0]->is_constant(mod);
	    }

	    if (name == "$pow" ||
	        name == "$atan2" ||
	        name == "$hypot") {
		  if (parms_.size() != 2 || parms_[0] == 0 || parms_[1] == 0) {
			cerr << get_fileline() << ": error: " << name
			     << " takes two arguments." << endl;
			return false;
		  /* If the arguments are constant the function is constant. */
		  return parms_[0]->is_constant(mod) &&
		         parms_[1]->is_constant(mod);
		  }
	    }

	      /* These are only available with verilog-ams or icarus-misc. */
	    if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
	        (name == "$log" || name == "$abs")) {
		  if (parms_.size() != 1 || parms_[0] == 0) {
			cerr << get_fileline() << ": error: " << name
			     << " takes a single argument." << endl;
			return false;
		  }
		  /* If the argument is constant the function is constant. */
		  return parms_[0]->is_constant(mod);
	    }
	    if ((gn_icarus_misc_flag || gn_verilog_ams_flag) &&
	        (name == "$min" || name == "$max")) {
		  if (parms_.size() != 2 || parms_[0] == 0 || parms_[1] == 0) {
			cerr << get_fileline() << ": error: " << name
			     << " takes two arguments." << endl;
			return false;
		  /* If the arguments are constant the function is constant. */
		  return parms_[0]->is_constant(mod) &&
		         parms_[1]->is_constant(mod);
		  }
	    }

	    return false;  /* The other system functions are not constant. */
      }

	/* Checking for constant user functions goes here. */
      return false;
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

PEIdent::PEIdent(const pform_name_t&that)
: path_(that)
{
}

PEIdent::PEIdent(perm_string s)
{
      path_.push_back(name_component_t(s));
}

PEIdent::~PEIdent()
{
}

/*
 * An identifier can be in a constant expression if (and only if) it is
 * a parameter or genvar.
 *
 * NOTE: This test does not work if the name is hierarchical!
 */
bool PEIdent::is_constant(Module*mod) const
{
      if (mod == 0) return false;

	/*  */
      perm_string tmp = path_.back().name;

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->parameters.find(tmp);
	if (cur != mod->parameters.end()) return true;
      }

      { map<perm_string,Module::param_expr_t>::const_iterator cur;
        cur = mod->localparams.find(tmp);
	if (cur != mod->localparams.end()) return true;
      }

      { map<perm_string,LineInfo*>::const_iterator cur;
        cur = mod->genvars.find(tmp);
	if (cur != mod->genvars.end()) return true;
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
