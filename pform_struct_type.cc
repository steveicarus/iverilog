/*
 * Copyright (c) 2011-2014 Stephen Williams (steve@icarus.com)
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
# include  "parse_misc.h"
# include  "ivl_assert.h"

ivl_variable_type_t struct_type_t::figure_packed_base_type(void) const
{
      if (! packed_flag)
	    return IVL_VT_NO_TYPE;

      if (members.get() == 0)
	    return IVL_VT_NO_TYPE;

      ivl_variable_type_t base_type = IVL_VT_BOOL;

      ivl_assert(*this, members.get());
      for (list<struct_member_t*>::iterator cur = members->begin()
		 ; cur != members->end() ; ++ cur) {

	    struct_member_t*tmp = *cur;

	    ivl_variable_type_t tmp_type = IVL_VT_NO_TYPE;
	    if (tmp->type.get())
		  tmp_type = tmp->type->figure_packed_base_type();

	    if (tmp_type == IVL_VT_BOOL) {
		  continue;
	    }

	    if (tmp_type == IVL_VT_LOGIC) {
		  base_type = IVL_VT_LOGIC;
		  continue;
	    }

	      // Oh no! Member is not a packable type!
	    return IVL_VT_NO_TYPE;
      }

      return base_type;
}

/*
 * When we parse a packed struct, we can early on (right here) figure
 * out the base type of the packed variable. Elaboration, later on,
 * well figure out the rest.
 */
static void pform_set_packed_struct(struct_type_t*struct_type, perm_string name, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      ivl_variable_type_t base_type = struct_type->figure_packed_base_type();

      PWire*net = pform_get_make_wire_in_scope(name, net_type, NetNet::NOT_A_PORT, base_type);
      assert(net);
      net->set_data_type(struct_type);
      pform_bind_attributes(net->attributes, attr, true);
}

static void pform_set_struct_type(struct_type_t*struct_type, perm_string name, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      if (struct_type->packed_flag) {
	    pform_set_packed_struct(struct_type, name, net_type, attr);
	    return;
      }

	// For now, can only handle packed structs. The parser generates
	// a "sorry" message, so no need to do anything here.
}

void pform_set_struct_type(struct_type_t*struct_type, list<perm_string>*names, NetNet::Type net_type, list<named_pexpr_t>*attr)
{
      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++ cur) {
	    pform_set_struct_type(struct_type, *cur, net_type, attr);
      }
}

static void pform_makewire(const struct vlltype&li,
			   struct_type_t*struct_type,
			   NetNet::PortType ptype,
			   perm_string name,
			   list<named_pexpr_t>*)
{
      ivl_variable_type_t base_type = struct_type->figure_packed_base_type();

      PWire*cur = pform_get_make_wire_in_scope(name, NetNet::WIRE, ptype, base_type);
      assert(cur);
      FILE_NAME(cur, li);
      cur->set_data_type(struct_type);
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
