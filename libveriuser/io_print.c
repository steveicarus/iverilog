/*
 * Copyright (c) 2002-2014 Michael Ruff (mruff at chiaro.com)
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

# include  <vpi_user.h>
# include  <veriuser.h>

/*
 * io_printf implemented using VPI interface
 */
void io_printf(const char *fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);
      vpi_vprintf(fmt, ap);
      va_end(ap);
}

void tf_warning(const char *fmt, ...)
{
      va_list ap;

      vpi_printf("warning! ");

      va_start(ap, fmt);
      vpi_vprintf(fmt, ap);
      va_end(ap);
}

void tf_error(const char *fmt, ...)
{
      va_list ap;

      vpi_printf("error! ");

      va_start(ap, fmt);
      vpi_vprintf(fmt, ap);
      va_end(ap);
}

PLI_INT32 tf_message(PLI_INT32 level, char*facility,
		     char*messno, char*fmt, ...)
{
      va_list ap;

      (void)level; /* Parameter is not used. */

      vpi_printf("%s[%s] ", facility, messno);

      va_start(ap, fmt);
      vpi_vprintf(fmt, ap);
      va_end(ap);

      vpi_printf("\n");
      return 0;
}
