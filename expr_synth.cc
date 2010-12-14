/*
 * Copyright (c) 1999-2010 Stephen Williams (steve@icarus.com)
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
# include  <typeinfo>

# include  "netlist.h"
# include  "netmisc.h"
# include  "compiler.h"

NetNet* NetExpr::synthesize(Design*des)
{
      cerr << get_line() << ": internal error: cannot synthesize expression: "
	   << *this << endl;
      cerr << get_line() << ":               : typeid="
	   << typeid(*this).name() << endl;
      des->errors += 1;

      return 0;
}

/*
 * For the NetEBitSel expression, create a NetMux node that selects a
 * bit from the input.
 */
NetNet* NetEBitSel::synthesize(Design*des)
{
      NetNet*net = sig_->synthesize(des);
      assert(net);

      NetNet*adr = idx_->synthesize(des);
      if (adr == 0)
	    return 0;

      NetScope*scope = adr->scope();

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::WIRE, 1);
      osig->set_line(*this);
      osig->local_flag(true);

      NetMux*mux = new NetMux(scope, scope->local_symbol(),
			      1, net->pin_count(), adr->pin_count());
      mux->set_line(*this);
      des->add_node(mux);

      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1)
	    connect(mux->pin_Data(0, idx),  net->pin(idx));

      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(mux->pin_Sel(idx), adr->pin(idx));

      connect(mux->pin_Result(0), osig->pin(0));

      return osig;
}

/*
 * Make an LPM_ADD_SUB device from addition operators.
 */
NetNet* NetEBAdd::synthesize(Design*des)
{
      assert((op()=='+') || (op()=='-'));

      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      assert(expr_width() >= lsig->pin_count());
      assert(expr_width() >= rsig->pin_count());

      lsig = pad_to_width(des, lsig, expr_width());
      rsig = pad_to_width(des, rsig, expr_width());

      assert(lsig->pin_count() == rsig->pin_count());
      unsigned width=lsig->pin_count();

      perm_string path = lsig->scope()->local_symbol();
      NetNet*osig = new NetNet(lsig->scope(), path, NetNet::IMPLICIT, width);
      osig->local_flag(true);

      perm_string oname = osig->scope()->local_symbol();
      NetAddSub *adder = new NetAddSub(lsig->scope(), oname, width);
      for (unsigned idx = 0 ;  idx < width;  idx += 1) {
	    connect(lsig->pin(idx), adder->pin_DataA(idx));
	    connect(rsig->pin(idx), adder->pin_DataB(idx));
	    connect(osig->pin(idx), adder->pin_Result(idx));
      }
      des->add_node(adder);

      switch (op()) {
	  case '+':
	    adder->attribute(perm_string::literal("LPM_Direction"), verinum("ADD"));
	    break;
	  case '-':
	    adder->attribute(perm_string::literal("LPM_Direction"), verinum("SUB"));
	    break;
      }

      return osig;
}

/*
 * The bitwise logic operators are turned into discrete gates pretty
 * easily. Synthesize the left and right sub-expressions to get
 * signals, then just connect a single gate to each bit of the vector
 * of the expression.
 */
NetNet* NetEBBits::synthesize(Design*des)
{
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      NetScope*scope = lsig->scope();
      assert(scope);
      string path = des->local_symbol(scope->name());

      if (lsig->pin_count() != rsig->pin_count()) {
	    cerr << get_line() << ": internal error: bitwise (" << op_
		 << ") widths do not match: " << lsig->pin_count()
		 << " != " << rsig->pin_count() << endl;
	    cerr << get_line() << ":               : width="
		 << lsig->pin_count() << ": " << *left_ << endl;
	    cerr << get_line() << ":               : width="
		 << rsig->pin_count() << ": " << *right_ << endl;
	    return 0;
      }

      assert(lsig->pin_count() == rsig->pin_count());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lsig->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    perm_string oname = scope->local_symbol();
	    NetLogic*gate;

	      /* If the rsig bit is constant, then look for special
		 cases that I can use to reduce the generated
		 logic. If I find one, then handle it immediately and
		 skip the rest of the processing of this bit. */
	    if (rsig->pin(idx).nexus()->drivers_constant()) {
		  verinum::V bval = rsig->pin(idx).nexus()->driven_value();

		    /* (A & 0) is (0) */
		  if ((op() == '&') && bval == verinum::V0) {
			connect(osig->pin(idx), rsig->pin(idx));
			continue;
		  }

		    /* (A & 1) is A */
		  if ((op() == '&') && bval == verinum::V1) {
			connect(osig->pin(idx), lsig->pin(idx));
			continue;
		  }
	    }

	    switch (op()) {
		case '&':
		  gate = new NetLogic(scope, oname, 3, NetLogic::AND);
		  break;
		case '|':
		  gate = new NetLogic(scope, oname, 3, NetLogic::OR);
		  break;
		case '^':
		  gate = new NetLogic(scope, oname, 3, NetLogic::XOR);
		  break;
		case 'O':
		  gate = new NetLogic(scope, oname, 3, NetLogic::NOR);
		  break;
		case 'X':
		  gate = new NetLogic(scope, oname, 3, NetLogic::XNOR);
		  break;
		default:
		  assert(0);
	    }

	    connect(osig->pin(idx), gate->pin(0));
	    connect(lsig->pin(idx), gate->pin(1));
	    connect(rsig->pin(idx), gate->pin(2));

	    des->add_node(gate);
      }

      return osig;
}

