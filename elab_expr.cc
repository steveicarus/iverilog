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
#ident "$Id: elab_expr.cc,v 1.1 1999/09/20 02:21:10 steve Exp $"
#endif


# include  "pform.h"
# include  "netlist.h"


NetExpr* PEIdent::elaborate_expr(Design*des, const string&path) const
{
	// System identifiers show up in the netlist as identifiers.
      if (text_[0] == '$')
	    return new NetEIdent(text_, 64);

      string name = path+"."+text_;

	// If the identifier name is a parameter name, then return
	// a reference to the parameter expression.
      if (const NetExpr*ex = des->find_parameter(path, text_)) {
	    NetExpr*tmp;
	    if (dynamic_cast<const NetExpr*>(ex))
		  tmp = ex->dup_expr();
	    else
		  tmp = new NetEParam(path, text_);

	    tmp->set_line(*this);
	    return tmp;
      }

	// If the identifier names a signal (a register or wire)
	// then create a NetESignal node to handle it.
      if (NetNet*net = des->find_signal(path, text_)) {

	      // If this is a part select of a signal, then make a new
	      // temporary signal that is connected to just the
	      // selected bits.
	    if (lsb_) {
		  assert(msb_);
		  verinum*lsn = lsb_->eval_const(des, path);
		  verinum*msn = msb_->eval_const(des, path);
		  if ((lsn == 0) || (msn == 0)) {
			cerr << get_line() << ": Part select expresions "
			      " must be constant expressions." << endl;
			des->errors += 1;
			return 0;
		  }

		  assert(lsn);
		  assert(msn);
		  unsigned long lsv = lsn->as_ulong();
		  unsigned long msv = msn->as_ulong();
		  unsigned long wid = 1 + ((msv>lsv)? (msv-lsv) : (lsv-msv));
		  assert(wid <= net->pin_count());
		  assert(net->sb_to_idx(msv) >= net->sb_to_idx(lsv));

		  string tname = des->local_symbol(path);
		  NetESignal*tmp = new NetESignal(tname, wid);
		  tmp->set_line(*this);

		    // Connect the pins from the lsb up to the msb.
		  unsigned off = net->sb_to_idx(lsv);
		  for (unsigned idx = 0 ;  idx < wid ;  idx += 1)
			connect(tmp->pin(idx), net->pin(idx+off));

		  des->add_node(tmp);
		  return tmp;
	    }

	      // If the bit select is constant, then treat it similar
	      // to the part select, so that I save the effort of
	      // making a mux part in the netlist.
	    verinum*msn;
	    if (msb_ && (msn = msb_->eval_const(des, path))) {
		  assert(idx_ == 0);
		  unsigned long msv = msn->as_ulong();

		  string tname = des->local_symbol(path);
		  NetESignal*tmp = new NetESignal(tname, 1);
		  tmp->set_line(*this);
		  connect(tmp->pin(0), net->pin(msv));

		  des->add_node(tmp);
		  return tmp;
	    }

	    NetESignal*node = new NetESignal(net);
	    des->add_node(node);
	    assert(idx_ == 0);

	      // Non-constant bit select? punt and make a subsignal
	      // device to mux the bit in the net.
	    if (msb_) {
		  NetExpr*ex = msb_->elaborate_expr(des, path);
		  NetESubSignal*ss = new NetESubSignal(node, ex);
		  ss->set_line(*this);
		  return ss;
	    }

	      // All else fails, return the signal itself as the
	      // expression.
	    assert(msb_ == 0);
	    return node;
      }

	// If the identifier names a memory, then this is a
	// memory reference and I must generate a NetEMemory
	// object to handle it.
      if (NetMemory*mem = des->find_memory(name)) {
	    assert(msb_ != 0);
	    assert(lsb_ == 0);
	    assert(idx_ == 0);
	    NetExpr*i = msb_->elaborate_expr(des, path);
	    if (i == 0) {
		  cerr << get_line() << ": Unable to exaborate "
			"index expression `" << *msb_ << "'" << endl;
		  des->errors += 1;
		  return 0;
	    }

	    NetEMemory*node = new NetEMemory(mem, i);
	    node->set_line(*this);
	    return node;
      }

	// I cannot interpret this identifier. Error message.
      cerr << get_line() << ": Unable to bind wire/reg/memory "
	    "`" << path << "." << text_ << "'" << endl;
      des->errors += 1;
      return 0;
}

/*
 * $Log: elab_expr.cc,v $
 * Revision 1.1  1999/09/20 02:21:10  steve
 *  Elaborate parameters in phases.
 *
 */

