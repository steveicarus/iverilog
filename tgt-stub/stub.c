/*
 * Copyright (c) 2000-2004 Stephen Williams (steve@icarus.com)
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
#ident "$Id: stub.c,v 1.94 2004/12/29 23:55:43 steve Exp $"
#endif

# include "config.h"
# include "priv.h"
# include <assert.h>

FILE*out;

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

void show_expression(ivl_expr_t net, unsigned ind)
{
      unsigned idx;
      const ivl_expr_type_t code = ivl_expr_type(net);
      ivl_parameter_t par = ivl_expr_parameter(net);
      unsigned width = ivl_expr_width(net);
      const char*sign = ivl_expr_signed(net)? "signed" : "unsigned";
      const char*vt = "?";

      switch (ivl_expr_value(net)) {
	  case IVL_VT_VOID:
	    vt = "void";
	    break;
	  case IVL_VT_REAL:
	    vt = "real";
	    break;
	  case IVL_VT_VECTOR:
	    vt = "vector";
	    break;
      }

      switch (code) {

	  case IVL_EX_BITSEL:
	    fprintf(out, "%*s<%s[] width=%u, %s>\n", ind, "",
		    ivl_signal_name(ivl_expr_signal(net)), width, sign);
	    show_expression(ivl_expr_oper1(net), ind+3);
	    break;

	  case IVL_EX_BINARY:
	    fprintf(out, "%*s<\"%c\" width=%u, %s, type=%s>\n", ind, "",
		    ivl_expr_opcode(net), width, sign, vt);
	    show_expression(ivl_expr_oper1(net), ind+3);
	    show_expression(ivl_expr_oper2(net), ind+3);
	    break;

	  case IVL_EX_CONCAT:
	    fprintf(out, "%*s<concat repeat=%u, width=%u, %s, type=%s>\n",
		    ind, "", ivl_expr_repeat(net), width, sign, vt);
	    for (idx = 0 ;  idx < ivl_expr_parms(net) ;  idx += 1)
		  show_expression(ivl_expr_parm(net, idx), ind+3);

	    break;

	  case IVL_EX_NUMBER: {
		const char*bits = ivl_expr_bits(net);

		fprintf(out, "%*s<number=%u'b", ind, "", width);
		for (idx = width ;  idx > 0 ;  idx -= 1)
		      fprintf(out, "%c", bits[idx-1]);

		fprintf(out, ", %s", sign);
		if (par != 0)
		      fprintf(out, ", parameter=%s",
			      ivl_parameter_basename(par));

		fprintf(out, ">\n");
		break;
	  }

	  case IVL_EX_SELECT:
	      /* The SELECT expression can be used to express part
		 select, or if the base is null vector extension. */
	    if (ivl_expr_oper2(net)) {
		  fprintf(out, "%*s<select: width=%u, %s>\n", ind, "",
			  width, sign);
		  show_expression(ivl_expr_oper1(net), ind+3);
		  show_expression(ivl_expr_oper2(net), ind+3);
	    } else {
		  fprintf(out, "%*s<expr pad: width=%u, %s>\n", ind, "",
			  width, sign);
		  show_expression(ivl_expr_oper1(net), ind+3);
	    }
	    break;

	  case IVL_EX_STRING:
	    fprintf(out, "%*s<string=\"%s\", width=%u", ind, "",
		    ivl_expr_string(net), ivl_expr_width(net));
	    if (par != 0)
		      fprintf(out, ", parameter=%s",
			      ivl_parameter_basename(par));

	    fprintf(out, ">\n");
	    break;

	  case IVL_EX_SFUNC:
	    fprintf(out, "%*s<function=\"%s\", width=%u, %s, vt=%d>\n",
		    ind, "", ivl_expr_name(net), ivl_expr_width(net),
		    sign, ivl_expr_value(net));
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

	  case IVL_EX_VARIABLE:
	    fprintf(out, "%*s<variable %s, type=%s>\n",
		    ind, "", ivl_expr_name(net), vt);
	    break;

	  case IVL_EX_REALNUM:
	      {
		    int idx;
		    union foo {
			  double rv;
			  unsigned char bv[sizeof(double)];
		    } tmp;
		    tmp.rv = ivl_expr_dvalue(net);
		    fprintf(out, "%*s<realnum=%f (", ind, "", tmp.rv);
		    for (idx = sizeof(double) ;  idx > 0 ;  idx -= 1)
			  fprintf(out, "%02x", tmp.bv[idx-1]);
		    fprintf(out, ")");
		    if (par != 0)
			  fprintf(out, ", parameter=%s",
				  ivl_parameter_basename(par));

		    fprintf(out, ">\n");
	      }
	      break;

	  default:
	    fprintf(out, "%*s<expr_type=%u>\n", ind, "", code);
	    break;
      }
}


