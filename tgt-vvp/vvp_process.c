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
#if !defined(WINNT)
#ident "$Id: vvp_process.c,v 1.8 2001/03/27 03:31:07 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>

static int show_statement(ivl_statement_t net);

static unsigned local_count = 0;
static unsigned thread_count = 0;

/*
 * This file includes the code needed to generate VVP code for
 * processes. Scopes are already declared, we generate here the
 * executable code for the processes.
 */

unsigned bitchar_to_idx(char bit)
{
      switch (bit) {
	  case '0':
	    return 0;
	  case '1':
	    return 1;
	  case 'x':
	    return 2;
	  case 'z':
	    return 3;
	  default:
	    assert(0);
	    return 0;
      }
}

/*
 * These functions handle the blocking assignment. Use the %set
 * instruction to perform the actual assignment, and calculate any
 * lvalues and rvalues that need calculating.
 *
 * The set_to_nexus function takes a particular nexus and generates
 * the %set statements to assign the value.
 *
 * The show_stmt_assign function looks at the assign statement, scans
 * the l-values, and matches bits of the r-value with the correct
 * nexus.
 */

static void set_to_nexus(ivl_nexus_t nex, unsigned bit)
{
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    unsigned pin = ivl_nexus_ptr_pin(ptr);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);

	    if (sig == 0)
		  continue;

	    fprintf(vvp_out, "    %%set V_%s[%u], %u;\n",
		    ivl_signal_name(sig), pin, bit);
      }
}

static int show_stmt_assign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_expr_t rval = ivl_stmt_rval(net);


	/* Handle the special case that the r-value is a constant. We
	   can generate the %set statement directly, without any worry
	   about generating code to evaluate the r-value expressions. */

      if (ivl_expr_type(rval) == IVL_EX_NUMBER) {
	    unsigned idx;
	    const char*bits = ivl_expr_bits(rval);

	      /* XXXX Only single l-value supported for now */
	    assert(ivl_stmt_lvals(net) == 1);

	    lval = ivl_stmt_lval(net, 0);
	      /* XXXX No mux support yet. */
	    assert(ivl_lval_mux(lval) == 0);

	    for (idx = 0 ;  idx < ivl_lval_pins(lval) ;  idx += 1)
		  set_to_nexus(ivl_lval_pin(lval, idx),
			       bitchar_to_idx(bits[idx]));

	    return 0;
      }

      { struct vector_info res = draw_eval_expr(rval);
        unsigned wid = res.wid;
	unsigned idx;

	  /* XXXX Only single l-value supported for now */
	assert(ivl_stmt_lvals(net) == 1);

	lval = ivl_stmt_lval(net, 0);
	  /* XXXX No mux support yet. */
	assert(ivl_lval_mux(lval) == 0);

	if (ivl_lval_pins(lval) < wid)
	      wid = ivl_lval_pins(lval);

	for (idx = 0 ;  idx < wid ;  idx += 1)
	      set_to_nexus(ivl_lval_pin(lval, idx), res.base+idx);

	for (idx = wid ;  idx < ivl_lval_pins(lval) ;  idx += 1)
	      set_to_nexus(ivl_lval_pin(lval, idx), 0);
      }

      return 0;
}

static int show_stmt_condit(ivl_statement_t net)
{
      int rc = 0;
      unsigned lab_false, lab_out;
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(exp);

      assert(cond.wid == 1);

      lab_false = local_count++;
      lab_out = local_count++;

      fprintf(vvp_out, "    %%jmp/0xz  T_%05d.%d, %u;\n",
	      thread_count, lab_false, cond.base);

      rc += show_statement(ivl_stmt_cond_true(net));

      if (ivl_stmt_cond_false(net)) {
	    fprintf(vvp_out, "    %%jmp T_%05d.%d;\n", thread_count, lab_out);
	    fprintf(vvp_out, "T_%05d.%u\n", thread_count, lab_false);

	    rc += show_statement(ivl_stmt_cond_false(net));

	    fprintf(vvp_out, "T_%05d.%u\n", thread_count, lab_out);

      } else {
	    fprintf(vvp_out, "T_%05d.%u\n", thread_count, lab_false);
      }

      return rc;
}

/*
 * The delay statement is easy. Simply write a ``%delay <n>''
 * instruction to delay the thread, then draw the included statement.
 * The delay statement comes from verilog code like this:
 *
 *        ...
 *        #<delay> <stmt>;
 */
static int show_stmt_delay(ivl_statement_t net)
{
      int rc = 0;
      unsigned long delay = ivl_stmt_delay_val(net);
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);

      fprintf(vvp_out, "    %%delay %lu;\n", delay);
      rc += show_statement(stmt);

      return rc;
}

