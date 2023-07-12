/*
 * Copyright (c) 2002-2020 Stephen Williams (steve@icarus.com)
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

# include  "config.h"
# include  "netlist.h"
# include  "netenum.h"
# include  "netclass.h"
# include  "netdarray.h"
# include  "netscalar.h"
# include  "compiler.h"
# include  "netmisc.h"
# include  <iostream>
# include  "ivl_assert.h"

using namespace std;

NetExpr::NetExpr(unsigned w)
: net_type_(0), width_(w), signed_flag_(false)
{
}

NetExpr::NetExpr(ivl_type_t t)
: net_type_(t), width_(0), signed_flag_(false)
{
      if (t) {
	    width_ = t->packed_width();
	    signed_flag_ = t->get_signed();
      }
}

NetExpr::~NetExpr()
{
}

ivl_type_t NetExpr::net_type() const
{
      return net_type_;
}

void NetExpr::set_net_type(ivl_type_t type)
{
      net_type_ = type;
      if (type) {
	    width_ = type->packed_width();
	    signed_flag_ = type->get_signed();
      }
}

void NetExpr::cast_signed(bool flag)
{
      cast_signed_base_(flag);
}

bool NetExpr::has_width() const
{
      return true;
}

/*
 * the grand default data type is a logic vector.
 */
ivl_variable_type_t NetExpr::expr_type() const
{
      if (net_type_)
	    return net_type_->base_type();
      else
	    return IVL_VT_LOGIC;
}

const netenum_t*NetExpr::enumeration() const
{
      return dynamic_cast<const netenum_t*>(net_type_);
}

NetEArrayPattern::NetEArrayPattern(ivl_type_t lv_type, vector<NetExpr*>&items)
: NetExpr(lv_type), items_(items)
{
}

NetEArrayPattern::~NetEArrayPattern()
{
      for (size_t idx = 0 ; idx < items_.size() ; idx += 1)
	    delete items_[idx];
}

/*
 * Create an add/sub node from the two operands.
 */
