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
#ident "$Id: stub.c,v 1.57 2002/04/27 04:20:52 steve Exp $"
#endif

# include "config.h"

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

# include  "ivl_target.h"
# include  <stdio.h>

static FILE*out;

static void show_expression(ivl_expr_t net, unsigned ind)
{
      unsigned idx;
      const ivl_expr_type_t code = ivl_expr_type(net);
      unsigned width = ivl_expr_width(net);
      const char*sign = ivl_expr_signed(net)? "signed" : "unsigned";

      switch (code) {

	  case IVL_EX_BINARY:
	    fprintf(out, "%*s<\"%c\" width=%u, %s>\n", ind, "",
		    ivl_expr_opcode(net), width, sign);
	    show_expression(ivl_expr_oper1(net), ind+3);
	    show_expression(ivl_expr_oper2(net), ind+3);
	    break;

	  case IVL_EX_CONCAT:
	    fprintf(out, "%*s<concat repeat=%u, width=%u, %s>\n", ind, "",
		    ivl_expr_repeat(net), width, sign);
	    for (idx = 0 ;  idx < ivl_expr_parms(net) ;  idx += 1)
		  show_expression(ivl_expr_parm(net, idx), ind+3);

	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(net);

		fprintf(out, "%*s<number=%u'b", ind, "", width);
		for (idx = width ;  idx > 0 ;  idx -= 1)
		      fprintf(out, "%c", bits[idx-1]);

		fprintf(out, ", %s>\n", sign);
		break;
	  }

	  case IVL_EX_SELECT:
	    fprintf(out, "%*s<bit select, width=%u, %s>\n", ind, "",
		    width, sign);
	    show_expression(ivl_expr_oper1(net), ind+3);
	    show_expression(ivl_expr_oper2(net), ind+3);
	    break;

	  case IVL_EX_STRING:
	    fprintf(out, "%*s<string=\"%s\", width=%u>\n", ind, "",
		    ivl_expr_string(net), ivl_expr_width(net));
	    break;

	  case IVL_EX_SFUNC:
	    fprintf(out, "%*s<function=\"%s\", width=%u>\n", ind, "",
		    ivl_expr_name(net), ivl_expr_width(net));
	    { unsigned cnt = ivl_expr_parms(net);
	      unsigned idx;
	      for (idx = 0 ;  idx < cnt ;  idx += 1)
		    show_expression(ivl_expr_parm(net, idx), ind+3);
	    }
	    break;

	  case IVL_EX_SIGNAL:
	    fprintf(out, "%*s<signal=%s, width=%u, %s>\n", ind, "",
		    ivl_expr_name(net), width, sign);
	    break;

	  case IVL_EX_TERNARY:
	    fprintf(out, "%*s<ternary  width=%u, %s>\n", ind, "",
		    width, sign);
	    show_expression(ivl_expr_oper1(net), ind+4);
	    show_expression(ivl_expr_oper2(net), ind+4);
	    show_expression(ivl_expr_oper3(net), ind+4);
	    break;

	  case IVL_EX_UNARY:
	    fprintf(out, "%*s<unary \"%c\" width=%u, %s>\n", ind, "",
		    ivl_expr_opcode(net), width, sign);
	    show_expression(ivl_expr_oper1(net), ind+4);
	    break;

	  default:
	    fprintf(out, "%*s<expr_type=%u>\n", ind, "", code);
	    break;
      }
}

