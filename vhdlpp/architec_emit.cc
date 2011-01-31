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
 */

# include  "architec.h"
# include  "entity.h"
# include  "expression.h"
# include  <iostream>
# include  <typeinfo>
# include  <cassert>

int Architecture::emit(ostream&out, Entity*entity)
{
      int errors = 0;
      for (list<Architecture::Statement*>::iterator cur = statements_.begin()
		 ; cur != statements_.end() ; ++cur) {

	    errors += (*cur)->emit(out, entity, this);
      }

      return errors;
}

int Architecture::Statement::emit(ostream&out, Entity*, Architecture*)
{
      out << " // " << get_fileline() << ": internal error: "
	  << "I don't know how to emit this statement! "
	  << "type=" << typeid(*this).name() << endl;
      return 1;
}

int SignalAssignment::emit(ostream&out, Entity*ent, Architecture*arc)
{
      int errors = 0;

      assert(rval_.size() == 1);
      Expression*rval = rval_.front();

      out << "// " << get_fileline() << endl;
      out << "assign " << target_name_ << " = ";

      errors += rval->emit(out, ent, arc);

      out << ";" << endl;
      return errors;
}

