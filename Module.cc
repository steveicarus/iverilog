/*
 * Copyright (c) 1998-2022 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "Module.h"
# include  "PGate.h"
# include  "PWire.h"
# include  "ivl_assert.h"

using namespace std;

list<Module::named_expr_t> Module::user_defparms;

/* n is a permallocated string. */
Module::Module(LexicalScope*parent, perm_string n)
: PScopeExtra(n, parent)
{
      library_flag = false;
      is_cell = false;
      is_interface = false;
      program_block = false;
      uc_drive = UCD_NONE;
}

Module::~Module()
{
}

void Module::add_gate(PGate*gate)
{
      gates_.push_back(gate);
}

unsigned Module::port_count() const
{
      return ports.size();
}

/*
 * Return the array of PEIdent object that are at this port of the
 * module. If the port is internally unconnected, return an empty
 * array.
 */
const vector<PEIdent*>& Module::get_port(unsigned idx) const
{
      ivl_assert(*this, idx < ports.size());
      static const vector<PEIdent*> zero;

      if (ports[idx])
	    return ports[idx]->expr;
      else
	    return zero;
}

unsigned Module::find_port(const char*name) const
{
      ivl_assert(*this, name != 0);
      for (unsigned idx = 0 ;  idx < ports.size() ;  idx += 1) {
	    if (ports[idx] == 0) {
		    /* It is possible to have undeclared ports. These
		       are ports that are skipped in the declaration,
		       for example like so: module foo(x ,, y); The
		       port between x and y is unnamed and thus
		       inaccessible to binding by name. */
		  continue;
	    }
	    ivl_assert(*this, ports[idx]);
	    if (ports[idx]->name == name)
		  return idx;
      }

      return ports.size();
}

perm_string Module::get_port_name(unsigned idx) const
{

      ivl_assert(*this, idx < ports.size());
      if (ports[idx] == 0 || ports[idx]->name.str() == 0) {
              /* It is possible to have undeclared ports. These
                 are ports that are skipped in the declaration,
                 for example like so: module foo(x ,, y); The
                 port between x and y is unnamed and thus
                 inaccessible to binding by name. Port references
		 that aren't simple or escaped identifiers are
		 also inaccessible to binding by name. */
            return perm_string::literal("unnamed");
      }
      return ports[idx]->name;
}

PExpr* Module::get_port_default_value(unsigned idx) const
{
      ivl_assert(*this, idx < ports.size());
      return ports[idx] ? ports[idx]->default_value : 0;
}


PGate* Module::get_gate(perm_string name)
{
      for (list<PGate*>::iterator cur = gates_.begin()
		 ; cur != gates_.end() ; ++ cur ) {

	    if ((*cur)->get_name() == name)
		  return *cur;
      }

      return 0;
}

const list<PGate*>& Module::get_gates() const
{
      return gates_;
}

PNamedItem::SymbolType Module::symbol_type() const
{
      if (program_block)
            return PROGRAM;
      if (is_interface)
            return INTERFACE;

      return MODULE;
}

bool Module::can_be_toplevel() const
{
      // Don't choose library modules.
      if (library_flag)
	    return false;

      // Don't choose modules with parameters without default value
      for (std::map<perm_string,param_expr_t*>::const_iterator cur =
	    parameters.begin(); cur != parameters.end(); ++cur) {
	    if (cur->second->expr == 0)
		  return false;
      }

      return true;
}
