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
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvp_process.c,v 1.79 2003/02/03 01:09:20 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>

static int show_statement(ivl_statement_t net, ivl_scope_t sscope);

unsigned local_count = 0;
unsigned thread_count = 0;

static unsigned transient_id = 0;

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
 * The set_to_lvariable function takes a particular nexus and generates
 * the %set statements to assign the value.
 *
 * The show_stmt_assign function looks at the assign statement, scans
 * the l-values, and matches bits of the r-value with the correct
 * nexus.
 */

static void set_to_lvariable(ivl_lval_t lval, unsigned idx,
			     unsigned bit, unsigned wid)
{
      ivl_signal_t sig  = ivl_lval_sig(lval);
      unsigned part_off = ivl_lval_part_off(lval);

      if (ivl_lval_mux(lval)) {
	    assert(wid == 1);
	    fprintf(vvp_out, "    %%set/x0 V_%s, %u, %u;\n",
		    vvp_signal_label(sig), bit, ivl_signal_pins(sig)-1);
      } else if (wid == 1) {
	    fprintf(vvp_out, "    %%set V_%s[%u], %u;\n",
		    vvp_signal_label(sig), idx+part_off, bit);

      } else {
	    fprintf(vvp_out, "    %%set/v V_%s[%u], %u, %u;\n",
		    vvp_signal_label(sig), idx+part_off, bit, wid);

      }
}

static void set_to_memory(ivl_memory_t mem, unsigned idx, unsigned bit)
{
      if (idx)
	    fprintf(vvp_out, "    %%ix/add 3, 1;\n");
      fprintf(vvp_out, "    %%set/m M_%s, %u;\n",
	      vvp_memory_label(mem), bit);
}

/*
 * This generates an assign to a single bit of an lvalue variable. If
 * the bit is a part select, then index the label to set the right
 * bit. If there is an lvalue mux, then use the indexed assign to make
 * a calculated assign.
 */
static void assign_to_lvariable(ivl_lval_t lval, unsigned idx,
				unsigned bit, unsigned delay,
				int delay_in_index_flag)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      unsigned part_off = ivl_lval_part_off(lval);

      char *delay_suffix = delay_in_index_flag? "/d" : "";

      if (ivl_lval_mux(lval))
	    fprintf(vvp_out, "    %%assign/x0%s V_%s, %u, %u;\n",
		    delay_suffix, vvp_signal_label(sig), delay, bit);
      else
	    fprintf(vvp_out, "    %%assign%s V_%s[%u], %u, %u;\n",
		    delay_suffix, vvp_signal_label(sig),
		    idx+part_off, delay, bit);
}

static void assign_to_lvector(ivl_lval_t lval, unsigned idx,
			      unsigned bit, unsigned delay, unsigned width)
{
      ivl_signal_t sig = ivl_lval_sig(lval);
      unsigned part_off = ivl_lval_part_off(lval);
      assert(ivl_lval_mux(lval) == 0);

      fprintf(vvp_out, "    %%ix/load 0, %u;\n", width);
      fprintf(vvp_out, "    %%assign/v0 V_%s[%u], %u, %u;\n",
	      vvp_signal_label(sig), part_off+idx, delay, bit);

}

static void assign_to_memory(ivl_memory_t mem, unsigned idx, 
			     unsigned bit, unsigned delay)
{
      if (idx)
	    fprintf(vvp_out, "    %%ix/add 3, 1;\n");
      fprintf(vvp_out, "    %%assign/m M_%s, %u, %u;\n",
	      vvp_memory_label(mem), delay, bit);
}

/*
 * This function, in addition to setting the value into index 0, sets
 * bit 4 to 1 if the value is unknown.
 */
static void calculate_into_x0(ivl_expr_t expr)
{
      struct vector_info vec = draw_eval_expr(expr, 0);
      fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", vec.base, vec.wid);
      clr_vector(vec);
}

static void calculate_into_x1(ivl_expr_t expr)
{
      struct vector_info vec = draw_eval_expr(expr, 0);
      fprintf(vvp_out, "    %%ix/get 1, %u, %u;\n", vec.base, vec.wid);
      clr_vector(vec);
}

