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


# include  "vtype.h"
# include <iostream>
# include  <typeinfo>
# include  <cassert>

using namespace std;

int VType::decl_t::emit(ostream&out, perm_string name) const
{
      const char*wire = reg_flag? "reg" : "wire";

      switch (type) {
	  case VType::VNONE:
	    out << "// N type for " << name << endl;
	    break;
	  case VType::VLOGIC:
	    out << wire<< " logic ";
	    if (signed_flag)
		  out << "signed ";
	    if (msb != lsb)
		  out << "[" << msb << ":" << lsb << "] ";
	    out << "\\" << name << " ";
	    break;
	  case VType::VBOOL:
	    out << wire << " bool ";
	    if (signed_flag)
		  out << "signed ";
	    if (msb != lsb)
		  out << "[" << msb << ":" << lsb << "] ";
	    out << "\\" << name << " ";
	    break;
      }

      return 0;
}
