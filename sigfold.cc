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
#ident "$Id: sigfold.cc,v 1.1 1998/11/16 05:03:53 steve Exp $"
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

	    NetNet*sig = dynamic_cast<NetNet*>(cur);
	    if ((sig == 0)
		|| (depth(sig->name()) < mydepth)
		|| (sig->pin_count() > net->pin_count())) {
		  cur->pin(pin).next_link(cur, pin);
		  continue;
	    }

	    NetObj*nxt;
	    unsigned pnxt;
	    cur->pin(pin).next_link(nxt, pnxt);
	    sig->pin(pin).unlink();
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
 * Revision 1.1  1998/11/16 05:03:53  steve
 *  Add the sigfold function that unlinks excess
 *  signal nodes, and add the XNF target.
 *
 */

