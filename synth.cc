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
#ident "$Id: synth.cc,v 1.2 1999/11/02 04:55:34 steve Exp $"
#endif

/*
 * The synth function searches the behavioral description for
 * patterns that are known to represent LPM library components. This
 * is especially interesting for the sequential components such as
 * flip flops and latches. As threads are transformed into components,
 * the design is rewritten.
 *
 * Currently, this transform recognizes the following patterns:
 *
 *    always @(posedge CLK) Q = D
 *    always @(negedge CLK) Q = D
 *
 *    always @(posedge CLK) if (CE) Q = D;
 *    always @(negedge CLK) if (CE) Q = D;
 *
 * The r-value of the assignments must be identifiers (i.e. wires or
 * registers) and the CE must be single-bit identifiers. The generated
 * device will be wide enough to accomodate Q and D.
 */
# include  "functor.h"
# include  "netlist.h"

class synth_f  : public functor_t {

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
void synth_f::process(class Design*des, class NetProcTop*top)
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
void synth_f::proc_always_(class Design*des)
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
void synth_f::proc_casn_(class Design*des)
{
	// Turn the r-value into gates.
      NetNet*sig = asn_->rval()->synthesize(des);
      assert(sig);

	// The signal and the assignment must be the same width...
      assert(asn_->pin_count() == sig->pin_count());

      NetFF*ff = new NetFF(asn_->name(), asn_->pin_count());
      ff->attribute("LPM_FFType", "DFF");

      for (unsigned idx = 0 ;  idx < ff->width() ;  idx += 1) {
	    connect(ff->pin_Data(idx), sig->pin(idx));
	    connect(ff->pin_Q(idx), asn_->pin(idx));
      }

      switch (nclk_->type()) {
	  case NetNEvent::POSEDGE:
	    connect(ff->pin_Clock(), nclk_->pin(0));
	    break;

	  case NetNEvent::NEGEDGE:
	    connect(ff->pin_Clock(), nclk_->pin(0));
	    ff->attribute("Clock:LPM_Polarity", "INVERT");
	    break;
      }

      des->add_node(ff);

	// This process is matched and replaced with a DFF. Get
	// rid of the now useless NetProcTop.
      des->delete_process(top_);
}

/*
 * The process so far has been matched as:
 *
 *    always @(posedge nclk_) if ...;
 *    always @(negedge nclk_) if ...;
 */
void synth_f::proc_ccon_(class Design*des)
{
      if (con_->else_clause())
	    return;

      asn_ = dynamic_cast<NetAssign*>(con_->if_clause());
      if (asn_ == 0)
	    return;

      NetNet*sig = asn_->rval()->synthesize(des);
      assert(sig);

	// The signal and the assignment must be the same width...
      assert(asn_->pin_count() == sig->pin_count());

      NetESignal*ce = dynamic_cast<NetESignal*>(con_->expr());
      if (ce == 0)
	    return;
      if (ce->pin_count() != 1)
	    return;

      NetFF*ff = new NetFF(asn_->name(), asn_->pin_count());
      ff->attribute("LPM_FFType", "DFF");

      for (unsigned idx = 0 ;  idx < ff->width() ;  idx += 1) {
	    connect(ff->pin_Data(idx), sig->pin(idx));
	    connect(ff->pin_Q(idx), asn_->pin(idx));
      }

      switch (nclk_->type()) {
	  case NetNEvent::POSEDGE:
	    connect(ff->pin_Clock(), nclk_->pin(0));
	    connect(ff->pin_Enable(), ce->pin(0));
	    break;

	  case NetNEvent::NEGEDGE:
	    connect(ff->pin_Clock(), nclk_->pin(0));
	    connect(ff->pin_Enable(), ce->pin(0));
	    ff->attribute("Clock:LPM_Polarity", "INVERT");
	    break;
      }

      des->add_node(ff);


	// This process is matched and replaced with a DFF. Get
	// rid of the now useless NetProcTop.
      des->delete_process(top_);
}

void synth(Design*des)
{
      synth_f synth_obj;
      des->functor(&synth_obj);
}

/*
 * $Log: synth.cc,v $
 * Revision 1.2  1999/11/02 04:55:34  steve
 *  Add the synthesize method to NetExpr to handle
 *  synthesis of expressions, and use that method
 *  to improve r-value handling of LPM_FF synthesis.
 *
 *  Modify the XNF target to handle LPM_FF objects.
 *
 * Revision 1.1  1999/11/01 02:07:41  steve
 *  Add the synth functor to do generic synthesis
 *  and add the LPM_FF device to handle rows of
 *  flip-flops.
 *
 */

