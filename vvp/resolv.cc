/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: resolv.cc,v 1.2 2001/05/12 20:38:06 steve Exp $"
#endif

# include  "resolv.h"
# include  "schedule.h"

static void blend(unsigned&val, unsigned&drv0, unsigned drv1,
		  unsigned inp, unsigned inp0, unsigned inp1)
{
      switch (val) {
	  case 3:
	    val = inp;
	    drv0 = inp0;
	    drv1 = inp1;
	    break;

	  case 0:
	    switch (inp) {
		case 0:
		  if (drv0 < inp0)
			drv0 = inp0;
		  break;

		case 1:
		  if (drv0 < inp1) {
			val = 1;
			drv1 = inp1;
		  }
		  break;

		case 2:
		  if (drv0 < inp1) {
			val = 2;
			if (drv0 < inp0)
			      drv0 = inp0;
			if (drv1 < inp1)
			      drv0 = inp1;
		  }
		  break;
	    }
	    break;

	  case 1:
	    switch (inp) {
		case 0:
		  if (drv1 < inp0) {
			val = 0;
			drv1 = inp1;
		  }
		  break;

		case 1:
		  if (drv1 < inp1) {
			drv1 = inp1;
		  }
		  break;

		case 2:
		  if (drv1 < inp0) {
			val = 2;
			if (drv0 < inp0)
			      drv0 = inp0;
			if (drv1 < inp1)
			      drv0 = inp1;
		  }
		  break;
	    }
	    break;


	  case 2:
	    switch (inp) {
		case 0:
		  if (drv1 < inp0) {
			val = 0;
			drv0 = inp0;
			drv1 = inp1;
		  }
		  break;

		case 1:
		  if (drv0 < inp1) {
			val = 1;
			drv0 = inp0;
			drv1 = inp1;
		  }
		  break;

		case 2:
		  if (drv0 < inp0)
			drv0 = inp0;
		  if (drv1 < inp1)
			drv0 = inp1;
		  break;
	    }
	    break;

      }
}

/*
 * For now, cheat and resolve the values without using the actual
 * strengths. The strengths are not yet available to the inputs, so
 * this is all we can do for now.
 */
void vvp_resolv_s::set(vvp_ipoint_t ptr, functor_t fp, bool push)
{
      unsigned val = (fp->ival >> 0) & 3;
      unsigned drv0 = (fp->idrive >> 0) & 7;
      unsigned drv1 = (fp->idrive >> 3) & 7;

      blend(val, drv0, drv1,
	    (fp->ival >> 2) & 3,
	    (fp->idrive >> 6) & 7,
	    (fp->idrive >> 9) & 7);

      blend(val, drv0, drv1,
	    (fp->ival >> 4) & 3,
	    (fp->idrive >>12) & 7,
	    (fp->idrive >>15) & 7);

      blend(val, drv0, drv1,
	    (fp->ival >> 6) & 3,
	    (fp->idrive >>18) & 7,
	    (fp->idrive >>21) & 7);

      fp->odrive0 = drv0;
      fp->odrive1 = drv1;

	/* If the output changes, then create a propagation event. */
      if (val != fp->oval) {
	    fp->oval = val;
	    if (push)
		  functor_propagate(ptr);
	    else
		  schedule_functor(ptr, 0);
      }
}

/*
 * $Log: resolv.cc,v $
 * Revision 1.2  2001/05/12 20:38:06  steve
 *  A resolver that understands some simple strengths.
 *
 * Revision 1.1  2001/05/09 02:53:53  steve
 *  Implement the .resolv syntax.
 *
 */

