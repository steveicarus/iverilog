/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_lval.cc,v 1.6 2000/10/31 17:49:02 steve Exp $"
#endif

# include  "PExpr.h"
# include  "netlist.h"

/*
 * These methods generate a NetAssign_ object for the l-value of the
 * assignemnt. This is common code for the = and <= statements.
 *
 * What gets generated depends on the structure of the l-value. If the
 * l-value is a simple name (i.e. foo <= <value>) the the NetAssign_
 * is created the width of the foo reg and connected to all the
 * bits.
 *
 * If there is a part select (i.e. foo[3:1] <= <value>) the NetAssign_
 * is made only as wide as it needs to be (3 bits in this example) and
 * connected to the correct bits of foo. A constant bit select is a
 * special case of the part select. 
 *
 * If the bit-select is non-constant (i.e. foo[<expr>] = <value>) the
 * NetAssign_ is made wide enough to connect to all the bits of foo,
 * then the mux expression is elaborated and attached to the
 * NetAssign_ node as a b_mux value. The target must interpret the
 * presense of a bmux value as taking a single bit and assigning it to
 * the bit selected by the bmux expression.
 *
 * If the l-value expression is non-trivial, but can be fully
 * evaluated at compile time (meaning any bit selects are constant)
 * then elaboration will make a single NetAssign_ that connects to a
 * synthetic reg that in turn connects to all the proper pins of the
 * l-value.
 *
 * This last case can turn up in statements like: {a, b[1]} = c;
 * rather then create a NetAssign_ for each item in the contatenation,
 * elaboration makes a single NetAssign_ and connects it up properly.
 */


/*
 * The default interpretation of an l-value to a procedural assignment
 * is to try to make a net elaboration, and see if the result is
 * suitable for assignment.
 */
NetAssign_* PExpr::elaborate_lval(Design*des, NetScope*scope) const
{
      NetNet*ll = elaborate_net(des, scope->name(), 0, 0, 0, 0);
      if (ll == 0) {
	    cerr << get_line() << ": Assignment l-value too complex."
		 << endl;
	    return 0;
      }

      NetAssign_*lv = new NetAssign_(scope->local_symbol(), ll->pin_count());
      for (unsigned idx = 0 ; idx < ll->pin_count() ;  idx += 1)
	    connect(lv->pin(idx), ll->pin(idx));
      des->add_node(lv);
      return lv;
}

/*
 * Concatenation expressions can appear as l-values. Handle them here.
 *
 * If adjacent l-values in the concatenation are not bit selects, then
 * merge them into a single NetAssign_ object. This can happen is code
 * like ``{ ...a, b, ...}''. As long as "a" and "b" do not have bit
 * selects (or the bit selects are constant) we can merge the
 * NetAssign_ objects.
 *
 * Be careful to get the bit order right. In the expression ``{a, b}''
 * a is the MSB and b the LSB. Connect the LSB to the low pins of the
 * NetAssign_ object.
 */
NetAssign_* PEConcat::elaborate_lval(Design*des, NetScope*scope) const
{
      if (repeat_) {
	    cerr << get_line() << ": error: Repeat concatenations make "
		  "no sense in l-value expressions. I refuse." << endl;
	    des->errors += 1;
	    return 0;
      }

      NetAssign_*res = 0;

      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    NetAssign_*tmp = parms_[idx]->elaborate_lval(des, scope);

	      /* If the l-value doesn't elaborate, the error was
		 already detected and printed. We just skip it and let
		 the compiler catch more errors. */
	    if (tmp == 0)
		  continue;

	    assert(tmp);

	      /* If adjacent l-values in the concatenation are not bit
		 selects, then merge them into a single NetAssign_
		 object.

		 If we cannot merge, then just link the new one to the
		 previous one. */

	    if (res && (res->bmux() == 0) && (tmp->bmux() == 0)) {
		  unsigned wid1 = tmp->pin_count();
		  unsigned wid2 = res->pin_count() + tmp->pin_count();
		  NetAssign_*tmp2 = new NetAssign_(res->name(), wid2);

		  assert(tmp->more == 0);

		  tmp2->more = res->more;
		  res->more = 0;

		  for (unsigned idx = 0 ;  idx < wid1 ; idx += 1)
			connect(tmp2->pin(idx), tmp->pin(idx));

		  for (unsigned idx = 0 ;  idx < res->pin_count() ; idx += 1)
			connect(tmp2->pin(idx+wid1), res->pin(idx));

		  delete res;
		  delete tmp;
		  res = tmp2;
		  des->add_node(res);

	    } else {
		  NetAssign_*last = tmp;
		  while (last->more)
			last = last->more;

		  last->more = res;
		  res = tmp;
	    }
      }

      return res;
}

