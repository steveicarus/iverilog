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
#ident "$Id: vvp_process.c,v 1.57 2002/04/22 02:41:30 steve Exp $"
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

static void set_to_lvariable(ivl_lval_t lval, unsigned idx, unsigned bit)
{
      ivl_signal_t sig  = ivl_lval_sig(lval);
      unsigned part_off = ivl_lval_part_off(lval);

      if (ivl_lval_mux(lval))
	    fprintf(vvp_out, "    %%set/x V_%s, %u, 0;\n",
		    vvp_mangle_id(ivl_signal_name(sig)), bit);
      else
	    fprintf(vvp_out, "    %%set V_%s[%u], %u;\n",
		    vvp_mangle_id(ivl_signal_name(sig)), idx+part_off, bit);
}

static void set_to_memory(ivl_memory_t mem, unsigned idx, unsigned bit)
{
      if (idx)
	    fprintf(vvp_out, "    %%ix/add 3, 1;\n");
      fprintf(vvp_out, "    %%set/m M_%s, %u;\n",
	      vvp_mangle_id(ivl_memory_name(mem)), bit);
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
		    delay_suffix,
		    vvp_mangle_id(ivl_signal_name(sig)), delay, bit);
      else
	    fprintf(vvp_out, "    %%assign%s V_%s[%u], %u, %u;\n",
		    delay_suffix,
		    vvp_mangle_id(ivl_signal_name(sig)),
		    idx+part_off, delay, bit);
}

static void assign_to_memory(ivl_memory_t mem, unsigned idx, 
			     unsigned bit, unsigned delay)
{
      if (idx)
	    fprintf(vvp_out, "    %%ix/add 3, 1;\n");
      fprintf(vvp_out, "    %%assign/m M_%s, %u, %u;\n",
	      vvp_mangle_id(ivl_memory_name(mem)), delay, bit);
}

static void calculate_into_x0(ivl_expr_t expr)
{
      struct vector_info vec = draw_eval_expr(expr);
      fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", vec.base, vec.wid);
      clr_vector(vec);
}

static void calculate_into_x1(ivl_expr_t expr)
{
      struct vector_info vec = draw_eval_expr(expr);
      fprintf(vvp_out, "    %%ix/get 1, %u, %u;\n", vec.base, vec.wid);
      clr_vector(vec);
}

