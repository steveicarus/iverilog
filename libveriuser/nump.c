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
#ifdef HAVE_CVS_IDENT
#ident "$Id: nump.c,v 1.3 2003/05/28 03:38:05 steve Exp $"
#endif

#include  <vpi_user.h>
#include  <stdio.h>

/*
 * tf_nump implemented using VPI interface
 */

int tf_inump(void *obj)
{
      vpiHandle sys_h, sys_i;
      int cnt;

      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

        /* count number of args */
      for (cnt = 0; sys_i && vpi_scan(sys_i); cnt++);

      return cnt;
}

int tf_nump(void)
{
      vpiHandle sys_h = vpi_handle(vpiSysTfCall, 0 /* NULL */);
      return tf_inump(sys_h);
}

/*
 * $Log: nump.c,v $
 * Revision 1.3  2003/05/28 03:38:05  steve
 *  Implement tf_inump
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/05/30 02:12:17  steve
 *  Add tf_nump from mruff.
 *
 */