static int show_stmt_assign_vector(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_memory_t mem;

	/* Handle the special case that the r-value is a constant. We
	   can generate the %set statement directly, without any worry
	   about generating code to evaluate the r-value expressions. */

      if (ivl_expr_type(rval) == IVL_EX_NUMBER) {
	    unsigned lidx;
	    const char*bits = ivl_expr_bits(rval);
	    unsigned wid = ivl_expr_width(rval);
	    unsigned cur_rbit = 0;

	    for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
		  unsigned skip_set = transient_id++;
		  unsigned skip_set_flag = 0;
		  unsigned idx;
		  unsigned bit_limit = wid - cur_rbit;
		  lval = ivl_stmt_lval(net, lidx);

		    /* If there is a mux for the lval, calculate the
		       value and write it into index0. */
		  if (ivl_lval_mux(lval)) {
			calculate_into_x0(ivl_lval_mux(lval));
			  /* Generate code to skip around the set
			     if the index has X values. */
			fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
			skip_set_flag = 1;
		  }

		  mem = ivl_lval_mem(lval);
		  if (mem) {
			draw_memory_index_expr(mem, ivl_lval_idx(lval));
			  /* Generate code to skip around the set
			     if the index has X values. */
			fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
			skip_set_flag = 1;
		  }

		  if (bit_limit > ivl_lval_pins(lval))
			bit_limit = ivl_lval_pins(lval);

		  if (mem) {
			for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			      set_to_memory(mem, idx,
					    bitchar_to_idx(bits[cur_rbit]));

			      cur_rbit += 1;
			}

			for (idx = bit_limit
				   ; idx < ivl_lval_pins(lval) ; idx += 1)
			      set_to_memory(mem, idx, 0);


		  } else {
			idx = 0;
			while (idx < bit_limit) {
			      unsigned cnt = 1;
			      while (((idx + cnt) < bit_limit)
				     && (bits[cur_rbit] == bits[cur_rbit+cnt]))
				    cnt += 1;

			      set_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]),
					       cnt);

			      cur_rbit += cnt;
			      idx += cnt;
			}


			if (bit_limit < ivl_lval_pins(lval)) {
			      unsigned cnt = ivl_lval_pins(lval) - bit_limit;
			      set_to_lvariable(lval, bit_limit, 0, cnt);
			}
		  }

		  if (skip_set_flag) {
			fprintf(vvp_out, "t_%u ;\n", skip_set);
			clear_expression_lookaside();
		  }
	    }

	    return 0;
      }

      { struct vector_info res = draw_eval_expr(rval, 0);
        unsigned wid = res.wid;
	unsigned lidx;
	unsigned cur_rbit = 0;

	for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	      unsigned skip_set = transient_id++;
	      unsigned skip_set_flag = 0;
	      unsigned idx;
	      unsigned bit_limit = wid - cur_rbit;
	      lval = ivl_stmt_lval(net, lidx);

		/* If there is a mux for the lval, calculate the
		   value and write it into index0. */
	      if (ivl_lval_mux(lval)) {
		    calculate_into_x0(ivl_lval_mux(lval));
		    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		    skip_set_flag = 1;
	      }

	      mem = ivl_lval_mem(lval);
	      if (mem) {
		    draw_memory_index_expr(mem, ivl_lval_idx(lval));
		    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		    skip_set_flag = 1;
	      }

	      if (bit_limit > ivl_lval_pins(lval))
		    bit_limit = ivl_lval_pins(lval);

	      if (mem) {
		    for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			  unsigned bidx = res.base < 4
				? res.base
				: (res.base+cur_rbit);
			  set_to_memory(mem, idx, bidx);
			  cur_rbit += 1;
		    }

		    for (idx = bit_limit;  idx < ivl_lval_pins(lval); idx += 1)
			  set_to_memory(mem, idx, 0);

	      } else {

		    unsigned bidx = res.base < 4
			  ? res.base
			  : (res.base+cur_rbit);
		    set_to_lvariable(lval, 0, bidx, bit_limit);
		    cur_rbit += bit_limit;

		    if (bit_limit < ivl_lval_pins(lval)) {
			  unsigned cnt = ivl_lval_pins(lval) - bit_limit;
			  set_to_lvariable(lval, bit_limit, 0, cnt);
		    }
	      }


	      if (skip_set_flag) {
		    fprintf(vvp_out, "t_%u ;\n", skip_set);
		    clear_expression_lookaside();
	      }
	}

	if (res.base > 3)
	      clr_vector(res);
      }


      return 0;
}

static int show_stmt_assign_real(ivl_statement_t net)
{
      int res;
      ivl_lval_t lval;
      ivl_variable_t var;

      res = draw_eval_real(ivl_stmt_rval(net));
      clr_word(res);

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);
      var = ivl_lval_var(lval);
      assert(var != 0);

      fprintf(vvp_out, "    %%set/wr W_%s, %d;\n",
	      vvp_word_label(var), res);

      return 0;
}

static int show_stmt_assign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_variable_t var;

      lval = ivl_stmt_lval(net, 0);

      if ( (var = ivl_lval_var(lval)) != 0 ) {
	    switch (ivl_variable_type(var)) {
		case IVL_VT_VOID:
		  assert(0);
		  return 1;

		case IVL_VT_VECTOR:
		    /* Can't happen. */
		  assert(0);
		  return 1;

		case IVL_VT_REAL:
		  return show_stmt_assign_real(net);

	    }

      } else {
	    return show_stmt_assign_vector(net);
      }

      return 0;
}

