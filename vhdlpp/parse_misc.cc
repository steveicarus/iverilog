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

# include  "parse_misc.h"
# include  "parse_api.h"
# include  "entity.h"
# include  "architec.h"
# include  "expression.h"
# include  "vtype.h"
# include  "compiler.h"
# include  <iostream>
# include  <cassert>

using namespace std;

void bind_architecture_to_entity(const char*ename, Architecture*arch)
{
      perm_string ekey = lex_strings.make(ename);
      std::map<perm_string,Entity*>::const_iterator idx = design_entities.find(ekey);
      if (idx == design_entities.end()) {
	    cerr << arch->get_fileline() << ": error: No entity " << ekey
		 << " for architecture " << arch->get_name()
		 << "." << endl;
	    parse_errors += 1;
	    return;
      }

      Architecture*old_arch = idx->second->add_architecture(arch);
      if (old_arch != arch) {
	    cerr << arch->get_fileline() << ": error: "
		 << "Architecture " << arch->get_name()
		 << " for entity " << idx->second->get_name()
		 << " is already defined here: " << old_arch->get_fileline() << endl;
	    parse_errors += 1;
	    return;
      }
}

const VType* calculate_subtype(const char*base_name,
			       Expression*array_left,
			       bool downto,
			       Expression*array_right)
{
      const VType*base_type = global_types[lex_strings.make(base_name)];

      assert(array_left==0 || array_right!=0);

      const VTypeArray*base_array = dynamic_cast<const VTypeArray*> (base_type);
      if (base_array) {
	    assert(array_left && array_right);

	    vector<VTypeArray::range_t> range (base_array->dimensions());

	      // For now, I only know how to handle 1 dimension
	    assert(base_array->dimensions() == 1);

	    int64_t left_val;
	    int64_t right_val;
	    bool rc = array_left->evaluate(left_val);
	    assert(rc);

	    rc = array_right->evaluate(right_val);
	    assert(rc);

	    range[0] = VTypeArray::range_t(left_val, right_val);

	    VTypeArray*subtype = new VTypeArray(base_array->element_type(), range);
	    return subtype;
      }

      return base_type;
}
