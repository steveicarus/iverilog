/*
 * Copyright (c) 2000-2017 Stephen Williams (steve@icarus.com)
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

# include "config.h"

/*
 * This target writes a Verilog description of the design. The output
 * Verilog is a single module that has the name of the root module of
 * the design, but is internally the complete design.
 */

# include  "ivl_target.h"
# include  <stdio.h>
# include  <assert.h>

/* This is the output file where the Verilog program is sent. */
static FILE*out;

/*
 * Scoped objects are the signals, reg and wire and what-not. What
 * this function does is draw the objects of the scope, along with a
 * fake scope context so that the hierarchical name remains
 * pertinent.
 */
static void draw_scoped_objects(ivl_design_t des)
{
      ivl_scope_t root = ivl_design_root(des);
      unsigned cnt, idx;

      cnt = ivl_scope_sigs(root);
      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(root, idx);

	    switch (ivl_signal_type(sig)) {
		case IVL_SIT_REG:
		  if (ivl_signal_pins(sig) > 1)
			fprintf(out, "    reg [%u:0] %s;\n",
				ivl_signal_pins(sig),
				ivl_signal_basename(sig));
		  else
			fprintf(out, "    reg %s;\n",
				ivl_signal_basename(sig));
		  break;
		case IVL_SIT_TRI:
		  fprintf(out, "    wire %s;\n", ivl_signal_basename(sig));
		  break;
		default:
		  assert(0);
	    }
      }

}

/*
 * Given a nexus, this function draws a signal reference. We don't
 * care really whether the signal is a reg or wire, because this may
 * be an input or output of a gate. Just print it. And if this is a
 * bit of a vector, draw the bit select needed to get at the right bit.
 */
static void draw_nexus(ivl_nexus_t nex)
{
      ivl_signal_t sig=NULL;
      ivl_nexus_ptr_t ptr=NULL;
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ptr = ivl_nexus_ptr(nex, idx);
	    sig = ivl_nexus_ptr_sig(ptr);
	    if (sig)
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

/*
 * Draw a single logic gate. Escape the name so that it is preserved
 * completely. This drawing is happening in the root scope so signal
 * references can remain hierarchical.
 */
static int draw_logic(ivl_net_logic_t net)
{
      unsigned npins, idx;
      const char*name = ivl_logic_name(net);

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    fprintf(out, "    and \\%s (", name);
	    break;
	  case IVL_LO_BUF:
	    fprintf(out, "    buf \\%s (", name);
	    break;
	  case IVL_LO_OR:
	    fprintf(out, "    or \\%s (", name);
	    break;
	  case IVL_LO_XOR:
	    fprintf(out, "    xor \\%s (", name);
	    break;
	  default:
	    fprintf(out, "STUB: %s: unsupported gate\n", name);
	    return -1;
      }

      draw_nexus(ivl_logic_pin(net, 0));

      npins = ivl_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1) {
	    fprintf(out, ", ");
	    draw_nexus(ivl_logic_pin(net,idx));
      }

      fprintf(out, ");\n");
      return 0;
}

/*
 * Scan the scope and its children for logic gates. Use the draw_logic
 * function to draw the actual gate.
 */
static int draw_scope_logic(ivl_scope_t scope, void*x)
{
      unsigned cnt = ivl_scope_logs(scope);
      unsigned idx;

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    draw_logic(ivl_scope_log(scope, idx));
      }

      ivl_scope_children(scope, draw_scope_logic, 0);
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
		    case 'N':
		      fprintf(out, "!==");
		      break;
		    case 'r':
		      fprintf(out, ">>");
		      break;
		    default:
		      fprintf(out, "%c", code);
		}
		show_expression(ivl_expr_oper2(net));
		break;
	  }

	  case IVL_EX_CONCAT: {
		unsigned idx;
		fprintf(out, "{");
		show_expression(ivl_expr_parm(net, 0));
		for (idx = 1 ;  idx < ivl_expr_parms(net) ;  idx += 1) {
		      fprintf(out, ", ");
		      show_expression(ivl_expr_parm(net, idx));
		}
		fprintf(out, "}");
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

/*
 * An assignment is one of a possible list of l-values to a behavioral
 * assignment. Each l-value is either a part select of a signal or a
 * non-constant bit select.
 */
static void show_assign_lval(ivl_lval_t lval)
{
      ivl_nexus_t nex;
      ivl_signal_t sig=NULL;

      unsigned idx;
      unsigned lsb=0;

      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_mem(lval) == 0);

      nex = ivl_lval_pin(lval, 0);

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    unsigned pin;
	    ivl_nexus_ptr_t ptr;

	    ptr = ivl_nexus_ptr(nex, idx);
	    sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    lsb = ivl_nexus_ptr_pin(ptr);

	    for (pin = 1 ;  pin < ivl_lval_pins(lval) ;  pin += 1) {
		  if (ivl_signal_pin(sig, lsb+pin) != ivl_lval_pin(lval,pin))
			break;
	    }

	    if (pin < ivl_lval_pins(lval))
		  continue;

	    break;
      }

      assert(sig);

      if ((lsb > 0) || (lsb + ivl_lval_pins(lval)) < ivl_signal_pins(sig)) {
	    fprintf(out, "%s[%u:%u]", ivl_signal_name(sig),
		    lsb+ivl_lval_pins(lval)-1, lsb);

      } else {
	    fprintf(out, "%s", ivl_signal_name(sig));

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
	    fprintf(out, "%*sunknown statement type (%d)\n", ind, "", code);
      }
}

/*
 * Processes are all collected by ivl and I draw them here in the root
 * scope. This way, I don't need to do anything about scope
 * references.
 */
static int show_process(ivl_process_t net, void*x)
{
      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    fprintf(out, "    initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	  case IVL_PR_ALWAYS_COMB:
	  case IVL_PR_ALWAYS_FF:
	  case IVL_PR_ALWAYS_LATCH:
	    fprintf(out, "    always\n");
	    break;
	  case IVL_PR_FINAL:
	    fprintf(out, "    final\n");
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

	/* Declare all the signals. */
      draw_scoped_objects(des);

	/* Declare logic gates. */
      draw_scope_logic(ivl_design_root(des), 0);

	/* Write out processes. */
      ivl_design_process(des, show_process, 0);

      fprintf(out, "endmodule\n");
      fclose(out);

      return 0;
}