static int show_stmt_assign_nb(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_expr_t rval = ivl_stmt_rval(net);
      ivl_expr_t del  = ivl_stmt_delay_expr(net);
      ivl_memory_t mem;

      unsigned long delay = 0;
      if (del && (ivl_expr_type(del) == IVL_EX_ULONG)) {
	    delay = ivl_expr_uvalue(del);
	    del = 0;
      }

	/* Handle the special case that the r-value is a constant. We
	   can generate the %set statement directly, without any worry
	   about generating code to evaluate the r-value expressions. */

      if (ivl_expr_type(rval) == IVL_EX_NUMBER) {
	    unsigned lidx;
	    const char*bits = ivl_expr_bits(rval);
	    unsigned wid = ivl_expr_width(rval);
	    unsigned cur_rbit = 0;

	    if (del != 0)
		  calculate_into_x1(del);

	    for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
		  unsigned skip_set = transient_id++;
		  unsigned skip_set_flag = 0;
		  unsigned idx;
		  unsigned bit_limit = wid - cur_rbit;
		  lval = ivl_stmt_lval(net, lidx);

		    /* If there is a mux for the lval, calculate the
		       value and write it into index0. */
		  if (ivl_lval_mux(lval)) {
			calculate_into_x0(ivl_lval_mux(lval));
			fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
			skip_set_flag = 1;
		  }

		  mem = ivl_lval_mem(lval);
		  if (mem) {
			draw_memory_index_expr(mem, ivl_lval_idx(lval));
			fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
			skip_set_flag = 1;
		  }

		  if (bit_limit > ivl_lval_pins(lval))
			bit_limit = ivl_lval_pins(lval);

		  if (mem) {
			for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			      assign_to_memory(mem, idx, 
					       bitchar_to_idx(bits[cur_rbit]),
					       delay);
			      cur_rbit += 1;
			}

			for (idx = bit_limit
				   ; idx < ivl_lval_pins(lval)
				   ; idx += 1) {
			      assign_to_memory(mem, idx, 0, delay);
			}

		  } else if ((del == 0) && (bit_limit > 2)) {

			  /* We have a vector, but no runtime
			     calculated delays, to try to use vector
			     assign instructions. */
			idx = 0;
			while (idx < bit_limit) {
			      unsigned wid = 0;

			      do {
				    wid += 1;
				    if ((idx + wid) == bit_limit)
					  break;

			      } while (bits[cur_rbit] == bits[cur_rbit+wid]);

			      switch (wid) {
				  case 1:
				    assign_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]),
					       delay, 0);
				    break;
				  case 2:
				    assign_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]),
					       delay, 0);
				    assign_to_lvariable(lval, idx+1,
					       bitchar_to_idx(bits[cur_rbit]),
					       delay, 0);
				    break;
				  default:
				    assign_to_lvector(lval, idx,
					      bitchar_to_idx(bits[cur_rbit]),
					      delay, wid);
				    break;
			      }

			      idx += wid;
			      cur_rbit += wid;
			}

			if (bit_limit < ivl_lval_pins(lval)) {
			      unsigned wid = ivl_lval_pins(lval) - bit_limit;
			      assign_to_lvector(lval, bit_limit,
						0, delay, wid);
			}

		  } else {
			for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			      if (del != 0)
				    assign_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]),
					       1, 1);
			      else
				    assign_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]),
					       delay, 0);
			      cur_rbit += 1;
			}

			for (idx = bit_limit
				   ; idx < ivl_lval_pins(lval)
				   ; idx += 1) {
			      if (del != 0)
				    assign_to_lvariable(lval, idx, 0,
							1, 1);
			      else
				    assign_to_lvariable(lval, idx, 0,
							delay, 0);
			}
		  }

		  if (skip_set_flag) {
			fprintf(vvp_out, "t_%u ;\n", skip_set);
			clear_expression_lookaside();
		  }
	    }
	    return 0;
      }


      { struct vector_info res = draw_eval_expr(rval, 0);
        unsigned wid = res.wid;
	unsigned lidx;
	unsigned cur_rbit = 0;

	if (del != 0)
	      calculate_into_x1(del);

	for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	      unsigned skip_set = transient_id++;
	      unsigned skip_set_flag = 0;
	      unsigned idx;
	      unsigned bit_limit = wid - cur_rbit;
	      lval = ivl_stmt_lval(net, lidx);

		/* If there is a mux for the lval, calculate the
		   value and write it into index0. */
	      if (ivl_lval_mux(lval)) {
		    calculate_into_x0(ivl_lval_mux(lval));
		    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		    skip_set_flag = 1;
	      }

	      mem = ivl_lval_mem(lval);
	      if (mem) {
		    draw_memory_index_expr(mem, ivl_lval_idx(lval));
		    fprintf(vvp_out, "    %%jmp/1 t_%u, 4;\n", skip_set);
		    skip_set_flag = 1;
	      }

	      if (bit_limit > ivl_lval_pins(lval))
		    bit_limit = ivl_lval_pins(lval);

	      if ((bit_limit > 2) && (mem == 0) && (del == 0)) {

		    unsigned bidx = res.base < 4
			  ? res.base
			  : (res.base+cur_rbit);
		    assign_to_lvector(lval, 0, bidx, delay, bit_limit);
		    cur_rbit += bit_limit;

	      } else {
		    for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			  unsigned bidx = res.base < 4
				? res.base
				: (res.base+cur_rbit);
			  if (mem)
				assign_to_memory(mem, idx, bidx, delay);
			  else if (del != 0)
				assign_to_lvariable(lval, idx, bidx,
						    1, 1);
			  else
				assign_to_lvariable(lval, idx, bidx,
						    delay, 0);

			  cur_rbit += 1;
		    }
	      }

	      for (idx = bit_limit; idx < ivl_lval_pins(lval); idx += 1)
			  if (mem)
				assign_to_memory(mem, idx, 0, delay);
			  else if (del != 0)
				assign_to_lvariable(lval, idx, 0, 1, 1);
			  else
				assign_to_lvariable(lval, idx, 0, delay, 0);


	      if (skip_set_flag) {
		    fprintf(vvp_out, "t_%u ;\n", skip_set);
		    clear_expression_lookaside();
	      }
	}

	if (res.base > 3)
	      clr_vector(res);
      }

      return 0;
}

static int show_stmt_block(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned idx;
      unsigned cnt = ivl_stmt_block_count(net);

      for (idx = 0 ;  idx < cnt ;  idx += 1) {
	    rc += show_statement(ivl_stmt_block_stmt(net, idx), sscope);
      }

      return rc;
}

/*
 * This draws an invocation of a named block. This is a little
 * different because a subscope is created. We do that by creating
 * a thread to deal with this.
 */
static int show_stmt_block_named(ivl_statement_t net, ivl_scope_t scope)
{
      int rc;
      int out_id, sub_id;
      ivl_scope_t subscope = ivl_stmt_block_scope(net);

      out_id = transient_id++;
      sub_id = transient_id++;

      fprintf(vvp_out, "    %%fork t_%u, S_%s;\n",
	      sub_id, vvp_mangle_id(ivl_scope_name(subscope)));
      fprintf(vvp_out, "    %%jmp t_%u;\n", out_id);
      fprintf(vvp_out, "t_%u ;\n", sub_id);

      rc = show_stmt_block(net, subscope);
      fprintf(vvp_out, "    %%end;\n");

      fprintf(vvp_out, "t_%u %%join;\n", out_id);
      clear_expression_lookaside();

      return rc;
}


