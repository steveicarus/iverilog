/*
 * Copyright (c) 1998-2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: Module.cc,v 1.25 2004/10/04 01:10:51 steve Exp $"
#endif

# include "config.h"

# include  "Module.h"
# include  "PGate.h"
# include  "PWire.h"
# include  <assert.h>

/* n is a permallocated string. */
Module::Module(perm_string n)
: name_(n)
{
      default_nettype = NetNet::NONE;
}

Module::~Module()
{
}

void Module::add_gate(PGate*gate)
{
      gates_.push_back(gate);
}

void Module::add_task(perm_string name, PTask*task)
{
      tasks_[name] = task;
}

void Module::add_function(perm_string name, PFunction *func)
{
      funcs_[name] = func;
}

PWire* Module::add_wire(PWire*wire)
{
      PWire*&ep = wires_[wire->path()];
      if (ep) return ep;

      assert(ep == 0);
      ep = wire;
      return wire;
}

void Module::add_behavior(PProcess*b)
{
      behaviors_.push_back(b);
}

unsigned Module::port_count() const
{
      return ports.count();
}

/*
 * Return the array of PEIdent object that are at this port of the
 * module. If the port is internally unconnected, return an empty
 * array.
 */
const svector<PEIdent*>& Module::get_port(unsigned idx) const
{
      assert(idx < ports.count());
      static svector<PEIdent*> zero;

      if (ports[idx])
	    return ports[idx]->expr;
      else
	    return zero;
}

unsigned Module::find_port(const char*name) const
{
      assert(name != "");
      for (unsigned idx = 0 ;  idx < ports.count() ;  idx += 1) {
	    if (ports[idx] == 0) {
		    /* It is possible to have undeclared ports. These
		       are ports that are skipped in the declaration,
		       for example like so: module foo(x ,, y); The
		       port between x and y is unnamed and thus
		       inaccessible to binding by name. */
		  continue;
	    }
	    assert(ports[idx]);
	    if (ports[idx]->name == name)
		  return idx;
      }

      return ports.count();
}


PWire* Module::get_wire(const hname_t&name) const
{
      map<hname_t,PWire*>::const_iterator obj = wires_.find(name);
      if (obj == wires_.end())
	    return 0;
      else
	    return (*obj).second;
}

PGate* Module::get_gate(perm_string name)
{
      for (list<PGate*>::iterator cur = gates_.begin()
		 ; cur != gates_.end()
		 ; cur ++ ) {

	    if ((*cur)->get_name() == name)
		  return *cur;
      }

      return 0;
}

const map<hname_t,PWire*>& Module::get_wires() const
{
      return wires_;
}

const list<PGate*>& Module::get_gates() const
{
      return gates_;
}

const list<PProcess*>& Module::get_behaviors() const
{
      return behaviors_;
}


/*
 * $Log: Module.cc,v $
 * Revision 1.25  2004/10/04 01:10:51  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.24  2004/06/13 04:56:53  steve
 *  Add support for the default_nettype directive.
 *
 * Revision 1.23  2004/02/20 06:22:56  steve
 *  parameter keys are per_strings.
 *
 * Revision 1.22  2004/02/18 17:11:54  steve
 *  Use perm_strings for named langiage items.
 *
 * Revision 1.21  2003/04/02 03:00:14  steve
 *  Cope with empty module ports while binding by name.
 *
 * Revision 1.20  2003/03/06 04:37:12  steve
 *  lex_strings.add module names earlier.
 *
 * Revision 1.19  2002/08/12 01:34:58  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.18  2002/05/19 23:37:28  steve
 *  Parse port_declaration_lists from the 2001 Standard.
 *
 * Revision 1.17  2001/12/03 04:47:14  steve
 *  Parser and pform use hierarchical names as hname_t
 *  objects instead of encoded strings.
 *
 * Revision 1.16  2001/10/20 05:21:51  steve
 *  Scope/module names are char* instead of string.
 *
 * Revision 1.15  2001/07/25 03:10:48  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.14  2000/11/15 20:31:05  steve
 *  Fix warning about temporaries.
 *
 * Revision 1.13  2000/11/05 06:05:59  steve
 *  Handle connectsion to internally unconnected modules (PR#38)
 *
 * Revision 1.12  2000/05/16 04:05:15  steve
 *  Module ports are really special PEIdent
 *  expressions, because a name can be used
 *  many places in the port list.
 *
 * Revision 1.11  2000/03/12 17:09:40  steve
 *  Support localparam.
 *
 * Revision 1.10  2000/02/23 02:56:53  steve
 *  Macintosh compilers do not support ident.
 *
 * Revision 1.9  2000/01/09 20:37:57  steve
 *  Careful with wires connected to multiple ports.
 *
 * Revision 1.8  1999/12/11 05:45:41  steve
 *  Fix support for attaching attributes to primitive gates.
 *
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

