/*
 * Copyright (c) 2002-2010 Stephen Williams (steve@icarus.com)
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
# include  <stdlib.h>
# include  <stdio.h>
# include  "ivl_alloc.h"
# include  "globals.h"


char* substitutions(const char*str)
{
      size_t nbuf = strlen(str) + 1;
      char*buf = malloc(nbuf);
      char*cp = buf;

      while (*str) {

	    if ((str[0] == '$') && ((str[1] == '(') || str[1] == '{')) {
		    /* If I find a $(x) or ${x} string in the source, replace
		       it in the destination with the contents of the
		       environment variable x. */
		  char*name;
		  char*value;
		  const char*ep = strchr(str, (str[1]=='(') ? ')' : '}');
		  str += 2;

		  name = malloc(ep-str+1);
		  strncpy(name, str, ep-str);
		  name[ep-str] = 0;

		  str = ep + 1;

		  value = getenv(name);
		  if (value == 0) {
			fprintf(stderr, "Warning: environment variable "
			        "\"%s\" not found during command file "
			        "processing.\n", name);
			free(name);
			continue;
		  }
		  free(name);

		  if (strlen(value) >= (nbuf - (cp-buf))) {
			size_t old_size = cp - buf;
			nbuf = (cp - buf) + strlen(value) + 1;
			buf = realloc(buf, nbuf);
			cp = buf + old_size;
		  }

		  strcpy(cp, value);
		  cp += strlen(cp);

	    } else {
		  if ( cp == (buf + nbuf) ) {
			size_t old_size = nbuf;
			nbuf = old_size + 32;
			buf = realloc(buf, nbuf);
			cp = buf + old_size;
		  }

		  *cp++ = *str++;
	    }
      }

	/* Add the trailing nul to the string, and reallocate the
	   buffer to be a tight fit. */
      if ( cp == (buf + nbuf) ) {
	    size_t old_size = nbuf;
	    nbuf = old_size + 1;
	    buf = realloc(buf, nbuf);
	    buf[old_size] = 0;
      } else {
	    *cp++ = 0;
	    nbuf = cp - buf;
	    buf = realloc(buf, nbuf);
      }

      return buf;
}