static int show_stmt_case(ivl_statement_t net, ivl_scope_t sscope)
{
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(exp, 0);
      unsigned count = ivl_stmt_case_count(net);

      unsigned local_base = local_count;

      unsigned idx, default_case;

      local_count += count + 1;

	/* First draw the branch table.  All the non-default cases
	   generate a branch out of here, to the code that implements
	   the case. The default will fall through all the tests. */
      default_case = count;

      for (idx = 0 ;  idx < count ;  idx += 1) {
	    ivl_expr_t cex = ivl_stmt_case_expr(net, idx);
	    struct vector_info cvec;

	    if (cex == 0) {
		  default_case = idx;
		  continue;
	    }

	      /* Is the guard expression something I can pass to a
		 %cmpi/u instruction? If so, use that instead. */

	    if ((ivl_statement_type(net) == IVL_ST_CASE)
		&& (ivl_expr_type(cex) == IVL_EX_NUMBER)
		&& (! number_is_unknown(cex))
		&& number_is_immediate(cex, 16)) {

		  unsigned long imm = get_number_immediate(cex);

		  fprintf(vvp_out, "    %%cmpi/u %u, %lu, %u;\n",
			  cond.base, imm, cond.wid);
		  fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 6;\n",
			  thread_count, local_base+idx);

		  continue;
	    }

	      /* Oh well, do this case the hard way. */

	    cvec = draw_eval_expr_wid(cex, cond.wid, 0);
	    assert(cvec.wid == cond.wid);

	    switch (ivl_statement_type(net)) {

		case IVL_ST_CASE:
		  fprintf(vvp_out, "    %%cmp/u %u, %u, %u;\n",
			  cond.base, cvec.base, cond.wid);
		  fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 6;\n",
			  thread_count, local_base+idx);
		  break;

		case IVL_ST_CASEX:
		  fprintf(vvp_out, "    %%cmp/x %u, %u, %u;\n",
			  cond.base, cvec.base, cond.wid);
		  fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 4;\n",
			  thread_count, local_base+idx);
		  break;

		case IVL_ST_CASEZ:
		  fprintf(vvp_out, "    %%cmp/z %u, %u, %u;\n",
			  cond.base, cvec.base, cond.wid);
		  fprintf(vvp_out, "    %%jmp/1 T_%d.%d, 4;\n",
			  thread_count, local_base+idx);
		  break;

		default:
		  assert(0);
	    }
	    
	      /* Done with the case expression */
	    clr_vector(cvec);
      }

	/* Done with the condition expression */
      clr_vector(cond);

	/* Emit code for the default case. */
      if (default_case < count) {
	    ivl_statement_t cst = ivl_stmt_case_stmt(net, default_case);
	    show_statement(cst, sscope);
      }

	/* Jump to the out of the case. */
      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count,
	      local_base+count);

      for (idx = 0 ;  idx < count ;  idx += 1) {
	    ivl_statement_t cst = ivl_stmt_case_stmt(net, idx);

	    if (idx == default_case)
		  continue;

	    fprintf(vvp_out, "T_%d.%d ;\n", thread_count, local_base+idx);
	    clear_expression_lookaside();
	    show_statement(cst, sscope);

	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count,
		    local_base+count);

      }


	/* The out of the case. */
      fprintf(vvp_out, "T_%d.%d ;\n",  thread_count, local_base+count);
      clear_expression_lookaside();

      return 0;
}

static int show_stmt_cassign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t lsig;
      unsigned idx;
      char*tmp_label;

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_signal_pins(lsig) == ivl_stmt_nexus_count(net));
      assert(ivl_lval_part_off(lval) == 0);

      tmp_label = strdup(vvp_signal_label(lsig));
      for (idx = 0 ;  idx < ivl_stmt_nexus_count(net) ;  idx += 1) {
	    fprintf(vvp_out, "    %%cassign V_%s[%u], %s;\n",
		    tmp_label, idx,
		    draw_net_input(ivl_stmt_nexus(net, idx)));
      }
      free(tmp_label);

      return 0;
}

static int show_stmt_deassign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t lsig;
      unsigned idx;

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_part_off(lval) == 0);

      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "    %%deassign V_%s[%u], 1;\n",
		    vvp_signal_label(lsig), idx);
      }
      return 0;
}

static int show_stmt_condit(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_false, lab_out;
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(exp, STUFF_OK_XZ|STUFF_OK_47);

      assert(cond.wid == 1);

      lab_false = local_count++;
      lab_out = local_count++;

      fprintf(vvp_out, "    %%jmp/0xz  T_%d.%d, %u;\n",
	      thread_count, lab_false, cond.base);

	/* Done with the condition expression. */
      if (cond.base >= 8)
	    clr_vector(cond);

      if (ivl_stmt_cond_true(net))
	    rc += show_statement(ivl_stmt_cond_true(net), sscope);


      if (ivl_stmt_cond_false(net)) {
	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, lab_out);
	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_false);
	    clear_expression_lookaside();

	    rc += show_statement(ivl_stmt_cond_false(net), sscope);

	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_out);
	    clear_expression_lookaside();

      } else {
	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_false);
	    clear_expression_lookaside();
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
static int show_stmt_delay(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned long delay = ivl_stmt_delay_val(net);
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);

      fprintf(vvp_out, "    %%delay %lu;\n", delay);
	/* Lots of things can happen during a delay. */
      clear_expression_lookaside();

      rc += show_statement(stmt, sscope);

      return rc;
}

/*
 * The delayx statement is slightly more complex in that it is
 * necessary to calculate the delay first. Load the calculated delay
 * into and index register and use the %delayx instruction to do the
 * actual delay.
 */
