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
#ident "$Id: Module.cc,v 1.7 1999/09/17 02:06:25 steve Exp $"
#endif

# include  "Module.h"
# include  "PWire.h"

Module::Module(const string&name, const svector<Module::port_t*>*pp)
: name_(name)
{
      if (pp) {
	    ports_ = *pp;
	    for (unsigned idx = 0 ;  idx < ports_.count() ;  idx += 1) {
		  port_t*cur = ports_[idx];
		  if (cur == 0)
			continue;
		  for (unsigned jdx = 0 ;  jdx < cur->wires.count() ; jdx += 1)
			add_wire(cur->wires[jdx]);
	    }
      }
}

void Module::add_gate(PGate*gate)
{
      gates_.push_back(gate);
}

void Module::add_task(const string&name, PTask*task)
{
      tasks_[name] = task;
}

void Module::add_function(const string &name, PFunction *func)
{
      funcs_[name] = func;
}

void Module::add_wire(PWire*wire)
{
      wires_.push_back(wire);
}

void Module::add_behavior(PProcess*b)
{
      behaviors_.push_back(b);
}

unsigned Module::port_count() const
{
      return ports_.count();
}

const svector<PWire*>& Module::get_port(unsigned idx) const
{
      assert(idx < ports_.count());
      return ports_[idx]->wires;
}

unsigned Module::find_port(const string&name) const
{
      assert(name != "");
      for (unsigned idx = 0 ;  idx < ports_.count() ;  idx += 1)
	    if (ports_[idx]->name == name)
		  return idx;

      return ports_.count();
}


PWire* Module::get_wire(const string&name)
{
      for (list<PWire*>::iterator cur = wires_.begin()
		 ; cur != wires_.end()
		 ; cur ++ ) {

	    if ((*cur)->name() == name)
		  return *cur;
      }

      return 0;
}


/*
 * $Log: Module.cc,v $
 * Revision 1.7  1999/09/17 02:06:25  steve
 *  Handle unconnected module ports.
 *
 * Revision 1.6  1999/08/04 02:13:02  steve
 *  Elaborate module ports that are concatenations of
 *  module signals.
 *
 * Revision 1.5  1999/08/03 04:14:49  steve
 *  Parse into pform arbitrarily complex module
 *  port declarations.
 *
 * Revision 1.4  1999/07/31 19:14:47  steve
 *  Add functions up to elaboration (Ed Carter)
 *
 * Revision 1.3  1999/07/03 02:12:51  steve
 *  Elaborate user defined tasks.
 *
 * Revision 1.2  1999/06/17 05:34:42  steve
 *  Clean up interface of the PWire class,
 *  Properly match wire ranges.
 *
 * Revision 1.1  1998/11/03 23:28:51  steve
 *  Introduce verilog to CVS.
 *
 */