/*
 * Handle the ident as an l-value. This includes bit and part selects
 * of that ident.
 */
NetAssign_* PEIdent::elaborate_lval(Design*des, NetScope*scope) const
{
	/* Get the signal referenced by the identifier, and make sure
	   it is a register. (Wires are not allows in this context. */
      NetNet*reg = des->find_signal(scope, name());

      if (reg == 0) {
	    cerr << get_line() << ": error: Could not match signal ``" <<
		  name() << "'' in ``" << scope->name() << "''" << endl;
	    des->errors += 1;
	    return 0;
      }
      assert(reg);

      if ((reg->type() != NetNet::REG)
	  && (reg->type() != NetNet::INTEGER)
	  && (reg->type() != NetNet::TIME)) {
	    cerr << get_line() << ": error: " << name() <<
		  " is not a reg/integer/time in " << scope->name() <<
		  "." << endl;
	    des->errors += 1;
	    return 0;
      }

      long msb, lsb;
      NetExpr*mux;

      if (msb_ && lsb_) {
	      /* This handles part selects. In this case, there are
		 two bit select expressions, and both must be
		 constant. Evaluate them and pass the results back to
		 the caller. */
	    verinum*vl = lsb_->eval_const(des, scope->name());
	    if (vl == 0) {
		  cerr << lsb_->get_line() << ": error: "
			"Part select expressions must be constant: "
		       << *lsb_;
		  des->errors += 1;
		  return 0;
	    }
	    verinum*vm = msb_->eval_const(des, scope->name());
	    if (vl == 0) {
		  cerr << msb_->get_line() << ": error: "
			"Part select expressions must be constant: "
		       << *msb_;
		  des->errors += 1;
		  return 0;
	    }

	    msb = vm->as_long();
	    lsb = vl->as_long();
	    mux = 0;

      } else if (msb_) {

	      /* If there is only a single select expression, it is a
		 bit select. Evaluate the constant value and treat it
		 as a part select with a bit width of 1. If the
		 expression it not constant, then return the
		 expression as a mux. */
	    assert(lsb_ == 0);
	    verinum*v = msb_->eval_const(des, scope->name());
	    if (v == 0) {
		  NetExpr*m = msb_->elaborate_expr(des, scope);
		  assert(m);
		  msb = 0;
		  lsb = 0;
		  mux = m;

	    } else {

		  msb = v->as_long();
		  lsb = v->as_long();
		  mux = 0;
	    }

      } else {

	      /* No select expressions, so presume a part select the
		 width of the register. */

	    assert(msb_ == 0);
	    assert(lsb_ == 0);
	    msb = reg->msb();
	    lsb = reg->lsb();
	    mux = 0;
      }


      NetAssign_*lv;
      if (mux) {

	      /* If there is a non-constant bit select, make a
		 NetAssign_ the width of the target reg and attach a
		 bmux to select the target bit. */
	    unsigned wid = reg->pin_count();
	    lv = new NetAssign_(des->local_symbol(scope->name()), wid);

	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(lv->pin(idx), reg->pin(idx));

	    lv->set_bmux(mux);

      } else {

	      /* If the bit/part select is constant, then make the
		 NetAssign_ only as wide as it needs to be and connect
		 only to the selected bits of the reg. */
	    unsigned wid = (msb >= lsb)? (msb-lsb+1) : (lsb-msb+1);
	    assert(wid <= reg->pin_count());

	    lv = new NetAssign_(des->local_symbol(scope->name()), wid);
	    unsigned off = reg->sb_to_idx(lsb);
	    assert((off+wid) <= reg->pin_count());
	    for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
		  connect(lv->pin(idx), reg->pin(idx+off));

      }


      des->add_node(lv);

      return lv;
}

/*
 * $Log: elab_lval.cc,v $
 * Revision 1.6  2000/10/31 17:49:02  steve
 *  Support time variables.
 *
 * Revision 1.5  2000/10/26 17:09:46  steve
 *  Fix handling of errors in behavioral lvalues. (PR#28)
 *
 * Revision 1.4  2000/09/10 15:43:59  steve
 *  Some error checking.
 *
 * Revision 1.3  2000/09/10 03:59:59  steve
 *  Agressively merge NetAssign_ within concatenations.
 *
 * Revision 1.2  2000/09/10 02:18:16  steve
 *  elaborate complex l-values
 *
 * Revision 1.1  2000/09/09 15:21:26  steve
 *  move lval elaboration to PExpr virtual methods.
 *
 */