static int show_stmt_delayx(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      ivl_expr_t exp = ivl_stmt_delay_expr(net);
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);

      switch (ivl_expr_value(exp)) {

	  case IVL_VT_VECTOR: {
		struct vector_info del = draw_eval_expr(exp, 0);
		fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n",
			del.base, del.wid);
		clr_vector(del);
		break;
	  }

	  case IVL_VT_REAL: {
		int word = draw_eval_real(exp);
		fprintf(vvp_out, "    %%cvt/ir 0, %d;\n", word);
		clr_word(word);
		break;
	  }

	  default:
	    assert(0);
      }

      fprintf(vvp_out, "    %%delayx 0;\n");
	/* Lots of things can happen during a delay. */
      clear_expression_lookaside();

      rc += show_statement(stmt, sscope);
      return rc;
}

static int show_stmt_disable(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;

      ivl_scope_t target = ivl_stmt_call(net);
      fprintf(vvp_out, "    %%disable S_%s;\n", 
	      vvp_mangle_id(ivl_scope_name(target)));

      return rc;
}

static int show_stmt_force(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t lsig;
      unsigned idx;
      static unsigned force_functor_label = 0;

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_part_off(lval) == 0);

      force_functor_label += 1;
      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "f_%u.%u .force V_%s[%u], %s;\n",
		    force_functor_label, idx,
		    vvp_signal_label(lsig), idx,
		    draw_net_input(ivl_stmt_nexus(net, idx)));
      }

      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "    %%force f_%u.%u, 1;\n",
		    force_functor_label, idx);
      }
      return 0;
}

static int show_stmt_forever(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      ivl_statement_t stmt = ivl_stmt_sub_stmt(net);
      unsigned lab_top = local_count++;

      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_top);
      rc += show_statement(stmt, sscope);
      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_top);

      return rc;
}

static int show_stmt_fork(ivl_statement_t net, ivl_scope_t sscope)
{
      unsigned idx;
      int rc = 0;
      unsigned cnt = ivl_stmt_block_count(net);

      unsigned out = transient_id++;
      unsigned id_base = transient_id;
      transient_id += cnt-1;


	/* Draw a fork statement for all but one of the threads of the
	   fork/join. Send the threads off to a bit of code where they
	   are implemented. */
      for (idx = 0 ;  idx < cnt-1 ;  idx += 1) {
	    fprintf(vvp_out, "    %%fork t_%u, S_%s;\n",
		    id_base+idx, 
		    vvp_mangle_id(ivl_scope_name(sscope)));
      }

	/* Draw code to execute the remaining thread in the current
	   thread, then generate enough joins to merge back together. */
      rc += show_statement(ivl_stmt_block_stmt(net, cnt-1), sscope);

      for (idx = 0 ;  idx < cnt-1 ;  idx += 1) {
	    fprintf(vvp_out, "    %%join;\n");
      }
      fprintf(vvp_out, "    %%jmp t_%u;\n", out);

      for (idx = 0 ;  idx < cnt-1 ;  idx += 1) {
	    fprintf(vvp_out, "t_%u ;\n", id_base+idx);
	    clear_expression_lookaside();
	    rc += show_statement(ivl_stmt_block_stmt(net, idx), sscope);
	    fprintf(vvp_out, "    %%end;\n");
      }

	/* This is the label for the out. Use this to branch around
	   the implementations of all the child threads. */
      clear_expression_lookaside();
      fprintf(vvp_out, "t_%u ;\n", out);

      return rc;
}

/*
 * noop statements are implemented by doing nothing.
 */
static int show_stmt_noop(ivl_statement_t net)
{
      return 0;
}

static int show_stmt_release(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t lsig;
      unsigned idx;

	/* If there are no l-vals (the target signal has been elided)
	   then turn the release into a no-op. In other words, we are
	   done before we start. */
      if (ivl_stmt_lvals(net) == 0)
	    return 0;

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_part_off(lval) == 0);

	/* On release, reg variables hold the value that was forced on
	   to them. */
      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    if (ivl_signal_type(lsig) == IVL_SIT_REG) {
		  fprintf(vvp_out, "    %%load 4, V_%s[%u];\n",
			  vvp_signal_label(lsig), idx);
		  fprintf(vvp_out, "    %%set V_%s[%u], 4;\n",
			  vvp_signal_label(lsig), idx);
	    }
	    fprintf(vvp_out, "    %%release V_%s[%u];\n",
		    vvp_signal_label(lsig), idx);
      }

      return 0;
}

static int show_stmt_repeat(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_top = local_count++, lab_out = local_count++;
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cnt = draw_eval_expr(exp, 0);

	/* Test that 0 < expr */
      fprintf(vvp_out, "T_%u.%u %%cmp/u 0, %u, %u;\n", thread_count,
	      lab_top, cnt.base, cnt.wid);
      clear_expression_lookaside();
      fprintf(vvp_out, "    %%jmp/0xz T_%u.%u, 5;\n", thread_count, lab_out);
	/* This adds -1 (all ones in 2's complement) to the count. */
      fprintf(vvp_out, "    %%add %u, 1, %u;\n", cnt.base, cnt.wid);

      rc += show_statement(ivl_stmt_sub_stmt(net), sscope);

      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_top);
      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_out);
      clear_expression_lookaside();

      clr_vector(cnt);

      return rc;
}

static int show_stmt_trigger(ivl_statement_t net)
{
      ivl_event_t ev = ivl_stmt_event(net);
      assert(ev);
      fprintf(vvp_out, "    %%set E_%s, 0;\n", 
	      vvp_mangle_id(ivl_event_name(ev)));
      return 0;
}

