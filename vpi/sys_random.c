/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: sys_random.c,v 1.1 2000/05/04 03:37:59 steve Exp $"
#endif

# include  <vpi_user.h>
# include  <assert.h>
# include  <stdlib.h>

/*
 * Implement the $random system function. For now, ignore any
 * parameters and only produce a random number.
 */
static int sys_random_calltf(char*name)
{
      s_vpi_value val;
      vpiHandle call_handle;

      call_handle = vpi_handle(vpiSysTfCall, 0);
      assert(call_handle);

      val.format = vpiIntVal;
      val.value.integer = random();

      vpi_put_value(call_handle, &val, 0, vpiNoDelay);

      return 0;
}

static int sys_random_sizetf(char*x)
{
      return 32;
}

void sys_random_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type   = vpiSysFunc;
      tf_data.tfname = "$random";
      tf_data.calltf = sys_random_calltf;
      tf_data.compiletf = 0;
      tf_data.sizetf = sys_random_sizetf;
      tf_data.user_data = "$random";
      vpi_register_systf(&tf_data);
}

/*
 * $Log: sys_random.c,v $
 * Revision 1.1  2000/05/04 03:37:59  steve
 *  Add infrastructure for system functions, move
 *  $time to that structure and add $random.
 *
 */