/* IVL_LPM_CONCAT
 * The concat device takes N inputs (N=ivl_lpm_selects) and generates
 * a single output. The total output is known from the ivl_lpm_width
 * function. The widths of all the inputs are inferred from the widths
 * of the signals connected to the nexus of the inputs. The compiler
 * makes sure the input widths add up to the output width.
 */
static void show_lpm_concat(ivl_lpm_t net)
{
      unsigned idx;

      unsigned width_sum = 0;
      unsigned width = ivl_lpm_width(net);

      fprintf(out, "  LPM_CONCAT %s: <width=%u, inputs=%u>\n",
	      ivl_lpm_basename(net), width, ivl_lpm_selects(net));
      fprintf(out, "    O: %s\n", ivl_nexus_name(ivl_lpm_q(net,0)));

      for (idx = 0 ;  idx < ivl_lpm_selects(net) ;  idx += 1) {
	    unsigned ndx;
	    unsigned signal_width = 0;
	    ivl_nexus_t nex = ivl_lpm_data(net, idx);

	    for (ndx = 0 ;  ndx < ivl_nexus_ptrs(nex) ;  ndx += 1) {
		  ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, ndx);
		  ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
		  if (sig != 0) {
			signal_width = ivl_signal_width(sig);
			break;
		  }
	    }

	    fprintf(out, "    I%u: %s (width=%u)\n", idx,
		    ivl_nexus_name(nex), signal_width);
	    width_sum += signal_width;
      }

      if (width_sum != width) {
	    fprintf(out, "    ERROR! Got %u bits input, expecting %u!\n",
		    width_sum, width);
      }
}