/*
 * noop statements are implemented by doing nothing.
 */
static int show_stmt_noop(ivl_statement_t net)
{
      return 0;
}

static int show_system_task_call(ivl_statement_t net)
{
      unsigned idx;
      unsigned parm_count = ivl_stmt_parm_count(net);

      if (parm_count == 0) {
	    fprintf(vvp_out, "    %%vpi_call \"%s\";\n", ivl_stmt_name(net));
	    return 0;
      }

      fprintf(vvp_out, "    %%vpi_call \"%s\"", ivl_stmt_name(net));
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_stmt_parm(net, idx);

	    switch (ivl_expr_type(expr)) {

		case IVL_EX_SIGNAL:
		  fprintf(vvp_out, ", V_%s", ivl_expr_name(expr));
		  break;

		case IVL_EX_STRING:
		  fprintf(vvp_out, ", \"%s\"", ivl_expr_string(expr));
		  break;

		default:
		  fprintf(vvp_out, ", ?");
		  break;
	    }
      }

      fprintf(vvp_out, ";\n");
      return 0;
}

/*
 * This function draws a statement as vvp assembly. It basically
 * switches on the statement type and draws code based on the type and
 * further specifics.
 */
static int show_statement(ivl_statement_t net)
{
      const ivl_statement_type_t code = ivl_statement_type(net);
      int rc = 0;

      switch (code) {

	  case IVL_ST_ASSIGN:
	    rc += show_stmt_assign(net);
	    break;

	      /* Begin-end blocks simply draw their contents. */
	  case IVL_ST_BLOCK: {
		unsigned idx;
		unsigned cnt = ivl_stmt_block_count(net);
		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      rc += show_statement(ivl_stmt_block_stmt(net, idx));
		}
		break;
	  }

	  case IVL_ST_CONDIT:
	    rc += show_stmt_condit(net);
	    break;

	  case IVL_ST_DELAY:
	    rc += show_stmt_delay(net);
	    break;

	  case IVL_ST_NOOP:
	    rc += show_stmt_noop(net);
	    break;

	  case IVL_ST_STASK:
	    rc += show_system_task_call(net);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: Unable to draw statement type %u\n",
		    code);
	    rc += 1;
	    break;
      }

      return rc;
}

/*
 * The process as a whole is surrounded by this code. We generate a
 * start label that the .thread statement can use, and we generate
 * code to terminate the thread.
 */

int draw_process(ivl_process_t net, void*x)
{
      int rc = 0;
      ivl_scope_t scope = ivl_process_scope(net);

      local_count = 0;
      fprintf(vvp_out, "    .scope S_%s;\n", ivl_scope_name(scope));

	/* Generate the entry label. Just give the thread a number so
	   that we ar certain the label is unique. */
      fprintf(vvp_out, "T_%05d\n", thread_count);

	/* Draw the contents of the thread. */
      rc += show_statement(ivl_process_stmt(net));


	/* Terminate the thread with either an %end instruction (initial
	   statements) or a %jmp back to the beginning of the thread. */

      switch (ivl_process_type(net)) {

	  case IVL_PR_INITIAL:
	    fprintf(vvp_out, "    %%end;\n");
	    break;

	  case IVL_PR_ALWAYS:
	    fprintf(vvp_out, "    %%jmp T_%05d;\n", thread_count);
	    break;
      }

	/* Now write out the .thread directive that tells vvp where
	   the thread starts. */
      fprintf(vvp_out, "    .thread T_%05d;\n", thread_count);


      thread_count += 1;
      return rc;
}

/*
 * $Log: vvp_process.c,v $
 * Revision 1.8  2001/03/27 03:31:07  steve
 *  Support error code from target_t::end_design method.
 *
 * Revision 1.7  2001/03/25 03:53:24  steve
 *  Skip true clause if condition ix 0, x or z
 *
 * Revision 1.6  2001/03/25 03:24:10  steve
 *  Draw signal inputs to system tasks.
 *
 * Revision 1.5  2001/03/23 01:54:32  steve
 *  assignments with non-trival r-values.
 *
 * Revision 1.4  2001/03/22 05:06:21  steve
 *  Geneate code for conditional statements.
 *
 * Revision 1.3  2001/03/21 01:49:43  steve
 *  Scan the scopes of a design, and draw behavioral
 *  blocking  assignments of constants to vectors.
 *
 * Revision 1.2  2001/03/20 01:44:14  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.1  2001/03/19 01:20:46  steve
 *  Add the tgt-vvp code generator target.
 *
 */

