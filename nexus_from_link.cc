/*
 * Copyright (c) 2000 Stephen Williams (steve@.com)
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
#ident "$Id: nexus_from_link.cc,v 1.3 2000/05/07 04:37:56 steve Exp $"
#endif

# include  "netmisc.h"
# include  <strstream>
# include  <string>
# include  <typeinfo>

string nexus_from_link(const Link*lnk)
{
      const NetNet*sig = dynamic_cast<const NetNet*>(lnk->get_obj());
      unsigned pin = lnk->get_pin();

      for (const Link*cur = lnk->next_link()
		 ;  cur != lnk ;  cur = cur->next_link()) {

	    const NetNet*cursig = dynamic_cast<const NetNet*>(cur->get_obj());
	    if (cursig == 0)
		  continue;

	    if (sig == 0) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if ((cursig->pin_count() == 1) && (sig->pin_count() > 1))
		  continue;

	    if ((cursig->pin_count() > 1) && (sig->pin_count() == 1)) {
		  sig = cursig;
		  pin = cur->get_pin();
		  continue;
	    }

	    if (cursig->local_flag() && !sig->local_flag())
		  continue;

	    if (cursig->name() < sig->name())
		  continue;

	    sig = cursig;
	    pin = cur->get_pin();
      }

      if (sig == 0) {
	    const NetObj*obj = lnk->get_obj();
	    pin = lnk->get_pin();
	    cerr << "internal error: No signal for nexus of " <<
		  obj->name() << " pin " << pin << "(" <<
		  lnk->get_name() << "<" << lnk->get_inst() << ">)"
		  " type=" << typeid(*obj).name() << "?" << endl;
      }
      assert(sig);
      ostrstream tmp;
      tmp << sig->name();
      if (sig->pin_count() > 1)
	    tmp << "<" << pin << ">";
      tmp << ends;

      return tmp.str();
}

/*
 * $Log: nexus_from_link.cc,v $
 * Revision 1.3  2000/05/07 04:37:56  steve
 *  Carry strength values from Verilog source to the
 *  pform and netlist for gates.
 *
 *  Change vvm constants to use the driver_t to drive
 *  a constant value. This works better if there are
 *  multiple drivers on a signal.
 *
 * Revision 1.2  2000/03/20 17:40:54  steve
 *  More complete error message about no signal.
 *
 * Revision 1.1  2000/03/16 19:03:03  steve
 *  Revise the VVM backend to use nexus objects so that
 *  drivers and resolution functions can be used, and
 *  the t-vvm module doesn't need to write a zillion
 *  output functions.
 *
 */

