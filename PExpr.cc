/*
 * Copyright (c) 1998-2008,2010 Stephen Williams <steve@icarus.com>
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
# include  "PWire.h"
# include  "Module.h"
# include  "netmisc.h"
# include  <typeinfo>

PExpr::PExpr()
{
      expr_width_ = 0;
      expr_type_ = IVL_VT_NO_TYPE;
}

PExpr::~PExpr()
{
}

void PExpr::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
}

bool PExpr::has_aa_term(Design*, NetScope*) const
{
      return false;
}

bool PExpr::is_the_same(const PExpr*that) const
{
      return typeid(this) == typeid(that);
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

void PEBinary::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      assert(left_ && right_);
      left_->declare_implicit_nets(scope, type);
      right_->declare_implicit_nets(scope, type);
}

bool PEBinary::has_aa_term(Design*des, NetScope*scope) const
{
      assert(left_ && right_);
      return left_->has_aa_term(des, scope) || right_->has_aa_term(des, scope);
}

PEBComp::PEBComp(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
}

PEBComp::~PEBComp()
{
}

PEBLogic::PEBLogic(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
      assert(op == 'a' || op == 'o');
}

PEBLogic::~PEBLogic()
{
}

PEBLeftWidth::PEBLeftWidth(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
}

PEBLeftWidth::~PEBLeftWidth()
{
}

PEBPower::PEBPower(char op, PExpr*l, PExpr*r)
: PEBLeftWidth(op, l, r)
{
}

PEBPower::~PEBPower()
{
}

PEBShift::PEBShift(char op, PExpr*l, PExpr*r)
: PEBLeftWidth(op, l, r)
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

void PECallFunction::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
	    parms_[idx]->declare_implicit_nets(scope, type);
      }
}

bool PECallFunction::has_aa_term(Design*des, NetScope*scope) const
{
      bool flag = false;
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
	    flag = parms_[idx]->has_aa_term(des, scope) || flag;
      }
      return flag;
}

PEConcat::PEConcat(const svector<PExpr*>&p, PExpr*r)
: parms_(p), tested_widths_(p.count()), repeat_(r)
{
}

PEConcat::~PEConcat()
{
      delete repeat_;
}

void PEConcat::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      for (unsigned idx = 0 ; idx < parms_.count() ; idx += 1) {
	    parms_[idx]->declare_implicit_nets(scope, type);
      }
}

bool PEConcat::has_aa_term(Design*des, NetScope*scope) const
{
      bool flag = false;
      for (unsigned idx = 0 ; idx < parms_.count() ; idx += 1) {
	    flag = parms_[idx]->has_aa_term(des, scope) || flag;
      }
      if (repeat_)
            flag = repeat_->has_aa_term(des, scope) || flag;

      return flag;
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

bool PEEvent::has_aa_term(Design*des, NetScope*scope) const
{
      assert(expr_);
      return expr_->has_aa_term(des, scope);
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

PEIdent::PEIdent(const pform_name_t&that)
: path_(that), no_implicit_sig_(false)
{
}

PEIdent::PEIdent(perm_string s, bool no_implicit_sig)
: no_implicit_sig_(no_implicit_sig)
{
      path_.push_back(name_component_t(s));
}

PEIdent::~PEIdent()
{
}

void PEIdent::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
        /* We create an implicit wire if:
	   - this is a simple identifier
           - an identifier of that name has not already been declared in
             any enclosing scope.
	   - this is not an implicit named port connection */
     if (no_implicit_sig_)
	    return;
     if ((path_.size() == 1) && (path_.front().index.size() == 0)) {
            perm_string name = path_.front().name;
            LexicalScope*ss = scope;
            while (ss) {
                  if (ss->wires.find(name) != ss->wires.end())
                        return;
                  if (ss->localparams.find(name) != ss->localparams.end())
                        return;
                  if (ss->parameters.find(name) != ss->parameters.end())
                        return;
                  if (ss->genvars.find(name) != ss->genvars.end())
                        return;
                  if (ss->events.find(name) != ss->events.end())
                        return;
                  /* Strictly speaking, we should also check for name clashes
                     with tasks, functions, named blocks, module instances,
                     and generate blocks. However, this information is not
                     readily available. As these names would not be legal in
                     this context, we can declare implicit nets here and rely
                     on later checks for name clashes to report the error. */

                  ss = ss->parent_scope();
            }
            PWire*net = new PWire(name, type, NetNet::NOT_A_PORT, IVL_VT_LOGIC);
            net->set_file(get_file());
            net->set_lineno(get_lineno());
            net->set_range(0, 0, SR_NET, true);
            scope->wires[name] = net;
            if (warn_implicit) {
                  cerr << get_fileline() << ": warning: implicit "
                       "definition of wire '" << name << "'." << endl;
            }
      }
}

bool PEIdent::has_aa_term(Design*des, NetScope*scope) const
{
      NetNet*       net = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      const NetExpr*ex1, *ex2;

      scope = symbol_search(0, des, scope, path_, net, par, eve, ex1, ex2);

      if (scope)
            return scope->is_auto();
      else
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

PETernary::PETernary(PExpr*e, PExpr*t, PExpr*f)
: expr_(e), tru_(t), fal_(f)
{
}

PETernary::~PETernary()
{
}

void PETernary::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      assert(expr_ && tru_ && fal_);
      expr_->declare_implicit_nets(scope, type);
      tru_->declare_implicit_nets(scope, type);
      fal_->declare_implicit_nets(scope, type);
}

bool PETernary::has_aa_term(Design*des, NetScope*scope) const
{
      assert(expr_ && tru_ && fal_);
      return expr_->has_aa_term(des, scope)
           || tru_->has_aa_term(des, scope)
           || fal_->has_aa_term(des, scope);
}

PEUnary::PEUnary(char op, PExpr*ex)
: op_(op), expr_(ex)
{
}

PEUnary::~PEUnary()
{
}

void PEUnary::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      assert(expr_);
      expr_->declare_implicit_nets(scope, type);
}

bool PEUnary::has_aa_term(Design*des, NetScope*scope) const
{
      assert(expr_);
      return expr_->has_aa_term(des, scope);
}
