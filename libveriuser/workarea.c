/*
 * Copyright (c) 2002-2020 Stephen Williams (steve@icarus.com)
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

# include  <veriuser.h>
# include  <vpi_user.h>
# include  <stdlib.h>
# include  "ivl_alloc.h"
#include  "priv.h"

/*
 * Keep a list of sys handle to work area bindings.
 */
struct workarea_cell {
      vpiHandle sys;
      void* area;
      struct workarea_cell*next;
};

static struct workarea_cell*area_list = 0;

PLI_INT32 tf_setworkarea(void*workarea)
{
      struct workarea_cell*cur;

      cur = area_list;

      while (cur) {
	    if (cur->sys == cur_instance) {
		  cur->area = workarea;
		  return 0;
	    }

	    cur = cur->next;
      }

      cur = calloc(1, sizeof (struct workarea_cell));
      cur->next = area_list;
      cur->sys = cur_instance;
      cur->area = workarea;
      area_list = cur;

      return 0;
}

PLI_BYTE8* tf_getworkarea(void)
{
      struct workarea_cell*cur;

      cur = area_list;

      while (cur) {
	    if (cur->sys == cur_instance) {
		  return cur->area;
	    }

	    cur = cur->next;
      }

      return 0;
}
