/*
 * Copyright (c) 2000-2003 Stephen Williams (steve@icarus.com)
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
#ident "$Id: elab_anet.cc,v 1.10 2004/10/04 01:10:52 steve Exp $"
#endif

# include "config.h"

/*
 * The elaborate_anet methods elaborate expressions that are intended
 * to be the left side of procedural continuous assignments.
 */

# include  "PExpr.h"
# include  "netlist.h"
# include  "netmisc.h"
# include  <iostream>

NetNet* PExpr::elaborate_anet(Design*des, NetScope*scope) const
{
      cerr << get_line() << ": error: Invalid expression on left side "
	   << "of procedural continuous assignment." << endl;
      return 0;
}


NetNet* PEConcat::elaborate_anet(Design*des, NetScope*scope) const
{
      if (repeat_) {
	    cerr << get_line() << ": error: Repeat concatenations make "
		  "no sense in l-value expressions. I refuse." << endl;
	    des->errors += 1;
	    return 0;
      }

      svector<NetNet*>nets (parms_.count());
      unsigned pins = 0;
      unsigned errors = 0;

      for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {

	    if (parms_[idx] == 0) {
		  cerr << get_line() << ": error: Empty expressions "
		       << "not allowed in concatenations." << endl;
		  errors += 1;
		  continue;
	    }

	    nets[idx] = parms_[idx]->elaborate_anet(des, scope);
	    if (nets[idx] == 0)
		  errors += 1;
	    else
		  pins += nets[idx]->pin_count();
      }

	/* If any of the sub expressions failed to elaborate, then
	   delete all those that did and abort myself. */
      if (errors) {
	    for (unsigned idx = 0 ;  idx < nets.count() ;  idx += 1) {
		  if (nets[idx]) delete nets[idx];
	    }
	    des->errors += 1;
	    return 0;
      }

	/* Make the temporary signal that connects to all the
	   operands, and connect it up. Scan the operands of the
	   concat operator from least significant to most significant,
	   which is opposite from how they are given in the list.

	   Allow for a repeat count other then 1 by repeating the
	   connect loop as many times as necessary. */

      NetNet*osig = new NetNet(scope, scope->local_symbol(),
			       NetNet::IMPLICIT_REG, pins);

      pins = 0;
      for (unsigned idx = nets.count() ;  idx > 0 ;  idx -= 1) {
	    NetNet*cur = nets[idx-1];
	    for (unsigned pin = 0;  pin < cur->pin_count();  pin += 1) {
		  connect(osig->pin(pins), cur->pin(pin));
		  pins += 1;
	    }
      }

      osig->local_flag(true);
      return osig;
}

NetNet* PEIdent::elaborate_anet(Design*des, NetScope*scope) const
{
      assert(scope);

      NetNet*       sig = 0;
      NetMemory*    mem = 0;
      NetVariable*  var = 0;
      const NetExpr*par = 0;
      NetEvent*     eve = 0;

      symbol_search(des, scope, path_, sig, mem, var, par, eve);


      if (mem != 0) {
	    cerr << get_line() << ": error: memories not allowed "
		 << "on left side of procedural continuous "
		 << "assignment." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (eve != 0) {
	    cerr << get_line() << ": error: named events not allowed "
		 << "on left side of procedural continuous "
		 << "assignment." << endl;
	    des->errors += 1;
	    return 0;
      }

      if (sig == 0) {
	    cerr << get_line() << ": error: reg ``" << path_ << "'' "
		 << "is undefined in this scope." << endl;
	    des->errors += 1;
	    return 0;
      }

      switch (sig->type()) {
	  case NetNet::REG:
	  case NetNet::IMPLICIT_REG:
	    break;

	  default:
	    cerr << get_line() << ": error: " << path_ << " is not "
		 << "a reg in this context." << endl;
	    des->errors += 1;
	    return 0;
      }

      assert(sig);

      if (msb_ || lsb_) {

	    cerr << get_line() << ": error: bit/part selects not allowed "
		 << "on left side of procedural continuous assignment."
		 << endl;
	    des->errors += 1;
	    return 0;
      }

      return sig;
}

/*
 * $Log: elab_anet.cc,v $
 * Revision 1.10  2004/10/04 01:10:52  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.9  2003/09/19 03:50:12  steve
 *  Remove find_memory method from Design class.
 *
 * Revision 1.8  2003/06/21 01:21:43  steve
 *  Harmless fixup of warnings.
 *
 * Revision 1.7  2003/03/06 00:28:41  steve
 *  All NetObj objects have lex_string base names.
 *
 * Revision 1.6  2003/01/27 05:09:17  steve
 *  Spelling fixes.
 *
 * Revision 1.5  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.4  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.3  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.2  2001/01/06 02:29:36  steve
 *  Support arrays of integers.
 *
 * Revision 1.1  2000/12/06 06:31:09  steve
 *  Check lvalue of procedural continuous assign (PR#29)
 *
 */