static void show_lpm(ivl_lpm_t net)
{
      unsigned idx;
      ivl_nexus_t nex;
      unsigned width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD: {
		fprintf(out, "  LPM_ADD %s: <width=%u>\n",
			ivl_lpm_basename(net), width);
		fprintf(out, "    Q: %s\n", ivl_nexus_name(ivl_lpm_q(net, 0)));
		nex = ivl_lpm_data(net, 0);
		fprintf(out, "    DataA: %s\n", nex? ivl_nexus_name(nex) : "");
		ivl_nexus_t nex = ivl_lpm_datab(net, 0);
		fprintf(out, "    DataB: %s\n", nex? ivl_nexus_name(nex) : "");
		break;
	  }

	  case IVL_LPM_DIVIDE: {
		fprintf(out, "  LPM_DIVIDE %s: <width=%u %s>\n",
			ivl_lpm_basename(net), width,
			ivl_lpm_signed(net)? "signed" : "unsigned");
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

	  case IVL_LPM_CMP_NE: {
		fprintf(out, "  LPM_COMPARE(NE) %s: <width=%u>\n",
			ivl_lpm_basename(net), width);
		fprintf(out, "    Q: %s\n", ivl_nexus_name(ivl_lpm_q(net, 0)));
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

	  case IVL_LPM_CONCAT:
	    show_lpm_concat(net);
	    break;

	  case IVL_LPM_SHIFTL: {
		fprintf(out, "  LPM_SHIFTL %s: <width=%u, selects=%u %s>\n",
			ivl_lpm_basename(net), width, ivl_lpm_selects(net),
			ivl_lpm_signed(net)? "signed" : "unsigned");
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

	  case IVL_LPM_SHIFTR: {
		fprintf(out, "  LPM_SHIFTR %s: <width=%u, selects=%u %s>\n",
			ivl_lpm_basename(net), width, ivl_lpm_selects(net),
			ivl_lpm_signed(net)? "signed" : "unsigned");
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
			ivl_lpm_basename(net), width);
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
			ivl_lpm_basename(net), width);

		if (ivl_lpm_enable(net))
		      fprintf(out, "    clk: %s CE: %s\n",
			      ivl_nexus_name(ivl_lpm_clk(net)),
			      ivl_nexus_name(ivl_lpm_enable(net)));
		else
		      fprintf(out, "    clk: %s\n",
			      ivl_nexus_name(ivl_lpm_clk(net)));

		if (ivl_lpm_async_clr(net))
		      fprintf(out, "    Aclr: %s\n",
			      ivl_nexus_name(ivl_lpm_async_clr(net)));

		if (ivl_lpm_async_set(net)) {
		      fprintf(out, "    Aset: %s\n",
			      ivl_nexus_name(ivl_lpm_async_set(net)));
		      if (ivl_lpm_aset_value(net))
			    show_expression(ivl_lpm_aset_value(net), 10);
		}

		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Data %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_data(net, idx)));

		for (idx = 0 ;  idx < width ;  idx += 1)
		      fprintf(out, "    Q %u: %s\n", idx,
			      ivl_nexus_name(ivl_lpm_q(net, idx)));

		break;
	  }

	  case IVL_LPM_MULT: {
		fprintf(out, "  LPM_MULT %s: <width=%u>\n",
			ivl_lpm_basename(net), width);
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

	  case IVL_LPM_MUX: {
		unsigned sdx;

		fprintf(out, "  LPM_MUX %s: <width=%u, size=%u, sel_wid=%u>\n",
			ivl_lpm_basename(net), width, ivl_lpm_size(net),
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

	  case IVL_LPM_PART: {
		fprintf(out, "  LPM_PART %s: <width=%u, base=%u, signed=%d>\n",
			ivl_lpm_basename(net),
			width, ivl_lpm_base(net), ivl_lpm_signed(net));
		fprintf(out, "    O: %s\n", ivl_nexus_name(ivl_lpm_q(net,0)));
		fprintf(out, "    I: %s\n", ivl_nexus_name(ivl_lpm_data(net,0)));
		break;
	  }

	  default:
	    fprintf(out, "  LPM(%d) %s: <width=%u, signed=%d>\n",
		    ivl_lpm_type(net),
		    ivl_lpm_basename(net),
		    ivl_lpm_width(net),
		    ivl_lpm_signed(net));
      }
}


static int show_process(ivl_process_t net, void*x)
{
      unsigned idx;

      switch (ivl_process_type(net)) {
	  case IVL_PR_INITIAL:
	    fprintf(out, "initial\n");
	    break;
	  case IVL_PR_ALWAYS:
	    fprintf(out, "always\n");
	    break;
      }

      for (idx = 0 ;  idx < ivl_process_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t attr = ivl_process_attr_val(net, idx);
	    switch (attr->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "    (* %s *)\n", attr->key);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "    (* %s = \"%s\" *)\n", attr->key,
			  attr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    (* %s = %ld *)\n", attr->key,
			  attr->val.num);
		  break;
	    }
      }

      show_statement(ivl_process_stmt(net), 4);

      return 0;
}

static void show_parameter(ivl_parameter_t net)
{
      const char*name = ivl_parameter_basename(net);
      fprintf(out, "   parameter %s;\n", name);
      show_expression(ivl_parameter_expr(net), 7);
}

static void show_variable(ivl_variable_t net)
{
      const char*type = "?";
      const char*name = ivl_variable_name(net);

      switch (ivl_variable_type(net)) {
	  case IVL_VT_VOID:
	    type = "void";
	    break;
	  case IVL_VT_REAL:
	    type = "real";
	    break;
	  case IVL_VT_VECTOR:
	    type = "vector";
	    break;
      }

      fprintf(out, "  variable %s %s;\n", type, name);
}