static int show_stmt_assign(ivl_statement_t net)
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
		  unsigned idx;
		  unsigned bit_limit = wid - cur_rbit;
		  lval = ivl_stmt_lval(net, lidx);

		    /* If there is a mux for the lval, calculate the
		       value and write it into index0. */
		  if (ivl_lval_mux(lval))
			calculate_into_x0(ivl_lval_mux(lval));

		  mem = ivl_lval_mem(lval);
		  if (mem) 
			draw_memory_index_expr(mem, ivl_lval_idx(lval));

		  if (bit_limit > ivl_lval_pins(lval))
			bit_limit = ivl_lval_pins(lval);

		  for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			if (mem)
			      set_to_memory(mem, idx,
					    bitchar_to_idx(bits[cur_rbit]));
			else
			      set_to_lvariable(lval, idx,
					       bitchar_to_idx(bits[cur_rbit]));

			cur_rbit += 1;
		  }

		  for (idx = bit_limit ; idx < ivl_lval_pins(lval) ; idx += 1)
			if (mem)
			      set_to_memory(mem, idx, 0);
			else
			      set_to_lvariable(lval, idx, 0);

	    }

	    return 0;
      }

      { struct vector_info res = draw_eval_expr(rval);
        unsigned wid = res.wid;
	unsigned lidx;
	unsigned cur_rbit = 0;

	for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	      unsigned idx;
	      unsigned bit_limit = wid - cur_rbit;
	      lval = ivl_stmt_lval(net, lidx);

		/* If there is a mux for the lval, calculate the
		   value and write it into index0. */
	      if (ivl_lval_mux(lval))
		    calculate_into_x0(ivl_lval_mux(lval));

	      mem = ivl_lval_mem(lval);
	      if (mem) 
		    draw_memory_index_expr(mem, ivl_lval_idx(lval));

	      if (bit_limit > ivl_lval_pins(lval))
		    bit_limit = ivl_lval_pins(lval);

	      for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
		    unsigned bidx = res.base < 4
			  ? res.base
			  : (res.base+cur_rbit);
		    if (mem)
			  set_to_memory(mem, idx, bidx);
		    else
			  set_to_lvariable(lval, idx, bidx);

		    cur_rbit += 1;
	      }

	      for (idx = bit_limit ; idx < ivl_lval_pins(lval) ; idx += 1)
		    if (mem)
			  set_to_memory(mem, idx, 0);
		    else
			  set_to_lvariable(lval, idx, 0);

	}

	clr_vector(res);
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
		  unsigned idx;
		  unsigned bit_limit = wid - cur_rbit;
		  lval = ivl_stmt_lval(net, lidx);

		    /* If there is a mux for the lval, calculate the
		       value and write it into index0. */
		  if (ivl_lval_mux(lval))
			calculate_into_x0(ivl_lval_mux(lval));

		  mem = ivl_lval_mem(lval);
		  if (mem)
			draw_memory_index_expr(mem, ivl_lval_idx(lval));

		  if (bit_limit > ivl_lval_pins(lval))
			bit_limit = ivl_lval_pins(lval);

		  for (idx = 0 ;  idx < bit_limit ;  idx += 1) {
			if (mem)
			      assign_to_memory(mem, idx, 
					       bitchar_to_idx(bits[cur_rbit]),
					       delay);
			else if (del != 0)
			      assign_to_lvariable(lval, idx,
						  bitchar_to_idx(bits[cur_rbit]),
						  1, 1);
			else
			      assign_to_lvariable(lval, idx,
						  bitchar_to_idx(bits[cur_rbit]),
						  delay, 0);
			cur_rbit += 1;
		  }

		  for (idx = bit_limit; idx < ivl_lval_pins(lval); idx += 1)
			if (mem)
			      assign_to_memory(mem, idx, 0, delay);
			else if (del != 0)
			      assign_to_lvariable(lval, idx, 0,
						  1, 1);
			else
			      assign_to_lvariable(lval, idx, 0,
						  delay, 0);

	    }
	    return 0;
      }


      { struct vector_info res = draw_eval_expr(rval);
        unsigned wid = res.wid;
	unsigned lidx;
	unsigned cur_rbit = 0;

	if (del != 0)
	      calculate_into_x1(del);

	for (lidx = 0 ;  lidx < ivl_stmt_lvals(net) ;  lidx += 1) {
	      unsigned idx;
	      unsigned bit_limit = wid - cur_rbit;
	      lval = ivl_stmt_lval(net, lidx);

		/* If there is a mux for the lval, calculate the
		   value and write it into index0. */
	      if (ivl_lval_mux(lval))
		    calculate_into_x0(ivl_lval_mux(lval));


	      mem = ivl_lval_mem(lval);
	      if (mem) 
		    draw_memory_index_expr(mem, ivl_lval_idx(lval));

	      if (bit_limit > ivl_lval_pins(lval))
		    bit_limit = ivl_lval_pins(lval);

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

	      for (idx = bit_limit ;  idx < ivl_lval_pins(lval) ;  idx += 1)
		    if (mem)
			  assign_to_memory(mem, idx, 0, delay);
		    else if (del != 0)
			  assign_to_lvariable(lval, idx, 0, 1, 1);
		    else
			  assign_to_lvariable(lval, idx, 0, delay, 0);

	}

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

static int show_stmt_case(ivl_statement_t net, ivl_scope_t sscope)
{
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(exp);
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

	    cvec = draw_eval_expr_wid(cex, cond.wid);
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
	    show_statement(cst, sscope);

	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count,
		    local_base+count);

      }

	/* The out of the case. */
      fprintf(vvp_out, "T_%d.%d ;\n",  thread_count, local_base+count);

      return 0;
}