NetNet* NetEBComp::synthesize(Design*des)
{
      NetEConst*lcon = reinterpret_cast<NetEConst*>(left_);
      NetEConst*rcon = reinterpret_cast<NetEConst*>(right_);

	/* Handle the special case where one of the inputs is constant
	   0. We can use an OR gate to do the comparison. Synthesize
	   the non-const side as normal, then or(nor) the signals
	   together to get result. */
      if ((rcon && (rcon->value() == verinum(0UL,rcon->expr_width())))
	  || (lcon && (lcon->value() == verinum(0UL,lcon->expr_width())))) {

	    NetNet*lsig = rcon
		  ? left_->synthesize(des)
		  : right_->synthesize(des);
	    NetScope*scope = lsig->scope();
	    assert(scope);

	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, 1);
	    osig->local_flag(true);

	    NetLogic*gate;
	    switch (op_) {
		case 'e':
		case 'E':
		  gate = new NetLogic(scope, scope->local_symbol(),
				      lsig->pin_count()+1, NetLogic::NOR);
		  break;
		case 'n':
		case 'N':
		  gate = new NetLogic(scope, scope->local_symbol(),
				      lsig->pin_count()+1, NetLogic::OR);
		  break;

		case '>':
		    /* sig > 0 is true if any bit in sig is set. This
		       is very much like sig != 0. (0 > sig) shouldn't
		       happen. */
		  if (rcon) {
			gate = new NetLogic(scope, scope->local_symbol(),
					    lsig->pin_count()+1, NetLogic::OR);
		  } else {
			assert(0);
			gate = new NetLogic(scope, scope->local_symbol(),
				      lsig->pin_count()+1, NetLogic::NOR);
		  }
		  break;

		case '<':
		    /* 0 < sig is handled like sig > 0. */
		  if (! rcon) {
			gate = new NetLogic(scope, scope->local_symbol(),
					    lsig->pin_count()+1, NetLogic::OR);
		  } else {
			assert(0);
			gate = new NetLogic(scope, scope->local_symbol(),
				      lsig->pin_count()+1, NetLogic::NOR);
		  }
		  break;

		default:
		  assert(0);
	    }

	    connect(gate->pin(0), osig->pin(0));
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		  connect(gate->pin(idx+1), lsig->pin(idx));

	    des->add_node(gate);
	    return osig;
      }

      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      NetScope*scope = lsig->scope();
      assert(scope);

      unsigned width = lsig->pin_count();
      if (rsig->pin_count() > lsig->pin_count())
	    width = rsig->pin_count();

      lsig = pad_to_width(des, lsig, width);
      rsig = pad_to_width(des, rsig, width);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->local_flag(true);

	/* Handle the special case of a single bit equality
	   operation. Make an XNOR gate instead of a comparator. */
      if ((width == 1) && ((op_ == 'e') || (op_ == 'E'))) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::XNOR);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }

	/* Handle the special case of a single bit inequality
	   operation. This is similar to single bit equality, but uses
	   an XOR instead of an XNOR gate. */
      if ((width == 1) && ((op_ == 'n') || (op_ == 'N'))) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::XOR);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }


      NetCompare*dev = new NetCompare(scope, scope->local_symbol(), width);
      des->add_node(dev);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(dev->pin_DataA(idx), lsig->pin(idx));

      for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
	    connect(dev->pin_DataB(idx), rsig->pin(idx));


      switch (op_) {
	  case '<':
	    connect(dev->pin_ALB(), osig->pin(0));
	    break;
	  case '>':
	    connect(dev->pin_AGB(), osig->pin(0));
	    break;
	  case 'e': // ==
	  case 'E': // === ?
	    connect(dev->pin_AEB(), osig->pin(0));
	    break;
	  case 'G': // >=
	    connect(dev->pin_AGEB(), osig->pin(0));
	    break;
	  case 'L': // <=
	    connect(dev->pin_ALEB(), osig->pin(0));
	    break;
	  case 'n': // !=
	  case 'N': // !==
	    connect(dev->pin_ANEB(), osig->pin(0));
	    break;

	  default:
	    cerr << get_line() << ": internal error: cannot synthesize "
		  "comparison: " << *this << endl;
	    des->errors += 1;
	    return 0;
      }

      return osig;
}

