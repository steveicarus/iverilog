/*
 * Copyright (c) 2011-2012 Stephen Williams (steve@icarus.com)
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

# include  "pform.h"
# include  "parse_misc.h"
# include  "ivl_assert.h"

static ivl_variable_type_t figure_struct_base_type(struct_type_t*struct_type)
{
      ivl_variable_type_t base_type = IVL_VT_BOOL;

      for (list<struct_member_t*>::iterator cur = struct_type->members->begin()
		 ; cur != struct_type->members->end() ; ++ cur) {

	    struct_member_t*tmp = *cur;

	    if (tmp->type == IVL_VT_BOOL) {
		  continue;
	    }

	    if (tmp->type == IVL_VT_LOGIC) {
		  base_type = IVL_VT_LOGIC;
		  continue;
	    }
      }

      return base_type;
}

/*
 * When we parse a packed struct, we can early on (right here) figure
 * out the base type of the packed variable. Elaboration, later on,
 * well figure out the rest.
 */
static void pform_set_packed_struct(struct_type_t*struct_type, perm_string name)
{
      ivl_variable_type_t base_type = figure_struct_base_type(struct_type);

      PWire*net = pform_get_make_wire_in_scope(name, NetNet::REG, NetNet::NOT_A_PORT, base_type);
      net->set_struct_type(struct_type);
}

static void pform_set_struct_type(struct_type_t*struct_type, perm_string name)
{
      if (struct_type->packed_flag) {
	    pform_set_packed_struct(struct_type, name);
	    return;
      }

	// For now, can only handle packed structs.
      ivl_assert(*struct_type, 0);
}

void pform_set_struct_type(struct_type_t*struct_type, list<perm_string>*names)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur) {
	    pform_set_struct_type(struct_type, *cur);
      }
}

static void pform_makewire(const struct vlltype&li,
			   struct_type_t*struct_type,
			   NetNet::PortType ptype,
			   perm_string name,
			   list<named_pexpr_t>*)
{
      ivl_variable_type_t base_type = figure_struct_base_type(struct_type);

      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::WIRE, ptype, base_type);
      FILE_NAME(cur, li);
      cur->set_struct_type(struct_type);
}

void pform_makewire(const struct vlltype&li,
		    struct_type_t*struct_type,
		    NetNet::PortType ptype,
		    list<perm_string>*names,
		    list<named_pexpr_t>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur ) {
	    perm_string txt = *cur;
	    pform_makewire(li, struct_type, ptype, txt, attr);
      }

      delete names;
}
