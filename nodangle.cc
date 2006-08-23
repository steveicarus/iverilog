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
#ifdef HAVE_CVS_IDENT
#ident "$Id: nodangle.cc,v 1.21.2.2 2006/08/23 04:08:55 steve Exp $"
#endif

# include "config.h"

/*
 * This functor scans the design looking for dangling objects and
 * excess local signals. These deletions are not necessarily required
 * for proper functioning of anything, but they can clean up the
 * appearance of design files that are generated.
 */
# include  "functor.h"
# include  "netlist.h"
# include  "compiler.h"

class nodangle_f  : public functor_t {
    public:
      void event(Design*des, NetEvent*ev);
      void signal(Design*des, NetNet*sig);

      unsigned count_;
      unsigned stotal, etotal;
};

void nodangle_f::event(Design*des, NetEvent*ev)
{
	/* If there are no references to this event, then go right
	   ahead and delete in. There is no use looking further at
	   it. */
      if ((ev->nwait() + ev->ntrig() + ev->nexpr()) == 0) {
	    delete ev;
	    etotal += 1;
	    return;
      }

	/* Try to remove duplicate probes from the event. */
      for (unsigned idx = 0 ;  idx < ev->nprobe() ;  idx += 1) {
	    unsigned jdx = idx + 1;
	    while (jdx < ev->nprobe()) {
		  NetEvProbe*ip = ev->probe(idx);
		  NetEvProbe*jp = ev->probe(jdx);

		  if (ip->edge() != jp->edge()) {
			jdx += 1;
			continue;
		  }

		  bool fully_connected = true;
		  for (unsigned jpin = 0; jpin < jp->pin_count(); jpin += 1) {
			unsigned ipin = 0;
			bool connected_flag = false;
			for (ipin = 0 ; ipin < ip->pin_count(); ipin += 1)
			      if (connected(ip->pin(ipin), jp->pin(jpin))) {
				    connected_flag = true;
				    break;
			      }

			if (!connected_flag) {
			      fully_connected = false;
			      break;
			}
		  }

		  if (fully_connected) {
			delete jp;
		  } else {
			jdx += 1;
		  }
	    }
      }

	/* Try to find all the events that are similar to me, and
	   replace their references with references to me. */
      list<NetEvent*> match;
      ev->find_similar_event(match);
      for (list<NetEvent*>::iterator idx = match.begin()
		 ; idx != match.end() ;  idx ++) {

	    NetEvent*tmp = *idx;
	    assert(tmp != ev);
	    tmp ->replace_event(ev);
      }

}

void nodangle_f::signal(Design*des, NetNet*sig)
{
	/* Cannot delete signals referenced in an expression
	   or an l-value. */
      if (sig->get_refs() > 0)
	    return;

      if (sig->mref())
	    return;

	/* Cannot delete the ports of tasks or functions. There are
	   too many places where they are referenced. */
      if ((sig->port_type() != NetNet::NOT_A_PORT)
	  && (sig->scope()->type() == NetScope::TASK))
	    return;

      if ((sig->port_type() != NetNet::NOT_A_PORT)
	  && (sig->scope()->type() == NetScope::FUNC))
	    return;

	/* Can't delete ports of cells. */
      if ((sig->port_type() != NetNet::NOT_A_PORT)
	  && (sig->scope()->attribute(perm_string::literal("ivl_synthesis_cell")) != verinum()))
	    return;

	/* Check to see if the signal is completely unconnected. If
	   all the bits are unlinked, then delete it. */
      bool linked_flag = false;
      for (unsigned idx =  0 ;  idx < sig->pin_count() ;  idx += 1)
	    if (sig->pin(idx).is_linked()) {
		  linked_flag = true;
		  break;
	    }

      if (! linked_flag) {
	    delete sig;
	    stotal += 1;
	    return;
      }

	/* The remaining things can only be done to synthesized
	   signals, not ones that appear in the original Verilog. */
      if (! sig->local_flag())
	    return;

	/* Check to see if there is some significant signal connected
	   to every pin of this signal. */
      unsigned significant_flags = 0;
      for (unsigned idx = 0 ;  idx < sig->pin_count() ;  idx += 1) {
	    Nexus*nex = sig->pin(idx).nexus();

	    for (Link*cur = nex->first_nlink()
		       ; cur ;  cur = cur->next_nlink()) {

		  if (cur == &sig->pin(idx))
			continue;

		  NetNet*cursig = dynamic_cast<NetNet*>(cur->get_obj());
		  if (cursig == 0)
			continue;

		  if (cursig == sig)
			continue;

		  if (cursig->local_flag())
			continue;

		  significant_flags += 1;
		  break;
	    }

	    if (significant_flags <= idx)
		  break;
      }

	/* If every pin is connected to another significant signal,
	   then I can delete this one. */
      if (significant_flags == sig->pin_count()) {
	    count_ += 1;
	    delete sig;
	    stotal += 1;
      }
}

