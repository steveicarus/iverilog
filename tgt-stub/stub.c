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
#ident "$Id: stub.c,v 1.2 2000/08/14 04:39:57 steve Exp $"
#endif

# include  <ivl_target.h>
# include  <stdio.h>

static FILE*out;

int target_start_design(ivl_design_t des)
{
      const char*path = ivl_get_flag(des, "-o");
      if (path == 0) {
	    return -1;
      }

      out = fopen(path, "w");
      if (out == 0) {
	    perror(path);
	    return -2;
      }

      fprintf(out, "STUB: start_design\n");
      return 0;
}

void target_end_design(ivl_design_t des)
{
      fprintf(out, "STUB: end_design\n");
      fclose(out);
}

int target_net_bufz(const char*name, ivl_net_bufz_t net)
{
      fprintf(out, "STUB: %s: BUFZ\n", name);
      return 0;
}

int target_net_const(const char*name, ivl_net_const_t net)
{
      fprintf(out, "STUB: %s: constant\n", name);
      return 0;
}

int target_process(ivl_process_t net)
{
      fprintf(out, "STUB: process\n");
      return 0;
}

/*
 * $Log: stub.c,v $
 * Revision 1.2  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */

