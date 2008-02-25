/*
 * Copyright (c) 2006 Stephen Williams (steve@icarus.com)
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
#ident "$Id: PGenerate.cc,v 1.4 2007/06/02 03:42:12 steve Exp $"
#endif

# include  "PGenerate.h"
# include  "PWire.h"

PGenerate::PGenerate(unsigned id)
: id_number(id)
{
      parent = 0;
}

PGenerate::~PGenerate()
{
}

PWire* PGenerate::get_wire(perm_string name) const
{
      map<perm_string,PWire*>::const_iterator obj = wires.find(name);
      if (obj == wires.end())
	    return 0;
      else
	    return (*obj).second;
}

void PGenerate::add_gate(PGate*gate)
{
      gates.push_back(gate);
}

void PGenerate::add_behavior(PProcess*proc)
{
      behaviors.push_back(proc);
}
