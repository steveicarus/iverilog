/*
 * Copyright (c) 2003-2014 Stephen Williams (steve@icarus.com)
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

#include  <assert.h>
#include  <vpi_user.h>
#include  <acc_user.h>

const char* acc_fetch_type_str(PLI_INT32 type)
{
      switch (type) {
	  case accNet:
	    return "accNet";

	  case accReg:
	    return "accReg";

	  case accParameter:
	    return "accParameter";

	  case accConstant:
	    return "accConstant";
      }

      vpi_printf("acc_fetch_type_str: type %d is what accType?\n", (int)type);
      return "acc_fetch_type_str(unknown)";
}

/*
 * FIXME: What does this do? How should it be declared in acc_user.h?
 */
PLI_INT32 acc_fetch_paramtype(handle obj)
{
      (void)obj; /* Parameter is not used. */
      return 0;
}
