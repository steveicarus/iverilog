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
#ident "$Id: Module.cc,v 1.1 1998/11/03 23:28:51 steve Exp $"
#endif

# include  "Module.h"
# include  "PWire.h"

void Module::add_gate(PGate*gate)
{
      gates_.push_back(gate);
}

void Module::add_wire(PWire*wire)
{
      wires_.push_back(wire);
}

void Module::add_behavior(PProcess*b)
{
      behaviors_.push_back(b);
}

PWire* Module::get_wire(const string&name)
{
      for (list<PWire*>::iterator cur = wires_.begin()
		 ; cur != wires_.end()
		 ; cur ++ ) {

	    if ((*cur)->name == name)
		  return *cur;
      }

      return 0;
}


/*
 * $Log: Module.cc,v $
 * Revision 1.1  1998/11/03 23:28:51  steve
 *  Introduce verilog to CVS.
 *
 */

