/*
 * Copyright (c) 2010-2011 Stephen Williams (steve@icarus.com)
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

# include  "vvp_priv.h"
# include  "ivl_alloc.h"
# include  <stdlib.h>
# include  <string.h>
# include  <assert.h>

// HERE: This still needs work! Fall back to <width>'<signed>b format for
//       very large constants. This should also be int64_t sized values.
//       The run time still needs to be modified to work with signed values
//       and we also need to pass the file and line information.
static void draw_enum2_value(ivl_enumtype_t enumtype, unsigned idx)
{
      long val, mask;
      const char*bits = ivl_enum_bits(enumtype, idx);
      const char*bit;
      unsigned len = strlen(bits);
	/* Sign extend if needed. */
      if (ivl_enum_signed(enumtype)) {
	    assert(len <= sizeof(long)*8);
	    if ((bits[len-1] == '1') && (len < sizeof(long)*8)) {
		  val = -1L & ~((1L << len) - 1L);
// HERE: Remove once vvp has been updated.
		  if (ivl_enum_signed(enumtype)) {
			fprintf(stderr, "%s:%u: tgt-vvp sorry: signed "
			        "enumerations with negative values are not "
			        "currently supported by vvp.\n",
			        ivl_enum_file(enumtype),
			        ivl_enum_lineno(enumtype));
			vvp_errors += 1;
		  }
	    } else val = 0;
      } else {
	    assert(len < sizeof(long)*8);
	    val = 0;
      }
      for (bit = bits, mask = 1 ; *bit != 0 ; bit += 1, mask <<= 1) {
	    if (*bit == '1')
		  val |= mask;
      }

      fprintf(vvp_out, "%ld", val);
}

static void draw_enum4_value(ivl_enumtype_t enumtype, unsigned idx)
{
      const char*bits = ivl_enum_bits(enumtype, idx);
      const char*bit;

      fprintf(vvp_out, "%u'%sb", ivl_enum_width(enumtype),
                                 ivl_enum_signed(enumtype) ? "s" : "");

      for (bit = bits+strlen(bits) ; bit > bits ; bit -= 1)
	    fputc(bit[-1], vvp_out);

}

void draw_enumeration_in_scope(ivl_enumtype_t enumtype)
{
      unsigned idx;
      unsigned name_count = ivl_enum_names(enumtype);
      const char*dtype = ivl_enum_type(enumtype)==IVL_VT_BOOL? "2" : "4";

      fprintf(vvp_out, "enum%p .enum%s (%u)\n", enumtype,
	      dtype, ivl_enum_width(enumtype));

      for (idx = 0 ; idx < name_count ; idx += 1) {
	    fprintf(vvp_out, "   \"%s\" ", ivl_enum_name(enumtype, idx));

	    switch (ivl_enum_type(enumtype)) {
		case IVL_VT_BOOL:
		  draw_enum2_value(enumtype, idx);
		  break;
		case IVL_VT_LOGIC:
		  draw_enum4_value(enumtype, idx);
		  break;
		default:
		  assert(0);
	    }

	    if ((idx+1) < name_count)
		  fputc(',', vvp_out);
	    fputc('\n', vvp_out);
      }

      fprintf(vvp_out, " ;\n");
}
