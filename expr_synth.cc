/*
 * Copyright (c) 1999-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: expr_synth.cc,v 1.31 2001/12/30 17:06:52 steve Exp $"
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
      
      assert(lsig->pin_count() == rsig->pin_count());
      unsigned width=lsig->pin_count();

      string path = lsig->scope()->name()+"."+lsig->scope()->local_symbol();
      NetNet*osig = new NetNet(lsig->scope(), path, NetNet::IMPLICIT, width);

      string oname = des->local_symbol(path);
      NetAddSub *adder = new NetAddSub(lsig->scope(), oname, width);
      for (unsigned idx = 0 ;  idx < width;  idx += 1) {
	    connect(lsig->pin(idx), adder->pin_DataA(idx));
	    connect(rsig->pin(idx), adder->pin_DataB(idx));
	    connect(osig->pin(idx), adder->pin_Result(idx));
      }
      des->add_node(adder);

      switch (op()) {
	  case '+':
	    adder->attribute("LPM_Direction", "ADD");
	    break;
	  case '-':
	    adder->attribute("LPM_Direction", "SUB");
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
      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT,
			       lsig->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    string oname = des->local_symbol(path);
	    NetLogic*gate;

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
	    string path = des->local_symbol(scope->name());

	    NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, 1);
	    osig->local_flag(true);

	    NetLogic*gate;
	    switch (op_) {
		case 'e':
		case 'E':
		  gate = new NetLogic(scope, des->local_symbol(path),
				      lsig->pin_count()+1, NetLogic::NOR);
		  break;
		case 'n':
		case 'N':
		  gate = new NetLogic(scope, des->local_symbol(path),
				      lsig->pin_count()+1, NetLogic::OR);
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
      string path = des->local_symbol(scope->name());

      unsigned width = lsig->pin_count();
      if (rsig->pin_count() > lsig->pin_count())
	    width = rsig->pin_count();

      lsig = pad_to_width(des, lsig, width);
      rsig = pad_to_width(des, rsig, width);

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, 1);
      osig->local_flag(true);

	/* Handle the special case of a single bit equality
	   operation. Make an XNOR gate instead of a comparator. */
      if ((width == 1) && ((op_ == 'e') || (op_ == 'E'))) {
	    NetLogic*gate = new NetLogic(scope, des->local_symbol(path),
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
	    NetLogic*gate = new NetLogic(scope, des->local_symbol(path),
					 3, NetLogic::XOR);
	    connect(gate->pin(0), osig->pin(0));
	    connect(gate->pin(1), lsig->pin(0));
	    connect(gate->pin(2), rsig->pin(0));
	    des->add_node(gate);
	    return osig;
      }


      NetCompare*dev = new NetCompare(scope, des->local_symbol(path), width);
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

NetNet* NetEBDiv::synthesize(Design*des)
{
      cerr << get_line() << ": internal error: cannot synthesize division: "
	   << *this << endl;
      des->errors += 1;
      return 0;
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
      string path = des->local_symbol(scope->name());

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, 1);
      osig->local_flag(true);


      if (op() == 'o') {

	      /* Logic OR can handle the reduction *and* the logical
		 comparison with a single wide OR gate. So handle this
		 magically. */

	    string oname = des->local_symbol(path);
	    NetLogic*olog;

	    olog = new NetLogic(scope, oname,
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
	    string oname = des->local_symbol(path);

	    olog = new NetLogic(scope, oname, 3, NetLogic::AND);

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
	    connect(lsig->pin(0), olog->pin(2));
      }


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
      string path = scope->name() + "." + scope->local_symbol();
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

	    if (tmp[idx-1]->local_flag() && tmp[idx-1]->get_eref() == 0)
		  delete tmp[idx-1];
      }

      delete[]tmp;
      return osig;
}

NetNet* NetEConst::synthesize(Design*des)
{
      NetScope*scope = des->find_root_scope();
      assert(scope);

      string path = scope->name() + "." + scope->local_symbol();
      unsigned width=expr_width();

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, width);
      osig->local_flag(true);
      NetConst*con = new NetConst(scope, des->local_symbol(path), value());
      for (unsigned idx = 0 ;  idx < width;  idx += 1)
	    connect(osig->pin(idx), con->pin(idx));

      des->add_node(con);
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
      string path = des->local_symbol(scope->name());

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT,
			       isig->pin_count());
      osig->local_flag(true);

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    string oname = des->local_symbol(path);
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

