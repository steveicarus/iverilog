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
#ident "$Id: xnfsyn.cc,v 1.1 1999/07/18 05:52:47 steve Exp $"
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
 *    always @(posedge CLK) Q = D     // DFF
 */
# include  "functor.h"
# include  "netlist.h"

class xnfsyn_f  : public functor_t {

    public:
      void process(class Design*, class NetProcTop*);

    private:
      void proc_always(class Design*, class NetProcTop*);
};


/*
 * Look at a process, and divide the problem into always and initial
 * threads.
 */
void xnfsyn_f::process(class Design*des, class NetProcTop*top)
{
      switch (top->type()) {
	  case NetProcTop::KALWAYS:
	    proc_always(des, top);
	    break;
      }

}

/*
 * Look for DFF devices. The pattern I'm looking for is:
 *
 *     always @(posedge CLK) Q = D;
 *
 * This creates a DFF with the CLK, D and Q hooked up the obvious
 * way. CE and PRE/CLR are left unconnected.
 */
void xnfsyn_f::proc_always(class Design*des, class NetProcTop*top)
{
	// The statement must be a NetPEvent, ...
      const NetProc*st = top->statement();
      const NetPEvent*ev = dynamic_cast<const NetPEvent*>(st);
      if (ev == 0)
	    return;

	// ... there must be a single event source, ...
      svector<const NetNEvent*>*neb = ev->back_list();
      if (neb == 0)
	    return;
      if (neb->count() != 1) {
	    delete neb;
	    return;
      }
      const NetNEvent*nev = (*neb)[0];
      delete neb;

	// ... the event must be POSEDGE, ...
      if (nev->type() != NetNEvent::POSEDGE)
	    return;

	// the NetPEvent must guard an assignment, ...
      const NetAssign*asn = dynamic_cast<const NetAssign*>(ev->statement());
      if (asn == 0)
	    return;

	// ... and the rval must be a simple signal.
      const NetESignal*sig = dynamic_cast<const NetESignal*>(asn->rval());
      if (sig == 0)
	    return ;

	// XXXX For now, only handle single bit.
      assert(asn->pin_count() == 1);
      assert(sig->pin_count() == 1);

      NetUDP*dff = new NetUDP(asn->name(), 3, true);
      connect(dff->pin(0), asn->pin(0));
      connect(dff->pin(1), sig->pin(0));
      connect(dff->pin(2), nev->pin(0));

      dff->attribute("XNF-LCA", "DFF:Q,D,CLK");
      des->add_node(dff);

	// This process is matched and replaced with a DFF. Get
	// rid of the now useless NetProcTop.
      des->delete_process(top);
}

void xnfsyn(Design*des)
{
      xnfsyn_f xnfsyn_obj;
      des->functor(&xnfsyn_obj);
}

/*
 * $Log: xnfsyn.cc,v $
 * Revision 1.1  1999/07/18 05:52:47  steve
 *  xnfsyn generates DFF objects for XNF output, and
 *  properly rewrites the Design netlist in the process.
 *
 */

