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
#ident "$Id: stub.c,v 1.6 2000/08/27 15:51:51 steve Exp $"
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

      fprintf(out, "module %s;\n", ivl_get_root_name(des));
      return 0;
}

void target_end_design(ivl_design_t des)
{
      fprintf(out, "endmodule\n");
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
      unsigned npins, idx;

      switch (ivl_get_logic_type(net)) {
	  case IVL_AND:
	    fprintf(out, "      and %s (%s", name,
		    ivl_get_nexus_name(ivl_get_logic_pin(net, 0)));
	    break;
	  case IVL_OR:
	    fprintf(out, "      or %s (%s", name,
		    ivl_get_nexus_name(ivl_get_logic_pin(net, 0)));
	    break;
	  default:
	    fprintf(out, "STUB: %s: unsupported gate\n", name);
	    return -1;
      }

      npins = ivl_get_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1)
	    fprintf(out, ", %s",
		    ivl_get_nexus_name(ivl_get_logic_pin(net,idx)));

      fprintf(out, ");\n");

      return 0;
}

int target_net_probe(const char*name, ivl_net_probe_t net)
{
      fprintf(out, "STUB: %s: probe\n", name);
      return 0;
}

int target_net_signal(const char*name, ivl_net_signal_t net)
{
      fprintf(out, "STUB: %s: signal [%u]\n", name, ivl_get_signal_pins(net));
      return 0;
}

int target_process(ivl_process_t net)
{
      fprintf(out, "STUB: process\n");
      return 0;
}

/*
 * $Log: stub.c,v $
 * Revision 1.6  2000/08/27 15:51:51  steve
 *  t-dll iterates signals, and passes them to the
 *  target module.
 *
 *  Some of NetObj should return char*, not string.
 *
 * Revision 1.5  2000/08/26 00:54:03  steve
 *  Get at gate information for ivl_target interface.
 *
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

