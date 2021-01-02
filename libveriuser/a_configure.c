/*
 * Copyright (c) 2003-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include  <acc_user.h>
#include  <vpi_user.h>
#include  "priv.h"
#include  <string.h>

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
	    }
	    break;

	  case accEnableArgs:

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_configure(accEnableArgs, %s)\n",
			  value);
	    }

	    rc = 0;
	    if (strcmp(value,"acc_set_scope") == 0) {
		  vpi_printf("XXXX acc_configure argument: Sorry: "
			     "(accEnableArgs, %s\n", value);

	    } else if (strcmp(value,"no_acc_set_scope") == 0) {
		  vpi_printf("XXXX acc_configure argument: Sorry: "
			     "(accEnableArgs, %s\n", value);

	    } else {
		  vpi_printf("XXXX acc_configure argument error. "
			     "(accEnableArgs, %s(invalid)\n", value);
	    }

	    break;

	  default:

	    if (pli_trace) {
		  fprintf(pli_trace, "acc_configure(config=%d, %s)\n",
			  (int)config_param, value);
	    }
#if 0
	    vpi_printf("XXXX acc_configure(%d, %s)\n", (int)config_param,
	               value);
#else
	      /* Parameter is not necessarily a string. */
	    vpi_printf("XXXX acc_configure(%d, ...)\n", (int)config_param);
#endif
	    rc = 0;
	    break;
      }

      return rc;
}
