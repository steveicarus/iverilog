/*
 * Copyright (c) 2004-2007 Stephen Williams (steve@icarus.com)
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

# include "config.h"
# include "priv.h"
# include <assert.h>

static unsigned show_assign_lval(ivl_lval_t lval, unsigned ind)
{
      unsigned wid = 0;

      ivl_signal_t sig = ivl_lval_sig(lval);
      assert(sig);

      fprintf(out, "%*s{name=%s width=%u lvwidth=%u}\n",
	      ind, "",
	      ivl_signal_name(sig),
	      ivl_signal_width(sig),
	      ivl_lval_width(lval));

      if (ivl_lval_idx(lval)) {
	    fprintf(out, "%*sAddress-0 select expression:\n", ind+4, "");
	    show_expression(ivl_lval_idx(lval), ind+6);
	    if (ivl_signal_dimensions(sig) < 1) {
		  fprintf(out, "%*sERROR: Address on signal with "
			  "array dimensions=%u\n", ind+4, "",
			  ivl_signal_dimensions(sig));
		  stub_errors += 1;
	    }
      } else if (ivl_signal_array_count(sig) > 1) {
	    fprintf(out, "%*sERROR: Address missing on signal with "
		    "word count=%u\n", ind+4, "",
		    ivl_signal_array_count(sig));
	    stub_errors += 1;
      }

      if (ivl_lval_mux(lval)) {
	    fprintf(out, "%*sBit select expression:\n", ind+4, "");
	    show_expression(ivl_lval_mux(lval), ind+8);
      }
      if (ivl_lval_part_off(lval)) {
	    fprintf(out, "%*sPart select base:\n", ind+4, "");
	    show_expression(ivl_lval_part_off(lval), ind+8);
      }

      wid = ivl_lval_width(lval);

      return wid;
}

static void show_stmt_cassign(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      unsigned lwid = 0;

      fprintf(out, "%*sCONTINUOUS ASSIGN <lwidth=%u>\n", ind, "",
	      ivl_stmt_lwidth(net));

      for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1) {
	    lwid += show_assign_lval(ivl_stmt_lval(net, idx), ind+4);
      }

      fprintf(out, "%*sTotal expected l-value width: %u bits\n",
	      ind+4, "", lwid);

      show_expression(ivl_stmt_rval(net), ind+4);
}

static void show_stmt_delayx(ivl_statement_t net, unsigned ind)
{
      fprintf(out, "%*s#(X) /* calculated delay */\n", ind, "");
      show_expression(ivl_stmt_delay_expr(net), ind+4);
      show_statement(ivl_stmt_sub_stmt(net), ind+2);
}

static void show_stmt_force(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      unsigned lwid = 0;

      fprintf(out, "%*sforce <lwidth=%u>\n", ind, "",
	      ivl_stmt_lwidth(net));

      for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1) {
	    lwid += show_assign_lval(ivl_stmt_lval(net, idx), ind+4);
      }

      fprintf(out, "%*sTotal expected l-value width: %u bits\n",
	      ind+4, "", lwid);

      show_expression(ivl_stmt_rval(net), ind+4);
}

static void show_stmt_release(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      unsigned lwid = 0;

      fprintf(out, "%*srelease <lwidth=%u>\n", ind, "",
	      ivl_stmt_lwidth(net));

      for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1) {
	    lwid += show_assign_lval(ivl_stmt_lval(net, idx), ind+4);
      }

      fprintf(out, "%*sTotal l-value width: %u bits\n",
	      ind+4, "", lwid);
}

/*
 * A trigger statement is the "-> name;" syntax in Verilog, where a
 * trigger signal is sent to a named event. The trigger statement is
 * actually a very simple object.
 */
static void show_stmt_trigger(ivl_statement_t net, unsigned ind)
{
      unsigned cnt = ivl_stmt_nevent(net);
      unsigned idx;

      fprintf(out, "%*s->", ind, "");

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    ivl_event_t event = ivl_stmt_events(net, idx);
	    fprintf(out, " %s", ivl_event_basename(event));
      }

	/* The compiler should make exactly one target event, so if we
	   find more or less, then print some error text. */
      if (cnt != 1) {
	    fprintf(out, " /* ERROR: Expect one target event, got %u */", cnt);
      }

      fprintf(out, ";\n");
}

/*
 * The wait statement contains simply an array of events to wait on,
 * and a sub-statement to execute when an event triggers.
 */
void show_stmt_wait(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      const char*comma = "";
      fprintf(out, "%*s@(", ind, "");

      for (idx = 0 ;  idx < ivl_stmt_nevent(net) ;  idx += 1) {
	    ivl_event_t evnt = ivl_stmt_events(net, idx);

	    if (evnt == 0)
		  fprintf(out, "%s/*ERROR*/", comma);
	    else
		  fprintf(out, "%s%s.%s", comma,
			  ivl_scope_name(ivl_event_scope(evnt)),
			  ivl_event_basename(evnt));

	    comma = ", ";
      }

      fprintf(out, ")\n");
      show_statement(ivl_stmt_sub_stmt(net), ind+4);
}

