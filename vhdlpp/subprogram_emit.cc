/*
 * Copyright (c) 2013 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2013 / Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "subprogram.h"
# include  "sequential.h"
# include  "vtype.h"
# include  <iostream>

using namespace std;

int Subprogram::emit_package(ostream&fd) const
{
      int errors = 0;

      if (return_type_) {
	    fd << "function ";
	    return_type_->emit_def(fd);
	    fd << " " << name_;
	    fd << "(";
      } else {
	    fd << "task " << name_ << ";" << endl;
      }

      for (list<InterfacePort*>::const_iterator cur = ports_->begin()
		 ; cur != ports_->end() ; ++cur) {
	    if (cur != ports_->begin())
		  fd << ", ";
	    InterfacePort*curp = *cur;
	    switch (curp->mode) {
		case PORT_IN:
		  fd << "input ";
		  break;
		case PORT_OUT:
		  fd << "output ";
		  break;
		case PORT_NONE:
		  fd << "inout /* PORT_NONE? */ ";
		  break;
	    }

	    errors += curp->type->emit_def(fd);
	    fd << " \\" << curp->name << " ";
      }

      fd << ");" << endl;

      if (statements_) {
	    for (list<SequentialStmt*>::const_iterator cur = statements_->begin()
		       ; cur != statements_->end() ; ++cur) {
		  errors += (*cur)->emit(fd, 0, 0);
	    }
      } else {
	    fd << " begin /* empty body */ end" << endl;
      }

      if (return_type_)
	    fd << "endfunction" << endl;
      else
	    fd << "endtask" << endl;

      return errors;
}