NetNet* NetEBMult::synthesize(Design*des)
{
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      if (lsig == 0)
	    return 0;

      if (rsig == 0)
	    return 0;

      NetScope*scope = lsig->scope();
      assert(scope);

      NetMult*mult = new NetMult(scope, scope->local_symbol(),
				 expr_width(),
				 lsig->pin_count(),
				 rsig->pin_count());
      des->add_node(mult);

      mult->set_signed( has_sign() );
      mult->set_line(*this);

      for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
	    connect(mult->pin_DataA(idx), lsig->pin(idx));

      for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
	    connect(mult->pin_DataB(idx), rsig->pin(idx));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
	    connect(mult->pin_Result(idx), osig->pin(idx));

      return osig;
}

NetNet* NetEBDiv::synthesize(Design*des)
{
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig  = right_->synthesize(des);

      NetScope*scope = lsig->scope();

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);

      switch (op()) {

	  case '/': {
		NetDivide*div = new NetDivide(scope, scope->local_symbol(),
					      expr_width(),
					      lsig->pin_count(),
					      rsig->pin_count());
		des->add_node(div);

		for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		      connect(div->pin_DataA(idx), lsig->pin(idx));
		for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		      connect(div->pin_DataB(idx), rsig->pin(idx));
		for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
		      connect(div->pin_Result(idx), osig->pin(idx));
		break;
	  }

	  case '%': {
		NetModulo*div = new NetModulo(scope, scope->local_symbol(),
					      expr_width(),
					      lsig->pin_count(),
					      rsig->pin_count());
		des->add_node(div);

		for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx += 1)
		      connect(div->pin_DataA(idx), lsig->pin(idx));
		for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx += 1)
		      connect(div->pin_DataB(idx), rsig->pin(idx));
		for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1)
		      connect(div->pin_Result(idx), osig->pin(idx));
		break;
	  }

	  default: {
		cerr << get_line() << ": internal error: "
		     << "NetEBDiv has unexpected op() code: "
		     << op() << endl;
		des->errors += 1;

		delete osig;
		return 0;
	  }
      }

      return osig;
}

