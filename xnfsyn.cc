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
#ident "$Id: xnfsyn.cc,v 1.2 1999/07/18 21:17:51 steve Exp $"
#endif

/*
 * The xnfsyn function searches the behavioral description for
 * patterns that are known to represent XNF library components. This
 * is especially interesting for the sequential components such as
 * flip flops and latches. As threads are transformed into components,
 * the design is rewritten.
 *
 * Currently, this transform recognizes the following patterns:
 *
 *    always @(posedge CLK) Q = D     // DFF:D,Q,C
 *    always @(negedge CLK) Q = D     // DFF:D,Q,~C
 *
 *    always @(posedge CLK) if (CE) Q = D;
 *    always @(negedge CLK) if (CE) Q = D;
 *
 * The r-value of the assignments must be identifiers (i.e. wires or
 * registers) and the CE must be single-bine identifiers. Enough
 * devices will be created to accommodate the width of Q and D, though
 * the CLK and CE will be shared by all the devices.
 */
# include  "functor.h"
# include  "netlist.h"

class xnfsyn_f  : public functor_t {

    public:
      void process(class Design*, class NetProcTop*);

    private:
      void proc_always_(class Design*);
      void proc_casn_(class Design*);
      void proc_ccon_(class Design*);

	// The matcher does something like a recursive descent search
	// for the templates. These variables are filled in as the
	// searcher finds them.

      class NetProcTop*top_;

      class NetPEvent *pclk_;
      class NetNEvent *nclk_;

      class NetCondit *con_;
      class NetAssign *asn_;
};


/*
 * Look at a process, and divide the problem into always and initial
 * threads.
 */
void xnfsyn_f::process(class Design*des, class NetProcTop*top)
{
      switch (top->type()) {
	  case NetProcTop::KALWAYS:
	    top_ = top;
	    proc_always_(des);
	    break;
      }
}

/*
 * An "always ..." statement has been found.
 */
void xnfsyn_f::proc_always_(class Design*des)
{
	// The statement must be a NetPEvent, ...
      pclk_ = dynamic_cast<class NetPEvent*>(top_->statement());
      if (pclk_ == 0)
	    return;

	// ... there must be a single event source, ...
      svector<class NetNEvent*>*neb = pclk_->back_list();
      if (neb == 0)
	    return;
      if (neb->count() != 1) {
	    delete neb;
	    return;
      }
      nclk_ = (*neb)[0];
      delete neb;

	// ... the event must be an edge, ...
      switch (nclk_->type()) {
	  case NetNEvent::POSEDGE:
	  case NetNEvent::NEGEDGE:
	    break;
	  default:
	    return;
      }

	// Is this a clocked assignment?
      asn_ = dynamic_cast<NetAssign*>(pclk_->statement());
      if (asn_) {
	    proc_casn_(des);
	    return;
      }

      con_ = dynamic_cast<NetCondit*>(pclk_->statement());
      if (con_) {
	    proc_ccon_(des);
	    return;
      }
}

/*
 * The process so far has been matched as:
 *
 *   always @(posedge nclk_) asn_ = <r>;
 *   always @(negedge nclk_) asn_ = <r>;
 */
void xnfsyn_f::proc_casn_(class Design*des)
{

	// ... and the rval must be a simple signal.
      NetESignal*sig = dynamic_cast<NetESignal*>(asn_->rval());
      if (sig == 0)
	    return ;

	// The signal and the assignment must be the same width...
      assert(asn_->pin_count() == sig->pin_count());

	// Geneate enough DFF objects to handle the entire width.
      for (unsigned idx = 0 ;  idx < asn_->pin_count() ;  idx += 1) {

	      // XXXX FIXME: Objects need unique names!
	    NetUDP*dff = new NetUDP(asn_->name(), 3, true);

	    connect(dff->pin(0), asn_->pin(idx));
	    connect(dff->pin(1), sig->pin(idx));
	    connect(dff->pin(2), nclk_->pin(0));

	    switch (nclk_->type()) {
		case NetNEvent::POSEDGE:
		  dff->attribute("XNF-LCA", "DFF:Q,D,C");
		  break;
		case NetNEvent::NEGEDGE:
		  dff->attribute("XNF-LCA", "DFF:Q,D,~C");
		  break;
	    }

	    des->add_node(dff);
      }

	// This process is matched and replaced with a DFF. Get
	// rid of the now useless NetProcTop.
      des->delete_process(top_);
}

/*
 * The process si far has been matches as:
 *
 *    always @(posedge nclk_) if ...;
 *    always @(negedge nclk_) if ...;
 */
void xnfsyn_f::proc_ccon_(class Design*des)
{
      if (con_->else_clause())
	    return;

      asn_ = dynamic_cast<NetAssign*>(con_->if_clause());
      if (asn_ == 0)
	    return;

      NetESignal*sig = dynamic_cast<NetESignal*>(asn_->rval());
      if (sig == 0)
	    return;

	// The signal and the assignment must be the same width...
      assert(asn_->pin_count() == sig->pin_count());

      NetESignal*ce = dynamic_cast<NetESignal*>(con_->expr());
      if (ce == 0)
	    return;
      if (ce->pin_count() != 1)
	    return;

	// Geneate enough DFF objects to handle the entire width.
      for (unsigned idx = 0 ;  idx < asn_->pin_count() ;  idx += 1) {

	      // XXXX FIXME: Objects need unique names!
	    NetUDP*dff = new NetUDP(asn_->name(), 4, true);

	    connect(dff->pin(0), asn_->pin(idx));
	    connect(dff->pin(1), sig->pin(idx));
	    connect(dff->pin(2), nclk_->pin(0));
	    connect(dff->pin(3), ce->pin(0));

	    switch (nclk_->type()) {
		case NetNEvent::POSEDGE:
		  dff->attribute("XNF-LCA", "DFF:Q,D,C,CE");
		  break;
		case NetNEvent::NEGEDGE:
		  dff->attribute("XNF-LCA", "DFF:Q,D,~C,CE");
		  break;
	    }

	    des->add_node(dff);
      }

	// This process is matched and replaced with a DFF. Get
	// rid of the now useless NetProcTop.
      des->delete_process(top_);
}

void xnfsyn(Design*des)
{
      xnfsyn_f xnfsyn_obj;
      des->functor(&xnfsyn_obj);
}

/*
 * $Log: xnfsyn.cc,v $
 * Revision 1.2  1999/07/18 21:17:51  steve
 *  Add support for CE input to XNF DFF, and do
 *  complete cleanup of replaced design nodes.
 *
 * Revision 1.1  1999/07/18 05:52:47  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 */

