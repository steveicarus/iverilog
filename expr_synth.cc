/*
 * Copyright (c) 1999-2005 Stephen Williams (steve@icarus.com)
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
#ident "$Id: expr_synth.cc,v 1.80 2006/08/08 05:11:37 steve Exp $"
#endif

# include "config.h"

# include  <iostream>

# include  "netlist.h"
# include  "netmisc.h"

NetNet* NetExpr::synthesize(Design*des)
{
      cerr << get_line() << ": internal error: cannot synthesize expression: "
	   << *this << endl;
      des->errors += 1;
      return 0;
}

/*
 * Make an LPM_ADD_SUB device from addition operators.
 */
NetNet* NetEBAdd::synthesize(Design*des)
{
      assert((op()=='+') || (op()=='-'));

      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      assert(expr_width() >= lsig->vector_width());
      assert(expr_width() >= rsig->vector_width());

      lsig = pad_to_width(des, lsig, expr_width());
      rsig = pad_to_width(des, rsig, expr_width());

      assert(lsig->vector_width() == rsig->vector_width());
      unsigned width=lsig->vector_width();

      perm_string path = lsig->scope()->local_symbol();
      NetNet*osig = new NetNet(lsig->scope(), path, NetNet::IMPLICIT, width);
      osig->local_flag(true);
      osig->data_type(expr_type());

      perm_string oname = osig->scope()->local_symbol();
      NetAddSub *adder = new NetAddSub(lsig->scope(), oname, width);
      connect(lsig->pin(0), adder->pin_DataA());
      connect(rsig->pin(0), adder->pin_DataB());
      connect(osig->pin(0), adder->pin_Result());
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

      if (lsig->vector_width() != rsig->vector_width()) {
	    cerr << get_line() << ": internal error: bitwise (" << op_
		 << ") widths do not match: " << lsig->vector_width()
		 << " != " << rsig->vector_width() << endl;
	    cerr << get_line() << ":               : width="
		 << lsig->vector_width() << ": " << *left_ << endl;
	    cerr << get_line() << ":               : width="
		 << rsig->vector_width() << ": " << *right_ << endl;
	    return 0;
      }

      assert(lsig->vector_width() == rsig->vector_width());
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, lsig->vector_width());
      osig->local_flag(true);
      osig->data_type(expr_type());

      perm_string oname = scope->local_symbol();
      unsigned wid = lsig->vector_width();
      NetLogic*gate;

      switch (op()) {
	  case '&':
	    gate = new NetLogic(scope, oname, 3, NetLogic::AND, wid);
	    break;
	  case '|':
	    gate = new NetLogic(scope, oname, 3, NetLogic::OR, wid);
	    break;
	  case '^':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XOR, wid);
	    break;
	  case 'O':
	    gate = new NetLogic(scope, oname, 3, NetLogic::NOR, wid);
	    break;
	  case 'X':
	    gate = new NetLogic(scope, oname, 3, NetLogic::XNOR, wid);
	    break;
	  default:
	    assert(0);
      }

      connect(osig->pin(0), gate->pin(0));
      connect(lsig->pin(0), gate->pin(1));
      connect(rsig->pin(0), gate->pin(2));

      gate->set_line(*this);
      des->add_node(gate);

      return osig;
}

NetNet* NetEBComp::synthesize(Design*des)
{

      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      NetScope*scope = lsig->scope();
      assert(scope);

      unsigned width = lsig->vector_width();
      if (rsig->vector_width() > width)
	    width = rsig->vector_width();

      lsig = pad_to_width(des, lsig, width);
      rsig = pad_to_width(des, rsig, width);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->set_line(*this);
      osig->local_flag(true);
      osig->data_type(IVL_VT_LOGIC);

	/* Handle the special case of a single bit equality
	   operation. Make an XNOR gate instead of a comparator. */
      if ((width == 1) && ((op_ == 'e') || (op_ == 'E'))) {
	    NetLogic*gate = new NetLogic(scope, scope->local_symbol(),
					 3, NetLogic::XNOR, 1);
	    gate->set_line(*this);
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
					 3, NetLogic::XOR, 1);
	    gate->set_line(*this);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }


      NetCompare*dev = new NetCompare(scope, scope->local_symbol(), width);
      dev->set_line(*this);
      des->add_node(dev);

      connect(dev->pin_DataA(), lsig->pin(0));
      connect(dev->pin_DataB(), rsig->pin(0));


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

