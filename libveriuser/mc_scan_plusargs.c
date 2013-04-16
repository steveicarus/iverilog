/*
 * Copyright (c) 2002-2013 Michael Ruff (mruff at chiaro.com)
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

# include  <string.h>
# include  <vpi_user.h>
# include  <veriuser.h>

/*
 * mc_scan_plusargs implemented using VPI interface
 */
char *mc_scan_plusargs(char *plusarg)
{
      int argc, diff;
      char **argv;
      s_vpi_vlog_info vpi_vlog_info;

      /* get command line */
      if (! vpi_get_vlog_info(&vpi_vlog_info))
	    return (char *)0;

      /* for each argument */
      argv = vpi_vlog_info.argv;
      for (argc = 0; argc < vpi_vlog_info.argc; argc++, argv++) {
	    char *a, *p;
	    a = *argv;
	    p = plusarg;

	    /* only plusargs */
	    if (*a != '+') continue;
	    a += 1;

	    /* impossible matches */
	    if (strlen(a) < strlen(p)) continue;

	    diff = 0;
	    while (*p) {

		if (*a != *p) {
		    diff = 1;
		    break;
		}
		a++; p++;
	    }

	    if (!diff) return a;
      }

      /* didn't find it yet */
      return (char *)0;
}