static void show_lpm(ivl_lpm_t net)
{
      unsigned idx;
      unsigned width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD: {
		fprintf(out, "  LPM_ADD %s: <width=%u>\n",
			ivl_lpm_name(net), width);
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));
		for (idx = 0 ;  idx < width ;  idx += 1) {
		      ivl_nexus_t nex = ivl_lpm_data(net, idx);
		      fprintf(out, "    Data A %u: %s\n", idx,
			      nex? ivl_nexus_name(nex) : "");
		}
		for (idx = 0 ;  idx < width ;  idx += 1) {
		      ivl_nexus_t nex = ivl_lpm_datab(net, idx);
		      fprintf(out, "    Data B %u: %s\n", idx,
			      nex? ivl_nexus_name(nex) : "");
		}
		break;
	  }

	  case IVL_LPM_SHIFTL: {
		fprintf(out, "  LPM_SHIFTL %s: <width=%u, selects=%u>\n",
			ivl_lpm_name(net), width, ivl_lpm_selects(net));
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Data A %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_data(net, idx)));
		for (idx = 0 ;  idx < ivl_lpm_selects(net) ;  idx += 1)
		      fprintf(out, "    Shift %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_select(net, idx)));
		break;
	  }

	  case IVL_LPM_SUB: {
		fprintf(out, "  LPM_SUB %s: <width=%u>\n",
			ivl_lpm_name(net), width);
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Data A %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_data(net, idx)));
		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Data B %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_datab(net, idx)));
		break;
	  }

	  case IVL_LPM_FF: {

		fprintf(out, "  LPM_FF %s: <width=%u>\n",
			ivl_lpm_name(net), width);
		fprintf(out, "    clk: %s\n",
			ivl_nexus_name(ivl_lpm_clk(net)));

		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Data %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_data(net, idx)));

		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));

		break;
	  }

	  case IVL_LPM_MUX: {
		unsigned sdx;

		fprintf(out, "  LPM_MUX %s: <width=%u, size=%u, sel_wid=%u>\n",
			ivl_lpm_name(net), width, ivl_lpm_size(net),
			ivl_lpm_selects(net));

		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));

		for (idx = 0 ;  idx < ivl_lpm_selects(net) ;  idx += 1)
		      fprintf(out, "    S %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_select(net, idx)));

		for (sdx = 0 ;  sdx < ivl_lpm_size(net) ;  sdx += 1)
		      for (idx = 0 ;  idx < width ;  idx += 1)
			    fprintf(out, "    D%u %u: %s\n", sdx, idx,
				 ivl_nexus_name(ivl_lpm_data2(net,sdx,idx)));
		break;
	  }

	  default:
	    fprintf(out, "  %s: <width=%u>\n", ivl_lpm_name(net),
		    ivl_lpm_width(net));
      }
}

static void show_assign_lval(ivl_lval_t lval, unsigned ind)
{
      ivl_memory_t mem;

      if ( (mem = ivl_lval_mem(lval)) ) {

	    fprintf(out, "%*s%s[", ind, "", ivl_memory_name(mem));
	    fprintf(out, "]\n");

      } else {
	    unsigned pp;
	    ivl_nexus_t nex = ivl_lval_pin(lval, 0);

	    fprintf(out, "%*s{%s", ind, "", ivl_nexus_name(nex));
	    fprintf(out, "<nptrs=%u>", ivl_nexus_ptrs(nex));
	    for (pp = 1 ;  pp < ivl_lval_pins(lval) ;  pp += 1) {
		  nex = ivl_lval_pin(lval, pp);
		  fprintf(out, ", %s", ivl_nexus_name(nex));
		  fprintf(out, "<nptrs=%u>", ivl_nexus_ptrs(nex));
	    }
	    fprintf(out, "}\n");
      }
}

static void show_statement(ivl_statement_t net, unsigned ind)
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

	  case IVL_ST_DELAY:
	    fprintf(out, "%*s#%lu\n", ind, "", ivl_stmt_delay_val(net));
	    show_statement(ivl_stmt_sub_stmt(net), ind+2);
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
		ivl_event_t evnt = ivl_stmt_event(net);
		fprintf(out, "%*s@(%s)\n", ind, "", ivl_event_name(evnt));
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

static int show_process(ivl_process_t net, void*x)
{
      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    fprintf(out, "initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	    fprintf(out, "always\n");
	    break;
      }

      show_statement(ivl_process_stmt(net), 4);

      return 0;
}