NetNet* NetEUReduce::synthesize(Design*des)
{
      NetNet*isig = expr_->synthesize(des);

      NetScope*scope = isig->scope();
      assert(scope);
      string path = des->local_symbol(scope->name());

      NetNet*osig = new NetNet(scope, path, NetNet::IMPLICIT, 1);
      osig->local_flag(true);

      string oname = des->local_symbol(path);
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

/*
 * Synthesize a ?: operator an a NetMux device. Connect the condition
 * expression to the select input, then connect the true and false
 * expressions to the B and A inputs. This way, when the select input
 * is one, the B input, which is the true expression, is selected.
 */
NetNet* NetETernary::synthesize(Design *des)
{
      NetNet*csig = cond_->synthesize(des);
      NetNet*tsig = true_val_->synthesize(des);
      NetNet*fsig = false_val_->synthesize(des);

      string path = csig->scope()->name()+"."+csig->scope()->local_symbol();

      assert(csig->pin_count() == 1);
      assert(tsig->pin_count() == fsig->pin_count());
      unsigned width=tsig->pin_count();
      NetNet*osig = new NetNet(csig->scope(), path, NetNet::IMPLICIT, width);
      osig->local_flag(true);

      string oname = des->local_symbol(path);
      NetMux *mux = new NetMux(csig->scope(), oname, width, 2, 1);
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
      if ((lsi_ == 0) && (msi_ == (net_->pin_count() - 1)))
	    return net_;

      assert(msi_ >= lsi_);
      unsigned wid = msi_ - lsi_ + 1;

      NetScope*scope = net_->scope();
      assert(scope);

      string name = scope->name() + "." + scope->local_symbol();
      NetNet*tmp = new NetNet(scope, name, NetNet::NetNet::WIRE, wid);
      tmp->local_flag(true);

      for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
	    connect(tmp->pin(idx), net_->pin(idx+lsi_));

      return tmp;
}

/*
 * $Log: expr_synth.cc,v $
 * Revision 1.31  2001/12/30 17:06:52  steve
 *  Synthesize reduction logic.
 *
 * Revision 1.30  2001/12/18 05:34:02  steve
 *  Comments about MUX synthesis.
 *
 * Revision 1.29  2001/11/29 01:58:18  steve
 *  Handle part selects in l-values of DFF devices.
 *
 * Revision 1.28  2001/10/28 01:14:53  steve
 *  NetObj constructor finally requires a scope.
 *
 * Revision 1.27  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.26  2001/08/31 22:59:48  steve
 *  synthesize the special case of compare with 0.
 *
 * Revision 1.25  2001/08/05 02:49:07  steve
 *  Properly synthesize part selects.
 *
 * Revision 1.24  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.23  2001/07/07 01:38:45  steve
 *  Put synthesized signals in proper scope.
 *
 * Revision 1.22  2001/06/15 04:14:18  steve
 *  Generate vvp code for GT and GE comparisons.
 *
 * Revision 1.21  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.20  2001/02/15 06:59:36  steve
 *  FreeBSD port has a maintainer now.
 *
 * Revision 1.19  2001/01/18 03:16:35  steve
 *  NetMux needs a scope. (PR#115)
 *
 * Revision 1.18  2000/11/29 23:16:18  steve
 *  Do not delete synthesized signals used in expressions.
 *
 * Revision 1.17  2000/11/29 05:24:00  steve
 *  synthesis for unary reduction ! and N operators.
 *
 * Revision 1.16  2000/11/29 02:09:52  steve
 *  Add support for || synthesis (PR#53)
 *
 * Revision 1.15  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.14  2000/05/02 00:58:12  steve
 *  Move signal tables to the NetScope class.
 *
 * Revision 1.13  2000/04/28 18:43:23  steve
 *  integer division in expressions properly get width.
 *
 * Revision 1.12  2000/04/20 00:28:03  steve
 *  Catch some simple identity compareoptimizations.
 *
 * Revision 1.11  2000/04/16 23:32:18  steve
 *  Synthesis of comparator in expressions.
 *
 *  Connect the NetEvent and related classes
 *  together better.
 *
 * Revision 1.10  2000/02/23 02:56:54  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.9  2000/01/01 06:18:00  steve
 *  Handle synthesis of concatenation.
 *
 * Revision 1.8  1999/12/17 06:18:15  steve
 *  Rewrite the cprop functor to use the functor_t interface.
 *
 * Revision 1.7  1999/12/17 03:38:46  steve
 *  NetConst can now hold wide constants.
 *
 * Revision 1.6  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.5  1999/11/27 19:07:57  steve
 *  Support the creation of scopes.
 *
 * Revision 1.4  1999/11/19 03:00:59  steve
 *  Whoops, created a signal with a duplicate name.
 *
 * Revision 1.3  1999/11/05 04:40:40  steve
 *  Patch to synthesize LPM_ADD_SUB from expressions,
 *  Thanks to Larry Doolittle. Also handle constants
 *  in expressions.
 *
 *  Synthesize adders in XNF, based on a patch from
 *  Larry. Accept synthesis of constants from Larry
 *  as is.
 *
 * Revision 1.2  1999/11/04 03:53:26  steve
 *  Patch to synthesize unary ~ and the ternary operator.
 *  Thanks to Larry Doolittle <LRDoolittle@lbl.gov>.
 *
 *  Add the LPM_MUX device, and integrate it with the
 *  ternary synthesis from Larry. Replace the lpm_mux
 *  generator in t-xnf.cc to use XNF EQU devices to
 *  put muxs into function units.
 *
 *  Rewrite elaborate_net for the PETernary class to
 *  also use the LPM_MUX device.
 *
 * Revision 1.1  1999/11/02 04:55:34  steve
 *  Add the synthesize method to NetExpr to handle
 *  synthesis of expressions, and use that method
 *  to improve r-value handling of LPM_FF synthesis.
 *
 *  Modify the XNF target to handle LPM_FF objects.
 *
 */