NetNet* NetEBLogic::synthesize(Design*des)
{
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      if (lsig == 0)
	    return 0;

      if (rsig == 0)
	    return 0;

      NetScope*scope = lsig->scope();
      assert(scope);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->local_flag(true);


      if (op() == 'o') {

	      /* Logic OR can handle the reduction *and* the logical
		 comparison with a single wide OR gate. So handle this
		 magically. */

	    perm_string oname = scope->local_symbol();

	    NetLogic*olog = new NetLogic(scope, oname,
					 lsig->pin_count()+rsig->pin_count()+1,
					 NetLogic::OR);

	    connect(osig->pin(0), olog->pin(0));

	    unsigned pin = 1;
	    for (unsigned idx = 0 ;  idx < lsig->pin_count() ;  idx = 1)
		  connect(olog->pin(pin+idx), lsig->pin(idx));

	    pin += lsig->pin_count();
	    for (unsigned idx = 0 ;  idx < rsig->pin_count() ;  idx = 1)
		  connect(olog->pin(pin+idx), rsig->pin(idx));

	    des->add_node(olog);

      } else {
	    assert(op() == 'a');

	      /* Create the logic AND gate. This is a single bit
		 output, with inputs for each of the operands. */
	    NetLogic*olog;
	    perm_string oname = scope->local_symbol();

	    olog = new NetLogic(scope, oname, 3, NetLogic::AND);
	    olog->set_line(*this);

	    connect(osig->pin(0), olog->pin(0));
	    des->add_node(olog);

	      /* Here, I need to reduce the parameters with
		 reduction or. Only do this if we must. */
	    if (lsig->pin_count() > 1)
		  lsig = reduction_or(des, lsig);

	    if (rsig->pin_count() > 1)
		  rsig = reduction_or(des, rsig);

	      /* By this point, the left and right parameters have been
		 reduced to single bit values. Now we just connect them to
		 the logic gate. */
	    assert(lsig->pin_count() == 1);
	    connect(lsig->pin(0), olog->pin(1));

	    if (rsig->pin_count() != 1) {
		  cerr << olog->get_line() << ": internal error: "
		       << "right argument not reduced. expr=" << *this << endl;
	    }

	    assert(rsig->pin_count() == 1);
	    connect(rsig->pin(0), olog->pin(2));
      }


      return osig;
}

NetNet* NetEBShift::synthesize(Design*des)
{
      if (! dynamic_cast<NetEConst*>(right_)) {
	    NetExpr*tmp = right_->eval_tree();
	    if (tmp) {
		  delete right_;
		  right_ = tmp;
	    }
      }

      NetNet*lsig = left_->synthesize(des);
      if (lsig == 0)
	    return 0;

      bool right_flag  =  op_ == 'r' || op_ == 'R';
      bool signed_flag =  op_ == 'R';

      NetScope*scope = lsig->scope();

	/* Detect the special case where the shift amount is
	   constant. Evaluate the shift amount, and simply reconnect
	   the left operand to the output, but shifted. */
      if (NetEConst*rcon = dynamic_cast<NetEConst*>(right_)) {
	    verinum shift_v = rcon->value();
	    long shift = shift_v.as_long();

	    if (op() == 'r')
		  shift = 0-shift;

	    if (shift == 0)
		  return lsig;

	    NetNet*osig = new NetNet(scope, scope->local_symbol(),
				     NetNet::IMPLICIT, expr_width());
	    osig->local_flag(true);

	    NetConst*zcon = new NetConst(scope, scope->local_symbol(),
					 verinum::V0);
	    des->add_node(zcon);
	    NetNet*zsig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, 1);
	    connect(zcon->pin(0), zsig->pin(0));

	    if (shift > 0) {
		  unsigned long ushift = shift;
		  for (unsigned idx = 0; idx < osig->pin_count(); idx += 1)
			if (idx < ushift) {
			      connect(osig->pin(idx), zsig->pin(0));
			} else {
			      connect(osig->pin(idx), lsig->pin(idx-ushift));
			}
	    } else {
		  unsigned long dshift = 0-shift;
		  for (unsigned idx = 0; idx < osig->pin_count() ;  idx += 1)
			if (idx+dshift < lsig->pin_count())
			      connect(osig->pin(idx), lsig->pin(idx+dshift));
			else
			      connect(osig->pin(idx), zsig->pin(0));
	    }


	    return osig;
      }

      NetNet*rsig = right_->synthesize(des);
      if (rsig == 0)
	    return 0;

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);

      NetCLShift*dev = new NetCLShift(scope, scope->local_symbol(),
				      osig->pin_count(),
				      rsig->pin_count(),
				      right_flag, signed_flag);
      des->add_node(dev);

      for (unsigned idx = 0 ; idx < dev->width() ;  idx += 1)
	    connect(dev->pin_Result(idx), osig->pin(idx));

      assert(lsig->pin_count() >= dev->width());
      for (unsigned idx = 0 ;  idx < dev->width() ;  idx += 1)
	    connect(dev->pin_Data(idx), lsig->pin(idx));

      for (unsigned idx = 0 ;  idx < dev->width_dist() ;  idx += 1)
	    connect(dev->pin_Distance(idx), rsig->pin(idx));

      return osig;
}

