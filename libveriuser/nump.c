/*
 * Copyright (c) 2002-2020 Michael Ruff (mruff at chiaro.com)
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
#include  <stdio.h>
#include "veriuser.h"
#include  "priv.h"

/*
 * tf_nump implemented using VPI interface
 */

int tf_inump(void *obj)
{
      vpiHandle sys_h, sys_i;
      int cnt;

      sys_h = (vpiHandle)obj;
      sys_i = vpi_iterate(vpiArgument, sys_h);

        /* count number of args */
      for (cnt = 0; sys_i && vpi_scan(sys_i); cnt++);

      return cnt;
}

int tf_nump(void)
{
      return tf_inump(cur_instance);
}
