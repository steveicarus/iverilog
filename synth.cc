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
#ident "$Id: synth.cc,v 1.9 2000/04/16 22:57:34 steve Exp $"
#endif

/*
 * The synth function searches the behavioral description for
 * patterns that are known to represent LPM library components. This
 * is especially interesting for the sequential components such as
 * flip flops and latches. As threads are transformed into components,
 * the design is rewritten.
 */
# include  "functor.h"
# include  "netlist.h"

/*
 * This functor scans the behavioral code, looking for expressions to
 * synthesize. Although it uses the proc_match_t class, it doesn't
 * actually match anything, but transforms expressions into structural
 * netlists. The product of this should be a process where all the
 * expressions have been reduced to a signal ident, which references
 * the NetNet of the now synthesized expression.
 */
class do_expr  : public proc_match_t {

    public:
      do_expr(Design*d)
      : des_(d) { }

    private:

      Design*des_;

      virtual int assign(NetAssign*);
      virtual int event_wait(NetEvWait*);
	//virtual int assign_mem(NetAssignMem*);
      virtual int condit(NetCondit*);
};


int do_expr::assign(NetAssign*stmt)
{
      if (dynamic_cast<NetESignal*>(stmt->rval()))
	    return 0;

      NetNet*tmp = stmt->rval()->synthesize(des_);
      if (tmp == 0)
	    return 0;

      NetESignal*tmpe = new NetESignal(tmp);
      stmt->set_rval(tmpe);

      return 0;
}

int do_expr::condit(NetCondit*stmt)
{
	/* synthesize the condition expression, if necessary. */
      if (! dynamic_cast<NetESignal*>(stmt->expr())) {
	    NetNet*tmp = stmt->expr()->synthesize(des_);

	    if (tmp) {
		  NetESignal*tmpe = new NetESignal(tmp);
		  stmt->set_expr(tmpe);
	    }

      }

	/* Now recurse through the if and else clauses. */
      if (NetProc*tmp = stmt->if_clause())
	    tmp->match_proc(this);

      if (NetProc*tmp = stmt->else_clause())
	    tmp->match_proc(this);

      return 0;
}

int do_expr::event_wait(NetEvWait*stmt)
{
      NetProc*tmp = stmt->statement();
      if (tmp)
	    return tmp->match_proc(this);
      else
	    return 0;
}

/*
 * This transform recognizes the following patterns:
 *
 *    always @(posedge CLK) Q = D
 *    always @(negedge CLK) Q = D
 *
 *    always @(posedge CLK) if (CE) Q = D;
 *    always @(negedge CLK) if (CE) Q = D;
 *
 * These are memory assignments:
 *
 *    always @(posedge CLK) M[a] = D
 *    always @(negedge CLK) M[a] = D
 *
 *    always @(posedge CLK) if (CE) M[a] = D;
 *    always @(negedge CLK) if (CE) M[a] = D;
 *
 * The r-value of the assignments must be identifiers (i.e. wires or
 * registers) and the CE must be single-bit identifiers. The generated
 * device will be wide enough to accomodate Q and D.
 */
class match_dff : public proc_match_t {

    public:
      match_dff(Design*d, NetProcTop*t)
      : des_(d), top_(t), wclk_(0), eclk_(0), pclk_(0), con_(0), ce_(0),
	asn_(0), asm_(0), d_(0)
      { }

      ~match_dff() { }

      void make_it();

    private:
      void make_dff_();
      void make_ram_();

      virtual int assign(NetAssign*);
      virtual int assign_mem(NetAssignMem*);
      virtual int condit(NetCondit*);
      virtual int event_wait(NetEvWait*);

      Design*des_;
      NetProcTop*top_;

      NetEvWait*wclk_;
      NetEvent *eclk_;
      NetEvProbe*pclk_;

      NetCondit *con_;
      NetNet*ce_;

      NetAssign *asn_;
      NetAssignMem*asm_;

      NetNet*d_;
};

int match_dff::assign(NetAssign*as)
{
      if (!pclk_)
	    return 0;
      if (asn_ || asm_)
	    return 0;

      asn_ = as;
      d_ = asn_->rval()->synthesize(des_);
      if (d_ == 0)
	    return 0;

      return 1;
}