NetNet* NetEBPow::synthesize(Design*des)
{
      cerr << get_line() << ": internal error: Do not yet know how to handle"
	   << " power operator in this context." << endl;
      des->errors += 1;
      return 0;
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
				 lsig->vector_width(),
				 rsig->vector_width());
      des->add_node(mult);

      mult->set_signed( has_sign() );
      mult->set_line(*this);

      connect(mult->pin_DataA(), lsig->pin(0));
      connect(mult->pin_DataB(), rsig->pin(0));

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->data_type(lsig->data_type());
      osig->set_line(*this);
      osig->data_type(expr_type());
      osig->local_flag(true);

      connect(mult->pin_Result(), osig->pin(0));

      return osig;
}

NetNet* NetEBDiv::synthesize(Design*des)
{
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig  = right_->synthesize(des);

      NetScope*scope = lsig->scope();

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->set_line(*this);
      osig->data_type(lsig->data_type());
      osig->local_flag(true);

      switch (op()) {

	  case '/': {
		NetDivide*div = new NetDivide(scope, scope->local_symbol(),
					      expr_width(),
					      lsig->vector_width(),
					      rsig->vector_width());
		div->set_line(*this);
		des->add_node(div);

		connect(div->pin_DataA(), lsig->pin(0));
		connect(div->pin_DataB(), rsig->pin(0));
		connect(div->pin_Result(),osig->pin(0));
		break;
	  }

	  case '%': {
		NetModulo*div = new NetModulo(scope, scope->local_symbol(),
					      expr_width(),
					      lsig->vector_width(),
					      rsig->vector_width());
		div->set_line(*this);
		des->add_node(div);

		connect(div->pin_DataA(), lsig->pin(0));
		connect(div->pin_DataB(), rsig->pin(0));
		connect(div->pin_Result(),osig->pin(0));
		break;
	  }

	  default: {
		cerr << get_line() << ": internal error: "
		     << "NetEBDiv has unexpeced op() code: "
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
      osig->data_type(expr_type());
      osig->local_flag(true);


      if (op() == 'o') {

	      /* Logic OR can handle the reduction *and* the logical
		 comparison with a single wide OR gate. So handle this
		 magically. */

	    perm_string oname = scope->local_symbol();

	    NetLogic*olog = new NetLogic(scope, oname,
					 lsig->pin_count()+rsig->pin_count()+1,
					 NetLogic::OR, 1);

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

	    olog = new NetLogic(scope, oname, 3, NetLogic::AND, 1);

	    connect(osig->pin(0), olog->pin(0));
	    des->add_node(olog);

	      /* XXXX Here, I need to reduce the parameters with
		 reduction or. */


	      /* By this point, the left and right parameters have been
		 reduced to single bit values. Now we just connect them to
		 the logic gate. */
	    assert(lsig->pin_count() == 1);
	    connect(lsig->pin(0), olog->pin(1));

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
	    osig->data_type(expr_type());
	    osig->local_flag(true);

	    unsigned long ushift = shift>=0? shift : -shift;
	    if (ushift > osig->vector_width())
		  ushift = osig->vector_width();

	    verinum znum (verinum::V0, ushift, true);
	    NetConst*zcon = new NetConst(scope, scope->local_symbol(),
					 znum);
	    des->add_node(zcon);

	      /* Detect the special case that the shift is the size of
		 the whole expression. Simply connect the pad to the
		 osig and escape. */
	    if (ushift >= osig->vector_width()) {
		  connect(zcon->pin(0), osig->pin(0));
		  return osig;
	    }

	    NetNet*zsig = new NetNet(scope, scope->local_symbol(),
				     NetNet::WIRE, znum.len());
	    zsig->data_type(osig->data_type());
	    connect(zcon->pin(0), zsig->pin(0));

	    NetConcat*ccat = new NetConcat(scope, scope->local_symbol(),
					   osig->vector_width(), 2);
	    ccat->set_line(*this);
	    des->add_node(ccat);

	    connect(ccat->pin(0), osig->pin(0));
	    if (shift > 0) {
		  connect(ccat->pin(1), zsig->pin(0));
		  connect(ccat->pin(2), lsig->pin(0));
	    } else {
		  connect(ccat->pin(1), lsig->pin(0));
		  connect(ccat->pin(2), zsig->pin(0));
	    }

	    return osig;
      }

      NetNet*rsig = right_->synthesize(des);
      if (rsig == 0)
	    return 0;

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, expr_width());
      osig->data_type(expr_type());
      osig->local_flag(true);

      assert(op() == 'l');
      NetCLShift*dev = new NetCLShift(scope, scope->local_symbol(),
				      osig->vector_width(),
				      rsig->vector_width(),
				      right_flag, signed_flag);
      dev->set_line(*this);
      des->add_node(dev);

      connect(dev->pin_Result(), osig->pin(0));

      assert(lsig->vector_width() == dev->width());
      connect(dev->pin_Data(), lsig->pin(0));

      connect(dev->pin_Distance(), rsig->pin(0));

      return osig;
}

NetNet* NetEConcat::synthesize(Design*des)
{
	/* First, synthesize the operands. */
      NetNet**tmp = new NetNet*[parms_.count()];
      bool flag = true;
      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    tmp[idx] = parms_[idx]->synthesize(des);
	    if (tmp[idx] == 0)
		  flag = false;
      }

      if (flag == false)
	    return 0;

      assert(tmp[0]);
      NetScope*scope = tmp[0]->scope();
      assert(scope);

	/* Make a NetNet object to carry the output vector. */
      perm_string path = scope->local_symbol();
      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, expr_width());
      osig->local_flag(true);
      osig->data_type(tmp[0]->data_type());

      NetConcat*concat = new NetConcat(scope, scope->local_symbol(),
				       osig->vector_width(), parms_.count());
      concat->set_line(*this);
      des->add_node(concat);
      connect(concat->pin(0), osig->pin(0));

      for (unsigned idx = 0 ;  idx < parms_.count() ;  idx += 1) {
	    connect(concat->pin(idx+1), tmp[parms_.count()-idx-1]->pin(0));
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

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, width-1,0);
      osig->local_flag(true);
      osig->data_type(IVL_VT_LOGIC);
      osig->set_signed(has_sign());
      NetConst*con = new NetConst(scope, scope->local_symbol(), value());
      connect(osig->pin(0), con->pin(0));

      des->add_node(con);
      return osig;
}