NetNet* NetEConcat::synthesize(Design*des)
{
	/* First, synthesize the operands. */
      NetNet**tmp = new NetNet*[parms_.count()];
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1)
	    tmp[idx] = parms_[idx]->synthesize(des);

      assert(tmp[0]);
      NetScope*scope = tmp[0]->scope();
      assert(scope);

	/* Make a NetNet object to carry the output vector. */
      perm_string path = scope->local_symbol();
      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);

	/* Connect the output vector to the operands. */
      unsigned obit = 0;
      for (unsigned idx = parms_.count() ;  idx > 0 ;  idx -= 1) {

	    assert(tmp[idx-1]);

	    for (unsigned bit = 0;  bit < tmp[idx-1]->pin_count(); bit += 1) {
		  connect(osig->pin(obit), tmp[idx-1]->pin(bit));
		  obit += 1;
	    }

	    if (tmp[idx-1]->local_flag() && tmp[idx-1]->get_refs() == 0)
		  delete tmp[idx-1];
      }

      delete[]tmp;
      return osig;
}

NetNet* NetEConst::synthesize(Design*des)
{
      NetScope*scope = des->find_root_scope();
      assert(scope);

      perm_string path = scope->local_symbol();
      unsigned width=expr_width();

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, width);
      osig->local_flag(true);
      osig->set_signed(has_sign());
      NetConst*con = new NetConst(scope, scope->local_symbol(), value());
      for (unsigned idx = 0 ;  idx < width;  idx += 1)
	    connect(osig->pin(idx), con->pin(idx));

      des->add_node(con);
      return osig;
}

NetNet* NetEMemory::synthesize(Design*des)
{
      NetScope*scope = mem_->scope();

      NetNet*explode = mem_->explode_to_reg();
      unsigned width = expr_width();

      assert(idx_);
      NetNet*addr = idx_->synthesize(des);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT,
			       width);
      osig->set_line(*this);

      if (explode) {
	    if (debug_synth)
		  cerr << get_line() << ": debug: synthesize read of "
		       << explode->pin_count() << " bit exploded memory." << endl;

	      /* Only make a mux big enough to address the words that
		 the address can generate. (If the address is
		 0-extended, then only the low words are addressable.) */
	    unsigned use_count = mem_->count();
	    if (use_count > (1U << addr->pin_count())) {
		  use_count = 1 << addr->pin_count();

		  if (debug_synth)
			cerr << get_line() << ": debug: "
			     << "Index expression can only address "
			     << use_count << " of "
			     << mem_->count() << " words." << endl;
	    }

	      /* This is a reference to an exploded memory. So locate
		 the reg vector and use the addr expression as a
		 select into a MUX. */
	    NetMux*mux = new NetMux(scope, scope->local_symbol(),
				    width, use_count, addr->pin_count());
	    des->add_node(mux);
	    mux->set_line(*this);

	    for (unsigned idx = 0 ;  idx < width ;  idx += 1)
		  connect(mux->pin_Result(idx), osig->pin(idx));
	    for (unsigned idx = 0 ;  idx < mux->sel_width() ;  idx += 1)
		  connect(mux->pin_Sel(idx), addr->pin(idx));

	    for (unsigned wrd = 0 ;  wrd < use_count ;  wrd += 1)
		  for (unsigned idx = 0 ;  idx < width ;  idx += 1) {
			unsigned bit = wrd*width + idx;
			connect(mux->pin_Data(idx, wrd), explode->pin(bit));
		  }

	    if (debug_synth)
		  cerr << get_line() << ": debug: synthesis done." << endl;

      } else {
	    cerr << get_line() << ": internal error: Synthesize memory "
		 << "expression that is not exploded?" << endl;
	    des->errors += 1;
      }

      return osig;
}

NetNet* NetECReal::synthesize(Design*des)
{
      cerr << get_line() << ": error: Real constants are "
	   << "not synthesizable." << endl;
      des->errors += 1;
      return 0;
}

/*
 * The bitwise unary logic operator (there is only one) is turned
 * into discrete gates just as easily as the binary ones above.
 */
NetNet* NetEUBits::synthesize(Design*des)
{
      NetNet*isig = expr_->synthesize(des);

      NetScope*scope = isig->scope();
      assert(scope);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, isig->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    perm_string oname = scope->local_symbol();
	    NetLogic*gate;

	    switch (op()) {
		case '~':
		  gate = new NetLogic(scope, oname, 2, NetLogic::NOT);
		  break;
		default:
		  assert(0);
	    }

	    connect(osig->pin(idx), gate->pin(0));
	    connect(isig->pin(idx), gate->pin(1));

	    des->add_node(gate);
      }

      return osig;
}

