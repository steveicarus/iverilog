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
#ident "$Id: stupid.cc,v 1.1 1998/11/03 23:29:05 steve Exp $"
#endif

# include  "netlist.h"
# include  <vector>

vector<NetObj::Link*>* list_link_nodes(NetObj::Link&link)
{
      NetObj*net;
      unsigned npin;
      vector<NetObj::Link*>*result = new vector<NetObj::Link*>;

      link.cur_link(net, npin);
      NetObj*cur = net;
      unsigned cpin = npin;
      do {
	    if (dynamic_cast<NetNode*>(cur))
		  result->push_back(&cur->pin(cpin));

	    cur->pin(cpin).next_link(cur, cpin);
      } while ((cur != net) || (cpin != npin));

      return result;
}

/*
 * This function scans a design and removes artifacts from the
 * elaboration step, and maybe a few other stupid inefficiencies.
 */

class Functor  : public Design::SigFunctor {

    public:
      virtual void sig_function(NetNet*);
};


void Functor::sig_function(NetNet*net)
{
      for (unsigned idx = 0 ;  idx < net->pin_count() ;  idx += 1) {
	    vector<NetObj::Link*>*nodes = list_link_nodes(net->pin(idx));
#if 0
	    cerr << "XXXX " << net->name() << "[" << idx << "] "
		  "nodes->size() == " << nodes->size() << endl;
#endif
	    delete nodes;
      }
}

void stupid(Design*des)
{
      Functor fun;
      des->scan_signals(&fun);
}

/*
 * $Log: stupid.cc,v $
 * Revision 1.1  1998/11/03 23:29:05  steve
 *  Introduce verilog to CVS.
 *
 */

