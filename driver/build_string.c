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
#ifdef HAVE_CVS_IDENT
#ident "$Id: build_string.c,v 1.15 2003/11/13 04:09:49 steve Exp $"
#endif

# include "config.h"

# include  "globals.h"
# include  <string.h>
# include  <assert.h>

int build_string(char*output, size_t olen, const char*pattern)
{
      char tmp_buf[1024];

      char*output_save = output;
      while (*pattern) {

	    if (*pattern == '%') {
		  pattern += 1;
		  switch (*pattern) {
		      case 0:
			break;

		      case '%':
			*output++ = '%';
			break;

		      case '[': {
			    const char*tail;
			    pattern += 1;
			    assert(*pattern);
			    tail = strchr(pattern+1, ']');
			    assert(tail);
			    strncpy(tmp_buf, pattern+1, tail-pattern-1);
			    tmp_buf[tail-pattern-1] = 0;

			    if (((*pattern == 'v') && verbose_flag)
				|| ((*pattern == 'N') && npath)) {
				  int rc = build_string(output, olen,
							tmp_buf);
				  output += rc;
				  olen -= rc;
			    }
			    pattern = tail;
			    break;
		      }

		      case 'B':
			strcpy(output, base);
			output += strlen(base);
			olen -= strlen(base);
			break;

		      case 'C':
			strcpy(output, iconfig_path);
			output += strlen(iconfig_path);
			olen -= strlen(iconfig_path);
			break;

		      case 'c':
			strcpy(output, iconfig_common_path);
			output += strlen(iconfig_common_path);
			olen -= strlen(iconfig_common_path);
			break;

		      case 'N':
			if (npath) {
			      strcpy(output, npath);
			      output += strlen(npath);
			      olen -= strlen(npath);
			}
			break;

		  }
		  pattern += 1;

	    } else {
		  *output++ = *pattern++;
		  olen -= 1;
	    }
      }

      *output = 0;
      return output-output_save;
}

/*
 * $Log: build_string.c,v $
 * Revision 1.15  2003/11/13 04:09:49  steve
 *  Pass flags through the temporary config file.
 *
 * Revision 1.14  2003/11/01 04:21:57  steve
 *  Add support for a target static config file.
 *
 * Revision 1.13  2003/09/23 05:57:15  steve
 *  Pass -m flag from driver via iconfig file.
 *
 * Revision 1.12  2003/09/22 01:12:09  steve
 *  Pass more ivl arguments through the iconfig file.
 *
 * Revision 1.11  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 */