void show_statement(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {

	  case IVL_ST_ALLOC:
	    fprintf(out, "%*sallocate automatic storage ...\n", ind, "");
            break;

	  case IVL_ST_ASSIGN:
	    fprintf(out, "%*sASSIGN <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

	    if (ivl_stmt_delay_expr(net))
		  show_expression(ivl_stmt_delay_expr(net), idx+4);

	    if (ivl_stmt_rval(net))
		  show_expression(ivl_stmt_rval(net), ind+4);
	    break;

	  case IVL_ST_ASSIGN_NB:
	    fprintf(out, "%*sASSIGN_NB <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

	    if (ivl_stmt_delay_expr(net)) {
		  fprintf(out, "%*s<internal delay>\n", ind+4, "");
		  show_expression(ivl_stmt_delay_expr(net), ind+6);
	    }

	    if (ivl_stmt_rval(net))
		  show_expression(ivl_stmt_rval(net), ind+4);
	    break;

	  case IVL_ST_BLOCK: {
		unsigned cnt = ivl_stmt_block_count(net);
		ivl_scope_t sscope = ivl_stmt_block_scope(net);
		if (sscope)
		      fprintf(out, "%*sbegin : %s\n", ind, "",
			      ivl_scope_name(sscope));
		else
		      fprintf(out, "%*sbegin\n", ind, "");

		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      ivl_statement_t cur = ivl_stmt_block_stmt(net, idx);
		      show_statement(cur, ind+4);
		}
		fprintf(out, "%*send\n", ind, "");
		break;
	  }

	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	  case IVL_ST_CASER:
	  case IVL_ST_CASE: {
		unsigned cnt = ivl_stmt_case_count(net);
		fprintf(out, "%*scase (...) <%u cases>\n", ind, "", cnt);
		show_expression(ivl_stmt_cond_expr(net), ind+4);

		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      ivl_expr_t ex = ivl_stmt_case_expr(net, idx);
		      ivl_statement_t st = ivl_stmt_case_stmt(net, idx);
		      if (ex == 0)
			    fprintf(out, "%*sdefault\n", ind+4, "");
		      else
			    show_expression(ex, ind+4);

		      show_statement(st, ind+4);
		}

		fprintf(out, "%*sendcase\n", ind, "");
		break;
	  }

	  case IVL_ST_CASSIGN:
	    show_stmt_cassign(net, ind);
	    break;

	  case IVL_ST_CONDIT: {
		ivl_expr_t ex = ivl_stmt_cond_expr(net);
		ivl_statement_t t = ivl_stmt_cond_true(net);
		ivl_statement_t f = ivl_stmt_cond_false(net);

		fprintf(out, "%*sif (...)\n", ind, "");
		if (ex) {
		      show_expression(ex, ind+4);
		} else {
		      fprintf(out, "%*sERROR: Condition expression is NIL;\n", ind+4, "");
		      stub_errors += 1;
		}
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

	  case IVL_ST_CONTRIB:
	    fprintf(out, "%*sCONTRIBUTION ( <+ )\n", ind, "");
	    show_expression(ivl_stmt_lexp(net), ind+4);
	    show_expression(ivl_stmt_rval(net), ind+4);
	    break;

	  case IVL_ST_DEASSIGN:
	    fprintf(out, "%*sDEASSIGN <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

	    break;

	  case IVL_ST_DELAY:
	    fprintf(out, "%*s#%" PRIu64 "\n", ind, "", ivl_stmt_delay_val(net));
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  case IVL_ST_DELAYX:
	    show_stmt_delayx(net, ind);
	    break;

	  case IVL_ST_FORCE:
	    show_stmt_force(net, ind);
	    break;

	  case IVL_ST_FORK: {
		unsigned cnt = ivl_stmt_block_count(net);
		fprintf(out, "%*sfork\n", ind, "");
		for (idx = 0 ;  idx < cnt ;  idx += 1) {
		      ivl_statement_t cur = ivl_stmt_block_stmt(net, idx);
		      show_statement(cur, ind+4);
		}
		fprintf(out, "%*sjoin\n", ind, "");
		break;
	  }

	  case IVL_ST_FREE:
	    fprintf(out, "%*sfree automatic storage ...\n", ind, "");
            break;

	  case IVL_ST_NOOP:
	    fprintf(out, "%*s/* noop */;\n", ind, "");
	    break;

	  case IVL_ST_RELEASE:
	    show_stmt_release(net, ind);
	    break;

	  case IVL_ST_STASK: {
		fprintf(out, "%*sCall %s(%u parameters); /* %s:%u */\n",
			ind, "", ivl_stmt_name(net), ivl_stmt_parm_count(net),
			ivl_stmt_file(net), ivl_stmt_lineno(net));
		for (idx = 0 ;  idx < ivl_stmt_parm_count(net) ;  idx += 1)
		      if (ivl_stmt_parm(net, idx))
			    show_expression(ivl_stmt_parm(net, idx), ind+4);
		break;
	  }

	  case IVL_ST_TRIGGER:
	    show_stmt_trigger(net, ind);
	    break;

	  case IVL_ST_UTASK:
	    fprintf(out, "%*scall task ...\n", ind, "");
	    break;

	  case IVL_ST_WAIT:
	    show_stmt_wait(net, ind);
	    break;

	  case IVL_ST_WHILE:
	    fprintf(out, "%*swhile\n", ind, "");
	    show_expression(ivl_stmt_cond_expr(net), ind+4);
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  default:
	    fprintf(out, "%*sunknown statement type (%u)\n", ind, "", code);
      }
}
