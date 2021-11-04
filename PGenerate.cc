/*
 * Copyright (c) 2006-2021 Stephen Williams (steve@icarus.com)
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

# include  "PGenerate.h"
# include  "PWire.h"
# include  "ivl_assert.h"

using namespace std;

PGenerate::PGenerate(LexicalScope*parent, unsigned id)
: LexicalScope(parent), id_number(id)
{
      scheme_type = GS_NONE;
      directly_nested = false;
      local_index = false;
      loop_init = 0;
      loop_test = 0;
      loop_step = 0;
}

PGenerate::~PGenerate()
{
}

void PGenerate::add_gate(PGate*gate)
{
      gates.push_back(gate);
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

PNamedItem::SymbolType PGenerate::symbol_type() const
{
      return GENBLOCK;
}