/*
* Create a NetLiteral object to represent real valued constants.
*/
NetNet* NetECReal::synthesize(Design*des)
{
      NetScope*scope = des->find_root_scope();
      assert(scope);

      perm_string path = scope->local_symbol();

      NetNet*osig = new NetNet(scope, path, NetNet::WIRE, 1);
      osig->local_flag(true);
      osig->data_type(IVL_VT_REAL);
      osig->set_signed(has_sign());
      osig->set_line(*this);

      NetLiteral*con = new NetLiteral(scope, scope->local_symbol(), value_);
      des->add_node(con);
      con->set_line(*this);

      connect(osig->pin(0), con->pin(0));
      return osig;
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

      unsigned width = isig->vector_width();
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, width);
      osig->data_type(expr_type());
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case '~':
	    gate = new NetLogic(scope, oname, 2, NetLogic::NOT, width);
	    break;
	  default:
	    assert(0);
      }

      connect(osig->pin(0), gate->pin(0));
      connect(isig->pin(0), gate->pin(1));

      des->add_node(gate);

      return osig;
}

NetNet* NetEUReduce::synthesize(Design*des)
{
      NetNet*isig = expr_->synthesize(des);

      NetScope*scope = isig->scope();
      assert(scope);

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, 1);
      osig->data_type(expr_type());
      osig->local_flag(true);

      perm_string oname = scope->local_symbol();
      NetLogic*gate;

      switch (op()) {
	  case 'N':
	  case '!':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::NOR, 1);
	    break;

	  case '&':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::AND, 1);
	    break;

	  case '|':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::OR, 1);
	    break;

	  case '^':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::XOR, 1);
	    break;

	  case 'A':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::NAND, 1);
	    break;

	  case 'X':
	    gate = new NetLogic(scope, oname, isig->pin_count()+1,
				NetLogic::XNOR, 1);
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

NetNet* NetEMemory::synthesize(Design *des)
{
      NetNet*adr = idx_->synthesize(des);

      NetScope*scope = adr->scope();

      NetRamDq*ram = new NetRamDq(scope, scope->local_symbol(),
				  mem_, adr->vector_width());
      des->add_node(ram);
      ram->set_line(*this);

      connect(ram->pin_Address(), adr->pin(0));

	/* Create an output signal to receive the data. Assume that
	   memories return LOGIC. */
      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT, ram->width());
      osig->data_type(IVL_VT_LOGIC);
      osig->local_flag(true);
      osig->set_line(*this);

      connect(ram->pin_Q(), osig->pin(0));

      return osig;

}

