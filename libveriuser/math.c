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
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: math.c,v 1.3 2003/06/13 19:23:42 steve Exp $"
#endif

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

/*
 * $Log: math.c,v $
 * Revision 1.3  2003/06/13 19:23:42  steve
 *  Add a bunch more PLI1 routines.
 *
 * Revision 1.2  2003/06/04 01:56:20  steve
 * 1) Adds configure logic to clean up compiler warnings
 * 2) adds acc_compare_handle, acc_fetch_range, acc_next_scope and
 *    tf_isetrealdelay, acc_handle_scope
 * 3) makes acc_next reentrant
 * 4) adds basic vpiWire type support
 * 5) fills in some acc_object_of_type() and acc_fetch_{full}type()
 * 6) add vpiLeftRange/RigthRange to signals
 *
 * Revision 1.1  2003/04/23 15:01:29  steve
 *  Add tf_synchronize and tf_multiply_long.
 *
 */