static int show_stmt_cassign(ivl_statement_t net)
{
      ivl_lval_t lval;
      ivl_signal_t lsig;
      unsigned idx;

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_signal_pins(lsig) == ivl_stmt_nexus_count(net));
      assert(ivl_lval_part_off(lval) == 0);

      for (idx = 0 ;  idx < ivl_stmt_nexus_count(net) ;  idx += 1) {
	    fprintf(vvp_out, "    %%cassign V_%s[%u], %s;\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx,
		    draw_net_input(ivl_stmt_nexus(net, idx)));
      }

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
		    vvp_mangle_id(ivl_signal_name(lsig)), idx);
      }
      return 0;
}

static int show_stmt_condit(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_false, lab_out;
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cond = draw_eval_expr(exp);

      assert(cond.wid == 1);

      lab_false = local_count++;
      lab_out = local_count++;

      fprintf(vvp_out, "    %%jmp/0xz  T_%d.%d, %u;\n",
	      thread_count, lab_false, cond.base);

	/* Done with the condition expression. */
      clr_vector(cond);

      if (ivl_stmt_cond_true(net))
	    rc += show_statement(ivl_stmt_cond_true(net), sscope);

      if (ivl_stmt_cond_false(net)) {
	    fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, lab_out);
	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_false);

	    rc += show_statement(ivl_stmt_cond_false(net), sscope);

	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_out);

      } else {
	    fprintf(vvp_out, "T_%d.%u ;\n", thread_count, lab_false);
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

      { struct vector_info del = draw_eval_expr(exp);
        fprintf(vvp_out, "    %%ix/get 0, %u, %u;\n", del.base, del.wid);
	clr_vector(del);
      }

      fprintf(vvp_out, "    %%delayx 0;\n");

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

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_part_off(lval) == 0);

      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "f_%s.%u .force V_%s[%u], %s;\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx,
		    vvp_mangle_id(ivl_signal_name(lsig)), idx,
		    draw_net_input(ivl_stmt_nexus(net, idx)));
      }

      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "    %%force f_%s.%u, 1;\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx);
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
      static int transient_id = 0;
      unsigned cnt = ivl_stmt_block_count(net);

      int out = transient_id++;

	/* Draw a fork statement for all but one of the threads of the
	   fork/join. Send the threads off to a bit of code where they
	   are implemented. */
      for (idx = 0 ;  idx < cnt-1 ;  idx += 1) {
	    fprintf(vvp_out, "    %%fork t_%u, S_%s;\n",
		    transient_id+idx, 
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
	    fprintf(vvp_out, "t_%u ;\n", transient_id+idx);
	    rc += show_statement(ivl_stmt_block_stmt(net, idx), sscope);
	    fprintf(vvp_out, "    %%end;\n");
      }

	/* This is the label for the out. Use this to branch around
	   the implementations of all the child threads. */
      fprintf(vvp_out, "t_%u ;\n", out);

      transient_id += cnt-1;

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

      assert(ivl_stmt_lvals(net) == 1);
      lval = ivl_stmt_lval(net, 0);

      lsig = ivl_lval_sig(lval);
      assert(lsig != 0);
      assert(ivl_lval_mux(lval) == 0);
      assert(ivl_lval_part_off(lval) == 0);

      for (idx = 0 ;  idx < ivl_lval_pins(lval) ; idx += 1) {
	    fprintf(vvp_out, "    %%load 4, V_%s[%u];\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx);
	    fprintf(vvp_out, "    %%set V_%s[%u], 4;\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx);
	    fprintf(vvp_out, "    %%release V_%s[%u];\n",
		    vvp_mangle_id(ivl_signal_name(lsig)), idx);
      }
      return 0;
}

static int show_stmt_repeat(ivl_statement_t net, ivl_scope_t sscope)
{
      int rc = 0;
      unsigned lab_top = local_count++, lab_out = local_count++;
      ivl_expr_t exp = ivl_stmt_cond_expr(net);
      struct vector_info cnt = draw_eval_expr(exp);

	/* Test that 0 < expr */
      fprintf(vvp_out, "T_%u.%u %%cmp/u 0, %u, %u;\n", thread_count,
	      lab_top, cnt.base, cnt.wid);
      fprintf(vvp_out, "    %%jmp/0xz T_%u.%u, 5;\n", thread_count, lab_out);
	/* This adds -1 (all ones in 2's complement) to the count. */
      fprintf(vvp_out, "    %%add %u, 1, %u;\n", cnt.base, cnt.wid);

      rc += show_statement(ivl_stmt_sub_stmt(net), sscope);

      fprintf(vvp_out, "    %%jmp T_%u.%u;\n", thread_count, lab_top);
      fprintf(vvp_out, "T_%u.%u ;\n", thread_count, lab_out);

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
      return 0;
}

static int show_stmt_wait(ivl_statement_t net, ivl_scope_t sscope)
{
      ivl_event_t ev = ivl_stmt_event(net);
      fprintf(vvp_out, "    %%wait E_%s;\n", 
	      vvp_mangle_id(ivl_event_name(ev)));

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

      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, top_label);

	/* Draw the evaluation of the condition expression, and test
	   the result. If the expression evaluates to false, then
	   branch to the out label. */
      cvec = draw_eval_expr(ivl_stmt_cond_expr(net));
      if (cvec.wid > 1)
	    cvec = reduction_or(cvec);

      fprintf(vvp_out, "    %%jmp/0xz T_%d.%d, %u;\n",
	      thread_count, out_label, cvec.base);
      clr_vector(cvec);

	/* Draw the body of the loop. */
      rc += show_statement(ivl_stmt_sub_stmt(net), sscope);

	/* This is the bottom of the loop. branch to the top where the
	   test is repeased, and also draw the out label. */
      fprintf(vvp_out, "    %%jmp T_%d.%d;\n", thread_count, top_label);
      fprintf(vvp_out, "T_%d.%d ;\n", thread_count, out_label);
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
	    return 0;
      }

      for (idx = 0 ;  idx < parm_count ;  idx += 1) {
	    ivl_expr_t expr = ivl_stmt_parm(net, idx);
	    
	    switch (ivl_expr_type(expr)) {
		case IVL_EX_NONE:
		case IVL_EX_NUMBER:
		case IVL_EX_STRING:
		case IVL_EX_SCOPE:
		case IVL_EX_SFUNC:
		  continue;

		case IVL_EX_SIGNAL:
		    /* If the signal node is narrower then the signal
		       itself, then this is a part select so I'm going
		       to need to evaluate the expression. */
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
		default:
		  break;
	    }

	    vec = (struct vector_info *)
		  realloc(vec, (vecs+1)*sizeof(struct vector_info));
	    vec[vecs] = draw_eval_expr(expr);
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
				vvp_mangle_id(ivl_expr_name(expr)));
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
			fprintf(vvp_out, ", $time");
		  else
			fprintf(vvp_out, ", ?");
		  continue;
		  
		case IVL_EX_MEMORY:
		  if (!ivl_expr_oper1(expr)) {
			fprintf(vvp_out, ", M_%s", 
				vvp_mangle_id(ivl_expr_name(expr)));
			continue;
		  }
		  break;

		default:
		  break;
	    }
	    assert(veci < vecs);
	    fprintf(vvp_out, ", T<%u,%u,%s>", vec[veci].base,
		    vec[veci].wid, ivl_expr_signed(expr)? "s" : "u");
	    veci++;
      }
      
      assert(veci == vecs);

      if (vecs) {
	    for (idx = 0; idx < vecs; idx++)
		  clr_vector(vec[idx]);
	    free(vec);
      }

      fprintf(vvp_out, ";\n");

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

      assert(def);
      rc += show_statement(def, scope);

      fprintf(vvp_out, "    %%end;\n");

      thread_count += 1;
      return rc;
}

