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
#ident "$Id: priv.c,v 1.3 2003/05/18 00:16:35 steve Exp $"
#endif

# include  "priv.h"
# include  <string.h>
# include  <assert.h>

FILE* pli_trace = 0;

static char string_buffer[8192];
static unsigned string_fill = 0;

static void buffer_reset(void)
{
      string_fill = 0;
}

char* __acc_newstring(const char*txt)
{
      char*res;
      unsigned len;

      if (txt == 0)
	    return 0;

      len = strlen(txt);
      assert(len < sizeof string_buffer);

      if ((string_fill + len + 1) >= sizeof string_buffer)
	    buffer_reset();

      res = string_buffer + string_fill;
      strcpy(string_buffer + string_fill, txt);

      string_fill += len + 1;

      return res;
}

/*
 * $Log: priv.c,v $
 * Revision 1.3  2003/05/18 00:16:35  steve
 *  Add PLI_TRACE tracing of PLI1 modules.
 *
 *  Add tf_isetdelay and friends, and add
 *  callback return values for acc_vcl support.
 *
 * Revision 1.2  2003/03/13 05:07:46  steve
 *  Declaration warnings.
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

