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
#ident "$Id: putlongp.c,v 1.4 2004/09/10 23:13:05 steve Exp $"
#endif

#include  <stdio.h>
#include  <assert.h>
#include  <veriuser.h>
#include  <vpi_user.h>

/*
 * tf_putlongp implemented using VPI interface
 */
void tf_putlongp(int n, int lowvalue, int highvalue)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value val;
      int type;
      char str[20];


      assert(n >= 0);

      /* get task/func handle */
      sys_h = vpi_handle(vpiSysTfCall, 0);
      sys_i = vpi_iterate(vpiArgument, sys_h);

      type = vpi_get(vpiType, sys_h);

      /* verify function */
      assert(!(n == 0 && type != vpiSysFuncCall));

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) assert(0);
	    n--;
      }
      if (!arg_h) arg_h = sys_h;

      /* fill in vpi_value */
      sprintf(str, "%x%08x", highvalue, lowvalue);
      val.format = vpiHexStrVal;
      val.value.str = str;
      vpi_put_value(arg_h, &val, 0, vpiNoDelay);

      vpi_free_object(sys_i);
}

/*
 * $Log: putlongp.c,v $
 * Revision 1.4  2004/09/10 23:13:05  steve
 *  Compile cleanup of C code.
 *
 * Revision 1.3  2003/03/15 05:42:39  steve
 *  free argument iterators.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/07 16:21:13  steve
 *  Add tf_putlongp and tf_putp.
 *
 */
