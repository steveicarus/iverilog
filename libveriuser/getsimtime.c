/* vi:sw=6
 * Copyright (c) 2002,2003 Michael Ruff (mruff at chiaro.com)
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
#ident "$Id: getsimtime.c,v 1.7 2003/05/28 03:14:20 steve Exp $"
#endif

#include  <veriuser.h>
#include  <vpi_user.h>
#include  <stdlib.h>
#include  <math.h>

/*
 * some TF time routines implemented using VPI interface
 */

static long long
scale(int high, int low, void*obj) {
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));
      long long scaled;

      scaled = high;
      scaled = (scaled << 32) | low;
      scaled *= pow(10, vpi_get(vpiTimePrecision,0) -
			vpi_get(vpiTimeUnit,obj ? (vpiHandle)obj : hand));

      return scaled;
}


PLI_INT32 tf_gettime(void)
{
      s_vpi_time time;
      time.type = vpiSimTime;
      vpi_get_time (0, &time);
      return scale(time.high, time.low, 0) & 0xffffffff;
}

char *tf_strgettime(void)
{
      static char buf[32];
      s_vpi_time time;

      time.type = vpiSimTime;
      vpi_get_time (0, &time);
      if (time.high)
	    snprintf(buf, sizeof(buf)-1, "%u%08u", time.high, time.low);
      else
	    snprintf(buf, sizeof(buf)-1, "%u", time.low);
      return buf;
}

PLI_INT32 tf_getlongtime(PLI_INT32 *high)
{
      s_vpi_time time;
      long long scaled;
      time.type = vpiSimTime;
      vpi_get_time (0, &time);
      scaled = scale(time.high, time.low, 0);

      *high = (scaled >> 32) & 0xffffffff;
      return scaled & 0xffffffff;
}

PLI_INT32 tf_igetlongtime(PLI_INT32 *high, void*obj)
{
      s_vpi_time time;
      long long scaled;
      time.type = vpiSimTime;
      vpi_get_time ((vpiHandle)obj, &time);
      scaled = scale(time.high, time.low, obj);

      *high = (scaled >> 32) & 0xffffffff;
      return scaled & 0xffffffff;
}

/* Alias for commercial simulators */
PLI_INT32 tf_getlongsimtime(PLI_INT32 *high) \
      __attribute__ ((weak, alias ("tf_getlongtime")));


PLI_INT32 tf_gettimeprecision(void)
{
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));
      return vpi_get(vpiTimePrecision, hand);
}

PLI_INT32 tf_igettimeprecision(void*obj)
{
      vpiHandle hand = vpi_handle(vpiScope, (vpiHandle)obj);
      return vpi_get(vpiTimePrecision, hand);
}


PLI_INT32 tf_gettimeunit()
{
      vpiHandle hand = vpi_handle(vpiScope, vpi_handle(vpiSysTfCall,0));
      return vpi_get(vpiTimeUnit, hand);
}

PLI_INT32 tf_igettimeunit(void*obj)
{
      return vpi_get(!obj ? vpiTimePrecision : vpiTimeUnit, (vpiHandle)obj);
}


/*
 * $Log: getsimtime.c,v $
 * Revision 1.7  2003/05/28 03:14:20  steve
 *  Missing time related declarations.
 *
 * Revision 1.6  2003/05/27 16:22:10  steve
 *  PLI get time units/precision.
 *
 * Revision 1.5  2003/04/12 18:57:14  steve
 *  More acc_ function stubs.
 *
 * Revision 1.4  2003/03/13 04:35:09  steve
 *  Add a bunch of new acc_ and tf_ functions.
 *
 * Revision 1.3  2003/03/06 00:27:54  steve
 *  Fill in required fields when getting time.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/05/31 18:25:51  steve
 *  Add tf_getlongtime (mruff)
 *
 */
