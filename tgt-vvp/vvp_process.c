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
#ident "$Id: vvp_process.c,v 1.2 2001/03/20 01:44:14 steve Exp $"
#endif

# include  "vvp_priv.h"

/*
 * This file includes the code needed to generate VVP code for
 * processes. Scopes are already declared, we generate here the
 * executable code for the processes.
 */


static void show_system_task_call(ivl_statement_t net)
{
      unsigned idx;
      unsigned parm_count = ivl_stmt_parm_count(net);

      if (parm_count == 0) {
	    fprintf(vvp_out, "    %%vpi_call \"%s\";\n", ivl_stmt_name(net));
	    return;
      }

      fprintf(vvp_out, "    %%vpi_call \"%s\"", ivl_stmt_name(net));
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_stmt_parm(net, idx);

	    switch (ivl_expr_type(expr)) {

		case IVL_EX_STRING:
		  fprintf(vvp_out, ", \"%s\"", ivl_expr_string(expr));
		  break;

		default:
		  fprintf(vvp_out, ", ?");
		  break;
	    }
      }

      fprintf(vvp_out, ";\n");
}

/*
 * This function draws a statement as vvp assembly. It basically
 * switches on the statement type and draws code based on the type and
 * further specifics.
 */
static void show_statement(ivl_statement_t net)
{
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {

	      /* Begin-end blocks simply draw their contents. */
	  case IVL_ST_BLOCK: {
		unsigned idx;
		unsigned cnt = ivl_stmt_block_count(net);
		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      show_statement(ivl_stmt_block_stmt(net, idx));
		}
		break;
	  }

	  case IVL_ST_STASK:
	    show_system_task_call(net);
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: Unable to draw statement type %u\n",
		    code);
	    break;
      }
}

/*
 * The process as a whole is surrounded by this code. We generate a
 * start label that the .thread statement can use, and we generate
 * code to terminate the thread.
 */

int draw_process(ivl_process_t net, void*x)
{
      static unsigned thread_count = 0;
      ivl_scope_t scope = ivl_process_scope(net);

      fprintf(vvp_out, "    .scope S_%s;\n", ivl_scope_name(scope));

	/* Generate the entry label. Just give the thread a number so
	   that we ar certain the label is unique. */
      fprintf(vvp_out, "T_%05d\n", thread_count);

	/* Draw the contents of the thread. */
      show_statement(ivl_process_stmt(net));


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
      return 0;
}

/*
 * $Log: vvp_process.c,v $
 * Revision 1.2  2001/03/20 01:44:14  steve
 *  Put processes in the proper scope.
 *
 * Revision 1.1  2001/03/19 01:20:46  steve
 *  Add the tgt-vvp code generator target.
 *
 */