NetNet* NetEUFunc::synthesize(Design*des)
{
      assert(func_);

      NetFuncDef* def = func_->func_def();
      assert(def);

      assert(parms_.count() == def->port_count());

      svector<NetNet*> inports (parms_.count());

      unsigned errors = 0;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    inports[idx] = parms_[idx]->synthesize(des);
	    if (inports[idx] == 0)
		  errors += 1;
      }

      if (errors > 0) {
	    cerr << get_line() << ": error: "
		 << "Cannot continue with function instance synthesis."
		 << endl;
	    return 0;
      }

      NetNet*out = def->synthesize(des, inports);

      if (! out) {
	    cerr << get_line() << ": error: "
		 << "User defined functions do not synthesize." << endl;
	    def->dump(cerr, 4);
	    des->errors += 1;
	    return 0;
      }

      return out;
}

NetNet* NetFuncDef::synthesize(Design*des, const svector<NetNet*>&inports_)
{

	/* First run synthesis for the function definition as if this
	   where a toplevel process. This will create the logic that
	   the function represents, but connected to the port nets for
	   the function definition. We will detach those common ports
	   later. */
      NexusSet nex_set;
      statement_->nex_output(nex_set);

      const perm_string tmp1 = scope()->local_symbol();
      NetNet*nex_out = new NetNet(scope(), tmp1, NetNet::WIRE,
				  nex_set.count());
      for (unsigned idx = 0 ;  idx < nex_out->pin_count() ;  idx += 1)
	    connect(nex_set[idx], nex_out->pin(idx));

      bool flag = statement_->synth_async_noaccum(des, scope(), false, 0,
						  nex_out, nex_out);

      if (!flag) {
	    delete nex_out;
	    return 0;
      }

	/* Connect the inports_ vectors to the input ports and detach
	   the static arg ports themselves. This moves the input from
	   the synthesized device from the static ports to the actual
	   ports from the instance context. */
      for (unsigned idx = 0 ;  idx < port_count() ;  idx += 1) {
	    NetNet*in = inports_[idx];
	    NetNet*arg = ports_[idx];

	    in = pad_to_width(des, in, arg->pin_count());

	    for (unsigned pin = 0 ;  pin < arg->pin_count() ;  pin += 1) {
		  connect(in->pin(pin), arg->pin(pin));
		  arg->pin(pin).unlink();
	    }
      }

	/* Detach the output signal from the synthesized result. We
	   use instead the nex_out that was returned from the
	   synthesis of the function. This is how we account for the
	   fact that the function may be synthesized multiple times to
	   go into multiple expression. Each synthesis needs a unique
	   output. */
      for (unsigned idx = 0 ;  idx < result_sig_->pin_count() ;  idx += 1)
	    result_sig_->pin(idx).unlink();

      return nex_out;
}

NetNet* NetEUReduce::synthesize(Design*des)
{
      NetNet*isig = expr_->synthesize(des);

      NetScope*scope = isig->scope();
      assert(scope);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case 'N':
	  case '!':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::NOR);
	    break;

	  case '&':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::AND);
	    break;

	  case '|':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::OR);
	    break;

	  case '^':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::XOR);
	    break;

	  case 'A':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::NAND);
	    break;

	  case 'X':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::XNOR);
	    break;

	  default:
	    cerr << get_line() << ": internal error: "
		 << "Unable to synthesize " << *this << "." << endl;
	    return 0;
      }

      des->add_node(gate);
      connect(gate->pin(0), osig->pin(0));
      for (unsigned idx = 0 ;  idx < isig->pin_count() ;  idx += 1)
	    connect(gate->pin(1+idx), isig->pin(idx));

      return osig;
}

