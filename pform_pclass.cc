/*
 * Copyright (c) 2012 Stephen Williams (steve@icarus.com)
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

# include  "pform.h"
# include  "PClass.h"
# include  "parse_misc.h"

static PClass*pform_cur_class = 0;

void pform_start_class_declaration(const struct vlltype&loc, class_type_t*type)
{
      PClass*class_scope = pform_push_class_scope(loc, type->name);
      class_scope->type = type;
      assert(pform_cur_class == 0);
      pform_cur_class = class_scope;
}

void pform_class_property(const struct vlltype&loc,
			  property_qualifier_t property_qual,
			  data_type_t*data_type,
			  list<decl_assignment_t*>*decls)
{
      assert(pform_cur_class);

      if (property_qual.test_static()) {
	      // I think the thing to do with static properties is to
	      // make them PWires directly in the PClass scope. They
	      // are wires like program/modules wires, and not
	      // instance members.
	    VLerror(loc, "sorry: static class properties not implemented.");
	    return;
      }

      for (list<decl_assignment_t*>::iterator cur = decls->begin()
		 ; cur != decls->end() ; ++cur) {

	    decl_assignment_t*curp = *cur;
	    pform_cur_class->type->properties[curp->name] = data_type;
      }

      VLerror(loc, "sorry: class properties not implemented yet.");
}

void pform_end_class_declaration(void)
{
      assert(pform_cur_class);
      pform_cur_class = 0;
      pform_pop_scope();
}