NetNet* NetESelect::synthesize(Design *des)
{

      NetNet*sub = expr_->synthesize(des);
      if (sub == 0)
	    return 0;

      NetScope*scope = sub->scope();

      unsigned off = 0;

      if (base_ != 0) {
	      // For now, only handle constant part selects in this
	      // context. NOTE: the elaboration that created the
	      // NetESelect already translated the part base to
	      // canonical form, so the base_ is canonical already.
	    NetEConst*bcon = dynamic_cast<NetEConst*>(base_);
	    assert(bcon);

	    long bval = bcon->value().as_long();
	    assert(bval >= 0);
	    off = bval;
      }

	/* If there is a part select, then generate a PartSelect node
	   to actually do the part select. This does not expansion,
	   that is handled later. */
      if ((off != 0) || (off+expr_width() < sub->vector_width())) {
	    unsigned wid = expr_width();
	    if ((wid + off) > sub->vector_width())
		  wid = sub->vector_width() - off;

	    NetPartSelect*sel = new NetPartSelect(sub, off, wid,
						  NetPartSelect::VP);
	    sel->set_line(*this);
	    des->add_node(sel);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, expr_width());
	    tmp->data_type(sub->data_type());
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    sub = tmp;
	    connect(sub->pin(0), sel->pin(0));
      }

	/* Done? Vector is already the right width? then stop now. */
      if (sub->vector_width() == expr_width())
	    return sub;

      NetNet*net = new NetNet(scope, scope->local_symbol(),
			      NetNet::IMPLICIT, expr_width());
      net->data_type(expr_type());
      if (has_sign()) {
	    NetSignExtend*pad = new NetSignExtend(scope,
						  scope->local_symbol(),
						  expr_width());
	    pad->set_line(*this);
	    des->add_node(pad);

	    connect(pad->pin(1), sub->pin(0));
	    connect(pad->pin(0), net->pin(0));

      } else {

	    NetConcat*cat = new NetConcat(scope, scope->local_symbol(),
					  expr_width(), 2);
	    cat->set_line(*this);
	    des->add_node(cat);

	    assert(expr_width() > sub->vector_width());
	    unsigned pad_width = expr_width() - sub->vector_width();
	    verinum pad((uint64_t)0, pad_width);
	    NetConst*con = new NetConst(scope, scope->local_symbol(),
					pad);
	    con->set_line(*this);
	    des->add_node(con);

	    NetNet*tmp = new NetNet(scope, scope->local_symbol(),
				    NetNet::IMPLICIT, pad_width);
	    tmp->data_type(expr_type());
	    tmp->local_flag(true);
	    tmp->set_line(*this);
	    connect(tmp->pin(0), con->pin(0));

	    connect(cat->pin(0), net->pin(0));
	    connect(cat->pin(1), sub->pin(0));
	    connect(cat->pin(2), con->pin(0));
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

      assert(csig->vector_width() == 1);

      unsigned width=expr_width();
      NetNet*osig = new NetNet(csig->scope(), path, NetNet::IMPLICIT, width);
      osig->data_type(expr_type());
      osig->local_flag(true);

	/* Make sure both value operands are the right width. */
      tsig = crop_to_width(des, pad_to_width(des, tsig, width), width);
      fsig = crop_to_width(des, pad_to_width(des, fsig, width), width);

      assert(width == tsig->vector_width());
      assert(width == fsig->vector_width());

      perm_string oname = csig->scope()->local_symbol();
      NetMux *mux = new NetMux(csig->scope(), oname, width,
			       2, csig->vector_width());
      connect(tsig->pin(0), mux->pin_Data(1));
      connect(fsig->pin(0), mux->pin_Data(0));
      connect(osig->pin(0), mux->pin_Result());
      connect(csig->pin(0), mux->pin_Sel());
      des->add_node(mux);

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
      return net_;
}

