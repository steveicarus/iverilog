/*
 * Copyright (c) 2002 Stephen Williams (steve@icarus.com)
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
#ident "$Id: substit.c,v 1.5 2003/12/19 01:27:10 steve Exp $"
#endif

# include  <string.h>
# include  <stdlib.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif


char* substitutions(const char*str)
{
      size_t nbuf = strlen(str) + 1;
      char*buf = malloc(nbuf);
      char*cp = buf;

      while (*str) {

	    if ((str[0] == '$') && (str[1] == '(')) {
		    /* If I find a $(x) string in the source, replace
		       it in the destination with the contents of the
		       environment variable x. */
		  char*name;
		  char*value;
		  const char*ep = strchr(str, ')');
		  str += 2;

		  name = malloc(ep-str+1);
		  strncpy(name, str, ep-str);
		  name[ep-str] = 0;

		  str = ep + 1;

		  value = getenv(name);
		  free(name);
		  if (value == 0)
			continue;

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


/*
 * $Log: substit.c,v $
 * Revision 1.5  2003/12/19 01:27:10  steve
 *  Fix various unsigned compare warnings.
 *
 * Revision 1.4  2002/08/12 01:35:01  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.3  2002/08/11 23:47:04  steve
 *  Add missing Log and Ident strings.
 *
 * Revision 1.2  2002/06/25 01:33:01  steve
 *  include malloc.h only when available.
 *
 * Revision 1.1  2002/06/23 20:10:51  steve
 *  Variable substitution in command files.
 *
 */

