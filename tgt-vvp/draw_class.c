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


# include  "vvp_priv.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>
# include  <inttypes.h>

static void show_prop_type(ivl_type_t ptype)
{
	// XXXX: For now, assume all properties are 32bit integers.
      fprintf(vvp_out, "\"b32\"");
}

void draw_class_in_scope(ivl_type_t classtype)
{
      int idx;
      fprintf(vvp_out, "C%p  .class \"%s\" [%d]\n",
	      classtype, ivl_type_name(classtype), ivl_type_properties(classtype));

      for (idx = 0 ; idx < ivl_type_properties(classtype) ; idx += 1) {
	    fprintf(vvp_out, " %3d: \"%s\", ", idx, ivl_type_prop_name(classtype,idx));
	    show_prop_type(ivl_type_prop_type(classtype,idx));
	    fprintf(vvp_out, "\n");
      }

      fprintf(vvp_out, " ;\n");
}
