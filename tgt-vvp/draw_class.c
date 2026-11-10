/*
 * Copyright (c) 2012-2026 Stephen Williams (steve@icarus.com)
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

static void show_prop_type_vector(ivl_type_t ptype)
{
      ivl_variable_type_t data_type = ivl_type_base(ptype);
      unsigned packed_dimensions = ivl_type_packed_dimensions(ptype);

      const char*signed_flag = ivl_type_signed(ptype)? "s" : "";
      char code = data_type==IVL_VT_BOOL? 'b' : 'L';

      if (packed_dimensions == 0) {
	    fprintf(vvp_out, "\"%s%c1\"", signed_flag, code);

      } else {
	    fprintf(vvp_out, "\"%s%c%u\"", signed_flag, code,
		    ivl_type_packed_width(ptype));
      }
}

static void show_prop_type(ivl_type_t ptype)
{
      ivl_variable_type_t data_type = ivl_type_base(ptype);
      unsigned packed_dimensions = ivl_type_packed_dimensions(ptype);

      switch (data_type) {
	  case IVL_VT_REAL:
	    fprintf(vvp_out, "\"r\"");
	    break;
	  case IVL_VT_STRING:
	    fprintf(vvp_out, "\"S\"");
	    break;
	  case IVL_VT_BOOL:
	  case IVL_VT_LOGIC:
	    show_prop_type_vector(ptype);
	    break;
	  case IVL_VT_DARRAY:
	  case IVL_VT_QUEUE:
	  case IVL_VT_CLASS:
	    fprintf(vvp_out, "\"o\"");
	    if (packed_dimensions > 0) {
		  unsigned idx;
		  fprintf(vvp_out, " ");
		  for (idx = 0 ; idx < packed_dimensions ; idx += 1) {
			fprintf(vvp_out, "[%d:%d]",
				ivl_type_packed_msb(ptype,idx),
				ivl_type_packed_lsb(ptype,idx));
		  }
	    }
	    break;
	  default:
	    fprintf(vvp_out, "\"<ERROR-no-type>\"");
	    assert(0);
	    break;
      }
}

void draw_class_in_scope(ivl_type_t classtype)
{
      int idx;
      fprintf(vvp_out, "C%p  .class \"%s\" [%d]\n",
	      classtype, vvp_mangle_name(ivl_type_name(classtype)),
	      ivl_type_properties(classtype));

      for (idx = 0 ; idx < ivl_type_properties(classtype) ; idx += 1) {
	    fprintf(vvp_out, " %3d: \"%s\", ", idx,
		    vvp_mangle_name(ivl_type_prop_name(classtype, idx)));
	    show_prop_type(ivl_type_prop_type(classtype,idx));
	    fprintf(vvp_out, "\n");
      }

      fprintf(vvp_out, " ;\n");

      if (ivl_type_super(classtype))
	    fprintf(vvp_out, "    .super C%p, C%p;\n", classtype,
		    ivl_type_super(classtype));

      for (idx = 0 ; idx < ivl_type_virtuals(classtype) ; idx += 1) {
	    ivl_scope_t impl = ivl_type_virtual(classtype, idx);
	    ivl_scope_t root = ivl_scope_virtual_root(impl);
	    fprintf(vvp_out, "    .virt C%p, S_%p, TA_%p_%p;\n",
		    classtype, root, root, impl);
      }
}

int draw_class_cast(ivl_expr_t dest, ivl_expr_t src,
		    int report, int want_result)
{
      int is_prop = ivl_expr_type(dest) == IVL_EX_PROPERTY;
      unsigned lab_fail = local_count++;
      unsigned lab_end = local_count++;
      int errors;

	/* A property is stored through the object that holds it, so
	   that object goes on the stack below the new value. */
      if (is_prop)
	    fprintf(vvp_out, "    %%load/obj v%p_0;\n", ivl_expr_signal(dest));

      errors = draw_eval_object(src);

      fprintf(vvp_out, "    %%cast/class C%p, %d;\n",
	      is_prop ? ivl_expr_net_type(dest)
		      : ivl_signal_net_type(ivl_expr_signal(dest)), report);
      fprintf(vvp_out, "    %%jmp/0 T_%u.%u, 4;\n", thread_count, lab_fail);

      if (is_prop) {
	    fprintf(vvp_out, "    %%store/prop/obj %d, 0;\n",
		    ivl_expr_property_idx(dest));
	    fprintf(vvp_out, "    %%pop/obj 1, 0;\n");
      } else {
	    fprintf(vvp_out, "    %%store/obj v%p_0;\n", ivl_expr_signal(dest));
      }
      if (want_result)
	    fprintf(vvp_out, "    %%pushi/vec4 1, 0, 32;\n");
      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_end);

      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_fail);
      fprintf(vvp_out, "    %%pop/obj %d, 0;\n", is_prop ? 2 : 1);
      if (want_result)
	    fprintf(vvp_out, "    %%pushi/vec4 0, 0, 32;\n");
      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_end);

      return errors;
}
