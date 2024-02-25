/*
 * Copyright (c) 1998-2024 Stephen Williams <steve@icarus.com>
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  <iostream>

# include  "compiler.h"
# include  "PExpr.h"
# include  "PWire.h"
# include  "Module.h"
# include  "ivl_assert.h"
# include  "netmisc.h"
# include  "util.h"
# include  <typeinfo>

using namespace std;

PExpr::PExpr()
: expr_type_(IVL_VT_NO_TYPE)
{
      expr_width_  = 0;
      min_width_   = 0;
      signed_flag_ = false;
}

PExpr::~PExpr()
{
}

void PExpr::declare_implicit_nets(LexicalScope*, NetNet::Type)
{
}

bool PExpr::has_aa_term(Design*, NetScope*) const
{
      return false;
}

NetNet* PExpr::elaborate_lnet(Design*, NetScope*) const
{
      cerr << get_fileline() << ": error: "
           << "expression not valid in assign l-value: "
           << *this << endl;
      return 0;
}

NetNet* PExpr::elaborate_bi_net(Design*, NetScope*) const
{
      cerr << get_fileline() << ": error: "
           << "expression not valid as argument to inout port: "
           << *this << endl;
      return 0;
}

bool PExpr::is_collapsible_net(Design*, NetScope*, NetNet::PortType) const
{
      return false;
}


const char* PExpr::width_mode_name(width_mode_t mode)
{
      switch (mode) {
          case PExpr::SIZED:
            return "sized";
          case PExpr::UNSIZED:
            return "unsized";
          case PExpr::EXPAND:
            return "expand";
          case PExpr::LOSSLESS:
            return "lossless";
          case PExpr::UPSIZE:
            return "upsize";
          default:
            return "??";
      }
}

PEAssignPattern::PEAssignPattern()
{
}

PEAssignPattern::PEAssignPattern(const list<PExpr*>&p)
: parms_(p.begin(), p.end())
{
}

PEAssignPattern::~PEAssignPattern()
{
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
      if (left_) left_->declare_implicit_nets(scope, type);
      if (right_) right_->declare_implicit_nets(scope, type);
}

bool PEBinary::has_aa_term(Design*des, NetScope*scope) const
{
      ivl_assert(*this, left_ && right_);
      return left_->has_aa_term(des, scope) || right_->has_aa_term(des, scope);
}

PECastSize::PECastSize(PExpr*si, PExpr*b)
: size_(si), base_(b)
{
}

PECastSize::~PECastSize()
{
}

bool PECastSize::has_aa_term(Design *des, NetScope *scope) const
{
	return base_->has_aa_term(des, scope);
}

PECastType::PECastType(data_type_t*t, PExpr*b)
: target_(t), base_(b)
{
}

PECastType::~PECastType()
{
}

bool PECastType::has_aa_term(Design *des, NetScope *scope) const
{
	return base_->has_aa_term(des, scope);
}

PECastSign::PECastSign(bool signed_flag, PExpr *base)
: base_(base)
{
    signed_flag_ = signed_flag;
}

bool PECastSign::has_aa_term(Design *des, NetScope *scope) const
{
	return base_->has_aa_term(des, scope);
}

PEBComp::PEBComp(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
      l_width_ = 0;
      r_width_ = 0;
}

PEBComp::~PEBComp()
{
}

PEBLogic::PEBLogic(char op, PExpr*l, PExpr*r)
: PEBinary(op, l, r)
{
      ivl_assert(*this, op == 'a' || op == 'o' || op == 'q' || op == 'Q');
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

PECallFunction::PECallFunction(const pform_name_t &n, const vector<named_pexpr_t> &parms)
: path_(n), parms_(parms), is_overridden_(false)
{
}

PECallFunction::PECallFunction(PPackage *pkg, const pform_name_t &n, const vector<named_pexpr_t> &parms)
: path_(pkg, n), parms_(parms), is_overridden_(false)
{
}

static pform_name_t pn_from_ps(perm_string n)
{
      name_component_t tmp_name (n);
      pform_name_t tmp;
      tmp.push_back(tmp_name);
      return tmp;
}

PECallFunction::PECallFunction(PPackage *pkg, const pform_name_t &n, const list<named_pexpr_t> &parms)
: path_(pkg, n), parms_(parms.begin(), parms.end()), is_overridden_(false)
{
}

PECallFunction::PECallFunction(perm_string n, const vector<named_pexpr_t> &parms)
: path_(pn_from_ps(n)), parms_(parms), is_overridden_(false)
{
}

PECallFunction::PECallFunction(perm_string n)
: path_(pn_from_ps(n)), is_overridden_(false)
{
}

// NOTE: Anachronism. Try to work all use of svector out.
PECallFunction::PECallFunction(const pform_name_t &n, const list<named_pexpr_t> &parms)
: path_(n), parms_(parms.begin(), parms.end()), is_overridden_(false)
{
}

PECallFunction::PECallFunction(perm_string n, const list<named_pexpr_t> &parms)
: path_(pn_from_ps(n)), parms_(parms.begin(), parms.end()), is_overridden_(false)
{
}

PECallFunction::~PECallFunction()
{
}

void PECallFunction::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      for (const auto &parm : parms_) {
	    if (parm.parm)
		  parm.parm->declare_implicit_nets(scope, type);
      }
}

bool PECallFunction::has_aa_term(Design*des, NetScope*scope) const
{
      bool flag = false;
      for (const auto &parm : parms_) {
	    if (parm.parm)
		  flag |= parm.parm->has_aa_term(des, scope);
      }
      return flag;
}

PEConcat::PEConcat(const list<PExpr*>&p, PExpr*r)
: parms_(p.begin(), p.end()), width_modes_(SIZED, p.size()), repeat_(r)
{
      tested_scope_ = 0;
      repeat_count_ = 1;
}

PEConcat::~PEConcat()
{
      delete repeat_;
}

void PEConcat::declare_implicit_nets(LexicalScope*scope, NetNet::Type type)
{
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
	    parms_[idx]->declare_implicit_nets(scope, type);
      }
}

bool PEConcat::has_aa_term(Design*des, NetScope*scope) const
{
      bool flag = false;
      for (unsigned idx = 0 ; idx < parms_.size() ; idx += 1) {
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
      ivl_assert(*this, expr_);
      return expr_->has_aa_term(des, scope);
}

PExpr* PEEvent::expr() const
{
      return expr_;
}

PENull::PENull(void)
{
}

PENull::~PENull()
{
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

PEIdent::PEIdent(const pform_name_t&that, unsigned lexical_pos)
: path_(that), lexical_pos_(lexical_pos), no_implicit_sig_(false)
{
}

PEIdent::PEIdent(perm_string s, unsigned lexical_pos, bool no_implicit_sig)
: lexical_pos_(lexical_pos), no_implicit_sig_(no_implicit_sig)
{
      path_.name.push_back(name_component_t(s));
}

PEIdent::PEIdent(PPackage*pkg, const pform_name_t&that, unsigned lexical_pos)
: path_(pkg, that), lexical_pos_(lexical_pos), no_implicit_sig_(true)
{
}

PEIdent::~PEIdent()
{
}

static bool find_enum_constant(LexicalScope*scope, perm_string name)
{
      for (vector<enum_type_t*>::const_iterator cur = scope->enum_sets.begin() ;
           cur != scope->enum_sets.end() ; ++ cur) {
	    for (list<named_pexpr_t>::const_iterator idx = (*cur)->names->begin() ;
                 idx != (*cur)->names->end() ; ++ idx) {
                  if (idx->name == name) return true;
            }
      }
      return false;
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
     if (path_.package)
	    return;
     if (path_.name.size() == 1 && path_.name.front().index.empty()) {
            perm_string name = path_.name.front().name;
            LexicalScope*ss = scope;
            while (ss) {
                  if (ss->wires.find(name) != ss->wires.end())
                        return;
                  if (ss->parameters.find(name) != ss->parameters.end())
                        return;
                  if (ss->genvars.find(name) != ss->genvars.end())
                        return;
                  if (ss->events.find(name) != ss->events.end())
                        return;
                  if (find_enum_constant(ss, name))
                        return;
                  /* Strictly speaking, we should also check for name clashes
                     with tasks, functions, named blocks, module instances,
                     and generate blocks. However, this information is not
                     readily available. As these names would not be legal in
                     this context, we can declare implicit nets here and rely
                     on later checks for name clashes to report the error. */

                  ss = ss->parent_scope();
            }
            PWire*net = new PWire(name, lexical_pos_, type, NetNet::NOT_A_PORT);
            net->set_file(get_file());
            net->set_lineno(get_lineno());
            scope->wires[name] = net;
            if (warn_implicit) {
                  cerr << get_fileline() << ": warning: implicit "
                       "definition of wire '" << name << "'." << endl;
            }
      }
}