void nodangle(Design*des)
{
      nodangle_f fun;
      unsigned count_iterations = 0;
      fun.stotal = 0;
      fun.etotal = 0;

      do {
	    fun.count_ = 0;
	    des->functor(&fun);
	    count_iterations += 1;

	    if (verbose_flag) {
		  cout << " ... " << count_iterations << " iterations"
		       << " deleted " << fun.stotal << " dangling signals"
		       << " and " << fun.etotal << " events."
		       << " (count=" << fun.count_ << ")" << endl;
	    }

      } while (fun.count_ > 0);

}

/*
 * $Log: nodangle.cc,v $
 * Revision 1.21.2.2  2006/08/23 04:08:55  steve
 *  Do not count self as signifincant in nodangle.
 *
 * Revision 1.21.2.1  2006/03/16 05:40:19  steve
 *  Fix crash when memory exploding doesnot work
 *
 * Revision 1.21  2004/02/20 18:53:35  steve
 *  Addtrbute keys are perm_strings.
 *
 * Revision 1.20  2004/01/15 06:04:19  steve
 *  Remove duplicate NetEvProbe objects in nodangle.
 *
 * Revision 1.19  2003/06/25 04:46:03  steve
 *  Do not elide ports of cells.
 *
 * Revision 1.18  2003/04/22 04:48:30  steve
 *  Support event names as expressions elements.
 *
 * Revision 1.17  2002/08/12 01:35:00  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.16  2002/07/24 16:24:45  steve
 *  Rewrite find_similar_event to support doing
 *  all event matching and replacement in one
 *  shot, saving time in the scans.
 *
 * Revision 1.15  2002/05/26 01:39:02  steve
 *  Carry Verilog 2001 attributes with processes,
 *  all the way through to the ivl_target API.
 *
 *  Divide signal reference counts between rval
 *  and lval references.
 *
 * Revision 1.14  2002/02/02 06:13:38  steve
 *  event find_similar should not find self.
 *
 * Revision 1.13  2001/07/27 02:41:55  steve
 *  Fix binding of dangling function ports. do not elide them.
 *
 * Revision 1.12  2001/07/25 03:10:49  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.11  2001/02/17 05:14:35  steve
 *  Cannot elide task ports.
 *
 * Revision 1.10  2000/11/19 20:48:53  steve
 *  Killing some signals might make others killable.
 *
 * Revision 1.9  2000/11/18 05:12:45  steve
 *  Delete unreferenced signals no matter what.
 *
 * Revision 1.8  2000/06/25 19:59:42  steve
 *  Redesign Links to include the Nexus class that
 *  carries properties of the connected set of links.
 *
 * Revision 1.7  2000/05/31 02:26:49  steve
 *  Globally merge redundant event objects.
 *
 * Revision 1.6  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.5  2000/04/28 21:00:29  steve
 *  Over agressive signal elimination in constant probadation.
 *
 * Revision 1.4  2000/04/18 04:50:20  steve
 *  Clean up unneeded NetEvent objects.
 *
 * Revision 1.3  2000/02/23 02:56:55  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.2  1999/11/28 23:42:02  steve
 *  NetESignal object no longer need to be NetNode
 *  objects. Let them keep a pointer to NetNet objects.
 *
 * Revision 1.1  1999/11/18 03:52:20  steve
 *  Turn NetTmp objects into normal local NetNet objects,
 *  and add the nodangle functor to clean up the local
 *  symbols generated by elaboration and other steps.
 *
 */

