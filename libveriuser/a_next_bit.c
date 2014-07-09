/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@picturel.com)
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

# include  <assert.h>
# include  <ctype.h>
# include  <acc_user.h>
# include  <vpi_user.h>
# include  "priv.h"

handle acc_next_bit(handle ref, handle bit)
{
      (void)ref; /* Parameter is not used. */
      (void)bit; /* Parameter is not used. */
      if (pli_trace) {
	    fprintf(pli_trace, "acc_next_bit: enter.\n");
	    fflush(pli_trace);
      }

      fprintf(stderr, "acc_next_bit: XXXX not implemented. XXXX\n");

      if (pli_trace) {
	    fprintf(pli_trace, "acc_next_bit: return.\n");
	    fflush(pli_trace);
      }

      return 0;
}
