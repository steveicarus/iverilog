/*
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
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

#include  <vpi_user.h>
#include  <acc_user.h>
#include  "priv.h"

/*
 * acc_fetch_fullname implemented using VPI interface
 */
char *acc_fetch_fullname(handle object)
{
      return __acc_newstring(vpi_get_str(vpiFullName, object));
}

char* acc_fetch_name(handle object)
{
      return __acc_newstring(vpi_get_str(vpiName, object));
}

char* acc_fetch_defname(handle object)
{
      return __acc_newstring(vpi_get_str(vpiDefName, object));
}
