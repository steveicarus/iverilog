/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: expr_synth.cc,v 1.2 1999/11/04 03:53:26 steve Exp $"
#endif

# include  "netlist.h"


NetNet* NetExpr::synthesize(Design*des)
{
      cerr << get_line() << ": internal error: cannot synthesize expression: "
	   << *this << endl;
      des->errors += 1;
      return 0;
}

/*
 * The bitwise logic operators are turned into discrete gates pretty
 * easily. Synthesize the left and right sub-expressions to get
 * signals, then just connect a single gate to each bit of the vector
 * of the expression.
 */
NetNet* NetEBBits::synthesize(Design*des)
{
      string path = des->local_symbol("SYNTH");
      NetNet*lsig = left_->synthesize(des);
      NetNet*rsig = right_->synthesize(des);

      assert(lsig->pin_count() == rsig->pin_count());
      NetNet*osig = new NetNet(path, NetNet::IMPLICIT, lsig->pin_count());

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    string oname = des->local_symbol(path);
	    NetLogic*gate;

	    switch (op()) {
		case '&':
		  gate = new NetLogic(oname, 3, NetLogic::AND);
		  break;
		case '|':
		  gate = new NetLogic(oname, 3, NetLogic::OR);
		  break;
		case '^':
		  gate = new NetLogic(oname, 3, NetLogic::XOR);
		  break;
		case 'O':
		  gate = new NetLogic(oname, 3, NetLogic::NOR);
		  break;
		case 'X':
		  gate = new NetLogic(oname, 3, NetLogic::XNOR);
		  break;
		default:
		  assert(0);
	    }

	    connect(osig->pin(idx), gate->pin(0));
	    connect(lsig->pin(idx), gate->pin(1));
	    connect(rsig->pin(idx), gate->pin(2));

	    des->add_node(gate);
      }

      des->add_signal(osig);
      return osig;
}

/*
 * The bitwise unary logic operator (there is only one) is turned
 * into discrete gates just as easily as the binary ones above.
 */
NetNet* NetEUBits::synthesize(Design*des)
{
      string path = des->local_symbol("SYNTH");
      NetNet*isig = expr_->synthesize(des);

      NetNet*osig = new NetNet(path, NetNet::IMPLICIT, isig->pin_count());

      for (unsigned idx = 0 ;  idx < osig->pin_count() ;  idx += 1) {
	    string oname = des->local_symbol(path);
	    NetLogic*gate;

	    switch (op()) {
		case '~':
		  gate = new NetLogic(oname, 2, NetLogic::NOT);
		  break;
		default:
		  assert(0);
	    }

	    connect(osig->pin(idx), gate->pin(0));
	    connect(isig->pin(idx), gate->pin(1));

	    des->add_node(gate);
      }
      des->add_signal(osig);
      return osig;
}


NetNet* NetETernary::synthesize(Design *des)
{
      string path = des->local_symbol("SYNTH");
      NetNet*csig = cond_->synthesize(des);
      NetNet*tsig = true_val_->synthesize(des);
      NetNet*fsig = false_val_->synthesize(des);

      assert(csig->pin_count() == 1);
      assert(tsig->pin_count() == fsig->pin_count());
      unsigned width=tsig->pin_count();
      NetNet*osig = new NetNet(path, NetNet::IMPLICIT, width);

      string oname = des->local_symbol(path);
      NetMux *mux = new NetMux(oname, width, 2, 1);
      for (unsigned idx = 0 ;  idx < width;  idx += 1) {
	    connect(tsig->pin(idx), mux->pin_Data(idx, 1));
	    connect(fsig->pin(idx), mux->pin_Data(idx, 0));
	    connect(osig->pin(idx), mux->pin_Result(idx));
      }
      des->add_node(mux);
      connect(csig->pin(0), mux->pin_Sel(0));
      des->add_signal(osig);
      return osig;
}

NetNet* NetESignal::synthesize(Design*des)
{
      NetNet*sig = new NetNet(name(), NetNet::WIRE, pin_count());
      for (unsigned idx = 0 ;  idx < pin_count() ;  idx += 1)
	    connect(sig->pin(idx), pin(idx));
      des->add_signal(sig);
      return sig;
}

/*
 * $Log: expr_synth.cc,v $
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

