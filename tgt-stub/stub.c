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
#ident "$Id: stub.c,v 1.4 2000/08/20 04:13:57 steve Exp $"
#endif

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

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

int target_net_event(const char*name, ivl_net_event_t net)
{
      fprintf(out, "STUB: %s: event\n", name);
      return 0;
}

int target_net_logic(const char*name, ivl_net_logic_t net)
{
      switch (ivl_get_logic_type(net)) {
	  case IVL_AND:
	    fprintf(out, "STUB: %s: AND gate\n", name);
	    break;
	  case IVL_OR:
	    fprintf(out, "STUB: %s: OR gate\n", name);
	    break;
	  default:
	    fprintf(out, "STUB: %s: unsupported gate\n", name);
	    return -1;
      }

      return 0;
}

int target_net_probe(const char*name, ivl_net_probe_t net)
{
      fprintf(out, "STUB: %s: probe\n", name);
      return 0;
}
int target_process(ivl_process_t net)
{
      fprintf(out, "STUB: process\n");
      return 0;
}

/*
 * $Log: stub.c,v $
 * Revision 1.4  2000/08/20 04:13:57  steve
 *  Add ivl_target support for logic gates, and
 *  make the interface more accessible.
 *
 * Revision 1.3  2000/08/19 18:12:42  steve
 *  Add target calls for scope, events and logic.
 *
 * Revision 1.2  2000/08/14 04:39:57  steve
 *  add th t-dll functions for net_const, net_bufz and processes.
 *
 * Revision 1.1  2000/08/12 16:34:37  steve
 *  Start stub for loadable targets.
 *
 */