static int show_stmt_utask(ivl_statement_t net)
{
      ivl_scope_t task = ivl_stmt_call(net);

      fprintf(vvp_out, "    %%fork TD_%s",
	      vvp_mangle_id(ivl_scope_name(task)));
      fprintf(vvp_out, ", S_%s;\n", 
	      vvp_mangle_id(ivl_scope_name(task)));
      fprintf(vvp_out, "    %%join;\n");
      clear_expression_lookaside();
      return 0;
}

static int show_stmt_wait(ivl_statement_t net, ivl_scope_t sscope)
{
      ivl_event_t ev = ivl_stmt_event(net);
      fprintf(vvp_out, "    %%wait E_%s;\n", 
	      vvp_mangle_id(ivl_event_name(ev)));

	/* Always clear the expression lookaside after a
	   %wait. Anything can happen while the thread is waiting. */
      clear_expression_lookaside();

      return show_statement(ivl_stmt_sub_stmt(net), sscope);
}

static struct vector_info reduction_or(struct vector_info cvec)
{
      struct vector_info result;

      switch (cvec.base) {
	  case 0:
	    result.base = 0;
	    result.wid = 1;
	    break;
	  case 1:
	    result.base = 1;
	    result.wid = 1;
	    break;
	  case 2:
	  case 3:
	    result.base = 0;
	    result.wid = 1;
	    break;
	  default:
	    clr_vector(cvec);
	    result.base = allocate_vector(1);
	    result.wid = 1;
	    fprintf(vvp_out, "    %%or/r %u, %u, %u;\n", result.base,
		    cvec.base, cvec.wid);
	    break;
      }

      return result;
}

static int show_stmt_while(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      struct vector_info cvec;

      unsigned top_label = local_count++;
      unsigned out_label = local_count++;

	/* Start the loop. The top of the loop starts a basic block
	   because it can be entered from above or from the bottom of
	   the loop. */
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, top_label);
      clear_expression_lookaside();

	/* Draw the evaluation of the condition expression, and test
	   the result. If the expression evaluates to false, then
	   branch to the out label. */
      cvec = draw_eval_expr(ivl_stmt_cond_expr(net), STUFF_OK_XZ|STUFF_OK_47);
      if (cvec.wid > 1)
	    cvec = reduction_or(cvec);

      fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, %u;\n",
	      thread_count, out_label, cvec.base);
      if (cvec.base >= 8)
	    clr_vector(cvec);

	/* Draw the body of the loop. */
      rc += show_statement(ivl_stmt_sub_stmt(net), sscope);

	/* This is the bottom of the loop. branch to the top where the
	   test is repeased, and also draw the out label. */
      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, top_label);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, out_label);
      clear_expression_lookaside();
      return rc;
}