/*
 * $Log: vvp_process.c,v $
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
 *
 * Revision 1.43  2001/08/26 23:00:13  steve
 *  Generate code for l-value bit selects.
 *
 * Revision 1.42  2001/08/25 23:50:03  steve
 *  Change the NetAssign_ class to refer to the signal
 *  instead of link into the netlist. This is faster
 *  and uses less space. Make the NetAssignNB carry
 *  the delays instead of the NetAssign_ lval objects.
 *
 *  Change the vvp code generator to support multiple
 *  l-values, i.e. concatenations of part selects.
 *
 * Revision 1.41  2001/08/16 03:45:17  steve
 *  statement ends after while loop labels.
 *
 * Revision 1.40  2001/07/28 01:18:07  steve
 *  Evaluate part selects when passed to system tasks.
 *
 * Revision 1.39  2001/07/19 04:55:06  steve
 *  Support calculated delays in vvp.tgt.
 *
 * Revision 1.38  2001/06/29 02:41:05  steve
 *  Handle null parameters to system tasks.
 *
 * Revision 1.37  2001/06/23 00:30:42  steve
 *  Handle short inputs to tasks. (Stephan Boettcher)
 *
 * Revision 1.36  2001/06/18 03:10:34  steve
 *   1. Logic with more than 4 inputs
 *   2. Id and name mangling
 *   3. A memory leak in draw_net_in_scope()
 *   (Stephan Boettcher)
 *
 * Revision 1.35  2001/05/24 04:31:00  steve
 *  Attach noops to case labels.
 *
 * Revision 1.34  2001/05/17 04:37:02  steve
 *  Behavioral ternary operators for vvp.
 *
 * Revision 1.33  2001/05/10 00:26:53  steve
 *  VVP support for memories in expressions,
 *  including general support for thread bit
 *  vectors as system task parameters.
 *  (Stephan Boettcher)
 *
 * Revision 1.32  2001/05/08 23:59:33  steve
 *  Add ivl and vvp.tgt support for memories in
 *  expressions and l-values. (Stephan Boettcher)
 *
 * Revision 1.31  2001/05/03 04:55:28  steve
 *  Generate null statements for conditional labels.
 *
 * Revision 1.30  2001/04/21 03:26:23  steve
 *  Right shift by constant.
 *
 * Revision 1.29  2001/04/21 00:55:46  steve
 *  Generate code for disable.
 *
 * Revision 1.28  2001/04/18 05:12:03  steve
 *  Use the new %fork syntax.
 *
 * Revision 1.27  2001/04/15 02:58:11  steve
 *  vvp support for <= with internal delay.
 *
 * Revision 1.26  2001/04/06 02:28:03  steve
 *  Generate vvp code for functions with ports.
 *
 * Revision 1.25  2001/04/05 03:20:58  steve
 *  Generate vvp code for the repeat statement.
 *
 * Revision 1.24  2001/04/04 04:50:35  steve
 *  Support forever loops in the tgt-vvp target.
 *
 * Revision 1.23  2001/04/04 04:28:41  steve
 *  Fix broken look scanning down bits of number.
 *
 * Revision 1.22  2001/04/04 04:14:09  steve
 *  emit vpi parameters values as vectors.
 *
 * Revision 1.21  2001/04/03 04:50:37  steve
 *  Support non-blocking assignments.
 *
 * Revision 1.20  2001/04/02 04:09:20  steve
 *  thread bit allocation leak in assign.
 *
 * Revision 1.19  2001/04/02 02:28:13  steve
 *  Generate code for task calls.
 *
 * Revision 1.18  2001/04/02 00:27:53  steve
 *  Scopes and numbers as vpi_call parameters.
 *
 * Revision 1.17  2001/04/01 06:49:04  steve
 *  Generate code for while statements.
 */

