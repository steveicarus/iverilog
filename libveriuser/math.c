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
#ident "$Id: math.c,v 1.1 2003/04/23 15:01:29 steve Exp $"
#endif

# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include "vpi_user.h"
# include "veriuser.h"

void tf_multiply_long(PLI_INT32*aof_low1, PLI_INT32*aof_high1,
		      PLI_INT32 aof_low2, PLI_INT32 aof_high2)
{
      if (sizeof(long) == 8) {
	    long a, b;
	    a = (*aof_high1 << 32) | (*aof_low1);
	    b = (aof_high2 << 32)  | (aof_low2);
	    a *= b;
	    *aof_low1 = a & 0xffffffff;
	    a >>= 32;
	    *aof_high1 = a & 0xffffffff;

      } else if (sizeof(long long) == 8) {
	    long long a, b;
	    a = (*aof_high1 << 32) | (*aof_low1);
	    b = ( aof_high2 << 32) | ( aof_low2);
	    a *= b;
	    *aof_low1 = a & 0xffffffff;
	    a >>= 32;
	    *aof_high1 = a & 0xffffffff;
      } else {
	    assert(0);
      }
}

/*
 * $Log: math.c,v $
 * Revision 1.1  2003/04/23 15:01:29  steve
 *  Add tf_synchronize and tf_multiply_long.
 *
 */

