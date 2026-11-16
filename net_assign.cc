/*
 * Copyright (c) 2000-2026 Stephen Williams (steve@icarus.com)
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

# include  "netlist.h"
# include  "netmisc.h"
# include  "netclass.h"
# include  "netdarray.h"
# include  "netparray.h"
# include  "netenum.h"
# include  "ivl_assert.h"

using namespace std;

/*
 * NetAssign
 */

/*
 * Return the width and type context for evaluating the RHS of an assignment
 * to this l-value. A concatenation needs a four-state context if any component
 * is four-state, so evaluating the RHS does not discard X/Z values needed by
 * that component. Two-state components are converted when stored. A single
 * l-value supplies its own type, including non-integral types.
 */
lval_info_t get_lval_info(const NetAssign_*idx)
{
      lval_info_t info;
      for (; idx ; idx = idx->more) {
	    info.width += idx->lwidth();
	    if (info.type != IVL_VT_LOGIC) {
		  info.type = idx->expr_type();
	    }
      }
      return info;
}

NetAssign_::NetAssign_(NetAssign_*n)
: nest_(n), sig_(0), word_(0), base_(0), sel_type_(IVL_SEL_OTHER)
{
      more = 0;
      turn_sig_to_wire_on_release_ = false;
}

NetAssign_::NetAssign_(NetNet*s)
: nest_(0), sig_(s), word_(0), base_(0), sel_type_(IVL_SEL_OTHER)
{
      more = 0;
      turn_sig_to_wire_on_release_ = false;
}

NetAssign_::~NetAssign_()
{
        // The reference held by this l-value is released after the body.
      if (sig_ && turn_sig_to_wire_on_release_ && sig_->peek_lref() == 1) {
	    sig_->type(NetNet::WIRE);
      }

      ivl_assert(*this, more == 0 );
      delete word_;
}

string NetAssign_::get_fileline() const
{
      if (sig_) return sig_->get_fileline();
      else return nest_->get_fileline();
}

NetScope*NetAssign_::scope() const
{
      if (sig_) return sig_->scope();
      else return nest_->scope();
}

void NetAssign_::set_word(NetExpr*r)
{
      ivl_assert(*this, word_ == 0);
      word_ = r;
}

NetExpr* NetAssign_::word()
{
      return word_;
}

const NetExpr* NetAssign_::word() const
{
      return word_;
}

const NetExpr* NetAssign_::get_base() const
{
      return base_;
}

ivl_select_type_t NetAssign_::select_type() const
{
      return sel_type_;
}

bool NetAssign_::get_signed() const
{
      ivl_type_t type = net_type();
      return type && type->get_signed();
}

void NetAssign_::set_untyped()
{
      untyped_ = true;
}

unsigned NetAssign_::lwidth() const
{
      if (base_) return part_width_;

      auto ntype = component_type();
      ivl_assert(*this, ntype);
      return ntype->packed_width();
}

ivl_variable_type_t NetAssign_::expr_type() const
{
      auto ntype = component_type();
      if (ntype)
	    return ntype->base_type();

      ivl_assert(*this, sig_);
      return sig_->data_type();
}

ivl_type_t NetAssign_::net_type() const
{
	// Concatenations and explicit selects do not retain a declared type.
      if (untyped_ || more) return nullptr;

      return component_type();
}

ivl_type_t NetAssign_::component_type() const
{
	// A selected member can have its own data type.
      if (base_)
	    return part_data_type_;

      ivl_type_t ntype;
      if (nest_) {
	    ntype = nest_->net_type();
      } else {
	    ivl_assert(*this, sig_);

	    if (sig_->unpacked_dimensions() && !word_)
		  ntype = sig_->array_type();
	    else
		  ntype = sig_->net_type();
      }

      if (!member_.nil()) {
	    const netclass_t *class_type = dynamic_cast<const netclass_t*>(ntype);
	    ivl_assert(*this, class_type);
	    ntype = class_type->get_prop_type(member_idx_);
      }

      if (word_) {
	    if (const netdarray_t *darray = dynamic_cast<const netdarray_t*>(ntype))
		  ntype = darray->element_type();
	    else if (const netuarray_t *uarray = dynamic_cast<const netuarray_t*>(ntype))
		  ntype = uarray->element_type();
      }

      return ntype;
}

perm_string NetAssign_::name() const
{
      if (sig_) {
	    return sig_->name();
      } else {
	    return perm_string::literal("");
      }
}