NetEBAdd::NetEBAdd(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBAdd::~NetEBAdd()
{
}


static ivl_variable_type_t arith_expr_type(const NetExpr *l, const NetExpr *r)
{
      auto l_expr_type = l->expr_type();
      auto r_expr_type = r->expr_type();

      if (l_expr_type == IVL_VT_REAL ||
          r_expr_type == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (l_expr_type == IVL_VT_LOGIC ||
          r_expr_type == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      return IVL_VT_BOOL;
}

ivl_variable_type_t NetEBAdd::expr_type() const
{
      return arith_expr_type(left_, right_);
}

/*
 * Create a comparison operator with two sub-expressions.
 */
NetEBComp::NetEBComp(char op__, NetExpr*l, NetExpr*r)
: NetEBinary(op__, l, r, 1, false)
{
}

NetEBComp::~NetEBComp()
{
}

bool NetEBComp::has_width() const
{
      return true;
}

ivl_variable_type_t NetEBComp::expr_type() const
{
	// Case compare always returns BOOL
      if (op() == 'E' || op() == 'N')
	    return IVL_VT_BOOL;

      if (left()->expr_type() == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      if (right()->expr_type() == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      return IVL_VT_BOOL;
}

NetEBDiv::NetEBDiv(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBDiv::~NetEBDiv()
{
}

ivl_variable_type_t NetEBDiv::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      // div is always 4-state, even if both inputs are 2-state because division
      // by 0 can yield 'x
      return IVL_VT_LOGIC;
}

NetEBMinMax::NetEBMinMax(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBMinMax::~NetEBMinMax()
{
}

ivl_variable_type_t NetEBMinMax::expr_type() const
{
      return arith_expr_type(left_, right_);
}

NetEBMult::NetEBMult(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBMult::~NetEBMult()
{
}

ivl_variable_type_t NetEBMult::expr_type() const
{
      return arith_expr_type(left_, right_);
}

NetEBPow::NetEBPow(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBPow::~NetEBPow()
{
}

ivl_variable_type_t NetEBPow::expr_type() const
{
      return arith_expr_type(left_, right_);
}

NetEBShift::NetEBShift(char op__, NetExpr*l, NetExpr*r, unsigned wid, bool signed_flag)
: NetEBinary(op__, l, r, wid, signed_flag)
{
}

NetEBShift::~NetEBShift()
{
}

bool NetEBShift::has_width() const
{
      return left_->has_width();
}

ivl_variable_type_t NetEBShift::expr_type() const
{
      if (left_->expr_type() == IVL_VT_LOGIC ||
          right_->expr_type() == IVL_VT_LOGIC)
	    return IVL_VT_LOGIC;

      return IVL_VT_BOOL;
}

NetEConcat::NetEConcat(unsigned cnt, unsigned r, ivl_variable_type_t vt)
: parms_(cnt), repeat_(r), expr_type_(vt)
{
      expr_width(0);
}

NetEConcat::~NetEConcat()
{
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
	    delete parms_[idx];
}

ivl_variable_type_t NetEConcat::expr_type() const
{
      return expr_type_;
}

void NetEConcat::set(unsigned idx, NetExpr*e)
{
      ivl_assert(*this, idx < parms_.size());
      ivl_assert(*this, parms_[idx] == 0);
      parms_[idx] = e;
      expr_width( expr_width() + repeat_ * e->expr_width() );
}

NetEConstEnum::NetEConstEnum(perm_string n, const netenum_t *enum_set,
			     const verinum &val)
: NetEConst(enum_set, val), name_(n)
{
      ivl_assert(*this, has_width());
}

NetEConstEnum::~NetEConstEnum()
{
}

NetECReal::NetECReal(const verireal&val)
: value_(val)
{
      expr_width(1);
      cast_signed_base_(true);
}

NetECReal::~NetECReal()
{
}

const verireal& NetECReal::value() const
{
      return value_;
}

ivl_variable_type_t NetECReal::expr_type() const
{
      return IVL_VT_REAL;
}

NetECRealParam::NetECRealParam(const NetScope*s, perm_string n, const verireal&v)
: NetECReal(v), scope_(s), name_(n)
{
}

NetECRealParam::~NetECRealParam()
{
}

perm_string NetECRealParam::name() const
{
      return name_;
}

const NetScope* NetECRealParam::scope() const
{
      return scope_;
}

NetECString::NetECString(const std::string& val)
: NetEConst(verinum(val))
{
}

NetECString::~NetECString()
{
}

ivl_variable_type_t NetECString::expr_type() const
{
      return IVL_VT_STRING;
}

NetELast::NetELast(NetNet*s)
: sig_(s)
{
}

NetELast::~NetELast()
{
}

ivl_variable_type_t NetELast::expr_type() const
{
      return IVL_VT_BOOL;
}

NetENetenum::NetENetenum(const netenum_t*s)
: netenum_(s)
{
}

NetENetenum::~NetENetenum()
{
}

const netenum_t* NetENetenum::netenum() const
{
      return netenum_;
}

NetENew::NetENew(ivl_type_t t)
: NetExpr(t), size_(0), init_val_(0)
{
}

NetENew::NetENew(ivl_type_t t, NetExpr*size, NetExpr*init_val)
: NetExpr(t), size_(size), init_val_(init_val)
{
}

NetENew::~NetENew()
{
}

ivl_variable_type_t NetENew::expr_type() const
{
      return size_ ? IVL_VT_DARRAY : IVL_VT_CLASS;
}

NetENull::NetENull()
{
}

NetENull::~NetENull()
{
}

NetEProperty::NetEProperty(NetNet*net, size_t pidx, NetExpr*idx)
: net_(net), pidx_(pidx), index_(idx)
{
      const netclass_t*use_type = dynamic_cast<const netclass_t*>(net->net_type());
      ivl_assert(*this, use_type);

      ivl_type_t prop_type = use_type->get_prop_type(pidx_);
      if (idx) {
	    auto array_type = dynamic_cast<const netarray_t*>(prop_type);
	    ivl_assert(*this, array_type);
	    set_net_type(array_type->element_type());
      } else {
	    set_net_type(prop_type);
      }
}

NetEProperty::~NetEProperty()
{
}

NetESelect::NetESelect(NetExpr*exp, NetExpr*base, unsigned wid,
                       ivl_select_type_t sel_type)
: expr_(exp), base_(base), sel_type_(sel_type)
{
      expr_width(wid);
}

NetESelect::NetESelect(NetExpr*exp, NetExpr*base, unsigned wid,
                       ivl_type_t use_type)
: NetExpr(use_type), expr_(exp), base_(base), sel_type_(IVL_SEL_OTHER)
{
      expr_width(wid);
}

NetESelect::~NetESelect()
{
      delete expr_;
      delete base_;
}

const NetExpr*NetESelect::sub_expr() const
{
      return expr_;
}

const NetExpr*NetESelect::select() const
{
      return base_;
}

ivl_select_type_t NetESelect::select_type() const
{
      return sel_type_;
}

ivl_variable_type_t NetESelect::expr_type() const
{
      if (net_type())
	    return net_type()->base_type();

      ivl_variable_type_t type = expr_->expr_type();

	// Special case: If the sub-expression is an IVL_VT_STRING,
	// then this node is representing a character select. The
	// width is the width of a byte, and the data type is BOOL.
      if (type == IVL_VT_STRING && expr_width()==8)
	    return IVL_VT_BOOL;

      return type;
}

NetESFunc::NetESFunc(const char*n, ivl_variable_type_t t,
		     unsigned width, unsigned np, bool is_overridden)
: name_(0), type_(t), parms_(np), is_overridden_(is_overridden)
{
      name_ = lex_strings.add(n);
      expr_width(width);
}

NetESFunc::NetESFunc(const char*n, ivl_type_t rtype, unsigned np)
: NetExpr(rtype), name_(0), type_(rtype->base_type()), parms_(np),
  is_overridden_(false)
{
      name_ = lex_strings.add(n);
}

NetESFunc::~NetESFunc()
{
      for (unsigned idx = 0 ;  idx < parms_.size() ;  idx += 1)
	    if (parms_[idx]) delete parms_[idx];

	/* name_ string ls lex_strings allocated. */
}

const char* NetESFunc::name() const
{
      return name_;
}

unsigned NetESFunc::nparms() const
{
      return parms_.size();
}

void NetESFunc::parm(unsigned idx, NetExpr*v)
{
      ivl_assert(*this, idx < parms_.size());
      if (parms_[idx])
	    delete parms_[idx];
      parms_[idx] = v;
}

const NetExpr* NetESFunc::parm(unsigned idx) const
{
      ivl_assert(*this, idx < parms_.size());
      return parms_[idx];
}

NetExpr* NetESFunc::parm(unsigned idx)
{
      ivl_assert(*this, idx < parms_.size());
      return parms_[idx];
}

ivl_variable_type_t NetESFunc::expr_type() const
{
      return type_;
}

NetEShallowCopy::NetEShallowCopy(NetExpr*arg1, NetExpr*arg2)
: arg1_(arg1), arg2_(arg2)
{
}

NetEShallowCopy::~NetEShallowCopy()
{
}

ivl_variable_type_t NetEShallowCopy::expr_type() const
{
      return arg1_->expr_type();
}

NetEAccess::NetEAccess(NetBranch*br, ivl_nature_t nat)
: branch_(br), nature_(nat)
{
      cast_signed_base_(true);
}

NetEAccess::~NetEAccess()
{
}

ivl_variable_type_t NetEAccess::expr_type() const
{
      return IVL_VT_REAL;
}
