/* vi:sw=6
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: a_handle_tfarg.c,v 1.1 2002/06/02 19:03:58 steve Exp $"
#endif

#include  <acc_user.h>
#include  <vpi_user.h>

/*
 * acc_handle_tfarg implemented using VPI interface
 */
handle acc_handle_tfarg(int n)
{
      vpiHandle sys_h, sys_i, rtn_h;
      
      if (n > 0) {
	    sys_h = vpi_handle(vpiSysTfCall, 0 /* NULL */);
	    sys_i = vpi_iterate(vpiArgument, sys_h);

	    /* find nth arg */
	    while (n > 0) {
		  rtn_h = vpi_scan(sys_i);
		  n--;
	    }
      } else {
	    rtn_h = (vpiHandle) 0;
      }

      return rtn_h;
}

/*
 * $Log: a_handle_tfarg.c,v $
 * Revision 1.1  2002/06/02 19:03:58  steve
 *  Add acc_handle_tfarg and acc_next_topmode
 *
 */
