/*
 * Copyright (c) 2000-2010 Stephen Williams (steve@icarus.com)
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
# include <assert.h>

/*
 * This is a sample target module. All this does is write to the
 * output file some information about each object handle when each of
 * the various object functions is called. This can be used to
 * understand the behavior of the core as it uses a target module.
 */

# include  "ivl_target.h"
# include  <stdio.h>

static FILE*out;

static void show_attribute(ivl_attribute_t attr, unsigned ind)
{
      switch (attr->type) {
	  case IVL_ATT_VOID:
	    fprintf(out, "%*s(* %s *)\n", ind,"", attr->key);
	    break;
	  case IVL_ATT_STR:
	    fprintf(out, "%*s(* %s = \"%s\" *)\n", ind,"", attr->key,
		    attr->val.str);
	    break;
	  case IVL_ATT_NUM:
	    fprintf(out, "%*s(* %s = %ld *)\n", ind,"", attr->key,
		    attr->val.num);
	    break;
      }
}

static void show_expression(ivl_expr_t net, unsigned ind)
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

	  case IVL_EX_BINARY: {
		ivl_expr_t oper1 = ivl_expr_oper1(net);
		ivl_expr_t oper2 = ivl_expr_oper2(net);
		fprintf(out, "%*s<\"%c\" width=%u, %s, type=%s>\n", ind, "",
			ivl_expr_opcode(net), width, sign, vt);
		if (oper1)
		      show_expression(oper1, ind+3);
		else
		      fprintf(out, "%*sERROR: nil oper1\n", ind+3, "");
		if (oper2)
		      show_expression(oper2, ind+3);
		else
		      fprintf(out, "%*sERROR: nil oper2\n", ind+3, "");
		break;
	  }

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

	  case IVL_EX_ULONG: {
		fprintf(out, "%*s<ulong=%u'd%lu, %s>\n",
			ind, "", width, ivl_expr_uvalue(net), sign);
		break;
	  }

	  case IVL_EX_SELECT:
	      /* The SELECT expression can be used to express part
		 select, or if the base is null vector extension. */
	    if (ivl_expr_oper2(net)) {
		  ivl_expr_t o1 = ivl_expr_oper1(net);
		  fprintf(out, "%*s<select: width=%u, %s>\n", ind, "",
			  width, sign);
		  if (o1)
			show_expression(ivl_expr_oper1(net), ind+3);
		  else
			fprintf(out, "%*sERROR: Missing oper1\n", ind+3, "");
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

static void show_lpm_ram(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned count = ivl_lpm_size(net);

      unsigned idx, sdx;

      fprintf(out, "  LPM_RAM_DQ %s (word-width=%u, count=%u)\n",
	      ivl_lpm_basename(net), ivl_lpm_width(net), count);
      for (idx = 0 ;  idx < ivl_lpm_selects(net) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_select(net, idx);
	    fprintf(out, "    Address %u: %s\n", idx,
		    nex? ivl_nexus_name(nex) : "");
      }

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_q(net, idx);
	    fprintf(out, "    Q %u: %s\n", idx, nex? ivl_nexus_name(nex) : "");
      }

      for (sdx = 0 ;  sdx < count ;  sdx += 1) {
	    for (idx = 0 ;  idx < width ;  idx += 1) {
		  ivl_nexus_t nex = ivl_lpm_data2(net, sdx, idx);
		  fprintf(out, "    Word%u %u: %s\n",
			  sdx, idx, nex? ivl_nexus_name(nex) : "");
	    }
      }
}

static void show_lpm_demux(ivl_lpm_t net)
{
      unsigned idx;
      unsigned width = ivl_lpm_width(net);
      unsigned awid = ivl_lpm_selects(net);
      unsigned size = ivl_lpm_size(net);

      fprintf(out, "  LPM_DEMUX %s (bus-width=%u, addr-width=%u, size=%u)\n",
	      ivl_lpm_basename(net), width, awid, size);

      if (width%size != 0) {
	    fprintf(out, "    ERROR: width %% size == %u\n", width%size);
      }

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_q(net, idx);
	    fprintf(out, "    Q %u: %s\n", idx,
		    nex? ivl_nexus_name(nex) : "");
      }
      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_data(net, idx);
	    fprintf(out, "    Data %u: %s\n", idx,
		    nex? ivl_nexus_name(nex) : "");
      }
      for (idx = 0 ;  idx < awid ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_select(net, idx);
	    fprintf(out, "    Address %u: %s\n", idx,
		    nex? ivl_nexus_name(nex) : "");
      }

      for (idx = 0 ;  idx < width/size ;  idx += 1) {
	    ivl_nexus_t nex = ivl_lpm_datab(net, idx);
	    fprintf(out, "    WriteIn %u: %s\n", idx,
		    nex? ivl_nexus_name(nex) : "");
      }
}