/*
 * $Log: expr_synth.cc,v $
 * Revision 1.80  2006/08/08 05:11:37  steve
 *  Handle 64bit delay constants.
 *
 * Revision 1.79  2006/07/31 03:50:17  steve
 *  Add support for power in constant expressions.
 *
 * Revision 1.78  2006/07/08 21:48:46  steve
 *  Handle real valued literals in net contexts.
 *
 * Revision 1.77  2006/05/01 20:47:59  steve
 *  More explicit datatype setup.
 *
 * Revision 1.76  2006/05/01 05:40:21  steve
 *  fix net type of multiply output.
 *
 * Revision 1.75  2006/04/30 05:17:48  steve
 *  Get the data type of part select results right.
 *
 * Revision 1.74  2006/01/03 05:15:33  steve
 *  Fix the return type of a synthesized divide.
 *
 * Revision 1.73  2005/09/15 23:04:09  steve
 *  Make sure div, mod and mult nodes have line number info.
 *
 * Revision 1.72  2005/08/31 05:07:31  steve
 *  Handle memory references is continuous assignments.
 *
 * Revision 1.71  2005/06/13 23:22:14  steve
 *  use NetPartSelect to shrink part from high bits.
 *
 * Revision 1.70  2005/06/13 22:26:03  steve
 *  Make synthesized padding vector-aware.
 *
 * Revision 1.69  2005/05/15 04:47:00  steve
 *  synthesis of Logic and shifts using vector gates.
 *
 * Revision 1.68  2005/05/06 00:25:13  steve
 *  Handle synthesis of concatenation expressions.
 *
 * Revision 1.67  2005/04/25 01:30:31  steve
 *  synthesis of add and unary get vector widths right.
 *
 * Revision 1.66  2005/04/24 23:44:02  steve
 *  Update DFF support to new data flow.
 *
 * Revision 1.65  2005/03/12 06:43:35  steve
 *  Update support for LPM_MOD.
 *
 * Revision 1.64  2005/02/19 02:43:38  steve
 *  Support shifts and divide.
 *
 * Revision 1.63  2005/02/12 06:25:40  steve
 *  Restructure NetMux devices to pass vectors.
 *  Generate NetMux devices from ternary expressions,
 *  Reduce NetMux devices to bufif when appropriate.
 *
 * Revision 1.62  2005/01/28 05:39:33  steve
 *  Simplified NetMult and IVL_LPM_MULT.
 *
 * Revision 1.61  2005/01/16 04:20:32  steve
 *  Implement LPM_COMPARE nodes as two-input vector functors.
 *
 * Revision 1.60  2004/12/11 02:31:26  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.59  2004/06/30 02:16:26  steve
 *  Implement signed divide and signed right shift in nets.
 *
 * Revision 1.58  2004/06/16 16:21:34  steve
 *  Connect rsif of multiply to DataB.
 *
 * Revision 1.57  2004/06/12 15:00:02  steve
 *  Support / and % in synthesized contexts.
 *
 * Revision 1.56  2004/06/01 01:04:57  steve
 *  Fix synthesis method for logical and/or
 *
 * Revision 1.55  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.54  2004/02/18 17:11:56  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.53  2004/02/15 04:23:48  steve
 *  Fix evaluation of compare to constant expression.
 *
 * Revision 1.52  2003/11/10 19:39:20  steve
 *  Remove redundant scope tokens.
 *
 * Revision 1.51  2003/10/27 06:04:21  steve
 *  More flexible width handling for synthesized add.
 *
 * Revision 1.50  2003/09/26 02:44:27  steve
 *  Assure ternary arguments are wide enough.
 *
 * Revision 1.49  2003/09/03 23:31:36  steve
 *  Support synthesis of constant downshifts.
 *
 * Revision 1.48  2003/08/28 04:11:18  steve
 *  Spelling patch.
 *
 * Revision 1.47  2003/08/09 03:23:40  steve
 *  Add support for IVL_LPM_MULT device.
 *
 * Revision 1.46  2003/07/26 03:34:42  steve
 *  Start handling pad of expressions in code generators.
 *
 * Revision 1.45  2003/06/24 01:38:02  steve
 *  Various warnings fixed.
 *
 * Revision 1.44  2003/04/19 04:52:56  steve
 *  Less picky about expression widths while synthesizing ternary.
 *
 * Revision 1.43  2003/04/08 05:07:15  steve
 *  Detect constant shift distances in synthesis.
 *
 * Revision 1.42  2003/04/08 04:33:55  steve
 *  Synthesize shift expressions.
 *
 * Revision 1.41  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.40  2003/02/26 01:29:24  steve
 *  LPM objects store only their base names.
 *
 * Revision 1.39  2003/01/30 16:23:07  steve
 *  Spelling fixes.
 *
 * Revision 1.38  2003/01/26 21:15:58  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.37  2002/11/17 23:37:55  steve
 *  Magnitude compare to 0.
 *
 * Revision 1.36  2002/08/12 01:34:59  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.35  2002/07/07 22:31:39  steve
 *  Smart synthesis of binary AND expressions.
 *
 * Revision 1.34  2002/07/05 21:26:17  steve
 *  Avoid emitting to vvp local net symbols.
 *
 * Revision 1.33  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 */

