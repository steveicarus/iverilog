/*
 * Copyright (c) 2002-2013 Stephen Williams (steve@icarus.com)
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
# include  "compiler.h"
# include  "netmisc.h"
# include  <iostream>
# include  "ivl_assert.h"

NetExpr::NetExpr(unsigned w)
: net_type_(0), width_(w), signed_flag_(false)
{
}

NetExpr::NetExpr(ivl_type_t t)
: net_type_(t), width_(0), signed_flag_(false)
{
}

NetExpr::~NetExpr()
{
}

ivl_type_t NetExpr::net_type() const
{
      return net_type_;
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
      return 0;
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

ivl_variable_type_t NetEBAdd::expr_type() const
{
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
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
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;
      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
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
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
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
      if (right_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;
      if (left_->expr_type() == IVL_VT_REAL)
	    return IVL_VT_REAL;

      return IVL_VT_LOGIC;
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

bool NetEConcat::has_width() const
{
      return true;
}

void NetEConcat::set(unsigned idx, NetExpr*e)
{
      assert(idx < parms_.size());
      assert(parms_[idx] == 0);
      parms_[idx] = e;
      expr_width( expr_width() + repeat_ * e->expr_width() );
}

NetEConstEnum::NetEConstEnum(Definitions*s, perm_string n, const netenum_t*eset, const verinum&v)
: NetEConst(v), scope_(s), enum_set_(eset), name_(n)
{
      assert(has_width());
}

NetEConstEnum::~NetEConstEnum()
{
}

const netenum_t*NetEConstEnum::enumeration() const
{
      return enum_set_;
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

bool NetECReal::has_width() const
{
      return true;
}

ivl_variable_type_t NetECReal::expr_type() const
{
      return IVL_VT_REAL;
}

NetECRealParam::NetECRealParam(NetScope*s, perm_string n, const verireal&v)
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
: obj_type_(t), size_(0), init_val_(0)
{
}

NetENew::NetENew(ivl_type_t t, NetExpr*size, NetExpr*init_val)
: obj_type_(t), size_(size), init_val_(init_val)
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

NetEProperty::NetEProperty(NetNet*net, perm_string pnam, NetExpr*idx)
: net_(net), index_(idx)
{
      const netclass_t*use_type = dynamic_cast<const netclass_t*>(net->net_type());
      assert(use_type);

      pidx_ = use_type->property_idx_from_name(pnam);
      ivl_type_t prop_type = use_type->get_prop_type(pidx_);
      expr_width(prop_type->packed_width());
      cast_signed(prop_type->get_signed());
}

NetEProperty::~NetEProperty()
{
}

ivl_variable_type_t NetEProperty::expr_type() const
{
      const netclass_t*use_type = dynamic_cast<const netclass_t*>(net_->net_type());
      assert(use_type);

      ivl_type_t prop_type = use_type->get_prop_type(pidx_);
      return prop_type->base_type();
}

NetESelect::NetESelect(NetExpr*exp, NetExpr*base, unsigned wid,
                       ivl_select_type_t sel_type)
: expr_(exp), base_(base), sel_type_(sel_type)
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
      ivl_variable_type_t type = expr_->expr_type();

	// Special case: If the sub-expression is an IVL_VT_STRING,
	// then this node is representing a character select. The
	// width is the width of a byte, and the data type is BOOL.
      if (type == IVL_VT_STRING && expr_width()==8)
	    return IVL_VT_BOOL;

      if (type != IVL_VT_DARRAY)
	    return type;

      ivl_assert(*this, type == IVL_VT_DARRAY);

	// Special case: If the expression is a DARRAY, then the
	// sub-expression must be a NetESignal and the type of the
	// NetESelect expression is the element type of the arrayed signal.
      NetESignal*sig = dynamic_cast<NetESignal*>(expr_);
      ivl_assert(*this, sig);
      const netarray_t*array_type = dynamic_cast<const netarray_t*> (sig->sig()->net_type());
      ivl_assert(*this, array_type);

      return array_type->element_type()->base_type();
}

bool NetESelect::has_width() const
{
      return true;
}

NetESFunc::NetESFunc(const char*n, ivl_variable_type_t t,
		     unsigned width, unsigned np)
: name_(0), type_(t), enum_type_(0), parms_(np)
{
      name_ = lex_strings.add(n);
      expr_width(width);
}

NetESFunc::NetESFunc(const char*n, ivl_type_t rtype, unsigned np)
: NetExpr(rtype), name_(0), type_(IVL_VT_NO_TYPE), enum_type_(0), parms_(np)
{
      name_ = lex_strings.add(n);
      expr_width(rtype->packed_width());
	// FIXME: For now, assume that all uses of this constructor
	// are for the IVL_VT_DARRAY type. Eventually, the type_
	// member will go away.
      if (dynamic_cast<const netdarray_t*>(rtype))
	    type_ = IVL_VT_DARRAY;
      else if (dynamic_cast<const netclass_t*>(rtype))
	    type_ = IVL_VT_CLASS;
      else
	    ivl_assert(*this, 0);
}

NetESFunc::NetESFunc(const char*n, const netenum_t*enum_type, unsigned np)
: name_(0), type_(enum_type->base_type()), enum_type_(enum_type), parms_(np)
{
      name_ = lex_strings.add(n);
      expr_width(enum_type->packed_width());
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
      assert(idx < parms_.size());
      if (parms_[idx])
	    delete parms_[idx];
      parms_[idx] = v;
}

const NetExpr* NetESFunc::parm(unsigned idx) const
{
      assert(idx < parms_.size());
      return parms_[idx];
}

NetExpr* NetESFunc::parm(unsigned idx)
{
      assert(idx < parms_.size());
      return parms_[idx];
}

ivl_variable_type_t NetESFunc::expr_type() const
{
      return type_;
}

const netenum_t* NetESFunc::enumeration() const
{
      return enum_type_;
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
}

NetEAccess::~NetEAccess()
{
}

ivl_variable_type_t NetEAccess::expr_type() const
{
      return IVL_VT_REAL;
}
