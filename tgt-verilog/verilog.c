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
#ident "$Id: verilog.c,v 1.10 2000/10/08 04:01:55 steve Exp $"
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

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    fprintf(out, "      and %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUF:
	    fprintf(out, "      buf %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_OR:
	    fprintf(out, "      or %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_XOR:
	    fprintf(out, "      xor %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  default:
	    fprintf(out, "STUB: %s: unsupported gate\n", name);
	    return -1;
      }

      npins = ivl_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1)
	    fprintf(out, ", %s", ivl_nexus_name(ivl_logic_pin(net,idx)));

      fprintf(out, ");\n");

      return 0;
}

int target_net_probe(const char*name, ivl_net_probe_t net)
{
      fprintf(out, "STUB: %s: probe\n", name);
      return 0;
}

int target_net_signal(const char*name, ivl_signal_t net)
{
      unsigned cnt = ivl_signal_pins(net);

      switch (ivl_signal_type(net)) {

	  case IVL_SIT_REG:
	    fprintf(out, "      reg [%u:0] %s; // %s\n", cnt-1,
		    ivl_signal_basename(net), name);
	    break;

	  case IVL_SIT_WIRE:
	    fprintf(out, "      wire [%u:0] %s; // %s\n", cnt-1,
		    ivl_signal_basename(net), name);
	    break;

	  default:
	    fprintf(out, "      <huh!?> [%u:0] %s;\n", cnt-1, name);
	    break;
      }

      return 0;
}

static void show_expression(ivl_expr_t net)
{
      if (net == 0)
	    return;

      switch (ivl_expr_type(net)) {

	  case IVL_EX_BINARY: {
		char code = ivl_expr_opcode(net);
		show_expression(ivl_expr_oper1(net));
		switch (code) {
		    case 'e':
		      fprintf(out, "==");
		      break;
		    case 'n':
		      fprintf(out, "!=");
		      break;
		    default:
		      fprintf(out, "%c", code);
		}
		show_expression(ivl_expr_oper2(net));
		break;
	  }

	  case IVL_EX_NUMBER: {
		int sigflag     = ivl_expr_signed(net);
		unsigned idx, width  = ivl_expr_width(net);
		const char*bits = ivl_expr_bits(net);

		fprintf(out, "%u'%sb", width, sigflag? "s" : "");
		for (idx = width ;  idx > 0 ;  idx -= 1)
		      fprintf(out, "%c", bits[idx-1]);
		break;
	  }

	  case IVL_EX_SFUNC:
	    fprintf(out, "%s", ivl_expr_name(net));
	    break;

	  case IVL_EX_STRING:
	    fprintf(out, "\"%s\"", ivl_expr_string(net));
	    break;

	  case IVL_EX_SIGNAL:
	    fprintf(out, "%s", ivl_expr_name(net));
	    break;

	  default:
	    fprintf(out, "...");
      }
}

static void show_statement(ivl_statement_t net, unsigned ind)
{
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {
	  case IVL_ST_ASSIGN:
	    fprintf(out, "%*s? = ", ind, "");
	    show_expression(ivl_stmt_rval(net));
	    fprintf(out, ";\n");
	    break;

	  case IVL_ST_BLOCK: {
		unsigned cnt = ivl_stmt_block_count(net);
		unsigned idx;
		fprintf(out, "%*sbegin\n", ind, "");
		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      ivl_statement_t cur = ivl_stmt_block_stmt(net, idx);
		      show_statement(cur, ind+4);
		}
		fprintf(out, "%*send\n", ind, "");
		break;
	  }

	  case IVL_ST_CONDIT: {
		ivl_statement_t t = ivl_stmt_cond_true(net);
		ivl_statement_t f = ivl_stmt_cond_false(net);

		fprintf(out, "%*sif (", ind, "");
		show_expression(ivl_stmt_cond_expr(net));
		fprintf(out, ")\n");

		if (t)
		      show_statement(t, ind+4);
		else
		      fprintf(out, "%*s;\n", ind+4, "");

		if (f) {
		      fprintf(out, "%*selse\n", ind, "");
		      show_statement(f, ind+4);
		}

		break;
	  }

	  case IVL_ST_DELAY:
	    fprintf(out, "%*s#%lu\n", ind, "", ivl_stmt_delay_val(net));
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  case IVL_ST_NOOP:
	    fprintf(out, "%*s/* noop */;\n", ind, "");
	    break;

	  case IVL_ST_STASK:
	    if (ivl_stmt_parm_count(net) == 0) {
		  fprintf(out, "%*s%s;\n", ind, "", ivl_stmt_name(net));

	    } else {
		  unsigned idx;
		  fprintf(out, "%*s%s(", ind, "", ivl_stmt_name(net));
		  show_expression(ivl_stmt_parm(net, 0));
		  for (idx = 1 ;  idx < ivl_stmt_parm_count(net) ; idx += 1) {
			fprintf(out, ", ");
			show_expression(ivl_stmt_parm(net, idx));
		  }
		  fprintf(out, ");\n");
	    }
	    break;

	  case IVL_ST_WAIT:
	    fprintf(out, "%*s@(...)\n", ind, "");
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  case IVL_ST_WHILE:
	    fprintf(out, "%*swhile (<?>)\n", ind, "");
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  default:
	    fprintf(out, "%*sunknown statement type (%u)\n", ind, "", code);
      }
}

int target_process(ivl_process_t net)
{
      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    fprintf(out, "      initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	    fprintf(out, "      always\n");
	    break;
      }

      show_statement(ivl_process_stmt(net), 8);

      return 0;
}

/*
 * $Log: verilog.c,v $
 * Revision 1.10  2000/10/08 04:01:55  steve
 *  Back pointers in the nexus objects into the devices
 *  that point to it.
 *
 *  Collect threads into a list in the design.
 *
 * Revision 1.9  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.8  2000/10/06 23:46:51  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.7  2000/10/05 05:03:02  steve
 *  xor and constant devices.
 *
 * Revision 1.6  2000/10/04 02:24:20  steve
 *  print reg signals.
 *
 * Revision 1.5  2000/09/30 02:18:15  steve
 *  ivl_expr_t support for binary operators,
 *  Create a proper ivl_scope_t object.
 *
 * Revision 1.4  2000/09/26 00:30:07  steve
 *  Add EX_NUMBER and ST_TRIGGER to dll-api.
 *
 * Revision 1.3  2000/09/24 15:46:00  steve
 *  API access to signal type and port type.
 *
 * Revision 1.2  2000/09/24 02:21:54  steve
 *  Add support for signal expressions.
 *
 * Revision 1.1  2000/09/23 05:15:07  steve
 *  Add enough tgt-verilog code to support hello world.
 *
 * Revision 1.9  2000/09/22 03:58:30  steve
 *  Access to the name of a system task call.
 *
 * Revision 1.8  2000/09/19 04:15:27  steve
 *  Introduce the means to get statement types.
 *
 * Revision 1.7  2000/09/18 01:24:32  steve
 *  Get the structure for ivl_statement_t worked out.
 *
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

