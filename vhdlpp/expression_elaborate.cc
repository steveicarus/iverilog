/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "expression.h"
# include  "architec.h"
# include  "entity.h"
# include  <iostream>

using namespace std;

int Expression::elaborate_lval(Entity*, Architecture*)
{
      cerr << get_fileline() << ": error: Expression is not a valie l-value." << endl;
      return 1;
}

int ExpName::elaborate_lval(Entity*ent, Architecture*arc)
{
      int errors = 0;

      if (const InterfacePort*cur = ent->find_port(name_)) {
	    if (cur->mode != PORT_OUT) {
		  cerr << get_fileline() << ": error: Assignment to "
			"input port " << name_ << "." << endl;
		  return errors += 1;
	    }

	    ent->set_declaration_l_value(name_, true);
	    return errors;
      }

      Signal*sig = arc->find_signal(name_);
      if (sig == 0) {
	    cerr << get_fileline() << ": error: Signal/variable " << name_
		 << " not found in this context." << endl;
	    return errors + 1;
      }

      return errors;
}

int ExpNameALL::elaborate_lval(Entity*ent, Architecture*arc)
{
      return Expression::elaborate_lval(ent, arc);
}
