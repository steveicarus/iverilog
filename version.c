/*
 * Copyright (c) 2009-2015 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "version_tag.h"
# include  <stdio.h>
# include  <string.h>

static void run_string(const char*txt)
{
      const char*cp = txt;
      while (*cp) {
	    if (cp[0] == '%' && cp[1] != 0) {
		  switch (cp[1]) {
		      case 'M':
			fprintf(stdout, "%d", VERSION_MAJOR);
			break;
		      case 'n':
			fprintf(stdout, "%d", VERSION_MINOR);
			break;
		      case 'E':
			fprintf(stdout, "%s", VERSION_EXTRA);
			break;
		      case 'T':
			fprintf(stdout, "%s", VERSION_TAG);
			break;
		      case '%':
			putc('%', stdout);
			break;
		      default:
			break;
		  }
		  cp += 2;

	    } else if (cp[0] == '\\' && cp[1] != 0) {
		  switch (cp[1]) {
		      case 'n':
			putc('\n', stdout);
			break;
		      default:
			putc(cp[1], stdout);
			break;
		  }
		  cp += 2;

	    } else {
		  putc(cp[0], stdout);
		  cp += 1;
	    }
      }
}

int main(int argc, char*argv[])
{
      int idx;

      if (argc == 1) {
	    printf("%s\n", VERSION);
	    return 0;
      }

      run_string(argv[1]);
      for (idx = 2 ; idx < argc ; idx += 1) {
	    printf(" ");
	    run_string(argv[idx]);
      }

      return 0;
}
