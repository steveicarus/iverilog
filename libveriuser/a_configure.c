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
#ident "$Id: a_configure.c,v 1.2 2003/05/18 00:16:35 steve Exp $"
#endif

#include  <acc_user.h>
#include  <vpi_user.h>
#include  "priv.h"

int acc_configure(PLI_INT32 config_param, const char*value)
{
      int rc;
      switch (config_param) {
	  case accDevelopmentVersion:
	    vpi_printf("Request PLI Development Version %s\n", value);
	    rc = 1;

	    if (pli_trace) {
		  fprintf(pli_trace,
			  "acc_configure(accDevelopmentVersion, %s)\n",
			  value);
		  fflush(pli_trace);
	    }
	    break;

	  default:

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_configure(config=%d, %s)\n",
			  config_param, value);
		  fflush(pli_trace);
	    }

	    vpi_printf("XXXX acc_configure(%d, %s)\n", config_param, value);
	    rc = 0;
	    break;
      }

      return rc;
}

/*
 * $Log: a_configure.c,v $
 * Revision 1.2  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.1  2003/02/17 06:39:47  steve
 *  Add at least minimal implementations for several
 *  acc_ functions. Add support for standard ACC
 *  string handling.
 *
 *  Add the _pli_types.h header file to carry the
 *  IEEE1364-2001 standard PLI type declarations.
 *
 */

