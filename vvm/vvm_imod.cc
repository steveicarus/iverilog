/*
 * Copyright (c) 2000 Stephen Williams (steve@picturel.com)
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
#if !defined(WINNT) && !defined(WINNT)
#ident "$Id: vvm_imod.cc,v 1.1 2000/05/19 04:22:56 steve Exp $"
#endif


# include  "vvm_func.h"
# include  "vvm_gates.h"
# include  <assert.h>

void vvm_binop_imod(vvm_bitset_t&v, const vvm_bitset_t&l, const vvm_bitset_t&r)
{
      assert(v.get_width() <= 8*sizeof(unsigned long));
      assert(l.get_width() <= 8*sizeof(unsigned long));
      assert(r.get_width() <= 8*sizeof(unsigned long));

      unsigned long lv = 0, rv = 0;
      unsigned long vv;

      for (unsigned idx = 0 ;  idx < l.get_width() ;  idx += 1) {

	    if (B_ISXZ(l[idx]))
		  goto unknown_result;

	    if (B_IS1(l[idx]))
		  lv |= 1 << idx;
      }

      for (unsigned idx = 0 ;  idx < r.get_width() ;  idx += 1) {

	    if (B_ISXZ(r[idx]))
		  goto unknown_result;

	    if (B_IS1(r[idx]))
		  rv |= 1 << idx;
      }

      if (rv == 0)
	    goto unknown_result;

      vv = lv % rv;

      for (unsigned idx = 0 ;  idx < v.get_width() ;  idx += 1) {

	    if (vv & 1)
		  v[idx] = St1;
	    else
		  v[idx] = St0;

	    vv >>= 1;
      }

      return;

 unknown_result:
      for (unsigned idx= 0 ;  idx < v.get_width() ;  idx += 1)
	    v[idx] = StX;
}


/*
 * $Log: vvm_imod.cc,v $
 * Revision 1.1  2000/05/19 04:22:56  steve
 *  Add the integer modulus function.
 *
 */

