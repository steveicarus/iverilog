/*
 * Copyright (c) 1998 Stephen Williams (steve@icarus.com)
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
#ident "$Id: sigfold.cc,v 1.3 1999/11/06 04:51:11 steve Exp $"
#endif

# include  "netlist.h"

/*
 * Some targets require that a list of connections include exactly one
 * signal. This processing step deletes excess signals to meet that
 * requirement.
 *
 * If there is a ring that has more then one signal connected
 * together, the following heuristic is used to choose which signal to
 * keep:
 *
 *	If the name of signal X is deeper in the design hierarchy then
 *	signal Y, keep signal Y.
 *
 *	Else, if X is a bus that is not as wide as Y, keep signal Y.
 *
 * The result of this should be that signals of enclosing modules will
 * be preferred over signals of instantiated modules, and bussed
 * signals (vectors) will be preferred over scaler signals.
 *
 * The presence of attributes modifies the folding process, by placing
 * restrictions on what is allowed.
 *
 *      If two signals have an attribute key X, they can be folded
 *      only if X has the same value for both signals.
 *
 *      If signal A has attribute X and signal B does not, then
 *      signals can be folded if B can take the attribute.
 *
 *      All the pins in a vector have identical attributes.
 */

static unsigned depth(const string&sym)
{
      unsigned cnt = 0;
      for (unsigned idx = 0 ;  idx < sym.length() ;  idx += 1)
	    if (sym[idx] == '.')
		  cnt += 1;

      return cnt;
}

static bool is_any_signal(const NetNet*)
{
      return true;
}

static void clear_extra_signals(NetNet*net, unsigned npin)
{
      const unsigned mydepth = depth(net->name());
      NetObj*cur = net;
      unsigned pin = npin;
      for (net->pin(pin).next_link(cur, pin) ; cur != net ; ) {

	      // sig is the node I am going to try to subsume.
	    NetNet*sig = dynamic_cast<NetNet*>(cur);

	      // Skip the node if it isn't even a signal.
	    if (sig == 0) {
		  cur->pin(pin).next_link(cur, pin);
		  continue;
	    }

	    if (NetTmp*tmp = dynamic_cast<NetTmp*>(sig)) {
		  cerr << "internal warning: sigfold found NetTmp--"
			"deleting it." << endl;

		    // save the next link, ...
		  NetObj*nxt;
		  unsigned pnxt;
		  cur->pin(pin).next_link(nxt, pnxt);

		    // disconnect the target signal, ...
		  sig->pin(pin).unlink();

		    // And onward.
		  cur = nxt;
		  pin = pnxt;
		  continue;
	    }

	      // Skip the node if it has incompatible attributes.
	    if (! net->has_compat_attributes(*sig)) {

		    // SPECIAL CASE!
		  if (sig->has_compat_attributes(*net)) {
			net->pin(npin).unlink();
			return;
		  }

		  cur->pin(pin).next_link(cur, pin);
		  continue;
	    }

	      // Skip the node if it is higher up.
	    if (depth(sig->name()) < mydepth) {
		  cur->pin(pin).next_link(cur, pin);
		  continue;
	    }

	      // Skip the node if it is a larger vector.
	    if (sig->pin_count() > net->pin_count()) {
		  cur->pin(pin).next_link(cur, pin);
		  continue;
	    }

	      // save the next link, ...
	    NetObj*nxt;
	    unsigned pnxt;
	    cur->pin(pin).next_link(nxt, pnxt);

	      // disconnect the target signal, ...
	    sig->pin(pin).unlink();

	      // And onward.
	    cur = nxt;
	    pin = pnxt;
      }
}

static void delete_if_unconnected(NetNet*net)
{
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1)
	    if (net->pin(idx).is_linked())
		  return;

      delete net;
}

void sigfold(Design*des)
{
      des->clear_signal_marks();
      while (NetNet*obj = des->find_signal(&is_any_signal)) {
	    for (unsigned idx = 0 ;  idx < obj->pin_count() ;  idx += 1)
		  clear_extra_signals(obj, idx);
	    obj->set_mark();
      }

	// After the disconnect step, I may have signals that are not
	// connected to anything. Delete those.

      des->clear_signal_marks();
      while (NetNet*obj = des->find_signal(&is_any_signal)) {
	    obj->set_mark();
	    delete_if_unconnected(obj);
      }
}

/*
 * $Log: sigfold.cc,v $
 * Revision 1.3  1999/11/06 04:51:11  steve
 *  Catch NetTmp objects.
 *
 * Revision 1.2  1998/12/02 04:37:13  steve
 *  Add the nobufz function to eliminate bufz objects,
 *  Object links are marked with direction,
 *  constant propagation is more careful with wide links,
 *  Signal folding is aware of attributes, and
 *  the XNF target can dump UDP objects based on LCA
 *  attributes.
 *
 * Revision 1.1  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 */