NetNet* NetESelect::synthesize(Design *des)
{
	// XXXX For now, only support pad form
      assert(base_ == 0);

      NetNet*sub = expr_->synthesize(des);
      if (sub == 0)
	    return 0;

      NetScope*scope = sub->scope();
      NetNet*net = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, expr_width());
      if (has_sign()) {
	    unsigned idx;

	    for (idx = 0 ;  idx < sub->pin_count() ;  idx += 1)
		  connect(sub->pin(idx), net->pin(idx));

	    for ( ;  idx < net->pin_count(); idx += 1)
		  connect(sub->pin(sub->pin_count()-1), net->pin(idx));

      } else {
	    unsigned idx;
	    for (idx = 0 ;  idx < sub->pin_count() ;  idx += 1)
		  connect(sub->pin(idx), net->pin(idx));

	    NetConst*tmp = new NetConst(scope, scope->local_symbol(),
					verinum::V0);

	    for ( ;  idx < net->pin_count() ;  idx += 1)
		  connect(net->pin(idx), tmp->pin(0));

	    des->add_node(tmp);
      }

      return net;
}

/*
 * Synthesize a ?: operator as a NetMux device. Connect the condition
 * expression to the select input, then connect the true and false
 * expressions to the B and A inputs. This way, when the select input
 * is one, the B input, which is the true expression, is selected.
 */
NetNet* NetETernary::synthesize(Design *des)
{
      NetNet*csig = cond_->synthesize(des);
      NetNet*tsig = true_val_->synthesize(des);
      NetNet*fsig = false_val_->synthesize(des);

      perm_string path = csig->scope()->local_symbol();

      assert(csig->pin_count() == 1);

      unsigned width=expr_width();
      NetNet*osig = new NetNet(csig->scope(), path, NetNet::IMPLICIT, width);
      osig->local_flag(true);

	/* Make sure both value operands are the right width. */
      tsig = pad_to_width(des, tsig, width);
      fsig = pad_to_width(des, fsig, width);

      assert(width <= tsig->pin_count());
      assert(width <= fsig->pin_count());

      perm_string oname = csig->scope()->local_symbol();
      NetMux *mux = new NetMux(csig->scope(), oname, width, 2, 1);
      mux->set_line(*this);
      for (unsigned idx = 0 ;  idx < width;  idx += 1) {
	    connect(tsig->pin(idx), mux->pin_Data(idx, 1));
	    connect(fsig->pin(idx), mux->pin_Data(idx, 0));
	    connect(osig->pin(idx), mux->pin_Result(idx));
      }
      des->add_node(mux);
      connect(csig->pin(0), mux->pin_Sel(0));

      return osig;
}

/*
 * When synthesizing a signal expression, it is usually fine to simply
 * return the NetNet that it refers to. If this is a part select,
 * though, a bit more work needs to be done. Return a temporary that
 * represents the connections to the selected bits.
 *
 * For example, if there is a reg foo, like so:
 *     reg [5:0] foo;
 * and this expression node represents a part select foo[3:2], then
 * create a temporary like so:
 *
 *                     foo
 *                    +---+
 *                    | 5 |
 *                    +---+
 *         tmp        | 4 |
 *        +---+       +---+
 *        | 1 | <---> | 3 |
 *        +---+       +---+
 *        | 0 | <---> | 2 |
 *        +---+       +---+
 *                    | 1 |
 *                    +---+
 *                    | 0 |
 *                    +---+
 * The temporary is marked as a temporary and returned to the
 * caller. This causes the caller to get only the selected part of the
 * signal, and when it hooks up to tmp, it hooks up to the right parts
 * of foo.
 */
NetNet* NetESignal::synthesize(Design*des)
{
      if (warn_unused) {
	    if (net_->peek_lref() == 0 && net_->type()==NetNet::REG) {
		  cerr << get_line() << ": warning: "
		       << "reg " << net_->name()
		       << " is used by logic but never assigned." << endl;
	    }

	    if (net_->peek_lref() == 0 && net_->type()==NetNet::INTEGER) {
		  cerr << get_line() << ": warning: "
		       << "reg " << net_->name()
		       << " is used by logic but never assigned." << endl;
	    }
      }

	/* If there is no part select, then the synthesis is trivial. */
      if ((lsi_ == 0) && (msi_ == (net_->pin_count() - 1)))
	    return net_;

      assert(msi_ >= lsi_);
      unsigned wid = msi_ - lsi_ + 1;

      NetScope*scope = net_->scope();
      assert(scope);

      perm_string name = scope->local_symbol();
      NetNet*tmp = new NetNet(scope, name, NetNet::WIRE, wid);
      tmp->local_flag(true);
      tmp->set_line(*this);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
	    connect(tmp->pin(idx), net_->pin(idx+lsi_));

      return tmp;
}