static void show_event(ivl_event_t net)
{
      unsigned idx;
      fprintf(out, "  event %s (%u pos, %u neg, %u any);\n",
	      ivl_event_name(net), ivl_event_npos(net),
	      ivl_event_nneg(net), ivl_event_nany(net));

      for (idx = 0 ;  idx < ivl_event_nany(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_any(net, idx);
	    fprintf(out, "      ANYEDGE: %s\n", ivl_nexus_name(nex));
      }

      for (idx = 0 ;  idx < ivl_event_nneg(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_neg(net, idx);
	    fprintf(out, "      NEGEDGE: %s\n", ivl_nexus_name(nex));
      }

      for (idx = 0 ;  idx < ivl_event_npos(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_event_pos(net, idx);
	    fprintf(out, "      POSEDGE: %s\n", ivl_nexus_name(nex));
      }
}

static void show_signal(ivl_signal_t net)
{
      unsigned pin;
      const char*type = "?";
      const char*port = "";
      const char*sign = ivl_signal_signed(net)? "signed" : "unsigned";

      switch (ivl_signal_type(net)) {
	  case IVL_SIT_REG:
	    type = "reg";
	    break;
	  case IVL_SIT_TRI:
	    type = "tri";
	    break;
	  case IVL_SIT_SUPPLY0:
	    type = "supply0";
	    break;
	  case IVL_SIT_SUPPLY1:
	    type = "supply1";
	    break;
      }

      switch (ivl_signal_port(net)) {

	  case IVL_SIP_INPUT:
	    port = "input ";
	    break;

	  case IVL_SIP_OUTPUT:
	    port = "output ";
	    break;

	  case IVL_SIP_INOUT:
	    port = "inout ";
	    break;
      }

      fprintf(out, "  %s %s %s[%u] %s\n", type, sign, port,
	      ivl_signal_pins(net), ivl_signal_basename(net));

      for (pin = 0 ;  pin < ivl_signal_pins(net) ;  pin += 1) {
	    unsigned idx;
	    ivl_nexus_t nex = ivl_signal_pin(net, pin);

	    fprintf(out, "    [%u]: nexus=%s\n", pin, ivl_nexus_name(nex));

	    for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
		  ivl_net_const_t con;
		  ivl_net_logic_t log;
		  ivl_lpm_t lpm;
		  ivl_signal_t sig;
		  ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);

		  static const char* str_tab[8] = {
			"HiZ", "small", "medium", "weak",
			"large", "pull", "strong", "supply"};

		  const char*dr0 = str_tab[ivl_nexus_ptr_drive0(ptr)];
		  const char*dr1 = str_tab[ivl_nexus_ptr_drive1(ptr)];

		  if ((sig = ivl_nexus_ptr_sig(ptr))) {
			fprintf(out, "      %s[%u] (%s0, %s1)\n",
				ivl_signal_name(sig),
				ivl_nexus_ptr_pin(ptr), dr0, dr1);

		  } else if ((log = ivl_nexus_ptr_log(ptr))) {
			fprintf(out, "      %s[%u] (%s0, %s1)\n",
				ivl_logic_name(log),
				ivl_nexus_ptr_pin(ptr), dr0, dr1);

		  } else if ((lpm = ivl_nexus_ptr_lpm(ptr))) {
			fprintf(out, "      LPM %s (%s0, %s1)\n",
				ivl_lpm_name(lpm), dr0, dr1);

		  } else if ((con = ivl_nexus_ptr_con(ptr))) {
			const char*bits = ivl_const_bits(con);
			unsigned pin = ivl_nexus_ptr_pin(ptr);

			fprintf(out, "      const-%c (%s0, %s1)\n",
				bits[pin], dr0, dr1);


		  } else {
			fprintf(out, "      ?[%u] (%s0, %s1)\n",
				ivl_nexus_ptr_pin(ptr), dr0, dr1);
		  }
	    }
      }
}

static void show_logic(ivl_net_logic_t net)
{
      unsigned npins, idx;
      const char*name = ivl_logic_basename(net);

      switch (ivl_logic_type(net)) {
	  case IVL_LO_AND:
	    fprintf(out, "  and %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUF:
	    fprintf(out, "  buf %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUFIF0:
	    fprintf(out, "  bufif0 %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUFIF1:
	    fprintf(out, "  bufif1 %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_BUFZ:
	    fprintf(out, "  bufz #(%u) %s (%s",
		    ivl_logic_delay(net, 0),
		    name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_OR:
	    fprintf(out, "  or %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_XOR:
	    fprintf(out, "  xor %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  default:
	    fprintf(out, "  unsupported gate %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    return;
      }

      npins = ivl_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1)
	    fprintf(out, ", %s", ivl_nexus_name(ivl_logic_pin(net,idx)));

      fprintf(out, ");\n");
}

static int show_scope(ivl_scope_t net, void*x)
{
      unsigned idx;

      fprintf(out, "scope: %s (%u signals, %u logic)",
	      ivl_scope_name(net), ivl_scope_sigs(net),
	      ivl_scope_logs(net));

      switch (ivl_scope_type(net)) {
	  case IVL_SCT_MODULE:
	    fprintf(out, " module %s\n", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_FUNCTION:
	    fprintf(out, " function %s\n", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_BEGIN:
	    fprintf(out, " begin : %s\n", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_FORK:
	    fprintf(out, " fork : %s\n", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_TASK:
	    fprintf(out, " task %s\n", ivl_scope_tname(net));
	    break;
	  default:
	    fprintf(out, " type(%u) %s\n", ivl_scope_type(net),
		    ivl_scope_tname(net));
	    break;
      }

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1)
	    show_event(ivl_scope_event(net, idx));

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1)
	    show_signal(ivl_scope_sig(net, idx));

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1)
	    show_logic(ivl_scope_log(net, idx));

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1)
	    show_lpm(ivl_scope_lpm(net, idx));

      fprintf(out, "end scope %s\n", ivl_scope_name(net));
      return ivl_scope_children(net, show_scope, 0);
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

      fprintf(out, "root module = %s;\n",
	      ivl_scope_name(ivl_design_root(des)));

      show_scope(ivl_design_root(des), 0);
      ivl_design_process(des, show_process, 0);
      fclose(out);

      return 0;
}


/*
 * $Log: stub.c,v $
 * Revision 1.57  2002/04/27 04:20:52  steve
 *  Dump parametres for system functions.
 *
 * Revision 1.56  2002/04/25 05:03:11  steve
 *  Dump bit select expressions.
 *
 * Revision 1.55  2002/04/22 03:15:25  steve
 *  Keep delays applied to BUFZ devices.
 *
 * Revision 1.54  2002/04/22 02:40:32  steve
 *  Dump the while loop expression.
 *
 * Revision 1.53  2002/04/14 19:02:34  steve
 *  Ternary expressions can be signed.
 *
 * Revision 1.52  2002/04/14 02:44:53  steve
 *  Show unary subexpressions.
 *
 * Revision 1.51  2001/12/15 02:13:17  steve
 *  The IVL_SIT_WIRE type does not exist, it is a
 *  synonym for IVL_SIT_TRI.
 *
 * Revision 1.50  2001/09/30 16:45:10  steve
 *  Fix some Cygwin DLL handling. (Venkat Iyer)
 *
 * Revision 1.49  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.48  2001/07/22 21:30:56  steve
 *  Recognize supply signal types.
 *
 * Revision 1.47  2001/07/04 22:59:25  steve
 *  handle left shifter in dll output.
 *
 * Revision 1.46  2001/07/01 23:44:19  steve
 *  Print memory l-values.
 *
 * Revision 1.45  2001/06/07 04:20:10  steve
 *  Account for carry out on add devices.
 *
 * Revision 1.44  2001/06/07 03:09:37  steve
 *  support subtraction in tgt-vvp.
 *
 * Revision 1.43  2001/06/07 02:12:43  steve
 *  Support structural addition.
 *
 * Revision 1.42  2001/05/22 02:14:47  steve
 *  Update the mingw build to not require cygwin files.
 *
 * Revision 1.41  2001/05/20 15:09:40  steve
 *  Mingw32 support (Venkat Iyer)
 *
 * Revision 1.40  2001/05/03 01:52:45  steve
 *  dll build of many probes forgot to index the probe.
 *
 * Revision 1.39  2001/04/29 23:17:38  steve
 *  Carry drive strengths in the ivl_nexus_ptr_t, and
 *  handle constant devices in targets.'
 *
 * Revision 1.38  2001/04/26 05:12:02  steve
 *  Implement simple MUXZ for ?: operators.
 *
 * Revision 1.37  2001/04/05 01:12:28  steve
 *  Get signed compares working correctly in vvp.
 *
 * Revision 1.36  2001/04/03 04:50:37  steve
 *  Support non-blocking assignments.
 *
 * Revision 1.35  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.34  2001/04/01 01:48:21  steve
 *  Redesign event information to support arbitrary edge combining.
 *
 * Revision 1.33  2001/03/31 17:36:39  steve
 *  Generate vvp code for case statements.
 *
 * Revision 1.32  2001/03/30 05:49:52  steve
 *  Generate code for fork/join statements.
 *
 * Revision 1.31  2001/03/29 02:52:39  steve
 *  Add unary ~ operator to tgt-vvp.
 *
 * Revision 1.30  2001/03/28 06:07:39  steve
 *  Add the ivl_event_t to ivl_target, and use that to generate
 *  .event statements in vvp way ahead of the thread that uses it.
 *
 * Revision 1.29  2001/02/07 22:22:00  steve
 *  ivl_target header search path fixes.
 *
 * Revision 1.28  2001/01/15 00:47:02  steve
 *  Pass scope type information to the target module.
 *
 * Revision 1.27  2001/01/15 00:05:39  steve
 *  Add client data pointer for scope and process scanners.
 *
 * Revision 1.26  2000/12/05 06:29:33  steve
 *  Make signal attributes available to ivl_target API.
 *
 * Revision 1.25  2000/11/12 17:47:29  steve
 *  flip-flop pins for ivl_target API.
 *
 * Revision 1.24  2000/11/11 00:03:36  steve
 *  Add support for the t-dll backend grabing flip-flops.
 *
 * Revision 1.23  2000/10/28 22:32:34  steve
 *  API for concatenation expressions.
 *
 * Revision 1.22  2000/10/28 17:55:03  steve
 *  stub for the concat operator.
 *
 * Revision 1.21  2000/10/25 05:41:24  steve
 *  Get target signal from nexus_ptr.
 *
 * Revision 1.20  2000/10/21 16:49:45  steve
 *  Reduce the target entry points to the target_design.
 *
 * Revision 1.19  2000/10/18 20:04:39  steve
 *  Add ivl_lval_t and support for assignment l-values.
 *
 * Revision 1.18  2000/10/15 21:02:08  steve
 *  Makefile patches to support target loading under cygwin.
 *
 * Revision 1.17  2000/10/15 04:46:23  steve
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
 *
 * Revision 1.16  2000/10/08 05:00:04  steve
 *  Missing stream in call to fprintf.
 *
 * Revision 1.15  2000/10/07 19:45:43  steve
 *  Put logic devices into scopes.
 *
 * Revision 1.14  2000/10/06 23:46:51  steve
 *  ivl_target updates, including more complete
 *  handling of ivl_nexus_t objects. Much reduced
 *  dependencies on pointers to netlist objects.
 *
 * Revision 1.13  2000/10/05 05:03:01  steve
 *  xor and constant devices.
 */

