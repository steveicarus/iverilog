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
#ident "$Id: verilog.c,v 1.14 2000/10/25 05:41:55 steve Exp $"
#endif

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

# include  <ivl_target.h>
# include  <stdio.h>
# include  <assert.h>

static FILE*out;

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

static void show_assign_lval(ivl_lval_t lval)
{
      ivl_nexus_t nex;
      ivl_nexus_ptr_t ptr;
      ivl_signal_t sig;

      unsigned idx;

      assert(ivl_lval_pins(lval) == 1);
      assert(ivl_lval_mux(lval) == 0);

      nex = ivl_lval_pin(lval, 0);

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ptr = ivl_nexus_ptr(nex, idx);
	    sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    assert(ivl_signal_type(sig) == IVL_SIT_REG);
	    break;
      }

      assert(sig);

      if (ivl_signal_pins(sig) == 1) {
	    fprintf(out, "%s", ivl_signal_name(sig));

      } else {
	    fprintf(out, "%s[%u]", ivl_signal_name(sig),
		    ivl_nexus_ptr_pin(ptr));
      }
}

static void show_assign_lvals(ivl_statement_t net)
{
      const unsigned cnt = ivl_stmt_lvals(net);

      if (cnt == 1) {
	    show_assign_lval(ivl_stmt_lval(net, 0));

      } else {
	    unsigned idx;
	    fprintf(out, "{");
	    show_assign_lval(ivl_stmt_lval(net, 0));
	    for (idx = 1 ;  idx < cnt ;  idx += 1) {
		  fprintf(out, ", ");
		  show_assign_lval(ivl_stmt_lval(net, idx));
	    }
	    fprintf(out, "}");
      }
}

static void show_statement(ivl_statement_t net, unsigned ind)
{
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {
	  case IVL_ST_ASSIGN:
	    fprintf(out, "%*s", ind, "");
	    show_assign_lvals(net);
	    fprintf(out, " = ");
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

static int show_process(ivl_process_t net)
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

int target_design(ivl_design_t des)
{
      const char*path = ivl_design_flag(des, "-o");
      if (path == 0) {
	    return -1;
      }

      out = fopen(path, "w");
      if (out == 0) {
	    perror(path);
	    return -2;
      }

      fprintf(out, "module %s;\n", ivl_scope_name(ivl_design_root(des)));

      ivl_design_process(des, show_process);

      fprintf(out, "endmodule\n");
      fclose(out);

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

#ifdef __CYGWIN32__
#include <cygwin/cygwin_dll.h>
DECLARE_CYGWIN_DLL(DllMain);
#endif

/*
 * $Log: verilog.c,v $
 * Revision 1.14  2000/10/25 05:41:55  steve
 *  Scan the processes, and get the target signals
 *
 * Revision 1.13  2000/10/21 16:49:45  steve
 *  Reduce the target entry points to the target_design.
 *
 * Revision 1.12  2000/10/15 21:02:09  steve
 *  Makefile patches to support target loading under cygwin.
 *
 * Revision 1.11  2000/10/15 04:46:23  steve
 *  Scopes and processes are accessible randomly from
 *  the design, and signals and logic are accessible
 *  from scopes. Remove the target calls that are no
 *  longer needed.
 *
 *  Add the ivl_nexus_ptr_t and the means to get at
 *  them from nexus objects.
 *
 *  Give names to methods that manipulate the ivl_design_t
 *  type more consistent names.
 */