static int show_system_task_call(ivl_statement_t net)
{
      unsigned idx;
      unsigned parm_count = ivl_stmt_parm_count(net);
      struct vector_info *vec = 0x0;
      unsigned int vecs= 0;
      unsigned int veci= 0;
      
      if (parm_count == 0) {
	    fprintf(vvp_out, "    %%vpi_call \"%s\";\n", ivl_stmt_name(net));
	    clear_expression_lookaside();
	    return 0;
      }

	/* Figure out how many expressions are going to be evaluated
	   for this task call. I won't need to evaluate expressions
	   for items that are VPI objects directly. */
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_stmt_parm(net, idx);
	    
	    switch (ivl_expr_type(expr)) {

		    /* These expression types can be handled directly,
		       with VPI handles of their own. Therefore, skip
		       them in the process of evaluating expressions. */
		case IVL_EX_NONE:
		case IVL_EX_NUMBER:
		case IVL_EX_STRING:
		case IVL_EX_SCOPE:
		case IVL_EX_SFUNC:
		case IVL_EX_VARIABLE:
		  continue;

		case IVL_EX_SIGNAL:
		    /* If the signal node is narrower then the signal
		       itself, then this is a part select so I'm going
		       to need to evaluate the expression.

		       If I don't need to do any evaluating, then skip
		       it as I'll be passing the handle to the signal
		       itself. */
		  if (ivl_expr_width(expr) !=
		      ivl_signal_pins(ivl_expr_signal(expr))) {
			break;
		  } else {
			continue;
		  }


		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			continue;
		  }

		    /* Everything else will need to be evaluated and
		       passed as a constant to the vpi task. */
		default:
		  break;
	    }

	    vec = (struct vector_info *)
		  realloc(vec, (vecs+1)*sizeof(struct vector_info));

	    switch (ivl_expr_value(expr)) {
		case IVL_VT_VECTOR:
		  vec[vecs] = draw_eval_expr(expr, 0);
		  break;
		case IVL_VT_REAL:
		  vec[vecs].base = draw_eval_real(expr);
		  vec[vecs].wid = 0;
		  break;
		default:
		  assert(0);
	    }
	    vecs++;
      }
      
      fprintf(vvp_out, "    %%vpi_call \"%s\"", ivl_stmt_name(net));
      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_stmt_parm(net, idx);

	    switch (ivl_expr_type(expr)) {
		case IVL_EX_NONE:
		  fprintf(vvp_out, ", \" \"");
		  continue;

		case IVL_EX_NUMBER: {
		      unsigned bit, wid = ivl_expr_width(expr);
		      const char*bits = ivl_expr_bits(expr);

		      fprintf(vvp_out, ", %u'%sb", wid,
			      ivl_expr_signed(expr)? "s" : "");
		      for (bit = wid ;  bit > 0 ;  bit -= 1)
			    fputc(bits[bit-1], vvp_out);
		      continue;
		}

		case IVL_EX_SIGNAL:
		    /* If this is a part select, then the value was
		       calculated above. Otherwise, just pass the
		       signal. */
		  if (ivl_expr_width(expr) !=
		      ivl_signal_pins(ivl_expr_signal(expr))) {
			break;
		  } else {
			fprintf(vvp_out, ", V_%s", 
				vvp_signal_label(ivl_expr_signal(expr)));
			continue;
		  }

		case IVL_EX_VARIABLE: {
		      ivl_variable_t var = ivl_expr_variable(expr);
		      fprintf(vvp_out, ", W_%s", vvp_word_label(var));
		      continue;
		}

		case IVL_EX_STRING:
		  fprintf(vvp_out, ", \"%s\"", 
			  ivl_expr_string(expr));
		  continue;

		case IVL_EX_SCOPE:
		  fprintf(vvp_out, ", S_%s",
			  vvp_mangle_id(ivl_scope_name(ivl_expr_scope(expr))));
		  continue;

		case IVL_EX_SFUNC:
		  if (strcmp("$time", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $time");
		  else if (strcmp("$stime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $stime");
		  else if (strcmp("$realtime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $realtime");
		  else if (strcmp("$simtime", ivl_expr_name(expr)) == 0)
			fprintf(vvp_out, ", $simtime");
		  else
			fprintf(vvp_out, ", ?%s?", ivl_expr_name(expr));
		  continue;
		  
		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			fprintf(vvp_out, ", M_%s", 
				vvp_memory_label(ivl_expr_memory(expr)));
			continue;
		  }
		  break;

		default:
		  break;
	    }
	    assert(veci < vecs);

	    switch (ivl_expr_value(expr)) {

		case IVL_VT_VECTOR:
		  fprintf(vvp_out, ", T<%u,%u,%s>", vec[veci].base,
			  vec[veci].wid, ivl_expr_signed(expr)? "s" : "u");
		  break;

		case IVL_VT_REAL:
		  fprintf(vvp_out, ", W<%u,r>", vec[veci].base);
		  break;

		default:
		  assert(0);
	    }
	    veci++;
      }
      
      assert(veci == vecs);

      if (vecs) {
	    for (idx = 0; idx < vecs; idx++) {
		  if (vec[idx].wid > 0)
			clr_vector(vec[idx]);
	    }
	    free(vec);
      }

      fprintf(vvp_out, ";\n");

	/* VPI calls can manipulate anything, so clear the expression
	   lookahead table after the call. */
      clear_expression_lookaside();

      return 0;
}

/*
 * This function draws a statement as vvp assembly. It basically
 * switches on the statement type and draws code based on the type and
 * further specifics.
 */
static int show_statement(ivl_statement_t net, ivl_scope_t sscope)
{
      const ivl_statement_type_t code = ivl_statement_type(net);
      int rc = 0;

      switch (code) {

	  case IVL_ST_ASSIGN:
	    rc += show_stmt_assign(net);
	    break;

	  case IVL_ST_ASSIGN_NB:
	    rc += show_stmt_assign_nb(net);
	    break;

	  case IVL_ST_BLOCK:
	    if (ivl_stmt_block_scope(net))
		  rc += show_stmt_block_named(net, sscope);
	    else
		  rc += show_stmt_block(net, sscope);
	    break;

	  case IVL_ST_CASE:
	  case IVL_ST_CASEX:
	  case IVL_ST_CASEZ:
	    rc += show_stmt_case(net, sscope);
	    break;

	  case IVL_ST_CASSIGN:
	    rc += show_stmt_cassign(net);
	    break;

	  case IVL_ST_CONDIT:
	    rc += show_stmt_condit(net, sscope);
	    break;

	  case IVL_ST_DEASSIGN:
	    rc += show_stmt_deassign(net);
	    break;

	  case IVL_ST_DELAY:
	    rc += show_stmt_delay(net, sscope);
	    break;

	  case IVL_ST_DELAYX:
	    rc += show_stmt_delayx(net, sscope);
	    break;

	  case IVL_ST_DISABLE:
	    rc += show_stmt_disable(net, sscope);
	    break;

	  case IVL_ST_FORCE:
	    rc += show_stmt_force(net);
	    break;

	  case IVL_ST_FOREVER:
	    rc += show_stmt_forever(net, sscope);
	    break;

	  case IVL_ST_FORK:
	    rc += show_stmt_fork(net, sscope);
	    break;

	  case IVL_ST_NOOP:
	    rc += show_stmt_noop(net);
	    break;

	  case IVL_ST_RELEASE:
	    rc += show_stmt_release(net);
	    break;

	  case IVL_ST_REPEAT:
	    rc += show_stmt_repeat(net, sscope);
	    break;

	  case IVL_ST_STASK:
	    rc += show_system_task_call(net);
	    break;

	  case IVL_ST_TRIGGER:
	    rc += show_stmt_trigger(net);
	    break;

	  case IVL_ST_UTASK:
	    rc += show_stmt_utask(net);
	    break;

	  case IVL_ST_WAIT:
	    rc += show_stmt_wait(net, sscope);
	    break;

	  case IVL_ST_WHILE:
	    rc += show_stmt_while(net, sscope);
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
      ivl_statement_t stmt = ivl_process_stmt(net);

      local_count = 0;
      fprintf(vvp_out, "    .scope S_%s;\n", 
	      vvp_mangle_id(ivl_scope_name(scope)));

	/* Generate the entry label. Just give the thread a number so
	   that we ar certain the label is unique. */
      fprintf(vvp_out, "T_%d ;\n", thread_count);
      clear_expression_lookaside();

	/* Draw the contents of the thread. */
      rc += show_statement(stmt, scope);


	/* Terminate the thread with either an %end instruction (initial
	   statements) or a %jmp back to the beginning of the thread. */

      switch (ivl_process_type(net)) {

	  case IVL_PR_INITIAL:
	    fprintf(vvp_out, "    %%end;\n");
	    break;

	  case IVL_PR_ALWAYS:
	    fprintf(vvp_out, "    %%jmp T_%d;\n", thread_count);
	    break;
      }

	/* Now write out the .thread directive that tells vvp where
	   the thread starts. */
      fprintf(vvp_out, "    .thread T_%d;\n", thread_count);


      thread_count += 1;
      return rc;
}

int draw_task_definition(ivl_scope_t scope)
{
      int rc = 0;
      ivl_statement_t def = ivl_scope_def(scope);

      fprintf(vvp_out, "TD_%s ;\n", vvp_mangle_id(ivl_scope_name(scope)));
      clear_expression_lookaside();

      assert(def);
      rc += show_statement(def, scope);

      fprintf(vvp_out, "    %%end;\n");

      thread_count += 1;
      return rc;
}

int draw_func_definition(ivl_scope_t scope)
{
      int rc = 0;
      ivl_statement_t def = ivl_scope_def(scope);

      fprintf(vvp_out, "TD_%s ;\n", vvp_mangle_id(ivl_scope_name(scope)));
      clear_expression_lookaside();

      assert(def);
      rc += show_statement(def, scope);

      fprintf(vvp_out, "    %%end;\n");

      thread_count += 1;
      return rc;
}

/*
 * $Log: vvp_process.c,v $
 * Revision 1.79  2003/02/03 01:09:20  steve
 *  Allow $display of $simtime.
 *
 * Revision 1.78  2003/01/27 00:14:37  steve
 *  Support in various contexts the $realtime
 *  system task.
 *
 * Revision 1.77  2003/01/26 21:16:00  steve
 *  Rework expression parsing and elaboration to
 *  accommodate real/realtime values and expressions.
 *
 * Revision 1.76  2002/11/21 22:43:13  steve
 *  %set/x0 instruction to support bounds checking.
 *
 * Revision 1.75  2002/11/17 18:31:09  steve
 *  Generate unique labels for force functors.
 *
 * Revision 1.74  2002/11/08 05:00:31  steve
 *  Use the vectorized %assign where appropriate.
 *
 * Revision 1.73  2002/11/07 05:19:55  steve
 *  Use Vector %set to set constants in variables.
 *
 * Revision 1.72  2002/11/07 03:12:18  steve
 *  Vectorize load from REG variables.
 *
 * Revision 1.71  2002/09/27 20:24:42  steve
 *  Allow expression lookaside map to spam statements.
 *
 * Revision 1.70  2002/09/27 16:33:34  steve
 *  Add thread expression lookaside map.
 *
 * Revision 1.69  2002/09/24 04:20:32  steve
 *  Allow results in register bits 47 in certain cases.
 *
 * Revision 1.68  2002/09/13 03:12:50  steve
 *  Optimize ==1 when in context where x vs z doesnt matter.
 *
 * Revision 1.67  2002/09/01 00:19:35  steve
 *  Watch for x indices in l-value of non-blocking assignments.
 *
 * Revision 1.66  2002/08/31 03:48:50  steve
 *  Fix reverse bit ordered bit select in continuous assignment.
 *
 * Revision 1.65  2002/08/27 05:39:57  steve
 *  Fix l-value indexing of memories and vectors so that
 *  an unknown (x) index causes so cell to be addresses.
 *
 *  Fix tangling of label identifiers in the fork-join
 *  code generator.
 *
 * Revision 1.64  2002/08/19 00:06:12  steve
 *  Allow release to handle removal of target net.
 *
 * Revision 1.63  2002/08/12 01:35:04  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.62  2002/08/07 00:54:39  steve
 *  Add force to nets.
 *
 * Revision 1.61  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.60  2002/08/03 22:30:48  steve
 *  Eliminate use of ivl_signal_name for signal labels.
 *
 * Revision 1.59  2002/06/02 18:57:17  steve
 *  Generate %cmpi/u where appropriate.
 *
 * Revision 1.58  2002/05/27 00:08:45  steve
 *  Support carrying the scope of named begin-end
 *  blocks down to the code generator, and have
 *  the vvp code generator use that to support disable.
 *
 * Revision 1.57  2002/04/22 02:41:30  steve
 *  Reduce the while loop expression if needed.
 *
 * Revision 1.56  2002/04/21 22:31:02  steve
 *  Redo handling of assignment internal delays.
 *  Leave it possible for them to be calculated
 *  at run time.
 *
 * Revision 1.55  2002/04/14 19:19:21  steve
 *  Handle empty true case of conditional statements.
 *
 * Revision 1.54  2002/04/14 03:54:40  steve
 *  Vector constants to vpi_call can have sign.
 *
 * Revision 1.53  2002/04/14 02:56:19  steve
 *  Support signed expressions through to VPI.
 *
 * Revision 1.52  2002/01/11 05:23:05  steve
 *  Handle certain special cases of stime.
 *
 * Revision 1.51  2001/12/05 05:41:20  steve
 *  Make sure fork labels are globally unique.
 *
 * Revision 1.50  2001/11/18 01:28:18  steve
 *  Generate force code for variable l-values.
 *
 * Revision 1.49  2001/11/14 03:28:49  steve
 *  DLL target support for force and release.
 *
 * Revision 1.48  2001/11/01 19:31:40  steve
 *  make fork label into complete statemnt.
 *
 * Revision 1.47  2001/11/01 04:26:57  steve
 *  Generate code for deassign and cassign.
 *
 * Revision 1.46  2001/10/19 23:52:36  steve
 *  Add trailing ; to fork-join out labels.
 *
 * Revision 1.45  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 *
 * Revision 1.44  2001/09/01 00:58:16  steve
 *  dead comments.
 */