int match_dff::assign_mem(NetAssignMem*as)
{
      if (!pclk_)
	    return 0;
      if (asn_ || asm_)
	    return 0;

      asm_ = as;
      d_ = asm_->rval()->synthesize(des_);
      if (d_ == 0)
	    return 0;

      return 1;
}

int match_dff::condit(NetCondit*co)
{
      if (!pclk_)
	    return 0;
      if (con_ || asn_ || asm_)
	    return 0;

      con_ = co;
      if (con_->else_clause())
	    return 0;
      ce_ = con_->expr()->synthesize(des_);
      if (ce_ == 0)
	    return 0;

      return con_->if_clause()->match_proc(this);
}

int match_dff::event_wait(NetEvWait*evw)
{
      if (evw->nevents() != 1)
	    return 0;

      wclk_ = evw;
      eclk_ = evw->event(0);

      if (eclk_->nprobe() != 1)
	    return 0;

      pclk_ = eclk_->probe(0);

      return wclk_->statement()->match_proc(this);
}

void match_dff::make_it()
{
      if (asn_) {
	    assert(asm_ == 0);
	    make_dff_();
      } else {
	    assert(asm_);
	    make_ram_();
      }
}

void match_dff::make_dff_()
{
      NetFF*ff = new NetFF(asn_->name(), asn_->pin_count());

      for (unsigned idx = 0 ;  idx < ff->width() ;  idx += 1) {
	    connect(ff->pin_Data(idx), d_->pin(idx));
	    connect(ff->pin_Q(idx), asn_->pin(idx));
      }

      connect(ff->pin_Clock(), pclk_->pin(0));
      if (ce_) connect(ff->pin_Enable(), ce_->pin(0));

      ff->attribute("LPM_FFType", "DFF");
      if (pclk_->edge() == NetEvProbe::NEGEDGE)
	    ff->attribute("Clock:LPM_Polarity", "INVERT");

      des_->add_node(ff);
}

void match_dff::make_ram_()
{
      NetMemory*mem = asm_->memory();
      NetNet*adr = asm_->index();
      NetRamDq*ram = new NetRamDq(des_->local_symbol(mem->name()), mem,
				  adr->pin_count());
      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(adr->pin(idx), ram->pin_Address(idx));

      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1)
	    connect(ram->pin_Data(idx), d_->pin(idx));

      if (ce_)
	    connect(ram->pin_WE(), ce_->pin(0));

      assert(pclk_->edge() == NetEvProbe::POSEDGE);
      connect(ram->pin_InClock(), pclk_->pin(0));

      ram->absorb_partners();
      des_->add_node(ram);
}

class match_block : public proc_match_t {

    public:
      match_block(Design*d, NetProcTop*t)
      : des_(d), top_(t)
      { }

      ~match_block() { }

    private:

      Design*des_;
      NetProcTop*top_;
};

class synth_f  : public functor_t {

    public:
      void process(class Design*, class NetProcTop*);

    private:
      void proc_always_(class Design*);

      NetProcTop*top_;
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
      do_expr expr_pat(des);
      top_->statement()->match_proc(&expr_pat);

      match_dff dff_pat(des, top_);
      if (top_->statement()->match_proc(&dff_pat)) {
	    dff_pat.make_it();
	    des->delete_process(top_);
	    return;
      }

      match_block block_pat(des, top_);
      if (top_->statement()->match_proc(&block_pat)) {
	    cerr << "XXXX: recurse and return here" << endl;
      }
}


void synth(Design*des)
{
      synth_f synth_obj;
      des->functor(&synth_obj);
}

/*
 * $Log: synth.cc,v $
 * Revision 1.9  2000/04/16 22:57:34  steve
 *  Catch expressions that are part of conditionals.
 *
 * Revision 1.8  2000/04/12 20:02:53  steve
 *  Finally remove the NetNEvent and NetPEvent classes,
 *  Get synthesis working with the NetEvWait class,
 *  and get started supporting multiple events in a
 *  wait in vvm.
 *
 * Revision 1.7  2000/04/02 04:26:07  steve
 *  Remove the useless sref template.
 *
 * Revision 1.6  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.5  2000/02/13 04:35:43  steve
 *  Include some block matching from Larry.
 *
 * Revision 1.4  1999/12/05 02:24:09  steve
 *  Synthesize LPM_RAM_DQ for writes into memories.
 *
 * Revision 1.3  1999/12/01 06:06:16  steve
 *  Redo synth to use match_proc_t scanner.
 *
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

