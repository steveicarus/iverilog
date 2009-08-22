/*
 * Copyright (c) 2006,2009 Stephen Williams (steve@icarus.com)
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

# include  "PGenerate.h"
# include  "PWire.h"
# include  "ivl_assert.h"

PGenerate::PGenerate(unsigned id)
: id_number(id)
{
      direct_nested_ = false;
      parent = 0;
      lexical_scope = 0;
}

PGenerate::~PGenerate()
{
}

void PGenerate::add_gate(PGate*gate)
{
      gates.push_back(gate);
}

void PGenerate::probe_for_direct_nesting_(void)
{
      direct_nested_ = false;

      ivl_assert(*this, scheme_type==GS_CASE_ITEM || scheme_type==GS_CONDIT || scheme_type==GS_ELSE);

	// If this scheme has received an explicit name, then it
	// cannot be direct nested.
      if (scope_name[0] != '$') return;

      if (tasks.size() > 0) return;
      if (funcs.size() > 0) return;
      if (gates.size() > 0) return;
      if (parameters.size() > 0) return;
      if (localparams.size() > 0) return;
      if (events.size() > 0) return;
      if (wires.size() > 0) return;
      if (genvars.size() > 0) return;
      if (behaviors.size() > 0) return;
      if (analog_behaviors.size() > 0) return;

      if (generate_schemes.size() == 0) return;

      switch (generate_schemes.size()) {
	  case 1: {
		PGenerate*child = generate_schemes.front();
		if (child->scheme_type == GS_CONDIT)
		      direct_nested_ = true;
		if (child->scheme_type == GS_CASE)
		      direct_nested_ = true;
		break;
	  }

	  case 2: {
		PGenerate*child1 = generate_schemes.front();
		PGenerate*child2 = generate_schemes.back();
		if (child1->scheme_type==GS_CONDIT && child2->scheme_type==GS_ELSE)
		      direct_nested_ = true;
		if (child2->scheme_type==GS_CONDIT && child1->scheme_type==GS_ELSE)
		      direct_nested_ = true;
		break;
	  }
      }
}

ostream& operator << (ostream&out, PGenerate::scheme_t type)
{
      switch (type) {
	  case PGenerate::GS_NONE:
	    out << "GS_NONE";
	    break;
	  case PGenerate::GS_LOOP:
	    out << "GS_LOOP";
	    break;
	  case PGenerate::GS_CONDIT:
	    out << "GS_CONDIT";
	    break;
	  case PGenerate::GS_ELSE:
	    out << "GS_ELSE";
	    break;
	  case PGenerate::GS_CASE:
	    out << "GS_CASE";
	    break;
	  case PGenerate::GS_CASE_ITEM:
	    out << "GS_CASE_ITEM";
	    break;
	  case PGenerate::GS_NBLOCK:
	    out << "GS_NBLOCK";
	    break;
      }
      return out;
}