static void show_event(ivl_event_t net)
{
      unsigned idx;
      fprintf(out, "  event %s (%u pos, %u neg, %u any);\n",
	      ivl_event_basename(net), ivl_event_npos(net),
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
      unsigned idx;
      ivl_nexus_t nex;

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
	  case IVL_SIT_TRI0:
	    type = "tri0";
	    break;
	  case IVL_SIT_TRI1:
	    type = "tri1";
	    break;
	  case IVL_SIT_SUPPLY0:
	    type = "supply0";
	    break;
	  case IVL_SIT_SUPPLY1:
	    type = "supply1";
	    break;
	  default:
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

	  case IVL_SIP_NONE:
	    break;
      }

      nex = ivl_signal_nex(net);

      fprintf(out, "  %s %s %s[%d:%d] %s  <width=%u> nexus=%s\n",
	      type, sign, port,
	      ivl_signal_msb(net), ivl_signal_lsb(net),
	      ivl_signal_basename(net), ivl_signal_width(net),
	      ivl_nexus_name(nex));


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
		  fprintf(out, "      LPM %s.%s (%s0, %s1)\n",
			  ivl_scope_name(ivl_lpm_scope(lpm)),
			  ivl_lpm_basename(lpm), dr0, dr1);

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

      for (idx = 0 ;  idx < ivl_signal_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t atr = ivl_signal_attr_val(net, idx);

	    switch (atr->type) {
		case IVL_ATT_STR:
		  fprintf(out, "    %s = %s\n", atr->key, atr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    %s = %ld\n", atr->key, atr->val.num);
		  break;
		case IVL_ATT_VOID:
		  fprintf(out, "    %s\n", atr->key);
		  break;
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
	  case IVL_LO_NOT:
	    fprintf(out, "  not #(%u) %s (%s",
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

	  case IVL_LO_UDP:
	    fprintf(out, "  primitive %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;

	  default:
	    fprintf(out, "  unsupported gate %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
      }

      npins = ivl_logic_pins(net);
      for (idx = 1 ;  idx < npins ;  idx += 1) {
	    ivl_nexus_t nex = ivl_logic_pin(net,idx);

	    if (nex == 0)
		  fprintf(out, ", <HiZ>");
	    else
		  fprintf(out, ", %s", ivl_nexus_name(nex));
      }

      fprintf(out, "); <width=%u>\n", ivl_logic_width(net));

      npins = ivl_logic_attr_cnt(net);
      for (idx = 0 ;  idx < npins ;  idx += 1) {
	    ivl_attribute_t cur = ivl_logic_attr_val(net,idx);
	    switch (cur->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "    %s\n", cur->key);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "    %s = %ld\n", cur->key, cur->val.num);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "    %s = %s\n", cur->key, cur->val.str);
		  break;
	    }
      }
}

static int show_scope(ivl_scope_t net, void*x)
{
      unsigned idx;

      fprintf(out, "scope: %s (%u parameters, %u signals, %u logic)",
	      ivl_scope_name(net), ivl_scope_params(net),
	      ivl_scope_sigs(net), ivl_scope_logs(net));

      switch (ivl_scope_type(net)) {
	  case IVL_SCT_MODULE:
	    fprintf(out, " module %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_FUNCTION:
	    fprintf(out, " function %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_BEGIN:
	    fprintf(out, " begin : %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_FORK:
	    fprintf(out, " fork : %s", ivl_scope_tname(net));
	    break;
	  case IVL_SCT_TASK:
	    fprintf(out, " task %s", ivl_scope_tname(net));
	    break;
	  default:
	    fprintf(out, " type(%u) %s", ivl_scope_type(net),
		    ivl_scope_tname(net));
	    break;
      }

      fprintf(out, " time units = 10e%d\n", ivl_scope_time_units(net));

      for (idx = 0 ;  idx < ivl_scope_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t attr = ivl_scope_attr_val(net, idx);
	    switch (attr->type) {
		case IVL_ATT_VOID:
		  fprintf(out, "  (* %s *)\n", attr->key);
		  break;
		case IVL_ATT_STR:
		  fprintf(out, "  (* %s = \"%s\" *)\n", attr->key,
			  attr->val.str);
		  break;
		case IVL_ATT_NUM:
		  fprintf(out, "  (* %s = %ld *)\n", attr->key,
			  attr->val.num);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_params(net) ;  idx += 1)
	    show_parameter(ivl_scope_param(net, idx));

      for (idx = 0 ;  idx < ivl_scope_vars(net) ;  idx += 1)
	    show_variable(ivl_scope_var(net, idx));

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
 * Revision 1.94  2004/12/29 23:55:43  steve
 *  Unify elaboration of l-values for all proceedural assignments,
 *  including assing, cassign and force.
 *
 *  Generate NetConcat devices for gate outputs that feed into a
 *  vector results. Use this to hande gate arrays. Also let gate
 *  arrays handle vectors of gates when the outputs allow for it.
 *
 * Revision 1.93  2004/12/18 18:55:08  steve
 *  Better detail on event trigger and wait statements.
 *
 * Revision 1.92  2004/12/12 18:15:06  steve
 *  Arrange statement dumping in new source files.
 *
 * Revision 1.91  2004/12/11 02:31:28  steve
 *  Rework of internals to carry vectors through nexus instead
 *  of single bits. Make the ivl, tgt-vvp and vvp initial changes
 *  down this path.
 *
 * Revision 1.90  2004/10/04 01:10:57  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.89  2004/09/25 01:57:33  steve
 *  Dump tri0 and tri1 nets.
 *
 * Revision 1.88  2004/06/30 03:05:04  steve
 *  Dump variable type of system function.
 *
 * Revision 1.87  2004/06/30 02:16:27  steve
 *  Implement signed divide and signed right shift in nets.
 *
 * Revision 1.86  2004/06/17 16:06:19  steve
 *  Help system function signedness survive elaboration.
 *
 * Revision 1.85  2004/06/16 16:22:04  steve
 *  Dump NE LPM device.
 *
 * Revision 1.84  2003/12/03 04:27:10  steve
 *  Pre-gcc3 compile error.
 *
 * Revision 1.83  2003/12/03 02:46:24  steve
 *  Add support for wait on list of named events.
 *
 * Revision 1.82  2003/12/03 01:54:07  steve
 *  Handle erroneous event lists.
 *
 * Revision 1.81  2003/07/26 03:34:43  steve
 *  Start handling pad of expressions in code generators.
 *
 * Revision 1.80  2003/06/23 01:25:44  steve
 *  Module attributes make it al the way to ivl_target.
 *
 * Revision 1.79  2003/05/14 05:26:41  steve
 *  Support real expressions in case statements.
 *
 * Revision 1.78  2003/05/13 01:56:15  steve
 *  Allow primitives to hvae unconnected input ports.
 *
 * Revision 1.77  2003/04/11 05:18:08  steve
 *  Handle signed magnitude compare all the
 *  way through to the vvp code generator.
 *
 * Revision 1.76  2003/03/29 05:51:26  steve
 *  Sign extend NetMult inputs if result is signed.
 *
 * Revision 1.75  2003/03/10 23:40:54  steve
 *  Keep parameter constants for the ivl_target API.
 *
 * Revision 1.74  2003/03/07 06:04:58  steve
 *  Raw dump of double values for testing purposes.
 *
 * Revision 1.73  2003/02/25 03:39:53  steve
 *  Eliminate use of ivl_lpm_name.
 *
 * Revision 1.72  2003/01/26 21:15:59  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.71  2002/12/21 00:55:58  steve
 *  The $time system task returns the integer time
 *  scaled to the local units. Change the internal
 *  implementation of vpiSystemTime the $time functions
 *  to properly account for this. Also add $simtime
 *  to get the simulation time.
 */
