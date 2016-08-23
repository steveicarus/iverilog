/*
 * Copyright (c) 2011,2013 Stephen Williams (steve@icarus.com)
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
 *    Picture Elements, Inc., 777 Panoramic Way, Berkeley, CA 94704.
 */

# include  "parse_misc.h"
# include  "parse_types.h"
# include  "parse_api.h"
# include  "entity.h"
# include  "architec.h"
# include  "expression.h"
# include  "vtype.h"
# include  "compiler.h"
# include  <iostream>
# include  <cassert>
# include  <cstring>

using namespace std;

void bind_entity_to_active_scope(const char*ename, ActiveScope*scope)
{
      perm_string ekey = lex_strings.make(ename);
      std::map<perm_string,Entity*>::const_iterator idx = design_entities.find(ekey);
      if (idx == design_entities.end()) {
	    return;
      }

      scope->bind(idx->second);
}

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

      /* FIXME: entities can have multiple architectures attached to them
      This is to be configured by VHDL's configurations (not yet implemented) */
      Architecture*old_arch = idx->second->add_architecture(arch);
      if (old_arch != arch) {
	    cerr << arch->get_fileline() << ": warning: "
		 << "Architecture " << arch->get_name()
		 << " for entity " << idx->first
		 << " is already defined here: " << old_arch->get_fileline() << endl;
	    parse_errors += 1;
      }
}

static const VType* calculate_subtype_array(const YYLTYPE&loc, const char*base_name,
					    ScopeBase* /* scope */,
					    Expression*array_left,
					    bool downto,
					    Expression*array_right)
{
      const VType*base_type = parse_type_by_name(lex_strings.make(base_name));

      if (base_type == 0) {
	    errormsg(loc, "Unable to find array base type '%s'.\n", base_name);
	    return 0;
      }

      assert(array_left==0 || array_right!=0);

      // unfold typedef, there might be VTypeArray inside
      const VType*origin_type = base_type;
      const VTypeDef*type_def = dynamic_cast<const VTypeDef*> (base_type);
      if (type_def) {
          base_type = type_def->peek_definition();
      }

      const VTypeArray*base_array = dynamic_cast<const VTypeArray*> (base_type);
      if (base_array) {
	    assert(array_left && array_right);

	    vector<VTypeArray::range_t> range (base_array->dimensions());

	      // For now, I only know how to handle 1 dimension
	    assert(base_array->dimensions().size() == 1);

	    range[0] = VTypeArray::range_t(array_left, array_right, downto);

	      // use typedef as the element type if possible
	    const VType*element = type_def ? origin_type : base_array->element_type();

	    VTypeArray*subtype = new VTypeArray(element, range,
                                                base_array->signed_vector());
	    subtype->set_parent_type(base_array);
	    return subtype;
      }

      return base_type;
}

const VType* calculate_subtype_array(const YYLTYPE&loc, const char*base_name,
				     ScopeBase*scope, list<ExpRange*>*ranges)
{
      if (ranges->size() == 1) {
	    ExpRange*tmpr = ranges->front();
	    Expression*lef = tmpr->left();
	    Expression*rig = tmpr->right();
	    return calculate_subtype_array(loc, base_name, scope,
					   lef,
					   (tmpr->direction() == ExpRange::DOWNTO
						? true
						: false),
					   rig);
      }

      sorrymsg(loc, "Don't know how to handle multiple ranges here.\n");
      return 0;
}

const VType* calculate_subtype_range(const YYLTYPE&loc, const char*base_name,
				     ScopeBase*scope,
				     Expression*range_left,
				     int direction,
				     Expression*range_right)
{
      const VType*base_type = parse_type_by_name(lex_strings.make(base_name));

      if (base_type == 0) {
	    errormsg(loc, "Unable to find range base type '%s'.\n", base_name);
	    return 0;
      }

      assert(range_left && range_right);

      int64_t left_val, right_val;
      VTypeRange*subtype;

      if(range_left->evaluate(scope, left_val) && range_right->evaluate(scope, right_val)) {
	    subtype = new VTypeRangeConst(base_type, left_val, right_val);
      } else {
	    subtype = new VTypeRangeExpr(base_type, range_left, range_right, direction);
      }

      return subtype;
}

ExpString*parse_char_enums(const char*str)
{
      if(!strcasecmp(str, "LF"))
	    return new ExpString("\\n");

      if(!strcasecmp(str, "CR"))
	    return new ExpString("\\r");

      return NULL;
}
