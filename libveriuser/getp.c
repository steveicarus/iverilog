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
#ident "$Id: getp.c,v 1.7 2004/10/04 01:10:56 steve Exp $"
#endif

# include  <assert.h>
# include  <ctype.h>
# include  <veriuser.h>
# include  <vpi_user.h>
# include  "priv.h"

/*
 * tf_getp and friends, implemented using VPI interface
 */
PLI_INT32 tf_igetp(PLI_INT32 n, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      int rtn = 0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      /* If it is a constant string, return a pointer to it else int value */
      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    value.format = vpiStringVal;
	    vpi_get_value(arg_h, &value);
	    rtn = (int) value.value.str;	/* Oh my */
      } else {
	    value.format = vpiIntVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.integer;
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_igetp(n=%d, obj=%p) --> %d\n",
		  n, obj, rtn);
      }

      return rtn;
}

PLI_INT32 tf_getp(PLI_INT32 n)
{
      int rtn = tf_igetp(n, vpi_handle(vpiSysTfCall, 0));

      return rtn;
}


double tf_igetrealp(PLI_INT32 n, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      double rtn = 0.0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    rtn = 0.0;
      } else {
	    value.format = vpiRealVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.real;
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_igetrealp(n=%d, obj=%p) --> %f\n",
		  n, obj, rtn);
      }

      return rtn;
}

double tf_getrealp(PLI_INT32 n)
{
      double rtn = tf_igetrealp(n, vpi_handle(vpiSysTfCall, 0));

      return rtn;
}


char *tf_istrgetp(PLI_INT32 n, PLI_INT32 fmt, void *obj)
{
      vpiHandle sys_h, sys_i, arg_h = 0;
      s_vpi_value value;
      char *rtn = 0;

      assert(n > 0);

      /* get task/func handle */
      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

      /* find nth arg */
      while (n > 0) {
	    if (!(arg_h = vpi_scan(sys_i))) { goto out; }
	    n--;
      }

      if (vpi_get(vpiType, arg_h) == vpiConstant &&
	  vpi_get(vpiConstType, arg_h) == vpiStringConst)
      {
	    value.format = vpiStringVal;
	    vpi_get_value(arg_h, &value);
	    rtn = value.value.str;
      } else {
	    value.format = -1;
	    switch (tolower(fmt)) {
		  case 'b': value.format = vpiBinStrVal; break;
		  case 'o': value.format = vpiOctStrVal; break;
		  case 'd': value.format = vpiDecStrVal; break;
		  case 'h': value.format = vpiHexStrVal; break;
	    }
	    if (value.format > 0) {
		  vpi_get_value(arg_h, &value);
		  rtn = value.value.str;
	    }
      }

      vpi_free_object(sys_i);

out:
      if (pli_trace) {
	    fprintf(pli_trace, "tf_istrgetp(n=%d, fmt=%c, obj=%p) --> \"%s\"\n",
		  n, fmt, obj, rtn);
      }

      return rtn;
}

char *tf_strgetp(PLI_INT32 n, PLI_INT32 fmt)
{
      char *rtn = tf_istrgetp(n, fmt, vpi_handle(vpiSysTfCall, 0));

      return rtn;
}

/*
 * $Log: getp.c,v $
 * Revision 1.7  2004/10/04 01:10:56  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.6  2003/06/17 16:55:07  steve
 *  1) setlinebuf() for vpi_trace
 *  2) Addes error checks for trace file opens
 *  3) removes now extraneous flushes
 *  4) fixes acc_next() bug
 *
 * Revision 1.5  2003/05/30 04:22:13  steve
 *  Add tf_strgetp functions.
 *
 * Revision 1.4  2003/05/29 03:46:21  steve
 *  Add tf_getp/putp support for integers
 *  and real valued arguments.
 *
 *  Add tf_mipname function.
 *
 * Revision 1.3  2003/03/15 05:42:39  steve
 *  free argument iterators.
 *
 * Revision 1.2  2002/08/12 01:35:02  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.1  2002/06/07 02:58:59  steve
 *  Add a bunch of acc/tf functions. (mruff)
 *
 */
