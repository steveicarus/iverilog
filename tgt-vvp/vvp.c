/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#ident "$Id: vvp.c,v 1.2 2001/03/21 01:49:43 steve Exp $"
#endif

/*
 */

# include  "vvp_priv.h"
# include  <assert.h>

FILE*vvp_out = 0;

int target_design(ivl_design_t des)
{
      ivl_scope_t root;
      const char*path = ivl_design_flag(des, "-o");
      assert(path);

      vvp_out = fopen(path, "w");
      if (vvp_out == 0) {
	    perror(path);
	    return -1;
      }

      root = ivl_design_root(des);
      draw_scope(root, 0);

      ivl_design_process(des, draw_process, 0);

      fclose(vvp_out);

      return 0;
}

#ifdef __CYGWIN32__
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

/*
 */