NetNet* NetAssign_::sig() const
{
      ivl_assert(*this, sig_ ? nest_ == 0 : nest_ != 0);
      return sig_.get();
}

void NetAssign_::set_part(NetExpr*base, unsigned wid,
                          ivl_select_type_t sel_type)
{
      base_ = base;
      part_width_ = wid;
      sel_type_ = sel_type;
}

void NetAssign_::set_part(NetExpr*base, ivl_type_t data_type)
{
      part_data_type_ = data_type;
      set_part(base, data_type->packed_width());
}

void NetAssign_::set_part(NetExpr*base, ivl_type_t data_type, unsigned wid)
{
      part_data_type_ = data_type;
      set_part(base, wid);
      set_untyped();
}

void NetAssign_::set_property(const perm_string&mname, unsigned idx)
{
      member_ = mname;
      member_idx_ = idx;
}

/*
 */
void NetAssign_::turn_sig_to_wire_on_release()
{
      turn_sig_to_wire_on_release_ = true;
}

NetAssignBase::NetAssignBase(NetAssign_*lv, NetExpr*rv)
: lval_(lv), rval_(rv), delay_(0)
{
}

NetAssignBase::~NetAssignBase()
{
      delete rval_;
      while (lval_) {
	    NetAssign_*tmp = lval_;
	    lval_ = tmp->more;
	    tmp->more = 0;
	    delete tmp;
      }
}

NetExpr* NetAssignBase::rval()
{
      return rval_;
}

const NetExpr* NetAssignBase::rval() const
{
      return rval_;
}

void NetAssignBase::set_rval(NetExpr*r)
{
      delete rval_;
      rval_ = r;
}

NetAssign_* NetAssignBase::l_val(unsigned idx)
{
      NetAssign_*cur = lval_;
      while (idx > 0) {
	    if (cur == 0)
		  return cur;

	    cur = cur->more;
	    idx -= 1;
      }

      ivl_assert(*this, idx == 0);
      return cur;
}

const NetAssign_* NetAssignBase::l_val(unsigned idx) const
{
      const NetAssign_*cur = lval_;
      while (idx > 0) {
	    if (cur == 0)
		  return cur;

	    cur = cur->more;
	    idx -= 1;
      }

      ivl_assert(*this, idx == 0);
      return cur;
}

unsigned NetAssignBase::l_val_count() const
{
      const NetAssign_*cur = lval_;
      unsigned cnt = 0;
      while (cur) {
	    cnt += 1;
	    cur = cur->more;
      }

      return cnt;
}

unsigned NetAssignBase::lwidth() const
{
      unsigned sum = 0;
      for (const NetAssign_*cur = lval_ ;  cur ;  cur = cur->more)
	    sum += cur->lwidth();
      return sum;
}

void NetAssignBase::set_delay(NetExpr*expr)
{
      delay_ = expr;
}

const NetExpr* NetAssignBase::get_delay() const
{
      return delay_;
}

NetAssign::NetAssign(NetAssign_*lv, NetExpr*rv)
: NetAssignBase(lv, rv), op_(0)
{
}

NetAssign::NetAssign(NetAssign_*lv, char op, NetExpr*rv)
: NetAssignBase(lv, rv), op_(op)
{
}

NetAssign::~NetAssign()
{
}

NetAssignNB::NetAssignNB(NetAssign_*lv, NetExpr*rv, NetEvWait*ev, NetExpr*cnt)
: NetAssignBase(lv, rv), event_(ev), count_(cnt)
{
}

NetAssignNB::~NetAssignNB() = default;

unsigned NetAssignNB::nevents() const
{
      if (event_) return event_->nevents();
      return 0;
}

const NetEvent*NetAssignNB::event(unsigned idx) const
{
      if (event_) return event_->event(idx);
      return 0;
}

const NetExpr*NetAssignNB::get_count() const
{
      return count_.get();
}

NetCAssign::NetCAssign(NetAssign_*lv, NetExpr*rv)
: NetAssignBase(lv, rv)
{
}

NetCAssign::~NetCAssign()
{
}

NetDeassign::NetDeassign(NetAssign_*l)
: NetAssignBase(l, 0)
{
}

NetDeassign::~NetDeassign()
{
}

NetForce::NetForce(NetAssign_*lv, NetExpr*rv)
: NetAssignBase(lv, rv)
{
}

NetForce::~NetForce()
{
}

NetRelease::NetRelease(NetAssign_*l)
: NetAssignBase(l, 0)
{
}

NetRelease::~NetRelease()
{
}