bool PEIdent::has_aa_term(Design*des, NetScope*scope) const
{
      symbol_search_results sr;
      if (!symbol_search(this, des, scope, path_, lexical_pos_, &sr))
	    return false;

      // Class properties are not considered automatic since a non-blocking
      // assignment to an object stored in an automatic variable is supposed to
      // capture a reference to the object, not the variable.
      if (!sr.path_tail.empty() && sr.net && sr.net->class_type())
	    return false;

      return sr.scope->is_auto();
}

PENewArray::PENewArray(PExpr*size_expr, PExpr*init_expr)
: size_(size_expr), init_(init_expr)
{
}

PENewArray::~PENewArray()
{
      delete size_;
}

PENewClass::PENewClass(void)
{
}

PENewClass::PENewClass(const list<named_pexpr_t> &p, data_type_t *class_type)
: parms_(p.begin(), p.end()), class_type_(class_type)
{
}

PENewClass::~PENewClass()
{
}

PENewCopy::PENewCopy(PExpr*src)
: src_(src)
{
}

PENewCopy::~PENewCopy()
{
}

PENumber::PENumber(verinum*vp)
: value_(vp)
{
      ivl_assert(*this, vp);
}

PENumber::~PENumber()
{
      delete value_;
}

const verinum& PENumber::value() const
{
      return *value_;
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
      ivl_assert(*this, expr_ && tru_ && fal_);
      expr_->declare_implicit_nets(scope, type);
      tru_->declare_implicit_nets(scope, type);
      fal_->declare_implicit_nets(scope, type);
}

bool PETernary::has_aa_term(Design*des, NetScope*scope) const
{
      ivl_assert(*this, expr_ && tru_ && fal_);
      return expr_->has_aa_term(des, scope)
           || tru_->has_aa_term(des, scope)
           || fal_->has_aa_term(des, scope);
}

PETypename::PETypename(data_type_t*dt)
: data_type_(dt)
{
}

PETypename::~PETypename()
{
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
      ivl_assert(*this, expr_);
      expr_->declare_implicit_nets(scope, type);
}

bool PEUnary::has_aa_term(Design*des, NetScope*scope) const
{
      ivl_assert(*this, expr_);
      return expr_->has_aa_term(des, scope);
}

PEVoid::PEVoid()
{
}

PEVoid::~PEVoid()
{
}
