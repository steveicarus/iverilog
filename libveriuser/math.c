/*
 * Copyright (c) 2003 Stephen Williams (steve@icarus.com)
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

# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include "config.h"
# include "vpi_user.h"
# include "veriuser.h"

void tf_multiply_long(PLI_INT32*aof_low1, PLI_INT32*aof_high1,
		      PLI_INT32 aof_low2, PLI_INT32 aof_high2)
{
      ivl_u64_t a, b;
      a = *aof_high1;
      a = (a << 32) | *aof_low1;
      b = aof_high2;
      b = (b << 32) | aof_low2;
      a *= b;
      *aof_high1 = (a >> 32) & 0xffffffff;
      *aof_low1 = a & 0xffffffff;
}

void tf_real_to_long(double real, PLI_INT32*low, PLI_INT32*high)
{
      ivl_u64_t rtn = (ivl_u64_t)real;
      *high = (rtn >> 32) & 0xffffffff;
      *low = rtn & 0xffffffff;
}

void tf_long_to_real(PLI_INT32 low, PLI_INT32 high, double *real)
{
      ivl_u64_t a;
      a = high;
      a = (a << 32) | low;
      *real = (double)a;
}