static void show_lpm(ivl_lpm_t net)
{
      unsigned idx;
      unsigned width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_ADD: {
		fprintf(out, "  LPM_ADD %s: <width=%u>\n",
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

	  case IVL_LPM_CMP_EQ: {
		fprintf(out, "  LPM_COMPARE(EQ) %s: <width=%u>\n",
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

	  case IVL_LPM_CMP_GE: {
		fprintf(out, "  LPM_COMPARE(GE) %s: <width=%u>\n",
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

	  case IVL_LPM_DECODE: {
		fprintf(out, "  LPM_DECODE %s (word-width=%u)\n",
			ivl_lpm_basename(net), ivl_lpm_width(net));
		for (idx = 0 ;  idx < ivl_lpm_selects(net) ;  idx += 1) {
		      ivl_nexus_t nex = ivl_lpm_select(net, idx);
		      fprintf(out, "    Address %u: %s\n", idx,
			      nex? ivl_nexus_name(nex) : "");
		}
		break;
	  }

	  case IVL_LPM_DEMUX:
	    show_lpm_demux(net);
	    break;

	  case IVL_LPM_RAM:
	    show_lpm_ram(net);
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

		if (ivl_lpm_decode(net)) {
		      ivl_lpm_t dec = ivl_lpm_decode(net);
		      fprintf(out, "    decoder: %s\n",
			      ivl_lpm_basename(dec));
		}

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

		if (ivl_lpm_sync_clr(net))
		      fprintf(out, "    Sclr: %s\n",
			      ivl_nexus_name(ivl_lpm_sync_clr(net)));

		if (ivl_lpm_async_set(net)) {
		      fprintf(out, "    Aset: %s\n",
			      ivl_nexus_name(ivl_lpm_async_set(net)));
		      if (ivl_lpm_aset_value(net))
			    show_expression(ivl_lpm_aset_value(net), 10);
		}

		if (ivl_lpm_sync_set(net)) {
		      fprintf(out, "    Sset: %s\n",
			      ivl_nexus_name(ivl_lpm_sync_set(net)));
		      if (ivl_lpm_sset_value(net))
			    show_expression(ivl_lpm_sset_value(net), 10);
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

		for (idx = 0 ;  idx < width ;  idx += 1) {
		      ivl_nexus_t nex = ivl_lpm_q(net, idx);
		      fprintf(out, "    Q %u: %s\n", idx,
			      nex? ivl_nexus_name(nex) : "");
		}

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
	    fprintf(out, "  LPM(%d) %s: <width=%u, signed=%d>\n",
		    ivl_lpm_type(net),
		    ivl_lpm_basename(net),
		    ivl_lpm_width(net),
		    ivl_lpm_signed(net));
      }

      for (idx = 0 ;  idx < ivl_lpm_attr_cnt(net) ;  idx += 1) {
	    ivl_attribute_t atr = ivl_lpm_attr_val(net,idx);
	    show_attribute(atr, 4);
      }
}

static void show_assign_lval(ivl_lval_t lval, unsigned ind)
{
      ivl_memory_t mem;
      ivl_variable_t var;

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
	    unsigned pp;
	    ivl_nexus_t nex = ivl_lval_pin(lval, 0);

	    fprintf(out, "%*spart_off=%u {%s", ind, "",
		    ivl_lval_part_off(lval),
		    ivl_nexus_name(nex));
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
	    show_attribute(attr,4);
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
      }

      for (pin = 0 ;  pin < ivl_signal_attr_cnt(net) ;  pin += 1) {
	    ivl_attribute_t atr = ivl_signal_attr_val(net, pin);

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

	  case IVL_LO_EEQ:
	    fprintf(out, "  EEQ %s (%s", name,
		    ivl_nexus_name(ivl_logic_pin(net,0)));
	    break;

	  case IVL_LO_NOT:
	    fprintf(out, "  not #(%u) %s (%s",
		    ivl_logic_delay(net, 0),
		    name,
		    ivl_nexus_name(ivl_logic_pin(net, 0)));
	    break;
	  case IVL_LO_NOR:
	    fprintf(out, "  nor %s (%s", name,
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
	    fprintf(out, "  unsupported gate(%u) %s (%s",
		    ivl_logic_type(net), name,
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

      fprintf(out, ");\n");

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
