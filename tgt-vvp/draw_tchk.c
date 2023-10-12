/*
 * Copyright (c) 2023 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2023 Leo Moser (leo.moser@pm.me)
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
# include  <math.h>
# include  <string.h>
# include  <inttypes.h>
# include  <limits.h>
# include  <assert.h>
# include  "ivl_alloc.h"

/*
 * This function draws a timing check
 */
void draw_tchk_in_scope(ivl_tchk_t tchk)
{
      // TODO use nexus???

      fprintf(vvp_out, "    .tchk_width ;\n");
}
