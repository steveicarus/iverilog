/*
 * Copyright (c) 2004 Stephen Williams (steve@icarus.com)
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: statement.c,v 1.2 2004/12/15 17:11:13 steve Exp $"
#endif

# include "config.h"
# include "priv.h"
# include <assert.h>

static unsigned show_assign_lval(ivl_lval_t lval, unsigned ind)
{
      ivl_memory_t mem;
      ivl_variable_t var;
      unsigned wid = 0;

      if ( (mem = ivl_lval_mem(lval)) ) {

	    ivl_scope_t scope = ivl_memory_scope(mem);
	    fprintf(out, "%*s%s . %s[\n", ind, "",
		    ivl_scope_name(scope),
		    ivl_memory_basename(mem));
	    show_expression(ivl_lval_idx(lval), ind+4);
	    fprintf(out, "%*s]\n", ind, "");

      } else if ( (var = ivl_lval_var(lval)) ) {

	    fprintf(out, "%*svariable %s\n", ind, "", ivl_variable_name(var));

      } else {
	    ivl_signal_t sig = ivl_lval_sig(lval);

	    fprintf(out, "%*s{name=%s width=%u part_off=%u lvwidth=%u}\n",
		    ind, "",
		    ivl_signal_name(sig),
		    ivl_signal_width(sig),
		    ivl_lval_part_off(lval),
		    ivl_lval_width(lval));

	    wid = ivl_lval_width(lval);
      }

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

void show_statement(ivl_statement_t net, unsigned ind)
{
      unsigned idx;
      const ivl_statement_type_t code = ivl_statement_type(net);

      switch (code) {

	  case IVL_ST_ASSIGN:
	    fprintf(out, "%*sASSIGN <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

	    if (ivl_stmt_rval(net))
		  show_expression(ivl_stmt_rval(net), ind+4);
	    break;

	  case IVL_ST_ASSIGN_NB:
	    fprintf(out, "%*sASSIGN_NB <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

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
		unsigned idx;
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
		show_expression(ex, ind+4);
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

	  case IVL_ST_DEASSIGN:
	    fprintf(out, "%*sDEASSIGN <lwidth=%u>\n", ind, "",
		    ivl_stmt_lwidth(net));

	    for (idx = 0 ;  idx < ivl_stmt_lvals(net) ;  idx += 1)
		  show_assign_lval(ivl_stmt_lval(net, idx), ind+4);

	    break;

	  case IVL_ST_DELAY:
	    fprintf(out, "%*s#%lu\n", ind, "", ivl_stmt_delay_val(net));
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
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

	  case IVL_ST_NOOP:
	    fprintf(out, "%*s/* noop */;\n", ind, "");
	    break;

	  case IVL_ST_RELEASE:
	    show_stmt_release(net, ind);
	    break;

	  case IVL_ST_STASK: {
		fprintf(out, "%*sCall %s(%u parameters);\n", ind, "",
			ivl_stmt_name(net), ivl_stmt_parm_count(net));
		for (idx = 0 ;  idx < ivl_stmt_parm_count(net) ;  idx += 1)
		      if (ivl_stmt_parm(net, idx))
			    show_expression(ivl_stmt_parm(net, idx), ind+4);
		break;
	  }

	  case IVL_ST_TRIGGER:
	    fprintf(out, "%*s-> ...\n", ind, "");
	    break;

	  case IVL_ST_UTASK:
	    fprintf(out, "%*scall task ...\n", ind, "");
	    break;

	  case IVL_ST_WAIT: {
		const char*comma = "";
		fprintf(out, "%*s@(", ind, "");

		for (idx = 0 ;  idx < ivl_stmt_nevent(net) ;  idx += 1) {
		      ivl_event_t evnt = ivl_stmt_events(net, idx);

		      if (evnt == 0)
			    fprintf(out, "%s/*ERROR*/", comma);
		      else
			    fprintf(out, "%s%s", comma, ivl_event_name(evnt));

		      comma = ", ";
		}

		fprintf(out, ")\n");
		show_statement(ivl_stmt_sub_stmt(net), ind+4);
		break;
	  }

	  case IVL_ST_WHILE:
	    fprintf(out, "%*swhile\n", ind, "");
	    show_expression(ivl_stmt_cond_expr(net), ind+4);
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
	    break;

	  default:
	    fprintf(out, "%*sunknown statement type (%u)\n", ind, "", code);
      }
}

