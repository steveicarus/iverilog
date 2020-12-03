/*
 * Copyright (c) 2003-2020 Stephen Williams (steve@icarus.com)
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

# include  "priv.h"
# include  <string.h>
# include  <assert.h>

vpiHandle cur_instance = 0;

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
